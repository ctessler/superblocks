.PHONY: all clean

EPS = $(wildcard eps/*.eps)
TEX = $(wildcard *.tex)
BIB = $(wildcard *.bib)

all: crpd_lppp.pdf

clean:
	rm -rf *.dvi *.log *.aux *.pdf
	rm -rf eps/*.pdf

crpd_lppp.pdf: $(TEX) $(EPS) $(BIB)
	pdflatex crpd_lppp.tex
	bibtex crpd_lppp
	pdflatex crpd_lppp
	rm -f *.log *.aux *.dvi

