//
// Created by Alexey Lebed on 9/26/22.
//

#ifndef G1TANK_CONNECTIONMANAGER_H
#define G1TANK_CONNECTIONMANAGER_H
#include "iostream"
#include <vector>
using namespace std;

template <typename T>
class ConnectionManager {
    vector<T> connections {};

public:
    virtual void add_connection(T connection){};
    virtual void remove_connection(T& connection){};
};


#endif //G1TANK_CONNECTIONMANAGER_H
