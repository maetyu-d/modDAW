cmake_minimum_required(VERSION 3.22)

project(modular_sc_daw VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (DEFINED JUCE_SOURCE_DIR AND EXISTS "${JUCE_SOURCE_DIR}/CMakeLists.txt")
    add_subdirectory("${JUCE_SOURCE_DIR}" JUCE)
elseif (DEFINED ENV{JUCE_SOURCE_DIR} AND EXISTS "$ENV{JUCE_SOURCE_DIR}/CMakeLists.txt")
    add_subdirectory("$ENV{JUCE_SOURCE_DIR}" JUCE)
else()
    message(FATAL_ERROR "Set JUCE_SOURCE_DIR to a JUCE checkout with CMake support.")
endif()

juce_add_gui_app(modular_sc_daw
    PRODUCT_NAME "Modular SC DAW"
    COMPANY_NAME "Prototype"
)

juce_generate_juce_header(modular_sc_daw)

target_sources(modular_sc_daw PRIVATE
    Source/Main.cpp
    Source/App.h
    Source/App.cpp
    Source/MainWindow.h
    Source/MainWindow.cpp
    Source/ui/MainComponent.h
    Source/ui/MainComponent.cpp
    Source/ui/AutomationPanelComponent.h
    Source/ui/AutomationPanelComponent.cpp
    Source/ui/GlobalRulerComponent.h
    Source/ui/GlobalRulerComponent.cpp
    Source/ui/StatusBarComponent.h
    Source/ui/StatusBarComponent.cpp
    Source/ui/LogPanelComponent.h
    Source/ui/LogPanelComponent.cpp
    Source/ui/MixerPanelComponent.h
    Source/ui/MixerPanelComponent.cpp
    Source/ui/RouteGraphPanelComponent.h
    Source/ui/RouteGraphPanelComponent.cpp
    Source/ui/RouteListPanelComponent.h
    Source/ui/RouteListPanelComponent.cpp
    Source/ui/CodeSurfaceComponent.h
    Source/ui/CodeSurfaceComponent.cpp
    Source/ui/TimingInspectorComponent.h
    Source/ui/TimingInspectorComponent.cpp
    Source/engine/EngineProcessManager.h
    Source/engine/EngineProcessManager.cpp
    Source/engine/AutomationState.h
    Source/engine/AutomationState.cpp
    Source/engine/EngineConnection.h
    Source/engine/EngineConnection.cpp
    Source/engine/AnalysisState.h
    Source/engine/AnalysisState.cpp
    Source/engine/AudioEngine.h
    Source/engine/AudioEngine.cpp
    Source/engine/BehaviourParser.h
    Source/engine/BehaviourParser.cpp
    Source/engine/MessageEnvelope.h
    Source/engine/MessageEnvelope.cpp
    Source/engine/MessageParser.h
    Source/engine/MessageParser.cpp
    Source/engine/ClockDomainState.h
    Source/engine/ClockDomainState.cpp
    Source/engine/ClockDomain.h
    Source/engine/ClockDomain.cpp
    Source/engine/ClockDomainManager.h
    Source/engine/ClockDomainManager.cpp
    Source/engine/MixerEngine.h
    Source/engine/MixerEngine.cpp
    Source/engine/MixerState.h
    Source/engine/MixerState.cpp
    Source/engine/RoutingGraph.h
    Source/engine/RoutingGraph.cpp
    Source/engine/RouteState.h
    Source/engine/RouteState.cpp
    Source/engine/StructuralState.h
    Source/engine/StructuralState.cpp
    Source/engine/RegionState.h
    Source/engine/RegionState.cpp
    Source/engine/RecoveryState.h
    Source/engine/RecoveryState.cpp
    Source/engine/RenderState.h
    Source/engine/RenderState.cpp
    Source/engine/Scheduler.h
    Source/engine/Scheduler.cpp
    Source/engine/Module.h
    Source/engine/ModuleRegistry.h
    Source/engine/ModuleRegistry.cpp
    Source/engine/ModuleState.h
    Source/engine/ModuleState.cpp
    Source/engine/SubprocessHandle.h
    Source/engine/SubprocessHandle.cpp
    Source/engine/TransportState.h
    Source/engine/TransportState.cpp
    Source/engine/TransportEngine.h
    Source/engine/TransportEngine.cpp
    Source/engine/ValidationState.h
    Source/engine/ValidationState.cpp
    Source/util/Ids.h
    Source/util/TimeUtils.h
    Source/ui/ClockDomainsPanelComponent.h
    Source/ui/ClockDomainsPanelComponent.cpp
    Source/ui/ModuleLanesComponent.h
    Source/ui/ModuleLanesComponent.cpp
    Source/ui/ModulesPanelComponent.h
    Source/ui/ModulesPanelComponent.cpp
    Source/ui/TransportPanelComponent.h
    Source/ui/TransportPanelComponent.cpp
    Source/ui/ValidationPanelComponent.h
    Source/ui/ValidationPanelComponent.cpp
    Source/modules/DemoToneModule.h
    Source/modules/DemoToneModule.cpp
    Source/modules/PatternModule.h
    Source/modules/PatternModule.cpp
    Source/modules/DroneModule.h
    Source/modules/DroneModule.cpp
)

target_compile_definitions(modular_sc_daw PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    MODULAR_SC_DAW_ROOT=\"${CMAKE_CURRENT_SOURCE_DIR}/..\"
)

target_link_libraries(modular_sc_daw PRIVATE
    juce::juce_gui_extra
    juce::juce_audio_utils
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
