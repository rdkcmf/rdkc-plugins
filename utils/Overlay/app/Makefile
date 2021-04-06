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

# Add dependent libraries
USE_CONFIGMGR = yes
USE_RFCCONFIG = yes
SUPPORT_MXML = yes
USE_RDKCSTREAM = yes
USE_LIBSYSWRAPPER = yes
ifeq ($(BUILD_MODE),WITH_GST)
USE_GST = yes
else
USE_PLUGINS = yes
endif
USE_BREAKPAD = yes

include ${RDK_PROJECT_ROOT_PATH}/utility/AppsRule.mak
LDFLAGS = $(LIBFLAGS)

RELEASE_TARGET :=  rdkc_overlay
DEBUG_TARGET :=  rdkc_overlay_debug

CFLAGS  += -I$(RDK_SOURCE_PATH)/soc/Overlay_src/include
CFLAGS  += -I$(RDK_PROJECT_ROOT_PATH)/plugins

ifeq ($(BUILD_MODE),WITH_GST)
LDFLAGS += -L$(RDK_PROJECT_ROOT_PATH)/plugins/soc -lhalgstrtc
endif

ifeq ($(PLATFORM),S3L)
CFLAGS  += -DS3L
endif

CFLAGS  += -g

SOURCES := RdkCOverlayApp.cpp

OBJECTS = $(SOURCES:.cpp=.o)

all: $(RELEASE_TARGET) $(DEBUG_TARGET) install

$(RELEASE_TARGET):$(OBJECTS)
	$(CXX) $(LDFLAGS) $^ -o $@
	$(STRIP) $(RELEASE_TARGET)

$(DEBUG_TARGET):$(OBJECTS)
	$(CXX) $(LDFLAGS) $^ -o $@

%.o:%.cpp
	$(CXX) $(CFLAGS) -c $(SOURCES) -o $@

install:
ifneq ($(XCAM_MODEL), XHB1)
	$(PLATFORM_BREAKPAD_BINARY) $(DEBUG_TARGET) > $(RELEASE_TARGET).sym
	sed -i "1s/$(DEBUG_TARGET)/$(RELEASE_TARGET)/" $(RELEASE_TARGET).sym
	mv -f $(RELEASE_TARGET).sym  $(PLATFORM_SYMBOL_PATH)
	echo "Debug symbol created for $(RELEASE_TARGET)"
endif

clean:
	rm -rf $(RELEASE_TARGET) $(OBJECTS)
