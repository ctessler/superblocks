CC=sparc-elf-gcc
CFLAGS=-O0 -g -msoft-float -lsmall

NAME=cnt

all: $(NAME)-dmatrix.txt $(NAME)-imatrix.txt \
	$(NAME)-dcache.png $(NAME)-icache.png

.PHONY: collect patches

$(NAME)-dcache.png $(NAME)-dcache.eps : patches
	cd caches ; gnuplot dcache.p
	cd caches ; gnuplot dcache-eps.p
	cp caches/ucb_bound.txt $(NAME)-ucb.txt
	cp caches/$(NAME)-dcache.png $(NAME)-dcache.png
	cp caches/$(NAME)-dcache.eps $(NAME)-dcache.eps


$(NAME)-icache.png $(NAME)-icache.eps : patches
	cd caches ; gnuplot icache.p
	cd caches ; gnuplot icache-eps.p
	cp caches/$(NAME)-icache.png $(NAME)-icache.png
	cp caches/$(NAME)-icache.eps $(NAME)-icache.eps

patches: caches/icache.p caches/dcache.p
	cd caches ; for i in ../patch/*; do patch -p1 < $$i; done

$(NAME)-dmatrix.txt: caches/dcache.matrix
	cp caches/dcache.matrix $(NAME)-dmatrix.txt

$(NAME)-imatrix.txt: caches/icache.matrix
	cp caches/icache.matrix $(NAME)-imatrix.txt

caches/icache.p caches/dcache.p caches/icache.matrix caches/dcache.matrix: caches
	cd caches ; cache-iset.pl

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
