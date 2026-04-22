(
var root;

root = PathName(thisProcess.nowExecutingPath).pathOnly;

(root +/+ "Protocol.scd").load;
(root +/+ "EngineState.scd").load;
(root +/+ "Transport.scd").load;
(root +/+ "ClockDomains.scd").load;
(root +/+ "DemoModules.scd").load;
(root +/+ "Validation.scd").load;
(root +/+ "Mixer.scd").load;

~engineState[\bootTime] = Date.getDate.stamp;
~engineState[\transport] = ~transportState;
~engineState[\clockDomains] = ~clockDomains;
~engineState[\modules] = ~demoModules;
~engineState[\validation] = ~validateEngineState.value;
~engineState[\mixer] = ~mixerState;
~startSocketLoop.value;
~startModuleActionLoop.value;
~bootAudioEngine.value;
~startTransportBroadcastLoop.value;
)
