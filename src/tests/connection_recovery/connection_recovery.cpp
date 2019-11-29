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
#include <cassert>

#if _WIN32
#	include <winsock2.h>
#	include <ws2tcpip.h>
#else
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <arpa/inet.h>
#	include <netinet/in.h>
#	include <unistd.h>
#endif

static unsigned short int SERVER_PORT = 1521;
static std::string const SERVER_NAME = "127.0.0.1";
static std::string const SERVER_ADDR = "localhost:" + std::to_string(SERVER_PORT);
static std::mutex client_mutex;
static std::mutex server_mutex;
static std::condition_variable client_cv;
static std::condition_variable server_cv;
static std::atomic_flag finish_server_flag = ATOMIC_FLAG_INIT;
static std::atomic_flag finish_client_flag = ATOMIC_FLAG_INIT;

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

// package PingPongNs____________________________________________
// service PingPongService { ... _______________________________|_____________
//                                                              |            |
//                                                              v            v
class concrete_async_server : public pfs::grpc::async_server<PingPongNs::PingPongService>
{
    using base_class = pfs::grpc::async_server<PingPongNs::PingPongService>;
    using service_class = base_class::service_class;
    using service_type = base_class::service_type;

    bool _pong_activated = false;

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
//                               |___________
//                                          |
//                                          v
        using response_type = PingPongNs::Pong;

        using method_type = pfs::grpc::async_unary_method<
                  service_class
                , request_type
                , response_type>;

