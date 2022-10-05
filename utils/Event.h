//
// Created by alexeylebed on 10/4/22.
//

#ifndef G1TANK_EVENT_H
#define G1TANK_EVENT_H
#include "iostream"
#include "include_boost_asio.h"
#include <vector>
using namespace std;

template<typename EventEmitter>
struct Event {
    EventEmitter* emitter;
    string action;
    string  message;
    void* data;

    Event(const string& action, const string& message, void* emitter)
        :action(action), message(message), emitter((EventEmitter*)emitter){};

    Event(const string& action, const string& message, void* emitter, void* data)
            :action(action), message(message), emitter((EventEmitter*)emitter), data(data){};

    Event(const string& action, const void* emitter)
            :action(action), emitter((EventEmitter*)emitter){};
};

template <typename  EventEmitter>
class EventListener {
public:
    virtual void on_event(const shared_ptr<Event<EventEmitter>>& event){};
};

template<typename T>
class EventEmitter {
public:
    virtual void add_event_listener(const shared_ptr<EventListener<T>>& candidate) {
        auto it = find(listeners.begin(), listeners.end(), candidate);
        if(it == listeners.end()){
            listeners.push_back(candidate);
        }
    };
    virtual void remove_event_listener(const shared_ptr<EventListener<T>>& candidate){
        auto it = find(listeners.begin(), listeners.end(), candidate);
        if(it != listeners.end()){
            listeners.erase(it);
        }
    };
    virtual void emit_event(const string& action, const string& message, void* data){
        auto event = make_shared<Event<T>>(action, message, (void*) this , data);
        for(const auto& listener: listeners){
            listener->on_event(event);
        }
    };

    virtual void emit_event(const string& action, const string& message){
        auto event = make_shared<Event<T>>(action, message, (void*)this);
        for(const auto& listener: listeners){
            listener->on_event(event);
        }
    }
    virtual void emit_event(const string& action){
        auto event = make_shared<Event<T>>(action, (void*) this);
        for(const auto& listener: listeners){
            listener->on_event(event);
        }
    };

protected:
    vector<shared_ptr<EventListener<T>>> listeners{};
};





#endif //G1TANK_EVENT_H
