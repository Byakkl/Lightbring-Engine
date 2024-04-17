#pragma once

#include <unordered_map>
#include <functional>

template<typename... Args>
class Event{
public:
    /// @brief Type used for handling registering and unregistering with the event
    using SubscriptionId = size_t;
    /// @brief Shorthand for the function type used by the instance of the template
    using FunctionType = std::function<void(Args...)>;

    /// @brief Registers a callback to the event
    /// @param listener The callback to register with
    /// @return Returns an id used for unregistering from the event
    SubscriptionId Register(const FunctionType& listener)
    {
        //Fetch the id value and increment the id counter
        SubscriptionId id = nextId++;
        //Add the callback to the list
        listeners[id] = listener;
        //Return the id
        return id;
    }

    /// @brief Unregisters a callback from the event
    /// @param id The id of the callback to unregister
    void Unregister(SubscriptionId id)
    {
        //Remove the entry
        listeners.erase(id);
    }

    /// @brief Invokes the event
    /// @param ...args 
    void Invoke(Args... args)
    {
        //Go through the map of callbacks and invoke the functions
        for(const auto& pair : listeners)
            pair.second(args...);
    }

private:
    //Map of IDs to registered callbacks
    std::unordered_map<SubscriptionId, FunctionType> listeners;
    //Counter to provide unique IDs for this event on registration of a new callback
    SubscriptionId nextId = 0;
};