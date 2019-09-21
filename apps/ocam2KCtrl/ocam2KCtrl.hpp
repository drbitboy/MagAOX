/** \file ocam2KCtrl.hpp
  * \brief The MagAO-X OCAM2K EMCCD camera controller.
  *
  * \author Jared R. Males (jaredmales@gmail.com)
  *
  * \ingroup ocam2KCtrl_files
  */

#ifndef ocam2KCtrl_hpp
#define ocam2KCtrl_hpp


#include <edtinc.h>



#include "../../libMagAOX/libMagAOX.hpp" //Note this is included on command line to trigger pch
#include "../../magaox_git_version.h"

typedef MagAOX::app::MagAOXApp<true> MagAOXAppT; //This needs to be before pdvUtils.hpp for logging to work.

#include "fli/ocam2_sdk.h"
#include "ocamUtils.hpp"

namespace MagAOX
{
namespace app
{

/** \defgroup ocam2KCtrl OCAM2K EMCCD Camera
  * \brief Control of the OCAM2K EMCCD Camera.
  *
  * <a href="../handbook/apps/ocam2KCtrl.html">Application Documentation</a>
  *
  * \ingroup apps
  *
  */

/** \defgroup ocam2KCtrl_files OCAM2K EMCCD Camera Files
  * \ingroup ocam2KCtrl
  */

/** MagAO-X application to control the OCAM 2K EMCCD
  *
  * \ingroup ocam2KCtrl
  * 
  */
class ocam2KCtrl : public MagAOXApp<>, public dev::stdCamera<ocam2KCtrl>, public dev::edtCamera<ocam2KCtrl>, public dev::frameGrabber<ocam2KCtrl>,  public dev::dssShutter<ocam2KCtrl>, public dev::telemeter<ocam2KCtrl>
{
   friend class dev::stdCamera<ocam2KCtrl>;
   friend class dev::edtCamera<ocam2KCtrl>;
   friend class dev::frameGrabber<ocam2KCtrl>;
   friend class dev::dssShutter<ocam2KCtrl>;
   friend class dev::telemeter<ocam2KCtrl>;
   
   typedef MagAOXApp<> MagAOXAppT;
   
protected:

   /** \name configurable parameters 
     *@{
     */ 

   //Camera:

   std::string m_ocamDescrambleFile; ///< Path the OCAM 2K pixel descrambling file, relative to MagAO-X config directory.

   unsigned m_maxEMGain {600}; ///< The maximum allowable EM gain settable by the user.
   
   ///@}
   
   ocam2_id m_ocam2_id {0}; ///< OCAM SDK id.
   
   float m_fpsSet {0}; ///< The commanded fps, as returned by the camera

   long m_currImageNumber {-1}; ///< The current image number, retrieved from the image itself.
       
   long m_lastImageNumber {-1};  ///< The last image number, saved from the last loop through.
   
   unsigned m_protectionResetConfirmed {0}; ///< Counter indicating the number of times that the protection reset has been requested within 10 seconds, for confirmation.

   double m_protectionResetReqTime {0}; ///< The time at which protection reset was requested.  You have 10 seconds to confirm.

   unsigned m_emGain {1}; ///< The current EM gain.

   bool m_poweredOn {false};
   
   ocamTemps m_temps; ///< Structure holding the last temperature measurement.
   
public:

   ///Default c'tor
   ocam2KCtrl();

   ///Destructor
   ~ocam2KCtrl() noexcept;

   /// Setup the configuration system (called by MagAOXApp::setup())
   virtual void setupConfig();

   /// load the configuration system results (called by MagAOXApp::setup())
   virtual void loadConfig();

   /// Startup functions
   /** Sets up the INDI vars, and the f.g. thread.
     *
     */
   virtual int appStartup();

   /// Implementation of the FSM for the OCAM 2K.
   virtual int appLogic();

   /// Implementation of the on-power-off FSM logic
   virtual int onPowerOff();

   /// Implementation of the while-powered-off FSM
   virtual int whilePowerOff();

   /// Do any needed shutdown tasks. 
   virtual int appShutdown();

   /// Get the current device temperatures
   /**
     * \returns 0 on success
     * \returns -1 on error
     */ 
   int getTemps();
   
   /// Set the CCD temperature setpoint.
   /**
     * \returns 0 on success
     * \returns -1 on error
     */
   int setTemp( float temp /**< [in] The new CCD temp setpoing [C] */ );
   
   /// Get the current frame rate.
   /**
     * \returns 0 on success
     * \returns -1 on error
     */
   int getFPS();
   
