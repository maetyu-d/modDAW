#include "RouteListPanelComponent.h"

namespace
{
juce::String selectorTextForEndpoint(const RouteEndpointEntry& endpoint)
{
    return endpoint.displayName + "  [" + endpoint.id + "]";
}
}

RouteListPanelComponent::RouteListPanelComponent()
{
    titleLabel.setText("Explicit Routes (sclang)", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    summaryLabel.setJustificationType(juce::Justification::centredRight);

    routeText.setMultiLine(true);
    routeText.setReadOnly(true);
    routeText.setScrollbarsShown(true);
    routeText.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff101419));
    routeText.setColour(juce::TextEditor::textColourId, juce::Colour(0xffd7e2ef));
    routeText.setFont(juce::FontOptions(12.0f));

    familySelector.addItem("audio", 1);
    familySelector.addItem("control", 2);
    familySelector.addItem("event", 3);
    familySelector.addItem("structural", 4);
    familySelector.addItem("sync", 5);
    familySelector.setSelectedId(1, juce::dontSendNotification);
    familySelector.addListener(this);

    createButton.addListener(this);
    deleteButton.addListener(this);

    addAndMakeVisible(titleLabel);
    addAndMakeVisible(summaryLabel);
    addAndMakeVisible(routeText);
    addAndMakeVisible(familySelector);
    addAndMakeVisible(sourceSelector);
    addAndMakeVisible(destinationSelector);
    addAndMakeVisible(createButton);
    addAndMakeVisible(deleteSelector);
    addAndMakeVisible(deleteButton);

    refreshText();
}

void RouteListPanelComponent::setRouteState(const RouteState& newState)
{
    auto snapshot = makeStateSnapshot(newState);
    if (snapshot == lastStateSnapshot)
        return;

    lastStateSnapshot = snapshot;
    state = newState;
    refreshEndpointSelectors();
    refreshDeleteSelector();
    refreshText();
    repaint();
}

void RouteListPanelComponent::resized()
{
    auto area = getLocalBounds().reduced(12);
    auto top = area.removeFromTop(24);
    titleLabel.setBounds(top.removeFromLeft(230));
    summaryLabel.setBounds(top);
    area.removeFromTop(8);

    auto createRow = area.removeFromTop(26);
    familySelector.setBounds(createRow.removeFromLeft(86));
    createRow.removeFromLeft(8);
    sourceSelector.setBounds(createRow.removeFromLeft(juce::jmax(150, createRow.getWidth() / 3)));
    createRow.removeFromLeft(8);
    destinationSelector.setBounds(createRow.removeFromLeft(juce::jmax(150, createRow.getWidth() / 2)));
    createRow.removeFromLeft(8);
    createButton.setBounds(createRow);
    area.removeFromTop(8);

    auto deleteRow = area.removeFromTop(26);
    deleteSelector.setBounds(deleteRow.removeFromLeft(juce::roundToInt(static_cast<float>(deleteRow.getWidth()) * 0.72f)));
    deleteRow.removeFromLeft(8);
    deleteButton.setBounds(deleteRow);
    area.removeFromTop(8);

    routeText.setBounds(area);
}

void RouteListPanelComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff171b20));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);

    g.setColour(juce::Colour(0xff2b333d));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 8.0f, 1.0f);
}

void RouteListPanelComponent::buttonClicked(juce::Button* button)
{
    if (button == &createButton)
    {
        if (onCreateRoute && sourceSelector.getSelectedId() > 0 && destinationSelector.getSelectedId() > 0)
            onCreateRoute(familySelector.getText(),
                          sourceSelector.getText().fromLastOccurrenceOf("[", false, false).upToFirstOccurrenceOf("]", false, false).trim(),
                          destinationSelector.getText().fromLastOccurrenceOf("[", false, false).upToFirstOccurrenceOf("]", false, false).trim(),
                          true);
        return;
    }

    if (button == &deleteButton)
    {
        if (onDeleteRoute && deleteSelector.getSelectedId() > 0)
            onDeleteRoute(deleteSelector.getText().upToFirstOccurrenceOf(" | ", false, false).trim());
    }
}

void RouteListPanelComponent::comboBoxChanged(juce::ComboBox* comboBox)
{
    if (comboBox == &familySelector)
        refreshEndpointSelectors();
}

void RouteListPanelComponent::refreshText()
{
    summaryLabel.setText(state.toSummaryString(), juce::dontSendNotification);

    if (state.routes.isEmpty())
    {
        routeText.setText("No routes defined.", juce::dontSendNotification);
        return;
    }

    juce::StringArray lines;
    for (const auto& route : state.routes)
        lines.add(route.toSummaryString());

    routeText.setText(lines.joinIntoString("\n"), juce::dontSendNotification);
}

void RouteListPanelComponent::refreshEndpointSelectors()
{
    populateEndpointSelector(sourceSelector, "output");
    populateEndpointSelector(destinationSelector, "input");
}

void RouteListPanelComponent::refreshDeleteSelector()
{
    auto previousText = deleteSelector.getText();
    deleteSelector.clear(juce::dontSendNotification);

    int itemId = 1;
    for (const auto& route : state.routes)
        deleteSelector.addItem(route.routeId + " | " + route.source + " -> " + route.destination, itemId++);

    if (previousText.isNotEmpty())
        deleteSelector.setText(previousText, juce::dontSendNotification);

    if (deleteSelector.getSelectedId() == 0 && deleteSelector.getNumItems() > 0)
        deleteSelector.setSelectedId(1, juce::dontSendNotification);
}

void RouteListPanelComponent::populateEndpointSelector(juce::ComboBox& selector, const juce::String& direction)
{
    auto previousText = selector.getText();
    selector.clear(juce::dontSendNotification);

    const auto family = familySelector.getText();
    int itemId = 1;

    for (const auto& endpoint : state.endpoints)
    {
        if (endpoint.family == family && endpoint.direction == direction)
            selector.addItem(selectorTextForEndpoint(endpoint), itemId++);
    }

    if (previousText.isNotEmpty())
        selector.setText(previousText, juce::dontSendNotification);

    if (selector.getSelectedId() == 0 && selector.getNumItems() > 0)
        selector.setSelectedId(1, juce::dontSendNotification);
}

juce::String RouteListPanelComponent::makeStateSnapshot(const RouteState& routeState) const
{
    juce::StringArray lines;

    for (const auto& endpoint : routeState.endpoints)
        lines.add("endpoint|" + endpoint.id + "|" + endpoint.family + "|" + endpoint.direction);

    for (const auto& route : routeState.routes)
        lines.add("route|" + route.routeId + "|" + route.family + "|" + route.source + "|"
                  + route.destination + "|" + (route.enabled ? "1" : "0"));

    return lines.joinIntoString("\n");
}
