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
USE_LIBSYSWRAPPER = yes

include ${RDK_PROJECT_ROOT_PATH}/utility/AppsRule.mak
LIBS = $(LIBFLAGS)

TARGET :=  librdkcutils.so

CFLAGS += -I$(RDK_SOURCE_PATH)/utils/Overlay/include

SOURCES := RdkCOverlay.cpp

OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET):$(OBJECTS)
	$(CXX) -shared $^ $(LDFLAGS) -o $@

%.o:%.cpp
	$(CXX) -c $< $(CFLAGS)  -o $@
clean:
	rm -rf $(TARGET) $(OBJECTS)
