//
// Created by alexeylebed on 9/26/22.
//

#ifndef G1TANK_WEBSOCKET_H
#define G1TANK_WEBSOCKET_H


#include "iostream"
#include <vector>
#include <set>
#include "include_boost_asio.h"
#include "ConnectionManager.h"
#include "timestamp.h"
#include "nlohmann/json.hpp"
#include "coroutine_err_handler.h"
#include "Event.h"

using namespace std;
using ws_stream = beast::websocket::stream<tcp::socket&>;

enum class WebsocketStatus {created, connected, closed};


class Websocket: public enable_shared_from_this<Websocket>, public EventEmitter {
public:
    Websocket(tcp::socket socket, asio::io_context& ctx);
    WebsocketStatus status {WebsocketStatus::created};
    void send_message(const string& message);

    ~Websocket(){
        cout << "WS destructor is called" << endl;
    }

private:
    struct PingPong {
        int last_sent;
        int last_received;
        void on_received(const string& message);
        Websocket* ws;
        explicit PingPong(Websocket*);
        boost::asio::deadline_timer timer;
    };

    PingPong* pingpong {nullptr};
    asio::io_context& ctx;
    beast::flat_buffer buffer;
    tcp::socket socket;
    ws_stream* transport {nullptr};

    void on_message(const string& message);
    awaitable<void>wait_and_read();
    awaitable<void> send(const string& message);
    void read();

    friend class WebsocketManager;

};

class WebsocketManager:
        public enable_shared_from_this<WebsocketManager>,
        public ConnectionManager<Websocket>,
        public EventListener,
        public EventEmitter
{
public:
    WebsocketManager(asio::io_context& ctx, string  port);
    void listen();
private:
    asio::io_context& ctx;
    awaitable<void> listener();
    const string port;
    void on_open(const shared_ptr<Websocket>& ws);
    void on_event(const shared_ptr<Event>& event) override;
};


#endif //G1TANK_WEBSOCKET_H
