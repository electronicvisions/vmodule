/***************************************************************************

changed: do not write binary to file, instead, return buffer

usage: int readhex(char *mcsfilename,unsigned char * binbuf,size_t bufsize);
if return value ==0 binbuf contains binary verion of hex file

 * 386hex2bin is used for format HEX386 to BIN(binary) written by Wei Wei in
 * ShangHai. Please email comments, suggestions, bugs to cortex-a8@163.com .

 * Wei Wei is a firm software engineer on 8051, MIPS, ARM7 and ARM9. Now he is
 * applying himself to researching cortex-a8 which is a branch of ARM. He is
 * an expert on U-boot and Linux 2.6.

 * The Intel HEX file is an ASCII text file with lines of text that
 * follow the Intel HEX file format. Each line in an Intel HEX file
 * contains one HEX record. These records are made up of hexadecimal
 * numbers that represent machine language code and/or constant data.
 * Intel HEX files are often used to transfer the program and data that
 * would be stored in a ROM or EPROM. Most EPROM programmers or emulators
 * can use Intel HEX files.
 *
 * Extended Linear Address Records (HEX386)
 *
 * Extended linear address records are also known as 32-bit address
 * records and HEX386 records. These records contain the upper 16 bits
 * (bits 16-31) of the data address. The extended linear address record
 * always has two data bytes and appears as follows:
 *  :02000004FFFFFC
 *  where:
 * 02 is the number of data bytes in the record.
 * 0000 is the address field. For the extended linear address record,
 *  this field is always 0000.
 * 04 is the record type 04 (an extended linear address record).
 * FFFF is the upper 16 bits of the address.
 * FC is the checksum of the record and is calculated as
 *      01h + NOT(02h + 00h + 00h + 04h + FFh + FFh).

 * More detail about HEX386 see http://www.keil.com/support/docs/1584.htm

 * The function of this code is
 * This code is free. You can use it for commercial purposes, and you can aslo
 * improve it in any way.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "readhex.h"

FILE	*hex;
FILE 	*bin;
unsigned char	checksum=0;

/* transfer char data to data. like '1'-> 1, 'a'-> 0xa */
unsigned char char2date(char ascii)
{
    if('0'<=ascii && '9'>=ascii )
        return(ascii - '0');
    else if('A'<=ascii && 'F'>=ascii)
        return((ascii -'A') + 10);
    else if('a'<=ascii && 'f'>=ascii)
        return((ascii -'a') + 10);
    else
        return 0;
}

/* get a char from HEX file, and transfer it to date */
char gethex(void)
{
    unsigned char ch1, ch2;
    ch1 = fgetc(hex);
    ch2 = fgetc(hex);
    ch1 = char2date(ch1);
    ch2 = char2date(ch2);
    checksum = checksum + (ch1<<4) +ch2;
    return ((ch1<<4) +ch2);
}

/* get two char from HEX file, and transfer it to 16-bit data */
unsigned int getaddr(void)
{
    unsigned char ch1, ch2;
    ch1 = gethex();
    ch2 = gethex();
    return((ch1<<8) + ch2);
}

int readhex(const char * mcsfile,unsigned char *buf,size_t bufsize)
{
    unsigned int HEXSIZE=bufsize;
    const char *HEXname=mcsfile;
    unsigned char	*binbuffer=buf;

    int error=0;
    unsigned int	addr =0;
    unsigned int	sign =0;
    unsigned int	page =0;
    unsigned char	ch, *p;
    int	count;
    int 	i;


    unsigned char hexbuffer[16];

    if( NULL == (hex=fopen(HEXname,"r+")) )
    {
        return -3; //can not open file
    }

    while(!feof(hex))
    {
        if( ':' == (ch = fgetc(hex)) )
        {
            /* read a fresh hex line date, and store it in struct binbuffer[] */
            count = gethex();
            addr = getaddr();
            sign = gethex();

            for( i=0; i<count; i++)
            {
                hexbuffer[i] = gethex();
            }
            gethex();
            if( 0 != checksum%256 )
            {
                error=-1; //checksum error
                break;
            }/* read hex line terminate */

            /* hex386 privately owned, operate the high 16bit address */
            if( 2==count && 4==sign )
            {
                page = (hexbuffer[0]*256 + hexbuffer[1]) << 16;
//				printf("page: %x\n",page);
            }
            /* normal operation */
            else if( 0!=count && 0==sign )
            {
                p = binbuffer + addr + page;
                for(i=0; i<count; i++)
                {
                    if(p>binbuffer+HEXSIZE)
                    {
                        error=-6;    //buffer too small
                        break;
                    }
                    *p = hexbuffer[i];
                    p++;
                }
            }
            else if(count==0 && sign==1) //inserted explicit test for end condition to avoid any change in p
            {
                error=0;
                break;
            }
            else
            {
                error=-2; // unknown check code
                break;
            }

        }
    }
    if(fclose(hex)) return -4 ; //can not close hex file
    if(error<0)return error;
    else return (unsigned int)(p-binbuffer); //valid data in binbuffer

}

