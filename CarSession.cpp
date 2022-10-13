//
// Created by alexeylebed on 9/28/22.
//
#include "CarSession.h"
#include "boost/asio/read.hpp"
#include "PilotSession.h"

CarSession::CarSession(uuid car_id, const shared_ptr<Websocket>& ws,  asio::io_context& ctx)
    :car_id(car_id),
     ctx(ctx),
     session_id(boost::uuids::random_generator()()),
     ws(ws){}

CarSessionManager::CarSessionManager(asio::io_context& ctx)
    :ws_connections(make_shared<WebsocketManager>(ctx, 8080)), ctx(ctx){}

void CarSessionManager::init(const shared_ptr<PilotSessionManager>& pilot_session_manager) {
    ws_connections->add_event_listener(shared_from_this());
    this->pilot_session_manager = pilot_session_manager;
}

void CarSessionManager::on_event(const shared_ptr<Event<WebsocketManager>>& event) {
    if(event->action == "close"){
        cout << "CarSessionManager WS close" << endl;
        auto ws = (Websocket*)event->data;
        shared_ptr<CarSession> session  {nullptr};
        for(const auto& connection: connections){
            if(connection->ws.get() == ws){
                session = connection;
            }
        }
        session->emit_event("close");
        remove_connection(session);
        return;
    }
    auto j = nlohmann::json::parse(event->message);
    if(!j["action"].empty() && j["action"] == "auth_session" && !j["car_id"].empty()){
        cout << "Auth session message is received, car_id: " << j["car_id"] << endl;
        auto it = connections.begin();
        while(it != connections.end()){
            shared_ptr<CarSession> session = *it;
            if(session->car_id == str_to_uuid(j["car_id"])){
                cerr << "CarSession exists" <<  endl;
                break;
            }
            it++;
        }
        if(it == connections.end()){
            cout << "Creating a new CarSession" <<  endl;
            auto ws = shared_ptr<Websocket>((Websocket *)event->data);
            auto session = make_shared<CarSession>(str_to_uuid(j["car_id"]), ws, ctx);
            session->add_event_listener(shared_from_this());
            ws->add_event_listener(session);
            connections.push_back(session);
        }
    }
}

void CarSessionManager::on_event(const shared_ptr<Event<CarSession>> &event) {

}

CarSession *CarSessionManager::handle_car_control_request(uuid car_id, PilotSession* candidate) {
    CarSession* result {nullptr};
    for(const auto& session: connections){
        if(session->pilot == nullptr && session->car_id == car_id){
            result = session.get();
            session->pilot = candidate;
            session->add_event_listener(shared_ptr<PilotSession>(candidate));
            candidate->add_event_listener(session);
        }
    }
    return result;
}


