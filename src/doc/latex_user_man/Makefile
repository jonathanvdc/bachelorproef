#-------------
# MACROS
#-------------

CLEANDIR = \
	echo cleaning up `pwd`; \
	rm -f core *~ *.bak *.backup *.I .*.d~; \
	rm -f *.log *.aux *.bbl *.blg *.dvi *.lof *.lot *.lol *.toc *.idx *.ilg *.ind; \
	rm -f *-eps-converted-to.pdf *.synctex.gz *.tps *.out

DISTCLEANDIR = \
	echo distcleaning `pwd`; $(CLEANDIR) && rm -f UserManual.pdf

PDFLATEX = pdflatex -interaction=batchmode --enable-write18
BIBTEX   = bibtex -terse

#-------------
# Targets
#-------------
.PHONY: default all pdf clean distclean

default: all

clean:
	@ $(CLEANDIR)

distclean:
	@ $(DISTCLEANDIR)

all:
	$(PDFLATEX) UserManual.tex
	$(BIBTEX) UserManual
	$(PDFLATEX) UserManual.tex
	$(PDFLATEX) UserManual.tex

#############################################################################
