#include "DroneModule.h"
#include "../engine/BehaviourParser.h"

namespace
{
CodeSurfaceEntry makeDroneSurface(const juce::String& surfaceId,
                                  const juce::String& displayName,
                                  const juce::String& role,
                                  const juce::String& code)
{
    CodeSurfaceEntry surface;
    surface.surfaceId = surfaceId;
    surface.displayName = displayName;
    surface.role = role;
    surface.code = code;
    surface.state = "active";
    surface.diagnostic = "JUCE native behaviour";
    return surface;
}
}

DroneModule::DroneModule()
{
    state.id = "module.drone";
    state.displayName = "Free Drone";
    state.clockDomainId = "domain.free";
    state.timingMode = "free-running";
    state.lifecycleState = "active";
    state.behaviourType = "drone";
    state.laneType = "module";
    state.capabilities.add("processor");
    state.capabilities.add("free-running");
    state.capabilities.add("code-surface");
    state.codeSurfaces.add(makeDroneSurface("pattern", "Pattern", "pattern", "density: 0.25"));
    state.codeSurfaces.add(makeDroneSurface("processor", "Processor", "processor", "frequency: 82.41\nspread: 1.5"));
    state.codeSurfaceState = "active";
    state.lastCodeEvalMessage = "native behaviour active";
    publishBehaviour();
}

ModuleEntry DroneModule::buildState() const
{
    return state;
}

juce::String DroneModule::moduleId() const
{
    return state.id;
}

juce::String DroneModule::clockDomainId() const
{
    return state.clockDomainId;
}

void DroneModule::markActivationQueued(const juce::String& boundaryLabel, int targetBarIndex)
{
    state.lifecycleState = "queued-next-bar";
    state.pendingActivationBarIndex = targetBarIndex;
    state.lastCodeEvalMessage = "activation queued for " + boundaryLabel;
}

void DroneModule::activateNow(const juce::String& boundaryLabel)
{
    state.lifecycleState = "active";
    state.pendingActivationBarIndex = 0;
    state.lastCodeEvalMessage = "activated on " + boundaryLabel;
}

Module::QueueResult DroneModule::queueSurfaceUpdate(const juce::String& surfaceId,
                                                    const juce::String& codeText,
                                                    const juce::String& boundaryLabel,
                                                    int targetBarIndex)
{
    Behaviour nextBehaviour = activeBehaviour;
    juce::String diagnostic;
    auto result = parseQueuedSurface(surfaceId, codeText, nextBehaviour, diagnostic);

    for (auto& surface : state.codeSurfaces)
    {
        if (surface.surfaceId != surfaceId)
            continue;

        if (! result.success)
        {
            surface.state = "error";
            surface.diagnostic = diagnostic;
            state.codeSurfaceState = "error";
            state.lastCodeEvalMessage = diagnostic;
            return result;
        }

        pendingBehaviour = nextBehaviour;
        surface.pendingCode = codeText;
        surface.pendingCodeSwapBarIndex = targetBarIndex;
        surface.state = "queued-next-bar";
        surface.diagnostic = "apply on " + boundaryLabel;
        state.pendingCodeSurface = codeText;
        state.pendingCodeSwapBarIndex = targetBarIndex;
        state.codeSurfaceState = "queued-next-bar";
        state.lastCodeEvalMessage = "queued for " + boundaryLabel;
        return result;
    }

    result.success = false;
    result.diagnostic = "unknown surface";
    return result;
}

void DroneModule::applyQueuedSurfaceNow(const juce::String& surfaceId,
                                        const juce::String& boundaryLabel)
{
    for (auto& surface : state.codeSurfaces)
    {
        if (surface.surfaceId != surfaceId)
            continue;

        if (surface.pendingCode.isNotEmpty())
            surface.code = surface.pendingCode;

        surface.pendingCode.clear();
        surface.pendingCodeSwapBarIndex = 0;
        surface.state = "active";
        surface.diagnostic = "applied on " + boundaryLabel;
        ++surface.revision;
        activeBehaviour = pendingBehaviour;
        state.pendingCodeSurface.clear();
        state.pendingCodeSwapBarIndex = 0;
        state.codeSurfaceState = "active";
        state.lastCodeEvalMessage = "applied on " + boundaryLabel;
        publishBehaviour();
        return;
    }
}

Module::QueueResult DroneModule::parseQueuedSurface(const juce::String& surfaceId,
                                                    const juce::String& codeText,
                                                    Behaviour& nextBehaviour,
                                                    juce::String& diagnostic) const
{
    QueueResult result;
    const auto parsed = BehaviourParser::parseScalarBlock(codeText);
    result.success = parsed.success;
    result.diagnostic = parsed.diagnostic;
    diagnostic = parsed.diagnostic;

    if (! parsed.success)
        return result;

    if (surfaceId == "pattern")
    {
        nextBehaviour.density = juce::jlimit(0.0, 1.0, BehaviourParser::readScalar(parsed, "density", nextBehaviour.density));
        return result;
    }

    if (surfaceId == "processor")
    {
        nextBehaviour.frequency = juce::jmax(20.0, BehaviourParser::readScalar(parsed, "frequency", nextBehaviour.frequency));
        nextBehaviour.spread = juce::jlimit(1.0, 3.0, BehaviourParser::readScalar(parsed, "spread", nextBehaviour.spread));
        return result;
    }

    result.success = false;
    result.diagnostic = "unsupported surface";
    diagnostic = result.diagnostic;
    return result;
}

void DroneModule::publishBehaviour()
{
    state.density = activeBehaviour.density;
    state.baseFrequency = activeBehaviour.frequency;
    state.spread = activeBehaviour.spread;
    state.codeSurface = state.codeSurfaces.getReference(0).code;
}
