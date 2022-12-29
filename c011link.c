/*
 *	c011link.c
 *
 *	macihenroomfiddling@gmail.com
 */

#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "linkio.h"
#include "inmos.h"
#include "c011.h"


LINK
OpenLink(char *Name)
{
    static bool open = FALSE;
    if (!open) {
    	c011_init();
    	open = TRUE;
    }
    return 1;
}

int 
CloseLink(LINK LinkId)
{
    return SUCCEEDED;
}

int
ReadLink(LINK LinkId, unsigned char *Buffer, unsigned int Count, int Timeout)
{
	int result;

	/* Read into buffer */
	result = c011_read_bytes( Buffer, Count, Timeout );
	if (result >= 0) {
	    return result;
	} else {
	    return ER_LINK_CANT;
	}
}

int
WriteLink(LINK LinkId, unsigned char *Buffer, unsigned int Count, int Timeout)
{
	int result;
	result = c011_write_bytes (Buffer, Count, Timeout);
	if (result >= 0) {
	    return result;
	} else {
	    return ER_LINK_CANT;
	}
}

int
ResetLink(LINK LinkId)
{
    c011_reset();
    //The whitecross HSL takes some time to cascade reset
    sleep(1);
    //Set Whitecross HSL to byte mode
    c011_set_byte_mode();
    return SUCCEEDED;
}


int
AnalyseLink(LINK LinkId)
{
    c011_analyse ();
    return SUCCEEDED;
}


int
TestError(LINK LinkId)
{
    return ER_LINK_CANT;
}

int
TestRead(LINK LinkId)
{
    return ((c011_read_input_status() & 0x01) == 0x01);
}

int
TestWrite(LINK LinkId)
{
    return ((c011_read_output_status() & 0x01) == 0x01);
}



