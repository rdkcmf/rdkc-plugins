/*
##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2019 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################
*/
/** @file: RdkCPluginFactory.cpp
 *  @description: Implementation of plugin factory functions
 *
 *  @author Comcast
 *  @bug No known bugs.
 */

/*************************       INCLUDES         *************************/
/** @description including necessary files.
 */
#include "RdkCPluginFactory.h"

#ifdef SERCOMM_PLATFORM

	#include "RdkCSercommVideoCapturer.h"
	#ifdef ENABLE_RDKC_AUDIO_SUPPORT
		#include "RdkCSercommAudioCapturer.h"
    	#endif

#endif

/* RDK Logging */
#include "rdk_debug.h"


using namespace std;

bool IsGSTEnabledInRFC(char* file );
/*************************       DEFINES         *************************/

/* Platform type is a compilation flag, based on this appropriate VideoCapturer instance will be created */
//#define SERCOMM_PLATFORM


/*************************       GLOBAL VARIABLES         *************************/

/* Pointer to store the PluginFactory Instance */
static RdkCPluginFactory* FactoryInstance = NULL;

/* Temporary variable to hold the instance of platform specific encoded VideoCapturer class */
static RdkCVideoCapturer* VCInstance = NULL;

#ifdef ENABLE_RDKC_AUDIO_SUPPORT
RdkCAudioCapturer* ACInstance = NULL;
#endif


/*************************       FUNCTION DEFINITIONS         *************************/

/**
 * @description: Invoking RDK Logger initialization function
 * @param: void
 * @return: none
 */
void LOGInit()
{
    /* RDK logger initialization */
    rdk_logger_init("/etc/debug.ini");
}

/** @description: Creating PluginFactory instance and returning to the application
 *  @param: none
 *  @return: FactoryInstance
 */
RdkCPluginFactory* CreatePluginFactoryInstance()
{
    /* Initialize rdklogger */
    LOGInit();
    RDK_LOG( RDK_LOG_DEBUG,"LOG.RDK.PLUGINS","Create Plugin Factoru Instance Called\n");    

    if(NULL == FactoryInstance)
    {
        RDK_LOG( RDK_LOG_DEBUG,"LOG.RDK.PLUGINS","%s(%d): creating plugin factory instance\n", __FILE__, __LINE__);

        /* Create RdkCPluginFactory object */
        FactoryInstance = new RdkCPluginFactory();
        if(FactoryInstance != NULL)
        {
            RDK_LOG( RDK_LOG_DEBUG,"LOG.RDK.PLUGINS","%s(%d): plugin factory instance creation success\n", __FILE__, __LINE__);
        }
        else
        {
            RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.PLUGINS","%s(%d): failed to create pluginFactory instance\n", __FILE__, __LINE__);
            /* Application has to handle the failure scenarios */
        }
    }

    return FactoryInstance;
}

/** @description: Constructor
 *  @param: none
 *  @return: none
 */

RdkCPluginFactory::RdkCPluginFactory()
{
   RDK_LOG( RDK_LOG_DEBUG,"LOG.RDK.PLUGINS","%s(%d): plugin factory constructor\n", __FILE__, __LINE__); 
}

/** @description: Destructor
 *  @param: none
 *  @return: none
 */

RdkCPluginFactory::~RdkCPluginFactory()
{
    RDK_LOG( RDK_LOG_DEBUG,"LOG.RDK.PLUGINS","%s(%d): plugin factory destructor\n", __FILE__, __LINE__);
}

/** @description: Creating Platform specific VideoCapturer instance based on the compilation flag
 *  @param: none
 *  @return: platform specific video capturer Instance
 */
void* RdkCPluginFactory::CreateVideoCapturer()
{

    RDK_LOG( RDK_LOG_DEBUG,"LOG.RDK.PLUGINS","CREATEVIDEOCAPTURER CALLED\n");
    /* Platform type is a compilation flag, so based on that appropriate VideoCapturer instance will be created */
#ifdef SERCOMM_PLATFORM
    if(NULL == VCInstance)
    {
	RDK_LOG( RDK_LOG_DEBUG,"LOG.RDK.PLUGINS","GST disabled via RFC, using sercomm capturer\n");
        RDK_LOG( RDK_LOG_DEBUG,"LOG.RDK.PLUGINS","%s(%d): creating sercomm video capturer instance\n", __FILE__, __LINE__);
#ifdef HYDRA_CAMERA_HANDLER
	RdkCSercommEncodedVideoCapturerHydra* SVCInstance = new RdkCSercommEncodedVideoCapturerHydra();
#else

        RdkCSercommEncodedVideoCapturer* SVCInstance = new RdkCSercommEncodedVideoCapturer();
#endif
        VCInstance = (RdkCVideoCapturer*) SVCInstance;

        if(VCInstance != NULL)
        {
            RDK_LOG( RDK_LOG_DEBUG,"LOG.RDK.PLUGINS","%s(%d): sercomm video capturer instance creation success\n", __FILE__, __LINE__);
        }
        else
        {
            RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.PLUGINS","%s(%d): failed to create sercomm video capturer instance\n", __FILE__, __LINE__);
        }

    }
#elif QUANTA_PLATFORM
    if(NULL == VCInstance)
    {
        RDK_LOG( RDK_LOG_INFO,"LOG.RDK.PLUGINS","%s(%d): creating quanta video capturer instance\n", __FILE__, __LINE__);

        RdkCQuantaEncodedVideoCapturer* QVCInstance = new RdkCQuantaEncodedVideoCapturer();
        VCInstance = (RdkCVideoCapturer*) QVCInstance;

        if(VCInstance != NULL)
        {
            RDK_LOG( RDK_LOG_INFO,"LOG.RDK.PLUGINS","%s(%d): creating quanta video capturer instance\n", __FILE__, __LINE__);
        }
        else
        {
            RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.PLUGINS","%s(%d): failed to create quanta video capturer instance\n", __FILE__, __LINE__);
        }
    }
#endif /* QUANTA_PLATFORM */

    return VCInstance;
}

