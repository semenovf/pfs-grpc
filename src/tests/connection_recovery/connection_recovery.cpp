////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-grpc](https://github.com/semenovf/pfs-grpc) library.
//
// Changelog:
//      2019.10.20 Initial version
////////////////////////////////////////////////////////////////////////////////
#include "pfs/grpc/async_client.hpp"
#include "pfs/grpc/async_server.hpp"
#include "connection_recovery.pb.h"
#include "connection_recovery.grpc.pb.h"
#include <mutex>
#include <condition_variable>
#include <thread>

static std::string const SERVER_ADDR = "localhost:1521";
static std::mutex client_mutex;
static std::mutex server_mutex;
static std::condition_variable client_cv;
static std::condition_variable server_cv;
static std::atomic_flag finish_server_flag = ATOMIC_FLAG_INIT;

inline std::ostream & client_log ()
{
    std::cout << "Client: ";
    return std::cout;
}

inline std::ostream & server_log ()
{
    std::cout << "Server: ";
    return std::cout;
}

void sleep (int seconds)
{
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

inline void finish_server ()
{
    finish_server_flag.test_and_set();
}

inline bool finish_server_predicate ()
{
    auto result = finish_server_flag.test_and_set();
    finish_server_flag.clear();
    return result;
}

// package PingPongNs____________________________________________
// service PingPongService { ... _______________________________|_____________
//                                                              |            |
//                                                              v            v
class concrete_async_server : public pfs::grpc::async_server<PingPongNs::PingPongService>
{
    using base_class = pfs::grpc::async_server<PingPongNs::PingPongService>;
    using service_class = base_class::service_class;
    using service_type = base_class::service_type;

public:
    concrete_async_server () : base_class() {}
    concrete_async_server (std::string const & server_addr)
        : base_class(server_addr)
    {}

    void register_ping ()
    {
//  rpc PingPong(Ping) returns (Pong) {}
//                 |________________________
//                                         |
//                                         v
        using request_type = PingPongNs::Ping;

//  rpc PingPong(Ping) returns (Pong) {}
//                                |__________
//                                          |
//                                          v
        using response_type = PingPongNs::Pong;

        using method_type = pfs::grpc::async_unary_method<
                  service_class
                , request_type
                , response_type>;

        auto f = [this] (request_type const &, response_type *) {
            server_log() << "ping\n";
        };

        server_log() << "pong\n";

//  rpc PingPong(Ping) returns (Pong) {}
//         |__________________________________________________________________
//                                                                    |      |
        register_method<request_type, response_type>(//               v      v
                  & PingPongNs::PingPongService::AsyncService::RequestPingPong
                , method_type::request_handler(f));
    }
};

// package PingPongNs____________________________________________
// service PingPongService { ... _______________________________|_______________
//                                                              |              |
//                                                              v              v
class concrete_async_client : public pfs::grpc::async_client<PingPongNs::PingPongService>
{
    using base_class = pfs::grpc::async_client<PingPongNs::PingPongService>;

    std::chrono::system_clock::time_point _last_pong_time_point;

public:
    concrete_async_client () : base_class()
    {
        _last_pong_time_point = std::chrono::system_clock::now();
    }

////////////////////////////////////////////////////////////////////////////////
// rpc PingPong(Ping) returns (Pong) {}
////////////////////////////////////////////////////////////////////////////////
    bool ping ()
    {
        auto f = [this] (PingPongNs::Pong const &) {
            client_log() << "pong\n";
            _last_pong_time_point = std::chrono::system_clock::now();
        };

        client_log() << "ping\n";

        return this->call(PingPongNs::Ping{}
                , & PingPongNs::PingPongService::Stub::PrepareAsyncPingPong
                , pfs::grpc::async_unary<PingPongNs::Pong>::response_handler(f));
    }
};

////////////////////////////////////////////////////////////////////////////////
// client
////////////////////////////////////////////////////////////////////////////////
void client ()
{
    concrete_async_client client;

    client_log() << "attempt to connect server while it is not started\n";

    if (!client.connect(SERVER_ADDR, 2000)) {
        client_log() << "connection failure\n";
    }

    client_log() << "notify server to start\n";

    // Notify to run server
    server_cv.notify_one();

    client_log() << "waiting for server started ...\n";

    std::unique_lock<std::mutex> locker(server_mutex);
    client_cv.wait(locker);

    client_log() << "server started\n";

    if (!client.connect(SERVER_ADDR, 2000)) {
        client_log() << "connection failure\n";
    }

    client.ping();

    client_log() << "finishing server\n";
    finish_server();
}

////////////////////////////////////////////////////////////////////////////////
// server
////////////////////////////////////////////////////////////////////////////////
void server ()
{
    concrete_async_server * server = nullptr;
    bool server_started = false;

    std::unique_lock<std::mutex> locker(client_mutex);
    server_cv.wait(locker);

    server_log() << "starting ...\n";

    server = new concrete_async_server;

    if (server->listen(SERVER_ADDR)) {
        server_started = true;
        server->register_ping();
        server_log() << "started\n";
    } else {
        server_log() << "starting failure\n";
    }

    // Notify client server started (or not)
    client_cv.notify_one();

    if (server_started) {
        server_log() << "run\n";
        server->run(finish_server_predicate);
    } else {
        return;
    }
}

int main ()
{
    std::thread server_thread { [] { server(); }};
    std::thread client_thread { [] { client(); }};

    server_thread.join();
    client_thread.join();

    return 0;
}
