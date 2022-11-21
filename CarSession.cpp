//
// Created by alexeylebed on 9/28/22.
//
#include "CarSession.h"
#include "boost/asio/read.hpp"
#include "PilotSession.h"

CarSession::CarSession(uuid car_id, Websocket* ws,  asio::io_context& ctx)
    :car_id(car_id),
     ctx(ctx),
     session_id(boost::uuids::random_generator()()),
     ws(ws){}

void CarSession::redirect_message_to_pilot(const nlohmann::json& j) const {
    if(pilot == nullptr){
        cerr << "Unable to find a pilot" << endl;
    } else {
        cout << "Redirecting message " << j[ACTION] << " to the Pilot" << endl;
        pilot->ws->send_message(j.dump());
    }
}

void CarSession::on_event(Event<Websocket>* event) {
    cout << "CarSession WS event: " << event->action << " " << event->message << endl;
    if(event->action == CLOSE || event->message.empty()){
        return;
    }

    auto j = nlohmann::json::parse(event->message);
    if (j[CAR_ID].empty() || j[ACTION].empty()) return;
    if(ws_events.find(j[ACTION]) == ws_events.end()) return;

    if (j[ACTION] == WEBRTC_OFFER) {
        on_webrtc_offer(event, j);
    }
}

void CarSession::on_webrtc_offer(Event<Websocket>* event, nlohmann::json& j){
    redirect_message_to_pilot(j);
}

void CarSession::remove_pilot() {
    if(pilot != nullptr){
        this->remove_event_listener(pilot);
        pilot->remove_event_listener(this);
        pilot = nullptr;
    }
}

CarSessionManager::CarSessionManager(asio::io_context& ctx)
    :ws_connections(make_shared<WebsocketManager>(ctx, 8080)), ctx(ctx){}

void CarSessionManager::init() {
    ws_connections->add_event_listener(this);
}

void CarSessionManager::stop() {
    for(auto& connection: connections){
        connection->remove_event_listener(this);
        connection->remove_pilot();
    }
    cout << "CarSessionManager::stop()" << endl;
    connections.clear();
    ws_connections->is_running = false;
    ws_connections->remove_event_listener(this);
    ws_connections->stop();
}


void CarSessionManager::on_event(Event<WebsocketManager>* event) {

    if(event->action == CLOSE){
        on_close(event);
        return;
    }

    cout << "CarSessionManager WS Event: " <<  event->message << endl;
    auto j = nlohmann::json::parse(event->message);
    if(j[CAR_ID].empty() || j[ACTION].empty()) return;
    if(ws_events.find(j[ACTION]) == ws_events.end()) return;

    if(j[ACTION] == AUTH_SESSION){
        on_auth_session(event, j);
    }
}

void CarSessionManager::on_close(Event<WebsocketManager>* event) {
    cout << "CarSessionManager WS close" << endl;
    auto ws = (Websocket*)event->data;
    shared_ptr<CarSession> session  {nullptr};
    for(const auto& connection: connections){
        if(connection->ws == ws){
            session = connection;
        }
    }
    session->remove_event_listener(this);
    ws->remove_event_listener(session.get());
    remove_connection(session);
}

void CarSessionManager::on_auth_session(Event<WebsocketManager>* event, nlohmann::json& j) {
    cout << "Auth session message is received, car_id: " << j[CAR_ID] << endl;
    auto it = connections.begin();
    while(it != connections.end()){
        shared_ptr<CarSession> session = *it;
        if(session->car_id == str_to_uuid(j[CAR_ID])){
            cerr << "Car session exists" <<  endl;
            break;
        }
        it++;
    }
    if(it == connections.end()){
        try{
            cout << "Creating a new car session" <<  endl;
            auto ws = (Websocket *)event->data;
            auto session = make_shared<CarSession>(str_to_uuid(j[CAR_ID]), ws, ctx);
            nlohmann::json j;
            j[ACTION] = AUTH_ACCEPT;
            session->ws->send_message(j.dump());
            session->add_event_listener(this);
            ws->add_event_listener(session.get());
            connections.push_back(session);
        } catch (std::exception& e){
            cerr << "Failed to create a car session. " << e.what()<< endl;
        }
    }
}

void CarSessionManager::on_event(Event<CarSession>* event) {

}

CarSession *CarSessionManager::handle_car_control_request(uuid car_id, PilotSession* candidate) {
    CarSession* result {nullptr};
    for(const auto& session: connections){
        if(session->pilot == nullptr && session->car_id == car_id){
            result = session.get();
            session->pilot = candidate;
            session->add_event_listener(candidate);
            candidate->add_event_listener(session.get());
        }
    }
    return result;
}


