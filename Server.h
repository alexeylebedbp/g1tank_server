//
// Created by Alexey Lebed on 9/26/22.
//

#ifndef G1TANK_SERVER_H
#define G1TANK_SERVER_H

#include "iostream"
#include <vector>
#include "CarConnection.h"
#include "PilotConnection.h"

using namespace std;

class Server {
    vector<CarConnection> car_connections{};
    vector<PilotConnection>pilot_connections{};

};


#endif //G1TANK_SERVER_H
