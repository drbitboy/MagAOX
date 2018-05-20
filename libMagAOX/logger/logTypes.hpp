/** \file logTypes.hpp 
  * \brief The MagAO-X logger log types.
  * \author Jared R. Males (jaredmales@gmail.com)
  *
  * History:
  * - 2017-06-27 created by JRM
  */ 
#ifndef logger_logTypes_hpp
#define logger_logTypes_hpp


#include "../time/timespecX.hpp"
#include "../app/stateCodes.hpp"

#include "capnp/logEntry.capnp.h"
#include <capnp/message.h>
#include <capnp/serialize-packed.h>

#include "logLevels.hpp"
#include "logTypesBasics.hpp"


namespace MagAOX
{
namespace logger 
{

typedef uint16_t eventCodeT;

typedef ::capnp::MallocMessageBuilder builderT;

typedef ::capnp::FlatArrayMessageReader readerT;

/// The basic log entry type.
/** \ingroup logtypes
  *
  */ 
struct log_entry
{
   /// Serialize a log entry
   /** Uses cap'n proto generated structs to serialize the log level, timespec, and the log entry message.
     * The log entry message serialization is handled by the approprate log type.
     *
     * \returns 0 on success.
     * \returns \<0 on error.
     * 
     * \tparam logT is the log entry type which handles the messaage serialization.
     * 
     */
   template <typename logT>
   static int serialize( builderT & builder,         ///< [out] The message builder to hold the serialized log.
                         const logLevelT & lvl,      ///< [in] The log level.
                         const time::timespecX & ts, ///< [in] The time stamp of the log.
                         const logT & msg            ///< [in] The log-type specific message to log.
                       )
   {
      LogEntry::Builder logEntry = builder.initRoot<LogEntry>();
      
      logEntry.setLevel(lvl);
      logEntry.setTimeS(ts.time_s);
      logEntry.setTimeNS(ts.time_ns);
      
      return logT::serialize( logEntry, msg);
   }
   
   /// Get the event code from a log message buffer
   /** The event code, which corresponds to an enum in the cap'n proto LogEntry structure, identifies the log entry uniquely.
     * 
     * \returns the event code.
     */ 
   static eventCodeT eventCode( readerT & reader /**< [in] The message reader holding the serialized log.*/ )
   {
      LogEntry::Reader logEntry = reader.getRoot<LogEntry>();
      return logEntry.which();
   }
   
   /// Get the log level from a log message buffer
   /**
     * \returns the log level.
     */ 
   static logLevelT logLevel( readerT & reader /**< [in] The message reader holding the serialized log.*/ )
   {
      LogEntry::Reader logEntry = reader.getRoot<LogEntry>();
      return logEntry.getLevel();
   }
   
   /// Get the timestamp of the log entry.
   /**
     * \returns the timestamp as a time::timespecX.
     */ 
   static time::timespecX timestamp( readerT & reader  /**< [in] The message reader holding the serialized log.*/ )
   {
      LogEntry::Reader logEntry = reader.getRoot<LogEntry>();
      
      time::timespecX ts;
      
      ts.time_s = logEntry.getTimeS();
      ts.time_ns = logEntry.getTimeNS();
      
      return ts;
   }
   
   /// Unserialze the log entry message
   /**
     * \returns 0 on sucess
     * \returns \<0 on error
     *
     * \tparam logT is the log type
     */ 
   template <typename logT>
   static int unserialize( logT & msg,      ///< [out] The log-type specific message from the log
                           readerT & reader ///< [in] The message reader holding the serialized log.
                         )
   {
      LogEntry::Reader logEntry = reader.getRoot<LogEntry>();
      
      return logT::unserialize( msg, logEntry );
   }
}; //log_type
   

/// Log entry recording the build-time git state.
/** \ingroup logtypes
  */
struct git_state 
{
   ///The default level 
   static const logLevelT defaultLevel = logLevels::INFO;
   
   std::string m_repoName;  ///< The name of the repository             
   std::string m_sha1; ///< The sha1 hash of the latest commit
   bool m_modified {0}; ///< Whether or not the repositor has been modified since the last commit
  
   /// Serialize the git state into the cap'n proto buffer.
   /**
     * \returns 0 on success
     * \returns \<0 on error
     */  
   static int serialize( LogEntry::Builder & logEntry, ///< [out] The LogEntry holding the serialized log.  Initialized from builderT.
                         const git_state & msg         ///< [in] The git_state to log.
                       )
   {
      logEntry.setGitState(logEntry.getGitState());
      logEntry.getGitState().setRepoName(msg.m_repoName);
      logEntry.getGitState().setSha1(msg.m_sha1);
      logEntry.getGitState().setModified(msg.m_modified);
      return 0;
   }
   
