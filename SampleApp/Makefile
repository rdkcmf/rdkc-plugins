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
USE_RFCCONFIG = yes
USE_CONFIGMGR = yes
USE_GST = yes
USE_RDKCSTREAM = yes

include ${RDK_PROJECT_ROOT_PATH}/utility/AppsRule.mak
LDFLAGS = $(LIBFLAGS)

# To build hydra in plugins, BUILD_PLATFORM should be enabled with HYDRA_CAMERA_HANDLER
BUILD_PLATFORM := HYDRA_CAMERA_HANDLER

PLUGIN_ROOT_DIR  := $(RDK_SOURCE_PATH)/

# plugins Includes
INCPATH += $(PLUGIN_ROOT_DIR)/

# Opensource Includes for RDKC
ifeq ($(BUILD_PLATFORM), PLATFORM_RDKC)
INCPATH += $(RDK_PROJECT_ROOT_PATH)/plugins/
INCPATH += $(RDK_PROJECT_ROOT_PATH)/plugins/soc/
endif

# Linking path for RDKC
ifeq ($(BUILD_PLATFORM), PLATFORM_RDKC)
LDFLAGS += -L$(RDK_PROJECT_ROOT_PATH)/plugins/soc/ -lhalgstrtc
endif

ifeq ($(BUILD_PLATFORM), HYDRA_CAMERA_HANDLER)
LDFLAGS += -L$(RDK_PROJECT_ROOT_PATH)/plugins/soc/ -lhalgstrtc
endif

CFLAGS += $(addprefix -I, $(INCPATH))
CFLAGS += -fPIC -Wall 
LDFLAGS += -lpthread 

RM = rm -f  
TARGET_TEST := Start2

all: $(TARGET_TEST)

TEST_SRCS = start.cpp
TEST_OBJ = $(TEST_SRCS:.cpp=.o)

$(TARGET_TEST): $(TEST_OBJ)
	$(CXX) $^  $(LDFLAGS) -o $@

%.o:%.cpp
	$(CXX) -c $< $(CFLAGS) $(INCPATH) -o $@

clean:
	-${RM} ${TARGET_TEST} ${TEST_OBJ} $(SRCS:.cpp=.o)	

.PHONY: all clean
