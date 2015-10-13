DEBUG = -g -O0
#DEBUG = -O3
CCOPTS = -D_GNU_SOURCE -Wstrict-prototypes -Wall $(EXTRA_CCOPTS)
INCLUDE = -I/usr/local/include
CFLAGS = $(CCOPTS) $(INCLUDE) $(DEBUG) -I../include -std=gnu99

LDFLAGS = -L/usr/local/lib -L../lib
LDLIBS = -lwiringPi -lpthread -lm

SUBDIRS=lib digiled

all:
	@set -e; \
	for i in $(SUBDIRS); \
	do $(MAKE) $(MFLAGS) -C $$i; done

clean:
	for i in $(SUBDIRS); \
	do $(MAKE) $(MFLAGS) -C $$i clean; done

.EXPORT_ALL_VARIABLES:

