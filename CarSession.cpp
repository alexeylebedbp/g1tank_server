//
// Created by alexeylebed on 9/28/22.
//
#include "CarSession.h"
#include "uuid.h"
#include "boost/asio/read.hpp"

CarSession::CarSession(uuid car_id, const shared_ptr<Websocket>& ws,  asio::io_context& ctx)
    :car_id(car_id),
     ctx(ctx),
     session_id(boost::uuids::random_generator()()),
     ws(ws){}

CarSessionManager::CarSessionManager(asio::io_context& ctx)
    :ws_connections(make_shared<WebsocketManager>(ctx, "8081")), ctx(ctx){}

void CarSessionManager::init() {
    ws_connections->add_event_listener(shared_from_this());
}

void CarSessionManager::on_event(const shared_ptr<Event<WebsocketManager>>& event) {
    if(event->action == "close"){
        cout << "CarSessionManager unsubscribe" << endl;
        event->emitter->remove_event_listener(shared_from_this());
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


