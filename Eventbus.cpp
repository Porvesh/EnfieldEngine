#include "headers/Eventbus.h"
#include <algorithm>
#include <iostream>

using Subscription = std::pair<luabridge::LuaRef, luabridge::LuaRef>;
std::unordered_map<std::string, std::vector<Subscription>> EventBus::subscriptions;
std::vector<std::tuple<std::string, luabridge::LuaRef, luabridge::LuaRef>> EventBus::pendingSubscriptions;
std::vector<std::tuple<std::string, luabridge::LuaRef, luabridge::LuaRef>> EventBus::pendingUnsubscriptions;

void EventBus::Publish(const std::string& event_type, luabridge::LuaRef event_object) {
    auto& subscribers = subscriptions[event_type];
    for (auto& [component, function] : subscribers) {
        try {
            function(component, event_object);
        }
        catch (const luabridge::LuaException& e) {
            std::cerr << "LuaException in event publish: " << e.what() << std::endl;
        }
    }
}

void EventBus::Subscribe(const std::string& event_type, luabridge::LuaRef component, luabridge::LuaRef function) {
    pendingSubscriptions.push_back({ event_type, component, function });
}

void EventBus::Unsubscribe(const std::string& event_type, luabridge::LuaRef component, luabridge::LuaRef function) {
    pendingUnsubscriptions.push_back({ event_type, component, function });
}

void EventBus::ProcessSubscriptions() {
    // Apply subscriptions
    for (auto& [event_type, component, function] : pendingSubscriptions) {
        subscriptions[event_type].emplace_back(component, function);
    }
    pendingSubscriptions.clear();

    for (auto& pending : pendingUnsubscriptions) {
        auto& [event_type, component, function] = pending;

        // Capture the variables needed by reference before the structured binding declaration
        luabridge::LuaRef& captured_component = component;
        luabridge::LuaRef& captured_function = function;

        auto& subscribers = subscriptions[event_type];
        subscribers.erase(
            std::remove_if(subscribers.begin(), subscribers.end(),
                [&captured_component, &captured_function](const Subscription& sub) {
                    return sub.first == captured_component && sub.second == captured_function;
                }
            ),
            subscribers.end()
                    );
    }
    pendingUnsubscriptions.clear();
}


void EventBus::clearAll() {
    subscriptions.clear();
    pendingSubscriptions.clear();
    pendingUnsubscriptions.clear();
}