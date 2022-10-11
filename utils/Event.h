//
// Created by alexeylebed on 10/4/22.
//

#ifndef G1TANK_EVENT_H
#define G1TANK_EVENT_H
#include "iostream"
#include "include_boost_asio.h"
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
public:
    virtual void on_event(const shared_ptr<Event<T>>& event){};
};

template<typename T>
class EventEmitter {
public:
    void add_event_listener(const shared_ptr<EventListener<T>>& candidate) {
        auto it = find(listeners.begin(), listeners.end(), candidate);
        if(it == listeners.end()){
            listeners.push_back(candidate);
        }
    };

    void remove_event_listener(const shared_ptr<EventListener<T>>& candidate){
        listeners_to_remove.push(candidate);
    };

    virtual void emit_event(const string& action, const string& message, void* data){
        auto event = make_shared<Event<T>>(action, message, (T*)this , data);
        for(const auto& listener: listeners){
            listener->on_event(event);
        }
        cleanup();
    };

    virtual void emit_event(const string& action, const string& message){
        auto event = make_shared<Event<T>>(action, message, (T*)this);
        for(const auto& listener: listeners){
            listener->on_event(event);
        }
        cleanup();
    }
    virtual void emit_event(const string& action){
        auto event = make_shared<Event<T>>(action, (T*)this);
        for(const auto& listener: listeners){
            listener->on_event(event);
        }
        cleanup();
    };

protected:
    vector<shared_ptr<EventListener<T>>> listeners{};
    stack<shared_ptr<EventListener<T>>> listeners_to_remove{};

private:
    void cleanup(){
        while (!listeners_to_remove.empty()){
            auto next = listeners_to_remove.top();
            listeners_to_remove.pop();
            auto it = find(listeners.begin(), listeners.end(), next);
            if(it != listeners.end()){
                listeners.erase(it);
            }
        }
    }
};





#endif //G1TANK_EVENT_H
