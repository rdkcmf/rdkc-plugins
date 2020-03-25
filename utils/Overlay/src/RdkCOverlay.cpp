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
#include "RdkCOverlay.h"

static int fDescOverlay;

static int overlaySize;
static int overlayYUVSize;
static u8  *overlayAddr;
static u8  *overlayData;

static u32 clutAddrOffset;

static int overlayImgHeight;
static int overlayImgWidth;
static int overlayImgPitch;
static int overlayImgSize;
static int overlayImgColorCount;

static int BmpDataOffset;
static int transparency;
static iav_overlay_insert overlay, tempOverlay;

/* defines if reverting to previous overlay is possible */
static int revertOverlay;

/*
 * @brief: Overlay initialization
 * @param: void
 * @return: OVERLAY_SUCCESS on success, OVERLAY_FAILURE on failure
 */
int rdkc_init_overlay(void)
{
    struct iav_querybuf queryBuf;
    memset(&queryBuf,0,sizeof(iav_querybuf));

    if ((fDescOverlay = open("/dev/iav", O_RDWR, 0)) < 0) {
        RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): /dev/iav\n", __FUNCTION__, __LINE__);
        return OVERLAY_FAILURE;
    }

    queryBuf.buf = IAV_BUFFER_OVERLAY;
    if (ioctl(fDescOverlay, IAV_IOC_QUERY_BUF, &queryBuf) < 0) {
        RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): IAV_IOC_QUERY_BUF\n", __FUNCTION__, __LINE__);
        return OVERLAY_FAILURE;
    }

    overlaySize = queryBuf.length;
    overlayAddr = (u8*)mmap(NULL, overlaySize, PROT_WRITE, MAP_SHARED,
        fDescOverlay, queryBuf.offset);


    if (overlayAddr == MAP_FAILED) {
        RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): mmap failed\n", __FUNCTION__, __LINE__);
        return OVERLAY_FAILURE;
    }

    //split into MAX_ENCODE_STREAM parts
    overlayYUVSize = (overlaySize - OVERLAY_YUV_OFFSET) / MAX_STREAM_NUM;

    return OVERLAY_SUCCESS;
}

/*
 * @brief: Verify stream id
 * @param: streamID: stream id
 * @return: OVERLAY_SUCCESS on success, OVERLAY_FAILURE on failure
 */
static int verify_stream_encode_state(int streamID)
{
    u32 state;

    if((streamID < 0) || (streamID > MAX_STREAM_NUM)) {
	RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): Invalid stream ID\n", __FUNCTION__, __LINE__);
	return OVERLAY_FAILURE;
    }

    if (ioctl(fDescOverlay, IAV_IOC_GET_IAV_STATE, &state) < 0) {
        RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): IAV_IOC_GET_IAV_STATE\n", __FUNCTION__, __LINE__);
        return OVERLAY_FAILURE;
    }

    if ((state != IAV_STATE_PREVIEW) &&
        (state != IAV_STATE_ENCODING)) {
        RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): Overlay need iav to be in preview or encoding, cur state is :%d\n", __FUNCTION__, __LINE__,state);
	return OVERLAY_FAILURE;
    }

    return OVERLAY_SUCCESS;
}

/*
 * @brief: Verify area id
 * @param: areaID: area id
 * @return: OVERLAY_SUCCESS on success, OVERLAY_FAILURE on failure
 */
static int verify_area_ID(int areaID)
{
    if((areaID < 0) || (areaID > MAX_AREA_NUM)) {
        RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d):Invalid area ID\n", __FUNCTION__, __LINE__);
        return OVERLAY_FAILURE;
    }

    return OVERLAY_SUCCESS;
}

/*
 * @brief: Verify overlay image
 * @param: overlayImage: Imgae name
 * @return: OVERLAY_SUCCESS on success, OVERLAY_FAILURE on failure
 */
