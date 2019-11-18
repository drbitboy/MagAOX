/** \file tcsInterface.hpp
  * \brief The MagAO-X TCS Interface header file
  *
  * \ingroup tcsInterface_files
  */

#ifndef tcsInterface_hpp
#define tcsInterface_hpp

#include <cmath>

#include "../../libMagAOX/libMagAOX.hpp" //Note this is included on command line to trigger pch
#include "../../magaox_git_version.h"


#include "../../libMagAOX/app/dev/telemeter.hpp"


/** \defgroup tcsInterface
  * \brief The MagAO-X application to do interface with the Clay TCS
  *
  * <a href="../handbook/apps/XXXXXX.html">Application Documentation</a>
  *
  * \ingroup apps
  *
  */

/** \defgroup tcsInterface_files
  * \ingroup tcsInterface
  */

namespace MagAOX
{
namespace app
{

/// The MagAO-X Clay Telescope TCS Interface
/** 
  * \ingroup tcsInterface
  */
class tcsInterface : public MagAOXApp<true>, public dev::ioDevice, public dev::telemeter<tcsInterface>
{

   //Give the test harness access.
   friend class tcsInterface_test;

   friend class dev::telemeter<tcsInterface>;
   
   
protected:

   /** \name Configurable Parameters
     *@{
     */
   
   std::string m_deviceAddr {"localhost"}; ///< The device address
   int m_devicePort {5811}; ///< The device port
   
   ///@}


   tty::netSerial m_sock; 
   
   //Telescope position:
   double m_telEpoch {0};
   double m_telRA {0};
   double m_telDec {0};
   double m_telEl {0};
   double m_telHA {0};
   double m_telAM {0};
   double m_telRotOff {0};
   
   pcf::IndiProperty m_indiP_telpos;
   
   //Telescope Data:
   int m_telROI {0};
   int m_telTracking {0};
   int m_telGuiding {0};
   int m_telSlewing {0};
   int m_telGuiderMoving {0};
   double m_telAz {0};
   double m_telZd {0};
   double m_telPA {0};
   double m_telDomeAz {0};
   int m_telDomeStat {0};
   
   pcf::IndiProperty m_indiP_teldata;
   
   double m_catRA {0};
   double m_catDec {0};
   double m_catEp {0};
   double m_catRo {0};
   std::string m_catRm;
   std::string m_catObj;
   
   pcf::IndiProperty m_indiP_catalog; ///< INDI Property for the catalog text information
   pcf::IndiProperty m_indiP_catdata; ///< INDI Property for the catalog data
   
   double m_telSecZ {0};
   double m_telEncZ {0};
   double m_telSecX {0};
   double m_telEncX {0};
   double m_telSecY {0};
   double m_telEncY {0};
   double m_telSecH {0};
   double m_telEncH {0};
   double m_telSecV {0};
   double m_telEncV {0};
   
   pcf::IndiProperty m_indiP_vaneend; ///< INDI Property for the vane end positions

public:
   /// Default c'tor.
   tcsInterface();

   /// D'tor, declared and defined for noexcept.
   ~tcsInterface() noexcept
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

   /// Implementation of the FSM for tcsInterface.
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

   int getMagTelStatus( std::string & response,
                        const std::string &statreq
                      );

   int parse_xms( double &x, 
                  double &m, 
                  double &s,
                  const std::string & xmsstr
                );
   
   std::vector<std::string> parse_teldata( std::string &tdat );
   
   
   //The "dump" commands:
   
   int getTelPos();
   int getTelData();
   int getCatData();
   int getVaneData();
   
   int updateINDI();
   
   /** \name Telemeter Interface
     * @{
     */ 
   int checkRecordTimes();
   
   int recordTelem( const telem_telpos * );
   
   int recordTelem( const telem_teldata * );
   
   int recordTelPos(bool force = false);
   
   int recordTelData(bool force = false);
   ///@}
   
   int m_loopState = 0;
   pcf::IndiProperty m_indiP_loopState; ///< Property used to report the loop state
   
   INDI_SETCALLBACK_DECL(tcsInterface, m_indiP_loopState);
   
   
   /** \name Woofer Offloading
     * Handling of offloads from the average woofer shape to the telescope
     * @{
     */
   
   bool m_offloadThreadInit {true}; ///< Initialization flag for the offload thread.
   
   std::thread m_offloadThread; ///< The offloading thread.

   /// Offload thread starter function
   static void offloadThreadStart( tcsInterface * t /**< [in] pointer to this */);
   
   /// Offload thread function
   /** Runs until m_shutdown is true.
     */
   void offloadThreadExec();
   
   int doTToffload( float TT_0,
                    float TT_1
                  );
   
   int sendTToffload( float TT_0,
                    float TT_1
                  );
   
   int doFoffload( float F_0 );
   
   int sendFoffload( float F_0 );
   
   
   pcf::IndiProperty m_indiP_offloadCoeffs; ///< Property used to report the latest woofer modal coefficients for offloading
   
   INDI_SETCALLBACK_DECL(tcsInterface, m_indiP_offloadCoeffs);
   
   std::vector<std::vector<float>> m_offloadRequests;
   size_t m_firstRequest {0};
   size_t m_lastRequest {std::numeric_limits<size_t>::max()};
   size_t m_nRequests {0};
   size_t m_last_nRequests {0};
   
   //The TT control matrix
   float m_offlTT_C_00 {1};
   float m_offlTT_C_01 {0};
   float m_offlTT_C_10 {1};
   float m_offlTT_C_11 {0};
   
   bool m_offlTT_enabled {false};
   bool m_offlTT_dump {false};
   float m_offlTT_avgInt {1.0};
   float m_offlTT_gain {0.1};
   float m_offlTT_thresh {0.1};
   
   pcf::IndiProperty m_indiP_offlTTenable;
   INDI_NEWCALLBACK_DECL(tcsInterface, m_indiP_offlTTenable);
   
   pcf::IndiProperty m_indiP_offlTTdump;
   INDI_NEWCALLBACK_DECL(tcsInterface, m_indiP_offlTTdump);
   
   pcf::IndiProperty m_indiP_offlTTavgInt;
   INDI_NEWCALLBACK_DECL(tcsInterface, m_indiP_offlTTavgInt);
   
   pcf::IndiProperty m_indiP_offlTTgain;
   INDI_NEWCALLBACK_DECL(tcsInterface, m_indiP_offlTTgain);
   
   pcf::IndiProperty m_indiP_offlTTthresh;
   INDI_NEWCALLBACK_DECL(tcsInterface, m_indiP_offlTTthresh);
   
   //The Focus control constant
   float m_offlCFocus_00 {1};
   
   bool m_offlF_enabled {false};
   bool m_offlF_dump {false};
   float m_offlF_avgInt {1.0};
   float m_offlF_gain {0.1};
   float m_offlF_thresh {0.1};
   
   pcf::IndiProperty m_indiP_offlFenable;
   INDI_NEWCALLBACK_DECL(tcsInterface, m_indiP_offlFenable);
   
   pcf::IndiProperty m_indiP_offlFdump;
   INDI_NEWCALLBACK_DECL(tcsInterface, m_indiP_offlFdump);
   
   pcf::IndiProperty m_indiP_offlFavgInt;
   INDI_NEWCALLBACK_DECL(tcsInterface, m_indiP_offlFavgInt);
   
   pcf::IndiProperty m_indiP_offlFgain;
   INDI_NEWCALLBACK_DECL(tcsInterface, m_indiP_offlFgain);
   
