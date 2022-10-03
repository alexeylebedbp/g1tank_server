//
// Created by alexeylebed on 9/26/22.
//

#include "Websocket.h"
#include <utility>


auto exception_handler_generator (const string owner){
    return [owner](std::exception_ptr e){
        try {
            if (e) {std::rethrow_exception(e);}
        } catch(const std::exception& e) {
            std::cout << "Coro exception " << owner << e.what() << endl;
        }
    };
}

WebsocketEvent::WebsocketEvent(string message, WebsocketEventType type, const shared_ptr<Websocket>& ws)
    :message(std::move(message)), type(type) , ws(ws){}

Websocket::Websocket(tcp::socket socket_, asio::io_context& ctx): socket(std::move(socket_)), ctx(ctx){
    transport = new ws_stream(socket);
    pingpong = new PingPong(this);
}

void Websocket::on_message(const string &message) {
    cout << "ON_MESSAGE: " << message << endl;
    if(message == "__pong__") {
        pingpong->on_received(message);
        return;
    }

    for(auto subscriber: subscribers){
        auto event = WebsocketEvent(message, WebsocketEventType::message, shared_ptr<Websocket>(this));
        subscriber->handle_event(event);
    }
}

void Websocket::send_message(const string& message) {
    co_spawn(ctx.get_executor(),
             [self = shared_from_this(), message]{ return self->send(message); },
             exception_handler_generator("Websocket::send_message")
    );
    cout << "Message sent: " << message << endl;
}

void Websocket::subscribe(WebsocketEventSubscriber* candidate) {
    auto it = find(subscribers.begin(), subscribers.end(), candidate);
    if(it == subscribers.end()){
        subscribers.push_back(candidate);
    }
}

void Websocket::unsubscribe(WebsocketEventSubscriber* candidate) {
    auto it = find(subscribers.begin(), subscribers.end(), candidate);
    if(it != subscribers.end()){
        subscribers.erase(it);
    }
}

awaitable<void> Websocket::send(const string &message) {
    co_await transport->async_write(asio::buffer(message), use_awaitable);
}

void Websocket::read() {
    co_spawn(ctx.get_executor(),
             [self = shared_from_this()]{ return self->wait_and_read(); },
             [self = shared_from_this()](std::exception_ptr e){
                 try {
                     if (e) {std::rethrow_exception(e);}
                 } catch(const std::exception& e) {
                     cout << typeid(e).name() << endl;
                     if(string(e.what()) == "End of file [asio.misc:2]"){
                         self->status = WebsocketStatus::closed;
                         auto event = WebsocketEvent(string(e.what()), WebsocketEventType::close, self);
                         for(auto subscriber: self->subscribers){
                             subscriber->handle_event(event);
                         }
                     }
                     std::cout << "Coro exception " << "Websocket::read() " << e.what() << endl;
                 }
             }
    );
}

awaitable<void> Websocket::wait_and_read() {
    while(status != WebsocketStatus::closed){
        co_await transport->async_read(buffer, use_awaitable);
        std::cout << beast::make_printable(buffer.data()) << std::endl;
        string res = beast::buffers_to_string(buffer.data());
        buffer.consume(buffer.size());
        if(!res.empty()){
            on_message(res);
        }
    }
    cout << "Returning from wait and read, ws is closed" << endl;
}

Websocket::PingPong::PingPong(Websocket *ws): ws(ws), timer(ws->ctx), last_sent(0), last_received(0){}

void Websocket::PingPong::on_received(const string& message){
    last_received = ms_timestamp();
    cout << "PingPong latency, milliseconds: " << (last_received - last_sent) << endl;
    if(last_received - last_sent > 20){
        ws->send_message("red");
    }
    timer.expires_from_now( boost::posix_time::seconds(5));
    timer.async_wait([this](const boost::system::error_code&){
        ws->send_message("__ping__");
        last_sent = ms_timestamp();
    });
}

awaitable<void> WebsocketManager::listener() {
    cout << "WebsocketManager: listening new connections..." << endl;
    auto _executor = co_await boost::asio::this_coro::executor;
    tcp::acceptor _acceptor(_executor, {tcp::v4(), 8080});
    for (;;) {
        auto websocket = make_shared<Websocket>(co_await _acceptor.async_accept( use_awaitable), ctx);
        co_await websocket->transport->async_accept(use_awaitable);
        on_open(websocket);
    }
}

void WebsocketManager::on_open(shared_ptr<Websocket> websocket) {
    add_connection(websocket);
    websocket->status = WebsocketStatus::connected;
    websocket->subscribe(this);
    websocket->send_message("__ping__");
    websocket->pingpong->last_sent = ms_timestamp();
    websocket->read();
}

void WebsocketManager::listen(){
    co_spawn(ctx.get_executor(),
             [self = shared_from_this()]{ return self->listener(); },
             exception_handler_generator("WebsocketManager::listen")
    );
}

WebsocketManager::WebsocketManager(asio::io_context &ctx, ssl::context &ssl_ctx, string port):ctx(ctx), ssl_ctx(ssl_ctx), port(std::move(port)) {}

void WebsocketManager::subscribe(WebsocketEventSubscriber* candidate) {
    auto it = find(subscribers.begin(), subscribers.end(), candidate);
    if(it == subscribers.end()){
        subscribers.push_back(candidate);
    }
}

void WebsocketManager::unsubscribe(WebsocketEventSubscriber* candidate) {
    auto it = find(subscribers.begin(), subscribers.end(), candidate);
    if(it != subscribers.end()){
        subscribers.erase(it);
    }
}

void WebsocketManager::handle_event(WebsocketEvent& event) {
    if(event.type == WebsocketEventType::close){
        cout << "WSManager unsubscribe" << endl;
        event.ws->unsubscribe(this);
    }
    for(auto subscriber: subscribers){
        subscriber->handle_event(event);
    }
}


