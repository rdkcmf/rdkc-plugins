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
#ifndef __RDKC_OVERLAY_H__
#define __RDKC_OVERLAY_H__

#include <iostream>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "iav_ioctl.h"
#include "RdkCUtilsCommon.h"

#define OVERLAY_CLUT_NUM	(16)
#define OVERLAY_CLUT_SIZE	(1024)
#define OVERLAY_CLUT_OFFSET	(0)
#define OVERLAY_YUV_OFFSET	(OVERLAY_CLUT_NUM * OVERLAY_CLUT_SIZE)
#define ID_BMP			0x4d42
#define EIGHT_BIT_BMP		8
#define HEX_0x1F		0x1F
#define HEX_0x3			0x3
#define BITMAPFILEHEADER_SIZE   14	//without padding
#define BITMAPINFOHEADER_SIZE   40	//without padding

enum rdkc_status
{
    RDKC_FAILURE = -1,
    RDKC_SUCCESS =  0
};

enum overlay_status
{
    OVERLAY_FAILURE = -1,
    OVERLAY_SUCCESS = 0
};

enum overlay_state
{
    OVERLAY_DISABLE = 0,
    OVERLAY_ENABLE = 1
};

//bmp file header  14bytes
struct BITMAPFILEHEADER
{
    u16 bfType;					// file type
    u32 bfSize;					//file size
    u16 bfReserved1;
    u16 bfReserved2;
    u32 bfOffBits;
};

//bmp inforamtion header 40bytes
struct BITMAPINFOHEADER
{
    u32	biSize;
    u32 biWidth;		//bmp width
    u32 biHeight;		//bmp height
    u16 biPlanes;
    u16 biBitCount;		// 1,4,8,16,24 ,32 color attribute
    u32 biCompression;
    u32 biSizeImage;		//Image size
    u32 biXPelsPerMerer;
    u32 biYPelsPerMerer;
    u32 biClrUsed;
    u32 biClrImportant;
};

//clut, if 256 bit map it needs,
struct RGBQUAD
{
    u8 rgbBlue;
    u8 rgbGreen;
    u8 rgbRed;
    u8 rgbReserved;
};

//color lookup table
struct osdClut {
    u8 v;
    u8 u;
    u8 y;
    u8 alpha;
};

/**
 * @brief This function is used to map overlay buffer to the process
 * @param  void
 * @return OVERLAY_SUCCESS on success and OVERLAY_FAILURE in other cases.
 */
int rdkc_init_overlay(void);

/*
 * @brief This function is used to set overlay information
 * @param info overlay information
 * @return OVERLAY_SUCCESS on success and OVERLAY_FAILURE in other cases.
 */
int rdkc_set_overlay(OverlayInfo* info);

#endif //__RDKC_OVERLAY_H__
