CC=sparc-elf-gcc
CFLAGS=-O0 -g -msoft-float -lsmall

NAME=bsort100

all: $(NAME)-dmatrix.txt $(NAME)-imatrix.txt \
	$(NAME)-dcache.png $(NAME)-icache.png

$(NAME)-dcache.png $(NAME)-dcache.eps : caches/PATCHED
	cd caches ; gnuplot dcache.p
	cd caches ; gnuplot dcache-eps.p
	cp caches/ucb_bound.txt $(NAME)-ucb.txt
	cp -a caches/bsort-dcache.png $(NAME)-dcache.png
	cp -a caches/bsort-dcache.eps $(NAME)-dcache.eps


$(NAME)-icache.png $(NAME)-icache.eps : caches/PATCHED
	cd caches ; gnuplot icache.p
	cd caches ; gnuplot icache-eps.p
	cp -a caches/icache.png $(NAME)-icache.png
	cp -a caches/bsort-icache.eps $(NAME)-icache.eps

caches/PATCHED: caches
	cd caches ; for i in ../patch/*; do patch -p1 < $$i; done
	touch caches/PATCHED

$(NAME)-dmatrix.txt: caches/ISET
	cp -a caches/dcache.matrix $(NAME)-dmatrix.txt

$(NAME)-imatrix.txt: caches/ISET
	cp -a caches/icache.matrix $(NAME)-imatrix.txt

caches/ISET: caches
	cd caches; cache-iset.pl
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
	rm -f $(NAME)-cycles.txt $(NAME)-dmatrix.txt $(NAME)-imatrix.txt
	rm -f $(NAME)-icache.png $(NAME)-icache.eps
	rm -f $(NAME)-dcache.png $(NAME)-dcache.eps
	rm -f $(NAME)-ucb.txt
	rm -rf caches
