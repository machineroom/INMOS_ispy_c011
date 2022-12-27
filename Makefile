
#
#   ISPY / MTEST / FTEST / LOAD / CKMON / BOOTPATH Makefile
#

O               = .o
COPTS           = -Wall
OPTIM           = -O2
CC              = gcc -c $(OPTIM) $(COPTS)
DEFAULTLINK	= \"/dev/link0\"
LINKOBJS        = c011link.o c011.o
LINK            = gcc $(OPTIM)
LIBRARIES       = -lbcm2835

all:     ispy mtest
check.c: type16.h type32.h check16.h check32.h
mtest.c: mtest16.h mtest32.h

clean:
		rm -f *.o ispy mtest CHECK??.TCO check??.h TYPE??.TCO type??.h

ispy:      check.o cklib.o $(LINKOBJS) 
		$(LINK) -o ispy check.o cklib.o $(LINKOBJS) $(LIBRARIES)

mtest:      mtest.o cklib.o $(LINKOBJS) mtest16.h mtest32.h
		$(LINK) -o mtest mtest.o cklib.o $(LINKOBJS) $(LIBRARIES)

%.o : %.c
		$(CC) -o $@ $^

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

