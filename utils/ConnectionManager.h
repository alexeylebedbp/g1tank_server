//
// Created by Alexey Lebed on 9/26/22.
//

#ifndef G1TANK_CONNECTIONMANAGER_H
#define G1TANK_CONNECTIONMANAGER_H
#include "iostream"
#include <vector>
#include "Event.h"
using namespace std;

template <typename T>
class ConnectionManager {
    typedef shared_ptr<T> ptr;
protected:
    vector<ptr> connections;
public:
     void add_connection(const ptr& connection){
        connections.push_back(connection);
    };

    void remove_connection(const ptr& connection){
        auto to_delete = connections.end();
        for(auto it = connections.begin(); it != connections.end(); it++){
            if(*it == connection){
                to_delete = it;
                break;
            }
        }
        if(to_delete != connections.end()) connections.erase(to_delete);
    };
};



#endif //G1TANK_CONNECTIONMANAGER_H
