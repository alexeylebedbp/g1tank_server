//
// Created by alexeylebed on 9/26/22.
//

#ifndef G1TANK_WEBSOCKETSERVER_H
#define G1TANK_WEBSOCKETSERVER_H


#include "iostream"
#include <vector>
#include <set>
#include "include_boost_asio.h"
#include "ConnectionManager.h"

using namespace std;
namespace websocket = beast::websocket;
using ssl_stream = ssl::stream<tcp::socket&>;
using ws_ssl_stream = beast::websocket::stream<ssl_stream&>;

class WebsocketMessageSubscriber {
public:
    WebsocketMessageSubscriber() = default;
    virtual void handle_message(const string& message){};
};


class Websocket: public std::enable_shared_from_this<Websocket> {
    ws_ssl_stream ws;
    asio::io_context& ctx;
    vector<WebsocketMessageSubscriber*> subscribers{};
public:
    Websocket(ssl_stream& stream, asio::io_context& ctx);
    awaitable<void> send(const string& message);

    void on_message(const string& message);
    void subscribe(WebsocketMessageSubscriber*);
    void unsubscribe(WebsocketMessageSubscriber*);

private:
    awaitable<void>read_once();
    awaitable<void> read();


};


class WebsocketManager: public std::enable_shared_from_this<WebsocketManager>, public ConnectionManager<Websocket>{
    asio::io_context& ctx;
    ssl::context& ssl_ctx;
    awaitable<void> listener();
    tcp::acceptor acceptor;

public:
    WebsocketManager(asio::io_context& ctx, ssl::context& ssl_ctx);
    void run();
    void add_connection(Websocket* connection) override;
    void remove_connection(Websocket* connection) override;

};

#endif //G1TANK_WEBSOCKETSERVER_H
