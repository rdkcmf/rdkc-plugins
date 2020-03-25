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
#ifndef __RDKC_UTILS_COMMOM__
#define __RDKC_UTILS_COMMOM__

#include "rdk_debug.h"
#define LOGGER_PATH "/etc/debug.ini"

/****************************** OVERLAY ******************************/

/* Stream ID for Overlay */
enum OverlayStreamID{
	STREAM_1 = 0,
	STREAM_2 = 1,
	STREAM_3 = 2,
	STREAM_4 = 3,
	MAX_STREAM_NUM = 4
};

/* Area ID for Overlay */
enum OverlayAreaID{
	AREA_1 = 0,
	AREA_2 = 1,
	AREA_3 = 2,
	AREA_4 = 3,
	MAX_AREA_NUM = 4
};

/* Info about overlay */
struct OverlayInfo{
	OverlayStreamID streamID;
	OverlayAreaID areaID;
	int x;
	int y;
	char BMPfilename[1024];
	int alpha;
};

/****************************** OVERLAY ******************************/

#endif //__RDKC_UTILS_COMMOM__
