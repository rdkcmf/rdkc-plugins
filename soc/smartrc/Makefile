modules := src

all:
	@for m in $(modules); do echo $$m; make -C $$m $@ || exit 1; done

clean:
	@for m in $(modules); do echo $$m; make -C $$m $@ || exit 1; done

install:
	@for m in $(modules); do echo $$m; make -C $$m $@ || exit 1; done

uninstall:
	@for m in $(modules); do echo $$m; make -C $$m $@ || exit 1; done

.PHONY: all clean install uninstall

