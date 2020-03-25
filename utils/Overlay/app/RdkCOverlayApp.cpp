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
#include "RdkCOverlayApp.h"

static OverlayInfo info;			/* OverlayInfo structure object */
static volatile sig_atomic_t updateOverlay = 1;	/* It is set to 1 as to set Overlay bydefault when the process starts */

static RdkCPluginFactory* pluginsFactory = NULL; 
static RdkCVideoCapturer* recorder = NULL;

/*
 * @brief: update overlay configurations based on config file
 * @param: width of frame on which overlay is to be applied 
 * @param: height of frame on which overlay is to be applied 
 * @return : RDKC_SUCCESS on success
 */
int get_overlay_params(int width, int height)
{
    char* configParam = NULL;

    if(RDKC_FAILURE == config_init()) {
	RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): ConfigMgr not initialised properly.\n", __FUNCTION__, __LINE__);
    }


#if 0
    /* Stream id */
    configParam = (char*) rdkc_envGet(STREAM_ID);
    if(!configParam) {
	RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): Error retriving parameter %s. Setting it to default.\n", __FUNCTION__, __LINE__,STREAM_ID);
        info.streamID = DEFAULT_STREAM_ID;
    }
    else {
        info.streamID = (OverlayStreamID) atoi(configParam);
    }
#endif

    /* Area id */
    configParam = (char*) rdkc_envGet(AREA_ID);
    if(!configParam) {
	RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): Error retriving parameter %s. Setting it to default.\n", __FUNCTION__, __LINE__,AREA_ID);
        info.areaID = DEFAULT_AREA_ID;
    }
    else {
        info.areaID = (OverlayAreaID) atoi(configParam);
    }

    /* Overlay x coordinate */
    if(FRAME_720P_WIDTH == width && FRAME_720P_HEIGHT == height) {
	configParam = (char*) rdkc_envGet(OVERLAY_X_ORDINATE_720P);
    	if(!configParam) {
		RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): Error retriving parameter %s. Setting it to default.\n", __FUNCTION__, __LINE__,OVERLAY_X_ORDINATE_720P);
		info.x = DEFAULT_OVERLAY_X_ORDINATE;
    	}
    	else {
		info.x = atoi(configParam);
    	}
    }
    else if(FRAME_1080P_WIDTH == width && FRAME_1080P_HEIGHT == height) {
	configParam = (char*) rdkc_envGet(OVERLAY_X_ORDINATE_1080P);
    	if(!configParam) {
		RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): Error retriving parameter %s. Setting it to default.\n", __FUNCTION__, __LINE__,OVERLAY_X_ORDINATE_1080P);
		info.x = DEFAULT_OVERLAY_X_ORDINATE;
    	}
    	else {
		info.x = atoi(configParam);
    	}
    }


    /* Overlay y coordinate */
    if(FRAME_720P_WIDTH == width && FRAME_720P_HEIGHT == height) {
	configParam = (char*) rdkc_envGet(OVERLAY_Y_ORDINATE_720P);
    	if(!configParam) {
		RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): Error retriving parameter %s. Setting it to default.\n", __FUNCTION__, __LINE__,OVERLAY_Y_ORDINATE_720P);
		info.y = DEFAULT_OVERLAY_Y_ORDINATE;
    	}
    	else {
		info.y = atoi(configParam);
    	}
    }
    else if(FRAME_1080P_WIDTH == width && FRAME_1080P_HEIGHT == height) {
	configParam = (char*) rdkc_envGet(OVERLAY_Y_ORDINATE_1080P);
    	if(!configParam) {
		RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): Error retriving parameter %s. Setting it to default.\n", __FUNCTION__, __LINE__,OVERLAY_Y_ORDINATE_1080P);
		info.y = DEFAULT_OVERLAY_Y_ORDINATE;
    	}
    	else {
		info.y = atoi(configParam);
    	}
    }


    /* Overlay transparency */
    configParam = (char*) rdkc_envGet(OVERLAY_TRANSPARENCY);
    if(!configParam) {
	RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): Error retriving parameter %s. Setting it to default.\n", __FUNCTION__, __LINE__,OVERLAY_TRANSPARENCY);
        info.alpha = DEFAULT_OVERLAY_TRANSPARENCY;
    }
    else {
	info.alpha = atoi(configParam);
    }

    /* Overlay image path */
    configParam = (char*) rdkc_envGet(OVERLAY_IMAGE_PATH);
    memset(info.BMPfilename, 0, sizeof(info.BMPfilename));
    if(!configParam) {
	RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): Error retriving parameter %s. Setting it to default.\n", __FUNCTION__, __LINE__,OVERLAY_IMAGE_PATH);
        strcpy(info.BMPfilename, DEFAULT_OVERLAY_IMAGE);
    }
    else {
        strcpy(info.BMPfilename, configParam);
    }

    if( RDKC_FAILURE == config_release() ) {
	RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): Error in config_release\n", __FUNCTION__, __LINE__);
    }

    RDK_LOG( RDK_LOG_INFO,"LOG.RDK.OVERLAY","%s(%d): Overlay parameters: streamID : %d, areaID : %d, x : %d, y : %d, alpha : %d, BMPfilename : %s\n", __FUNCTION__, __LINE__, info.streamID, info.areaID, info.x, info.y, info.alpha, info.BMPfilename);
    return RDKC_SUCCESS;
}

