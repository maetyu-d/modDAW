#include "DemoToneModule.h"
#include "../engine/BehaviourParser.h"

namespace
{
CodeSurfaceEntry makeSurface(const juce::String& surfaceId,
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

DemoToneModule::DemoToneModule()
{
    state.id = "module.tone";
    state.displayName = "Tone Pulse";
    state.clockDomainId = "global.main";
    state.timingMode = "global-following";
    state.lifecycleState = "ready";
    state.behaviourType = "demo-tone";
    state.laneType = "module";
    state.capabilities.add("pattern");
    state.capabilities.add("lifecycle");
    state.capabilities.add("code-surface");
    state.codeSurfaces.add(makeSurface("pattern", "Pattern", "pattern", "steps: [1, 0, 1, 0]"));
    state.codeSurfaces.add(makeSurface("processor", "Processor", "processor", "frequency: 220\naccent: 0.85"));
    state.codeSurfaceState = "active";
    state.lastCodeEvalMessage = "native behaviour active";
    publishBehaviour();
}

ModuleEntry DemoToneModule::buildState() const
{
    return state;
}

juce::String DemoToneModule::moduleId() const
{
    return state.id;
}

juce::String DemoToneModule::clockDomainId() const
{
    return state.clockDomainId;
}

void DemoToneModule::markActivationQueued(const juce::String& boundaryLabel, int targetBarIndex)
{
    state.lifecycleState = "queued-next-bar";
    state.pendingActivationBarIndex = targetBarIndex;
    state.lastCodeEvalMessage = "activation queued for " + boundaryLabel;
}

void DemoToneModule::activateNow(const juce::String& boundaryLabel)
{
    state.lifecycleState = "active";
    state.pendingActivationBarIndex = 0;
    state.lastCodeEvalMessage = "activated on " + boundaryLabel;
}

Module::QueueResult DemoToneModule::queueSurfaceUpdate(const juce::String& surfaceId,
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

void DemoToneModule::applyQueuedSurfaceNow(const juce::String& surfaceId,
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

Module::QueueResult DemoToneModule::parseQueuedSurface(const juce::String& surfaceId,
                                                       const juce::String& codeText,
                                                       Behaviour& nextBehaviour,
                                                       juce::String& diagnostic) const
{
    QueueResult result;

    if (surfaceId == "pattern")
    {
        const auto parsed = BehaviourParser::parseBinarySteps(codeText);
        result.success = parsed.success;
        result.diagnostic = parsed.diagnostic;
        diagnostic = parsed.diagnostic;
        if (parsed.success)
            nextBehaviour.steps = parsed.steps;
        return result;
    }

    if (surfaceId == "processor")
    {
        const auto parsed = BehaviourParser::parseScalarBlock(codeText);
        result.success = parsed.success;
        result.diagnostic = parsed.diagnostic;
        diagnostic = parsed.diagnostic;
        if (parsed.success)
        {
            nextBehaviour.frequency = BehaviourParser::readScalar(parsed, "frequency", nextBehaviour.frequency);
            nextBehaviour.accent = juce::jlimit(0.1, 1.5, BehaviourParser::readScalar(parsed, "accent", nextBehaviour.accent));
        }
        return result;
    }

    result.success = false;
    result.diagnostic = "unsupported surface";
    diagnostic = result.diagnostic;
    return result;
}

void DemoToneModule::publishBehaviour()
{
    state.stepPattern = activeBehaviour.steps;
    state.baseFrequency = activeBehaviour.frequency;
    state.accent = activeBehaviour.accent;
    state.codeSurface = state.codeSurfaces.getReference(0).code;
}
