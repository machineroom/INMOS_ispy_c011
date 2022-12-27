
#
#   ISPY / MTEST / FTEST / LOAD / CKMON / BOOTPATH Makefile
#

E               =
O               = .o
MODEL           =
COPTS           = $(MODEL) -Wall
OPTIM           = -O2
CC              = gcc -c $(OPTIM) $(COPTS)
CCLM            = $(CC)
COPTSLM         = $(COPTS)
DEFAULTLINK	= \"/dev/link0\"
LINKOBJS        = c011link.o c011.o
LINK            = gcc $(OPTIM)
LIBRARIES       = -lbcm2835
RM              = rm
OCCAM		= oc -y -a -n -k -v -e -w -h -T
FIND            = grep

all:     ispy$(E) mtest$(E)

TESTSRCS=test2.occ test414.occ test425.occ test80x.occ test800.occ test801.occ
TESTHSRCS=test2.h test414.h test425.h test32.h test80x.h test800.h test801.h
CSRCS= *.c checklib.h cklib.h
OCCSRCS=check*.occ mtest*.occ

archive:
	zip check.zip $(OCCSRCS) $(CSRCS)

# this makefile in 'included' by another makefile

clean:
		rm -f *.o ispy mtest CHECK??.TCO check??.h TYPE??.TCO type??.h

#
#  ispy
#

ispy$(E):      check$(O) cklib$(O) $(LINKOBJS) 
		$(LINK) -o ispy$(E) check$(O) cklib$(O) $(LINKOBJS) $(LIBRARIES)

check$(O):      check.c checklib.h inmos.h cklib.h \
		type32.h type16.h check32.h check16.h iserver.h
		$(CC) -DDEFAULTLINK=$(DEFAULTLINK) -o check$(O) check.c


#
#  mtest
#

mtest$(E):      mtest$(O) cklib$(O) $(LINKOBJS)
		$(LINK) -o mtest$(E) mtest$(O) cklib$(O) $(LINKOBJS) $(LIBRARIES)

mtest$(O):      mtest.c cklib.h inmos.h  \
		mtest32.h mtest16.h checklib.h
		$(CC) -o mtest$(O) mtest.c


#
#  ckmon
#

ckmon$(E):      ckmon$(O) screen$(O) cklib$(O) $(LINKOBJS)
		$(LINK) -o ckmon$(E) ckmon$(O) screen$(O) cklib$(O) $(LINKOBJS)

ckmon$(O):      ckmon.c link.h screen.h inmos.h opcodes.h \
		cklib.h boot.h peek.h pp32.h pp16.h
		$(CC) -o ckmon$(O) ckmon.c

screen$(O):     screen.c screen.h
		$(CC) screen$(O) screen.c

#
#  support routines
#

cklib$(O):      cklib.c cklib.h checklib.h
		$(CC) -o cklib$(O) cklib.c

hostend$(O):    hostend.c inmos.h \
		iserver.h pack.h
		$(CC) -o hostend$(O) hostend.c
		
#
#  link code
#

c011link.o:
	$(CCLM) $(COPTSLM) c011link.c -o c011link.o

c011.o:			
	$(CCLM) $(COPTSLM) c011.c -o c011.o

#
#  end of C makefile
#
#

D7305A=$(HOME)/d7305a/install/D7305A

DB=dosbox\
	-c "mount D $(shell pwd)"\
	-c "mount E $(D7305A)"\
	-c "PATH=Z:;e:\tools"\
	-c "SET ISEARCH=e:\libs\\"\
	-c "D:"

OCCAM		= oc /y /a /n /k /v /e /w /h
FIND            = grep

16bit = type16.h check16.h mtest16.h
32bit = type32.h check32.h mtest32.h

$(filter %.h,$(32bit)): %.h : %.occ
	$(DB) -c "$(OCCAM) /TA /o $(basename $^).tco $^ > oc.txt" -c "ilist /c $(basename $^).tco /o $(basename $^).tcl > ilist.txt" -c "exit"
	cat OC.TXT
	cat ILIST.TXT
	./tco2h.py $(shell echo $(basename $^) | tr '[:lower:]' '[:upper:]').TCL > $@
	$(FIND) "code size" $@

$(filter %.h,$(16bit)): %.h : %.occ
	$(DB) -c "$(OCCAM) /T2 /o $(basename $^).tco $^ > oc.txt" -c "ilist /c $(basename $^).tco /o $(basename $^).tcl > ilist.txt" -c "exit"
	cat OC.TXT
	cat ILIST.TXT
	./tco2h.py $(shell echo $(basename $^) | tr '[:lower:]' '[:upper:]').TCL > $@
	$(FIND) "code size" $@