   pcf::IndiProperty m_indiP_offlFthresh;
   INDI_NEWCALLBACK_DECL(tcsInterface, m_indiP_offlFthresh);
   
   
   
   
   
   float m_offlCComa_00 {1};
   float m_offlCComa_01 {0};
   float m_offlCComa_10 {1};
   float m_offlCComa_11 {0};
   
   ///@}
};

inline
tcsInterface::tcsInterface() : MagAOXApp(MAGAOX_CURRENT_SHA1, MAGAOX_REPO_MODIFIED)
{
   return;
}

inline
void tcsInterface::setupConfig()
{
   config.add("offload.TT_avgInt", "", "offload.TT_avgInt", argType::Required, "offload", "TT_avgInt", false, "float", "Woofer to Telescope T/T offload averaging interval [sec] ");
   config.add("offload.TT_gain", "", "offload.TT_gain", argType::Required, "offload", "TT_gain", false, "float", "Woofer to Telescope T/T offload gain");
   config.add("offload.TT_thresh", "", "offload.TT_thresh", argType::Required, "offload", "TT_thresh", false, "float", "Woofer to Telescope T/T offload threshold");
   
   config.add("offload.TT_C_00", "", "offload.TT_C_00", argType::Required, "offload", "TT_C_00", false, "float", "Woofer to Telescope T/T offload control matrix [0,0] of a 2x2 matrix");
   config.add("offload.TT_C_01", "", "offload.TT_C_01", argType::Required, "offload", "TT_C_01", false, "float", "Woofer to Telescope T/T offload control matrix [0,1] of a 2x2 matrix ");
   config.add("offload.TT_C_10", "", "offload.TT_C_10", argType::Required, "offload", "TT_C_10", false, "float", "Woofer to Telescope T/T offload control matrix [1,0] of a 2x2 matrix ");
   config.add("offload.TT_C_11", "", "offload.TT_C_11", argType::Required, "offload", "TT_C_11", false, "float", "Woofer to Telescope T/T offload control matrix [1,1] of a 2x2 matrix ");
   
   
   config.add("offload.F_avgInt", "", "offload.F_avgInt", argType::Required, "offload", "F_avgInt", false, "float", "Woofer to Telescope Focus offload averaging interval [sec] ");
   config.add("offload.F_gain", "", "offload.F_gain", argType::Required, "offload", "F_gain", false, "float", "Woofer to Telescope Focus offload gain");
   config.add("offload.F_thresh", "", "offload.F_thresh", argType::Required, "offload", "F_thresh", false, "float", "Woofer to Telescope Focus offload threshold");
   
   config.add("offload.CFocus00", "", "offload.CFocus00", argType::Required, "offload", "CFocus00", false, "float", "Woofer to Telescope Focus offload control scale factor.");
   
   config.add("offload.CComa00", "", "offload.CComa00", argType::Required, "offload", "CComa00", false, "float", "Woofer to Telescope Coma offload control matrix [0,0] of a 2x2 matrix");
   config.add("offload.CComa01", "", "offload.CComa01", argType::Required, "offload", "CComa01", false, "float", "Woofer to Telescope Coma offload control matrix [0,1] of a 2x2 matrix ");
   config.add("offload.CComa10", "", "offload.CComa10", argType::Required, "offload", "CComa10", false, "float", "Woofer to Telescope Coma offload control matrix [1,0] of a 2x2 matrix ");
   config.add("offload.CComa11", "", "offload.CComa11", argType::Required, "offload", "CComa11", false, "float", "Woofer to Telescope Coma offload control matrix [1,1] of a 2x2 matrix ");
   
   
   dev::ioDevice::setupConfig(config);
   dev::telemeter<tcsInterface>::setupConfig(config);
}

inline
int tcsInterface::loadConfigImpl( mx::app::appConfigurator & _config )
{
   _config(m_offlTT_avgInt, "offload.TT_avgInt");
   _config(m_offlTT_gain, "offload.TT_gain");
   _config(m_offlTT_thresh, "offload.TT_thresh");
   
   _config(m_offlTT_C_00, "offload.TT_C_00");
   _config(m_offlTT_C_01, "offload.TT_C_01");
   _config(m_offlTT_C_10, "offload.TT_C_10");
   _config(m_offlTT_C_11, "offload.TT_C_11");
   
   _config(m_offlF_avgInt, "offload.F_avgInt");
   _config(m_offlF_gain, "offload.F_gain");
   _config(m_offlF_thresh, "offload.F_thresh");
   
   _config(m_offlCFocus_00, "offload.CFocus00");
   
   _config(m_offlCComa_00, "offload.CComa00");
   _config(m_offlCComa_01, "offload.CComa01");
   _config(m_offlCComa_10, "offload.CComa10");
   _config(m_offlCComa_11, "offload.CComa11");
   
   dev::ioDevice::loadConfig(_config);
   
   dev::telemeter<tcsInterface>::loadConfig(_config);
   
   return 0;
}

inline
void tcsInterface::loadConfig()
{
   loadConfigImpl(config);
}

inline
int tcsInterface::appStartup()
{
   createROIndiNumber( m_indiP_telpos, "telpos", "Telscope Position", "TCS");
   indi::addNumberElement<double>( m_indiP_telpos, "epoch", 0, std::numeric_limits<double>::max(), 0, "%0.6f");
   m_indiP_telpos["epoch"] = m_telEpoch;
   indi::addNumberElement<double>( m_indiP_telpos, "ra", 0, 360, 0, "%0.6f");
   m_indiP_telpos["ra"] = m_telRA;
   indi::addNumberElement<double>( m_indiP_telpos, "dec", -90, 90, 0, "%0.6f");
   m_indiP_telpos["dec"] = m_telDec;
   indi::addNumberElement<double>( m_indiP_telpos, "el", 0, 90, 0, "%0.6f");
   m_indiP_telpos["el"] = m_telEl;
   indi::addNumberElement<double>( m_indiP_telpos, "ha", -180, 160, 0, "%0.6f");
   m_indiP_telpos["ha"] = m_telHA;
   indi::addNumberElement<double>( m_indiP_telpos, "am", 0, 4, 0, "%0.2f");
   m_indiP_telpos["am"] = m_telAM;
   indi::addNumberElement<double>( m_indiP_telpos, "rotoff", 0, 360, 0, "%0.6f");
   m_indiP_telpos["rotoff"] = m_telRotOff;
   
   registerIndiPropertyReadOnly(m_indiP_telpos);
   
   
   createROIndiNumber( m_indiP_teldata, "teldata", "Telscope Data", "TCS");
   indi::addNumberElement<int>( m_indiP_teldata, "roi", 0, 10, 1, "%d");
   m_indiP_teldata["roi"] = m_telROI;
   indi::addNumberElement<int>( m_indiP_teldata, "tracking", 0, 1, 1, "%d");
   m_indiP_teldata["tracking"] = m_telTracking;
   indi::addNumberElement<int>( m_indiP_teldata, "guiding", 0, 1, 1, "%d");
   m_indiP_teldata["guiding"] = m_telGuiding;
   indi::addNumberElement<int>( m_indiP_teldata, "slewing", 0, 1, 1, "%d");
   m_indiP_teldata["slewing"] = m_telSlewing;
   indi::addNumberElement<int>( m_indiP_teldata, "guider_moving", 0, 1, 1, "%d");
   m_indiP_teldata["guider_moving"] = m_telGuiderMoving;
   indi::addNumberElement<double>( m_indiP_teldata, "az", 0, 360, 0, "%0.6f");
   m_indiP_teldata["az"] = m_telAz;
   indi::addNumberElement<double>( m_indiP_teldata, "zd", 0, 90, 0, "%0.6f");
   m_indiP_teldata["zd"] = m_telZd;
   indi::addNumberElement<double>( m_indiP_teldata, "pa", 0, 360, 0, "%0.6f");
   m_indiP_teldata["pa"] = m_telPA;
   indi::addNumberElement<double>( m_indiP_teldata, "dome_az", 0, 360, 0, "%0.6f");
   m_indiP_teldata["dome_az"] = m_telDomeAz;
   indi::addNumberElement<int>( m_indiP_teldata, "dome_stat", 0, 1, 1, "%d");
   m_indiP_teldata["dome_stat"] = m_telDomeStat;
   
   registerIndiPropertyReadOnly(m_indiP_teldata);
      
   
   createROIndiText( m_indiP_catalog, "catalog", "object", "Catalog Entry", "TCS", "Object Name");
   m_indiP_catalog.add(pcf::IndiElement("rotmode"));
   m_indiP_catalog["rotmode"].setLabel("Rotator Mode");
 
   registerIndiPropertyReadOnly(m_indiP_catalog);
 
   createROIndiNumber( m_indiP_catdata, "catdata", "Catalog Entry Data", "TCS");
   indi::addNumberElement<double>( m_indiP_catdata, "ra", 0, 360, 0, "%0.6f");
   m_indiP_catdata["ra"] = m_catRA;
   indi::addNumberElement<double>( m_indiP_catdata, "dec", -90, 90, 0, "%0.6f");
   m_indiP_catdata["dec"] = m_catDec;
   indi::addNumberElement<double>( m_indiP_catdata, "epoch", 0, std::numeric_limits<double>::max(), 0, "%0.6f");
   m_indiP_catdata["epoch"] = m_catEp;
   indi::addNumberElement<double>( m_indiP_catdata, "rotoff", 0, 360, 0, "%0.6f");
   m_indiP_catdata["rotoff"] = m_catRo;
   
   registerIndiPropertyReadOnly(m_indiP_catdata);
   
   
   
   createROIndiNumber( m_indiP_vaneend, "vaneend", "Vane End Data", "TCS");
   indi::addNumberElement<double>( m_indiP_vaneend, "secz", std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), 0, "%0.6f");
   m_indiP_vaneend["secz"] = m_telSecZ;
   indi::addNumberElement<double>( m_indiP_vaneend, "encz", std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), 0, "%0.6f");
   m_indiP_vaneend["encz"] = m_telEncZ;
   indi::addNumberElement<double>( m_indiP_vaneend, "secx", std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), 0, "%0.6f");
   m_indiP_vaneend["secx"] = m_telSecX;
   indi::addNumberElement<double>( m_indiP_vaneend, "encx", std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), 0, "%0.6f");
   m_indiP_vaneend["encx"] = m_telEncX;
   indi::addNumberElement<double>( m_indiP_vaneend, "secy", std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), 0, "%0.6f");
   m_indiP_vaneend["secy"] = m_telSecY;
   indi::addNumberElement<double>( m_indiP_vaneend, "ency", std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), 0, "%0.6f");
   m_indiP_vaneend["ency"] = m_telEncY;
   indi::addNumberElement<double>( m_indiP_vaneend, "sech", std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), 0, "%0.6f");
   m_indiP_vaneend["sech"] = m_telSecH;
   indi::addNumberElement<double>( m_indiP_vaneend, "ench", std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), 0, "%0.6f");
   m_indiP_vaneend["ench"] = m_telEncH;
   indi::addNumberElement<double>( m_indiP_vaneend, "secv", std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), 0, "%0.6f");
   m_indiP_vaneend["secv"] = m_telSecV;
   indi::addNumberElement<double>( m_indiP_vaneend, "encv", std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), 0, "%0.6f");
   m_indiP_vaneend["encv"] = m_telEncV;
   
   registerIndiPropertyReadOnly(m_indiP_vaneend);
   
   signal(SIGPIPE, SIG_IGN);
   
   
   if(dev::ioDevice::appStartup() < 0)
   {
      return log<software_error,-1>({__FILE__,__LINE__});
   }
   
   if(dev::telemeter<tcsInterface>::appStartup() < 0)
   {
      return log<software_error,-1>({__FILE__,__LINE__});
   }
   
   createStandardIndiRequestSw( m_indiP_offlTTdump, "offlTT_dump");
   if( registerIndiPropertyNew( m_indiP_offlTTdump, st_newCallBack_m_indiP_offlTTdump) < 0)
   {
      log<software_error>({__FILE__,__LINE__});
      return -1;
   }
      
   createStandardIndiToggleSw( m_indiP_offlTTenable, "offlTT_enable");
   if( registerIndiPropertyNew( m_indiP_offlTTenable, st_newCallBack_m_indiP_offlTTenable) < 0)
   {
      log<software_error>({__FILE__,__LINE__});
      return -1;
   }
   
   createStandardIndiNumber( m_indiP_offlTTavgInt, "offlTT_avgInt", 0, 3600, 1, "%d");
   m_indiP_offlTTavgInt["current"].set(m_offlTT_avgInt);
   m_indiP_offlTTavgInt["target"].set(m_offlTT_avgInt);
   if( registerIndiPropertyNew( m_indiP_offlTTavgInt, st_newCallBack_m_indiP_offlTTavgInt) < 0)
   {
      log<software_error>({__FILE__,__LINE__});
      return -1;
   }
   
   createStandardIndiNumber( m_indiP_offlTTgain, "offlTT_gain", 0.0, 1.0, 0.0, "%0.2f");
   m_indiP_offlTTgain["current"].set(m_offlTT_gain);
   m_indiP_offlTTgain["target"].set(m_offlTT_gain);
   if( registerIndiPropertyNew( m_indiP_offlTTgain, st_newCallBack_m_indiP_offlTTgain) < 0)
   {
      log<software_error>({__FILE__,__LINE__});
      return -1;
   }
   
   createStandardIndiNumber( m_indiP_offlTTthresh, "offlTT_thresh", 0.0, 1.0, 0.0, "%0.2f");
   m_indiP_offlTTthresh["current"].set(m_offlTT_thresh);
   m_indiP_offlTTthresh["target"].set(m_offlTT_thresh);
   if( registerIndiPropertyNew( m_indiP_offlTTthresh, st_newCallBack_m_indiP_offlTTthresh) < 0)
   {
      log<software_error>({__FILE__,__LINE__});
      return -1;
   }
   
   createStandardIndiRequestSw( m_indiP_offlFdump, "offlF_dump");
   if( registerIndiPropertyNew( m_indiP_offlFdump, st_newCallBack_m_indiP_offlFdump) < 0)
   {
      log<software_error>({__FILE__,__LINE__});
      return -1;
   }
      
   createStandardIndiToggleSw( m_indiP_offlFenable, "offlF_enable");
   if( registerIndiPropertyNew( m_indiP_offlFenable, st_newCallBack_m_indiP_offlFenable) < 0)
   {
      log<software_error>({__FILE__,__LINE__});
      return -1;
   }
   
   createStandardIndiNumber( m_indiP_offlFavgInt, "offlF_avgInt", 0, 3600, 1, "%d");
   m_indiP_offlFavgInt["current"].set(m_offlF_avgInt);
   m_indiP_offlFavgInt["target"].set(m_offlF_avgInt);
   if( registerIndiPropertyNew( m_indiP_offlFavgInt, st_newCallBack_m_indiP_offlFavgInt) < 0)
   {
      log<software_error>({__FILE__,__LINE__});
      return -1;
   }
   
   createStandardIndiNumber( m_indiP_offlFgain, "offlF_gain", 0.0, 1.0, 0.0, "%0.2f");
   m_indiP_offlFgain["current"].set(m_offlF_gain);
   m_indiP_offlFgain["target"].set(m_offlF_gain);
   if( registerIndiPropertyNew( m_indiP_offlFgain, st_newCallBack_m_indiP_offlFgain) < 0)
   {
      log<software_error>({__FILE__,__LINE__});
      return -1;
   }
   
   createStandardIndiNumber( m_indiP_offlFthresh, "offlF_thresh", 0.0, 1.0, 0.0, "%0.2f");
   m_indiP_offlFthresh["current"].set(m_offlF_thresh);
   m_indiP_offlFthresh["target"].set(m_offlF_thresh);
   if( registerIndiPropertyNew( m_indiP_offlFthresh, st_newCallBack_m_indiP_offlFthresh) < 0)
   {
      log<software_error>({__FILE__,__LINE__});
      return -1;
   }
   
   //Get the loop state for managing offloading
   REG_INDI_SETPROP(m_indiP_loopState, "aoloop", "loopState");
   
   
   m_offloadRequests.resize(5);
   for(size_t n=0; n < m_offloadRequests.size();++n) m_offloadRequests[n].resize(10,0);
   
   
   if(threadStart( m_offloadThread, m_offloadThreadInit, 0, "offload", this, offloadThreadStart) < 0)
   {
      log<software_error>({__FILE__, __LINE__});
      return -1;
   }
   
   
   //Register to receive the coeff updates from Kyle
   REG_INDI_SETPROP(m_indiP_offloadCoeffs, "w2tcsOffloader", "zCoeffs");
   
   
   state(stateCodes::NOTCONNECTED);
   
   
   return 0;
}

