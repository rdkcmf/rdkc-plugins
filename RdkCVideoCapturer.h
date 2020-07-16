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

#ifndef __RDKCVIDEOCAPTURER_H_
#define __RDKCVIDEOCAPTURER_H_

/*************************       INCLUDES         *************************/

//#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "RdkCPluginFactory.h"
//#include "RdkCOverlay.h"

/* enable this for multiple application support */
#define MULTIPLE_APP_SUPPORT 1
#define DEFAULT_FPS 29
#define DEFAULT_BITRATE 4000000
#define DEFAULT_GOV 30

#define MAX_ENCODE_STREAM_NUM   4

typedef unsigned char       u8; /** UNSIGNED  8-bit data type */
typedef unsigned short     u16; /** UNSIGNED 16-bit data type */
typedef unsigned int       u32; /** UNSIGNED 32-bit data type */
typedef unsigned long long u64; /** UNSIGNED 64-bit data type */

using namespace std;

#ifdef MULTIPLE_APP_SUPPORT
/* Note : any changes in the order or values will effect the whole logic,needs to be taken care in the implementation */
typedef enum {
    SRC_BUFFER_1 = 0,        //Source buffer 1
    SRC_BUFFER_2 = 1,        //Source buffer 2
    SRC_BUFFER_3 = 2,        //Source buffer 3
    SRC_BUFFER_4 = 3,        //Source buffer 4
    H264_STREAM_1= 4,        //Stream 1
    H264_STREAM_2= 5,        //Stream 2
    H264_STREAM_3= 6,        //Stream 3
    H264_STREAM_4= 7,        //Stream 4
    NUMBER_OF_KEY_VALUE=8,
} KEY_VALUE_BUF_STRM;

typedef enum {
    KEYVALUE_FAILURE = -1,  //Failed to get the key value
    KEYVALUE_SUCESS = 0,    //successfully got the key value
    KEYVALUE_MDF = 1,       //key value modified
} KEY_VALUE_ERROR_CODE;
#endif

typedef enum tagVideoFormatTypeEnum
{
     /* 0x00 */  VIDEO_FORMAT_TYPE_H264
  ,  /* 0x01 */  VIDEO_FORMAT_TYPE_YUV
  ,  /* 0x02 */  VIDEO_FORMAT_TYPE_ME1
  , /* 0x03 */ VIDEO_FORMAT_TYPE_UNKNOWN
}VIDEO_FORMAT_TYPE_ENUM;

typedef enum {
    STREAM_TYPE_NONE    = 0,
    STREAM_TYPE_H264    = 1,
    STREAM_TYPE_MJPEG   = 2,
    STREAM_TYPE_NUM     = 3,
    STREAM_TYPE_INVALID = 4,
}STREAM_TYPE;

/*************************       STRUCTURES         *************************/

typedef struct window_s {
    unsigned int width;
    unsigned int height;
}window_t;

typedef struct video_stream_config
{
        u16 stream_type;                // 1=H264, 2=MJPEG
        u32 width;                      // unit is pixel
        u32 height;                     // unit is pixel
        u32 frame_rate;                 // 1~30 
        u32 gov_length;                 // 2~150 
        u32 profile;                    // 66=Baseline 77=Main 100=High
        u32 quality_type;               // 0=Constant bit rate(default) 1=Fix quality, variable bit rate 2=ABR
        u32 quality_level;              // 1=Very low 2=Low 3=Normal 4=High 5=Very high
        u32 bit_rate;                   // unit is Kbps (64,128,256,384,512,768,1000,2000)
        int map_buffer_id;              //0->main_buffer,1->second_buffer,2->third_buffer,3->forth_buffer
}video_stream_config_t;

typedef struct framedesc_s{
    unsigned int id;             /* read bits desc for all streams if -1 */
    unsigned int time_ms;        /* timeout in ms; -1 means non-blocking, 0 means blocking */
    unsigned int data_addr_offset;
    STREAM_TYPE stream_type;
    unsigned int pic_type;
    unsigned int stream_end;
    unsigned int jpeg_quality;
    unsigned int reserved;
    unsigned long long arm_pts;
    unsigned long long dsp_pts;
    unsigned int frame_num;
    unsigned int session_id;
    unsigned int size;
    window_t reso;
}framedesc_t;

