//
// Created by alexeylebed on 9/26/22.
//

#include "WebsocketServer.h"

bool verify_ssl_certificate(bool preverified, ssl::verify_context& ctx){
    char subject_name[256];
    X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
    cout << "Verifying: " << subject_name << std::endl;
    cout << "Pre-verified: " << preverified  << endl << endl;
    return preverified;
}

Websocket::Websocket(ssl_stream &stream, asio::io_context& ctx): ws(stream), ctx(ctx) {
    cout << "Websocket 17, running read" << endl;
    co_spawn(ctx.get_executor(),
             [self = shared_from_this()]{ return self->read(); },
             [](std::exception_ptr e){
                 try {
                     if (e) {std::rethrow_exception(e);}
                 } catch(const std::exception& e) {
                     cout << "Caught exception \"" << e.what() << endl;
                 }
             }
    );
}

awaitable<void> Websocket::send(const string &message) {
     co_await ws.async_write(asio::buffer(message), use_awaitable);
}

void Websocket::on_message(const string &message) {
    cout << "WebsocketServer 34 on_message" << message << endl;
    for(auto subscriber: subscribers){
        subscriber->handle_message(message);
    }
}

void Websocket::subscribe(WebsocketMessageSubscriber* candidate) {
    auto it = find(subscribers.begin(), subscribers.end(), candidate);
    if(it != subscribers.end()){
        subscribers.push_back(candidate);
    }
}

void Websocket::unsubscribe(WebsocketMessageSubscriber* candidate) {
    auto it = find(subscribers.begin(), subscribers.end(), candidate);
    if(it != subscribers.end()){
        subscribers.erase(it);
    }
}

awaitable<void> Websocket::read() {
    for (;;) {
        co_await read_once();
    }
}


awaitable<void> Websocket::read_once() {
    beast::flat_buffer buffer;
    co_await ws.async_read(buffer, use_awaitable);
    std::cout << beast::make_printable(buffer.data()) << std::endl;
    string res = beast::buffers_to_string(buffer.data());
    on_message(res);
}


WebsocketManager::WebsocketManager(asio::io_context &ctx, ssl::context &ssl_ctx): ctx(ctx), ssl_ctx(ssl_ctx), acceptor(ctx){
}
awaitable<void> WebsocketManager::listener() {
    cout << "WebsocketManager 84,  listening new connections..." << endl;
    auto executor = co_await boost::asio::this_coro::executor;
    tcp::acceptor acceptor(executor, {tcp::v4(), 8080});
    for (;;) {
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
        auto stream = ssl_stream(socket, ssl_ctx);
        ssl_ctx.set_verify_mode(ssl::context::verify_peer | ssl::context::verify_fail_if_no_peer_cert);
        ssl_ctx.set_default_verify_paths();
        stream.set_verify_callback(verify_ssl_certificate);
        co_await stream.async_handshake(ssl::stream_base::server, use_awaitable);
        cout << "WebsocketManager 94, a new connection established" << endl;
        add_connection(new Websocket(stream, ctx));
    }

}

void WebsocketManager::run(){
    co_spawn(this->ctx.get_executor(),
             [self = shared_from_this()]{ return self->listener(); },
             [](std::exception_ptr e){
                 try {
                     if (e) {std::rethrow_exception(e);}
                 } catch(const std::exception& e) {
                     cout << "Caught exception \"" << e.what() << endl;
                 }
             }
    );
}

void WebsocketManager::add_connection(Websocket* connection) {
    connections.push_back(connection);
}

void WebsocketManager::remove_connection(Websocket* connection) {
    for(auto it = connections.begin(); it != connections.end(); it++){
        if(*it == connection){
            connections.erase(it);
        }
    }
}
