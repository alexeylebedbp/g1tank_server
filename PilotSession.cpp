//
// Created by alexeylebed on 10/3/22.
//

#include "PilotSession.h"
#include "CarSession.h"


PilotSession::PilotSession(uuid pilot_id, Websocket* ws,  asio::io_context& ctx, const shared_ptr<PilotSessionManager>& manager)
        :pilot_id(pilot_id),
         ctx(ctx),
         session_id(boost::uuids::random_generator()()),
         manager(manager.get()),
         ws(ws){}

CarSession* PilotSession::get_car_control(const uuid& car_id) {
    if(car != nullptr){
        cerr << "PilotSession::get_car_control already controls a car" << endl;
        return nullptr;
    } else {
        return manager->get_car_control(car_id, this);
    }
}

void PilotSession::redirect_message_to_car(nlohmann::json& j) {
    if(car == nullptr){
        cerr << "Unable to find a car" << endl;
    } else {
        cout << "Redirecting message " << j[ACTION] << " to the Car" << endl;
        car->ws->send_message(j.dump());
    }
}

void PilotSession::on_event(Event<Websocket>* event) {
    cout << "PilotSession WS event: " << event->action << " " << event->message << endl;

    if(event->action == CLOSE || event->message.empty()){
        return;
    }

    auto j = nlohmann::json::parse(event->message);
    if (j[PILOT_ID].empty() || j[ACTION].empty()) return;
    if(ws_events.find(j[ACTION]) == ws_events.end()) return;

    if (j[ACTION] == GET_CAR_CONTROL) {
        on_get_car_control(event, j);
    } else if(j[ACTION] == MOVE){
        on_move(event, j);
    } else if(j[ACTION] == OFFER_REQUEST){
        on_offer_request(event, j);
    } else if(j[ACTION] == WEBRTC_ANSWER){
        on_webrtc_answer(event, j);
    } else if(j[ACTION] == BYEBYE){
        on_byebye(event, j);
    }
}

void PilotSession::on_event(Event<CarSession>* event){
    cout << "PilotSession Car event: " << event->message << endl;

    auto j = nlohmann::json::parse(event->message);
    if (j[PILOT_ID].empty() || j[ACTION].empty()) return;
    if(car_events.find(j[ACTION]) == ws_events.end()) return;

    if(event->action == "close"){
        on_car_disconnected(event, j);
    }
}

void PilotSession::on_get_car_control(Event<Websocket>* event, nlohmann::json& j){
    try {
        uuid car_id = str_to_uuid(j[CAR_ID]);
        CarSession* _car = get_car_control(car_id);
        if(_car == nullptr){
            nlohmann::json res;
            res[ACTION] = FAILED_TO_OBTAIN_CAR_CONTROL;
            res[CAR_ID] = j[CAR_ID];
            this->ws->send_message(res.dump());
            return;
        }
        this->add_car(_car);
        cout << "Pilot "<< this->pilot_id << " got control on the car " <<  this->car->car_id << endl;
        nlohmann::json res;
        res[ACTION] = CAR_CONTROL_OBTAINED;
        res[CAR_ID] = j[CAR_ID];
        this->ws->send_message(res.dump());
    } catch(std::exception& e){
        cerr << "Couldn't get car control" << endl;
    }
}

void PilotSession::on_move(Event<Websocket>* event, nlohmann::json &j) {
    redirect_message_to_car(j);
}

void PilotSession::on_offer_request(Event<Websocket>* event, nlohmann::json &j) {
    redirect_message_to_car(j);
}

void PilotSession::on_webrtc_answer(Event<Websocket>* event, nlohmann::json &j) {
    redirect_message_to_car(j);
}

void PilotSession::on_car_disconnected(Event<CarSession>* event, nlohmann::json &j) {
    cout << "PilotSession: CarSession is closed" << endl;
    nlohmann::json res;
    res[ACTION] = CAR_DISCONNECTED;
    res[CAR_ID] = car->car_id;
    ws->send_message(res.dump());
    remove_car();
}

void PilotSession::on_byebye(Event<Websocket>* event, nlohmann::json &j) {
    ws->remove_event_listener(this);
    remove_car();
}

void PilotSession::init() {
    ws->add_event_listener(this);
}

void PilotSession::add_car(CarSession* car_) {
    car = car_;
    car->pilot = this;
    this->add_event_listener(car_);
    car_->add_event_listener(this);
}

void PilotSession::remove_car(){
    if(this->car){
        this->remove_event_listener(car);
        car->remove_event_listener(this);
        this->car = nullptr;
    }
}

PilotSessionManager::PilotSessionManager(asio::io_context& ctx)
        :ws_connections(make_shared<WebsocketManager>(ctx, 8081)),
         ctx(ctx){}

void PilotSessionManager::init(const shared_ptr<CarSessionManager>& car_manager, Server* server_){
    ws_connections->add_event_listener(this);
    this->car_session_manager = car_manager.get();
    this->server = server_;
}

void PilotSessionManager::stop() {
    connections.clear();
    ws_connections->is_running = false;
    ws_connections->remove_event_listener(this);
    ws_connections->stop();
}


void PilotSessionManager::on_event(Event<WebsocketManager>* event) {
    cout << "PilotSessionManager: " <<  event->message << endl;
    if(event->action == CLOSE){
        on_close(event);
        return;
    }
    try {
        auto j = nlohmann::json::parse(event->message);
        if(j[PILOT_ID].empty() || j[ACTION].empty()) return;
        if(ws_events.find(j[ACTION]) == ws_events.end()) return;

        if(j[ACTION] == AUTH_SESSION){
            on_auth_session(event, j);
        } else if(j[ACTION] == BYEBYE){
            on_stop_signal();
        }
    } catch (std::exception& e){
        cerr << "PilotSessionManager WebsocketManager event handling error" << e.what()<< endl;
    }
}

void PilotSessionManager::on_auth_session(Event<WebsocketManager>* event, nlohmann::json& j) {
    cout << "Auth session message is received, pilot_id: " << j[PILOT_ID] << endl;
    auto it = connections.begin();
    while(it != connections.end()){
        shared_ptr<PilotSession> session = *it;
        if(session->pilot_id == str_to_uuid(j[PILOT_ID])){
            cerr << "Pilot session exists" <<  endl;
            break;
        }
        it++;
    }
    if(it == connections.end()){
        try{
            auto session = make_shared<PilotSession>(
                str_to_uuid(j[PILOT_ID]),
                (Websocket *)event->data, ctx,
                shared_from_this()
            );
            session->add_event_listener(this);
            session->init();
            connections.push_back(session);
            cout << "Pilot Session Created!" << endl;
        } catch (std::exception& e){
            cerr << "Failed to create a pilot session. " << e.what()<< endl;
        }
    }
}

void PilotSessionManager::on_close(Event<WebsocketManager>* event) {
    cout << "PilotSessionManager WS close" << endl;
    auto ws = (Websocket*)event->data;
    shared_ptr<PilotSession> session  {nullptr};
    for(const auto& connection: connections){
        if(connection->ws == ws){
            session = connection;
        }
    }
    session->remove_event_listener(this);
    ws->remove_event_listener(session.get());
    remove_connection(session);
}


CarSession* PilotSessionManager::get_car_control(uuid car_id, PilotSession* pilot) {
    return car_session_manager->handle_car_control_request(car_id, pilot);
}

void PilotSessionManager::on_event(Event<PilotSession>* event) {}