//
// Created by alexeylebed on 10/3/22.
//

#ifndef G1TANK_PILOTSESSION_H
#define G1TANK_PILOTSESSION_H

#include "Websocket.h"
#include "include_boost_asio.h"
#include "coroutine_err_handler.h"
#include "constants.h"

class CarSession;
class CarSessionManager;
class PilotSessionManager;

class PilotSession:
        public enable_shared_from_this<PilotSession>,
        public EventListener<Websocket>,
        public EventListener<CarSession>,
        public EventEmitter<PilotSession>
{
    CarSession* car {nullptr};

    ///Websocket events
    const std::set<string> ws_events {GET_CAR_CONTROL,MOVE, OFFER_REQUEST, WEBRTC_ANSWER};
    void on_get_car_control(Event<Websocket>* event, nlohmann::json& j);
    void on_move(Event<Websocket>* event, nlohmann::json& j);
    void on_offer_request(Event<Websocket>* event, nlohmann::json& j);
    void on_webrtc_answer(Event<Websocket>* event, nlohmann::json& j);
    void on_byebye(Event<Websocket>* event, nlohmann::json& j);
    void redirect_message_to_car(nlohmann::json&);

    ///Car events
    const std::set<string> car_events{"close"};
    void on_car_disconnected(Event<CarSession>* event, nlohmann::json& j);


public:
    uuid session_id;
    uuid pilot_id;
    Websocket* ws;
    asio::io_context &ctx;
    PilotSessionManager* manager;

    PilotSession(uuid pilot_id, Websocket* ws, asio::io_context &ctx, const shared_ptr<PilotSessionManager>& manager);
    CarSession* get_car_control(const uuid& car_id);
    void init();

    void add_car(CarSession*);
    void remove_car();

    void on_event(Event<CarSession>*) override;
    void on_event(Event<Websocket>*) override;

    ~PilotSession(){
        cout << "Destroying PilotSession" << endl;
    }
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
    std::set<string> ws_event_types {CLOSE, AUTH_SESSION};

    void on_event(Event<WebsocketManager>* event) override;
    void on_event(Event<PilotSession>* event) override;
    void on_stop_signal() const;

    ///PilotSessionEvents
    const std::set<string> pilot_events{BYEBYE};

    ///WebsocketManager events
    const std::set<string> ws_events {AUTH_SESSION, CLOSE, BYEBYE};
    void on_auth_session(Event<WebsocketManager>* event, nlohmann::json& j);
    void on_close(Event<WebsocketManager>* event);


public:
    explicit PilotSessionManager(asio::io_context&);
    shared_ptr<WebsocketManager>ws_connections;
    Server* server {nullptr};
    CarSession* get_car_control(uuid car_id, PilotSession* pilot);
    void init(const shared_ptr<CarSessionManager>&, Server*);
    void stop();

};

#endif //G1TANK_PILOTSESSION_H
