////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-grpc](https://github.com/semenovf/pfs-grpc) library.
//
// Changelog:
//      2019.10.10 Initial version
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <grpcpp/grpcpp.h>
#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/async_unary_call.h>
#include <functional>
#include <memory>
#include <string>
#include <list>

namespace pfs {
namespace grpc {

using grpc_status = ::grpc::Status;
using grpc_client_context = ::grpc::ClientContext;
using grpc_completion_queue = ::grpc::CompletionQueue;

////////////////////////////////////////////////////////////////////////////////
// REFERENCES:
//
// 1.[gRPC Basics - C++](https://grpc.io/docs/tutorials/basic/cpp/)
// 2.[helloworld/greeter_async_client.cc](https://github.com/grpc/grpc/blob/master/examples/cpp/helloworld/greeter_async_client.cc)
// 3.[gRPC environment variables](https://github.com/grpc/grpc/blob/master/doc/environment_variables.md)
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// basic_async
////////////////////////////////////////////////////////////////////////////////
class basic_async
{
public:
    using error_handler = std::function<void (grpc_status const &)>;

protected:
    bool _complete = false; // is request complete
    grpc_status _status;
    grpc_client_context _context;
    error_handler _on_error = [] (grpc_status const & status) {
        std::cerr << "ERROR: RPC request failed with code "
                << status.error_code()
                << ": " << status.error_message() << "\n";
    };

public:
    basic_async () {}
    virtual ~basic_async () {}

    grpc_client_context & context ()
    {
        return _context;
    }

    virtual void process_response (bool ok) = 0;
};

////////////////////////////////////////////////////////////////////////////////
// async_unary
//==============================================================================
// 'Simple RPC' ('Unary RPC', one-to-one call) where the client sends a request to the server
// using the stub and waits for a response to come back, just like a normal
// function call.
////////////////////////////////////////////////////////////////////////////////
template <typename ResponseType>
class async_unary : public basic_async
{
public:
    using response_type = ResponseType;
    using async_interface = ::grpc::ClientAsyncResponseReader<response_type>;
    using rpc_type = std::unique_ptr<async_interface>;
    using response_handler = std::function<void (response_type const &)>;

protected:
    rpc_type         _rpc;
    response_type    _response;
    response_handler _on_response;

public:
    // GCC 4.7.2 failed with error:
    // error: cannot allocate an object of abstract type ‘pfs::grpc::basic_async’
    // note:   because the following virtual functions are pure within ‘pfs::grpc::basic_async’:
    // note:     virtual void pfs::grpc::basic_async::process_response(bool)
    //                              |
    //                              v
    // async_unary () : basic_async{} {}
    async_unary () : basic_async() {}

    virtual ~async_unary () {}

    void prepare_async (rpc_type && rpc, response_handler && on_response)
    {
        _on_response = std::forward<response_handler>(on_response);
        _rpc = std::forward<rpc_type>(rpc);
        _rpc->StartCall();
        _rpc->Finish(& _response, & _status, this);
    }

    virtual void process_response (bool ok) override
    {
        GPR_ASSERT(ok);

        if (_status.ok()) {
            _on_response(_response);
        } else {
            _on_error(_status);
        }

        delete this;
    }
};

////////////////////////////////////////////////////////////////////////////////
// async_server_streaming
//==============================================================================
// 'Server-side streaming RPC' (one-to-many call) where the client sends
// a request to the server and gets a stream to read a sequence of messages
// back. The client reads from the returned stream until there are no more
// messages. As you can see in our example, you specify a server-side streaming
// method by placing the stream keyword before the response type.
//
////////////////////////////////////////////////////////////////////////////////
template <typename ResponseType>
class async_server_streaming : public basic_async
{
public:
    using response_type = ResponseType;
    using async_interface = ::grpc::ClientAsyncReader<response_type>;
    using rpc_type = std::unique_ptr<async_interface>;
    using response_handler = std::function<void (std::list<response_type> const &)>;

protected:
    rpc_type                 _rpc;
    std::list<response_type> _responses;
    response_handler         _on_response;

public:
    async_server_streaming () : basic_async() {}
    virtual ~async_server_streaming () {}

