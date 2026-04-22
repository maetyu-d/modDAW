(
var root;
root = PathName(thisProcess.nowExecutingPath).pathOnly;
(root +/+ "app" +/+ "Main.scd").load;
)
