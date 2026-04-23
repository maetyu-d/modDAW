#include "RouteState.h"

namespace
{
juce::String readString(const juce::var& object, const juce::Identifier& propertyName)
{
    if (auto* dynamicObject = object.getDynamicObject())
        return dynamicObject->getProperty(propertyName).toString();

    return {};
}

bool readBool(const juce::var& object, const juce::Identifier& propertyName, bool fallbackValue)
{
    if (auto* dynamicObject = object.getDynamicObject())
    {
        auto value = dynamicObject->getProperty(propertyName);
        return value.isVoid() ? fallbackValue : static_cast<bool>(value);
    }

    return fallbackValue;
}
}

juce::String RouteEndpointEntry::toSummaryString() const
{
    return displayName + " [" + family + " " + direction + "]";
}

juce::String RouteEntry::toSummaryString() const
{
    return routeId + " | " + family + " | " + source + " -> " + destination
         + " | " + (enabled ? "enabled" : "disabled");
}

RouteState RouteState::fromPayload(const juce::var& payload)
{
    RouteState state;

    if (auto* dynamicObject = payload.getDynamicObject())
    {
        if (auto* routesArray = dynamicObject->getProperty("routes").getArray())
        {
            for (const auto& item : *routesArray)
            {
                RouteEntry route;
                route.routeId = readString(item, "routeId");
                route.family = readString(item, "family");
                route.source = readString(item, "source");
                route.destination = readString(item, "destination");
                route.enabled = readBool(item, "enabled", false);
                state.routes.add(route);
            }
        }

        if (auto* endpointsArray = dynamicObject->getProperty("endpoints").getArray())
        {
            for (const auto& item : *endpointsArray)
            {
                RouteEndpointEntry endpoint;
                endpoint.id = readString(item, "id");
                endpoint.displayName = readString(item, "displayName");
                endpoint.family = readString(item, "family");
                endpoint.direction = readString(item, "direction");
                endpoint.ownerId = readString(item, "ownerId");
                state.endpoints.add(endpoint);
            }
        }
    }

    return state;
}

juce::String RouteState::toSummaryString() const
{
    return juce::String(routes.size()) + " routes, " + juce::String(endpoints.size()) + " endpoints";
}

const RouteEntry* RouteState::findRouteById(const juce::String& routeId) const
{
    for (const auto& route : routes)
        if (route.routeId == routeId)
            return &route;

    return nullptr;
}