/*
 * @breif: init
 * @param: void
 * @return: RDKC_SUCCESS on success, RDKC_FAILURE on failure
 */
int init()
{
    pluginsFactory = CreatePluginFactoryInstance();
    if(!pluginsFactory) {
	RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): Error creating plugin factory\n", __FUNCTION__, __LINE__);
	return RDKC_FAILURE;
    }

    recorder = (RdkCVideoCapturer*) pluginsFactory -> CreateVideoCapturer();
    if(!recorder) {
	RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): Error creating Recoder\n", __FUNCTION__, __LINE__);
        return RDKC_FAILURE;
    }

    if( OVERLAY_FAILURE == recorder -> InitOverlay() ) {
	return RDKC_FAILURE;
    }

    return RDKC_SUCCESS;
}

#if 0

/*
 * @breif: Check if the process is already running
 * @param: process id
 * @return: RDKC_SUCCESS on running process, RDKC_FAILURE on no process
 */
static int check_pid_alive(pid_t pid)
{
	if (PROCESS_EXIST == kill(pid, 0)) {
		return RDKC_SUCCESS;
	}
	// process doesn't exist
	return RDKC_FAILURE;
}

/*
 * @breif: Verify if already any instance for this process is running
 * @param: filename
 * @return: -2 for erro, fd on success
 */
static int overlay_check_filelock(char *fname)
{
        int fd = -1;
        pid_t pid = -1;

        fd = open(fname, O_WRONLY | O_CREAT | O_EXCL, 0644);
        if(fd < 0 && errno == EEXIST) {
                fd = open(fname, O_RDONLY, 0644);
                if (fd >= 0) {
                        read(fd, &pid, sizeof(pid));
                        kill(pid, SIGTERM);
                        close(fd);
                        sleep(1);
                        if (RDKC_SUCCESS == check_pid_alive(pid)) {
                                kill(pid, SIGTERM);
                        }
                }
                unlink(fname);
                return -2;
        }

        return fd;
}

/*
 * @breif: signal handler function
 * @param: signal
 * @return: void
 */
void overlay_signal_handler( int signum )
{
	updateOverlay = 1;
}

