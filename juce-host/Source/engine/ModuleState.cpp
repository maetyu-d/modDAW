#include "ModuleState.h"

namespace
{
juce::String readString(const juce::NamedValueSet& values, const juce::Identifier& id)
{
    if (auto* value = values.getVarPointer(id))
        return value->toString();

    return {};
}

int readInt(const juce::NamedValueSet& values, const juce::Identifier& id, int fallback)
{
    if (auto* value = values.getVarPointer(id))
        return value->isVoid() ? fallback : static_cast<int>(*value);

    return fallback;
}

CodeSurfaceEntry parseSurface(const juce::var& item)
{
    CodeSurfaceEntry surface;

    if (auto* object = item.getDynamicObject())
    {
        const auto& values = object->getProperties();
        surface.surfaceId = readString(values, "surfaceId");
        surface.displayName = readString(values, "displayName");
        surface.role = readString(values, "role");
        surface.code = readString(values, "code");
        surface.pendingCode = readString(values, "pendingCode");
        surface.state = readString(values, "state");
        surface.diagnostic = readString(values, "diagnostic");
        surface.revision = readInt(values, "revision", 1);
        surface.pendingCodeSwapBarIndex = readInt(values, "pendingCodeSwapBarIndex", 0);
    }

    return surface;
}
}

juce::String CodeSurfaceEntry::currentEditableCode() const
{
    return pendingCode.isNotEmpty() ? pendingCode : code;
}

juce::String ModuleEntry::toSummaryString() const
{
    return displayName + " | " + lifecycleState + " | " + clockDomainId + " | " + timingMode + " | " + behaviourType;
}

juce::String ModuleEntry::currentEditableCodeSurface() const
{
    if (const auto* surface = defaultSurface())
        return surface->currentEditableCode();

    return pendingCodeSurface.isNotEmpty() ? pendingCodeSurface : codeSurface;
}

const CodeSurfaceEntry* ModuleEntry::findSurfaceById(const juce::String& surfaceId) const
{
    for (const auto& surface : codeSurfaces)
        if (surface.surfaceId == surfaceId)
            return &surface;

    return nullptr;
}

const CodeSurfaceEntry* ModuleEntry::defaultSurface() const
{
    if (const auto* pattern = findSurfaceById("pattern"))
        return pattern;

    return codeSurfaces.isEmpty() ? nullptr : &codeSurfaces.getReference(0);
}

ModuleState ModuleState::fromPayload(const juce::var& payload)
{
    ModuleState state;
    auto* object = payload.getDynamicObject();

    if (object == nullptr)
        return state;

    auto modulesVar = object->getProperty("modules");
    if (! modulesVar.isArray())
        return state;

    for (const auto& item : *modulesVar.getArray())
    {
        auto* moduleObject = item.getDynamicObject();

        if (moduleObject == nullptr)
            continue;

        const auto& values = moduleObject->getProperties();
        ModuleEntry entry;
        entry.id = readString(values, "id");
        entry.displayName = readString(values, "displayName");
        entry.clockDomainId = readString(values, "clockDomainId");
        entry.timingMode = readString(values, "timingMode");
        entry.lifecycleState = readString(values, "lifecycleState");
        entry.behaviourType = readString(values, "behaviourType");
        entry.laneType = readString(values, "laneType");
        entry.lastStructuralDirective = readString(values, "lastStructuralDirective");
        entry.analysisSummary = readString(values, "analysisSummary");
        entry.lastAnalysisInfluence = readString(values, "lastAnalysisInfluence");
        entry.codeSurface = readString(values, "codeSurface");
        entry.pendingCodeSurface = readString(values, "pendingCodeSurface");
        entry.codeSurfaceState = readString(values, "codeSurfaceState");
        entry.lastCodeEvalMessage = readString(values, "lastCodeEvalMessage");
        entry.pendingActivationBarIndex = readInt(values, "pendingActivationBarIndex", 0);
        entry.pendingCodeSwapBarIndex = readInt(values, "pendingCodeSwapBarIndex", 0);
        entry.baseFrequency = values["baseFrequency"];
        entry.accent = values["accent"];
        entry.density = values["density"];
        entry.spread = values["spread"];

        if (auto* steps = values["stepPattern"].getArray())
        {
            for (const auto& step : *steps)
                entry.stepPattern.add(static_cast<int>(step));
        }

        if (auto* capabilities = values["capabilities"].getArray())
        {
            for (const auto& capability : *capabilities)
                entry.capabilities.add(capability.toString());
        }

        if (auto* surfaces = values["codeSurfaces"].getArray())
        {
            for (const auto& surfaceItem : *surfaces)
                entry.codeSurfaces.add(parseSurface(surfaceItem));
        }

        state.modules.add(entry);
    }

    return state;
}

juce::String ModuleState::toSummaryString() const
{
    return juce::String(modules.size()) + " modules";
}

const ModuleEntry* ModuleState::findById(const juce::String& id) const
{
    for (const auto& module : modules)
        if (module.id == id)
            return &module;

    return nullptr;
}
