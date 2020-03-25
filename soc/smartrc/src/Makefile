# Add dependent libraries
USE_CONFIGMGR = yes
USE_RFCCONFIG = yes
USE_STREAMUTILS = yes
USE_SYSUTILS = yes
USE_RDKCSTREAM = yes
USE_RDKC = yes
SUPPORT_MXML = yes
USE_SMARTRC = yes

include ${RDK_PROJECT_ROOT_PATH}/utility/AppsRule.mak
LDFLAGS = $(LIBFLAGS)
CXXFLAGS = $(CFLAGS)

OBJDIR=obj

SMARTRC_SRC=test_smartrc_Amba.c
SMARTRC_OBJS=$(patsubst %.c, $(OBJDIR)/%.o, $(SMARTRC_SRC))

all: smartrc


$(OBJDIR)/%.o: %.c
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	$(CC) -c $(CXXFLAGS) $< -o $@

smartrc: $(SMARTRC_OBJS)
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	$(CC) $(SMARTRC_OBJS) -L. $(LDFLAGS) -o $@

clean:
	rm -rf $(OBJDIR)
	rm -f smartrc

.PHONY: all clean

