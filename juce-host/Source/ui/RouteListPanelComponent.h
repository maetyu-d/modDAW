#pragma once

#include <JuceHeader.h>
#include "../engine/RouteState.h"

class RouteListPanelComponent final : public juce::Component,
                                      private juce::Button::Listener,
                                      private juce::ComboBox::Listener
{
public:
    RouteListPanelComponent();

    std::function<void(const juce::String& family,
                       const juce::String& source,
                       const juce::String& destination,
                       bool enabled)> onCreateRoute;
    std::function<void(const juce::String& routeId)> onDeleteRoute;

    void setRouteState(const RouteState& newState);

    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    void buttonClicked(juce::Button* button) override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;

    void refreshText();
    void refreshEndpointSelectors();
    void refreshDeleteSelector();
    void populateEndpointSelector(juce::ComboBox& selector, const juce::String& direction);
    juce::String makeStateSnapshot(const RouteState& routeState) const;

    juce::Label titleLabel;
    juce::Label summaryLabel;
    juce::TextEditor routeText;
    juce::ComboBox familySelector;
    juce::ComboBox sourceSelector;
    juce::ComboBox destinationSelector;
    juce::ComboBox deleteSelector;
    juce::TextButton createButton { "Create Route" };
    juce::TextButton deleteButton { "Delete" };
    RouteState state;
    juce::String lastStateSnapshot;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RouteListPanelComponent)
};
