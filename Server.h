//
// Created by Alexey Lebed on 9/26/22.
//

#ifndef G1TANK_SERVER_H
#define G1TANK_SERVER_H

#include "iostream"
#include <vector>
#include "Websocket.h"
#include "include_boost_asio.h"
#include "CarSession.h"

using namespace std;

struct Server {
    asio::io_context ctx;
    ssl::context ssl_ctx{ssl::context::tls_server};
    shared_ptr<WebsocketManager> car_connections;
    shared_ptr<WebsocketManager> pilot_connections;
    shared_ptr<CarSessionManager> car_sessions;
    Server();

    void run();
};


#endif //G1TANK_SERVER_H
