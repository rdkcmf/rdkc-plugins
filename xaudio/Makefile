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

MKDIR = /bin/mkdir -p
INSTALL = install 

# Add dependent libraries
USE_AAC = yes
SUPPORT_MXML = yes

include ${RDK_PROJECT_ROOT_PATH}/utility/AppsRule.mak
LDFLAGS = $(LIBFLAGS)
CXXFLAGS = $(CFLAGS)

RM = rm -fr

CXXFLAGS += -g -std=c++14 -fPIC -Wall -Wextra
ifeq ($(XCAM_MODEL), SCHC2)
CXXFLAGS += -DXCAM2
endif

TARGET_APP = xaudiotest
SRCS_APP += main.cpp logger.cpp
OBJS_APP = $(SRCS_APP:.cpp=.o)

RELEASE_TARGET = libxaudioapi.so
DEBUG_TARGET = libxaudioapi_debug.so
SRCS_LIB += xaudioapi.cpp logger.cpp
OBJS_LIB = $(SRCS_LIB:.cpp=.o)

all: $(RELEASE_TARGET) $(TARGET_APP) $(DEBUG_TARGET) install

$(RELEASE_TARGET): $(OBJS_LIB)
	$(CXX) -o $(@) --shared $^ $(LDFLAGS)
	$(STRIP) $(RELEASE_TARGET)

$(DEBUG_TARGET): $(OBJS_LIB)
	$(CXX) -o $(@) --shared $^ $(LDFLAGS)

$(TARGET_APP): $(OBJS_APP)
	$(CXX) -o $(@) $^ $(LDFLAGS) -L./ -lxaudioapi
	$(STRIP) $(TARGET_APP)

%.o:%.cpp
	$(CXX) -c $< $(CXXFLAGS)  -o $@

install:
ifeq ($(ENABLE_XSTREAMER), true)
	echo "xStreamer is enabled, skip copying xaudio library symbols, instead it gets copied from xStreamer repo"
else

ifneq ($(XCAM_MODEL), XHB1)
	$(PLATFORM_BREAKPAD_BINARY) $(DEBUG_TARGET) > $(RELEASE_TARGET).sym
	sed -i "1s/$(DEBUG_TARGET)/$(RELEASE_TARGET)/" $(RELEASE_TARGET).sym
	mv -f $(RELEASE_TARGET).sym  $(PLATFORM_SYMBOL_PATH)
	echo "Debug symbol created for $(RELEASE_TARGET)"
endif

endif

clean:
	$(RM) $(RELEASE_TARGET) $(DEBUG_TARGET) $(OBJS_LIB) $(TARGET_APP) $(OBJS_APP)
