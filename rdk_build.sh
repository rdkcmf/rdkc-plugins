#!/bin/bash
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

#######################################
#
# Build Framework standard script for
#
# webrtc source code

# use -e to fail on any shell issue
# -e is the requirement from Build Framework
set -e

# default PATHs - use `man readlink` for more info
# the path to combined build
export RDK_PROJECT_ROOT_PATH=${RDK_PROJECT_ROOT_PATH-`readlink -m ..`}
export COMBINED_ROOT=$RDK_PROJECT_ROOT_PATH

# path to build script (this script)
export RDK_SCRIPTS_PATH=${RDK_SCRIPTS_PATH-`readlink -m $0 | xargs dirname`}

# path to components sources and target
export RDK_SOURCE_PATH=${RDK_SOURCE_PATH-`readlink -m .`}
export RDK_TARGET_PATH=${RDK_TARGET_PATH-$RDK_SOURCE_PATH}

#default component name
export RDK_COMPONENT_NAME=${RDK_COMPONENT_NAME-`basename $RDK_SOURCE_PATH`}
export BUILDS_DIR=$RDK_PROJECT_ROOT_PATH
export RDK_DIR=$BUILDS_DIR
export ENABLE_RDKC_LOGGER_SUPPORT=true
export DCA_PATH=$RDK_SOURCE_PATH
source $RDK_SOURCE_PATH/soc/build/soc_env.sh

if [ "$XCAM_MODEL" == "SCHC2" ]; then
. ${RDK_PROJECT_ROOT_PATH}/build/components/amba/sdk/setenv2
elif [ "$XCAM_MODEL" == "SERXW3" ] || [ "$XCAM_MODEL" == "SERICAM2" ]; then
. ${RDK_PROJECT_ROOT_PATH}/build/components/sdk/setenv2
else #No Matching platform
    echo "Source environment that include packages for your platform. The environment variables PROJ_PRERULE_MAK_FILE should refer to the platform s PreRule make"
fi

export PLATFORM_SDK=${RDK_TOOLCHAIN_PATH}
export FSROOT=$RDK_FSROOT_PATH
export FSROOT_PATH=$RDK_FSROOT_PATH

if [ "$XCAM_MODEL" == "SCHC2" ] || [ "$XCAM_MODEL" == "XHB1" ]; then
    echo "Enable xStreamer by default for xCam2 and DBC"
    export ENABLE_XSTREAMER=true
else
    echo "Disable xStreamer by default for xCam and iCam2"
    export ENABLE_XSTREAMER=false
fi

# parse arguments
INITIAL_ARGS=$@

function usage()
{
    set +x
    echo "Usage: `basename $0` [-h|--help] [-v|--verbose] [action]"
    echo "    -h    --help                  : this help"
    echo "    -v    --verbose               : verbose output"
    echo
    echo "Supported actions:"
    echo "      configure, clean, build (DEFAULT), rebuild, install"
}

ARGS=$@


# This Function to perform pre-build configurations before building plugin code
function configure()
{
    echo "Pre-build configurations for  code ..."
    echo "Vijay : $PLATFORM"
    cd $RDK_SOURCE_PATH
    clean

    cd $RDK_DIR
    
    if [ "$PLATFORM" = "S2L" ];then
    	if [ ! -f ${BUILD_SDK_PATH}/.sdk.patched ] ; then
        	echo "Applying patch to amba header file"
         	patch -p0 < ${BUILD_SDK_PATH}/sdk.patch
         	touch ${BUILD_SDK_PATH}/.sdk.patched
    	else
        	echo "Patch already applied to amba header file"
    	fi
    fi
}

# This Function to perform clean the build if any exists already
function clean()
{
    echo "Start Clean"
    cd $RDK_SOURCE_PATH
    cd soc

    make clean
    
    cd -
    #cd gst
  
    #make clean

   # cd $RDK_SOURCE_PATH/SampleApp
   # make clean

    cd $RDK_SOURCE_PATH/utils/Overlay
    make clean


    if [ "$XCAM_MODEL" = "SCHC2" ]; then
        cd $RDK_SOURCE_PATH/xaudio
        make clean

        cd $RDK_SOURCE_PATH/soc/gdma
        make clean
    fi
}

# This Function peforms the build to generate the webrtc.node
function build()
{
    configure
    echo "Configure is done"

    if [ "$XCAM_MODEL" == "SCHC2" ]; then
      cd $RDK_SOURCE_PATH/soc/gdma
      make
    fi

    cd $RDK_SOURCE_PATH

    echo "RDK_SOURCE_PATH :::::: $RDK_SOURCE_PATH"
    cd soc
    make
    make cleanfactory

    if [ "$XCAM_MODEL" == "SCHC2" ]; then
      cd iav_encoder
      make 
      cd -

      cd smartrc
      make
    fi

    cd -
    #cd gst
    #make
    #make cleanfactory
    echo "Build Sample rdk-c................................."
    #cd $RDK_SOURCE_PATH/SampleApp
    #make

    cd $RDK_SOURCE_PATH/utils/Overlay
    make

    #echo "Build Sample GST................................."
    #cd $RDK_SOURCE_PATH/gst/SampleApp
    #make

    if [ "$XCAM_MODEL" = "SCHC2" ]; then
        cd $RDK_SOURCE_PATH/xaudio
        make
    fi

    echo "Plugins build is done"
    echo "Starting make install... $RDK_COMPONENT_NAME"
    install
}