int main()
{
    pid_t pid = -1;
    int file_fd = -1;

    /* Rdk-logger initilization */
    rdk_logger_init(LOGGER_PATH);

    /* process control */
    file_fd = overlay_check_filelock((char*) LOCK_FILENAME_XFINITY_OVERLAY);
    if (-2 == file_fd) {
	file_fd = overlay_check_filelock((char*) LOCK_FILENAME_XFINITY_OVERLAY);
    }
    if (file_fd < 0) {
	RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): Error opening file \n", __FUNCTION__, __LINE__);
	return RDKC_FAILURE;
    }
    pid = getpid();
    write(file_fd, &pid, sizeof(pid));
    close(file_fd);

    signal(SIGUSR1, overlay_signal_handler);

    if( RDKC_FAILURE == init() ) {
	RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): init failed \n", __FUNCTION__, __LINE__);
	return RDKC_FAILURE;
    }

    while(1) {
    	if(updateOverlay) {
	    get_overlay_params();
    	    if ( OVERLAY_FAILURE == rdkc_set_overlay(&info) ) {
		RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): ERROR::rdkc_set_overlay \n", __FUNCTION__, __LINE__);
		return RDKC_FAILURE;
    	    }

	    updateOverlay = updateOverlay >> 1;
        }

	    RDK_LOG( RDK_LOG_INFO,"LOG.RDK.OVERLAY","%s(%d):Putting the process to sleep for 1 hr  \n", __FUNCTION__, __LINE__);
	    sleep(MAX_SLEEP_DURATION);
	    RDK_LOG( RDK_LOG_INFO,"LOG.RDK.OVERLAY","%s(%d):Sleep Duration over  \n", __FUNCTION__, __LINE__);
    }
}

int main(int argc, char** argv)
{

    int  ctr = 1;
    int streamID = -1;
    int width = 0;
    int height = 0;

    /* Rdk-logger initilization */
    rdk_logger_init(LOGGER_PATH);

    if( RDKC_FAILURE == init() ) {
        RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): init failed \n", __FUNCTION__, __LINE__);
        return RDKC_FAILURE;
    }

    while(ctr < argc) {
	memset(&info, 0, sizeof(OverlayInfo));
	streamID = atoi(argv[ctr]);
	info.streamID = (OverlayStreamID) streamID;

	//To determine overlay co-ordinates acc. to frame resolution
	recorder -> GetResolution(streamID,width,height);
    	get_overlay_params(width,height);

	if ( OVERLAY_FAILURE == recorder -> SetOverlay(&info) ) {
                RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): ERROR::rdkc_set_overlay \n", __FUNCTION__, __LINE__);
                return RDKC_FAILURE;
        }
	ctr++;
    }
}
#endif

/* @breif: Get Resolution for the particular stream
 * @param: stream ID
 * @param: Ref to frame Height
 * @param: Ref to frame Width
 * @return: void
 */
void getStreamResolution( int streamID, int &fHeight, int &fWidth)
{
    //get resolution for the frame
    if(recorder) {
	recorder -> GetResolution(streamID, fWidth, fHeight);
    }
}

/* @breif: Get dimension of the overlay image
 * @param: path to overlay image
 * @param: Ref to image Height
 * @param: Ref to image Width
 * @return: void
 */
void getImgDimension (char* imgPath, int &imgHeight, int &imgWidth)
{
    FILE *fp = NULL;
    BITMAPFILEHEADER fileHead;
    BITMAPINFOHEADER infoHead;

    if ((fp = fopen(imgPath, "r")) == NULL) {
        RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): %s open fail or not exists \n", __FUNCTION__, __LINE__,imgPath);
        return;
    }

    memset(&fileHead, 0, sizeof(BITMAPFILEHEADER));
    memset(&infoHead, 0, sizeof(BITMAPINFOHEADER));
    fread(&fileHead.bfType, sizeof(fileHead.bfType), 1, fp);
    fread(&fileHead.bfSize, sizeof(fileHead.bfSize), 1, fp);
    fread(&fileHead.bfReserved1, sizeof(fileHead.bfReserved1), 1, fp);
    fread(&fileHead.bfReserved2, sizeof(fileHead.bfReserved2), 1, fp);
    fread(&fileHead.bfOffBits, sizeof(fileHead.bfOffBits), 1, fp);

    if (fileHead.bfType != ID_BMP) {
        RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): this is not a bmp file \n", __FUNCTION__, __LINE__);
        fclose(fp);
        return;
    }

    fread(&infoHead, sizeof(BITMAPINFOHEADER), 1, fp);

    imgWidth = infoHead.biWidth;
    imgHeight = infoHead.biHeight;

    fclose(fp);
}

/* @breif: print help 
 * @return: void
 */