   /// Set the frame rate.
   /**
     * \returns 0 on success
     * \returns -1 on error
     */
   int setFPS( float fps  /**< [in] the new value of framerate [fps] */ );
   
   /// Reset the EM Protection 
   /** 
     * \returns 0 on success
     * \returns -1 on error
     */
   int resetEMProtection();
   
   /// Get the current EM Gain.
   /**
     * \returns 0 on success
     * \returns -1 on error
     */
   int getEMGain();
   
   /// Set the EM gain.
   /**
     * \returns 0 on success
     * \returns -1 on error
     */
   int setEMGain( unsigned emg  /**< [in] the new value of EM gain */ );
   
   /// Implementation of the framegrabber configureAcquisition interface
   /** Sends the mode command over serial, sets the FPS, and initializes the OCAM SDK.
     * 
     * \returns 0 on success
     * \returns -1 on error
     */
   int configureAcquisition();
   
   /// Implementation of the framegrabber startAcquisition interface
   /** Initializes m_lastImageNumber, and calls edtCamera::pdvStartAcquisition
     * 
     * \returns 0 on success
     * \returns -1 on error
     */
   int startAcquisition();
   
   /// Implementation of the framegrabber acquireAndCheckValid interface
   /** Calls edtCamera::pdvAcquire, then analyzes the OCAM generated framenumber for skips and corruption.
     * 
     * \returns 0 on success
     * \returns -1 on error
     */
   int acquireAndCheckValid();
   
   /// Implementation of the framegrabber loadImageIntoStream interface
   /** Conducts the OCAM descramble.
     * 
     * \returns 0 on success
     * \returns -1 on error
     */
   int loadImageIntoStream( void * dest  /**< [in] */);
   
   /// Implementation of the framegrabber reconfig interface
   /** Locks the INDI mutex and calls edtCamera::pdvReconfig.
     * \returns 0 on success
     * \returns -1 on error
     */
   int reconfig();
   
   
   //INDI:
protected:
   //declare our properties
   pcf::IndiProperty m_indiP_temps;
   pcf::IndiProperty m_indiP_fps;
   pcf::IndiProperty m_indiP_emProtReset;
   pcf::IndiProperty m_indiP_emGain;

public:
   //INDI_NEWCALLBACK_DECL(ocam2KCtrl, m_indiP_ccdtemp);
   INDI_NEWCALLBACK_DECL(ocam2KCtrl, m_indiP_fps);
   INDI_NEWCALLBACK_DECL(ocam2KCtrl, m_indiP_emProtReset);
   INDI_NEWCALLBACK_DECL(ocam2KCtrl, m_indiP_emGain);

   //telemeter:
   int checkRecordTimes();
   
   int recordTelem( const ocam_temps * );
   
