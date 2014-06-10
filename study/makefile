.PHONY: all clean

all: case_study.pdf

clean:
	rm -rf *.log *.aux case_study.pdf


case_study.pdf: case_study.tex 
	pdflatex case_study.tex
	rm *.log *.aux