typedef struct frame_info
{
        u16 stream_id;                  // 0~3
        u16 stream_type;                // Refer to RDKCStreamType
        u32 pic_type;                   // 0=MJPEG 1=IDR 2=I 3=P 4=B 5=JPEG_STREAM 6=JPEG_THUMBNAIL
        u32 frame_ptr;                  // The frame buffer pointer
        u32 frame_num;                  // The frame number, audio and video will have individual seq num
        u32 frame_size;
        u32 frame_timestamp;            // Frame timestamp, in milliseconds
        u32 jpeg_quality;               // 1~100, only when steam_type is MJPEG.
        u32 width;
        u32 height;
        u64 arm_pts;
        u64 dsp_pts;
        u16 padding_len;
        u32 padding_ptr;
        u8 reserved[6];
} frame_info_t;

typedef struct RDKC_PLUGIN_YUVInfo
{
        u16 buf_id;
        u8 *y_addr;     // YUV420: Must allocate buffer with size width * height
        u8 *uv_addr;    // YUV420: Must allocate buffer with size width * height / 2
        u32 width;
        u32 height;
        u32 pitch;
        u32 seq_num;
        u32 format;     //only for YUV frame
        u32 flag;
        u64 dsp_pts;
        u64 mono_pts;
}RDKC_PLUGIN_YUVInfo;

typedef struct RDKC_PLUGIN_YUVBufferInfo
{
        u32 width;
        u32 height;
        u32 buffer_type;        // ON / OFF
}RDKC_PLUGIN_YUVBufferInfo;

typedef struct PLUGIN_DayNightStatus
{
        u32 count_time;
        u32 day_night_flag;
}PLUGIN_DayNightStatus;

#ifdef MULTIPLE_APP_SUPPORT
typedef struct camera_resource_config_s{
    int Width;
    int Height;
    int FPS;
    int bitrate;
    int gop;
    VIDEO_FORMAT_TYPE_ENUM Format;
    int KeyValue;
    int RequiredChroma;
}camera_resource_config_t;
#endif


typedef enum
{
    AUDIO_CODEC_TYPE_G711_ALAW,
    AUDIO_CODEC_TYPE_G711_ULAW,
    AUDIO_CODEC_TYPE_G726,
    AUDIO_CODEC_TYPE_LPCM,
    AUDIO_CODEC_TYPE_AMR,
    AUDIO_CODEC_TYPE_AAC,
    AUDIO_CODEC_TYPE_MP3,
    AUDIO_CODEC_TYPE_MAX
} audio_codec_type;


typedef struct audio_stream_config
{
        u16 enable;                     // 0=disable, 1=enable
        u16 type;                       // 1=G.711 ulaw, 2=G.726, 3=LPCM, 5=AAC(if support AAC)
        u32 sample_rate;                // Reserverd. Audio sample rate, now only support 8000.
} audio_stream_config_t;

/*************************       CLASSES         *************************/

class RdkCEncodedVideoCapturerCallbacks
{
public:
        #if 0
        virtual int OnEncodedFrames(unsigned char *data, int size, int height, int width) = 0;
        #endif
        virtual int OnEncodedFrames(void* frame_info, unsigned char* encoded_data_addr) = 0; /* pure virtual function to be inherited in derived class */

        virtual ~RdkCEncodedVideoCapturerCallbacks()
        {
            #if 0
            RDK_LOG( RDK_LOG_INFO,"LOG.RDK.PLUGINS","%s(%d): EncodedVideoCapturerCallbacks destructor\n", __FILE__, __LINE__);
            #endif
        }
};

class RdkCYuvVideoCapturerCallbacks
{
public:

        virtual int OnYuvFrames(unsigned char *lumadata, unsigned char *chromadata,int lumasize, int chromasize, int height, int width) = 0;   /* pure virtual function to be inherited in derived class */
        virtual int OnYuvFrames(unsigned char *data, int size, int height, int width) = 0;   /* pure virtual function to be inherited in derived class */

        virtual ~RdkCYuvVideoCapturerCallbacks()
        {
            #if 0
            RDK_LOG( RDK_LOG_INFO,"LOG.RDK.PLUGINS","%s(%d): YuvVideoCapturerCallbacks destructor\n", __FILE__, __LINE__);
            #endif
        }
};

/* ADDED FOR ME1 */
class RdkCME1VideoCapturerCallbacks
{
public:

        virtual int OnME1Frames(unsigned char *data, int size, int height, int width) = 0;   /* pure virtual function to be inherited in derived class */

