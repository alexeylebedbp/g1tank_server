//
// Created by alexeylebed on 10/3/22.
//

#include "PilotSession.h"
#include "CarSession.h"


PilotSession::PilotSession(uuid pilot_id, Websocket* ws,  asio::io_context& ctx, const shared_ptr<PilotSessionManager>& manager)
        :pilot_id(pilot_id),
         ctx(ctx),
         session_id(boost::uuids::random_generator()()),
         manager(manager),
         ws(ws){}

CarSession* PilotSession::get_car_control(const uuid& car_id) {
    if(car != nullptr){
        cerr << "PilotSession::get_car_control already controls a car" << endl;
        return nullptr;
    } else {
        manager->get_car_control(car_id, this);
    }
}

void PilotSession::on_event(const shared_ptr<Event<CarSession>>& event){
    if(event->action == "close"){
        cout << "PilotSession: CarSession is closed" << endl;
        nlohmann::json j;
        j["action"] = "car_disconnected";
        j["car_id"] = car->car_id;
        ws->send_message(j.dump());
        remove_car(event->emitter);
    }
}

void PilotSession::init() {
    ws->add_event_listener(shared_from_this());
}

void PilotSession::add_car(CarSession* car_) {
    this->car = car_;
    car_->add_event_listener(shared_from_this());
}

void PilotSession::remove_car(CarSession* car_) {
    this->car = nullptr;
    //car_->remove_event_listener(shared_from_this());
}

CarSession* PilotSession::get_car() {
    return car;
}

PilotSessionManager::PilotSessionManager(asio::io_context& ctx)
        :ws_connections(make_shared<WebsocketManager>(ctx, 8081)),
         ctx(ctx){}

void PilotSessionManager::init(const shared_ptr<CarSessionManager>& car_manager){
    ws_connections->add_event_listener(shared_from_this());
    this->car_session_manager = car_manager.get();
}

void PilotSessionManager::on_event(const shared_ptr<Event<WebsocketManager>>& event) {
    if(event->action == "close"){
        cout << "PilotSessionManager WS close" << endl;
        auto ws = (Websocket*)event->data;
        shared_ptr<PilotSession> session  {nullptr};
        for(const auto& connection: connections){
            if(connection->ws.get() == ws){
                session = connection;
            }
        }
        remove_connection(session);
        return;
    }
    cout << "PilotSessionManager: " <<  event->message << endl;
    auto j = nlohmann::json::parse(event->message);
    if(j["pilot_id"].empty() || j["action"].empty()) return;
    if(j["action"] == "auth_session"){
        cout << "Auth session message is received, pilot_id: " << j["pilot_id"] << endl;
        auto it = connections.begin();
        while(it != connections.end()){
            shared_ptr<PilotSession> session = *it;
            if(session->pilot_id == str_to_uuid(j["pilot_id"])){
                cerr << "Pilot session exists" <<  endl;
                break;
            }
            it++;
        }
        if(it == connections.end()){
            try{
                auto session = make_shared<PilotSession>(str_to_uuid(j["pilot_id"]), (Websocket *)event->data, ctx, shared_from_this());
                session->add_event_listener(shared_from_this());
                session->init();
                connections.push_back(session);
                cout << "Pilot Session Created!" << endl;
            } catch (std::exception& e){
                cerr << "Failed to create a pilot session. " << e.what()<< endl;
            }
        }
    } else {
        for(const auto& connection:connections){
            if(connection->pilot_id == str_to_uuid(j["pilot_id"])){
                if(j["action"] == "get_car_control"){
                    try {
                        cout << 106 << endl;
                        uuid car_id = str_to_uuid(j["car_id"]);
                        CarSession* _car = get_car_control(car_id, connection.get());
                        if(_car == nullptr){
                            nlohmann::json res;
                            res["action"] = "failed_to_obtain_car_control";
                            res["car_id"] = j["car_id"];
                            connection->ws->send_message(res.dump());
                            return;
                        }
                        connection->add_car(_car);
                        cout << "Pilot "<< connection->pilot_id << " got control on the car " <<  connection->get_car()->car_id << endl;
                        nlohmann::json res;
                        res["action"] = "car_control_obtained";
                        res["car_id"] = j["car_id"];
                        connection->ws->send_message(res.dump());
                    } catch(std::exception& e){
                        cerr << "Couldn't get car control" << endl;
                    }
                } else if(j["action"] == "move"){
                    if(connection->get_car() == nullptr){
                        cerr << "Unable to find a car" << endl;
                    } else {
                        cout << "Redirecting move command to a Car" << endl;
                        connection->get_car()->ws->send_message(j.dump());
                    }
                } else if(j["action"] == "byebye"){
                    on_stop_signal();
                } else if(j["action"] == "webrtc_answer"){
                    if(connection->get_car() == nullptr){
                        cerr << "Unable to find a car" << endl;
                    } else {
                        cout << "Redirecting webrtc_answer command to a Car" << endl;
                        connection->get_car()->ws->send_message(j.dump());
                    }
                } else if(j["action"] == "offer_request"){
                    if(connection->get_car() == nullptr){
                        cerr << "Unable to find a car" << endl;
                    } else {
                        cout << "Redirecting webrtc_answer command to a Car" << endl;
                        connection->get_car()->ws->send_message(j.dump());
                    }
                }
            }
        }
    }
}


CarSession* PilotSessionManager::get_car_control(uuid car_id, PilotSession* pilot) {
    return car_session_manager->handle_car_control_request(car_id, pilot);
}

void PilotSessionManager::on_event(const shared_ptr<Event<PilotSession>> &event) {}