static int verify_overlay_image(char* overlayImage)
{
    FILE *fp = NULL;
    BITMAPFILEHEADER fileHead;
    BITMAPINFOHEADER infoHead;

    if ((fp = fopen(overlayImage, "r")) == NULL) {
        RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): %s open fail or not exists \n", __FUNCTION__, __LINE__,overlayImage);
        return OVERLAY_FAILURE;
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
        return OVERLAY_FAILURE;
    }

    fread(&infoHead, sizeof(BITMAPINFOHEADER), 1, fp);

    BmpDataOffset = ftell(fp);

    int biW = infoHead.biWidth;
    int biH = infoHead.biHeight;
    int sizeImg = infoHead.biSizeImage;

    //std::cout << "Height:: " << biH << "\n";
    //std::cout << "Width:: " << biW << "\n";
    //std::cout << "Img Size:: " << sizeImg << "\n";

    if (infoHead.biBitCount > EIGHT_BIT_BMP) {
        RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): please use 8bit bmp file \n", __FUNCTION__, __LINE__);
        fclose(fp);
        return OVERLAY_FAILURE;
    }

    if ((biW & HEX_0x1F) || (biH & HEX_0x3)) {
        fclose(fp);
        RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): the image size is %dx%d, width must be multiple of 32, height must be multiple of 4.\n", __FUNCTION__, __LINE__, biW, biH);
        return OVERLAY_FAILURE;
    }

    overlayImgHeight = biH;
    overlayImgWidth  = biW;
    overlayImgPitch  = biW;
    overlayImgSize   = sizeImg;

    overlayImgColorCount = (fileHead.bfOffBits - BITMAPFILEHEADER_SIZE - BITMAPINFOHEADER_SIZE) / sizeof(RGBQUAD);

    fclose(fp);

    return OVERLAY_SUCCESS;
}

/*
 * @brief: Verify overlay info
 * @param: info: OverlayInfo structure
 * @return: OVERLAY_SUCCESS on success, OVERLAY_FAILURE on failure
 */
static int validate_overlay_info(OverlayInfo* info)
{
    if(OVERLAY_FAILURE == verify_stream_encode_state(info -> streamID))
	return OVERLAY_FAILURE;

    if(OVERLAY_FAILURE == verify_area_ID(info -> areaID))
	return OVERLAY_FAILURE;

    if(OVERLAY_FAILURE == verify_overlay_image(info -> BMPfilename))
	return OVERLAY_FAILURE;

    return OVERLAY_SUCCESS;
}

/*
 * @brief: Convert RGB pixel value to corresponding yuv value
 * @param: RGBQUAD: RGB
 * @param: osdClut: YUV
 * @return: void
 */
static void RgbToYuv(const RGBQUAD * rgb, osdClut *yuv)
{
    yuv->y = (u8)(0.257f * rgb->rgbRed + 0.504f * rgb->rgbGreen + 0.098f * rgb->rgbBlue + 16);
    yuv->u = (u8)(0.439f * rgb->rgbBlue - 0.291f * rgb->rgbGreen - 0.148f * rgb->rgbRed + 128);
    yuv->v = (u8)(0.439f * rgb->rgbRed - 0.368f * rgb->rgbGreen - 0.071f * rgb->rgbBlue + 128);

    // Make black colour full transparent
    if( (rgb->rgbRed == 0) && (rgb->rgbBlue == 0) && (rgb->rgbGreen == 0) ) {
	yuv->alpha =0;
    }
    else {
	yuv->alpha = transparency;
    }
}

/*
 * @brief: Convert RGB image to YUV image copies it to buffer
 * @param: streamID: stream id
 * @param: areaID: area id
 * @param: fname: file name
 * @param: data: frame data
 * @return: OVERLAY_SUCCESS on success, OVERLAY_FAILURE on failure
 */
static int bmp_convert(int streamID, int areaID, char *fname, u8* data)
{
    int count = 0;
    FILE *fp = NULL;
    int clutID = -1;
    osdClut *clutData = NULL;
    RGBQUAD  rgb;
    memset(&rgb, 0, sizeof(RGBQUAD));

    if ((fp = fopen(fname, "r")) == NULL) {
        RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): %s  open fail or not exists\n", __FUNCTION__, __LINE__, fname);
        return OVERLAY_FAILURE;
    }

    fseek(fp, BmpDataOffset, SEEK_SET);

    clutID = streamID * MAX_AREA_NUM + areaID;
    clutData = (osdClut *)
        (overlayAddr + OVERLAY_CLUT_SIZE * clutID);

    clutAddrOffset = (u32)((u8*)clutData - overlayAddr);

    memset(clutData, 0, OVERLAY_CLUT_SIZE);

    for(count = 0; count < overlayImgColorCount; count++) {
        fread(&rgb, sizeof(RGBQUAD), 1, fp);
        RgbToYuv(&rgb, &clutData[count]);
    }

    memset(data, 0, overlayImgSize);

    for(count = 0; count < overlayImgHeight; count++) {
        fread(data + (overlayImgHeight - 1 - count) * overlayImgWidth, 1, overlayImgWidth, fp);
    }

    fclose(fp);

    return OVERLAY_SUCCESS;
}

/*
 * @breif: revert overlay image to previous configuration
 * @param: void
 * @return: OVERLAY_SUCCESS on success, OVERLAY_FAILURE on failure
 */