    void prepare_async (rpc_type && rpc, response_handler && on_response)
    {
        _on_response = std::forward<response_handler>(on_response);
        _rpc = std::forward<rpc_type>(rpc);
        _rpc->StartCall(this);
    }

    virtual void process_response (bool ok) override
    {
        if (!_complete) {
            if (!ok) {
                _rpc->Finish(& _status, this);
                _complete = true;

                if (_status.ok()) {
                    _on_response(_responses);
                } else {
                    _on_error(_status);
                }
            } else {
                _responses.emplace_back();
                auto & response = _responses.back();
                _rpc->Read(& response, this);
            }
        } else {
            delete this;
        }
    }
};

////////////////////////////////////////////////////////////////////////////////
// async_server_pushing
//==============================================================================
// Modification of 'Server-side streaming RPC'.
// Activating responses from server
//
////////////////////////////////////////////////////////////////////////////////
template <typename ResponseType>
class async_server_pushing : public basic_async
{
public:
    using response_type = ResponseType;
    using async_interface = ::grpc::ClientAsyncReader<response_type>;
    using rpc_type = std::unique_ptr<async_interface>;
    using response_handler = std::function<void (response_type const &)>;

protected:
    rpc_type         _rpc;
    response_type    _response;
    response_handler _on_response;

public:
    async_server_pushing () : basic_async() {}
    virtual ~async_server_pushing () {}

    void prepare_async (rpc_type && rpc, response_handler && on_response)
    {
        _on_response = std::forward<response_handler>(on_response);
        _rpc = std::forward<rpc_type>(rpc);
        _rpc->StartCall(this);
    }

    virtual void process_response (bool ok) override
    {
        if (!_complete) {
            if (!ok) {
                _rpc->Finish(& _status, this);
                _complete = true;

                if (!_status.ok()) {
                    _on_error(_status);
                }
            } else {
                _rpc->Read(& _response, this);
                _on_response(_response);
            }
        } else {
            delete this;
        }
    }
};

////////////////////////////////////////////////////////////////////////////////
// async_client_streaming
//==============================================================================
// 'Client-side streaming RPC' (many-to-one call) where the client writes
// a sequence of messages and sends them to the server, again using a provided
// stream. Once the client has finished writing the messages, it waits for the
// server to read them all and return its response. You specify a client-side
// streaming method by placing the stream keyword before the request type.
//
////////////////////////////////////////////////////////////////////////////////
template <typename RequestType, typename ResponseType>
class async_client_streaming : public basic_async
{
public:
    using request_type = RequestType;
    using response_type = ResponseType;
    using async_interface = ::grpc::ClientAsyncWriter<request_type>;
    using rpc_type = std::unique_ptr<async_interface>;
    using response_handler = std::function<void (response_type const &)>;

protected:
    bool             _writing = true;
    rpc_type         _rpc;
    response_type    _response;
    response_handler _on_response;
    std::list<request_type> _requests;
    typename std::list<request_type>::iterator _current;

public:
    async_client_streaming (std::list<request_type> && requests)
        : basic_async()
        , _requests{std::forward<std::list<request_type>>(requests)}
    {}

    virtual ~async_client_streaming () {}

    response_type & response ()
    {
        return _response;
    }

    void prepare_async (rpc_type && rpc, response_handler && on_response)
    {
        _on_response = std::forward<response_handler>(on_response);
        _rpc = std::forward<rpc_type>(rpc);
        _rpc->StartCall(this);
        _current = _requests.begin();
    }

