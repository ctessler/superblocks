NAMES = adpcm bsort100 cnt cover crc fft1 fibcall lms ndes recursion
EX = $(patsubst %,../ex/%,$(NAMES))
TXTS = $(patsubst %,%.txt,$(NAMES))

$(TXTS):

%.txt: ../ex/%
	cat $$(echo $< |  sed 'smex/\(.*\)mex\/\1\/\1-c.txtm') | grep timed | \
	awk '{print "WCET: " $$5}' > $@
	cat $$(echo $< |  sed 'smex/\(.*\)mex\/\1\/\1-ucb.txtm') | \
	awk '{print "UCBS: " $$4}' >> $@

table: $(TXTS)
	uoe-breakdown.pl -b 390 -q -t $(TXTS)

clean:
	rm -f *.txt
