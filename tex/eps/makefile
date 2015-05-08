EPS = $(wildcard *.eps)
PDFS = $(patsubst %.eps,%.pdf,$(EPS))

all: $(PDFS)

clean:
	rm -f *.pdf

%.pdf: %.eps
	epstopdf --embed $^
