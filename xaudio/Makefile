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

CXXFLAGS += -std=c++14 -fPIC -Wall -Wextra
ifeq ($(XCAM_MODEL), SCHC2)
CXXFLAGS += -DXCAM2
endif

TARGET_APP = xaudiotest
SRCS_APP += main.cpp logger.cpp
OBJS_APP = $(SRCS_APP:.cpp=.o)

TARGET_LIB = libxaudioapi.so
SRCS_LIB += xaudioapi.cpp logger.cpp
OBJS_LIB = $(SRCS_LIB:.cpp=.o)

all: $(TARGET_LIB) $(TARGET_APP)

$(TARGET_LIB): $(OBJS_LIB)
	$(CXX) -o $(@) --shared $^ $(LDFLAGS)

$(TARGET_APP): $(OBJS_APP)
	$(CXX) -o $(@) $^ $(LDFLAGS) -L./ -lxaudioapi

%.o:%.cpp
	$(CXX) -c $< $(CXXFLAGS)  -o $@

clean:
	$(RM) $(TARGET_LIB) $(OBJS_LIB)  $(TARGET_APP) $(OBJS_APP)