inline
int tcsInterface::appLogic()
{
   if(state() == stateCodes::ERROR)
   {
      int rv = m_sock.serialInit(m_deviceAddr.c_str(), m_devicePort);

      if(rv == 0)
      {
         log<text_log>("In state ERROR, not due to loss of connection.  Can not go on.", logPrio::LOG_CRITICAL);
         return -1;
      }
      
      state(stateCodes::NOTCONNECTED);
      return 0;
   }
   
   if( state() == stateCodes::NOTCONNECTED )
   {
      static int lastrv = 0; //Used to handle a change in error within the same state.  Make general?
      static int lasterrno = 0;
       
      
      int rv = m_sock.serialInit(m_deviceAddr.c_str(), m_devicePort);

      if(rv == 0)
      {
         state(stateCodes::CONNECTED);

         if(!stateLogged())
         {
            std::stringstream logs;
            logs << "Connected to " << m_deviceAddr << ":" << m_devicePort;
            log<text_log>(logs.str());
         }
         lastrv = rv;
         lasterrno = errno;
      }
      else
      {
         if(!stateLogged())
         {
            log<text_log>({"Failed to connect to " + m_deviceAddr + ":" + std::to_string(m_devicePort)}, logPrio::LOG_ERROR);
         }
         if( rv != lastrv )
         {
            log<software_error>( {__FILE__,__LINE__, 0, rv,  tty::ttyErrorString(rv)} );
            lastrv = rv;
         }
         if( errno != lasterrno )
         {
            log<software_error>( {__FILE__,__LINE__, errno});
            lasterrno = errno;
         }
         return 0;
      }
   }
   
   if(state() == stateCodes::CONNECTED)
   {
      std::string response;
      
      if(getTelPos() < 0)
      {
         log<text_log>("Error from getTelPos", logPrio::LOG_ERROR);
         return 0; //app state will be set based on what the error was
      }
      
      if(getTelData() < 0)
      {
         log<text_log>("Error from getTelData", logPrio::LOG_ERROR);
         return 0;
      }
      
      if(getCatData() < 0)
      {
         log<text_log>("Error from getTelData", logPrio::LOG_ERROR);
         return 0;
      }
      
      if(getVaneData() < 0)
      {
         log<text_log>("Error from getVaneData", logPrio::LOG_ERROR);
         return 0;
      }
      
      telemeter<tcsInterface>::appLogic();
      
      if(updateINDI() < 0)
      {
         log<text_log>("Error from updateINDI", logPrio::LOG_ERROR);
         return 0;
      }
   }
   
   
   
   return 0;
}

