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
    ///Websocket events
    const std::set<string> ws_events {WEBRTC_OFFER};
    void on_webrtc_offer(Event<Websocket>* event, nlohmann::json& j);

    void redirect_message_to_pilot(const nlohmann::json& j) const;

public:
    uuid session_id;
    uuid car_id;
    Websocket* ws;
    asio::io_context& ctx;
    CarSession(uuid car_id, Websocket* ws,  asio::io_context& ctx);
    PilotSession* pilot {nullptr};
    void on_event(Event<Websocket>*) override;
    void remove_pilot();

    ~CarSession(){
        cout << "Destroying CarSession" << endl;
    }
};


class CarSessionManager:
        public enable_shared_from_this<CarSessionManager>,
        public ConnectionManager<CarSession>,
        public EventEmitter<CarSessionManager>,
        public EventListener<WebsocketManager>,
        public EventListener<CarSession>
{
    asio::io_context& ctx;
    void on_event(Event<WebsocketManager>* event) override;
    void on_event(Event<CarSession>* event) override;

    ///WebsocketManager events
    const std::set<string> ws_events {AUTH_SESSION, CLOSE};
    void on_auth_session(Event<WebsocketManager>* event, nlohmann::json& j);
    void on_close(Event<WebsocketManager>* event);

public:
    explicit CarSessionManager(asio::io_context&);
    shared_ptr<WebsocketManager>ws_connections;
    void init();
    void stop();
    CarSession* handle_car_control_request(uuid car_id, PilotSession* candidate);
};


#endif //G1TANK_CARSESSION_H
