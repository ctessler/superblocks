SUBDIRS = adpcm bsort100 cnt cover crc fft1 fibcall lms ndes recursion
CLEANDIRS := $(SUBDIRS)

.PHONY: all subdirs $(SUBDIRS) 

all: TARGET=all
clean: TARGET=clean

all clean subdirs: $(SUBDIRS)

$(SUBDIRS):
	cd $@ ; make $(TARGET)


