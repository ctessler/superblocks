.PHONY: all clean graphs

EPS = $(wildcard eps/*.eps)
TEX = $(wildcard *.tex)
BIB = $(wildcard *.bib)

all: crpd_lppp.pdf crpd_lppp_appendix.pdf

graphs:
	cd eps ; make

clean:
	rm -rf *.dvi *.log *.aux *.pdf
	rm -rf eps/*.pdf
	cd eps ; make clean

crpd_lppp.pdf: $(TEX) $(EPS) $(BIB) graphs
	pdflatex crpd_lppp.tex
	bibtex crpd_lppp
	pdflatex crpd_lppp
	rm -f *.log *.aux *.dvi

crpd_lppp_appendix.pdf: appendix.tex  crpd_lppp_appendix.tex graphs
	pdflatex crpd_lppp_appendix.tex
	pdflatex crpd_lppp_appendix.tex
	rm -f *.log *.aux *.dvi
