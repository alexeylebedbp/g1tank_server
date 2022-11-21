//
// Created by alexeylebed on 9/26/22.
//

#include "Websocket.h"
#include <utility>


Websocket::Websocket(tcp::socket socket_, asio::io_context& ctx): socket(std::move(socket_)), ctx(ctx){
    transport = make_shared<ws_stream>(socket);
    pingpong =  make_shared<PingPong>(this);
}

void Websocket::on_message(const string &message) {
    if(message == PONG) {
        pingpong->on_received(message);
        return;
    }
    emit_event(MESSAGE, message, (void*) this);
}

void Websocket::send_message(const string& message) {
    if(status == WebsocketStatus::connected){
        try {
            transport->write(asio::buffer(message));
        } catch (std::exception& e){
            std::cerr << "Websocket::send_message() " << e.what() << endl;
            this->status = WebsocketStatus::closed;
            this->emit_event(CLOSE);
        }
    }
}

awaitable<void> Websocket::send(const string &message) {
    //TODO: currently send_message is a sync function because it's not a good idea to write more them one messages at a time. So queue needed.
    co_await transport->async_write(asio::buffer(message), use_awaitable);
}

void Websocket::read() {
    co_spawn(ctx.get_executor(),
             [self = shared_from_this()]{ return self->wait_and_read(); },
             [self = shared_from_this()](std::exception_ptr e){
                 try {
                     if (e) {std::rethrow_exception(e);}
                 } catch(const std::exception& e) {
                     self->status = WebsocketStatus::closed;
                     self->emit_event(CLOSE);
                     std::cerr << "Coro exception " << "Websocket::read() " << e.what() << endl;
                 }
             }
    );
}

awaitable<void> Websocket::wait_and_read() {
    while(status == WebsocketStatus::connected){
        co_await transport->async_read(buffer, use_awaitable);
        std::cout << beast::make_printable(buffer.data()) << std::endl;
        string res = beast::buffers_to_string(buffer.data());
        buffer.consume(buffer.size());
        if(!res.empty()){
            on_message(res);
        }
    }
}

Websocket::PingPong::PingPong(Websocket *ws)
    :ws(ws), latency_timer(ws->ctx), timeout(ws->ctx), last_sent(0), last_received(0){}



void Websocket::PingPong::on_received(const string& message){
    timeout.cancel();
    last_received = ms_timestamp();
    cout << "PingPong latency, milliseconds: " << (last_received - last_sent) << endl;

    if(last_received - last_sent > 200){
        ws->send_message(POOR_NETWORK_DETECTED);
    }

    latency_timer.expires_from_now( boost::posix_time::seconds(ping_pong_timeout));
    latency_timer.async_wait([this](const boost::system::error_code&){
        ws->send_message(PING);
        last_sent = ms_timestamp();
    });

    timeout.expires_from_now(boost::posix_time::seconds(disconnect_timeout));
    timeout.async_wait([this](const boost::system::error_code& e){
        if(e.value() == 0){
            cout <<"No response from WS client, disconnecting..." << endl;
            ws->status = WebsocketStatus::closed;
            ws->socket.close();
            ws->emit_event(CLOSE);
        }
    });
}


awaitable<void> WebsocketManager::listener() {
    cout << "WebsocketManager: listening new connections on PORT: " << port << endl;
    auto _executor = co_await boost::asio::this_coro::executor;
    tcp::acceptor _acceptor(_executor, {tcp::v4(), asio::ip::port_type(port)});
    while(is_running) {
        auto websocket = make_shared<Websocket>(co_await _acceptor.async_accept( use_awaitable), ctx);
        try {
            co_await websocket->transport->async_accept(use_awaitable);
            on_open(websocket);
        } catch (std::exception& e){
            cerr << "WebsocketManager::listener() " << e.what() << endl;
        }
    }
    cout << "WebsocketManager: STOP listening  on PORT: " << port << endl;
}

void WebsocketManager::on_open(const shared_ptr<Websocket>& websocket) {
    add_connection(websocket);
    websocket->status = WebsocketStatus::connected;
    websocket->add_event_listener(this);
    websocket->send_message(PING);
    websocket->pingpong->last_sent = ms_timestamp();
    websocket->read();
}

void WebsocketManager::listen(){
    co_spawn(ctx.get_executor(),
             [self = shared_from_this()]{ return self->listener(); },
             exception_handler_generator("WebsocketManager::listen")
    );
}

void WebsocketManager::stop(){
    for(auto& connection: connections){
        connection->remove_event_listener(this);
    }
    connections.clear();
}

WebsocketManager::WebsocketManager(asio::io_context &ctx, int port):ctx(ctx), port(std::move(port)) {}


void WebsocketManager::on_event(Event<Websocket>* event) {
    if (event->action == CLOSE) {
        cout << "WebsocketManager unsubscribe on WS CLOSE" << endl;
        event->emitter->remove_event_listener(this);
    }
    emit_event(event->action, event->message, (void*)event->emitter);
}

