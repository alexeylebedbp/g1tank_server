//
// Created by alexeylebed on 9/28/22.
//

#ifndef G1TANK_UUID_H
#define G1TANK_UUID_H
#include "include_boost_asio.h"

using namespace std;

uuid str_to_uuid(const string& str){
    return boost::lexical_cast<uuid>(str);
}

#endif //G1TANK_UUID_H
