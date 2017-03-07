#############################################################################
#  This file is part of the indismo software. 
#  It is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by 
#  the Free Software Foundation, either version 3 of the License, or any 
#  later version.
#  The software is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  You should have received a copy of the GNU General Public License,
#  along with the software. If not, see <http://www.gnu.org/licenses/>.
#  see http://www.gnu.org/licenses/.
#
#  Copyright 2015, L.Willem, S.Stijven & J.Broeckhove, University of Antwerp.
#############################################################################

####################################################################
#   $Id$
####################################################################

####################################################################
# MACROS
####################################################################

DIRS = $(shell find . -mindepth 1 -maxdepth 1 -type d -regex '\.\/[^\.].*')

CLEANDIR = \
	echo cleaning up `pwd`; \
	rm -f core *~ *.bak *.backup *.I .*.d~; \
	rm -f *.log *.aux *.bbl *.blg *.dvi *.lof *.lot *.lol *.toc *.idx *.ilg *.ind; \
	rm -f *-eps-converted-to.pdf *.synctex.gz *.tps *.out

DISTCLEANDIR = \
	$(CLEANDIR) && rm -f $(PDFS); echo distcleaning `pwd`

SOURCES  = $(shell echo UserManual.tex)
PDFS     = $(SOURCES:.tex=.pdf)
PDFLATEX = pdflatex -interaction=batchmode --enable-write18
BIBTEX   = bibtex -terse

####################################################################
# Targets
####################################################################

.PHONY: default all pdf 				\
	recurse.clean local.clean clean			\
	recurse.distclean local.distclean distclean

default: all
	
#-------
recurse.clean:
	@ $(foreach dir, $(DIRS), \
	    if test -f $(dir)/Makefile; then $(MAKE) -C $(dir) clean; fi;)

local.clean:
	@ $(CLEANDIR)

clean: recurse.clean local.clean

#-------
recurse.distclean:
	@ $(foreach dir, $(DIRS), \
	   if test -f $(dir)/Makefile; then $(MAKE) -C $(dir) distclean; fi;)
	
local.distclean:
	@ $(DISTCLEANDIR)
	
distclean: recurse.distclean local.distclean

#-------

all: pdf

pdf: $(PDFS)

####################################################################

$(PDFS): %.pdf : %.tex
	$(PDFLATEX) $<
	$(BIBTEX) $*
	$(PDFLATEX) $<
	$(PDFLATEX) $<

####################################################################
	
#############################################################################