   int recordTemps(bool force = false);
   
};

inline
ocam2KCtrl::ocam2KCtrl() : MagAOXApp(MAGAOX_CURRENT_SHA1, MAGAOX_REPO_MODIFIED)
{
   //--- MagAOXApp Power Mgt. ---
   m_powerMgtEnabled = true;
   m_powerOnWait = 10;
   
   //--- stdCamera ---
   m_usesROI = false;
   //m_usesModes is set to true by edtCamera
   m_startupTemp = 20;
   
   return;
}

inline
ocam2KCtrl::~ocam2KCtrl() noexcept
{
   return;
}

inline
void ocam2KCtrl::setupConfig()
{
   dev::stdCamera<ocam2KCtrl>::setupConfig(config);

   dev::edtCamera<ocam2KCtrl>::setupConfig(config);
      
   config.add("camera.ocamDescrambleFile", "", "camera.ocamDescrambleFile", argType::Required, "camera", "ocamDescrambleFile", false, "string", "The path of the OCAM descramble file, relative to MagAOX/config.");
   
   config.add("camera.maxEMGain", "", "camera.maxEMGain", argType::Required, "camera", "maxEMGain", false, "unsigned", "The maximum EM gain which can be set by  user. Default is 600.  Min is 1, max is 600.");
 
   dev::frameGrabber<ocam2KCtrl>::setupConfig(config);
   
   dev::dssShutter<ocam2KCtrl>::setupConfig(config);
   
   dev::telemeter<ocam2KCtrl>::setupConfig(config);
}


inline
void ocam2KCtrl::loadConfig()
{
   dev::stdCamera<ocam2KCtrl>::loadConfig(config);
   dev::edtCamera<ocam2KCtrl>::loadConfig(config);
   
   config(m_ocamDescrambleFile, "camera.ocamDescrambleFile");
   config(m_maxEMGain, "camera.maxEMGain");
   if(m_maxEMGain < 1)
   {
      m_maxEMGain = 1;
      log<text_log>("maxEMGain set to 1");
   }
   
   if(m_maxEMGain > 600)
   {
      m_maxEMGain = 600;
      log<text_log>("maxEMGain set to 600");
   }
   
   dev::frameGrabber<ocam2KCtrl>::loadConfig(config);
   dev::dssShutter<ocam2KCtrl>::loadConfig(config);
   dev::telemeter<ocam2KCtrl>::loadConfig(config);
}

inline
int ocam2KCtrl::appStartup()
{
   REG_INDI_NEWPROP_NOCB(m_indiP_temps, "temps", pcf::IndiProperty::Number);
   m_indiP_temps.add (pcf::IndiElement("cpu"));
   m_indiP_temps["cpu"].set(0);
   m_indiP_temps.add (pcf::IndiElement("power"));
   m_indiP_temps["power"].set(0);
   m_indiP_temps.add (pcf::IndiElement("bias"));
   m_indiP_temps["bias"].set(0);
   m_indiP_temps.add (pcf::IndiElement("water"));
   m_indiP_temps["water"].set(0);
   m_indiP_temps.add (pcf::IndiElement("left"));
   m_indiP_temps["left"].set(0);
   m_indiP_temps.add (pcf::IndiElement("right"));
   m_indiP_temps["right"].set(0);
   m_indiP_temps.add (pcf::IndiElement("cooling"));
   m_indiP_temps["cooling"].set(0);

   REG_INDI_NEWPROP(m_indiP_fps, "fps", pcf::IndiProperty::Number);
   m_indiP_fps.add (pcf::IndiElement("current"));
   m_indiP_fps["current"].set(0);
   m_indiP_fps.add (pcf::IndiElement("target"));
   m_indiP_fps.add (pcf::IndiElement("measured"));

   REG_INDI_NEWPROP(m_indiP_emProtReset, "emProtectionReset", pcf::IndiProperty::Text);
   m_indiP_emProtReset.add (pcf::IndiElement("current"));
   m_indiP_emProtReset.add (pcf::IndiElement("target"));
   
   REG_INDI_NEWPROP(m_indiP_emGain, "emgain", pcf::IndiProperty::Number);
   m_indiP_emGain.add (pcf::IndiElement("current"));
   m_indiP_emGain["current"].set(m_emGain);
   m_indiP_emGain.add (pcf::IndiElement("target"));
   
   if(dev::stdCamera<ocam2KCtrl>::appStartup() < 0)
   {
      return log<software_critical,-1>({__FILE__,__LINE__});
   }
   
   if(dev::edtCamera<ocam2KCtrl>::appStartup() < 0)
   {
      return log<software_critical,-1>({__FILE__,__LINE__});
   }
   
   if(dev::frameGrabber<ocam2KCtrl>::appStartup() < 0)
   {
      return log<software_critical,-1>({__FILE__,__LINE__});
   }
   
   if(dev::dssShutter<ocam2KCtrl>::appStartup() < 0)
   {
      return log<software_critical,-1>({__FILE__,__LINE__});
   }
   
   m_temps.setInvalid();
   if(dev::telemeter<ocam2KCtrl>::appStartup() < 0)
   {
      return log<software_error,-1>({__FILE__,__LINE__});
   }
   
   return 0;

}



inline
int ocam2KCtrl::appLogic()
{
   //first run frameGrabber's appLogic to see if the f.g. thread has exited.
   //Also does a POWERON check, so this should be before stdCamera
   ///\todo the frameGrabber POWERON check can probably be handled by onPowerOff...
   if(dev::frameGrabber<ocam2KCtrl>::appLogic() < 0)
   {
      return log<software_error, -1>({__FILE__, __LINE__});
   }
   
   //and run stdCamera's appLogic
   if(dev::stdCamera<ocam2KCtrl>::appLogic() < 0)
   {
      return log<software_error, -1>({__FILE__, __LINE__});
   }
   
   //and run edtCamera's appLogic
   if(dev::edtCamera<ocam2KCtrl>::appLogic() < 0)
   {
      return log<software_error, -1>({__FILE__, __LINE__});
   }
   
   //and run dssShutter's appLogic
   if(dev::dssShutter<ocam2KCtrl>::appLogic() < 0)
   {
      return log<software_error, -1>({__FILE__, __LINE__});
   }
   
   /*if( state() == stateCodes::POWERON )
   {
      if(powerOnWaitElapsed()) 
      {
         state(stateCodes::NOTCONNECTED);
         m_reconfig = true; //Trigger a f.g. thread reconfig.
      }
      else
      {
         return 0;
      }
   }*/

   if( state() == stateCodes::NOTCONNECTED || state() == stateCodes::ERROR)
   {
      m_temps.setInvalid();
      
      std::string response;

      //Might have gotten here because of a power off.
      if(MagAOXAppT::m_powerState == 0) return 0;
      
      int ret = pdvSerialWriteRead( response, "fps"); //m_pdv, "fps", m_readTimeout);
      if( ret == 0)
      {
         state(stateCodes::CONNECTED);
      }
      else
      {
         sleep(1);
         return 0;
      }
   }

   if( state() == stateCodes::CONNECTED )
   {
      //Get a lock
      std::unique_lock<std::mutex> lock(m_indiMutex);
      
      if( getFPS() == 0 )
      {
         if(m_fpsSet == 0) state(stateCodes::READY);
         else state(stateCodes::OPERATING);
         
         if(m_poweredOn && m_ccdTempSetpt > -999)
         {
            m_poweredOn = false;
            if(setTempSetPt() < 0)
            {
               return log<software_error,0>({__FILE__,__LINE__});
            }
         }
      }
      else
      {
         state(stateCodes::ERROR);
         return log<software_error,0>({__FILE__,__LINE__});
      }
   }

   if( state() == stateCodes::READY || state() == stateCodes::OPERATING )
   {
      //Get a lock if we can
      std::unique_lock<std::mutex> lock(m_indiMutex, std::try_to_lock);

      //but don't wait for it, just go back around.
      if(!lock.owns_lock()) return 0;
      
      if(getTemps() < 0)
      {
         if(MagAOXAppT::m_powerState == 0) return 0;
         m_temps.setInvalid();
         state(stateCodes::ERROR);
         return 0;
      }

      if(getFPS() < 0)
      {
         if(MagAOXAppT::m_powerState == 0) return 0;
         
         state(stateCodes::ERROR);
         return 0;
      }
      
      if(m_protectionResetConfirmed > 0 )
      {
         if( mx::get_curr_time() - m_protectionResetReqTime > 10.0)
         {
            m_protectionResetConfirmed = 0;
            updateIfChanged(m_indiP_emProtReset, "current", std::string(""));
            updateIfChanged(m_indiP_emProtReset, "target", std::string(""));
            log<text_log>("protection reset request not confirmed", logPrio::LOG_NOTICE);
         }
      }
      
      if(getEMGain () < 0)
      {
         if(MagAOXAppT::m_powerState == 0) return 0;
         
         state(stateCodes::ERROR);
         return 0;
      }
      
      if(frameGrabber<ocam2KCtrl>::updateINDI() < 0)
      {
         log<software_error>({__FILE__, __LINE__});
         state(stateCodes::ERROR);
         return 0;
      }
      
      if(stdCamera<ocam2KCtrl>::updateINDI() < 0)
      {
         log<software_error>({__FILE__, __LINE__});
         state(stateCodes::ERROR);
         return 0;
      }
      
      if(edtCamera<ocam2KCtrl>::updateINDI() < 0)
      {
         log<software_error>({__FILE__, __LINE__});
         state(stateCodes::ERROR);
         return 0;
      }
      
      if(dssShutter<ocam2KCtrl>::updateINDI() < 0)
      {
         log<software_error>({__FILE__, __LINE__});
         state(stateCodes::ERROR);
         return 0;
      }
      
      if(telemeter<ocam2KCtrl>::appLogic() < 0)
      {
         log<software_error>({__FILE__, __LINE__});
         return 0;
      }
      
   }

   //Fall through check?

   return 0;

}

inline
int ocam2KCtrl::onPowerOff()
{
   m_powerOnCounter = 0;
   
   std::lock_guard<std::mutex> lock(m_indiMutex);
   
   m_temps.setInvalid();
   //updateIfChanged(m_indiP_ccdtemp, "current", m_temps.CCD);
   //updateIfChanged(m_indiP_ccdtemp, "target", m_temps.CCD);
   
   updateIfChanged(m_indiP_temps, "cpu", m_temps.CPU);
   updateIfChanged(m_indiP_temps, "power", m_temps.POWER);
   updateIfChanged(m_indiP_temps, "bias", m_temps.BIAS);
   updateIfChanged(m_indiP_temps, "water", m_temps.WATER);
   updateIfChanged(m_indiP_temps, "left", m_temps.LEFT);
   updateIfChanged(m_indiP_temps, "right", m_temps.RIGHT);
   updateIfChanged(m_indiP_temps, "cooling", m_temps.COOLING_POWER);
      
   updateIfChanged(m_indiP_fps, "current", std::string(""));
   updateIfChanged(m_indiP_fps, "target", std::string(""));
   updateIfChanged(m_indiP_fps, "measured", std::string(""));
   
   updateIfChanged(m_indiP_emProtReset, "current", std::string(""));
   updateIfChanged(m_indiP_emProtReset, "target", std::string(""));
   
   updateIfChanged(m_indiP_emGain, "current", std::string(""));
   updateIfChanged(m_indiP_emGain, "target", std::string(""));
   
   ///\todo error check these base class fxns.
   stdCamera<ocam2KCtrl>::onPowerOff();
   
   edtCamera<ocam2KCtrl>::onPowerOff();
   
   dssShutter<ocam2KCtrl>::onPowerOff();

   //Setting m_poweredOn
   m_poweredOn = true;

   
   return 0;
}

inline
int ocam2KCtrl::whilePowerOff()
{
   std::lock_guard<std::mutex> lock(m_indiMutex);
   
   ///\todo error check these base class fxns.
   stdCamera<ocam2KCtrl>::whilePowerOff();
   
   edtCamera<ocam2KCtrl>::whilePowerOff();
   
   dssShutter<ocam2KCtrl>::whilePowerOff();
   
   return 0;
}

inline
int ocam2KCtrl::appShutdown()
{
   ///\todo error check these base class fxns.
   dev::stdCamera<ocam2KCtrl>::appShutdown();
   dev::edtCamera<ocam2KCtrl>::appShutdown();
   dev::frameGrabber<ocam2KCtrl>::appShutdown();
   dev::dssShutter<ocam2KCtrl>::appShutdown();
   dev::telemeter<ocam2KCtrl>::appShutdown();
   
   return 0;
}


inline
int ocam2KCtrl::getTemps()
{
   std::string response;

   if( pdvSerialWriteRead( response, "temp") == 0)// m_pdv, "temp", m_readTimeout) == 0)
   {
      ocamTemps temps;

      if(parseTemps( temps, response ) < 0) 
      {
         if(MagAOXAppT::m_powerState == 0) return -1;
         m_temps.setInvalid();
         m_ccdTemp = m_temps.CCD;
         m_ccdTempSetpt = m_temps.SET;
         
         m_tempControlStatus = "UNKNOWN";
         
         recordTemps();
         return log<software_error, -1>({__FILE__, __LINE__, "Temp. parse error"});
      }
      
      m_temps = temps;

      //stdCamera temp control:
      m_ccdTemp = m_temps.CCD;
      m_ccdTempSetpt = m_temps.SET;
      
      if(fabs(m_temps.CCD - m_temps.SET) < 1.0)
      {
         m_tempControlStatus = "ON TARGET";
         m_tempControlOnTarget = true;
      }
      else
      {
         m_tempControlStatus = "OFF TARGET";
         m_tempControlOnTarget = false;
      }
      
      //Telemeter:
      recordTemps();
      
      updateIfChanged(m_indiP_temps, "cpu", m_temps.CPU);
      updateIfChanged(m_indiP_temps, "power", m_temps.POWER);
      updateIfChanged(m_indiP_temps, "bias", m_temps.BIAS);
      updateIfChanged(m_indiP_temps, "water", m_temps.WATER);
      updateIfChanged(m_indiP_temps, "left", m_temps.LEFT);
      updateIfChanged(m_indiP_temps, "right", m_temps.RIGHT);
      updateIfChanged(m_indiP_temps, "cooling", m_temps.COOLING_POWER);
      return 0;

   }
   else return log<software_error,-1>({__FILE__, __LINE__});

}

inline
int ocam2KCtrl::.setTempSetPt()
{
   std::string response;

   std::string tempStr = std::to_string( m_ccdTempSetpt );
   
   ///\todo make more configurable
   if(temp >= 30 || temp < -50) 
   {
      return log<text_log,-1>({"attempt to set temperature outside valid range: " + tempStr}, logPrio::LOG_ERROR);
   }
   
   if( pdvSerialWriteRead( response, "temp " + tempStr) == 0)
   {
      ///\todo check response
      return log<text_log,0>({"set temperature: " + tempStr});
   }
   else return log<software_error,-1>({__FILE__, __LINE__});

}

inline
int ocam2KCtrl::getFPS()
{
   std::string response;

   if( pdvSerialWriteRead( response, "fps") == 0) // m_pdv, "fps", m_readTimeout) == 0)
   {
      float fps;
      if(parseFPS( fps, response ) < 0) 
      {
         if(MagAOXAppT::m_powerState == 0) return -1;
         return log<software_error, -1>({__FILE__, __LINE__, "fps parse error"});
      }
      m_fpsSet = fps;

      updateIfChanged(m_indiP_fps, "current", m_fpsSet);

      double fpsMeas = 0;
      
      updateIfChanged(m_indiP_fps, "measured", fpsMeas);
      
      return 0;

   }
   else return log<software_error,-1>({__FILE__, __LINE__});

}

inline
int ocam2KCtrl::setFPS(float fps)
{
   std::string response;

   ///\todo should we have fps range checks or let camera deal with it?
   
   std::string fpsStr= std::to_string(fps);
   if( pdvSerialWriteRead( response, "fps " + fpsStr ) == 0) //m_pdv, "fps " + fpsStr, m_readTimeout) == 0)
   {
      ///\todo check response
      log<text_log>({"set fps: " + fpsStr});
      
      return 0;
   }
   else return log<software_error,-1>({__FILE__, __LINE__});

}

inline 
int ocam2KCtrl::resetEMProtection()
{
   std::string response;
   
   if( pdvSerialWriteRead( response, "protection reset") == 0)
   {
      std::cerr << "\n******************************************\n";
      std::cerr << "protection reset:\n";
      std::cerr << response << "\n";
      std::cerr << "\n******************************************\n";
      ///\todo check response.
      
      updateIfChanged(m_indiP_emProtReset, "current", std::string("RESET"));
      updateIfChanged(m_indiP_emProtReset, "target", std::string(""));
      
      log<text_log>("overillumination protection has been reset", logPrio::LOG_NOTICE);
      
      m_protectionResetConfirmed = 0;
      return 0;

   }
   else return log<software_error,-1>({__FILE__, __LINE__});
   
}

inline
int ocam2KCtrl::getEMGain()
{
   std::string response;

   if( pdvSerialWriteRead( response, "gain") == 0)
   {
      unsigned emGain;
      if(parseEMGain( emGain, response ) < 0) 
      {
         if(MagAOXAppT::m_powerState == 0) return -1;
         return log<software_error, -1>({__FILE__, __LINE__, "EM Gain parse error"});
      }
      m_emGain = emGain;

      updateIfChanged(m_indiP_emGain, "current", m_emGain);
      
      return 0;

   }
   else return log<software_error,-1>({__FILE__, __LINE__});
}
   
inline
int ocam2KCtrl::setEMGain( unsigned emg )
{
   std::string response;

   if(emg < 1 || emg > m_maxEMGain)
   {
      log<text_log>("Attempt to set EM gain to " + std::to_string(emg) + " outside limits refused", logPrio::LOG_WARNING);
      return 0;
   }
   
   std::string emgStr= std::to_string(emg);
   if( pdvSerialWriteRead( response, "gain " + emgStr ) == 0) //m_pdv, "gain " + emgStr, m_readTimeout) == 0)
   {
      ///\todo check response
      log<text_log>({"set EM Gain: " + emgStr});
      
      return 0;
   }
   else return log<software_error,-1>({__FILE__, __LINE__});
   
}

inline
int ocam2KCtrl::configureAcquisition()
{
   //lock mutex
   std::unique_lock<std::mutex> lock(m_indiMutex);
   
   //Send command to camera to place it in the correct mode
   std::string response;
   if( pdvSerialWriteRead( response, m_cameraModes[m_modeName].m_serialCommand) != 0) //m_pdv, m_cameraModes[m_modeName].m_serialCommand, m_readTimeout) != 0)
   {
      log<software_error>({__FILE__, __LINE__, "Error sending command to set mode"});
      sleep(1);
      return -1;
   }
   
    ///\todo check response of pdvSerialWriteRead
   log<text_log>("camera configured with: " +m_cameraModes[m_modeName].m_serialCommand);
   
   if(m_fpsSet > 0) setFPS(m_fpsSet);
   
   log<text_log>("Send command to set mode: " + m_cameraModes[m_modeName].m_serialCommand);
   log<text_log>("Response was: " + response);
  
   updateIfChanged(m_indiP_mode, "current", m_modeName);
   updateIfChanged(m_indiP_mode, "target", std::string(""));
   
 
   /* Initialize the OCAM2 SDK
       */

   if(m_ocam2_id > 0)
   {
      ocam2_exit(m_ocam2_id);
   }
   ocam2_rc rc;
   ocam2_mode mode;

   int OCAM_SZ;
   if(m_raw_height == 121)
   {
      mode = OCAM2_NORMAL;
      OCAM_SZ = 240;
   }
   else if (m_raw_height == 62)
   {
      mode = OCAM2_BINNING;
      OCAM_SZ = 120;
   }
   else
   {
      log<text_log>("Unrecognized OCAM2 mode.", logPrio::LOG_ERROR);
      return -1;
   }

   std::string ocamDescrambleFile = m_configDir + "/" + m_ocamDescrambleFile;

   std::cerr << "ocamDescrambleFile: " << ocamDescrambleFile << std::endl;
   rc=ocam2_init(mode, ocamDescrambleFile.c_str(), &m_ocam2_id);
   if (rc != OCAM2_OK)
   {
      log<text_log>("ocam2_init error. Failed to initialize OCAM SDK with descramble file: " + ocamDescrambleFile, logPrio::LOG_ERROR);
      return -1;
   }
   

   log<text_log>("OCAM2K initialized. id: " + std::to_string(m_ocam2_id));
   log<text_log>(std::string("OCAM2K mode is:") + ocam2_modeStr(ocam2_getMode(m_ocam2_id)));
   
   m_width = OCAM_SZ;
   m_height = OCAM_SZ;
   m_dataType = _DATATYPE_INT16;
   
   return 0;
}
   
inline
int ocam2KCtrl::startAcquisition()
{
   m_lastImageNumber = -1;
   return edtCamera<ocam2KCtrl>::pdvStartAcquisition();
   
}

inline
int ocam2KCtrl::acquireAndCheckValid()
{
   edtCamera<ocam2KCtrl>::pdvAcquire( m_currImageTimestamp );
   
   /* Removed all pdv timeout and overrun checking, since we can rely on frame number from the camera
      to detect missed and corrupted frames.
   
      See ef0dd24 for last version with full checks in it.
   */
  
   //Get the image number to see if this is valid.
   //This is how it is in the ocam2_sdk:
   unsigned currImageNumber = ((int *)m_image_p)[OCAM2_IMAGE_NB_OFFSET/4]; /* int offset */
   m_currImageNumber = currImageNumber;
   
   //For the first loop after a restart
   if( m_lastImageNumber == -1 ) 
   {
      m_lastImageNumber = m_currImageNumber - 1;
   }
      
   if(m_currImageNumber - m_lastImageNumber != 1)
   {
      //Detect exact condition of a wraparound on the unsigned int.
      // Yes, this can only happen once every 13.72 days at 3622 fps 
      // But just in case . . .
      if(m_lastImageNumber != std::numeric_limits<unsigned int>::max() && m_currImageNumber != 0)
      {
         //The far more likely case is a problem...
   
         //If a reasonably small number of frames skipped, then we trust the image number
         if(m_currImageNumber - m_lastImageNumber > 1 && m_currImageNumber - m_lastImageNumber < 100)
         { 
            //This we handle as a non-timeout -- report how many frames were skipped
            long framesSkipped = m_currImageNumber - m_lastImageNumber;
            //and don't `continue` to top of loop
            
            log<text_log>("frames skipped: " + std::to_string(framesSkipped), logPrio::LOG_ERROR);
            
            m_nextMode = m_modeName;
            m_reconfig = 1;
           
            return 1;
            
         }
         else //but if it's any bigger or < 0, it's probably garbage
         {
            ///\todo need frame corrupt log type
            log<text_log>("frame number possibly corrupt: " + std::to_string(m_currImageNumber) + " - " + std::to_string(m_lastImageNumber), logPrio::LOG_ERROR);
            
            m_nextMode = m_modeName;
            m_reconfig = 1;
      
            //Reset the counters.
            m_lastImageNumber = -1;
            
            return 1;
         
         }
      }
   }
   m_lastImageNumber = m_currImageNumber;
   return 0;
}

inline
int ocam2KCtrl::loadImageIntoStream(void * dest)
{
   unsigned currImageNumber = 0;
   ocam2_descramble(m_ocam2_id, &currImageNumber, (short int *) dest, (short int *) m_image_p);
   
   //memcpy(dest, m_image_p, 120*120*2); //This is about 10 usec faster -- but we have to descramble.
   return 0;
}
   
inline
int ocam2KCtrl::reconfig()
{
   //lock mutex
   std::unique_lock<std::mutex> lock(m_indiMutex);
   
   return edtCamera<ocam2KCtrl>::pdvReconfig();
}
   

      
         
   
     
   
   

         

         
         
     
           
    
     

INDI_NEWCALLBACK_DEFN(ocam2KCtrl, m_indiP_ccdtemp)(const pcf::IndiProperty &ipRecv)
{
   if(MagAOXAppT::m_powerState == 0) return 0;
   
   if (ipRecv.getName() == m_indiP_ccdtemp.getName())
   {
      float current = 99, target = 99;

      try
      {
         current = ipRecv["current"].get<float>();
      }
      catch(...){}

      try
      {
         target = ipRecv["target"].get<float>();
      }
      catch(...){}

      
      //Check if target is empty
      if( target == 99 ) target = current;
      
      //Now check if it's valid?
      ///\todo implement more configurable max-set-able temperature
      if( target > 30 ) return 0;
      
      
      //Lock the mutex, waiting if necessary
      std::unique_lock<std::mutex> lock(m_indiMutex);
      
      updateIfChanged(m_indiP_ccdtemp, "target", target);
      
      return setTemp(target);
   }
   return -1;
}



INDI_NEWCALLBACK_DEFN(ocam2KCtrl, m_indiP_fps)(const pcf::IndiProperty &ipRecv)
{
   if(MagAOXAppT::m_powerState == 0) return 0;
   
   if (ipRecv.getName() == m_indiP_fps.getName())
   {
      float current = -99, target = -99;

      try
      {
         current = ipRecv["current"].get<float>();
      }
      catch(...){}
      
      try
      {
         target = ipRecv["target"].get<float>();
      }
      catch(...){}
      
      if(target == -99) target = current;
      
      if(target <= 0) return 0;
      
      //Lock the mutex, waiting if necessary
      std::unique_lock<std::mutex> lock(m_indiMutex);

      updateIfChanged(m_indiP_fps, "target", target);
      
      return setFPS(target);
      
   }
   return -1;
}

INDI_NEWCALLBACK_DEFN(ocam2KCtrl, m_indiP_emProtReset)(const pcf::IndiProperty &ipRecv)
{
   if(MagAOXAppT::m_powerState == 0) return 0;
   
   if (ipRecv.getName() == m_indiP_emProtReset.getName())
   {
      std::string current, target;

      if(ipRecv.find("current"))
      {
         current = ipRecv["current"].get<std::string>();
      }

      if(ipRecv.find("target"))
      {
         target = ipRecv["target"].get<std::string>();
      }
      
      if(target == "") target = current;
      
      target = mx::ioutils::toUpper(target);
      
      if(target != "RESET") return 0;
      
      //Lock the mutex, waiting if necessary
      std::unique_lock<std::mutex> lock(m_indiMutex);

      updateIfChanged(m_indiP_emProtReset, "target", target);
      
      
      if(m_protectionResetConfirmed == 0)
      {
         updateIfChanged(m_indiP_emProtReset, "current", std::string("CONFIRM"));
       
         m_protectionResetConfirmed = 1;
         
         m_protectionResetReqTime = mx::get_curr_time();
         
         log<text_log>("protection reset requested", logPrio::LOG_NOTICE);
         
         return 0;
      }
      
      //If here, this is a confirmation.
      return resetEMProtection();

      
   }
   return -1;
}


INDI_NEWCALLBACK_DEFN(ocam2KCtrl, m_indiP_emGain)(const pcf::IndiProperty &ipRecv)
{
   if(MagAOXAppT::m_powerState == 0) return 0;
   
   if (ipRecv.getName() == m_indiP_emGain.getName())
   {
      unsigned current = 0, target = 0;

      if(ipRecv.find("current"))
      {
         current = ipRecv["current"].get<unsigned>();
      }

      if(ipRecv.find("target"))
      {
         target = ipRecv["target"].get<unsigned>();
      }
      
      if(target == 0) target = current;
      
      if(target == 0) return 0;
      
      //Lock the mutex, waiting if necessary
      std::unique_lock<std::mutex> lock(m_indiMutex);

      updateIfChanged(m_indiP_emGain, "target", target);
      
      return setEMGain(target);
      
   }
   return -1;
}


inline
int ocam2KCtrl::checkRecordTimes()
{
   return telemeter<ocam2KCtrl>::checkRecordTimes(ocam_temps());
}
   
inline
int ocam2KCtrl::recordTelem( const ocam_temps * )
{
   return recordTemps(true);
}
 
inline
int ocam2KCtrl::recordTemps( bool force )
{
   static ocamTemps lastTemps;
   
   if(!(lastTemps == m_temps) || force)
   {
      telem<ocam_temps>({m_temps.CCD, m_temps.CPU, m_temps.POWER, m_temps.BIAS, m_temps.WATER, m_temps.LEFT, m_temps.RIGHT, m_temps.COOLING_POWER});
   }
   
   return 0;
} 
   
}//namespace app
} //namespace MagAOX
#endif
