
@0xf1797bcd9a00899b;

struct GitState
{
   repoName @0 :Text;
   sha1 @1 :Text;
   modified @2 :Bool;
}

struct TextLog
{
   text @0 :Text;
}


struct SoftwareLog
{
   file @0 :Text;
   linenum @1 :UInt32;
   code    @2 :Int32;
   explanation @3 :Text;
}

struct StateChange
{
   from @0 :Int16;
   to   @1 :Int16;
}


struct LogEntry
{
   level @0 :Int8;
   timeS @1 :Int64;
   timeNS @2 :Int64;

   union
   {
      gitState @3 :GitState;
      textLog  @4 :TextLog;
      userLog  @5 :TextLog;
      softwareDebug @6 :SoftwareLog;
      softwareDebug2 @7 :SoftwareLog;
      softwareInfo  @8 :SoftwareLog;
      softwareWarning @9 :SoftwareLog;
      softwareError   @10 :SoftwareLog;
      softwareCritical @11 :SoftwareLog;
      softwareFatal    @12 :SoftwareLog;
      loopClosed @13 :Void;
      loopPause @14 :Void;
      loopOpen  @15  :Void;
      stateChange @16 :StateChange;
   }
}