inline
int tcsInterface::appShutdown()
{
   return 0;
}

inline
int tcsInterface::getMagTelStatus( std::string & response,
                                   const std::string &statreq
                                 )
{

   int stat;
   char answer[512];
   std::string statreq_nl;


   //pthread_mutex_lock(&aoiMutex);

   #ifdef LOG_TCS_STATUS
   log<text_log<("Sending status request: " + statreq);
   #endif

   statreq_nl = statreq;
   statreq_nl += '\n';
   stat = m_sock.serialOut(statreq_nl.c_str(), statreq_nl.length());
   
   if(stat != NETSERIAL_E_NOERROR)
   {
      log<text_log>("Error sending status request: " + statreq, logPrio::LOG_ERROR);
      response = "";
      return 0;
   }
   
   stat = m_sock.serialInString(answer, 512, 1000, '\n');
   
   if(stat <= 0)
   {
      log<text_log>("No response received to status request: " + statreq, logPrio::LOG_ERROR);
      response = "";
      return 0;
   }

   char * nl = strchr(answer, '\n');
   if(nl) answer[nl-answer] = '\0';

   #ifdef LOG_TCS_STATUS
   log<text_log>("Received response: " + answer);
   #endif

   response = answer;
   
   return 0;
}

inline
std::vector<std::string> tcsInterface::parse_teldata( std::string &tdat )
{
   std::vector<std::string> vres;

   std::string tok;

   int pos1, pos2;

  
   //skip all leading spaces
   pos1 = tdat.find_first_not_of(" " , 0);
    
   if(pos1 == -1) pos1 = 0;

   pos2 = tdat.find_first_of(" ", pos1);

   while(pos2 > 0)
   {
      tok = tdat.substr(pos1, pos2-pos1);

      vres.push_back(tok);

      //now move past end of current spaces - might be more than one.
      pos1 = pos2;

      pos2 = tdat.find_first_not_of(" ", pos1);

      //and then find the end of this value.
      pos1 = pos2;

      pos2 = tdat.find_first_of(" ", pos1);
   }

   //If there is another value, we pick it up here.
   if(pos1 >= 0)
   {
      pos2 = tdat.length();

      tok = tdat.substr(pos1, pos2-pos1);

      pos2 = tok.find_first_of(" \n\r", 0);

      if(pos2 >= 0) tok.erase(pos2, tok.length()-pos2);

      vres.push_back(tok);
   }

   return vres;
}

inline
int tcsInterface::parse_xms( double &x, 
                             double &m, 
                             double &s,
                             const std::string & xmsstr
                           )
{
   int st, en;

   int sgn = 1;


   st = 0;
   en = xmsstr.find(':', st);
   
   x = strtod(xmsstr.substr(st, en-st).c_str(), 0);

   //Check for negative
   if(std::signbit(x)) sgn = -1;

   st = en + 1;
   
   en = xmsstr.find(':', st);
   
   m = sgn*strtod(xmsstr.substr(st, en-st).c_str(), 0);
   
   st = en+1;
   
   s = sgn*strtod(xmsstr.substr(st, xmsstr.length()-st).c_str(), 0);
   
   return 0;
}