    virtual void process_response (bool ok) override
    {
        if (!_complete) {
            if (_writing) {
                if (_current != _requests.end()) {
                    _rpc->Write(*_current, this);
                    ++_current;
                } else {
                    _rpc->WritesDone(this);
                    _writing = false;
                }
            } else {
                _rpc->Finish(& _status, this);
                _complete = true;
            }
        } else {
            if (_status.ok()) {
                _on_response(_response);
            } else {
                _on_error(_status);
            }

            delete this;
        }
    }
};

////////////////////////////////////////////////////////////////////////////////
// 'Bidirectional streaming RPC' (many-to-many call) where both sides send
// a sequence of messages using a read-write stream. The two streams operate
// independently, so clients and servers can read and write in whatever order
// they like: for example, the server could wait to receive all the client
// messages before writing its responses, or it could alternately read a
// message then write a message, or some other combination of reads and writes.
// The order of messages in each stream is preserved. You specify this type of
// method by placing the stream keyword before both the request and the response.
//
////////////////////////////////////////////////////////////////////////////////
template <typename RequestType, typename ResponseType>
class async_bidi_streaming : public basic_async
{
public:
    using request_type = RequestType;
    using response_type = ResponseType;
    using async_interface = ::grpc::ClientAsyncReaderWriter<request_type, response_type>;
    using rpc_type = std::unique_ptr<async_interface>;
    using response_handler = std::function<void (std::list<response_type> const &)>;

protected:
    bool             _writing = true;
    rpc_type         _rpc;
    response_handler _on_response;
    std::list<response_type> _responses;
    std::list<request_type> _requests;
    typename std::list<request_type>::iterator _current;

public:
    async_bidi_streaming (std::list<request_type> && requests)
        : basic_async()
        , _requests{std::forward<std::list<request_type>>(requests)}
    {}

    virtual ~async_bidi_streaming () {}

    void prepare_async (rpc_type && rpc, response_handler && on_response)
    {
        _on_response = std::forward<response_handler>(on_response);
        _rpc = std::forward<rpc_type>(rpc);
        _rpc->StartCall(this);
        _current = _requests.begin();
    }

    virtual void process_response (bool ok) override
    {
        if(!_complete) {
            if (_writing) {
                if (_current != _requests.end()) {
                    _rpc->Write(*_current, this);
                    ++_current;
                } else {
                    _rpc->WritesDone(this);
                    _writing = false;
                }
            } else {
                if (!ok) {
                    _rpc->Finish(& _status, this);
                    _complete = true;

                    if (_status.ok()) {
                        _on_response(_responses);
                    } else {
                        _on_error(_status);
                    }
                } else {
                    _responses.emplace_back();
                    auto & response = _responses.back();
                    _rpc->Read(& response, this);
                }
            }
        } else {
            delete this;
        }
    }
};

////////////////////////////////////////////////////////////////////////////////
// async_client
////////////////////////////////////////////////////////////////////////////////
template <typename ServiceType>
class async_client
{
public:
    using grpc_credentials_pointer = std::shared_ptr< ::grpc::ChannelCredentials>;

protected:
    using stub_type = typename ServiceType::Stub;

    std::unique_ptr<stub_type> _stub;
    grpc_completion_queue _cqueue;

public:
//     explicit async_client (std::unique_ptr<stub_type> && stub)
//         : _stub(std::forward<std::unique_ptr<stub_type>>(stub))
//     {}

    async_client (std::string const & server_addr
            , grpc_credentials_pointer const & creds = ::grpc::InsecureChannelCredentials())
        : _stub(ServiceType::NewStub(::grpc::CreateChannel(server_addr, creds)))
    {}

    template <typename RequestType, typename ResponseType>
    bool call (RequestType const & request
            , typename async_unary<ResponseType>::rpc_type (stub_type::* prepareAsyncReader) (
                      grpc_client_context *
                    , RequestType const &
                    , grpc_completion_queue *)
            , std::function<void (ResponseType const &)> && on_response)
    {
        // GCC 4.7.2 failed with error:
        //      no type named response_handler in ‘using async_unary ...
        //using async_type = async_unary<ResponseType>;
        typedef async_unary<ResponseType> async_type;

        auto c = new async_type;
        auto rpc((_stub.get()->* prepareAsyncReader)(
                  & c->context()
                , request
                , & _cqueue));
        c->prepare_async(std::move(rpc)
                , std::forward<typename async_type::response_handler>(on_response));

        return true;
    }