        virtual ~RdkCME1VideoCapturerCallbacks()
        {
            #if 0
            RDK_LOG( RDK_LOG_INFO,"LOG.RDK.PLUGINS","%s(%d): ME1VideoCapturerCallbacks destructor\n", __FILE__, __LINE__);
            #endif
        }
};

/* Platform specific camera class has to Inherit the methods from this class */
class RdkCVideoCapturer
{
    public:
        /* Pure virtual functions providing interface framework */
        virtual void Stop(VIDEO_FORMAT_TYPE_ENUM enVideoFormatType) = 0;
        virtual int SetResolution(int stream_key_val,int width,int height) = 0;
        virtual void GetResolution(int stream_key_val,int &width,int &height) = 0;
        virtual void SetBitRate() = 0;
        virtual void GetBitRate() = 0;
        virtual void SetFormat(VIDEO_FORMAT_TYPE_ENUM enVideoFormatType) = 0;
        virtual void GetFormat() = 0;
        virtual void SetMaxFrameRate() = 0;
        virtual void SetMinFrameRate() = 0;
        virtual void GetMaxFrameRate() = 0;
        virtual void GetMinFrameRate() = 0;
        virtual void SetEnabled() = 0;
        virtual void Pause() = 0;
        virtual void Resume() = 0;
        virtual int  SetFrameRate(int key_val,unsigned int fps = DEFAULT_FPS) = 0;
        virtual int  SetBitRate(int key_val, unsigned int br = DEFAULT_BITRATE) = 0;
        virtual int SendNextKeyFrame() = 0;
        virtual void SetMuteVideoStatus(int key_val, bool mute_video) = 0;
	virtual bool GetMuteVideoStatus(int key_val) = 0;
        virtual bool SetVideoStreamConfig(int streamID, void* config) = 0;
        virtual bool GetVideoStreamConfig(int streamID, void* config) = 0;
        virtual bool SetAudioStreamConfig(int streamID, void* config) = 0;
        virtual bool GetAudioStreamConfig(int streamID, void* config) = 0;
        virtual int getStreamHandler(int av_flag) = 0;
        virtual int SetGovLength(int key_val, unsigned int gov=DEFAULT_GOV)=0;
        virtual int SetSourceBufferResolution(int key_val,int width, int height)=0;
        virtual int  StreamInit(camera_resource_config_s *conf, int av_flag = 1) = 0;
	virtual int GetStream(void *frame_stream_info,int av_flag) = 0;
	virtual int CVRBuildInit(int fd, void *cvr_frame, int av_flag) = 0;
	virtual int CVRBuildWriteFrame(int fd, void *p_cvr_frame) = 0;
	virtual int StreamClose(int av_flag) = 0;
//	virtual int AudioStreamClose() = 0;

        /*YUV APIs*/
        virtual int GetSourceBufferConfig(int key_val, void *buf_info)= 0;
        virtual int ReadYUVData(int key_val, void * yuv_data, bool bypass_gdmacopy = false) = 0;
        virtual int ReadMEData(int key_val, void *me_data) = 0;
        virtual int ReadDNMode(void *DN_status)=0;
	virtual int RdkcVASendInit()=0;
	virtual int RdkcVASendResult(int va_send_id, void *va_result)=0;

	/*Overlay*/
	virtual int InitOverlay() = 0;
	virtual int SetOverlay(void *info) = 0;
	virtual int disableOverlay(int streamID, int areaID) = 0;

#ifdef MULTIPLE_APP_SUPPORT
        virtual void SetFramesCallback(camera_resource_config_t *conf, RdkCEncodedVideoCapturerCallbacks *SVCcbsInstance) = 0;
        virtual void SetFramesCallback(camera_resource_config_t *conf, RdkCYuvVideoCapturerCallbacks *YuvFrameCallbackParam) = 0;
        virtual void SetFramesCallback(camera_resource_config_t *conf, RdkCME1VideoCapturerCallbacks *ME1FrameCallbackParam) = 0; 

        virtual int  Start(camera_resource_config_s *conf) = 0;
	virtual void Stop(camera_resource_config_t *conf) = 0;
        virtual int  setStreamConfig(camera_resource_config_s *conf) = 0;
        virtual int GetResourceKey(bool is_predefined_value,camera_resource_config_t *conf) = 0;
#endif
	virtual ~RdkCVideoCapturer(){};
};

#endif /* __RDKCVIDEOCAPTURER_H_ */

/*************************       EOF         *************************/
