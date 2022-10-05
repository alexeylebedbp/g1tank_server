//
// Created by alexeylebed on 9/28/22.
//

#ifndef G1TANK_CARSESSION_H
#define G1TANK_CARSESSION_H
#include "Websocket.h"
#include "include_boost_asio.h"
#include "coroutine_err_handler.h"

class PilotSession;
class PilotSessionManager;

class CarSession:
        public enable_shared_from_this<CarSession>,
        public EventListener,
        public EventEmitter
{
public:
    uuid session_id;
    uuid car_id;
    shared_ptr<Websocket>ws;
    asio::io_context& ctx;
    CarSession(uuid car_id, const shared_ptr<Websocket>& ws,  asio::io_context& ctx);
    PilotSession* pilot {nullptr};
};


class CarSessionManager:
        public enable_shared_from_this<CarSessionManager>,
        public ConnectionManager<CarSession>,
        public EventEmitter,
        public EventListener
{
    asio::io_context& ctx;
    void on_event(const shared_ptr<Event>& event) override;
public:
    explicit CarSessionManager(asio::io_context&);
    shared_ptr<WebsocketManager>ws_connections;
    void init();
};


#endif //G1TANK_CARSESSION_H
