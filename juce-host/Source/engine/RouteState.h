#pragma once

#include <JuceHeader.h>

struct RouteEndpointEntry
{
    juce::String id;
    juce::String displayName;
    juce::String family;
    juce::String direction;
    juce::String ownerId;

    juce::String toSummaryString() const;
};

struct RouteEntry
{
    juce::String routeId;
    juce::String family;
    juce::String source;
    juce::String destination;
    bool enabled = false;

    juce::String toSummaryString() const;
};

struct RouteState
{
    juce::Array<RouteEntry> routes;
    juce::Array<RouteEndpointEntry> endpoints;

    static RouteState fromPayload(const juce::var& payload);
    juce::String toSummaryString() const;
    const RouteEntry* findRouteById(const juce::String& routeId) const;
};
