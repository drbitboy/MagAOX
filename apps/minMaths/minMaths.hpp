
#ifndef minMaths_hpp
#define minMaths_hpp

#include "../../libMagAOX/libMagAOX.hpp" //Note this is included on command line to trigger pch
#include "../../magaox_git_version.h"

#include <limits>

namespace MagAOX
{
namespace app
{

/*
void externalLog ( const std::string & name,
                         const int & code,
                         const std::string & valueStr,
                         const std::string & source
                       )
{
   std::cerr << name << " " << code << " " << valueStr << " " << source << "\n";
}*/


/** MagAO-X application to do math on some numbers
  *
  */
class minMaths : public MagAOXApp<>
{

protected:
   double m_val {0};

   // declare our properties
   pcf::IndiProperty m_indiP_myVal;
   pcf::IndiProperty m_indiP_myVal_maths;

   pcf::IndiProperty m_indiP_otherVal;

   pcf::IndiProperty m_indiP_setOtherVal;
   
   std::string m_myVal {"x"};
   std::string m_otherDevName;
   std::string m_otherValName;

   double m_startVal{0.0};

   int updateVals();

public:

   /// Default c'tor.
   minMaths();

   ~minMaths() noexcept
   {
   }


   /// Setup the configuration system (called by MagAOXApp::setup())
   virtual void setupConfig();

   /// Load the configuration system results (called by MagAOXApp::setup())
   virtual void loadConfig();

   /// Checks if the device was found during loadConfig.
   virtual int appStartup();

   /// Implementation of the FSM for the maths.
   virtual int appLogic();

   /// Do any needed shutdown tasks.  Currently nothing in this app.
   virtual int appShutdown();


   INDI_NEWCALLBACK_DECL(minMaths, m_indiP_myVal);

   INDI_SETCALLBACK_DECL(minMaths, m_indiP_otherVal);
   
