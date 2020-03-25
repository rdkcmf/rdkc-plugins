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
/*
 * Steps to use plugin for any application:
 * 1. Create the Instance of Plugin Factory.
 * 2. Using the Plugin Factory instance Create the video capturer(Camera instance).
 * 3. Set the Configuration for the required stream or buffer.
 * 4. Register your application.
 * 5. Start the camera.
 */
#include <iostream>
#include <RdkCVideoCapturer.h>
#include <RdkCPluginFactory.h>

#define FRAME_LIMIT 100

class Application : public RdkCEncodedVideoCapturerCallbacks, public RdkCYuvVideoCapturerCallbacks, public RdkCME1VideoCapturerCallbacks
{

private:
	FILE *fp;
	RdkCVideoCapturer* app_camera;
    camera_resource_config_t conf;

public:
	void init()
	{
        if( conf.Format == VIDEO_FORMAT_TYPE_YUV) {
            app_camera->SetFramesCallback(&conf, ( RdkCYuvVideoCapturerCallbacks* ) this);//registering application
        }
        else if( conf.Format == VIDEO_FORMAT_TYPE_H264) {
		app_camera->SetFramesCallback(&conf,( RdkCEncodedVideoCapturerCallbacks* ) this);//registering application
                                                                                                //callback
         }
        else if( conf.Format == VIDEO_FORMAT_TYPE_ME1) {
		app_camera->SetFramesCallback(&conf,( RdkCME1VideoCapturerCallbacks* ) this);//registering application
                                                                                                //callback
         }
	}

    #if 0
	int OnEncodedFrames(unsigned char *data, int size, int height, int width)
	{
		static int ctr=0;

		if(data)
		{
			if( fwrite(data, size, 1, fp) < 0){
				cout<< "Fail to write" <<endl;
			}
			ctr++;
		}
		if( ctr == 100){
			fclose(fp);
			exit(0);
		}
		return 0;
	}
    #endif

    /*
     *  Application callback function to handle Encoded data
     *  First 100 frames will be written to the file
     */

    int OnEncodedFrames(void* frame_info , unsigned char* encoded_data_addr)   //Callback function with new prototype
    {
        static int frame_ctr=0;

        if((encoded_data_addr + ((framedesc_t*)frame_info) -> data_addr_offset))
        {
            if(fwrite((encoded_data_addr+ ((framedesc_t*)frame_info) -> data_addr_offset), (((framedesc_t*)frame_info)->size), 1, fp) <0) {
                    cout << "Fail to write" << endl;
            }
            frame_ctr++;
        }

        if(FRAME_LIMIT == frame_ctr){
            fclose(fp);
            exit(0);
        }
        return 0;
    }

    /*
     *  Function to return mute video status implemented in webrtc
     */

    bool GetMuteVideoStatus(){
        return false;
    }


    /*
     * Application callback function to handle YUV data
     * First 100 frames will be written to the file
     */
    int OnYuvFrames(unsigned char *data, int size, int height, int width)
    {
        static int frame_ctr=0;

        if(data)
        {
            if( fwrite(data, size, 1, fp) < 0){
                cout<< "Fail to write" <<endl;
            }
            frame_ctr++;
        }

        if(FRAME_LIMIT == frame_ctr){
            fclose(fp);
            exit(0);
        }
        return 0;
    }

    int OnYuvFrames(unsigned char *lumadata, unsigned char *chromadata,int lumasize, int chromasize, int height, int width)
    {
        static int frame_ctr=0;

        if( lumadata && chromadata ) {
            if( fwrite(lumadata, lumasize, 1, fp) < 0){
                cout<< "Fail to write" <<endl;
            }
            if( fwrite(chromadata, chromasize, 1, fp) < 0){
                cout<<  "Fail to write" <<endl;
            }
        frame_ctr++;
        }

        if(FRAME_LIMIT == frame_ctr){
            fclose(fp);
            exit(0);
        }
        return 0;
    }
 
