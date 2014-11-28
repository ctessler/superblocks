CC=sparc-elf-gcc
CFLAGS=-O0 -g -msoft-float -lsmall

NAME := $(shell basename ${PWD})

#
# Do instruction caches?
#
ICACHE?=1
ifeq ($(ICACHE), 1)
	ITARGETS=$(NAME)-imatrix.txt $(NAME)-icache.png
else
     isetargs := --noinstruction
endif

#
# Do data caches?
#
DCACHE?=1
ifeq ($(DCACHE), 1)
	DTARGETS=$(NAME)-dmatrix.txt $(NAME)-dcache.png
else
     isetargs := --nodata
endif

TARGETS = $(ITARGETS) $(DTARGETS)

ifeq ($(DCACHE), 1)
  ifeq ($(ICACHE), 1)
	COMBINED=1
  endif
endif

ifeq ($(COMBINED), 1)
	TEMP := $(TARGETS) $(NAME)-ucb.txt
	TARGETS := $(TEMP)
endif

UCBTOP: $(TARGETS)

$(NAME)-ucb.txt: caches/ISET
	cp -a caches/ucb_bound.txt $(NAME)-ucb.txt

$(NAME)-dcache.png $(NAME)-dcache.eps : caches/PATCHED
	cd caches ; gnuplot dcache.p
	cd caches ; gnuplot dcache-eps.p
	cp caches/ducb_bound.txt $(NAME)-ducb.txt
	cp -a caches/*dcache.png $(NAME)-dcache.png
	cp -a caches/*dcache.eps $(NAME)-dcache.eps


$(NAME)-icache.png $(NAME)-icache.eps : caches/PATCHED
	cd caches ; gnuplot icache.p
	cd caches ; gnuplot icache-eps.p
	cp caches/iucb_bound.txt $(NAME)-iucb.txt
	cp -a caches/*icache.png $(NAME)-icache.png
	cp -a caches/*icache.eps $(NAME)-icache.eps

caches/PATCHED: caches
	cd caches ; for i in ../patch/*; do patch -p1 < $$i; done
	touch caches/PATCHED

$(NAME)-dmatrix.txt: caches/ISET
	cp -a caches/dcache.matrix $(NAME)-dmatrix.txt

$(NAME)-imatrix.txt: caches/ISET
	cp -a caches/icache.matrix $(NAME)-imatrix.txt

caches/ISET: caches
	cd caches; cache-iset.pl $(isetargs)
	touch caches/ISET

caches: $(NAME)-observed.txt
	collect-caches.pl $(NAME) $(NAME)-observed.txt

$(NAME)-observed.txt: $(NAME)-ordered.txt
	time-blocks.pl $(NAME) $(NAME)-ordered.txt

$(NAME)-ordered.txt: $(NAME)-256blocks.txt $(NAME)
	order-blocks.pl $(NAME) $(NAME)-256blocks.txt

$(NAME)-256blocks.txt: $(NAME)-blocks.txt $(NAME)
	sort -R $(NAME)-blocks.txt | head -n 256 > $(NAME)-256blocks.txt

$(NAME): $(NAME).c

clean:
	rm -f $(NAME) $(NAME)-observed.txt $(NAME)-ordered.txt $(NAME)-c.txt
	rm -f $(NAME)-256blocks.txt $(NAME)-ducb.txt $(NAME)-iucb.txt
	rm -f $(NAME)-cycles.txt $(NAME)-dmatrix.txt $(NAME)-imatrix.txt
	rm -f $(NAME)-icache.png $(NAME)-icache.eps
	rm -f $(NAME)-dcache.png $(NAME)-dcache.eps
	rm -f $(NAME)-ucb.txt
	rm -rf caches