    template <typename RequestType, typename ResponseType>
    bool call (RequestType const & request
            , typename async_server_streaming<ResponseType>::rpc_type (stub_type::* prepareAsyncReader) (
                      grpc_client_context *
                    , RequestType const &
                    , grpc_completion_queue *)
            , std::function<void (std::list<ResponseType> const &)> && on_response)
    {
        //using async_type = async_server_streaming<ResponseType>;
        typedef async_server_streaming<ResponseType> async_type;

        auto c = new async_type;
        auto rpc((_stub.get()->* prepareAsyncReader)(
                  & c->context()
                , request
                , & _cqueue));
        c->prepare_async(std::move(rpc)
                , std::forward<typename async_type::response_handler>(on_response));

        return true;
    }

    template <typename RequestType, typename ResponseType>
    bool call (std::list<RequestType> && requests
            , typename async_client_streaming<RequestType,ResponseType>::rpc_type (stub_type::* prepareAsyncWriter) (
                      grpc_client_context *
                    , ResponseType *
                    , grpc_completion_queue *)
            , std::function<void (ResponseType const &)> && on_response)
    {
        //using async_type = async_client_streaming<RequestType, ResponseType>;
        typedef async_client_streaming<RequestType, ResponseType> async_type;

        auto c = new async_type(std::forward<std::list<RequestType>>(requests));
        auto rpc((_stub.get()->* prepareAsyncWriter)(
                  & c->context()
                , & c->response()
                , & _cqueue));
        c->prepare_async(std::move(rpc)
                , std::forward<typename async_type::response_handler>(on_response));

        return true;
    }

    template <typename RequestType, typename ResponseType>
    bool call (std::list<RequestType> && requests
            , typename async_bidi_streaming<RequestType,ResponseType>::rpc_type (stub_type::* prepareAsyncReaderWriter) (
                      grpc_client_context *
                    , grpc_completion_queue *)
            , std::function<void (std::list<ResponseType> const &)> && on_response)
    {
        //using async_type = async_bidi_streaming<RequestType, ResponseType>;
        typedef async_bidi_streaming<RequestType, ResponseType> async_type;

        auto c = new async_type(std::forward<std::list<RequestType>>(requests));
        auto rpc((_stub.get()->* prepareAsyncReaderWriter)(
                  & c->context()
                , & _cqueue));
        c->prepare_async(std::move(rpc)
                , std::forward<typename async_type::response_handler>(on_response));

        return true;
    }

    template <typename RequestType, typename ResponseType>
    bool enable_push (RequestType const & request
            , typename async_server_streaming<ResponseType>::rpc_type (stub_type::* prepareAsyncReader) (
                      grpc_client_context *
                    , RequestType const &
                    , grpc_completion_queue *)
            , std::function<void (ResponseType const &)> && on_response)
    {
        //using async_type = async_server_pushing<ResponseType>;
        typedef async_server_pushing<ResponseType> async_type;

        auto c = new async_type;
        auto rpc((_stub.get()->* prepareAsyncReader)(
                  & c->context()
                , request
                , & _cqueue));
        c->prepare_async(std::move(rpc)
                , std::forward<typename async_type::response_handler>(on_response));

        return true;
    }

    void process ()
    {
        void * tag;
        bool ok = false;

        // Block until the next result is available in the completion queue "cq".
        // The return value of Next should always be checked. This return value
        // tells us whether there is any kind of event or the cq_ is shutting down.
        while (_cqueue.Next(& tag, & ok)) {
            auto response = static_cast<basic_async *>(tag);
            response->process_response(ok);
        }
    }
};

}} // pfs::grpc