    /*
     * Application callback function to handle ME1 data
     * First 100 frames will be written to the file
     */
    int OnME1Frames(unsigned char *data, int size, int height, int width)
    {
        static int frame_ctr=0;

        if(data) {
            if( fwrite(data, size, 1, fp) < 0){
                cout<< "Fail to write" <<endl;
            }
            frame_ctr++;
        }

        if(FRAME_LIMIT == frame_ctr){
            fclose(fp);
            exit(0);
        }
        return 0;
    }

    void StartCamera()
    {
	app_camera->Start( &conf);
#if 1
        //Changing Bitrate and FPS
        app_camera->SetBitRate(conf.KeyValue, 1000);  //Bitrate in kbps
        app_camera->SetFrameRate(conf.KeyValue, 18);
#endif

#if 0
        app_camera->ChangeBitRate(conf.KeyValue);
        app_camera->ChangeFrameRate(conf.KeyValue);
#endif
    }

	Application(RdkCVideoCapturer* _camera, camera_resource_config_t conf_arg )
	: fp(NULL)
	,app_camera(NULL)
	{
        char filename[10];
        if( conf_arg.Format == VIDEO_FORMAT_TYPE_H264 ) {
        sprintf(filename, "file%d.H264", conf_arg.KeyValue);//one file will be created for each app object
		fp = fopen(filename, "wo+");
        }
        else if( conf_arg.Format == VIDEO_FORMAT_TYPE_YUV ) {
            sprintf(filename, "file%d.yuv", conf_arg.KeyValue);//one file will be created for each app object
            fp = fopen(filename, "wo+");
        }
        else if( conf_arg.Format == VIDEO_FORMAT_TYPE_ME1 ) {
            sprintf(filename, "file%d.me1", conf_arg.KeyValue);//one file will be created for each app object
            fp = fopen(filename, "wo+");
        }
        conf = conf_arg;    //copying stream configuration in application

		if(!fp)
			return;
        app_camera = _camera;
        if(KEYVALUE_FAILURE ==app_camera->GetResourceKey(true,&conf))
		{
            printf("getresource failed !!!!! \n");
        }
    }

	~Application()
	{
		cout << "Destructor application" <<endl;
		fclose(fp);
		cout<< "File closed " <<endl;
	}

};

/* Thread function */

void *app_func(void *app)
{
    Application *curr_app = (Application *)app;
    curr_app->init();
	curr_app->StartCamera();
}


int main()
{
    RdkCPluginFactory* temp_factory = CreatePluginFactoryInstance(); //creating plugin factory instance

    RdkCVideoCapturer* main_camera = ( RdkCVideoCapturer* )
    temp_factory->CreateVideoCapturer();     //creating camera instance
    pthread_t App1Thread, App2Thread, App3Thread;     //Application thread
    camera_resource_config_t conf1, conf2, conf3;
    /* configuration for first application */
    conf1.Width=640;
    conf1.Height=480;
    conf1.Format=VIDEO_FORMAT_TYPE_H264;  //stream Configuration for first application
    conf1.KeyValue= H264_STREAM_1;

#if 0
    /* configuration for second application */
    conf2.Width=640;
    conf2.Height=480;
    conf2.Format=VIDEO_FORMAT_TYPE_YUV;  //yuv configuration for second application
    conf2.RequiredChroma = 1;
    conf2.KeyValue=SRC_BUFFER_4;

    /* configuration for third application */
    conf3.Width=640;
    conf3.Height=480;
    conf3.Format=VIDEO_FORMAT_TYPE_H264;  //stream Configuration for third application
    conf3.KeyValue= H264_STREAM_2;
#endif

    Application app1( main_camera, conf1 );	//first application object
#if 0
    Application app2( main_camera, conf2 );	//second application object
    Application app3( main_camera, conf3 );	//third application object
#endif

    pthread_create(&App1Thread, NULL, app_func, &app1);

#if 0
    pthread_create(&App2Thread, NULL, app_func, &app2);
    pthread_create(&App3Thread, NULL, app_func, &app3);
#endif

	while(1){
		usleep( 1000000 );
	}
}