   /// Un-serialize the cap'n proto buffer to read the git state
   /**
     * \returns 0 on success
     * \returns \<0 on error
     */
   static int unserialize( git_state & msg,            ///< [out] The git_state from the log.
                           LogEntry::Reader & logEntry ///< [in] The LogEntry holding the serialized log.
                         )
   {
      GitState::Reader gitState = logEntry.getGitState();
      
      msg.m_repoName = gitState.getRepoName();
      msg.m_sha1 = gitState.getSha1();
      msg.m_modified = gitState.getModified();
      
      return 0;
   }

   /// Un-serialize the cap'n proto buffer to read the repo name
   /** This particular function is offered to support finding the point of a restart by looking for "MAGAOX".
     * 
     * \returns 0 on success
     * \returns \<0 on error
     */
   static std::string repoName( readerT & reader /**< [in] The message reader holding the serialized log.*/ )
   {
      GitState::Reader gitState = reader.getRoot<LogEntry>().getGitState();
      
      return std::string(gitState.getRepoName());
   }
   
   /// Produce the standard formatted string for this log type.
   /** 
     * \returns 0 on success
     * \returns \<0 on error
     */
   static std::string msgString( git_state & msg /**< [out] The git_state to format.*/)
   {
      std::string str = msg.m_repoName + " GIT: ";
      str += msg.m_sha1;
      if(msg.m_modified) str += " MODIFIED";
      
      return str;
   }
   
}; //git_state 


///A simple text log.
/** \ingroup logtypes
  */
struct text_log 
{
   ///The default level 
   static const logLevelT defaultLevel = logLevels::INFO;
   
   ///This log entry consists of unformatted text.
   std::string m_text;               
  
   /// Serialize the text log into the cap'n proto buffer.
   /**
     * \returns 0 on success
     * \returns \<0 on error
     */
   static int serialize( LogEntry::Builder & logEntry, ///< [out] The LogEntry holding the serialized log.  Initialized from builderT.
                         const text_log  & msg         ///< [in] The text_log to log.
                       )
   {
      logEntry.setTextLog(logEntry.getTextLog());      
      logEntry.getTextLog().setText(msg.m_text);
      
      return 0;
   }
   
   /// Un-serialize the cap'n proto buffer to read the text log
   /**
     * \returns 0 on success
     * \returns \<0 on error
     */
   static int unserialize( text_log & msg,             ///< [out] The text_log from the log.
                           LogEntry::Reader & logEntry ///< [in] The LogEntry holding the serialized log.
                         )
   {
      TextLog::Reader textLog = logEntry.getTextLog();
      msg.m_text = textLog.getText();
      
      return 0;
   }

   /// Produce the standard formatted string for this log type.
   /** 
     * \returns 0 on success
     * \returns \<0 on error
     */
   static std::string msgString( text_log & msg )
   {
      return msg.m_text;
   }
   
};

///A simple text log, entered by the user.
/** \ingroup logtypes
  */
struct user_log
{
   ///The default level 
   static const logLevelT defaultLevel = logLevels::INFO;
   
   ///This log entry consists of unformatted text.
   std::string m_text;               
  
   /// Serialize the user log into the cap'n proto buffer.
   /**
     * \returns 0 on success
     * \returns \<0 on error
     */
   static int serialize( LogEntry::Builder & logEntry, ///< [out] The LogEntry holding the serialized log.  Initialized from builderT.
                         const user_log  & msg         ///< [in] The user_log to log.
                       )
   {
      logEntry.setUserLog(logEntry.getUserLog());      
      logEntry.getUserLog().setText(msg.m_text);
      
      return 0;
   }
   
   /// Un-serialize the cap'n proto buffer to read the user log
   /**
     * \returns 0 on success
     * \returns \<0 on error
     */
   static int unserialize( user_log & msg,             ///< [out] The user_log from the log.
                           LogEntry::Reader & logEntry ///< [in] The LogEntry holding the serialized log.
                         )
   {
      TextLog::Reader userLog = logEntry.getUserLog();
      msg.m_text = userLog.getText();
      
      return 0;
   }

   /// Produce the standard formatted string for this log type.
   /** The format is 
       \verbatim
         USER: text...
       \endverbatim
     *
     * \returns 0 on success
     * \returns \<0 on error
     */
   static std::string msgString( user_log & msg )
   {
      return "USER: " + msg.m_text;
   }
   
};


///A software debug log
/** \ingroup logtypes
  */
struct software_debug
{
   ///The default level 
   static const logLevelT defaultLevel = logLevels::DEBUG;
   
