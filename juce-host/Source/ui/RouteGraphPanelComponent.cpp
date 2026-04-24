#include "RouteGraphPanelComponent.h"

namespace
{
juce::String ownerTitle(const juce::String& ownerId)
{
    auto text = ownerId;
    text = text.replace("module.", "");
    text = text.replace("strip.module.", "strip ");
    text = text.replace("master.", "master ");
    text = text.replace("global.", "global ");
    text = text.replace(".", " ");
    return text;
}
}

RouteGraphPanelComponent::RouteGraphPanelComponent()
{
}

void RouteGraphPanelComponent::setRouteState(const RouteState& newState)
{
    state = newState;
    rebuildLayout();
    repaint();
}

void RouteGraphPanelComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto inner = bounds.reduced(12.0f);
    auto header = inner.removeFromTop(26.0f);

    g.setColour(juce::Colour(0xff171b20));
    g.fillRoundedRectangle(bounds, 8.0f);

    g.setColour(juce::Colour(0xff2b333d));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 8.0f, 1.0f);

    g.setColour(juce::Colour(0xfff3f5f7));
    g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    g.drawText("Route Graph", header.removeFromLeft(210.0f).toNearestInt(), juce::Justification::centredLeft);

    drawFamilyLegend(g, header);

    auto footer = inner.removeFromBottom(20.0f);
    inner.removeFromBottom(6.0f);

    if (nodes.isEmpty())
        rebuildLayout();

    for (const auto& route : state.routes)
        drawRouteLine(g, route, route.routeId == highlightedRouteId);

    for (const auto& node : nodes)
    {
        g.setColour(juce::Colour(0xff101419));
        g.fillRoundedRectangle(node.bounds, 7.0f);

        g.setColour(juce::Colour(0xff303945));
        g.drawRoundedRectangle(node.bounds.reduced(0.5f), 7.0f, 1.0f);

        g.setColour(juce::Colour(0xfff3f5f7));
        g.setFont(juce::FontOptions(12.5f, juce::Font::bold));
        g.drawText(node.displayName,
                   node.bounds.withTrimmedLeft(10.0f).withHeight(20.0f).toNearestInt(),
                   juce::Justification::centredLeft);
    }

    for (const auto& port : ports)
        drawPort(g, port);

    g.setColour(statusText.startsWith("Invalid") ? juce::Colour(0xffff9c8a) : juce::Colour(0xff9ea7b3));
    g.setFont(juce::FontOptions(11.5f));
    g.drawText(statusText, footer.toNearestInt(), juce::Justification::centredLeft);
}

void RouteGraphPanelComponent::mouseUp(const juce::MouseEvent& event)
{
    if (const auto* route = routeAt(event.position))
    {
        highlightedRouteId = route->routeId;
        selectedSourceEndpointId.clear();
        statusText = "Selected route " + route->routeId + ". Click it again to delete.";

        if (event.getNumberOfClicks() > 1 && onDeleteRoute)
        {
            statusText = "Deleting " + route->routeId;
            onDeleteRoute(route->routeId);
        }

        repaint();
        return;
    }

    const auto* clickedPort = portAt(event.position);
    if (clickedPort == nullptr)
        return;

    highlightedRouteId.clear();

    if (clickedPort->endpoint.direction == "output")
    {
        selectedSourceEndpointId = clickedPort->endpoint.id;
        statusText = "Selected source " + clickedPort->endpoint.displayName + ". Now click a compatible input.";
        repaint();
        return;
    }

    if (clickedPort->endpoint.direction == "input")
    {
        const auto* source = endpointForId(selectedSourceEndpointId);

        if (source == nullptr)
        {
            statusText = "Click an output port first, then this input.";
            repaint();
            return;
        }

        if (! canConnect(*source, clickedPort->endpoint))
        {
            statusText = "Invalid route: " + validationMessageFor(*source, clickedPort->endpoint);
            selectedSourceEndpointId.clear();
            repaint();
            return;
        }

        if (onCreateRoute)
        {
            statusText = "Creating " + source->family + " route.";
            onCreateRoute(source->family, source->id, clickedPort->endpoint.id, true);
        }

        selectedSourceEndpointId.clear();
        repaint();
    }
}

