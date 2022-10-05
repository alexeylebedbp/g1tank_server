//
// Created by alexeylebed on 10/3/22.
//

#include "PilotSession.h"

PilotSession::PilotSession(uuid pilot_id, const shared_ptr<Websocket>& ws,  asio::io_context& ctx)
        :pilot_id(pilot_id),
         ctx(ctx),
         session_id(boost::uuids::random_generator()()),
         ws(ws){}

bool PilotSession::get_car_control(uuid car_id) {
    if(car != nullptr){
        cerr << "PilotSession::get_car_control already controls a car" << endl;
        return false;
    } else {

    }
}

PilotSessionManager::PilotSessionManager(asio::io_context& ctx)
        :ws_connections(make_shared<WebsocketManager>(ctx, "8080")), ctx(ctx){}

void PilotSessionManager::init(){
    ws_connections->add_event_listener(shared_from_this());
}

void PilotSessionManager::on_event(const shared_ptr<Event<WebsocketManager>> &event) {
    //Auth and creation of a pilot session
}

void PilotSessionManager::on_event(const shared_ptr<Event<PilotSession>> &event) {
    //Auth and creation of a pilot session
}

bool PilotSessionManager::get_car_control(uuid car_id) {
    return false;
}
