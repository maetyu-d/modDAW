#pragma once

#include <JuceHeader.h>
#include "RouteState.h"

class RoutingGraph
{
public:
    using StateCallback = std::function<void(const RouteState&)>;

    RoutingGraph() = default;

    void setStateCallback(StateCallback callback);
    void initialiseState(const RouteState& initialState);
    RouteState getState() const;

    bool createRoute(const juce::String& family,
                     const juce::String& source,
                     const juce::String& destination,
                     bool enabled,
                     juce::String& errorText);
    bool deleteRoute(const juce::String& routeId);

    bool hasEnabledAudioRouteForModule(const juce::String& moduleId) const;

private:
    void emitStateChanged();
    const RouteEndpointEntry* findEndpoint(const juce::String& endpointId) const;

    mutable juce::CriticalSection lock;
    RouteState state;
    StateCallback stateCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RoutingGraph)
};
