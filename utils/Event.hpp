////
//// Created by alexeylebed on 10/5/22.
////
//#include "Event.h"
//template<typename T>
//void EventEmitter<T>::emit_event(const string& action, const string& message){
//    auto event = make_shared<Event<T>>(action, message, shared_ptr<T>(this));
//    for(const auto& listener: listeners){
//        listener->on_event(event);
//    }
//}
//template<typename T>
//void EventEmitter<T>::emit_event(const string& action){
//    auto event = make_shared<Event<T>>(action, shared_ptr<T>(this));
//    for(const auto& listener: listeners){
//        listener->on_event(event);
//    }
//}
//template<typename T>
//void EventEmitter<T>::emit_event(const string& action, const string& message, void* data){
//    auto event = make_shared<Event<T>>(action, message, shared_ptr<T>(this), data);
//    for(const auto& listener: listeners){
//        listener->on_event(event);
//    }
//}