inline
int tcsInterface::getTelPos()
{
   double  h,m,s;

   std::vector<std::string> pdat;
   std::string posstr;
   
   if(getMagTelStatus( posstr, "telpos") < 0)
   {
      state(stateCodes::NOTCONNECTED);
      log<text_log>("Error getting telescope position (telpos)", logPrio::LOG_ERROR);
      return -1;
   }

   pdat = parse_teldata(posstr);

   if(pdat[0] == "-1")
   {
      state(stateCodes::ERROR);
      log<text_log>("Error getting telescope position (telpos): TCS returned -1", logPrio::LOG_ERROR);
      return -1;
   }

   if(pdat.size() != 6)
   {
      state(stateCodes::ERROR);
      log<text_log>("Error getting telescope position (telpos): TCS response wrong size", logPrio::LOG_ERROR);
      return -1;
   }

   if(parse_xms(h,m,s,pdat[0]) != 0)
   {
      log<text_log>("Error parsing telescope RA", logPrio::LOG_ERROR);
      return -1;
   }

   m_telRA = (h + m/60. + s/3600.)*15.;

   if(parse_xms(h,m,s,pdat[1]) != 0)
   {
      log<text_log>("Error parsing telescope Dec", logPrio::LOG_ERROR);
      return -1;
   }
   
   m_telDec = h + m/60. + s/3600.;

   m_telEl = strtod(pdat[1].c_str(),0);// * 3600.;

   m_telEpoch = strtod(pdat[2].c_str(),0);

   if(parse_xms( h, m, s, pdat[3]) != 0)
   {
      log<text_log>("Error parsing telescope HA", logPrio::LOG_ERROR);
      return -1;
   }

   /************ BUG: this won't handle -0!
   */
   m_telHA = h + m/60. + s/3600.;

   m_telAM = strtod(pdat[4].c_str(),0);

   m_telRotOff = strtod(pdat[5].c_str(),0);

   if( recordTelPos() < 0)
   {
      return log<software_error,-1>({__FILE__,__LINE__});
   }
   
   return 0;
}

inline
int tcsInterface::getTelData()
{
   std::string xstr;
   std::vector<std::string> tdat;

   if(getMagTelStatus(xstr, "teldata") < 0)
   {
      state(stateCodes::NOTCONNECTED);
      log<text_log>("Error getting telescope data (teldata)", logPrio::LOG_ERROR);
      return -1;
   }

   tdat = parse_teldata(xstr);

   if(tdat[0] == "-1")
   {
      state(stateCodes::ERROR);
      log<text_log>("Error getting telescope data (teldata): TCS returned -1", logPrio::LOG_ERROR);
      return -1;
   }

   if(tdat.size() != 10)
   {
      state(stateCodes::ERROR);
      log<text_log>("[TCS] Error getting telescope data (teldata): TCS response wrong size", logPrio::LOG_ERROR);
      return -1;
   }


   m_telROI = atoi(tdat[0].c_str());

   //Parse the telguide string
   char bit[2] = {0,0};
   bit[1] = 0;
   bit[0] = tdat[1].c_str()[0];
   m_telTracking= atoi(bit);
   bit[0] = tdat[1].c_str()[1];
   m_telGuiding= atoi(bit);

   //parse the gdrmountmv string
   bit[0] = tdat[2].c_str()[0];
   m_telSlewing= atoi(bit);
   bit[0] = tdat[2].c_str()[1];
   m_telGuiderMoving = atoi(bit);

   //number 3 is mountmv

   m_telAz = strtod(tdat[4].c_str(),0);

   m_telEl = strtod(tdat[5].c_str(),0);

   m_telZd = strtod(tdat[6].c_str(),0);// * 3600.;

   m_telPA = strtod(tdat[7].c_str(),0);

   m_telDomeAz = strtod(tdat[8].c_str(),0);

   m_telDomeStat = atoi(tdat[9].c_str());

   if( recordTelData() < 0)
   {
      return log<software_error,-1>({__FILE__,__LINE__});
   }
   
   return 0;
}

inline
int tcsInterface::getCatData()
{
   double h, m,s;

   std::vector<std::string> cdat;
   std::string cstr;
   
   if( getMagTelStatus(cstr, "catdata") < 0)
   {
      state(stateCodes::NOTCONNECTED);
      log<text_log>("Error getting catalog data (catdata)", logPrio::LOG_ERROR);
      return -1;
   }

   cdat = parse_teldata(cstr);

   if(cdat[0] == "-1")
   {
      state(stateCodes::ERROR);
      log<text_log>("Error getting catalog data (catdata): TCS returned -1", logPrio::LOG_ERROR);
      return -1;
   }

   if(cdat.size() != 6)
   {
      state(stateCodes::ERROR);
      log<text_log>("Error getting catalog data (catdata): TCS response wrong size", logPrio::LOG_ERROR);
      return -1;
   }

   if(parse_xms(h,m,s,cdat[0]) != 0)
   {
      log<text_log>("Error parsing catalg RA", logPrio::LOG_ERROR);
      return -1;
   }

   m_catRA = (h + m/60. + s/3600.)*15.;
   
   if(parse_xms(h,m,s,  cdat[1] ) != 0)
   {
      log<text_log>("Error parsing catalog Dec", logPrio::LOG_ERROR);
      return -1;
   }
   
   m_catDec = h + m/60. + s/3600.;

   m_catEp = strtod(cdat[2].c_str(),0);

   m_catRo = strtod(cdat[3].c_str(),0);

   m_catRm = cdat[4];

   m_catObj = cdat[5];
   
   return 0;
}

int tcsInterface::getVaneData()
{
   std::string xstr;
   std::vector<std::string> vedat;

   if(getMagTelStatus(xstr,"vedata") < 0)
   {
      state(stateCodes::NOTCONNECTED);
      log<text_log>("Error getting telescope secondary positions (vedata)",logPrio::LOG_ERROR);
      return -1;
   }

   vedat = parse_teldata(xstr);

   if(vedat[0] == "-1")
   {
      state(stateCodes::ERROR);
      log<text_log>("Error getting telescope secondary positions (vedata): TCS returned -1",logPrio::LOG_ERROR);
      return -1;
   }

   if(vedat.size() != 10)
   {
      state(stateCodes::ERROR);
      log<text_log>("Error getting telescope secondary positions (vedata): TCS response wrong size",logPrio::LOG_ERROR);
      return -1;
   }


   m_telSecZ = strtod(vedat[0].c_str(),0);
   m_telEncZ = strtod(vedat[1].c_str(),0);
   m_telSecX = strtod(vedat[2].c_str(),0);
   m_telEncX = strtod(vedat[3].c_str(),0);
   m_telSecY = strtod(vedat[4].c_str(),0);
   m_telEncY = strtod(vedat[5].c_str(),0);
   m_telSecH = strtod(vedat[6].c_str(),0);
   m_telEncH = strtod(vedat[7].c_str(),0);
   m_telSecV = strtod(vedat[8].c_str(),0);
   m_telEncV = strtod(vedat[9].c_str(),0);
   return 0;
}

