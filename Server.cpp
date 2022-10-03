//
// Created by Alexey Lebed on 9/26/22.
//

#include "Server.h"

Server::Server()
    :car_connections(make_shared<WebsocketManager>(ctx, ssl_ctx, "8081")),
     pilot_connections(make_shared<WebsocketManager>(ctx, ssl_ctx, "8080")),
     car_sessions(make_shared<CarSessionManager>(car_connections))
 {
    car_connections->subscribe(car_sessions.get());
 }

void Server::run() {
    boost::asio::signal_set signals(ctx, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto){ctx.stop();});
    car_connections->listen();
    ctx.run();
};
