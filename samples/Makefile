SHELL=/bin/sh

CC = gcc
CFLAGS = -O2 -g -Wall -Wp,-D_FORTIFY_SOURCE=2 --param=ssp-buffer-size=1 -fstack-protector-all -march=nocona
INSTALL = /usr/bin/install -c
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_PROGRAM = ${INSTALL}
LDFLAGS = 
LIBS = -lssl -lcrypto -lbz2 -lpthread -lm 
PERL = @PERL@
MKDIR = ../mkinstalldirs

RM = rm -f

prefix = /local/squid
exec_prefix = ${prefix}
bindir = $(exec_prefix)/bin
infodir = $(prefix)/info

all::
	@echo making $@ in `basename \`pwd\``

update::
	@echo making $@ in `basename \`pwd\``

clean::
	@echo making $@ in `basename \`pwd\``
	$(RM) *~ *.bak core *.log *.error

realclean:: clean
	@echo making $@ in `basename \`pwd\``

distclean:: realclean
	@echo making $@ in `basename \`pwd\``

apidistclean:
	@echo making $@ in `basename \`pwd\``