inline
int tcsInterface::updateINDI()
{
   
   try
   {
      m_indiP_telpos["epoch"] = m_telEpoch;
      m_indiP_telpos["ra"] = m_telRA;
      m_indiP_telpos["dec"] = m_telDec;
      m_indiP_telpos["el"] = m_telEl;
      m_indiP_telpos["ha"] = m_telHA;
      m_indiP_telpos["am"] = m_telAM;
      m_indiP_telpos["rotoff"] = m_telRotOff;
   }
   catch(...)
   {
      log<software_error>({__FILE__,__LINE__,"INDI library exception"});
      return -1;
   }
   
   try
   {
      m_indiP_telpos.setState(INDI_OK);
   }
   catch(...)
   {
      log<software_error>({__FILE__,__LINE__,"INDI library exception"});
      return -1;
   }
   
   try
   {
      m_indiDriver->sendSetProperty (m_indiP_telpos);
   }
   catch(...)
   {
      log<software_error>({__FILE__,__LINE__,"INDI library exception"});
      return -1;
   }
   
   try
   {
      m_indiP_teldata["roi"] = m_telROI;
      m_indiP_teldata["tracking"] = m_telTracking;
      m_indiP_teldata["guiding"] = m_telGuiding;
      m_indiP_teldata["slewing"] = m_telSlewing;
      m_indiP_teldata["guider_moving"] = m_telGuiderMoving;
      m_indiP_teldata["az"] = m_telAz;
      m_indiP_teldata["zd"] = m_telZd;
      m_indiP_teldata["pa"] = m_telPA;
      m_indiP_teldata["dome_az"] = m_telDomeAz;
      m_indiP_teldata["dome_stat"] = m_telDomeStat;
   }
   catch(...)
   {
      log<software_error>({__FILE__,__LINE__,"INDI library exception"});
      return -1;
   }
   
   try
   {
      m_indiP_teldata.setState(INDI_OK);
   }
   catch(...)
   {
      log<software_error>({__FILE__,__LINE__,"INDI library exception"});
      return -1;
   }
   
   try
   {
      m_indiDriver->sendSetProperty (m_indiP_teldata);
   }
   catch(...)
   {
      log<software_error>({__FILE__,__LINE__,"INDI library exception"});
      return -1;
   }
   
   try
   {
      m_indiP_catalog["object"] = m_catObj;
      m_indiP_catalog["rotmode"] = m_catRo;
   }
   catch(...)
   {
      log<software_error>({__FILE__,__LINE__,"INDI library exception"});
      return -1;
   }
   
   try
   {
      m_indiP_catalog.setState(INDI_OK);
   }
   catch(...)
   {
      log<software_error>({__FILE__,__LINE__,"INDI library exception"});
      return -1;
   }
   
   try
   {
      m_indiDriver->sendSetProperty (m_indiP_catalog);
   }
   catch(...)
   {
      log<software_error>({__FILE__,__LINE__,"INDI library exception"});
      return -1;
   }
   
   try
   {
      m_indiP_catdata["ra"] = m_catRA;
      m_indiP_catdata["dec"] = m_catDec;
      m_indiP_catdata["epoch"] = m_catEp;
      m_indiP_catdata["rotoff"] = m_catRo;
   }
   catch(...)
   {
      log<software_error>({__FILE__,__LINE__,"INDI library exception"});
      return -1;
   }
   
   try
   {
      m_indiP_catdata.setState(INDI_OK);
   }
   catch(...)
   {
      log<software_error>({__FILE__,__LINE__,"INDI library exception"});
      return -1;
   }
   
   try
   {
      m_indiDriver->sendSetProperty (m_indiP_catdata);
   }
   catch(...)
   {
      log<software_error>({__FILE__,__LINE__,"INDI library exception"});
      return -1;
   }
   
   try
   {
      m_indiP_vaneend["secz"] = m_telSecZ;
      m_indiP_vaneend["encz"] = m_telEncZ;
      m_indiP_vaneend["secx"] = m_telSecX;
      m_indiP_vaneend["encx"] = m_telEncX;
      m_indiP_vaneend["secy"] = m_telSecY;
      m_indiP_vaneend["ency"] = m_telEncY;
      m_indiP_vaneend["sech"] = m_telSecH;
      m_indiP_vaneend["ench"] = m_telEncH;
      m_indiP_vaneend["secv"] = m_telSecV;
      m_indiP_vaneend["encv"] = m_telEncV;
   }
   catch(...)
   {
      log<software_error>({__FILE__,__LINE__,"INDI library exception"});
      return -1;
   }
   
   try
   {
      m_indiP_vaneend.setState(INDI_OK);
   }
   catch(...)
   {
      log<software_error>({__FILE__,__LINE__,"INDI library exception"});
      return -1;
   }
   
   try
   {
      m_indiDriver->sendSetProperty (m_indiP_vaneend);
   }
   catch(...)
   {
      log<software_error>({__FILE__,__LINE__,"INDI library exception"});
      return -1;
   }
   
   try
   {
      if(m_offlTT_dump)
      {
         updateSwitchIfChanged(m_indiP_offlTTdump, "request", pcf::IndiElement::On, INDI_BUSY);
      }
      else
      {
         updateSwitchIfChanged(m_indiP_offlTTdump, "request", pcf::IndiElement::Off, INDI_IDLE);
      }
      
      if(m_offlTT_enabled)
      {
         updateSwitchIfChanged(m_indiP_offlTTenable, "toggle", pcf::IndiElement::On, INDI_BUSY);
      }
      else
      {
         updateSwitchIfChanged(m_indiP_offlTTenable, "toggle", pcf::IndiElement::Off, INDI_IDLE);
      }
      
      updateIfChanged(m_indiP_offlTTavgInt, "current", m_offlTT_avgInt);
      updateIfChanged(m_indiP_offlTTgain, "current", m_offlTT_gain);
      updateIfChanged(m_indiP_offlTTthresh, "current", m_offlTT_thresh);
      
   }
   catch(...)
   {
      log<software_error>({__FILE__,__LINE__,"INDI library exception"});
      return -1;
   }
   
   return 0;
}

inline
int tcsInterface::checkRecordTimes()
{
   return telemeter<tcsInterface>::checkRecordTimes(telem_telpos(), telem_teldata());
}

inline
int tcsInterface::recordTelem( const telem_telpos * )
{
   recordTelPos(true);
   return 0;
}

inline
int tcsInterface::recordTelem( const telem_teldata * )
{
   recordTelData(true);
   return 0;
}
   
inline
int tcsInterface::recordTelPos(bool force)
{
   static double lastEpoch = 0;
   static double lastRA = 0;
   static double lastDec = 0;
   static double lastEl = 0;
   static double lastHA = 0;
   static double lastAM = 0;
   static double lastRotOff = 0;
   
   if( force || lastEpoch != m_telEpoch ||
                lastRA != m_telRA || 
                lastDec != m_telDec ||
                lastEl != m_telEl ||
                lastHA != m_telHA ||
                lastAM != m_telAM ||
                lastRotOff != m_telRotOff )
   {
      telem<telem_telpos>({m_telEpoch, m_telRA, m_telDec, m_telEl, m_telHA, m_telAM, m_telRotOff});
      
      lastEpoch = m_telEpoch;
      lastRA = m_telRA;
      lastDec = m_telDec;
      lastEl = m_telEl;
      lastHA = m_telHA;
      lastAM = m_telAM;
      lastRotOff = m_telRotOff;
   }
   
   return 0;
}

