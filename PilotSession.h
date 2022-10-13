//
// Created by alexeylebed on 10/3/22.
//

#ifndef G1TANK_PILOTSESSION_H
#define G1TANK_PILOTSESSION_H

#include "Websocket.h"
#include "include_boost_asio.h"
#include "coroutine_err_handler.h"

class CarSession;
class CarSessionManager;
class PilotSessionManager;

class PilotSession:
        public enable_shared_from_this<PilotSession>,
        public EventListener<Websocket>,
        public EventListener<CarSession>,
        public EventEmitter<PilotSession>
{
    CarSession* car{nullptr};
public:
    uuid session_id;
    uuid pilot_id;
    shared_ptr<Websocket> ws;
    asio::io_context &ctx;
    shared_ptr<PilotSessionManager> manager;

    PilotSession(uuid pilot_id, Websocket* ws, asio::io_context &ctx, const shared_ptr<PilotSessionManager>& manager);
    CarSession* get_car_control(const uuid& car_id);
    void init();

    CarSession* get_car();
    void add_car(CarSession*);
    void remove_car(CarSession*);

    void on_event(const shared_ptr<Event<CarSession>>&) override;
};

class PilotSessionManager:
        public enable_shared_from_this<PilotSessionManager>,
        public ConnectionManager<PilotSession>,
        public EventEmitter<PilotSessionManager>,
        public EventListener<WebsocketManager>,
        public EventListener<PilotSession>

{
    shared_ptr<CarSessionManager> car_session_manager{nullptr};
    asio::io_context& ctx;
    void on_event(const shared_ptr<Event<WebsocketManager>>& event) override;
    void on_event(const shared_ptr<Event<PilotSession>>& event) override;
public:
    explicit PilotSessionManager(asio::io_context&);
    shared_ptr<WebsocketManager>ws_connections;
    CarSession* get_car_control(uuid car_id, PilotSession* pilot);
    void init(const shared_ptr<CarSessionManager>&);

};

#endif //G1TANK_PILOTSESSION_H
