/** \file logTypesBasics.hpp 
  * \brief The MagAO-X logger basic log types.
  * \author Jared R. Males (jaredmales@gmail.com)
  *
  * History:
  * - 2018-01-01 created by JRM
  */ 
#ifndef logger_logTypesBasics_hpp
#define logger_logTypesBasics_hpp

namespace MagAOX
{
namespace logger 
{

/// Worker class for software logs
/** Such logs are used to log software status, warnings, and errors, as well as debugging.  
  * 
  * \ingroup logtypesbasics
  */
struct software_log
{
   template< typename swbuilderT, typename swlogT>
   static int serialize_software_log( swbuilderT builder,
                                      swlogT & msg
                                    )
   {
      builder.setFile(msg.m_file);
      builder.setLinenum(msg.m_linenum);
      builder.setCode(msg.m_code);
      builder.setExplanation(msg.m_explanation);
   }

   template<typename swlogT, typename swreaderT>
   static int unserialize_software_log( swlogT & msg,
                                        swreaderT reader
                                      )
   {
      msg.m_file = reader.getFile();
      msg.m_linenum = reader.getLinenum();
      msg.m_code = reader.getCode();
      msg.m_explanation = reader.getExplanation();
   }

   template<typename swlogT>
   static std::string msgString( swlogT & msg )
   {
      std::string ret = "SW FILE: ";
      ret += msg.m_file;
      ret += " LINE: ";
      ret += mx::convertToString(msg.m_linenum);
      ret += "  CODE: ";
      ret += mx::convertToString(msg.m_code);
      ret += " >";
      ret += msg.m_explanation;
      
      return ret;
   }
};




///Empty type for resolving logs with no message.
/** 
  * \ingroup logtypesbasics
  */ 
struct emptyMessage
{
};





} //namespace logger
} //namespace MagAOX

#endif //logger_logTypesBasics_hpp

