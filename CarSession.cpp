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

void CarSession::redirect_message_to_pilot(const nlohmann::json& j) const {
    if(pilot == nullptr){
        cerr << "Unable to find a pilot" << endl;
    } else {
        cout << "Redirecting move command to a Car" << endl;
        pilot->ws->send_message(j.dump());
    }
}

void CarSession::on_event(const shared_ptr<Event<Websocket>>& event) {
    cout << "CarSession WS event: " << event->message << endl;

    auto j = nlohmann::json::parse(event->message);
    if (j["car_id"].empty() || j["action"].empty()) return;
    if(ws_events.find(j["action"]) == ws_events.end()) return;

    if (j["action"] == "webrtc_offer") {
        on_webrtc_offer(event, j);
    }
}

void CarSession::on_webrtc_offer(const shared_ptr<Event<Websocket>>& event, nlohmann::json& j){
    redirect_message_to_pilot(j);
}

CarSessionManager::CarSessionManager(asio::io_context& ctx)
    :ws_connections(make_shared<WebsocketManager>(ctx, 8080)), ctx(ctx){}

void CarSessionManager::init(const shared_ptr<PilotSessionManager>& pilot_session_manager_) {
    ws_connections->add_event_listener(shared_from_this());
    this->pilot_session_manager = pilot_session_manager_.get();
}


void CarSessionManager::on_event(const shared_ptr<Event<WebsocketManager>>& event) {

    if(event->action == "close"){
        on_close(event);
        return;
    }

    cout << "CarSessionManager WS Event: " <<  event->message << endl;
    auto j = nlohmann::json::parse(event->message);
    if(j["car_id"].empty() || j["action"].empty()) return;
    if(ws_events.find(j["action"]) == ws_events.end()) return;

    if(j["action"] == "auth_session"){
        on_auth_session(event, j);
    }
}

void CarSessionManager::on_close(const shared_ptr<Event<WebsocketManager>>& event) {
    cout << "CarSessionManager WS close" << endl;
    auto ws = (Websocket*)event->data;
    shared_ptr<CarSession> session  {nullptr};
    for(const auto& connection: connections){
        if(connection->ws.get() == ws){
            session = connection;
        }
    }
    remove_connection(session);
}

void CarSessionManager::on_auth_session(const shared_ptr<Event<WebsocketManager>>& event, nlohmann::json& j) {
    cout << "Auth session message is received, car_id: " << j["car_id"] << endl;
    auto it = connections.begin();
    while(it != connections.end()){
        shared_ptr<CarSession> session = *it;
        if(session->car_id == str_to_uuid(j["car_id"])){
            cerr << "Car session exists" <<  endl;
            break;
        }
        it++;
    }
    if(it == connections.end()){
        try{
            cout << "Creating a new car session" <<  endl;
            auto ws = shared_ptr<Websocket>((Websocket *)event->data);
            auto session = make_shared<CarSession>(str_to_uuid(j["car_id"]), ws, ctx);
            nlohmann::json j;
            j["action"] = "auth_accept";
            session->ws->send_message(j.dump());
            session->add_event_listener(shared_from_this());
            ws->add_event_listener(session);
            connections.push_back(session);
        } catch (std::exception& e){
            cerr << "Failed to create a car session. " << e.what()<< endl;
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


