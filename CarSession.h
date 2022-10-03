//
// Created by alexeylebed on 9/28/22.
//

#ifndef G1TANK_СARSESSION_H
#define G1TANK_СARSESSION_H
#include "Websocket.h"
#include "include_boost_asio.h"

class CarSession {
public:
    uuid session_id;
    uuid car_id;
    shared_ptr<Websocket>ws;
    CarSession(uuid car_id, shared_ptr<Websocket> ws);
};


class CarSessionManager: public ConnectionManager<CarSession>, public WebsocketEventSubscriber {

};


#endif //G1TANK_СARSESSION_H
