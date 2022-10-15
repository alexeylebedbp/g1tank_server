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
#include "PilotSession.h"
using namespace std;

struct Server {
    asio::io_context ctx;
    shared_ptr<CarSessionManager> car_sessions;
    shared_ptr<PilotSessionManager> pilot_sessions;
    Server();

    void run();
    void stop();
};


#endif //G1TANK_SERVER_H
