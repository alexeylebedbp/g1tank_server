//
// Created by alexeylebed on 9/28/22.
//

#include "СarSession.h"
CarSession::CarSession(uuid car_id, shared_ptr<Websocket> ws)
    :car_id(car_id),
     session_id(boost::uuids::random_generator()()),
     ws(ws){}