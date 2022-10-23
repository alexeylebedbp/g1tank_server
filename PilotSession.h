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

    ///Websocket events
    const std::set<string> ws_events {"get_car_control","move", "offer_request", "webrtc_answer"};
    void on_get_car_control(const shared_ptr<Event<Websocket>>& event, nlohmann::json& j);
    void on_move(const shared_ptr<Event<Websocket>>& event, nlohmann::json& j);
    void on_offer_request(const shared_ptr<Event<Websocket>>& event, nlohmann::json& j);
    void on_webrtc_answer(const shared_ptr<Event<Websocket>>& event, nlohmann::json& j);
    void redirect_message_to_car(nlohmann::json&);

    ///Car events
    const std::set<string> car_events{"close"};
    void on_car_disconnected(const shared_ptr<Event<CarSession>>& event, nlohmann::json& j);


public:
    uuid session_id;
    uuid pilot_id;
    shared_ptr<Websocket> ws;
    asio::io_context &ctx;
    shared_ptr<PilotSessionManager> manager;

    PilotSession(uuid pilot_id, Websocket* ws, asio::io_context &ctx, const shared_ptr<PilotSessionManager>& manager);
    CarSession* get_car_control(const uuid& car_id);
    void init();

    void add_car(CarSession*);
    void remove_car(CarSession*);

    void on_event(const shared_ptr<Event<CarSession>>&) override;
    void on_event(const shared_ptr<Event<Websocket>>&) override;
};

class Server;

class PilotSessionManager:
        public enable_shared_from_this<PilotSessionManager>,
        public ConnectionManager<PilotSession>,
        public EventEmitter<PilotSessionManager>,
        public EventListener<WebsocketManager>,
        public EventListener<PilotSession>

{
    CarSessionManager* car_session_manager{nullptr};
    asio::io_context& ctx;
    std::set<string> ws_event_types {"close", "auth_session"};

    void on_event(const shared_ptr<Event<WebsocketManager>>& event) override;
    void on_event(const shared_ptr<Event<PilotSession>>& event) override;
    void on_stop_signal() const;

    ///PilotSessionEvents
    const std::set<string> pilot_events{"byebye"};

    ///WebsocketManager events
    const std::set<string> ws_events {"auth_session", "close"};
    void on_auth_session(const shared_ptr<Event<WebsocketManager>>& event, nlohmann::json& j);
    void on_close(const shared_ptr<Event<WebsocketManager>>& event);


public:
    explicit PilotSessionManager(asio::io_context&);
    shared_ptr<WebsocketManager>ws_connections;
    Server* server {nullptr};
    CarSession* get_car_control(uuid car_id, PilotSession* pilot);
    void init(const shared_ptr<CarSessionManager>&);

};

#endif //G1TANK_PILOTSESSION_H
