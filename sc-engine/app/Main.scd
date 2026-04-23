(
var root;

root = PathName(thisProcess.nowExecutingPath).pathOnly;
~engineRoot = root +/+ "..";

(root +/+ "Protocol.scd").load;
(root +/+ "EngineState.scd").load;
(root +/+ "Transport.scd").load;
(root +/+ "ClockDomains.scd").load;
(root +/+ "DemoModules.scd").load;
(root +/+ "Validation.scd").load;
(root +/+ "Mixer.scd").load;
(root +/+ "Routes.scd").load;
(root +/+ "Arrangement.scd").load;
(root +/+ "Automation.scd").load;
(root +/+ "Structural.scd").load;
(root +/+ "Performance.scd").load;
(root +/+ "Analysis.scd").load;
(root +/+ "ProjectState.scd").load;
(root +/+ "Render.scd").load;

~installStructuralModule.value;
~installAnalysisModule.value;
~engineState[\bootTime] = Date.getDate.stamp;
~engineState[\transport] = ~transportState;
~engineState[\clockDomains] = ~clockDomains;
~engineState[\modules] = ~demoModules;
~engineState[\validation] = ~validateEngineState.value;
~engineState[\mixer] = ~mixerState;
~engineState[\routes] = ~routeState;
~engineState[\regions] = ~arrangementRegions;
~engineState[\automation] = ~automationLanes;
~engineState[\structural] = ~structuralDirectives;
~engineState[\performance] = ~makePerformancePayload.value;
~engineState[\analysis] = ~makeAnalysisPayload.value;
~engineState[\recovery] = ~makeRecoveryPayload.value;
~engineState[\render] = ~makeRenderPayload.value;
~transportStop.value;
~startSocketLoop.value;
~startModuleActionLoop.value;
~startArrangementPlaybackLoop.value;
~startAutomationPlaybackLoop.value;
~startStructuralLoop.value;
~bootAudioEngine.value;
~startTransportBroadcastLoop.value;
)
