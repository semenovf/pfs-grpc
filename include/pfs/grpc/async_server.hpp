////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-io](https://github.com/semenovf/pfs-grpc) library.
//
// Changelog:
//      2019.10.21 Initial version
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <grpcpp/grpcpp.h>
#include <memory>

////////////////////////////////////////////////////////////////////////////////
// REFERENCES:
//
// 1.[gRPC Basics - C++](https://grpc.io/docs/tutorials/basic/cpp/)
// 2.[helloworld/greeter_async_server.cc](https://github.com/grpc/grpc/blob/master/examples/cpp/helloworld/greeter_async_server.cc)
////////////////////////////////////////////////////////////////////////////////

namespace pfs {
namespace grpc {

using server_completion_queue = ::grpc::ServerCompletionQueue;
using completion_queue = ::grpc::CompletionQueue;
using rpc_server = ::grpc::Server;
using server_builder = ::grpc::ServerBuilder;
using server_credentials_pointer = std::shared_ptr<::grpc::ServerCredentials>;
using server_context_type = ::grpc::ServerContext;
using status_type = ::grpc::Status;

////////////////////////////////////////////////////////////////////////////////
// basic_async_method
////////////////////////////////////////////////////////////////////////////////
template <typename ServiceClass>
class basic_async_method
{
public:
    using service_class = ServiceClass;
    using service_type = typename service_class::AsyncService;

protected:
    bool _complete = false; // is response complete
    service_type * _service;
    server_completion_queue * _cqueue;

    // Context for the rpc, allowing to tweak aspects of it such as the use
    // of compression, authentication, as well as to send metadata back to the
    // client.
    server_context_type _ctx;

public:
    basic_async_method (service_type * service, server_completion_queue * cqueue)
        : _service(service)
        , _cqueue(cqueue)
    {}

    server_context_type * server_context_ptr ()
    {
        return & _ctx;
    }

    virtual ~basic_async_method () {}

    virtual void process_request (bool ok) = 0;
};

////////////////////////////////////////////////////////////////////////////////
// async_unary_method
////////////////////////////////////////////////////////////////////////////////
template <typename ServiceClass, typename RequestType, typename ResponseType>
class async_unary_method : public basic_async_method<ServiceClass>
{
    using base_class = basic_async_method<ServiceClass>;

public:
    using service_type = typename base_class::service_type;
    using request_type = RequestType;
    using response_type = ResponseType;
    using async_interface = ::grpc::ServerAsyncResponseWriter<response_type>;
    typedef void (service_type::* request_registrar_type) (
              server_context_type *
            , RequestType *
            , async_interface *
            , completion_queue *
            , server_completion_queue *
            , void * tag);

    using request_handler = std::function<void (request_type const &
            , response_type *)>;

protected:
    request_type    _request;
    response_type   _response;
    async_interface _rpc;
    request_registrar_type _register_request;
    request_handler _on_request;

protected:
    async_unary_method (async_unary_method * spawner)
        : base_class(spawner->_service, spawner->_cqueue)
        , _rpc(base_class::server_context_ptr())
        , _register_request(spawner->_register_request)
        , _on_request(spawner->_on_request)
    {
        (this->_service->*_register_request)(& this->_ctx
                , & _request
                , & _rpc
                , this->_cqueue
                , this->_cqueue
                , this);
    }

public:
    async_unary_method (service_type * service, server_completion_queue * cqueue)
        : base_class(service, cqueue)
        , _rpc(base_class::server_context_ptr())
    {}

    void register_method (request_registrar_type register_request
            , request_handler && on_request)
    {
        _register_request = register_request;
        _on_request = std::forward<request_handler>(on_request);
        (this->_service->* _register_request)(& this->_ctx
                , & _request
                , & _rpc
                , this->_cqueue
                , this->_cqueue
                , this);

////////// UNARY /////////////////
//         void RequestCallStart(
//                   ::grpc::ServerContext * context
//                 , ::TestRpc::StartService * request
//                 , ::grpc::ServerAsyncResponseWriter<::TestRpc::ServiceStatus> * response
//                 , ::grpc::CompletionQueue * new_call_cq
//                 , ::grpc::ServerCompletionQueue * notification_cq
//                 , void * tag)
//
//         void RequestCallStop(
//                   ::grpc::ServerContext * context
//                 , ::TestRpc::StopService * request
//                 , ::grpc::ServerAsyncResponseWriter<::TestRpc::ServiceStatus> * response
//                 , ::grpc::CompletionQueue * new_call_cq
//                 , ::grpc::ServerCompletionQueue * notification_cq
//                 , void * tag)
//
////////// Server-side streaming RPC
//         void RequestCallListModules(
//                    ::grpc::ServerContext * context
//                  , ::TestRpc::ListModules * request
//                  , ::grpc::ServerAsyncWriter<::TestRpc::ModuleStatus> * writer
//                  , ::grpc::CompletionQueue * new_call_cq
//                  , ::grpc::ServerCompletionQueue * notification_cq
//                  , void * tag)
//
////////// Client-side streaming RPC
//         void RequestSendSegment(
//                   ::grpc::ServerContext * context
//                 , ::grpc::ServerAsyncReader<::TestRpc::Complete, ::TestRpc::Segment> * reader
//                 , ::grpc::CompletionQueue * new_call_cq
//                 , ::grpc::ServerCompletionQueue * notification_cq
//                 , void * tag)
//
////////// Bidirectional streaming RPC
//         void RequestCallStartModule(
//                    ::grpc::ServerContext* context
//                  , ::grpc::ServerAsyncReaderWriter<::TestRpc::ModuleStatus, ::TestRpc::StartModule> * stream
//                  , ::grpc::CompletionQueue * new_call_cq
//                  , ::grpc::ServerCompletionQueue * notification_cq
//                  , void * tag)
    }

