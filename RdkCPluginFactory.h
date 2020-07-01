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
/** @file RdkCPluginFactory.h
*  @description: Declaration of pluginfactory function prototypes
*                Declaration of RdkCPluginFactory class
*
*  @author Comcast
*  @bug No known bugs.
*/

#ifndef __RDKCPLUGINFACTORY_H_
#define __RDKCPLUGINFACTORY_H_

/*************************       INCLUDES         *************************/

#include <iostream>

#if defined ( PLUGINS_PLATFORM_RPI )
#include "RdkCVideoCapturer.h"
#endif

using namespace std;


/*************************       PRIVATE CLASSES         *************************/

class RdkCPluginFactory
{
    public:
        RdkCPluginFactory();

        ~RdkCPluginFactory();

        /* To create platform specific VideoCapturer instance */
        void* CreateVideoCapturer();

        /* To destroy platform specific VideoCapturer instance created */
        bool DestroyVideoCapturer();

#ifdef ENABLE_RDKC_AUDIO_SUPPORT
        /* To create platform specific AudioCapturer instance */
        void* CreateAudioCapturer();

        /* To destroy platform specific AudioCapturer instance created */
        bool DestroyAudioCapturer();
#endif

};


/*************************       FUNCTION PROTOTYPES         *************************/

/* Methods which are invoked from Application */
RdkCPluginFactory* CreatePluginFactoryInstance();

bool DestroyPluginFactoryInstance();

#endif /* __RDKCPLUGINFACTORY_H_ */

/*************************       EOF         *************************/