/** @description: Destroying Platform Specific VideoCapturer instance created
 *  @param: none
 *  @return: bool (true if success, false if failure).
 */
bool RdkCPluginFactory::DestroyVideoCapturer()
{
    if(VCInstance != NULL)
    {
        delete(VCInstance);
        VCInstance = NULL;
        RDK_LOG( RDK_LOG_INFO,"LOG.RDK.PLUGINS","%s(%d): Successfully relaesed Video Capturer Instance\n", __FILE__, __LINE__);
        return true;
    }

    return false;
}

#ifdef ENABLE_RDKC_AUDIO_SUPPORT
/** @description: Creating Platform specific VideoCapturer instance based on the compilation flag
 *  @param: none
 *  @return: platform specific video capturer Instance
 */
void* RdkCPluginFactory::CreateAudioCapturer()
{
    /* Platform type is a compilation flag, so based on that appropriate AudioCapturer instance will be created */
#ifdef SERCOMM_PLATFORM
    if(NULL == ACInstance)
    {
     if (IsGSTEnabledInRFC(RFC_GST))
     {
        RDK_LOG( RDK_LOG_DEBUG,"LOG.RDK.PLUGINS","GST is enabled, using gst audio capturer\n");
	RDK_LOG( RDK_LOG_INFO,"LOG.RDK.PLUGINS","%s(%d): creating GST audio capturer instance\n", __FILE__, __LINE__);
	RdkCGSTAudioCapturer* GVCInstance = new RdkCGSTAudioCapturer();
        ACInstance = (RdkCAudioCapturer*) GVCInstance;

        if(ACInstance != NULL)
        {
            RDK_LOG( RDK_LOG_INFO,"LOG.RDK.PLUGINS","%s(%d): creating GST audio capturer instance\n", __FILE__, __LINE__);
        }
        else
        {
            RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.PLUGINS","%s(%d): failed to GST quanta audio capturer instance\n", __FILE__, __LINE__);
        }

     }
     else
     {

        RDK_LOG( RDK_LOG_DEBUG,"LOG.RDK.PLUGINS","GST is disabled, using sercomm audio capturer\n");
        RDK_LOG( RDK_LOG_INFO,"LOG.RDK.PLUGINS","%s(%d): creating sercomm audio capturer instance\n", __FILE__, __LINE__);
#ifdef HYDRA_CAMERA_HANDLER
        RdkCSercommAudioCapturerHydra* SACInstance = new RdkCSercommAudioCapturerHydra();
#else

#endif
        ACInstance = (RdkCAudioCapturer*) SACInstance;

        if(ACInstance != NULL)
        {
            RDK_LOG( RDK_LOG_INFO,"LOG.RDK.PLUGINS","%s(%d): sercomm audio capturer instance creation success\n", __FILE__, __LINE__);
        }
        else
        {
            RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.PLUGINS","%s(%d): failed to create sercomm audio capturer instance\n", __FILE__, __LINE__);
        }
     }
    }
#elif QUANTA_PLATFORM
    if(NULL == ACInstance)
    {
        RDK_LOG( RDK_LOG_INFO,"LOG.RDK.PLUGINS","%s(%d): creating quanta audio capturer instance\n", __FILE__, __LINE__);

        RdkCQuantaAudioCapturer* QVCInstance = new RdkCQuantaAudioCapturer();
        ACInstance = (RdkCAudioCapturer*) QVCInstance;

        if(ACInstance != NULL)
        {
            RDK_LOG( RDK_LOG_INFO,"LOG.RDK.PLUGINS","%s(%d): creating quanta audio capturer instance\n", __FILE__, __LINE__);
        }
        else
        {
            RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.PLUGINS","%s(%d): failed to create quanta audio capturer instance\n", __FILE__, __LINE__);
        }
    }
#endif

    return ACInstance;
}

/** @description: Destroying Platform Specific AudioCapturer instance created
 *  @param: none
 *  @return: bool (true if success, false if failure).
 */
bool RdkCPluginFactory::DestroyAudioCapturer()
{
    if(ACInstance != NULL)
    {
        delete(ACInstance);
        ACInstance = NULL;
        RDK_LOG( RDK_LOG_INFO,"LOG.RDK.PLUGINS","%s(%d): Successfully relaesed Audio Capturer Instance\n", __FILE__, __LINE__);
        return true;
    }

    return false;
}
#endif //End of ENABLE_RDKC_AUDIO

/** @description: Destroying PluginFactory instance created
 *  @param: none
 *  @return: bool (true if success, false if failure).
 */
bool DestroyPluginFactoryInstance()
{
    if(FactoryInstance != NULL)
    {
        /* Delete the instance of Platform specific VideoCapturer */
        delete(FactoryInstance);
        FactoryInstance = NULL;
        RDK_LOG( RDK_LOG_INFO,"LOG.RDK.PLUGINS","%s(%d): Successfully relaesed Plugin Factory Instance\n", __FILE__, __LINE__);
        return true;
    }

    return false;
}

/*************************       EOF         *************************/