        auto f = [this] (request_type const &, response_type *) {
            server_log() << "ping\n";
            _pong_activated = true;
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

    bool server_timedout () const
    {
        auto now = std::chrono::system_clock::now();

//         if (now < _last_pong_time_point)
//             return true;
//
//         if (now - _last_pong_time_point > std::chrono::seconds(5))
//             return true;

        return false;
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

#if _WIN32
		// Workaround for MSVC 2015 compiler error: could not deduce template
		// argument for ...
		return this->call<PingPongNs::Ping, PingPongNs::Pong>(PingPongNs::Ping{}
			, &PingPongNs::PingPongService::Stub::PrepareAsyncPingPong
			, pfs::grpc::async_unary<PingPongNs::Pong>::response_handler(f));
#else
		return this->call(PingPongNs::Ping{}
			, &PingPongNs::PingPongService::Stub::PrepareAsyncPingPong
			, pfs::grpc::async_unary<PingPongNs::Pong>::response_handler(f));
#endif
    }
};

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
bool server_is_alive ()
{
    sockaddr_in  serveraddr4;
    memset(& serveraddr4, 0, sizeof(serveraddr4));
    serveraddr4.sin_family = AF_INET;
    serveraddr4.sin_port   = htons(SERVER_PORT);
    assert(inet_pton(AF_INET, SERVER_NAME.c_str(), & serveraddr4.sin_addr.s_addr) > 0);

    auto fd = ::socket(AF_INET, SOCK_STREAM, 0);
    auto rc = ::connect(fd, reinterpret_cast<sockaddr *>(& serveraddr4), sizeof(serveraddr4));

    bool result = true;

    if (rc < 0) {
        if (errno == ECONNREFUSED)
            result = false;
    }

#if _WIN32
	// Close the socket to release the resources associated
	// Normally an application calls shutdown() before closesocket 
	//   to  disables sends or receives on a socket first
	shutdown(fd, SD_BOTH);
	rc = closesocket(fd);
	if (rc == SOCKET_ERROR) {
		wprintf(L"closesocket failed with error = %d\n", WSAGetLastError());
	}
#else
    ::close(fd);
#endif

    return result;
}

inline void check_server_alive (int counter)
{
    if (server_is_alive())
        client_log() << "#" << counter << ": server is ALIVE\n";
    else
        client_log() << "#" << counter << ": server is not ALIVE\n";
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
inline void finish_server ()
{
    finish_server_flag.test_and_set();
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
inline void finish_client ()
{
    finish_client_flag.test_and_set();
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
inline bool finish_server_predicate ()
{
    auto result = finish_server_flag.test_and_set();
    finish_server_flag.clear();
    return result;
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
inline bool finish_client_predicate ()
{
    auto result = finish_client_flag.test_and_set();
    finish_client_flag.clear();
    return result;
}

////////////////////////////////////////////////////////////////////////////////
// client
////////////////////////////////////////////////////////////////////////////////
void client ()
{
    int counter = 0;
    concrete_async_client client;

    client_log() << "attempt to connect server while it is not started\n";

    if (!client.connect(SERVER_ADDR, 2000)) {
        client_log() << "connection failure\n";
    }

    client_log() << "notify server to start\n";

    // Notify to run server
    client_cv.notify_one();

    client_log() << "waiting for server started ...\n";

    std::unique_lock<std::mutex> locker(server_mutex);
    server_cv.wait(locker);

    client_log() << "server started\n";

    if (!client.connect(SERVER_ADDR, 2000)) {
        client_log() << "connection failure\n";
    }

    check_server_alive(++counter);

    std::thread client_reader { [& client] {
        client_log() << "start process responses\n";
        client.process(finish_client_predicate/* [& client] () -> bool {
            return client.server_timedout();
        }*/);
        client_log() << "finished process responses\n";
    }};

    check_server_alive(++counter);
    client.ping();

    client_log() << "finishing server\n";
    check_server_alive(++counter);
    finish_server();

    client_log() << "waiting for server finished ...\n";
    server_cv.wait(locker);
    client_log() << "server finished\n";

    check_server_alive(++counter);

    for (int i = 0; i < 2; i++) {
        client.ping();
        sleep(1);
    }

    check_server_alive(++counter);

    // Notify to run server
    client_cv.notify_one();

    client_log() << "waiting for server started ...\n";
    server_cv.wait(locker);
    client_log() << "server started\n";

    check_server_alive(++counter);

    for (int i = 0; i < 10; i++) {
        client.ping();
        sleep(1);
    }

    finish_server();
    client_reader.join();
}

////////////////////////////////////////////////////////////////////////////////
// server
////////////////////////////////////////////////////////////////////////////////
void server ()
{
    std::unique_ptr<concrete_async_server> server;
    bool server_started = false;

    // Wait while client notify to start server
    server_log() << "waiting for notification to start ...\n";
    std::unique_lock<std::mutex> locker(client_mutex);
    client_cv.wait(locker);

    server_log() << "starting ...\n";

    server.reset(new concrete_async_server);

    if (server->listen(SERVER_ADDR)) {
        server_started = true;
        server->register_ping();
        server_log() << "started\n";
    } else {
        server_log() << "starting failure\n";
    }

    // Notify client that server started (or not)
    server_cv.notify_one();

    if (server_started) {
        server_log() << "run\n";
        server->run(finish_server_predicate);
        server_log() << "finished\n";
        server.reset(nullptr);
    }

    server_cv.notify_one();

    client_cv.wait(locker);

    server_log() << "starting ...\n";

    server.reset(new concrete_async_server);

    if (server->listen(SERVER_ADDR)) {
        server_started = true;
        server->register_ping();
        server_log() << "started\n";
    } else {
        server_log() << "starting failure\n";
    }

    server_cv.notify_one();

    if (server_started) {
        server_log() << "run\n";
        server->run(finish_server_predicate);
        server_log() << "finished\n";
        server.reset(nullptr);
    }

    finish_client();
}

int main ()
{
#if _WIN32
	WSADATA wsaData = { 0 };
	// Initialize Winsock
	int rc = WSAStartup(MAKEWORD(2, 2), & wsaData);

	if (rc != 0) {
		wprintf(L"WSAStartup failed: %d\n", rc);
		return 1;
	}
#endif

    std::thread server_thread { [] { server(); }};
    std::thread client_thread { [] { client(); }};

    server_thread.join();
    client_thread.join();

#if _WIN32
	WSACleanup();
#endif

    return 0;
}
