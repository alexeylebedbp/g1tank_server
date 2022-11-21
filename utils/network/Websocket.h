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
#include "uuid.h"
#include "constants.h"

using namespace std;
using ws_stream = beast::websocket::stream<tcp::socket&>;

enum class WebsocketStatus {created, connected, closed};

class Websocket: public enable_shared_from_this<Websocket>, public EventEmitter<Websocket> {
public:
    Websocket(tcp::socket socket, asio::io_context& ctx);
    WebsocketStatus status {WebsocketStatus::created};
    void send_message(const string& message);

private:
    struct PingPong {
        int last_sent{0};
        int last_received{0};
        int disconnect_timeout {DISCONNECT_TIMEOUT};
        int ping_pong_timeout {PING_PONG_TIMEOUT};
        void on_received(const string& message);
        Websocket* ws;
        explicit PingPong(Websocket*);
        boost::asio::deadline_timer latency_timer;
        boost::asio::deadline_timer timeout;
    };

    shared_ptr<PingPong> pingpong {nullptr};
    shared_ptr<ws_stream> transport {nullptr};
    asio::io_context& ctx;
    beast::flat_buffer buffer;
    tcp::socket socket;

    void on_message(const string& message);
    awaitable<void>wait_and_read();
    awaitable<void> send(const string& message);
    void read();
    friend class WebsocketManager;

};

class WebsocketManager:
        public enable_shared_from_this<WebsocketManager>,
        public ConnectionManager<Websocket>,
        public EventListener<Websocket>,
        public EventEmitter<WebsocketManager>
{
public:
    WebsocketManager(asio::io_context& ctx, int  port);
    void listen();
    bool is_running {true};
    void stop();
private:
    asio::io_context& ctx;
    awaitable<void> listener();
    const int port;
    void on_open(const shared_ptr<Websocket>& ws);
    void on_event(Event<Websocket>* event) override;
};


#endif //G1TANK_WEBSOCKET_H