inline
int tcsInterface::recordTelData(bool force)
{
   static int lastROI = -999;
   static int lastTracking = -999;
   static int lastGuiding = -999;
   static int lastSlewing = -999;
   static int lastGuiderMoving = -999;
   static double lastAz = 0;
   static double lastZd = 0;
   static double lastPA = 0;
   static double lastDomeAz = 0;
   static int lastDomeStat = -999;
   
   if( force || lastROI != m_telROI || 
                lastTracking != m_telTracking || 
                lastGuiding != m_telGuiding ||
                lastSlewing != m_telSlewing ||
                lastGuiderMoving != m_telGuiderMoving ||
                lastAz != m_telAz ||
                lastZd != m_telZd ||
                lastPA != m_telPA ||
                lastDomeAz != m_telDomeAz ||
                lastDomeStat != m_telDomeStat )
   {
      telem<telem_teldata>({m_telROI,m_telTracking,m_telGuiding,m_telSlewing,m_telGuiderMoving,m_telAz, m_telZd, m_telPA,m_telDomeAz,m_telDomeStat});
      
      lastROI = m_telROI;
      lastTracking = m_telTracking;
      lastGuiding = m_telGuiding;
      lastSlewing = m_telSlewing;
      lastGuiderMoving = m_telGuiderMoving;
      lastAz = m_telAz;
      lastZd = m_telZd;
      lastPA = m_telPA;
      lastDomeAz = m_telDomeAz;
      lastDomeStat = m_telDomeStat;
   }
   
   return 0;
}

void tcsInterface::offloadThreadStart( tcsInterface * t )
{
   t->offloadThreadExec();
}

void tcsInterface::offloadThreadExec( )
{
   static int last_loopState = -1;
   
   while( m_offloadThreadInit == true && shutdown() == 0)
   {
      sleep(1);
   }
   
   
   float avg_TT_0;
   float avg_TT_1;
   int sincelast_TT = 0;
   
   float avg_F_0;
   int sincelast_F = 0;
   
   while(shutdown() == 0)
   {
      //Check if loop open
      if(m_loopState == 0)
      {
         //If this is new, then reset the averaging buffer
         if(m_loopState != last_loopState)
         {
            std::cerr << "resetting\n";
            m_firstRequest = 0;
            m_lastRequest = std::numeric_limits<size_t>::max();
            m_nRequests = 0;
            m_last_nRequests = 0;
            
         }         
         sleep(1);
         last_loopState = m_loopState;
         continue;
      }
      
      //Check if loop paused
      if(m_loopState == 1)
      {
         sleep(1);
         last_loopState = m_loopState;
         continue;
      }
 
      //Ok loop closed
      
      if(m_firstRequest == m_lastRequest) continue; //this really should mutexed instead
      
      //If we got a new offload request, process it
      if(m_last_nRequests != m_nRequests)
      {
         std::cerr << m_firstRequest << " " << m_lastRequest << " " << m_nRequests << std::endl;
         
         ///\todo offloading: These sections ought to be separate functions for clarity
         /* --- TT --- */ 
         avg_TT_0 = 0;
         avg_TT_1 = 0;
         
         int navg = 0;
 
         size_t i = m_lastRequest;
         
         for(size_t n=0; n < m_offlTT_avgInt; ++n)
         {
            avg_TT_0 += m_offloadRequests[0][i];
            avg_TT_1 += m_offloadRequests[1][i];
            ++navg;
            
            if(i== m_firstRequest) break;
            
            if(i == 0) i = m_offloadRequests[0].size()-1;
            else --i;
         }
         
         avg_TT_0 /= navg;
         avg_TT_1 /= navg;
   
         ++sincelast_TT;
         if(sincelast_TT > m_offlTT_avgInt)
         {
            doTToffload(avg_TT_0, avg_TT_1);
            sincelast_TT = 0;
         }
         
         
         /* --- Focus --- */
         avg_F_0 = 0;
         
         navg = 0;
 
         i = m_lastRequest;
         
         for(size_t n=0; n < m_offlF_avgInt; ++n)
         {
            avg_F_0 += m_offloadRequests[2][i];
            ++navg;
            
            if(i== m_firstRequest) break;
            
            if(i == 0) i = m_offloadRequests[0].size()-1;
            else --i;
         }
         
         avg_F_0 /= navg;
   
         ++sincelast_F;
         if(sincelast_F > m_offlF_avgInt)
         {
            doFoffload(avg_F_0);
            sincelast_F = 0;
         }
         
         
         
         
         
         m_last_nRequests = m_nRequests;
      }
      last_loopState = m_loopState;

      sleep(1);
      

   }
   
   return;
}

int tcsInterface::doTToffload( float tt_0,
                               float tt_1
                             )
{
   if(m_offlTT_dump)
   {
      std::cerr << "dumping: " << tt_0 << " " << tt_1 << "\n";
      sendTToffload(tt_0, tt_1);
      m_offlTT_dump = false;
   }
   else 
   {
      tt_0 *= m_offlTT_gain;
      tt_1 *= m_offlTT_gain;
      
      std::cerr << tt_0 << " " << tt_1 << "\n";
      
      if(fabs(tt_0) < m_offlTT_thresh) tt_0 = 0;
      if(fabs(tt_1) < m_offlTT_thresh) tt_1 = 0;

            
      if(tt_0 ==0 && tt_1 == 0)
      {
         std::cerr << "TT offload below threshold\n";
      }
      else if(m_offlTT_enabled)
      {
         std::cerr << "sendimg: " << tt_0 << " " << tt_1 << "\n";
         sendTToffload(tt_0, tt_1);
      }
      else
      {
         log<text_log>("TT offload above threshold but TT offloading disabled", logPrio::LOG_WARNING);
      }
   }
   return 0;
   
}

int tcsInterface::sendTToffload( float tt_0,
                                 float tt_1
                               )
{
   pcf::IndiProperty ip(pcf::IndiProperty::Number);
   ip.setDevice("modwfs");
   ip.setName("offset12");
   ip.add(pcf::IndiElement("dC1"));
   ip.add(pcf::IndiElement("dC2"));
   
   
   
   sendNewProperty (ip); 
   
   
   ip["dC1"] = tt_0;
   ip["dC2"] = tt_1;
   
   sendNewProperty(ip);
   
   return 0;
}

int tcsInterface::doFoffload( float F_0 )
{
   if(m_offlF_dump)
   {
      std::cerr << "Focus dumping: " << F_0 << "\n";
      sendFoffload(F_0);
      m_offlF_dump = false;
   }
   else 
   {
      F_0 *= m_offlF_gain;
            
      if(fabs(F_0) < m_offlF_thresh) F_0 = 0;
            
      if(F_0 == 0)
      {
         std::cerr << "Focus offload below threshold\n";
      }
      else if(m_offlF_enabled)
      {
         std::cerr << "Focus sendimg: " << F_0 << "\n";
         sendFoffload(F_0);
      }
      else
      {
         log<text_log>("Focus offload above threshold but Focus offloading disabled", logPrio::LOG_WARNING);
      }
   }
   return 0;
   
}

int tcsInterface::sendFoffload( float F_0 )
{
   static_cast<void>(F_0);
   
   log<text_log>("focus offloading not implemented!", logPrio::LOG_WARNING);
   
   return 0;
}

INDI_SETCALLBACK_DEFN(tcsInterface, m_indiP_loopState)(const pcf::IndiProperty &ipRecv)
{
   std::string state = ipRecv["state"].get();
   
   if(state == "open") m_loopState = 0;
   else if(state == "paused") m_loopState = 1;
   else m_loopState = 2;
   
   return 0;
}