# This Function peforms the rebuild to generate the webrtc.node
function rebuild()
{
    clean
    build
}

# This functions performs installation of webrtc-streaming-node output created into sercomm firmware binary
function install()
{
    echo "Start Plugins Installation"

    cd $RDK_SOURCE_PATH

    if [ -f "soc/libhalgstrtc.so" ]; then
       cp -R soc/libhalgstrtc.so $FSROOT_PATH/usr/lib
    fi
   
    if [ -f "soc/libhalrtc.so" ]; then
       cp -R soc/libhalrtc.so $FSROOT_PATH/usr/lib
    fi

    if [ -f "soc/gdma/libgdma.so" ]; then
       cp -R soc/gdma/libgdma.so $FSROOT_PATH/usr/lib
    fi

    #if [ -f "utils/Overlay/src/librdkcutils.so" ]; then
       #cp -R utils/Overlay/src/librdkcutils.so $FSROOT_PATH/usr/lib
    #fi

    if [ -f "utils/Overlay/app/rdkc_overlay" ]; then
       cp -R utils/Overlay/app/rdkc_overlay $RDK_PROJECT_ROOT_PATH/sdk/fsroot/src//vendor/img/fs/shadow_root/usr/local/bin/
    fi

    if [ -f "utils/Overlay/app/overlay.ini" ];then
       cp -R utils/Overlay/app/overlay.ini $FSROOT_PATH/etc/rfcdefaults/
    fi

    if [ -f "soc/Overlay_src/img/xfinity_logo.bmp" ]; then
	cp -R soc/Overlay_src/img/xfinity_logo.bmp $FSROOT_PATH/etc
    fi

    if [ -f "soc/Overlay_src/img/xfinity_logo_1080p.bmp" ]; then
        cp -R soc/Overlay_src/img/xfinity_logo_1080p.bmp $FSROOT_PATH/etc
    fi

    if [ -f "soc/Overlay_src/img/xfinity_logo_360p.bmp" ]; then
        cp -R soc/Overlay_src/img/xfinity_logo_360p.bmp $FSROOT_PATH/etc
    fi

    if [ -f "soc/Overlay_src/img/cox.bmp" ]; then
        cp -R soc/Overlay_src/img/cox.bmp $FSROOT_PATH/etc
    fi

    if [ -f "soc/Overlay_src/img/helix_logo.bmp" ]; then
	cp -R soc/Overlay_src/img/helix_logo.bmp $FSROOT_PATH/etc
    fi

    if [ -f "soc/Overlay_src/img/helix_logo_1080p.bmp" ]; then
        cp -R soc/Overlay_src/img/helix_logo_1080p.bmp $FSROOT_PATH/etc
    fi

    if [ -f "soc/Overlay_src/img/helix_logo_360p.bmp" ]; then
        cp -R soc/Overlay_src/img/helix_logo_360p.bmp $FSROOT_PATH/etc
    fi

    #if [ -f "gst/libgstrtc.so" ]; then
    #   cp -R gst/libgstrtc.so $FSROOT_PATH/usr/lib
    #fi

    if [ "$XCAM_MODEL" == "SCHC2" ]; then
      if [ -f "soc/iav_encoder/src/iav_encoder" ]; then
        cp -R soc/iav_encoder/src/iav_encoder $FSROOT/usr/bin
      fi
    fi

    if [ $ENABLE_XSTREAMER == 'true' ]; then
        echo "xStreamer is enabled, hence skip copying xaudio library and smartrc binary to final image, instead it gets copied from xStreamer repo"
    else
        echo "Copying xaudio library to final image"
        if [ "$XCAM_MODEL" = "SCHC2" ]; then
            if [ -f "xaudio/libxaudioapi.so" ]; then
              cp -R xaudio/libxaudioapi.so $FSROOT_PATH/usr/lib
            fi

            if [ -f "soc/smartrc/src/smartrc" ]; then
              cp -R soc/smartrc/src/smartrc $FSROOT/usr/bin
            fi
        fi
    fi

    echo "Plugins Installation is done"
}

function setHydra()
{
    echo "setHydra - Disable xStreamer"
    export ENABLE_XSTREAMER=false
}

# run the logic
#these args are what left untouched after parse_args
HIT=false

for i in "$@"; do
    case $i in
        enableHydra)  HIT=true; setHydra ;;
        configure)  HIT=true; configure ;;
        clean)      HIT=true; clean ;;
        build)      HIT=true; build ;;
        rebuild)    HIT=true; rebuild ;;
        install)    HIT=true; install ;;
        *)
            #skip unknown
        ;;
    esac
done

# if not HIT do build by default
if ! $HIT; then
  build
fi