static int revert_overlay()
{
    RDK_LOG( RDK_LOG_INFO,"LOG.RDK.OVERLAY","%s(%d): Reverting to previous working overlay configuration\n", __FUNCTION__, __LINE__);

    if (ioctl(fDescOverlay, IAV_IOC_SET_OVERLAY_INSERT,
        &tempOverlay) < 0) {
        RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): IAV_IOC_SET_OVERLAY_INSERT failed while reverting.\n", __FUNCTION__, __LINE__);

        return OVERLAY_FAILURE;
    }

    return OVERLAY_SUCCESS;
}

/*
 * @breif: saves overlay image info from buffer
 * @param: info: OverlayInfo structure
 * @param: stream: stream ID for which overlay info is needed
 * @return: OVERLAY_SUCCESS on success, OVERLAY_FAILURE on failure
 */
static int save_overlay(iav_overlay_insert *info , int stream)
{
    if(NULL == info) {
        RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): trying to access NULL memory location.\n", __FUNCTION__, __LINE__);
        return OVERLAY_FAILURE;
    }

    info -> id = stream;
    if (ioctl(fDescOverlay, IAV_IOC_GET_OVERLAY_INSERT, info) < 0) {
        RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): IAV_IOC_GET_OVERLAY_INSERT failed.\n", __FUNCTION__, __LINE__);
        return OVERLAY_FAILURE;
    }

    RDK_LOG(RDK_LOG_INFO,"LOG.RDK.OVERLAY","%s(%d): IAV_IOC_GET_OVERLAY_INSERT successful.\n", __FUNCTION__, __LINE__);
    return OVERLAY_SUCCESS;
}

/*
 * @breif: set overlay image on buffer
 * @param: info: OverlayInfo structure
 * @return: OVERLAY_SUCCESS on success, OVERLAY_FAILURE on failure
 */
int rdkc_set_overlay(OverlayInfo *info)
{
    if(OVERLAY_FAILURE == validate_overlay_info(info)){
	RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): Invalid data provided\n", __FUNCTION__, __LINE__);
	return OVERLAY_FAILURE;
    }

    transparency = info -> alpha;

    overlayData = overlayAddr + OVERLAY_YUV_OFFSET +
        overlayYUVSize * (info -> streamID);

    if(OVERLAY_SUCCESS == bmp_convert(info -> streamID, info -> areaID, info -> BMPfilename, overlayData))
	RDK_LOG( RDK_LOG_INFO,"LOG.RDK.OVERLAY","%s(%d): BMP image successfully converted\n", __FUNCTION__, __LINE__);

    memset(&overlay, 0, sizeof(overlay));
    memset(&tempOverlay, 0, sizeof(overlay));

    if(save_overlay(&tempOverlay, info -> streamID) == OVERLAY_SUCCESS) {
        revertOverlay = 1;
    }

    overlay.id = info -> streamID;
    overlay.enable = OVERLAY_ENABLE;
    overlay.area[info -> areaID].enable = OVERLAY_ENABLE;
    overlay.area[info -> areaID].width  = overlayImgWidth;
    overlay.area[info -> areaID].height = overlayImgHeight;
    overlay.area[info -> areaID].pitch  = overlayImgPitch;
    overlay.area[info -> areaID].total_size =
		overlayImgHeight * overlayImgPitch;

    if (overlay.area[info -> areaID].total_size > overlayYUVSize){
	RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): total size  of all areas can not be larger than the buffer size \n", __FUNCTION__, __LINE__);
	return OVERLAY_FAILURE;
    }

    overlay.area[info -> areaID].start_x = info -> x;
    overlay.area[info -> areaID].start_y = info -> y;
    overlay.area[info -> areaID].clut_addr_offset = clutAddrOffset;
    overlay.area[info -> areaID].data_addr_offset = (overlayData - overlayAddr);

    if (ioctl(fDescOverlay, IAV_IOC_SET_OVERLAY_INSERT,
        &overlay) < 0) {

	RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): IAV_IOC_SET_OVERLAY_INSERT failed.\n", __FUNCTION__, __LINE__);

	if(revertOverlay && (revert_overlay() != OVERLAY_SUCCESS)) {
            RDK_LOG( RDK_LOG_ERROR,"LOG.RDK.OVERLAY","%s(%d): Error while reverting to earlier overlay config.\n", __FUNCTION__, __LINE__);
            revertOverlay = 0;
            return OVERLAY_FAILURE;
        }

    }

    RDK_LOG( RDK_LOG_INFO ,"LOG.RDK.OVERLAY","%s(%d): IAV_IOC_SET_OVERLAY_INSERT successful.\n", __FUNCTION__, __LINE__);
    return OVERLAY_SUCCESS;
}
