#pragma once

#include <JuceHeader.h>
#include "AutomationState.h"
#include "ClockDomainState.h"
#include "MixerState.h"
#include "ModuleState.h"
#include "RegionState.h"
#include "RouteState.h"
#include "StructuralState.h"
#include "TransportState.h"

struct ProjectSnapshot
{
    juce::String schemaVersion = "moddaw.juce.project.1";
    juce::String savedAt;
    TransportState transport;
    ClockDomainState clockDomains;
    ModuleState modules;
    MixerState mixer;
    RouteState routes;
    RegionState regions;
    AutomationState automation;
    StructuralState structural;
};

class ProjectState
{
public:
    struct Result
    {
        bool success = false;
        juce::String path;
        juce::String message;
    };

    static juce::File defaultProjectFile();
    static Result save(const ProjectSnapshot& snapshot, const juce::File& file);
    static Result load(const juce::File& file, ProjectSnapshot& snapshot);
};