INDI_SETCALLBACK_DEFN(tcsInterface, m_indiP_offloadCoeffs)(const pcf::IndiProperty &ipRecv)
{
   
   if(m_loopState != 2) return 0;
   
   size_t nextReq = m_lastRequest + 1;
   
   if(nextReq >= m_offloadRequests[0].size()) nextReq = 0;
     
   std::cerr << "nextReq: " << nextReq << "\n";
   

   //Tip-Tilt
   float tt0 = ipRecv["00"].get<float>();
   float tt1 = ipRecv["01"].get<float>();
   
   m_offloadRequests[0][nextReq] = m_offlTT_C_00 * tt0 + m_offlTT_C_01 * tt1;
   m_offloadRequests[1][nextReq] = m_offlTT_C_10 * tt0 + m_offlTT_C_11 * tt1;

   //Focus
   float f0 = ipRecv["02"].get<float>();
   
   m_offloadRequests[2][nextReq] = m_offlCFocus_00 * f0;
   
   
   //Coma
   float c0 = ipRecv["03"].get<float>();
   float c1 = ipRecv["04"].get<float>();
   
   m_offloadRequests[3][nextReq] = m_offlCComa_00 * c0 + m_offlCComa_01 * c1;
   m_offloadRequests[4][nextReq] = m_offlCComa_10 * c0 + m_offlCComa_11 * c1; 

   //Now update circ buffer
   m_lastRequest = nextReq;
   ++m_nRequests;
   if(m_nRequests > m_offloadRequests[0].size()) ++m_firstRequest;
   if(m_firstRequest >= m_offloadRequests[0].size()) m_firstRequest = 0;
   return 0;
}

INDI_NEWCALLBACK_DEFN(tcsInterface, m_indiP_offlTTenable)(const pcf::IndiProperty &ipRecv)
{
   if(ipRecv.getName() != m_indiP_offlTTenable.getName())
   {
      log<software_error>({__FILE__,__LINE__, "wrong INDI property received."});
      return -1;
   }
   
   if(!ipRecv.find("toggle")) return 0;
   
   if( ipRecv["toggle"].getSwitchState() == pcf::IndiElement::On && m_offlTT_enabled == false)
   {
      updateSwitchIfChanged(m_indiP_offlTTenable, "toggle", pcf::IndiElement::On, INDI_BUSY);
    
      m_offlTT_enabled = true;
   }
   else if( ipRecv["toggle"].getSwitchState() == pcf::IndiElement::Off && m_offlTT_enabled == true)
   {
      updateSwitchIfChanged(m_indiP_offlTTenable, "toggle", pcf::IndiElement::Off, INDI_IDLE);
    
      m_offlTT_enabled = false;
   }
   
   return 0;  

}

INDI_NEWCALLBACK_DEFN(tcsInterface, m_indiP_offlTTdump)(const pcf::IndiProperty &ipRecv)
{
   if(ipRecv.getName() != m_indiP_offlTTdump.getName())
   {
      log<software_error>({__FILE__,__LINE__, "wrong INDI property received."});
      return -1;
   }
   
   if(!ipRecv.find("request")) return 0;
   
   if( ipRecv["request"].getSwitchState() == pcf::IndiElement::On)
   {
      updateSwitchIfChanged(m_indiP_offlTTdump, "request", pcf::IndiElement::On, INDI_BUSY);
    
      m_offlTT_dump = true;
   }
   
   return 0;  

}

INDI_NEWCALLBACK_DEFN(tcsInterface, m_indiP_offlTTavgInt)(const pcf::IndiProperty &ipRecv)
{
   int target;
   
   if( indiTargetUpdate( m_indiP_offlTTavgInt, target, ipRecv, true) < 0)
   {
      log<software_error>({__FILE__,__LINE__});
      return -1;
   }
   
   m_offlTT_avgInt = target;
      
   return 0;
}

INDI_NEWCALLBACK_DEFN(tcsInterface, m_indiP_offlTTgain)(const pcf::IndiProperty &ipRecv)
{
   float target;
   
   if( indiTargetUpdate( m_indiP_offlTTgain, target, ipRecv, true) < 0)
   {
      log<software_error>({__FILE__,__LINE__});
      return -1;
   }
   
   m_offlTT_gain = target;
      
   return 0;
}

INDI_NEWCALLBACK_DEFN(tcsInterface, m_indiP_offlTTthresh)(const pcf::IndiProperty &ipRecv)
{
   float target;
   
   std::cerr << "Got offl thresh\n";
   
   if( indiTargetUpdate( m_indiP_offlTTthresh, target, ipRecv, true) < 0)
   {
      log<software_error>({__FILE__,__LINE__});
      return -1;
   }
   
   m_offlTT_thresh = target;
      
   return 0;
}


INDI_NEWCALLBACK_DEFN(tcsInterface, m_indiP_offlFenable)(const pcf::IndiProperty &ipRecv)
{
   if(ipRecv.getName() != m_indiP_offlFenable.getName())
   {
      log<software_error>({__FILE__,__LINE__, "wrong INDI property received."});
      return -1;
   }
   
   if(!ipRecv.find("toggle")) return 0;
   
   if( ipRecv["toggle"].getSwitchState() == pcf::IndiElement::On && m_offlF_enabled == false)
   {
      updateSwitchIfChanged(m_indiP_offlFenable, "toggle", pcf::IndiElement::On, INDI_BUSY);
    
      m_offlF_enabled = true;
   }
   else if( ipRecv["toggle"].getSwitchState() == pcf::IndiElement::Off && m_offlF_enabled == true)
   {
      updateSwitchIfChanged(m_indiP_offlFenable, "toggle", pcf::IndiElement::Off, INDI_IDLE);
    
      m_offlF_enabled = false;
   }
   
   return 0;  

}

INDI_NEWCALLBACK_DEFN(tcsInterface, m_indiP_offlFdump)(const pcf::IndiProperty &ipRecv)
{
   if(ipRecv.getName() != m_indiP_offlFdump.getName())
   {
      log<software_error>({__FILE__,__LINE__, "wrong INDI property received."});
      return -1;
   }
   
   if(!ipRecv.find("request")) return 0;
   
   if( ipRecv["request"].getSwitchState() == pcf::IndiElement::On)
   {
      updateSwitchIfChanged(m_indiP_offlFdump, "request", pcf::IndiElement::On, INDI_BUSY);
    
      m_offlF_dump = true;
   }
   
   return 0;  

}

INDI_NEWCALLBACK_DEFN(tcsInterface, m_indiP_offlFavgInt)(const pcf::IndiProperty &ipRecv)
{
   int target;
   
   if( indiTargetUpdate( m_indiP_offlFavgInt, target, ipRecv, true) < 0)
   {
      log<software_error>({__FILE__,__LINE__});
      return -1;
   }
   
   m_offlF_avgInt = target;
      
   return 0;
}

INDI_NEWCALLBACK_DEFN(tcsInterface, m_indiP_offlFgain)(const pcf::IndiProperty &ipRecv)
{
   float target;
   
   if( indiTargetUpdate( m_indiP_offlFgain, target, ipRecv, true) < 0)
   {
      log<software_error>({__FILE__,__LINE__});
      return -1;
   }
   
   m_offlF_gain = target;
      
   return 0;
}

INDI_NEWCALLBACK_DEFN(tcsInterface, m_indiP_offlFthresh)(const pcf::IndiProperty &ipRecv)
{
   float target;
   
   std::cerr << "Got offl thresh\n";
   
   if( indiTargetUpdate( m_indiP_offlFthresh, target, ipRecv, true) < 0)
   {
      log<software_error>({__FILE__,__LINE__});
      return -1;
   }
   
   m_offlF_thresh = target;
      
   return 0;
}

} //namespace app
} //namespace MagAOX

#endif //tcsInterface_hpp
