/*

Basic UTF-8 decoding program. 
1) check for the length of sequence
2) try to decode it
   after checking the signature 
      011... 10...
3) print the point in deimal format

one integer (32 bits) in good anough to store the code point

*/

#include <stdio.h>
#include <string.h>

/* convert the hexadcinal into decimal format */

int decimal(char c)
{
    int retVal = -1;
    if (c >= '0' && c <= '9') {
	retVal = (c - '0');
    } else if (c >= 'A' && c <= 'F') {
	retVal = (c - 'A' + 10);
    }
    return (retVal);
}

/* need to convert the ocla char to integer to get correct information */
int OctalToInt(int *val, char *octal)
{
    int retVal = -1;
    *val = 0;
    int upperBit = decimal(octal[2]);
    int lowerBit = decimal(octal[3]);
    if (lowerBit != -1 && upperBit != -1) {
	*val = (upperBit << 4 | lowerBit);
	retVal = 0;
    }
    return retVal;
}

/* decode the sequence based on the length precomputed */
/* lenght could have been pre computed, but was seperated from sequence decoding logic */

int decodeSequence(char **bytes, int len, int *c)
{
    int val = 0;
    int retVal = 0;
    int byte1;
    OctalToInt(&byte1, bytes[0]);
    /* mask the length bits */
    switch (len) {
    case 2:
	byte1 = byte1 & 31;
	break;			/* 110x xxxx */
    case 3:
	byte1 = byte1 & 15;
	break;			/* 1110 xxxx */
    case 4:
	byte1 = byte1 & 7;
	break;			/* 1111 0xxx */
    case 5:
	byte1 = byte1 & 3;
	break;			/* 1111 10xx */
    case 6:
	byte1 = byte1 & 1;
	break;			/* 1111 110x */
    default:
	;			/* no masking */
    }
    val = val | byte1;		/* first byte */
    /* sequence bytes should start with 10 */
    for (int num = 1; num <= len - 1; num++) {
	OctalToInt(&byte1, bytes[num]);
	if ((byte1 & 0xC0) != 0x80) {
	    retVal = -1;
	    return retVal;
	}
	/* left shift to accomodate the new bits */
	/* fill the information */
	val = (val << 6) | (byte1 & 0x3f);
    }
    /* some basic error checking from limited knowledge of UTF-8 code */
    if ((val == 0xFFFE) || (val == 0xFFFF) || (val > 0x10FFFF)) {
	retVal = -1;
    }
    *c = val;
    return retVal;
}

/* length of UTF codes can be 1-6 bytes */
int lenghtOfSequence(char *firstByte)
{
    int retVal = -1;
    int byte;
    OctalToInt(&byte, firstByte);
    if (byte <= 0x7F) {		/* 0XXX XXXX one byte */
	retVal = 1;
    } else if ((byte & 0xE0) == 0xC0) {	/* 110X XXXX  two bytes */
	retVal = 2;
    } else if ((byte & 0xF0) == 0xE0) {	/* 1110 XXXX  three bytes */
	retVal = 3;
    } else if ((byte & 0xF8) == 0xF0) {	/* 1111 0XXX  four bytes */
	retVal = 4;
    } else if ((byte & 0xFC) == 0xF8) {	/* 1111 10XX  five bytes */
	retVal = 5;
    } else if ((byte & 0xFE) == 0xFC) {	/* 1111 110X  six bytes */
	retVal = 6;
    } else {
	/* not a valid first byte of UTF-8 sequence */
	retVal = -1;
    }
    return retVal;
}

/* main program to iterate on the utf-nodes  provided*/
/* skips a single node, if there is a error in decoding the node */

int main(int argc, char *argv[])
{
    /*UTF-8 code point can use 1 bytes to 6 bytes, so we need a int */
    int codePoint = 0;
    int len = 0;
    if (argc < 2) {
	printf("error\n");
    }
    for (int i = 1; i < argc;) {
	len = lenghtOfSequence(argv[i]);
	if (len == -1) {
	    printf("Error in UTF code, skipping node %s\n", argv[i]);
	    i++;
	} else if (-1 == decodeSequence(&argv[i], len, &codePoint)) {
	    printf("Error in UTF code, skipping node %s\n", argv[i]);
	    /* skip single node */
	    i++;
	} else {
	    printf("CodePoint = %d\n", codePoint);
	    /* skip the number of nodex which are decoded */
	    i += len;
	}
    }
    return 0;
}
