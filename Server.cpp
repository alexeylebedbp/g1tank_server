//
// Created by Alexey Lebed on 9/26/22.
//

#include "Server.h"

Server::Server()
    :car_sessions(make_shared<CarSessionManager>(ctx)),
     pilot_sessions(make_shared<PilotSessionManager>(ctx)){}

void Server::run() {
    boost::asio::signal_set signals(ctx, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto){ctx.stop();});
    car_sessions->init();
    pilot_sessions->init();
    car_sessions->ws_connections->listen();
    ctx.run();
};
