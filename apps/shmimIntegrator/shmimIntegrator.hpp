/** \file shmimIntegrator.hpp
  * \brief The MagAO-X generic ImageStreamIO stream integrator
  *
  * \ingroup app_files
  */

#ifndef shmimIntegrator_hpp
#define shmimIntegrator_hpp

#include <limits>

#include <mx/improc/eigenCube.hpp>
#include <mx/improc/eigenImage.hpp>

#include "../../libMagAOX/libMagAOX.hpp" //Note this is included on command line to trigger pch
#include "../../magaox_git_version.h"

namespace MagAOX
{
namespace app
{

   
/** \defgroup shmimIntegrator ImageStreamIO Stream Integrator
  * \brief Integrates (i.e. averages) an ImageStreamIO image stream.
  *
  * <a href="../handbook/apps/shmimIntegrator.html">Application Documentation</a>
  *
  * \ingroup apps
  *
  */

/** \defgroup shmimIntegrator_files ImageStreamIO Stream Integrator
  * \ingroup shmimIntegrator
  */

/** MagAO-X application to control integrating (averaging) an ImageStreamIO stream
  *
  * \ingroup shmimIntegrator
  * 
  */
class shmimIntegrator : public MagAOXApp<true>, public dev::shmimMonitor<shmimIntegrator>, public dev::frameGrabber<shmimIntegrator>
{

   //Give the test harness access.
   friend class shmimIntegrator_test;

   friend class dev::shmimMonitor<shmimIntegrator>;
   friend class dev::frameGrabber<shmimIntegrator>;
   
   //The base shmimMonitor type
   typedef dev::shmimMonitor<shmimIntegrator> shmimMonitorT;
   
   //The base frameGrabber type
   typedef dev::frameGrabber<shmimIntegrator> frameGrabberT;
   
   ///Floating point type in which to do all calculations.
   typedef float realT;
   
protected:

   /** \name Configurable Parameters
     *@{
     */
   
   unsigned m_nAverage {10}; ///< The number of frames to average.
   
   unsigned m_nUpdate {0}; ///< The rate at which to update the average.  If m_nUpdate < m_nAverage then this is a moving averager.
   
   ///@}

   mx::improc::eigenCube<realT> m_accumImages; ///< Cube used to accumulate images
   
   mx::improc::eigenImage<realT> m_avgImage; ///< The average image.

   unsigned m_nprocessed {0};
   size_t m_currImage {0};
   size_t m_sinceUpdate {0};
   bool m_updated {false};
   
   sem_t m_smSemaphore; ///< Semaphore used to synchronize the fg thread and the sm thread.
   
   realT (*pixget)(void *, size_t) {nullptr}; ///< Pointer to a function to extract the image data as our desired type realT.
   
public:
   /// Default c'tor.
   shmimIntegrator();

   /// D'tor, declared and defined for noexcept.
   ~shmimIntegrator() noexcept
   {}

   virtual void setupConfig();

   /// Implementation of loadConfig logic, separated for testing.
   /** This is called by loadConfig().
     */
   int loadConfigImpl( mx::app::appConfigurator & _config /**< [in] an application configuration from which to load values*/);

   virtual void loadConfig();

   /// Startup function
   /**
     *
     */
   virtual int appStartup();

   /// Implementation of the FSM for shmimIntegrator.
   /** 
     * \returns 0 on no critical error
     * \returns -1 on an error requiring shutdown
     */
   virtual int appLogic();

   /// Shutdown the app.
   /** 
     *
     */
   virtual int appShutdown();

   int allocate();
   
   int processImage( char* curr_src);
   
protected:

   /** \name dev::frameGrabber interface
     *
     * @{
     */
   
   /// Implementation of the framegrabber configureAcquisition interface
   /** 
     * \returns 0 on success
     * \returns -1 on error
     */
   int configureAcquisition();
   
   /// Implementation of the framegrabber startAcquisition interface
   /** 
     * \returns 0 on success
     * \returns -1 on error
     */
   int startAcquisition();
   
   /// Implementation of the framegrabber acquireAndCheckValid interface
   /** 
     * \returns 0 on success
     * \returns -1 on error
     */
   int acquireAndCheckValid();
   
   /// Implementation of the framegrabber loadImageIntoStream interface
   /** 
     * \returns 0 on success
     * \returns -1 on error
     */
   int loadImageIntoStream( void * dest  /**< [in] */);
   
   /// Implementation of the framegrabber reconfig interface
   /** 
     * \returns 0 on success
     * \returns -1 on error
     */
   int reconfig();
   
   ///@}
   
   pcf::IndiProperty m_indiP_nAverage;
   
   pcf::IndiProperty m_indiP_nUpdate;
   
