#include "ModuleRegistry.h"
#include "../modules/DemoToneModule.h"
#include "../modules/PatternModule.h"
#include "../modules/DroneModule.h"

ModuleRegistry::ModuleRegistry()
{
    scheduler.setResolveCallback([this](const Scheduler::Action& action)
    {
        resolveScheduledAction(action);
    });
}

ModuleRegistry::~ModuleRegistry() = default;

void ModuleRegistry::setStateCallback(StateCallback callback)
{
    const juce::ScopedLock scopedLock(lock);
    stateCallback = std::move(callback);
}

void ModuleRegistry::initialiseDemoModules()
{
    {
        const juce::ScopedLock scopedLock(lock);
        modules.clear(true);
        modules.add(new DemoToneModule());
        modules.add(new PatternModule());
        modules.add(new DroneModule());
    }

    emitStateChanged();
}

void ModuleRegistry::reset()
{
    const juce::ScopedLock scopedLock(lock);
    modules.clear(true);
    state = {};
    transportState = {};
    clockDomainState = {};
    scheduler.reset();
}

void ModuleRegistry::setTransportState(const TransportState& newTransportState)
{
    {
        const juce::ScopedLock scopedLock(lock);
        transportState = newTransportState;
    }

    scheduler.processTransportState(newTransportState);
    emitStateChanged();
}

void ModuleRegistry::setClockDomainState(const ClockDomainState& newState)
{
    {
        const juce::ScopedLock scopedLock(lock);
        clockDomainState = newState;
    }

    scheduler.setClockDomainState(newState);
}

ModuleState ModuleRegistry::getState() const
{
    const juce::ScopedLock scopedLock(lock);
    return state;
}

bool ModuleRegistry::scheduleActivation(const juce::String& moduleId, Scheduler::Policy policy)
{
    Scheduler::Action scheduledAction;
    bool resolveImmediately = false;

    {
        const juce::ScopedLock scopedLock(lock);
        if (auto* module = findModule(moduleId))
        {
            scheduledAction.kind = Scheduler::Action::Kind::activateModule;
            scheduledAction.moduleId = moduleId;
            scheduledAction.clockDomainId = module->clockDomainId();
            scheduledAction.policy = policy;
            scheduledAction = scheduler.schedule(scheduledAction, transportState);

            if (policy == Scheduler::Policy::immediate)
            {
                resolveImmediately = true;
            }
            else
            {
                module->markActivationQueued(scheduledAction.boundaryLabel, scheduledAction.targetBarIndex);
            }
        }
        else
            return false;
    }

    if (resolveImmediately)
        resolveScheduledAction(scheduledAction);
    else
        emitStateChanged();

    return true;
}

bool ModuleRegistry::scheduleSurfaceUpdate(const juce::String& moduleId,
                                           const juce::String& surfaceId,
                                           const juce::String& codeText,
                                           Scheduler::Policy policy)
{
    Scheduler::Action scheduledAction;
    bool resolveImmediately = false;

    {
        const juce::ScopedLock scopedLock(lock);
        if (auto* module = findModule(moduleId))
        {
            scheduledAction.kind = Scheduler::Action::Kind::applySurface;
            scheduledAction.moduleId = moduleId;
            scheduledAction.clockDomainId = module->clockDomainId();
            scheduledAction.surfaceId = surfaceId;
            scheduledAction.payload = codeText;
            scheduledAction.policy = policy;
            scheduledAction = scheduler.schedule(scheduledAction, transportState);

            const auto result = module->queueSurfaceUpdate(surfaceId, codeText,
                                                           scheduledAction.boundaryLabel,
                                                           scheduledAction.targetBarIndex);
            if (! result.success)
            {
                scheduler.cancel(scheduledAction.actionId);
                return false;
            }

            resolveImmediately = (policy == Scheduler::Policy::immediate);
        }
        else
            return false;
    }

    if (resolveImmediately)
        resolveScheduledAction(scheduledAction);
    else
        emitStateChanged();

    return true;
}

void ModuleRegistry::resolveScheduledAction(const Scheduler::Action& action)
{
    {
        const juce::ScopedLock scopedLock(lock);
        if (auto* module = findModule(action.moduleId))
        {
            switch (action.kind)
            {
                case Scheduler::Action::Kind::activateModule:
                    module->activateNow(action.boundaryLabel);
                    break;
                case Scheduler::Action::Kind::applySurface:
                    module->applyQueuedSurfaceNow(action.surfaceId, action.boundaryLabel);
                    break;
            }
        }
    }

    emitStateChanged();
}

void ModuleRegistry::emitStateChanged()
{
    StateCallback callbackCopy;
    ModuleState newState;

    {
        const juce::ScopedLock scopedLock(lock);
        state.modules.clear();
        for (auto* module : modules)
            state.modules.add(module->buildState());

        newState = state;
        callbackCopy = stateCallback;
    }

    if (callbackCopy)
        callbackCopy(newState);
}

Module* ModuleRegistry::findModule(const juce::String& moduleId) const
{
    for (auto* module : modules)
        if (module->moduleId() == moduleId)
            return module;

    return nullptr;
}
