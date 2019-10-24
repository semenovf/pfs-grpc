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
#include <list>
#include <memory>

////////////////////////////////////////////////////////////////////////////////
// REFERENCES:
//
// 1.[gRPC Basics - C++](https://grpc.io/docs/tutorials/basic/cpp/)
// 2.[helloworld/greeter_async_server.cc](https://github.com/grpc/grpc/blob/master/examples/cpp/helloworld/greeter_async_server.cc)
// 3.[gRPC environment variables](https://github.com/grpc/grpc/blob/master/doc/environment_variables.md)
////////////////////////////////////////////////////////////////////////////////

namespace pfs {
namespace grpc {

using server_completion_queue = ::grpc::ServerCompletionQueue;
using completion_queue = ::grpc::CompletionQueue;
using server_type = ::grpc::Server;
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
    typedef void (service_type::* request_registrar_func) (
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
    request_registrar_func _register_request;
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

    void register_method (request_registrar_func register_request
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
    }

    virtual void process_request (bool ok) override
    {
        // FIXME Handle error
        GPR_ASSERT(ok);

        if (! this->_complete) {
            // Spawn a new 'async_unary_method' instance to serve new clients
            // while we process the one for this 'async_unary_method'.
            // The instance will deallocate itself as part of its FINISH state.
            new async_unary_method(this);

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
// async_server_streaming_method
////////////////////////////////////////////////////////////////////////////////
template <typename ServiceClass, typename RequestType, typename ResponseType>
class async_server_streaming_method : public basic_async_method<ServiceClass>
{
    using base_class = basic_async_method<ServiceClass>;

public:
    using service_type = typename base_class::service_type;
    using request_type = RequestType;
    using response_type = ResponseType;
    using async_interface = ::grpc::ServerAsyncWriter<response_type>;
    typedef void (service_type::* request_registrar_func) (
              server_context_type *
            , RequestType *
            , async_interface *
            , completion_queue *
            , server_completion_queue *
            , void * tag);

    using request_handler = std::function<void (request_type const &
            , std::list<response_type> *)>;

protected:
    request_type    _request;
    response_type   _response;
    async_interface _rpc;
    request_registrar_func _register_request;
    request_handler _on_request;
    std::list<response_type> _responses;
    typename std::list<response_type>::iterator _current;

protected:
    async_server_streaming_method (async_server_streaming_method * spawner)
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
    async_server_streaming_method (service_type * service, server_completion_queue * cqueue)
        : base_class(service, cqueue)
        , _rpc(base_class::server_context_ptr())
    {}

    void register_method (request_registrar_func register_request
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
    }

    virtual void process_request (bool ok) override
    {
        if (!this->_complete) {
            if (_responses.empty()) {
                _on_request(_request, & _responses);
                _current = _responses.begin();
            }

            // SIGSEGV while call this->_ctx.IsCancelled()
            // TODO When this call is actually need?
            if (/*this->_ctx.IsCancelled() || */_current == _responses.end()) {
                this->_complete = true;
                _rpc.Finish(status_type{}, this);
                new async_server_streaming_method(this);
            } else {
                _rpc.Write(*_current, this);
                ++_current;
            }
        } else {
            delete this;
        }
    }
};

////////////////////////////////////////////////////////////////////////////////
// async_client_streaming_method
////////////////////////////////////////////////////////////////////////////////
template <typename ServiceClass, typename RequestType, typename ResponseType>
class async_client_streaming_method : public basic_async_method<ServiceClass>
{
    using base_class = basic_async_method<ServiceClass>;

public:
    using service_type = typename base_class::service_type;
    using request_type = RequestType;
    using response_type = ResponseType;
    using async_interface = ::grpc::ServerAsyncReader<response_type, request_type>;
    typedef void (service_type::* request_registrar_func) (
              server_context_type *
            , async_interface *
            , completion_queue *
            , server_completion_queue *
            , void * tag);

    using request_handler = std::function<void (std::list<request_type> const &
            , response_type *)>;

protected:
    async_interface         _rpc;
    request_registrar_func  _register_request;
    std::list<request_type> _requests;
    request_handler         _on_request;

protected:
    async_client_streaming_method (async_client_streaming_method * spawner)
        : base_class(spawner->_service, spawner->_cqueue)
        , _rpc(base_class::server_context_ptr())
        , _register_request(spawner->_register_request)
        , _on_request(spawner->_on_request)
    {
        (this->_service->*_register_request)(& this->_ctx
                , & _rpc
                , this->_cqueue
                , this->_cqueue
                , this);
    }

public:
    async_client_streaming_method (service_type * service, server_completion_queue * cqueue)
        : base_class(service, cqueue)
        , _rpc(base_class::server_context_ptr())
    {}

    void register_method (request_registrar_func register_request
            , request_handler && on_request)
    {
        _register_request = register_request;
        _on_request = std::forward<request_handler>(on_request);
        (this->_service->* _register_request)(& this->_ctx
                , & _rpc
                , this->_cqueue
                , this->_cqueue
                , this);
    }

    virtual void process_request (bool ok) override
    {
        if (! this->_complete) {
            if (ok) {
                _requests.emplace_back();
                auto & request = _requests.back();
                _rpc.Read(& request, this);
            } else {
                response_type response;
                _on_request(_requests, & response);
                this->_complete = true;
                _rpc.Finish(response, status_type{}, this);
                new async_client_streaming_method(this);
            }
        } else {
            delete this;
        }
    }
};

////////////////////////////////////////////////////////////////////////////////
// async_client_streaming_method
////////////////////////////////////////////////////////////////////////////////
template <typename ServiceClass, typename RequestType, typename ResponseType>
class async_bidi_streaming_method : public basic_async_method<ServiceClass>
{
    using base_class = basic_async_method<ServiceClass>;

public:
    using service_type = typename base_class::service_type;
    using request_type = RequestType;
    using response_type = ResponseType;
    using async_interface = ::grpc::ServerAsyncReaderWriter<response_type, request_type>;
    typedef void (service_type::* request_registrar_func) (
              server_context_type *
            , async_interface *
            , completion_queue *
            , server_completion_queue *
            , void * tag);

    using request_handler = std::function<void (std::list<request_type> const &
            , std::list<response_type> *)>;

protected:
    async_interface         _rpc;
    request_registrar_func  _register_request;
    std::list<request_type> _requests;
    std::list<response_type> _responses;
    typename std::list<response_type>::iterator _current;
    request_handler _on_request;
    bool _writing = false;

protected:
    async_bidi_streaming_method (async_bidi_streaming_method * spawner)
        : base_class(spawner->_service, spawner->_cqueue)
        , _rpc(base_class::server_context_ptr())
        , _register_request(spawner->_register_request)
        , _on_request(spawner->_on_request)
    {
        (this->_service->*_register_request)(& this->_ctx
                , & _rpc
                , this->_cqueue
                , this->_cqueue
                , this);
    }

public:
    async_bidi_streaming_method (service_type * service, server_completion_queue * cqueue)
        : base_class(service, cqueue)
        , _rpc(base_class::server_context_ptr())
    {}

    void register_method (request_registrar_func register_request
            , request_handler && on_request)
    {
        _register_request = register_request;
        _on_request = std::forward<request_handler>(on_request);
        (this->_service->* _register_request)(& this->_ctx
                , & _rpc
                , this->_cqueue
                , this->_cqueue
                , this);
    }

    virtual void process_request (bool ok) override
    {
        if (!this->_complete) {
            if (! _writing) {
                if (ok) {
                    _requests.emplace_back();
                    auto & request = _requests.back();
                    _rpc.Read(& request, this);
                } else {
                    _on_request(_requests, & _responses);
                    _current = _responses.begin();
                    _writing = true;
                    ok = true;
                }
            }

            if (_writing) {
                // SIGSEGV while call this->_ctx.IsCancelled()
                // TODO When this call is actually need?
                if (!ok || _current == _responses.end() /*|| this->_ctx.IsCancelled()*/) {
                    this->_complete = true;
                    _rpc.Finish(status_type{}, this);
                    new async_bidi_streaming_method(this);
                } else {
                    _rpc.Write(*_current, this);
                    ++_current;
                }
            }
        } else {
            delete this;
        }
    }
};

////////////////////////////////////////////////////////////////////////////////
// async_server
////////////////////////////////////////////////////////////////////////////////
template <typename ServiceClass>
class async_server
{
public:
    using service_class = ServiceClass;
    using service_type = typename service_class::AsyncService;
    using server_pointer = std::unique_ptr<server_type>;
    using server_completion_queue_pointer = std::unique_ptr<server_completion_queue>;

protected:
    server_completion_queue_pointer _cqueue;
    server_pointer _server;
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

            // TODO
            GPR_ASSERT(_cqueue->Next(& tag, & ok));

            static_cast<basic_async_method<ServiceClass> *>(tag)->process_request(ok);
        }
    }

    // Register unary method
    template <typename RequestType, typename ResponseType>
    bool register_method (typename async_unary_method<ServiceClass, RequestType, ResponseType>::request_registrar_func register_request
            ,  typename async_unary_method<ServiceClass, RequestType, ResponseType>::request_handler && on_request)
    {
        using async_method = async_unary_method<ServiceClass, RequestType, ResponseType>;

        auto m = new async_method(& _service, _cqueue.get());
        m->register_method(register_request
                , std::forward<typename async_method::request_handler>(on_request));
        return true;
    }

    // Register server-side streaming method
    template <typename RequestType, typename ResponseType>
    bool register_method (typename async_server_streaming_method<ServiceClass, RequestType, ResponseType>::request_registrar_func register_request
            ,  typename async_server_streaming_method<ServiceClass, RequestType, ResponseType>::request_handler && on_request)
    {
        using async_method = async_server_streaming_method<ServiceClass, RequestType, ResponseType>;

        auto m = new async_method(& _service, _cqueue.get());
        m->register_method(register_request
                , std::forward<typename async_method::request_handler>(on_request));
        return true;
    }

    // Register client-side streaming method
    template <typename RequestType, typename ResponseType>
    bool register_method (typename async_client_streaming_method<ServiceClass, RequestType, ResponseType>::request_registrar_func register_request
            ,  typename async_client_streaming_method<ServiceClass, RequestType, ResponseType>::request_handler && on_request)
    {
        using async_method = async_client_streaming_method<ServiceClass, RequestType, ResponseType>;

        auto m = new async_method(& _service, _cqueue.get());
        m->register_method(register_request
                , std::forward<typename async_method::request_handler>(on_request));
        return true;
    }

    // Register bidirectional streaming method
    template <typename RequestType, typename ResponseType>
    bool register_method (typename async_bidi_streaming_method<ServiceClass, RequestType, ResponseType>::request_registrar_func register_request
            ,  typename async_bidi_streaming_method<ServiceClass, RequestType, ResponseType>::request_handler && on_request)
    {
        using async_method = async_bidi_streaming_method<ServiceClass, RequestType, ResponseType>;

        auto m = new async_method(& _service, _cqueue.get());
        m->register_method(register_request
                , std::forward<typename async_method::request_handler>(on_request));
        return true;
    }

};

}} // pfs::grpc
