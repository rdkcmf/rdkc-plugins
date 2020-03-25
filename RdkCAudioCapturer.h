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
/** @file: RdkCVideoCapturer.h
*  @description: Declaration of videocapturer related function prototypes
*                Declaration of videocapturer class
*
*  @author Comcast
*  @bug No known bugs.
*/

#ifndef __RDKCAUDIOCAPTURER_H_
#define __RDKCAUDIOCAPTURER_H_

/*************************       INCLUDES         *************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "RdkCPluginFactory.h"

/************************        DEFINES          *************************/

#define AUDIO_SAMPLINGRATE_8K   8000
#define AUDIO_SAMPLINGRATE_16K  16000
#define AUDIO_SAMPLINGRATE_32K  32000
#define AUDIO_SAMPLINGRATE_48K  48000

typedef enum{
    RDKC_AUDIO_ERROR = -1,
    RDKC_AUDIO_SUCCESS = 0,
    RDKC_AUDIO_FAILURE,
    RDKC_AUDIO_SAMPLE_RATE_NOT_SUPPORTED,
}RDKC_AUDIO_RETURN_CODE;

typedef struct audio_framedesc_s
{
    unsigned short id;             /* read bits desc for all streams if -1 */
    unsigned int time_ms;        /* timeout in ms; -1 means non-blocking, 0 means blocking */
    unsigned short stream_type;
    unsigned int pic_type;
    unsigned int reserved;
    unsigned long long arm_pts;
    unsigned long long dsp_pts;
    unsigned int frame_num;
    unsigned int size;
    unsigned int timestamp;

}audio_framedesc_t;
/*************************       CLASSES         *************************/

class RdkCPCMAudioCapturerCallbacks
{
public:

        virtual int OnPCMAudio(unsigned char *audio_data, unsigned int timestamp) = 0; //timestamp need to be unsigned int 32 bit

        virtual ~RdkCPCMAudioCapturerCallbacks()
        {
        }


};

/* Platform specific camera class has to Inherit the methods from this class */
class RdkCAudioCapturer
{
public:
        /* Pure virtual functions providing interface framework */
	virtual int Start() = 0;
        virtual int Stop() = 0;
        virtual void Pause() = 0;
        virtual void Resume() = 0;
	virtual int  StreamInit(int samplingrate) = 0;
	virtual void RegisterAudioCallback(RdkCPCMAudioCapturerCallbacks *AudioFrameCallback) = 0;
	virtual ~RdkCAudioCapturer(){};
};

#endif /* __RDKCAUDIOCAPTURER_H_ */

/*************************       EOF         *************************/

