//
// Created by alexeylebed on 10/5/22.
//
#include "Event.h"

void EventEmitter::emit_event(const string& action, const string& message){
    auto event = make_shared<Event>(action, message,(void*) this);
    for(const auto& listener: listeners){
        listener->on_event(event);
    }
}

void EventEmitter::emit_event(const string& action){
    auto event = make_shared<Event>(action, (void*) this);
    for(const auto& listener: listeners){
        listener->on_event(event);
    }
}

void EventEmitter::emit_event(const string& action, const string& message, void* data){
    auto event = make_shared<Event>(action, message, (void*) this, data);
    for(const auto& listener: listeners){
        listener->on_event(event);
    }
}
