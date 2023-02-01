//
// Created by alexeylebed on 10/4/22.
//

#ifndef G1TANK_EVENT_H
#define G1TANK_EVENT_H
#include "iostream"
#include "include_boost_asio.h"
#include <utility>
#include <vector>
#include <stack>

using namespace std;

template<typename T>
struct Event {
    T* emitter;
    string action;
    string  message;
    void* data;

    Event(const string& action, const string& message, T* emitter_)
        :action(action), message(message), emitter(emitter_){};

    Event(const string& action, const string& message, T* emitter_, void* data)
            :action(action), message(message), emitter(emitter_), data(data){};

    Event(const string& action, T* emitter_)
            :action(action), emitter(emitter_){};
};

template <typename  T>
class EventListener {
    set<string>event_types;
public:
    virtual void on_event(Event<T>* event){};
};

template<typename T>
class EventEmitter {
    bool listeners_locked {false};

    void signal(Event<T>* event){
        listeners_locked = true;

        for(const auto& listener: listeners){
            stacked_listeners.push(listener);
        }

        ///Reverse vector elements to stick to LIFO order. Otherwise we get memory leak when stopping the app
        while (!stacked_listeners.empty()){
            auto listener = stacked_listeners.top();
            stacked_listeners.pop();
            listener->on_event(event);
        }

        delete event;
        listeners_locked = false;

        ///Handle remove_event_listener Event
        while (!listeners_to_remove.empty()){
            auto next = listeners_to_remove.top();
            listeners_to_remove.pop();
            auto it = find(listeners.begin(), listeners.end(), next);
            if(it != listeners.end()){
                listeners.erase(it);
            }
        }
    }
protected:
    vector<EventListener<T>*> listeners{};
    stack<EventListener<T>*> stacked_listeners{};
    stack<EventListener<T>*> listeners_to_remove{};
public:
    void add_event_listener(EventListener<T>* candidate) {
        auto it = find(listeners.begin(), listeners.end(), candidate);
        if(it == listeners.end()){
            listeners.push_back(candidate);
        }
    };

    void remove_event_listener(EventListener<T>* candidate){
        if(listeners_locked){
            listeners_to_remove.push(candidate);
        } else {
            auto it = find(listeners.begin(), listeners.end(), candidate);
            if(it != listeners.end()){
                listeners.erase(it);
            }
        }
    };


    virtual void emit_event(const string& action, const string& message, void* data){
        auto event = new Event<T>(action, message, (T*)this , data);
        signal(event);
    };

    virtual void emit_event(const string& action, const string& message){
        auto event = new Event<T>(action, message, (T*)this);
        signal(event);
    };

    virtual void emit_event(const string& action){
        auto event = new Event<T>(action, (T*)this);
        signal(event);
    };
};





#endif //G1TANK_EVENT_H