    virtual void process_request (bool ok) override
    {
        // FIXME Handle error
        GPR_ASSERT(ok);

        if (! this->_complete) {
            // Spawn a new 'async_unary_method' instance to serve new clients
            // while we process the one for this 'async_unary_method'.
            // The instance will deallocate itself as part of its FINISH state.
            new async_unary_method(this);//this->_service, this->_cqueue);

            // The actual processing.
            _on_request(_request, & _response);

            // And we are done! Let the gRPC runtime know we've finished, using the
            // memory address of this instance as the uniquely identifying tag for
            // the event.
            this->_complete = true;
            _rpc.Finish(_response, status_type::OK, this);
        } else {
            delete this;
        }
    }
};

////////////////////////////////////////////////////////////////////////////////
// async_stream_server_method
////////////////////////////////////////////////////////////////////////////////
// template <typename ServiceType, typename RequestType, typename ResponseType>
// class async_stream_server_method : public basic_async_method<ServiceType>
// {
//     using base_class = basic_async_method<ServiceType>;
//
// public:
//     using service_type = typename base_class::service_type;
//     using request_type = RequestType;
//     using response_type = ResponseType;
//     using async_interface = ::grpc::ServerAsyncWriter<response_type>;
//
// protected:
//     request_type    _request;
//     response_type   _response;
//     async_interface _rpc;
//
// public:
//     async_stream_server_method (service_type * service, server_completion_queue * cqueue)
//         : base_class(service, cqueue)
//         , _rpc(& base_class::_ctx)
//     {
// //         _service->RequestSayHello(& _ctx
// //                 , & _request
// //                 , & _rpc
// //                 , cqueue
// //                 , cqueue
// //                 , this);
//     }
//
//     virtual void process_request (bool = true) override
//     {
// //         if (! _complete) {
// //             new CallData(service_, cq_);
// // //             reply_.set_message(prefix + request_.name());
// //             _complete = true;
// //             _rpc.Finish(_response, status_type::OK, this);
// //         } else {
// //             delete this;
// //         }
//     }
// };

////////////////////////////////////////////////////////////////////////////////
// async_server
////////////////////////////////////////////////////////////////////////////////
template <typename ServiceClass>
class async_server
{
public:
    using service_class = ServiceClass;
    using service_type = typename service_class::AsyncService;

protected:
    std::unique_ptr<server_completion_queue> _cqueue;
    std::unique_ptr<rpc_server> _server;
    service_type _service;

public:
    async_server (std::string const & server_addr
            , server_credentials_pointer creds = ::grpc::InsecureServerCredentials())
    {
        server_builder builder;

        // Listen on the given address without any authentication mechanism.
        builder.AddListeningPort(server_addr, creds);

        // Register "service" as the instance through which we'll communicate
        // with clients. In this case it corresponds to an *asynchronous* service.
        builder.RegisterService(& _service);

        // Get hold of the completion queue used for the asynchronous communication
        // with the gRPC runtime.
        _cqueue = builder.AddCompletionQueue();

        // Finally assemble the server.
        _server = builder.BuildAndStart();
    }

    ~async_server ()
    {
        _server->Shutdown();
        _cqueue->Shutdown();
    }

    // There is no shutdown handling in this code.
    void run ()
    {
        void * tag;  // uniquely identifies a request.
        bool ok;

        while (true) {
            // Block waiting to read the next event from the completion queue. The
            // event is uniquely identified by its tag, which in this case is the
            // memory address of a CallData instance.
            // The return value of Next should always be checked. This return value
            // tells us whether there is any kind of event or _cqueue is shutting down.
            GPR_ASSERT(_cqueue->Next(& tag, & ok));
            static_cast<basic_async_method<ServiceClass> *>(tag)->process_request(ok);
        }
    }

    // Register unary method
    template <typename RequestType, typename ResponseType>
    bool register_method (typename async_unary_method<ServiceClass, RequestType, ResponseType>::request_registrar_type register_request
            ,  typename async_unary_method<ServiceClass, RequestType, ResponseType>::request_handler && on_request)
    {
        using async_method = async_unary_method<ServiceClass, RequestType, ResponseType>;

        auto m = new async_method(& _service, _cqueue.get());
        m->register_method(register_request
                , std::forward<typename async_method::request_handler>(on_request));
        return true;
    }
};

}} // pfs::grpc
