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
(root +/+ "ProjectState.scd").load;

~engineState[\bootTime] = Date.getDate.stamp;
~engineState[\transport] = ~transportState;
~engineState[\clockDomains] = ~clockDomains;
~engineState[\modules] = ~demoModules;
~engineState[\validation] = ~validateEngineState.value;
~engineState[\mixer] = ~mixerState;
~engineState[\routes] = ~routeState;
~engineState[\regions] = ~arrangementRegions;
~engineState[\automation] = ~automationLanes;
~startSocketLoop.value;
~startModuleActionLoop.value;
~startArrangementPlaybackLoop.value;
~startAutomationPlaybackLoop.value;
~bootAudioEngine.value;
~startTransportBroadcastLoop.value;
)
