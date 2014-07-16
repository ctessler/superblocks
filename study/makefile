.PHONY: all clean

EPS = $(wildcard eps/*.eps)

all: case_study.pdf

clean:
	rm -rf *.log *.aux case_study.pdf

case_study.pdf: case_study.tex $(EPS)
	pdflatex case_study.tex
	rm *.log *.aux