   std::string m_file;
   uint32_t m_linenum;
   int32_t m_code;
   std::string m_explanation;
   
   /// Serialize the software debug log into the cap'n proto buffer.
   /**
     * \returns 0 on success
     * \returns \<0 on error
     */
   static int serialize( LogEntry::Builder & logEntry, ///< [out] The LogEntry holding the serialized log.  Initialized from builderT.
                         const software_debug  & msg   ///< [in] The user_log to log.
                       )
   {
      logEntry.setSoftwareDebug(logEntry.getSoftwareDebug()); 
      return software_log::serialize_software_log( logEntry.getSoftwareDebug(), msg);
   }
   
   /// Un-serialize the cap'n proto buffer to read the user log
   /**
     * \returns 0 on success
     * \returns \<0 on error
     */
   static int unserialize( software_debug & msg,             ///< [out] The user_log from the log.
                           LogEntry::Reader & logEntry ///< [in] The LogEntry holding the serialized log.
                         )
   {
      return software_log::unserialize_software_log(msg, logEntry.getSoftwareDebug());
   }

   /// Produce the standard formatted string for this log type.
   /** The format is 
       \verbatim
         USER: text...
       \endverbatim
     *
     * \returns 0 on success
     * \returns \<0 on error
     */
   static std::string msgString( software_debug & msg )
   {
      return software_log::msgString(msg);      
   }
   
};


///A software debug2 log
/** \ingroup logtypes
  */
struct software_debug2
{
   ///The default level 
   static const logLevelT defaultLevel = logLevels::DEBUG2;
   
   std::string m_file;
   uint32_t m_linenum;
   int32_t m_code;
   std::string m_explanation;
   
   /// Serialize the software debug log into the cap'n proto buffer.
   /**
     * \returns 0 on success
     * \returns \<0 on error
     */
   static int serialize( LogEntry::Builder & logEntry, ///< [out] The LogEntry holding the serialized log.  Initialized from builderT.
                         const software_debug2  & msg   ///< [in] The user_log to log.
                       )
   {
      logEntry.setSoftwareDebug2(logEntry.getSoftwareDebug2()); 
      return software_log::serialize_software_log( logEntry.getSoftwareDebug2(), msg);
   }
   
   /// Un-serialize the cap'n proto buffer to read the user log
   /**
     * \returns 0 on success
     * \returns \<0 on error
     */
   static int unserialize( software_debug2 & msg,             ///< [out] The user_log from the log.
                           LogEntry::Reader & logEntry ///< [in] The LogEntry holding the serialized log.
                         )
   {
      return software_log::unserialize_software_log(msg, logEntry.getSoftwareDebug2());
   }

   /// Produce the standard formatted string for this log type.
   /** The format is 
       \verbatim
         USER: text...
       \endverbatim
     *
     * \returns 0 on success
     * \returns \<0 on error
     */
   static std::string msgString( software_debug2 & msg )
   {
      return software_log::msgString(msg);      
   }
   
};

#if 0



///Software DEBUG log entry
/** \ingroup logtypes
  */
struct software_debug : public software_log
{
   //Define the log name for use in the database
   //Event: "Software Debug"
   
   ///The event code
   static const eventCodeT eventCode = eventCodes::SOFTWARE_DEBUG;

   ///The default level 
   static const logLevelT defaultLevel = logLevels::DEBUG;   
};

///Software DEBUG2 log entry
/** \ingroup logtypes
  */
struct software_debug2 : public software_log
{
   //Define the log name for use in the database
   //Event: "Software Debug2"
   
   ///The event code
   static const eventCodeT eventCode = eventCodes::SOFTWARE_DEBUG2;

   ///The default level 
   static const logLevelT defaultLevel = logLevels::DEBUG2;   
};

///Software INFO log entry
/** \ingroup logtypes
  */
struct software_info : public software_log
{
   //Define the log name for use in the database
   //Event: "Software Info"
   
   ///The event code
   static const eventCodeT eventCode = eventCodes::SOFTWARE_INFO;

   ///The default level 
   static const logLevelT defaultLevel = logLevels::INFO;   
};

///Software WARN log entry
/** \ingroup logtypes
  */
struct software_warning : public software_log
{
   //Define the log name for use in the database
   //Event: "Software Warning"
   
   ///The event code
   static const eventCodeT eventCode = eventCodes::SOFTWARE_WARNING;

   ///The default level 
   static const logLevelT defaultLevel = logLevels::WARNING;   
};

///Software ERR log entry
/** \ingroup logtypes
  */
struct software_error : public software_log
{
   //Define the log name for use in the database
   //Event: "Software Error"
   
