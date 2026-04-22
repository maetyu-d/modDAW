#include "ModuleState.h"

namespace
{
juce::String readString(const juce::NamedValueSet& values, const juce::Identifier& id)
{
    if (auto* value = values.getVarPointer(id))
        return value->toString();

    return {};
}
}

juce::String ModuleEntry::toSummaryString() const
{
    return displayName + " | " + lifecycleState + " | " + clockDomainId + " | " + behaviourType;
}

juce::String ModuleEntry::currentEditableCodeSurface() const
{
    return pendingCodeSurface.isNotEmpty() ? pendingCodeSurface : codeSurface;
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
        entry.lifecycleState = readString(values, "lifecycleState");
        entry.behaviourType = readString(values, "behaviourType");
        entry.codeSurface = readString(values, "codeSurface");
        entry.pendingCodeSurface = readString(values, "pendingCodeSurface");
        entry.codeSurfaceState = readString(values, "codeSurfaceState");
        entry.lastCodeEvalMessage = readString(values, "lastCodeEvalMessage");
        entry.pendingCodeSwapBarIndex = static_cast<int>(values["pendingCodeSwapBarIndex"]);
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
