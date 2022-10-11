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
        public EventListener<Websocket>,
        public EventListener<PilotSession>,
        public EventEmitter<CarSession>
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
        public EventEmitter<CarSessionManager>,
        public EventListener<WebsocketManager>,
        public EventListener<CarSession>
{
    asio::io_context& ctx;
    shared_ptr<PilotSessionManager> pilot_session_manager{nullptr};
    void on_event(const shared_ptr<Event<WebsocketManager>>& event) override;
    void on_event(const shared_ptr<Event<CarSession>>& event) override;
public:
    explicit CarSessionManager(asio::io_context&);
    shared_ptr<WebsocketManager>ws_connections;
    void init(const shared_ptr<PilotSessionManager>& );
    CarSession* handle_car_control_request(uuid car_id, PilotSession* candidate);
};


#endif //G1TANK_CARSESSION_H
