//
// Created by alexeylebed on 10/3/22.
//

#ifndef G1TANK_COROUTINE_ERR_HANDLER_H
#define G1TANK_COROUTINE_ERR_HANDLER_H

#include "iostream"
using namespace std;

inline auto exception_handler_generator (const string owner){
    return [owner](exception_ptr e){
        try {
            if (e) {rethrow_exception(e);}
        } catch(const exception& e) {
            cout << "Coro exception " << owner << e.what() << endl;
        }
    };
}
#endif //G1TANK_COROUTINE_ERR_HANDLER_H
