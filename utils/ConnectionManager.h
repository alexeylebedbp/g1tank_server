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
    typedef shared_ptr<T> ptr;
protected:
    vector<ptr> connections;
public:
    virtual void add_connection(ptr connection){
        connections.push_back(connection);
    };
    virtual void remove_connection(ptr connection){
        for(auto it = connections.begin(); it != connections.end(); it++){
            if(*it == connection){
                connections.erase(it);
            }
        }
    };
};


#endif //G1TANK_CONNECTIONMANAGER_H