void RouteGraphPanelComponent::rebuildLayout()
{
    nodes.clear();
    ports.clear();

    auto graphArea = getLocalBounds().toFloat().reduced(16.0f);
    graphArea.removeFromTop(38.0f);
    graphArea.removeFromBottom(28.0f);

    juce::StringArray owners;
    for (const auto& endpoint : state.endpoints)
        owners.addIfNotAlreadyThere(endpoint.ownerId);

    const int nodeCount = owners.size();
    if (nodeCount <= 0)
        return;

    const int columns = 4;
    const int rows = juce::jmax(1, static_cast<int>(std::ceil(static_cast<double>(nodeCount) / static_cast<double>(columns))));
    const float gap = 10.0f;
    const float nodeWidth = (graphArea.getWidth() - (gap * static_cast<float>(columns - 1))) / static_cast<float>(columns);
    const float nodeHeight = juce::jlimit(72.0f, 118.0f, (graphArea.getHeight() - (gap * static_cast<float>(rows - 1))) / static_cast<float>(rows));

    for (int i = 0; i < nodeCount; ++i)
    {
        const int column = i % columns;
        const int row = i / columns;
        auto bounds = juce::Rectangle<float>(graphArea.getX() + (static_cast<float>(column) * (nodeWidth + gap)),
                                             graphArea.getY() + (static_cast<float>(row) * (nodeHeight + gap)),
                                             nodeWidth,
                                             nodeHeight);

        nodes.add({ owners[i], displayNameForOwner(owners[i]), bounds });
    }

    for (const auto& endpoint : state.endpoints)
    {
        for (const auto& node : nodes)
        {
            if (node.ownerId != endpoint.ownerId)
                continue;

            int sameDirectionIndex = 0;
            int sameDirectionCount = 0;

            for (const auto& candidate : state.endpoints)
            {
                if (candidate.ownerId == endpoint.ownerId && candidate.direction == endpoint.direction)
                {
                    if (candidate.id == endpoint.id)
                        sameDirectionIndex = sameDirectionCount;
                    ++sameDirectionCount;
                }
            }

            const auto portY = node.bounds.getY() + 32.0f + (static_cast<float>(sameDirectionIndex) * 18.0f);
            const bool isOutput = endpoint.direction == "output";
            auto centre = juce::Point<float>(isOutput ? node.bounds.getRight() - 12.0f : node.bounds.getX() + 12.0f,
                                             juce::jmin(portY, node.bounds.getBottom() - 14.0f));
            ports.add({ endpoint, { centre.x - 6.0f, centre.y - 6.0f, 12.0f, 12.0f }, centre });
            break;
        }
    }
}

void RouteGraphPanelComponent::drawRouteLine(juce::Graphics& g, const RouteEntry& route, bool highlighted)
{
    const auto* source = portForEndpoint(route.source);
    const auto* destination = portForEndpoint(route.destination);
    if (source == nullptr || destination == nullptr)
        return;

    auto colour = colourForFamily(route.family);
    if (! route.enabled)
        colour = colour.withAlpha(0.28f);
    else
        colour = colour.withAlpha(highlighted ? 0.95f : 0.62f);

    juce::Path path;
    path.startNewSubPath(source->centre);
    const auto controlOffset = juce::jmax(28.0f, std::abs(destination->centre.x - source->centre.x) * 0.35f);
    path.cubicTo(source->centre.translated(controlOffset, 0.0f),
                 destination->centre.translated(-controlOffset, 0.0f),
                 destination->centre);

    g.setColour(colour);
    g.strokePath(path, juce::PathStrokeType(highlighted ? 3.0f : 2.0f));
}

