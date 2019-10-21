////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-io](https://github.com/semenovf/pfs-grpc) library.
//
// Changelog:
//      2019.10.20 Initial version
////////////////////////////////////////////////////////////////////////////////
#include "pfs/grpc/async_client.hpp"
#include "pfs/grpc/async_server.hpp"
#include "test.pb.h"
#include "test.grpc.pb.h"
#include <thread>

#include "../catch.hpp"

static std::string const SERVER_ADDR = "localhost:1521";

// package TestRpc_______________________________________
// service TestService { ... ___________________________|__________
//                                                      |         |
//                                                      V         V
class concrete_async_server : public pfs::grpc::async_server<TestRpc::TestService>
{
    using base_class = pfs::grpc::async_server<TestRpc::TestService>;
    using service_class = base_class::service_class;
    using service_type = base_class::service_type;

public:
    concrete_async_server (std::string const & server_addr)
        : base_class(server_addr)
    {}

    void register_CallStart ()
    {
        using request_type = TestRpc::StartService;
        using response_type = TestRpc::ServiceStatus;
        using method_type = pfs::grpc::async_unary_method<
                  service_class
                , request_type
                , response_type>;

        auto f = [] (request_type const &, response_type * response) {
            std::cout << "server: StartService request\n";
            response->set_status(TestRpc::SERVICE_STARTED);
        };

        register_method<request_type, response_type>(
                  & TestRpc::TestService::AsyncService::RequestCallStart
                , method_type::request_handler(f));
    }

    void register_CallStop ()
    {
        using request_type = TestRpc::StopService;
        using response_type = TestRpc::ServiceStatus;
        using method_type = pfs::grpc::async_unary_method<
                  service_class
                , request_type
                , response_type>;

        auto f = [] (request_type const &, response_type * response) {
            std::cout << "server: StopService request\n";
            response->set_status(TestRpc::SERVICE_STOPPED);
        };

        register_method<request_type, response_type>(
                  & TestRpc::TestService::AsyncService::RequestCallStop
                , method_type::request_handler(f));
    }
};

// package TestRpc_______________________________________________
// service TestService { ... ___________________________________|__________
//                                                              |         |
//                                                              V         V
class concrete_async_client : public pfs::grpc::async_client<TestRpc::TestService>
{
    using base_class = pfs::grpc::async_client<TestRpc::TestService>;

public:
    concrete_async_client (std::string const & server_addr)
        : base_class(server_addr)
    {}

////////////////////////////////////////////////////////////////////////////////
// rpc CallStart(StartService) returns (ServiceStatus) {}
////////////////////////////////////////////////////////////////////////////////
    bool call_start ()
    {
        auto f = [] (TestRpc::ServiceStatus const & response) {
            std::cout << "client: StartService result: "
                    << "status=" << response.status() << "\n";
        };

        // Name 'PrepareAsyncCallStart' generates using concatenation of static
        // string 'PrepareAsync' and rpc name from .proto file ('CallStart').
        // This rule is valid for 'Simple RPC'
        return this->call(TestRpc::StartService{}
                , & TestRpc::TestService::Stub::PrepareAsyncCallStart
                , pfs::grpc::async_unary<TestRpc::ServiceStatus>::response_handler(f));
    }

////////////////////////////////////////////////////////////////////////////////
// rpc CallStop(StopService) returns (ServiceStatus) {}
////////////////////////////////////////////////////////////////////////////////
    bool call_stop ()
    {
        auto f = [] (TestRpc::ServiceStatus const & response) {
            std::cout << "client: StopService result: "
                    << "status=" << response.status() << "\n";
        };

        // Same as for 'call_start'
        return this->call(TestRpc::StopService{}
                , & TestRpc::TestService::Stub::PrepareAsyncCallStop
                , pfs::grpc::async_unary<TestRpc::ServiceStatus>::response_handler(f));
    }
};

TEST_CASE("Asynchronous RPC") {
    concrete_async_server server(SERVER_ADDR);
    concrete_async_client client(SERVER_ADDR);

    server.register_CallStart();
    server.register_CallStop();

    std::thread server_thread {
        [& server] {
            server.run();
    }};

    std::thread client_response_reader {
        [& client] {
            client.process();
    }};

    std::cout << "=== callStart === " << std::endl;
    client.call_start();

    std::cout << "=== callStop === " << std::endl;
    client.call_stop();

    std::cout << "=== callStart === " << std::endl;
    client.call_start();

//     std::cout << "=== enableStatus === " << std::endl;
//     enableStatus(& sptAsyncClient);
//
//     std::cout << "=== enableCommands === " << std::endl;
//     enableCommands(& sptAsyncClient);
//
//     std::cout << "=== pushGreeting === " << std::endl;
//     pushGreeting(& sptAsyncClient
//             , "Hello, "
//             , " Brave New"
//             , " World!");
//
//     std::cout << "=== pushPopGreeting === " << std::endl;
//     pushPopGreeting(& sptAsyncClient
//             , "Hello, "
//             , " Brave New"
//             , " World!");
//
//     std::cout << "=== Calls complete, waiting responses === " << std::endl;

    server_thread.join();
    client_response_reader.join();
}
