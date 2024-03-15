#
#   ISPY / MTEST makefile
#

COPTS           = -Wall
OPTIM           = -O2
CC              = gcc -c $(OPTIM) $(COPTS)
HEADERS			= cklib.h inmos.h boot.h linkio.h
LINKOBJS        = c011link.o c011.o gpiolib.c gpiochip_rp1.c util.c
LINKHEADERS     = c011.h pins.h gpiochip.h gpiochip_rp1.h
LINK            = gcc $(OPTIM)
LIBRARIES       = -lbcm2835

all:     ispy mtest

check.o: type16.h type32.h check16.h check32.h
mtest.o: mtest16.h mtest32.h

clean:
		rm -f *.o ispy mtest check??.h type??.h

ispy:      check.o cklib.o $(LINKOBJS) $(LINKHEADERS) $(HEADERS)
		$(LINK) -o ispy check.o cklib.o $(LINKOBJS) $(LIBRARIES)

mtest:      mtest.o cklib.o $(LINKOBJS) $(LINKHEADERS) $(HEADERS)
		$(LINK) -o mtest mtest.o cklib.o $(LINKOBJS) $(LIBRARIES)

%.o : %.c
		$(CC) -o $@ $<

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

$(filter %.h,$(32bit)): %.h : %.occ checklib.occ
	$(DB) -c "$(OCCAM) /TA /o $(basename $<).tco $< > oc.txt" -c "ilist /c $(basename $<).tco /o $(basename $<).tcl > ilist.txt" -c "exit"
	cat OC.TXT
	cat ILIST.TXT
	./tco2h.py $(shell echo $(basename $<) | tr '[:lower:]' '[:upper:]').TCL > $@
	$(FIND) "code size" $@

$(filter %.h,$(16bit)): %.h : %.occ checklib.occ
	$(DB) -c "$(OCCAM) /T2 /o $(basename $<).tco $< > oc.txt" -c "ilist /c $(basename $<).tco /o $(basename $<).tcl > ilist.txt" -c "exit"
	cat OC.TXT
	cat ILIST.TXT
	./tco2h.py $(shell echo $(basename $<) | tr '[:lower:]' '[:upper:]').TCL > $@
	$(FIND) "code size" $@

