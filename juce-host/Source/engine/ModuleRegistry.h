#pragma once

#include <JuceHeader.h>
#include "ClockDomainState.h"
#include "Module.h"
#include "Scheduler.h"

class ModuleRegistry
{
public:
    using StateCallback = std::function<void(const ModuleState&)>;

    ModuleRegistry();
    ~ModuleRegistry();

    void setStateCallback(StateCallback callback);
    void initialiseDemoModules();
    void reset();
    void setTransportState(const TransportState& transportState);
    void setClockDomainState(const ClockDomainState& state);
    ModuleState getState() const;

    bool scheduleActivation(const juce::String& moduleId, Scheduler::Policy policy);
    bool scheduleSurfaceUpdate(const juce::String& moduleId,
                               const juce::String& surfaceId,
                               const juce::String& codeText,
                               Scheduler::Policy policy);

private:
    void resolveScheduledAction(const Scheduler::Action& action);
    void emitStateChanged();
    Module* findModule(const juce::String& moduleId) const;

    mutable juce::CriticalSection lock;
    juce::OwnedArray<Module> modules;
    ModuleState state;
    TransportState transportState;
    ClockDomainState clockDomainState;
    Scheduler scheduler;
    StateCallback stateCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModuleRegistry)
};