   ///The event code
   static const eventCodeT eventCode = eventCodes::SOFTWARE_ERROR;

   ///The default level 
   static const logLevelT defaultLevel = logLevels::ERROR;   
};

///Software CRIT log entry
/** \ingroup logtypes
  */
struct software_critical : public software_log
{
   //Define the log name for use in the database
   //Event: "Software Critical"
   
   ///The event code
   static const eventCodeT eventCode = eventCodes::SOFTWARE_CRITICAL;

   ///The default level 
   static const logLevelT defaultLevel = logLevels::CRITICAL;   
};

///Software FATAL log entry
/** \ingroup logtypes
  */
struct software_fatal : public software_log
{
   //Define the log name for use in the database
   //Event: "Software Fatal"
   
   ///The event code
   static const eventCodeT eventCode = eventCodes::SOFTWARE_FATAL;

   ///The default level 
   static const logLevelT defaultLevel = logLevels::FATAL;   
};


///Loop Closed event log
/** \ingroup logtypes
  */
struct loop_closed : public empty_log
{
   //Define the log name for use in the database
   //Event: "Loop Closed"
   
   ///The event code
   static const eventCodeT eventCode = eventCodes::LOOP_CLOSED;

   ///The default level 
   static const logLevelT defaultLevel = logLevels::INFO;   
   
   static std::string msgString( messageT & msg  /**< [in] [unused] the empty message */ )
   {
      return "LOOP CLOSED";
   }
};

///Loop Paused event log
/** \ingroup logtypes
  */
struct loop_paused : public empty_log
{
   //Define the log name for use in the database
   //Event: "Loop Paused"
   
   ///The event code
   static const eventCodeT eventCode = eventCodes::LOOP_PAUSED;
   
   ///The default level 
   static const logLevelT defaultLevel = logLevels::INFO;   
   
   static std::string msgString( messageT & msg  /**< [in] [unused] the empty message */)
   {
      return "LOOP PAUSED";
   }
};

///Loop Open event log
/** \ingroup logtypes
  */
struct loop_open : public empty_log
{
   //Define the log name for use in the database
   //Event: "Loop Open"
   
   ///The event code
   static const eventCodeT eventCode = eventCodes::LOOP_OPEN;
   
   ///The default level 
   static const logLevelT defaultLevel = logLevels::INFO;   
   
   static std::string msgString( messageT & msg  /**< [in] [unused] the empty message */)
   {
      return "LOOP OPEN";
   }
   
};

///Application State Change
/** \ingroup logtypes
  */
struct state_change
{
   //Define the log name for use in the database
   //Event: "App State Change"

   //The event code 
   static const eventCodeT eventCode = eventCodes::STATE_CHANGE;
   
   //The default level 
   static const logLevelT defaultLevel = logLevels::INFO;
   
   ///The type of the message
   struct messageT
   {
      int from;
      int to;
   } __attribute__((packed));
   
   ///Get the length of the message.
   static msgLenT length( const messageT & msg /**< [in] [unused] the message itself */ )
   {
      return sizeof(messageT);
   }
  
   ///Format the buffer given the input message (a std::string).
   static int format( void * msgBuffer,    ///< [out] the buffer, must be pre-allocated to size length(msg)
                      const messageT & msg ///< [in] the message, a std::string, which is placed in the buffer
                    )
   {
      int * ibuff = reinterpret_cast<int *>(msgBuffer);
      
      ibuff[0] = msg.from;
      ibuff[1] = msg.to;
      
      return 0;
   }
   
   ///Extract the message from the buffer and fill in the mesage
   /** 
     * \returns 0 on success.
     * \returns -1 on an error.
     */ 
   static int extract( messageT & msg,   ///< [out] the message, an int[2], which is populated with the contents of buffer.
                       void * msgBuffer, ///< [in] the buffer containing the input codes as an int[2].
                       msgLenT len       ///< [in] the length of the string contained in buffer.
                     )
   {
      int * ibuff = reinterpret_cast<int *>(msgBuffer);
      
      msg.from = ibuff[0];
      msg.to = ibuff[1];
      
      return 0;
   }
   
   /// Format the message for text output, including translation of state codes to text form.
   /**
     * \returns the message formatted as "State changed from UNINITIALIZED to INITIALIZED"
     */ 
   static std::string msgString( messageT & msg /**< [in] the message structure */ )
   {
      std::stringstream s;
      s << "State changed from " << app::stateCodes::codeText(msg.from) << " to " << app::stateCodes::codeText(msg.to);
      return s.str();
   }
};

#endif

} //namespace logger
} //namespace MagAOX

#endif //logger_logTypes_hpp