void RouteGraphPanelComponent::drawPort(juce::Graphics& g, const PortLayout& port)
{
    const bool isSelected = port.endpoint.id == selectedSourceEndpointId;
    const auto colour = colourForFamily(port.endpoint.family);

    g.setColour(isSelected ? colour.brighter(0.4f) : colour);
    g.fillEllipse(port.bounds);

    g.setColour(isSelected ? juce::Colour(0xffffffff) : juce::Colour(0xff11161c));
    g.drawEllipse(port.bounds, isSelected ? 2.0f : 1.0f);

    auto label = port.endpoint.displayName
        .replace(" Audio ", " ")
        .replace(" Control ", " ")
        .replace(" Structural ", " ")
        .replace(" Event ", " ")
        .replace(" Sync ", " ");

    auto labelBounds = juce::Rectangle<float>(port.endpoint.direction == "output" ? port.centre.x - 118.0f : port.centre.x + 10.0f,
                                              port.centre.y - 7.0f,
                                              106.0f,
                                              14.0f);

    g.setColour(juce::Colour(0xff9ea7b3));
    g.setFont(juce::FontOptions(9.5f));
    g.drawText(label, labelBounds.toNearestInt(),
               port.endpoint.direction == "output" ? juce::Justification::centredRight : juce::Justification::centredLeft);
}

void RouteGraphPanelComponent::drawFamilyLegend(juce::Graphics& g, juce::Rectangle<float> area)
{
    static const char* families[] = { "audio", "control", "event", "structural", "sync" };

    auto x = area.getRight() - 390.0f;
    for (auto* family : families)
    {
        auto colour = colourForFamily(family);
        g.setColour(colour);
        g.fillRoundedRectangle({ x, area.getY() + 8.0f, 8.0f, 8.0f }, 4.0f);
        g.setColour(juce::Colour(0xff9ea7b3));
        g.setFont(juce::FontOptions(10.5f));
        g.drawText(family, juce::Rectangle<float>(x + 12.0f, area.getY() + 3.0f, 64.0f, 18.0f).toNearestInt(), juce::Justification::centredLeft);
        x += static_cast<float>(juce::String(family).length()) * 7.0f + 30.0f;
    }
}

const RouteEndpointEntry* RouteGraphPanelComponent::endpointForId(const juce::String& endpointId) const
{
    for (const auto& endpoint : state.endpoints)
        if (endpoint.id == endpointId)
            return &endpoint;

    return nullptr;
}

const RouteGraphPanelComponent::PortLayout* RouteGraphPanelComponent::portAt(juce::Point<float> position) const
{
    for (const auto& port : ports)
        if (port.bounds.expanded(5.0f).contains(position))
            return &port;

    return nullptr;
}

const RouteGraphPanelComponent::PortLayout* RouteGraphPanelComponent::portForEndpoint(const juce::String& endpointId) const
{
    for (const auto& port : ports)
        if (port.endpoint.id == endpointId)
            return &port;

    return nullptr;
}

const RouteEntry* RouteGraphPanelComponent::routeAt(juce::Point<float> position) const
{
    for (const auto& route : state.routes)
    {
        const auto* source = portForEndpoint(route.source);
        const auto* destination = portForEndpoint(route.destination);
        if (source == nullptr || destination == nullptr)
            continue;

        auto lineBounds = juce::Rectangle<float>(source->centre, destination->centre).expanded(8.0f);
        if (lineBounds.contains(position))
            return &route;
    }

    return nullptr;
}

juce::Colour RouteGraphPanelComponent::colourForFamily(const juce::String& family) const
{
    if (family == "audio") return juce::Colour(0xff63d8ff);
    if (family == "control") return juce::Colour(0xffffc857);
    if (family == "event") return juce::Colour(0xffff8f70);
    if (family == "structural") return juce::Colour(0xffb7f06a);
    if (family == "sync") return juce::Colour(0xffb99cff);
    return juce::Colour(0xff9ea7b3);
}

juce::String RouteGraphPanelComponent::displayNameForOwner(const juce::String& ownerId) const
{
    return ownerTitle(ownerId);
}

bool RouteGraphPanelComponent::canConnect(const RouteEndpointEntry& source, const RouteEndpointEntry& destination) const
{
    return source.direction == "output"
        && destination.direction == "input"
        && source.family == destination.family;
}

juce::String RouteGraphPanelComponent::validationMessageFor(const RouteEndpointEntry& source, const RouteEndpointEntry& destination) const
{
    if (source.direction != "output")
        return "source is not an output";
    if (destination.direction != "input")
        return "destination is not an input";
    if (source.family != destination.family)
        return source.family + " cannot connect to " + destination.family;

    return "unknown route validation failure";
}
