##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2020 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
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
USE_STREAMUTILS = yes
USE_SYSUTILS = yes
USE_RDKCSTREAM = yes
USE_RDKC = yes
SUPPORT_MXML = yes
USE_SMARTRC = yes
USE_LIBSYSWRAPPER = yes
USE_BREAKPAD = yes

include ${RDK_PROJECT_ROOT_PATH}/utility/AppsRule.mak
LDFLAGS = $(LIBFLAGS)
CXXFLAGS = $(CFLAGS)

CXXFLAGS += -g

OBJDIR=obj

SMARTRC_SRC=test_smartrc_Amba.c
SMARTRC_OBJS=$(patsubst %.c, $(OBJDIR)/%.o, $(SMARTRC_SRC))

RELEASE_TARGET = smartrc
DEBUG_TARGET = smartrc_debug

all: $(RELEASE_TARGET) $(DEBUG_TARGET) install

$(OBJDIR)/%.o: %.c
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	$(CC) -c $(CXXFLAGS) $< -o $@

$(RELEASE_TARGET): $(SMARTRC_OBJS)
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	$(CC) $(SMARTRC_OBJS) -L. $(LDFLAGS) -o $@
	$(STRIP) $(RELEASE_TARGET)

$(DEBUG_TARGET): $(SMARTRC_OBJS)
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	$(CC) $(SMARTRC_OBJS) -L. $(LDFLAGS) -o $@

install:
ifeq ($(ENABLE_XSTREAMER), true)
	echo "xStreamer is enabled, skip copying smartrc symbols, instead it gets copied from xStreamer repo"
else

ifneq ($(XCAM_MODEL), XHB1)
	$(PLATFORM_BREAKPAD_BINARY) $(DEBUG_TARGET) > $(RELEASE_TARGET).sym
	mv -f $(RELEASE_TARGET).sym  $(PLATFORM_SYMBOL_PATH)
	echo "Debug symbol created for $(RELEASE_TARGET)"
endif

endif

clean:
	rm -rf $(OBJDIR)
	rm -f $(RELEASE_TARGET) $(DEBUG_TARGET)

.PHONY: all clean

