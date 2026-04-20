#ifndef EVENTBUS_H
#define EVENTBUS_H

#include <string>
#include <vector>
#include <unordered_map>
#include "lua.hpp"
#include "LuaBridge.h"

class EventBus {
public:
    // Called from Lua
    static void Publish(const std::string& event_type, luabridge::LuaRef event_object);
    static void Subscribe(const std::string& event_type, luabridge::LuaRef component, luabridge::LuaRef function);
    static void Unsubscribe(const std::string& event_type, luabridge::LuaRef component, luabridge::LuaRef function);

    // Called by Engine at end of frame (before physics step)
    static void ProcessPendingSubscriptions();

    // Called on scene load / engine shutdown to clear all subscriptions
    static void Clear();

private:
    using Subscriber = std::pair<luabridge::LuaRef, luabridge::LuaRef>; // {component, function}

    // Active subscribers per event type
    static inline std::unordered_map<std::string, std::vector<Subscriber>> subscribers;

    // Pending subscribes/unsubscribes to apply at end of frame
    static inline std::vector<std::pair<std::string, Subscriber>> pending_subscribes;
    static inline std::vector<std::pair<std::string, Subscriber>> pending_unsubscribes;
};

#endif