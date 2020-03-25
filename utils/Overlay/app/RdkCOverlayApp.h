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
#ifndef __RDKCOVERLAYAPP_H_
#define __RDKCOVERLAYAPP_H_

#include <iostream>
#include <unistd.h>
#include <csignal>
#include <cstdlib>
#include <asm-generic/errno-base.h>
#include <fcntl.h>
#include <getopt.h>
#include <cerrno>
#include <sys/stat.h>

#include "RdkCPluginFactory.h"
#include "RdkCVideoCapturer.h"
#include "RdkCOverlay.h"
#include "dev_config.h"

#define MAX_SIZE			50
#define PROCESS_EXIST			0
#define LOCK_FILENAME_XFINITY_OVERLAY	"/tmp/xfinity_overlay.lock"
#define DEFAULT_STREAM_ID		STREAM_1
#define DEFAULT_AREA_ID			AREA_1
#define DEFAULT_OVERLAY_X_ORDINATE	20
#define DEFAULT_OVERLAY_Y_ORDINATE	40
#define DEFAULT_OVERLAY_TRANSPARENCY	150
#define DEFAULT_OVERLAY_IMAGE		"/etc/xfinity_logo.bmp"
#define MAX_SLEEP_DURATION		3600 /* 1 hour */

#define FRAME_720P_WIDTH		1280
#define FRAME_720P_HEIGHT		720
#define FRAME_1080P_WIDTH		1920
#define FRAME_1080P_HEIGHT		1080

enum rdkc_status
{
    RDKC_FAILURE = -1,
    RDKC_SUCCESS =  0
};

#endif
