//
// Created by alexeylebed on 9/28/22.
//

#include "CarSession.h"
#include "uuid.h"
CarSession::CarSession(uuid car_id, const shared_ptr<Websocket>& ws)
    :car_id(car_id),
     session_id(boost::uuids::random_generator()()),
     ws(ws){}

void CarSessionManager::handle_event(WebsocketEvent& event) {
    if(event.type == WebsocketEventType::close){
        cout << "CarSessionManager subscribe" << endl;
        event.ws->unsubscribe(this);
        return;
    }
    auto j = nlohmann::json::parse(event.message);
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
            connections.push_back(make_shared<CarSession>(str_to_uuid(j["car_id"]), event.ws));
        }
    }
}

CarSessionManager::CarSessionManager(const shared_ptr<WebsocketManager>& wsm): ws_connections(wsm) {}