   INDI_NEWCALLBACK_DECL(shmimIntegrator, m_indiP_nAverage);
   INDI_NEWCALLBACK_DECL(shmimIntegrator, m_indiP_nUpdate);
};

inline
shmimIntegrator::shmimIntegrator() : MagAOXApp(MAGAOX_CURRENT_SHA1, MAGAOX_REPO_MODIFIED)
{
   
   return;
}

inline
void shmimIntegrator::setupConfig()
{
   shmimMonitorT::setupConfig(config);
   
   frameGrabberT::setupConfig(config);
   
   config.add("integrator.nAverage", "", "integrator.nAverage", argType::Required, "integrator", "nAverage", false, "string", "The default number of frames to average.  Can be changed via INDI.");
   
   config.add("integrator.nUpdate", "", "integrator.nUpdate", argType::Required, "integrator", "nUpdate", false, "string", "The rate at which to update the average.  If m_nUpdate < m_nAverage then this is a moving averager.");
}

inline
int shmimIntegrator::loadConfigImpl( mx::app::appConfigurator & _config )
{
   
   shmimMonitorT::loadConfig(config);
   frameGrabberT::loadConfig(config);
   
   _config(m_nAverage, "integrator.nAverage");
   
   m_nUpdate=m_nAverage;
   
   _config(m_nUpdate, "integrator.nUpdate");
   
   return 0;
}

inline
void shmimIntegrator::loadConfig()
{
   loadConfigImpl(config);
}

inline
int shmimIntegrator::appStartup()
{
   
   createStandardIndiNumber<unsigned>( m_indiP_nAverage, "nAverage", 1, std::numeric_limits<unsigned>::max(), 1, "%u");
   updateIfChanged(m_indiP_nAverage, "current", m_nAverage, INDI_IDLE);
   updateIfChanged(m_indiP_nAverage, "target", m_nAverage, INDI_IDLE);
   
   if( registerIndiPropertyNew( m_indiP_nAverage, INDI_NEWCALLBACK(m_indiP_nAverage)) < 0)
   {
      log<software_error>({__FILE__,__LINE__});
      return -1;
   }
   
   createStandardIndiNumber<unsigned>( m_indiP_nUpdate, "nUpdate", 1, std::numeric_limits<unsigned>::max(), 1, "%u");
   updateIfChanged(m_indiP_nUpdate, "current", m_nUpdate, INDI_IDLE);
   updateIfChanged(m_indiP_nUpdate, "target", m_nUpdate, INDI_IDLE);
   
   if( registerIndiPropertyNew( m_indiP_nUpdate, INDI_NEWCALLBACK(m_indiP_nUpdate)) < 0)
   {
      log<software_error>({__FILE__,__LINE__});
      return -1;
   }
   
   if(sem_init(&m_smSemaphore, 0,0) < 0)
   {
      log<software_critical>({__FILE__, __LINE__, errno,0, "Initializing S.M. semaphore"});
      return -1;
   }
   
   if(shmimMonitorT::appStartup() < 0)
   {
      return log<software_error,-1>({__FILE__, __LINE__});
   }
   
   if(frameGrabberT::appStartup() < 0)
   {
      return log<software_error,-1>({__FILE__, __LINE__});
   }
   
   state(stateCodes::OPERATING);
    
   return 0;
}

inline
int shmimIntegrator::appLogic()
{
   if( shmimMonitorT::appLogic() < 0)
   {
      return log<software_error,-1>({__FILE__,__LINE__});
   }
   
   if( frameGrabberT::appLogic() < 0)
   {
      return log<software_error,-1>({__FILE__,__LINE__});
   }
   
   std::unique_lock<std::mutex> lock(m_indiMutex);
   
   if(shmimMonitorT::updateINDI() < 0)
   {
      log<software_error>({__FILE__, __LINE__});
   }
   
   if(frameGrabberT::updateINDI() < 0)
   {
      log<software_error>({__FILE__, __LINE__});
   }
      
  
   
   
   return 0;
}

inline
int shmimIntegrator::appShutdown()
{
   shmimMonitorT::appShutdown();
   
   frameGrabberT::appShutdown();
   
   return 0;
}

inline
int shmimIntegrator::allocate()
{
   std::unique_lock<std::mutex> lock(m_indiMutex);
   
   m_accumImages.resize(shmimMonitorT::m_width, shmimMonitorT::m_height, m_nAverage);
   m_accumImages.setZero();
   
   m_nprocessed = 0;
   m_currImage = 0;
   m_sinceUpdate = 0;
   
   m_avgImage.resize(shmimMonitorT::m_width, shmimMonitorT::m_height);
   
   pixget = getPixPointer<realT>(shmimMonitorT::m_dataType);
   
   if(pixget == nullptr)
   {
      log<software_error>({__FILE__, __LINE__, "bad data type"});
      return -1;
   }
   
   updateIfChanged(m_indiP_nAverage, "current", m_nAverage, INDI_IDLE);
   updateIfChanged(m_indiP_nAverage, "target", m_nAverage, INDI_IDLE);
   
   updateIfChanged(m_indiP_nUpdate, "current", m_nUpdate, INDI_IDLE);
   updateIfChanged(m_indiP_nUpdate, "target", m_nUpdate, INDI_IDLE);
   
   m_reconfig = true;
   
   return 0;
}
   
inline
int shmimIntegrator::processImage( char* curr_src )
{
  
   realT * data = m_accumImages.image(m_currImage).data();
   
   for(unsigned nn=0; nn < shmimMonitorT::m_width*shmimMonitorT::m_height; ++nn)
   {
      //data[nn] = *( (int16_t * ) (curr_src + nn*shmimMonitorT::m_typeSize));
      data[nn] = pixget(curr_src, nn);
   }
   ++m_nprocessed;
   ++m_currImage;
   if(m_currImage >= m_nAverage) m_currImage = 0;
      
   if(m_nprocessed < m_nAverage) //Check that we are burned in on first pass through cube
   {
      return 0;
   }

   ++m_sinceUpdate;

   if(m_sinceUpdate >= m_nUpdate)
   {
      if(m_updated)
      {
         return 0; //In case f.g. thread is behind, we skip and come back.
      }
      //Don't use eigenCube functions to avoid any omp 
      m_avgImage.setZero();
      for(size_t n =0; n < m_nAverage; ++n)
      {
         for(size_t ii=0; ii< shmimMonitorT::m_width; ++ii)
         {
            for(size_t jj=0; jj< shmimMonitorT::m_height; ++jj)
            {
               m_avgImage(ii,jj) += m_accumImages.image(n)(ii,jj);
            }
         }
      }
      m_avgImage /= m_nAverage;
      
      
      m_updated = true;
      
      //Now tell the f.g. to get going
      if(sem_post(&m_smSemaphore) < 0)
      {
         log<software_critical>({__FILE__, __LINE__, errno, 0, "Error posting to semaphore"});
         return -1;
      }
         
      m_sinceUpdate = 0;
   }

   return 0;
}

inline
int shmimIntegrator::configureAcquisition()
{
   std::unique_lock<std::mutex> lock(m_indiMutex);
   
   if(shmimMonitorT::m_width==0 || shmimMonitorT::m_height==0 || shmimMonitorT::m_dataType == 0)
   {
      //This means we haven't connected to the stream to average. so wait.
      sleep(1);
      return -1;
   }
   
   frameGrabberT::m_width = shmimMonitorT::m_width;
   frameGrabberT::m_height = shmimMonitorT::m_height;
   frameGrabberT::m_dataType = _DATATYPE_FLOAT;
   
   std::cerr << "shmimMonitorT::m_dataType: " << (int) shmimMonitorT::m_dataType << "\n";
   std::cerr << "frameGrabberT::m_dataType: " << (int) frameGrabberT::m_dataType << "\n";
   
   return 0;
}

inline
int shmimIntegrator::startAcquisition()
{
   return 0;
}

inline
int shmimIntegrator::acquireAndCheckValid()
{
   timespec ts;
         
   if(clock_gettime(CLOCK_REALTIME, &ts) < 0)
   {
      log<software_critical>({__FILE__,__LINE__,errno,0,"clock_gettime"}); 
      return -1;
   }
         
   ts.tv_sec += 1;
        
   if(sem_timedwait(&m_smSemaphore, &ts) == 0)
   {
      if( m_updated )
      {
         clock_gettime(CLOCK_REALTIME, &m_currImageTimestamp);
         return 0;
      }
      else
      {
         return 1;
      }
   }
   else
   {
      return 1;
   }
}

inline
int shmimIntegrator::loadImageIntoStream(void * dest)
{
   memcpy(dest, m_avgImage.data(), shmimMonitorT::m_width*shmimMonitorT::m_height*frameGrabberT::m_typeSize  ); 
   m_updated = false;
   return 0;
}

inline
int shmimIntegrator::reconfig()
{
   return 0;
}

INDI_NEWCALLBACK_DEFN(shmimIntegrator, m_indiP_nAverage)(const pcf::IndiProperty &ipRecv)
{
   if(ipRecv.getName() != m_indiP_nAverage.getName())
   {
      log<software_error>({__FILE__, __LINE__, "invalid indi property received"});
      return -1;
   }
   
   unsigned target;
   
   if( indiTargetUpdate( m_indiP_nAverage, target, ipRecv, true) < 0)
   {
      log<software_error>({__FILE__,__LINE__});
      return -1;
   }
   
   m_nAverage = target;
   
   m_restart = true;
   
   log<text_log>("set nAverage to " + std::to_string(m_nAverage), logPrio::LOG_NOTICE);
   
   return 0;
}

INDI_NEWCALLBACK_DEFN(shmimIntegrator, m_indiP_nUpdate)(const pcf::IndiProperty &ipRecv)
{
   if(ipRecv.getName() != m_indiP_nUpdate.getName())
   {
      log<software_error>({__FILE__, __LINE__, "invalid indi property received"});
      return -1;
   }
   
   unsigned target;
   
   if( indiTargetUpdate( m_indiP_nUpdate, target, ipRecv, true) < 0)
   {
      log<software_error>({__FILE__,__LINE__});
      return -1;
   }
   
   m_nUpdate = target;
   
   m_restart = true;
   
   log<text_log>("set nUpdate to " + std::to_string(m_nUpdate), logPrio::LOG_NOTICE);
   
   return 0;
}

} //namespace app
} //namespace MagAOX

#endif //shmimIntegrator_hpp
