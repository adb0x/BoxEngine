#include "EventBus.h"
#include "ComponentDB.h"
#include <algorithm>

void EventBus::Publish(const std::string& event_type, luabridge::LuaRef event_object) {
    auto it = subscribers.find(event_type);
    if (it == subscribers.end()) return;

    // Copy the list so subscribe/unsubscribe during publish doesn't invalidate iteration
    std::vector<Subscriber> current = it->second;

    for (auto& [component, function] : current) {
        try {
            if (event_object.isNil()) {
                // No event object passed � call with just self
                function(component);
            }
            else {
                function(component, event_object);
            }
        }
        catch (const luabridge::LuaException& e) {
            // Report error properly instead of swallowing
            std::string err = e.what();
            std::cout << "\033[31m" << err << "\033[0m" << std::endl;
        }
    }
}

void EventBus::Subscribe(const std::string& event_type, luabridge::LuaRef component, luabridge::LuaRef function) {
    pending_subscribes.push_back({ event_type, { component, function } });
}

void EventBus::Unsubscribe(const std::string& event_type, luabridge::LuaRef component, luabridge::LuaRef function) {
    pending_unsubscribes.push_back({ event_type, { component, function } });
}

void EventBus::ProcessPendingSubscriptions() {
    // Process unsubscribes first, then subscribes
    for (auto& entry : pending_unsubscribes) {
    const std::string& event_type = entry.first;
    const Subscriber& sub = entry.second;

    auto it = subscribers.find(event_type);
    if (it == subscribers.end()) continue;

    auto& vec = it->second;

    vec.erase(std::remove_if(vec.begin(), vec.end(),
        [&](const Subscriber& s) {
            return s.first == sub.first && s.second == sub.second;
        }),
        vec.end());
    }
    pending_unsubscribes.clear();

    for (auto& [event_type, sub] : pending_subscribes) {
        subscribers[event_type].push_back(sub);
    }
    pending_subscribes.clear();
}

void EventBus::Clear() {
    subscribers.clear();
    pending_subscribes.clear();
    pending_unsubscribes.clear();
}