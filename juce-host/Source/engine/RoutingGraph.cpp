#include "RoutingGraph.h"

void RoutingGraph::setStateCallback(StateCallback callback)
{
    const juce::ScopedLock scopedLock(lock);
    stateCallback = std::move(callback);
}

void RoutingGraph::initialiseState(const RouteState& initialState)
{
    {
        const juce::ScopedLock scopedLock(lock);
        state = initialState;
    }

    emitStateChanged();
}

RouteState RoutingGraph::getState() const
{
    const juce::ScopedLock scopedLock(lock);
    return state;
}

bool RoutingGraph::createRoute(const juce::String& family,
                               const juce::String& source,
                               const juce::String& destination,
                               bool enabled,
                               juce::String& errorText)
{
    {
        const juce::ScopedLock scopedLock(lock);

        const auto* sourceEndpoint = findEndpoint(source);
        const auto* destinationEndpoint = findEndpoint(destination);

        if (sourceEndpoint == nullptr || destinationEndpoint == nullptr)
        {
            errorText = "endpoint missing";
            return false;
        }

        if (sourceEndpoint->direction != "output")
        {
            errorText = "source must be output";
            return false;
        }

        if (destinationEndpoint->direction != "input")
        {
            errorText = "destination must be input";
            return false;
        }

        if (sourceEndpoint->family != family || destinationEndpoint->family != family)
        {
            errorText = "family mismatch";
            return false;
        }

        for (const auto& route : state.routes)
        {
            if (route.source == source && route.destination == destination)
            {
                errorText = "route already exists";
                return false;
            }
        }

        RouteEntry route;
        route.routeId = "route.native." + juce::String(state.routes.size() + 1);
        route.family = family;
        route.source = source;
        route.destination = destination;
        route.enabled = enabled;
        state.routes.add(route);
    }

    emitStateChanged();
    return true;
}

bool RoutingGraph::deleteRoute(const juce::String& routeId)
{
    bool removed = false;

    {
        const juce::ScopedLock scopedLock(lock);
        const auto before = state.routes.size();
        state.routes.removeIf([&](const RouteEntry& route) { return route.routeId == routeId; });
        removed = state.routes.size() != before;
    }

    if (removed)
        emitStateChanged();

    return removed;
}

bool RoutingGraph::hasEnabledAudioRouteForModule(const juce::String& moduleId) const
{
    const juce::ScopedLock scopedLock(lock);
    const auto endpointPrefix = moduleId + ".audio.out";
    for (const auto& route : state.routes)
    {
        if (! route.enabled || route.family != "audio")
            continue;

        if (route.source.startsWith(moduleId) || route.source == endpointPrefix || route.source.startsWith(moduleId + "."))
            return true;
    }

    return false;
}

void RoutingGraph::emitStateChanged()
{
    StateCallback callbackCopy;
    RouteState snapshot;

    {
        snapshot = state;
        callbackCopy = stateCallback;
    }

    if (callbackCopy)
        callbackCopy(snapshot);
}

const RouteEndpointEntry* RoutingGraph::findEndpoint(const juce::String& endpointId) const
{
    for (const auto& endpoint : state.endpoints)
        if (endpoint.id == endpointId)
            return &endpoint;

    return nullptr;
}
