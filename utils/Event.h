//
// Created by alexeylebed on 10/4/22.
//

#ifndef G1TANK_EVENT_H
#define G1TANK_EVENT_H
#include "iostream"
#include "include_boost_asio.h"
#include <vector>
using namespace std;


struct Event {
    void* emitter;
    string action;
    string  message;
    void* data;

    Event(const string& action, const string& message, void* emitter)
        :action(action), message(message), emitter(emitter){};

    Event(const string& action, const string& message, void* emitter, void* data)
            :action(action), message(message), emitter(emitter), data(data){};

    Event(const string& action, void* emitter)
            :action(action), emitter(emitter){};
};

class EventListener;

class EventEmitter {
public:
    virtual void add_event_listener(const shared_ptr<EventListener>& candidate) {
        auto it = find(listeners.begin(), listeners.end(), candidate);
        if(it == listeners.end()){
            listeners.push_back(candidate);
        }
    };
    virtual void remove_event_listener(const shared_ptr<EventListener>& candidate){
        auto it = find(listeners.begin(), listeners.end(), candidate);
        if(it != listeners.end()){
            listeners.erase(it);
        }
    };
    virtual void emit_event(const string& action, const string& message, void* data); // to forward events
    virtual void emit_event(const string& action, const string& message);
    virtual void emit_event(const string& action);

protected:
    vector<shared_ptr<EventListener>> listeners{};
};

class EventListener {
public:
    virtual void on_event(const shared_ptr<Event>& event){};
};







#endif //G1TANK_EVENT_H
