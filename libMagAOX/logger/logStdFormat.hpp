/** \file logStdFormat.hpp 
  * \brief Standard formating of log entries for readable output.
  * \author Jared R. Males (jaredmales@gmail.com)
  *
  * History:
  * - 2017-12-24 created by JRM
  */ 

#ifndef logger_logStdFormat_hpp
#define logger_logStdFormat_hpp

#include "logTypes.hpp"

namespace MagAOX
{
namespace logger 
{
   
/// Worker function that formats a log into the standard text representation.
/** \todo change to using a std::ios as input instead of only using std::cout
  *
  * \ingroup logformat
  */
template<typename logT>
void _stdFormat( ::capnp::FlatArrayMessageReader & reader )
{
   logLevelT lvl;
   
   time::timespecX ts;

   logT lmsg;

   
   
   ts = log_entry::timestamp(reader);
   lvl = log_entry::logLevel(reader);
   
   log_entry::unserialize( lmsg, reader);
   
   std::cout << ts.ISO8601DateTimeStrX() << " " << levelString(lvl) << " " << logT::msgString(lmsg) << "\n";
}

/// Place the log in standard text format, with event code specific formatting.
/** 
  * \ingroup logformat
  */ 
inline
void logStdFormat(::capnp::FlatArrayMessageReader & reader )
{
   uint16_t ec = log_entry::eventCode(reader);

   switch(ec)
   {
      case LogEntry::GIT_STATE:
         return _stdFormat<git_state>(reader);
      case LogEntry::TEXT_LOG:
         return _stdFormat<text_log>(reader);
      case LogEntry::USER_LOG:
         return _stdFormat<user_log>(reader);
      /*case state_change::eventCode:
         return _stdFormat<state_change>(buffer);*/
      case LogEntry::SOFTWARE_DEBUG:
         return _stdFormat<software_debug>(reader);
      case LogEntry::SOFTWARE_DEBUG2:
         return _stdFormat<software_debug2>(reader);
      /*case software_info::eventCode:
         return _stdFormat<software_info>(buffer);
      case software_warning::eventCode:
         return _stdFormat<software_warning>(buffer);
      case software_error::eventCode:
         return _stdFormat<software_error>(buffer);
      case software_critical::eventCode:
         return _stdFormat<software_critical>(buffer);
      case software_fatal::eventCode:
         return _stdFormat<software_fatal>(buffer);
      case loop_closed::eventCode:
         return _stdFormat<loop_closed>(buffer);
      case loop_paused::eventCode:
         return _stdFormat<loop_paused>(buffer);
      case loop_open::eventCode:
         return _stdFormat<loop_open>(buffer);*/
      
      default:
         std::cout << "Unknown log type: " << ec << "\n";
   }
}

} //namespace logger 
} //namespace MagAOX 

#endif //logger_logStdFormat_hpp
