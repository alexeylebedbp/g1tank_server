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

using namespace std;
using ws_stream = beast::websocket::stream<tcp::socket&>;

enum class WebsocketEventType {message, error, close};
enum class WebsocketStatus {created, connected, closed};

class Websocket;

struct WebsocketEvent {
    WebsocketEventType type;
    string message;
    shared_ptr<Websocket> ws;
    WebsocketEvent(string message, WebsocketEventType type, const shared_ptr<Websocket>& ws);
};

class WebsocketEventSubscriber {
public:
    WebsocketEventSubscriber() = default;
    virtual void handle_event(WebsocketEvent& event){};
};


class Websocket: public std::enable_shared_from_this<Websocket> {
public:
    Websocket(tcp::socket socket, asio::io_context& ctx);
    WebsocketStatus status {WebsocketStatus::created};
    void send_message(const string& message);
    void subscribe(WebsocketEventSubscriber*);
    void unsubscribe(WebsocketEventSubscriber*);

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
    vector<WebsocketEventSubscriber*> subscribers{};

    void on_message(const string& message);
    awaitable<void>wait_and_read();
    awaitable<void> send(const string& message);
    void read();

    friend class WebsocketManager;

};


class WebsocketManager: public std::enable_shared_from_this<WebsocketManager>, public ConnectionManager<Websocket>, WebsocketEventSubscriber{
public:
    WebsocketManager(asio::io_context& ctx, ssl::context& ssl_ctx, string  port);
    void listen();
    void subscribe(WebsocketEventSubscriber*);
    void unsubscribe(WebsocketEventSubscriber*);
private:
    asio::io_context& ctx;
    ssl::context& ssl_ctx;
    awaitable<void> listener();
    const string port;
    vector<WebsocketEventSubscriber*> subscribers{};
    void on_open(shared_ptr<Websocket> ws);
    void handle_event(WebsocketEvent& event) override;

};

#endif //G1TANK_WEBSOCKET_H