   INDI_NEWCALLBACK_DECL(minMaths,  m_indiP_setOtherVal);

};

minMaths::minMaths() : MagAOXApp(MAGAOX_CURRENT_SHA1, MAGAOX_REPO_MODIFIED)
{
   m_loopPause = 5000000000;   // Slow updates to 5s intervals
   return;
}

void minMaths::setupConfig()
{
   config.add("myVal", "", "myVal", argType::Required, "", "myVal", false, "string", "The name of this app's value.");
   config.add("otherDevName", "", "otherDevName", argType::Required, "", "otherDevName", false, "string", "The name of the other app name.");
   config.add("otherValName", "", "otherValName", argType::Required, "", "otherValName", false, "string", "The name of the other val name.");
   config.add("startVal", "", "startVal", argType::Required, "", "startVal", false, "float", "This app's value at startup; default=0.0");
}

void minMaths::loadConfig()
{
   config(m_myVal, "myVal");
   config(m_otherDevName, "otherDevName");
   config(m_otherValName, "otherValName");
   config(m_startVal, "startVal");

}

int minMaths::appStartup()
{
   // set up the x input property
   REG_INDI_NEWPROP(m_indiP_myVal, m_myVal, pcf::IndiProperty::Number);
   indi::addNumberElement<double>( m_indiP_myVal, "value",  std::numeric_limits<double>::min(),  std::numeric_limits<double>::max(), 1.0,  "%f", "");
   //m_indiP_myVal["value"].set<double>(0.0);
   m_indiP_myVal["value"].set<double>(m_startVal);


   // set up the result maths property
   REG_INDI_NEWPROP_NOCB(m_indiP_myVal_maths, "maths", pcf::IndiProperty::Number);
   indi::addNumberElement<double>(m_indiP_myVal_maths, "prod", std::numeric_limits<double>::min(), std::numeric_limits<double>::max(), 1.0, "%f", "");

   REG_INDI_SETPROP(m_indiP_otherVal,  m_otherDevName, m_otherValName);
   m_indiP_otherVal.add (pcf::IndiElement("value"));
   m_indiP_otherVal["value"].set<double>(0.0);

   createStandardIndiNumber<double>(  m_indiP_setOtherVal, "other_val", -1e50, 1e50, 0, "%0.f");
    m_indiP_setOtherVal["current"].set<double>(0.0);
    m_indiP_setOtherVal["target"].set<double>(0.0);
   registerIndiPropertyNew( m_indiP_setOtherVal, INDI_NEWCALLBACK( m_indiP_setOtherVal));
   
   updateVals();
   state(stateCodes::READY);
   return 0;
}

int minMaths::appLogic()
{
   return 0;

}

int minMaths::appShutdown()
{

   return 0;
}

int minMaths::updateVals()
{
   // extract value
   double v = m_indiP_myVal["value"].get<double>();

   if(v == -1) log<text_log>( "value set to -1!", logPrio::LOG_WARNING);
   if(v == -2) log<text_log>( "value set to -2!", logPrio::LOG_ERROR);
   if(v == -3) log<text_log>( "value set to -3!", logPrio::LOG_CRITICAL);
   if(v == -4) log<text_log>( "value set to -4!", logPrio::LOG_ALERT);
   if(v == -5) log<text_log>( "value set to -5!", logPrio::LOG_EMERGENCY);
   
   // fill maths
   m_indiP_myVal_maths["prod"] = v*m_indiP_otherVal["value"].get<double>();
   updateIfChanged( m_indiP_setOtherVal, "current", m_indiP_otherVal["value"].get<double>());

   log<text_log>("set new value: " + std::to_string(v), logPrio::LOG_NOTICE);
   // publish maths
   m_indiP_myVal_maths.setState (pcf::IndiProperty::Ok);
   if(m_indiDriver) m_indiDriver->sendSetProperty (m_indiP_myVal_maths);

   return 0;
}

INDI_NEWCALLBACK_DEFN(minMaths, m_indiP_myVal)(const pcf::IndiProperty &ipRecv)
{
   log<text_log>("(1of3)In minMaths::{newcallback}m_intiP_myVal", logPrio::LOG_NOTICE);
   log<text_log>("(2of3)ipRecv.getname() " + ipRecv.getName(), logPrio::LOG_NOTICE);
   log<text_log>("(3of3)m_indiP_myVal.getName() " + m_indiP_myVal.getName(), logPrio::LOG_NOTICE);

   if (ipRecv.getName() == m_indiP_myVal.getName())
   {
      // received a new value for property val
      m_indiP_myVal["value"] = ipRecv["value"].get<double>();
      m_indiP_myVal.setState (pcf::IndiProperty::Ok);
      m_indiDriver->sendSetProperty (m_indiP_myVal);

      updateVals();

      return 0;
   }
   return -1;
}

INDI_SETCALLBACK_DEFN(minMaths, m_indiP_otherVal)(const pcf::IndiProperty &ipRecv)
{
   m_indiP_otherVal = ipRecv;

   updateVals();
   return 0;
}

INDI_NEWCALLBACK_DEFN(minMaths,  m_indiP_setOtherVal)(const pcf::IndiProperty &ipRecv)
{
   log<text_log>("(1of3)In minMaths::{newcallback}m_intiP_setOtherVal", logPrio::LOG_NOTICE);
   log<text_log>("(2of3)ipRecv.getname() " + ipRecv.getName(), logPrio::LOG_NOTICE);
   log<text_log>("(3of3)m_indiP_setOtherVal.getName() " + m_indiP_setOtherVal.getName(), logPrio::LOG_NOTICE);

   if (ipRecv.getName() ==  m_indiP_setOtherVal.getName())
   {
      std::cerr << " m_indiP_setOtherVal\n";
      
      // received a new value for property val
       m_indiP_setOtherVal["target"] = ipRecv["target"].get<double>();
       m_indiP_setOtherVal.setState (pcf::IndiProperty::Ok);
      //m_indiDriver->sendSetProperty (m_indiP_myVal);

      sendNewProperty(m_indiP_otherVal, "value",  m_indiP_setOtherVal["target"].get<double>());
      
      updateVals();

      return 0;
   }
   return -1;
}

} //namespace app
} //namespace MagAOX

#endif //minMaths_hpp
