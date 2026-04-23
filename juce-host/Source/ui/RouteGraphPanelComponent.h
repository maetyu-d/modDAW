#pragma once

#include <JuceHeader.h>
#include "../engine/RouteState.h"

class RouteGraphPanelComponent final : public juce::Component
{
public:
    RouteGraphPanelComponent();

    std::function<void(const juce::String& family,
                       const juce::String& source,
                       const juce::String& destination,
                       bool enabled)> onCreateRoute;
    std::function<void(const juce::String& routeId)> onDeleteRoute;

    void setRouteState(const RouteState& newState);

    void paint(juce::Graphics& g) override;
    void mouseUp(const juce::MouseEvent& event) override;

private:
    struct NodeLayout
    {
        juce::String ownerId;
        juce::String displayName;
        juce::Rectangle<float> bounds;
    };

    struct PortLayout
    {
        RouteEndpointEntry endpoint;
        juce::Rectangle<float> bounds;
        juce::Point<float> centre;
    };

    void rebuildLayout();
    void drawRouteLine(juce::Graphics& g, const RouteEntry& route, bool highlighted);
    void drawPort(juce::Graphics& g, const PortLayout& port);
    void drawFamilyLegend(juce::Graphics& g, juce::Rectangle<float> area);

    const RouteEndpointEntry* endpointForId(const juce::String& endpointId) const;
    const PortLayout* portAt(juce::Point<float> position) const;
    const PortLayout* portForEndpoint(const juce::String& endpointId) const;
    const RouteEntry* routeAt(juce::Point<float> position) const;
    juce::Colour colourForFamily(const juce::String& family) const;
    juce::String displayNameForOwner(const juce::String& ownerId) const;
    bool canConnect(const RouteEndpointEntry& source, const RouteEndpointEntry& destination) const;
    juce::String validationMessageFor(const RouteEndpointEntry& source, const RouteEndpointEntry& destination) const;

    RouteState state;
    juce::Array<NodeLayout> nodes;
    juce::Array<PortLayout> ports;
    juce::String selectedSourceEndpointId;
    juce::String highlightedRouteId;
    juce::String statusText = "Click an output port, then a compatible input port.";

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RouteGraphPanelComponent)
};