void help()
{
    RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): Usage: rdkc_overlay -s <stream_id> -A <area_id> -f <overlay_fname> -x <x-ordinate> \ 
		                                             -y <y-ordinate> -a <alignment> -t <transparency>\n", __FUNCTION__, __LINE__);
}

/* @breif: main
   @param: no. of cmd line args
   @param: argv
   @return: int
 */
int main(int argc, char **argv)
{
    int c = 0;
    int optIndex = 0;
    int frameHeight = -1;
    int frameWidth = -1;
    int imgHeight = -1;
    int imgWidth = -1;
    int imgAlignment = -1;
    int streamID = -1;
    int xOffset = -1;
    int yOffset = -1;
    int alpha = DEFAULT_OVERLAY_TRANSPARENCY;
    int areaID = DEFAULT_AREA_ID;
    int overlayEnable = -1;
    //bool ordIsValid = false; 

    static struct option longOpt[] = {
	{"streamID", required_argument, 0, 0},
	{"areaID", optional_argument, 0, 0},
	{"file", required_argument, 0, 0 },
	{"alignment", required_argument, 0, 0 },
	{"xOffset", required_argument, 0, 0 },
	{"yOffset", required_argument, 0, 0 },
	{"enable", required_argument, 0, 0 },
	{"transparency", optional_argument, 0, 0 },
	{"help", no_argument, 0, 0 },
	{0, 0, 0, 0}
    };

    /* Rdk-logger initilization */
    rdk_logger_init(LOGGER_PATH);

    if( RDKC_FAILURE == init() ) {
        RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): init failed \n", __FUNCTION__, __LINE__);
        return RDKC_FAILURE;
    }

    while(true) {

	c = getopt_long(argc, argv, "s:A::f:a:x:y:e:t:", longOpt, &optIndex);
	if (c == -1) {
	    break;
	}

	switch(c){
	    case 's':
		//set stream ID for which overlay is to be applied
		//stream ID Range (0~3)
		streamID = atoi(optarg) - 1;

		if(streamID < 0  || streamID > 3) {
			RDK_LOG(RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): Invalid stream id. \n", __FUNCTION__, __LINE__);
			exit(0);
		}
		//Get frame resolution
		getStreamResolution(streamID, frameHeight, frameWidth);
		RDK_LOG(RDK_LOG_INFO,"LOG.RDK.OVERLAY","%s(%d): frame dimension:%dx%d \n", __FUNCTION__, __LINE__, frameWidth, frameHeight);
		info.streamID = (OverlayStreamID) streamID;
		RDK_LOG(RDK_LOG_INFO,"LOG.RDK.OVERLAY","%s(%d): stream ID  :%d\n", __FUNCTION__, __LINE__, info.streamID);
		break;

	    case 'A':
		if(optarg) {
		    areaID = atoi(optarg) - 1;
		    if(areaID < 0  || areaID > 3) {
			RDK_LOG(RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): Invalid area id. \n", __FUNCTION__, __LINE__);
			exit(0);
		    }
		} else {
		    areaID = DEFAULT_AREA_ID;
		}
		info.areaID = (OverlayAreaID) areaID;
		break;
 
	    case 'f':
		//overlay bmp file
		strcpy(info.BMPfilename, optarg);
		//calculate img height and width
		getImgDimension(info.BMPfilename, imgHeight, imgWidth);
		RDK_LOG(RDK_LOG_INFO,"LOG.RDK.OVERLAY","%s(%d): overlay image file dimension:%dx%d \n", __FUNCTION__, __LINE__, imgWidth, imgHeight);
		RDK_LOG(RDK_LOG_INFO,"LOG.RDK.OVERLAY","%s(%d): overlay image file :%s  \n", __FUNCTION__, __LINE__, info.BMPfilename);
		break;
	    case 'a':
		if  ( ((xOffset > 0) && (xOffset < (frameWidth-imgWidth)) ) &&
			  ((yOffset >  0) && (yOffset < (frameHeight-imgHeight)) ) ) {
			RDK_LOG(RDK_LOG_INFO,"LOG.RDK.OVERLAY","%s(%d): Valid overlay offset\n", __FUNCTION__, __LINE__);
		} else {
                        RDK_LOG(RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): Invalid offset - Exit overlay app\n", __FUNCTION__, __LINE__);
                        exit(0);
		}

		if( strcmp("TOP_LEFT", optarg) == 0 ) {
			info.x = xOffset;
			info.y = yOffset;
		} else if( strcmp("TOP_RIGHT", optarg) == 0 ) {
                        info.x = frameWidth - (xOffset + imgWidth) ;
			info.y = yOffset;
		} else if( strcmp("BOTTOM_LEFT", optarg) == 0 ) {
                        info.x = xOffset;
                        info.y = frameHeight - (yOffset + imgHeight);
		} else if( strcmp("BOTTOM_RIGHT", optarg) == 0 ) {
                        info.x = frameWidth - (xOffset + imgWidth);
                        info.y = frameHeight - (yOffset + imgHeight);
		} else {
			RDK_LOG(RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): Invalid Position argument.\n", __FUNCTION__, __LINE__);
			exit(0);
		}
		break;
	    case 'x':
		//set x ordinate
		xOffset = atoi(optarg);
#if 0
		if((imgAlignment == RIGHT_JUSTIFIED) &&
		   ((xOffset >  imgWidth) && (xOffset < frameWidth ))) {

		    info.x = frameWidth - xOffset;

		} else if((imgAlignment == LEFT_JUSTIFIED) &&
			  ((xOffset >  0) && (xOffset < (frameWidth-imgWidth) ))) {

		    info.x = xOffset;

		} else if((imgAlignment == TOP_JUSTIFIED) ||
			  (imgAlignment == BOTTOM_JUSTIFIED)){

		    info.x = xOffset;

		} else {
		    RDK_LOG(RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): Invalid X offset.\n", __FUNCTION__, __LINE__);
                    exit(0);
		}
		RDK_LOG(RDK_LOG_INFO,"LOG.RDK.OVERLAY","%s(%d): X-ordinate:%d\n", __FUNCTION__, __LINE__, info.x);
#endif
		break;

	    case 'y':
		//set y ordinate
		yOffset = atoi(optarg);
#if 0
		if((imgAlignment == BOTTOM_JUSTIFIED) &&
                   ((yOffset >  imgHeight) && (yOffset < frameHeight ))) {

                    info.y = frameHeight - yOffset;

                } else if((imgAlignment == TOP_JUSTIFIED) &&
                          ((yOffset >  0) && (yOffset < (frameHeight-imgHeight) ))) {

                    info.y = yOffset;

                } else if((imgAlignment == LEFT_JUSTIFIED) ||
                          (imgAlignment == RIGHT_JUSTIFIED)){

                    info.y = yOffset;

                } else {
                    RDK_LOG(RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): Invalid Y offset.\n", __FUNCTION__, __LINE__);
                    exit(0);
                }
		RDK_LOG(RDK_LOG_INFO,"LOG.RDK.OVERLAY","%s(%d): Y-ordinate:%d\n", __FUNCTION__, __LINE__, info.y);
#endif
		break;
	    case 'e':
		//set if overlay is enabled
		overlayEnable = atoi(optarg);
		break;

	    case 't':
		if(optarg) {
		    alpha = atoi(optarg);
		} else {
		    alpha = DEFAULT_OVERLAY_TRANSPARENCY; 
		}
		info.alpha = alpha;
		RDK_LOG(RDK_LOG_INFO,"LOG.RDK.OVERLAY","%s(%d): transparency:%d\n", __FUNCTION__, __LINE__, info.alpha);
		break;

	    case 'h':
		help();
		exit(0);
		break;

	    case '?':
		break;

	    default:
		break;
	};
    }
    if (overlayEnable) {
	if(OVERLAY_FAILURE == recorder -> SetOverlay(&info)) {
	    RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): ERROR::rdkc_set_overlay \n", __FUNCTION__, __LINE__);
	    return RDKC_FAILURE;
	}
    } else {
	if(OVERLAY_FAILURE == recorder -> disableOverlay(streamID, areaID)) {
	    RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): ERROR::Failed to disable overlay \n", __FUNCTION__, __LINE__);
	    return RDKC_FAILURE;
	}
    }
}
