SUBDIRS = adpcm bsort100 cnt cover crc fft1 fibcall lms ndes recursion
CLEANDIRS := $(SUBDIRS)

.PHONY: all subdirs $(SUBDIRS) package

all: TARGET=all
clean: TARGET=clean

all: package
all clean subdirs: $(SUBDIRS)

$(SUBDIRS):
	cd $@ ; make $(TARGET)

DATE := $(shell date +%F.%H%M)
package:
	mkdir $(DATE)
	cd $(DATE); mkdir $(SUBDIRS)
	for dir in $(SUBDIRS) ; do \
	    cp $$dir/*-c.txt $(DATE)/$$dir/. \
	    cp $$dir/*-cycles.txt $(DATE)/$$dir/. \
	    cp $$dir/*-?matrix.txt $(DATE)/$$dir/. \
	    cp $$dir/*-?ucb.txt $(DATE)/$$dir/. \
	    cp $$dir/*-ucb.txt $(DATE)/$$dir/. \
	; done
	tar -cvzf $(DATE).tar.gz $(DATE)


