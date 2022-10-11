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


PilotSessionManager::PilotSessionManager(asio::io_context& ctx)
        :ws_connections(make_shared<WebsocketManager>(ctx, 8081)), ctx(ctx){}

void PilotSessionManager::init(const shared_ptr<CarSessionManager>& car_manager){
    ws_connections->add_event_listener(shared_from_this());
    this->car_session_manager = car_manager;
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
                        uuid car_id = str_to_uuid(j["car_id"]);
                        CarSession* car = get_car_control(car_id, connection.get());
                        if(car == nullptr){
                            nlohmann::json res;
                            res["action"] = "failed_to_obtain_car_control";
                            res["car_id"] = j["car_id"];
                            connection->ws->send_message(res.dump());
                            return;
                        }
                        connection->car = car;
                        cout << "Pilot "<< connection->pilot_id << " got control on the car " <<  connection->car->car_id << endl;
                        nlohmann::json res;
                        res["action"] = "car_control_obtained";
                        res["car_id"] = j["car_id"];
                        connection->ws->send_message(res.dump());
                    } catch(std::exception& e){
                        cerr << "Couldn't get car control" << endl;
                    }
                } else if(j["action"] == "move"){
                    if(connection->car == nullptr){
                        cerr << "Unable to find a car" << endl;
                    } else {
                        cout << "Redirecting move command to a Car" << endl;
                        connection->car->ws->send_message(j.dump());
                    }
                }
            }
        }
    }
}

void PilotSession::init() {
    ws->add_event_listener(shared_from_this());
}


void PilotSessionManager::on_event(const shared_ptr<Event<PilotSession>> &event) {
    //Auth and creation of a pilot session
}

CarSession* PilotSessionManager::get_car_control(uuid car_id, PilotSession* pilot) {
    return car_session_manager->handle_car_control_request(car_id, pilot);
}
