#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

#include "utils/unicode_convert/table_big5_ucs2.h"
#include "utils/unicode_convert/table_gbk_ucs2.h"
#include "utils/unicode_convert/unicode_convert.h"

#include "utils/debug/art_assert.h"
#include "utils/debug/log.h"

#define NUMOF_TAB_GBK_TO_UCS2     sizeof(table_GBK_to_UCS2)/sizeof(table_GBK_to_UCS2[0])
#define NUMOF_TAB_BIG5_TO_UCS2    sizeof(table_BIG5_to_UCS2)/sizeof(table_BIG5_to_UCS2[0])

/*****************************************************************************
 *Convert 1 GBK encoding character to Unicode (UCS-2 and UCS-4) encoding
 * parameters:
 *    gbk         character in gbk encoding
 *    ucs         point to output buffer, where the unicode result store in,
 *                type unsigned long .
 *
 * return:
 *    1. return the number in bytes the character occupied in gkb encoding,if success.
 *         ascii character occupies 1 bytes, chinese character occupied 2 bytes.
 *    2. return 0 if fail.
 *
 * notice:
 *     1. Both GBK and Unicode request byte-order,we use little endian;
 ****************************************************************************/
static int encode_gbk_to_unicode_one(unsigned short gbk, unsigned short *ucs)
{
    ART_ASSERT(ucs != NULL);

    unsigned char *p = (unsigned char *) &gbk;
    unsigned char *phibyte = p + 1;
    unsigned int i;

    if ( *phibyte < 0x80 ) {
        *ucs = *phibyte;
        return 1;
    } else {
        for(i = 0; i < NUMOF_TAB_GBK_TO_UCS2; i++) {
            if(gbk == table_GBK_to_UCS2[i][0]) {
                *ucs = table_GBK_to_UCS2[i][1];
                return 2;
            }
        }
    }

    return 0;
}

static int encode_unicode_one_to_big5(unsigned short unicode, unsigned char *big5)
{
    ART_ASSERT(unicode != 0);
    unsigned int i;

    for(i = 0; i < NUMOF_TAB_BIG5_TO_UCS2; i++) {
        if(unicode == table_BIG5_to_UCS2[i][1]) {
            *big5 = (table_BIG5_to_UCS2[i][0]&0xff00)>>8;
            *(big5+1) = table_BIG5_to_UCS2[i][0]&0xff;
            return 2;
        }
    }
    return 0;
}

/*****************************************************************************
 * Convert 1 Unicode (UCS-2 and UCS-4) encoding character to utf8 encoding character
 *
 * parameters:
 *    unic        character in unicode encoding
 *    pOutput     point to output buffer, where the utf8 result store in,
 *    outsize     the size of pOutput
 *
 * return:
 *    1. return the number in bytes the character occupied in utf8 encoding,if success.
 *    2. return 0 if fail.
 *
  * notice:
 *     1. UTF8 does not request byte-order, but Unicode yes;
 *        we use little endian here (lower address stored first)
 *     2. pOutput request at least 6 bytes memory
 *
 ****************************************************************************/
static int encode_unicode_to_utf8_one(unsigned long unic, unsigned char *output, int out_size)
{
    ART_ASSERT(output != NULL && out_size >= 6);

    if ( unic <= 0x0000007F ) {
        // * U-00000000 - U-0000007F:  0xxxxxxx
        *output     = (unic & 0x7F);
        return 1;
    } else if ( unic >= 0x00000080 && unic <= 0x000007FF ) {
        // * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
        *(output+1) = (unic & 0x3F) | 0x80;
        *output     = ((unic >> 6) & 0x1F) | 0xC0;
        return 2;
    } else if ( unic >= 0x00000800 && unic <= 0x0000FFFF ) {
        // * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
        *(output+2) = (unic & 0x3F) | 0x80;
        *(output+1) = ((unic >>  6) & 0x3F) | 0x80;
        *output     = ((unic >> 12) & 0x0F) | 0xE0;
        return 3;
    } else if ( unic >= 0x00010000 && unic <= 0x001FFFFF ) {
        // * U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        *(output+3) = (unic & 0x3F) | 0x80;
        *(output+2) = ((unic >>  6) & 0x3F) | 0x80;
        *(output+1) = ((unic >> 12) & 0x3F) | 0x80;
        *output     = ((unic >> 18) & 0x07) | 0xF0;
        return 4;
    } else if ( unic >= 0x00200000 && unic <= 0x03FFFFFF ) {
        // * U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        *(output+4) = (unic & 0x3F) | 0x80;
        *(output+3) = ((unic >>  6) & 0x3F) | 0x80;
        *(output+2) = ((unic >> 12) & 0x3F) | 0x80;
        *(output+1) = ((unic >> 18) & 0x3F) | 0x80;
        *output     = ((unic >> 24) & 0x03) | 0xF8;
        return 5;
    } else if ( unic >= 0x04000000 && unic <= 0x7FFFFFFF ) {
        // * U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        *(output+5) = (unic & 0x3F) | 0x80;
        *(output+4) = ((unic >>  6) & 0x3F) | 0x80;
        *(output+3) = ((unic >> 12) & 0x3F) | 0x80;
        *(output+2) = ((unic >> 18) & 0x3F) | 0x80;
        *(output+1) = ((unic >> 24) & 0x3F) | 0x80;
        *output     = ((unic >> 30) & 0x01) | 0xFC;
        return 6;
    }

    return 0;
}

int enc_get_utf8_size(const unsigned char pInput)
{
   unsigned char c = pInput;
   signed char utf8_size;
   if(c< 0x80)
       utf8_size= 0;
   if(c>=0x80 && c<0xC0)
       utf8_size= -1;
   if(c>=0xC0 && c<0xE0)
       utf8_size= 2;
   if(c>=0xE0 && c<0xF0)
       utf8_size= 3;
   if(c>=0xF0 && c<0xF8)
       utf8_size= 4;
   if(c>=0xF8 && c<0xFC)
       utf8_size= 5;
   if(c>=0xFC)
       utf8_size= 6;
   return utf8_size;
}

/*****************************************************************************
 * TYPE    UNICODE                             UTF8
 * A       0x00-0x7F(0-127)                    0XXXXXXX

 * B       0x80-0x7FF(128-2047)                110XXXXX 10XXXXXX

 * C       0x800-0xFFFF(2048-65535)            1110XXXX 10XXXXXX 10XXXXXX

 * D       0x10000-0x10FFFF(65536 above)       11110XXX 10XXXXXX 10XXXXXX 10XXXXXX

 * "杨" unicode value 0x1A70,range in [0x800,0xFFFF],UTF8 use type c
 * 0x1A70    binary  0001   1010 01  11 0000    replace the X in type C
 *               1110XXXX 10XXXXXX 10XXXXXX
 *     should be 11100001 10101001 10110000
 *     so its utf8 code is 0xE1A9B0

 * convert 1 UTF8 coding character to Unicode(UCS-2 and UCS-4) coding character.
 * UTF8 use 1 byte to encode englisgh character and 3 bytes to encode chinese character
 * parameters:
 *    pInput      input para, UTF-8 encoding string
 *    Unic        output para, the string in Unicode after transform
 *
 * return:
 *    return the bytes number in UTF8 encoding if success,else return 0.
 *
 * notice:
 *     1. UTF8 no byte sequence, but Unicode in Big Endian or Little Endian;
 *        we use little endian here (lower address stored first)
 ****************************************************************************/
int enc_utf8_to_unicode_one(const unsigned char* pInput, unsigned char *Unic)
{
    ART_ASSERT(pInput != NULL && Unic != NULL);
    // b1 represent the highest byte in pInput in UTF-8 encoding,b2 the second highest
     char b1, b2, b3, b4, b5, b6;

    *Unic = 0x0; //
    int utfbytes = enc_get_utf8_size(*pInput);
    unsigned char *pOutput = (unsigned char *) Unic;
    switch ( utfbytes )
    {
        case 0:
            *pOutput     = *pInput;
            utfbytes    += 1;
            break;
        case 2:
            b1 = *pInput;
            b2 = *(pInput + 1);
            if ( (b2 & 0xE0) != 0x80 )
                return 0;
            *pOutput     = (b1 << 6) + (b2 & 0x3F);
            *(pOutput+1) = (b1 >> 2) & 0x07;
            break;
        case 3:
            b1 = *pInput;
            b2 = *(pInput + 1);
            b3 = *(pInput + 2);
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) )
                return 0;
            *pOutput     = (b2 << 6) + (b3 & 0x3F);
            *(pOutput+1) = (b1 << 4) + ((b2 >> 2) & 0x0F);
            break;
        case 4:
            b1 = *pInput;
            b2 = *(pInput + 1);
            b3 = *(pInput + 2);
            b4 = *(pInput + 3);
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)
                    || ((b4 & 0xC0) != 0x80) )
                return 0;
            *pOutput     = (b3 << 6) + (b4 & 0x3F);
            *(pOutput+1) = (b2 << 4) + ((b3 >> 2) & 0x0F);
            *(pOutput+2) = ((b1 << 2) & 0x1C)  + ((b2 >> 4) & 0x03);
            break;
        case 5:
            b1 = *pInput;
            b2 = *(pInput + 1);
            b3 = *(pInput + 2);
            b4 = *(pInput + 3);
            b5 = *(pInput + 4);
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)
                    || ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80) )
                return 0;
            *pOutput     = (b4 << 6) + (b5 & 0x3F);
            *(pOutput+1) = (b3 << 4) + ((b4 >> 2) & 0x0F);
            *(pOutput+2) = (b2 << 2) + ((b3 >> 4) & 0x03);
            *(pOutput+3) = (b1 << 6);
            break;
        case 6:
            b1 = *pInput;
            b2 = *(pInput + 1);
            b3 = *(pInput + 2);
            b4 = *(pInput + 3);
            b5 = *(pInput + 4);
            b6 = *(pInput + 5);
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)
                    || ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80)
                    || ((b6 & 0xC0) != 0x80) )
                return 0;
            *pOutput     = (b5 << 6) + (b6 & 0x3F);
            *(pOutput+1) = (b5 << 4) + ((b6 >> 2) & 0x0F);
            *(pOutput+2) = (b3 << 2) + ((b4 >> 4) & 0x03);
            *(pOutput+3) = ((b1 << 6) & 0x40) + (b2 & 0x3F);
            break;
        default:
            break;
    }
    return utfbytes;
}


int ln_encode_gbk_to_utf8(unsigned char *gbk_str, unsigned char *utf8, int utf8_len_max)
{
    int i, utf8_idx = 0;
    unsigned char chr, chr2;
    unsigned short gbk, unicode;

    ART_ASSERT(gbk_str && utf8 && (utf8_len_max >= 6));
    for (i = 0; gbk_str[i] != '\0'; ) {
        chr = *(gbk_str + i);
        if ((chr & 0x80) != 0 && chr >= 0x81 && chr <= 0xFE) {
            chr2 = *(gbk_str + i + 1);
            if((chr >= 0x40) && (chr <= 0xFE)) {
                gbk = (chr << 8) | chr2;
                encode_gbk_to_unicode_one(gbk, &unicode);
                utf8_idx += encode_unicode_to_utf8_one(unicode, utf8 + utf8_idx, utf8_len_max - utf8_idx);
                i += 2;
            }
        } else {
            *(utf8 + utf8_idx) = chr;
            i++;
            utf8_idx++;
        }
    }
    return utf8_idx;
}

int ln_encode_gbk_to_big5(unsigned char *gbk_str, unsigned char *big5, int big5_len_max)
{
    int i, big5_idx = 0;
    unsigned char chr, chr2;
    unsigned short gbk, unicode;

    ART_ASSERT(gbk_str && big5 && (big5_len_max >= 6));
    for (i = 0; gbk_str[i] != '\0'; ) {
        chr = *(gbk_str + i);
        if ((chr & 0x80) != 0 && chr >= 0x81 && chr <= 0xFE) {
            chr2 = *(gbk_str + i + 1);
            if((chr >= 0x40) && (chr <= 0xFE)) {
                gbk = (chr << 8) | chr2;
                encode_gbk_to_unicode_one(gbk, &unicode);
                big5_idx += encode_unicode_one_to_big5(unicode, big5+big5_idx);
                i += 2;
            }
        } else {
            *(big5 + big5_idx) = chr;
            i++;
            big5_idx++;
        }
    }
    return big5_idx;

}

int ln_encode_utf8_to_big5(unsigned char *utf8_str, unsigned char *big5)
{
    int i, big5_idx = 0;
    unsigned char chr, chr2 ,chr3;
    unsigned short  unicode;
    unsigned int   utf8;

    ART_ASSERT(utf8_str && big5);
    for (i = 0; utf8_str[i] != '\0'; ) {
        chr = *(utf8_str + i);
        if((chr>=0x80)&&(*(utf8_str + i + 1)!='\0')&&(*(utf8_str + i + 2)!='\0')){
            chr2=*(utf8_str + i + 1);
            chr3=*(utf8_str + i + 2);
            if((chr>=0xE0&&chr<=0xEF)&&(chr2>=0x80&&chr2<=0xBF)&&(chr3>=0x80&&chr3<=0xBF)){//it is a chinese character
                utf8 = (chr) | (chr2<<8)|(chr3<<16);//the first para of enc_utf8_to_unicode_one() must be little endian
                enc_utf8_to_unicode_one((unsigned char *)&utf8, (unsigned char *)&unicode);
                big5_idx += encode_unicode_one_to_big5(unicode, big5+big5_idx);
                i += 3;
            }
        } else {//ascii code
            *(big5 + big5_idx) = chr;
            i++;
            big5_idx++;
        }
    }
    return big5_idx;
}

int ln_is_str_utf8(const char* str)
{
    unsigned int i;
    unsigned int nBytes = 0;//Length of UFT8 is 1-6 bytes,ASCII only need 1 byte
    unsigned char chr = *str;
    bool bAllAscii = true;
    for (i = 0; str[i] != '\0'; ++i) {
        chr = *(str + i);
        //first to check whether it is a ascii code,which is encoding as 0xxxxxxx in utf8
        if (nBytes == 0 && (chr & 0x80) != 0) {
            bAllAscii = false;
        }
        if (nBytes == 0) {
            //it is not ascii code
            if (chr >= 0x80) {
                if (chr >= 0xFC && chr <= 0xFD) {
                    nBytes = 6;
                } else if (chr >= 0xF8) {
                    nBytes = 5;
                } else if (chr >= 0xF0) {
                    nBytes = 4;
                } else if (chr >= 0xE0) {
                    nBytes = 3;
                } else if (chr >= 0xC0) {
                    nBytes = 2;
                } else {
                    return false;
                }
                nBytes--;
            }
        } else {
            //10xxxxxx
            if ((chr & 0xC0) != 0x80) {
                return false;
            }
            //����Ϊ��Ϊֹ
            nBytes--;
        }
    }
    //
    if (nBytes != 0) {
        return false;
    }
    if (bAllAscii) { //
        return true;
    }
    return true;
}

/***************************************************************
    BIG5 encode is popular in TaiWan and Hongkong.
    1 chines character is encode to 2 bytes.
    first byte is value in[A1,F9], last byte is value in [40,7E] or [A1,FE]
    (1)if last byte's value in [40,7E], BIG5 encoding is probable.
    (2)if first byte is value in [A4,A9], BIG5 encoding is probable.
    (3)if first byte is value in [AA,AF] and last byte is value in [A1,FE],BIG5 encoding is certain

***************************************************************/
int ln_is_str_big5(const char* str)
{
    ART_ASSERT(str);

    unsigned int i;
    unsigned char chr = *str,chr2;
    bool bAllAscii = true; //check whether they are all ASCII,

    if(ln_is_str_utf8(str)){
        return false;
    }
    for (i = 0; str[i] != '\0'; ++i) {
        chr = *(str + i);
        if ((chr & 0x80) != 0) { // if not asscii code,maybe big5
            bAllAscii = false;
        }
            if (chr >= 0x80) {
                if (chr >= 0xA1 && chr <= 0xFE) {
                    if(*(str + i + 1)!='\0'){
                        chr2=*(str + i + 1);
                        if((chr >= 0xAA && chr <= 0xAF)&&(chr2 >= 0xA1 && chr2 <= 0xFE)){
                            return true;
                        }
                        if(chr2 >= 0x40 && chr2 <= 0x7E){
                            return true;
                        }
                        if(chr >= 0xA4 && chr <= 0xA9){
                            return true;
                        }
                   }
               }
          }
    }
    if (bAllAscii) { //all ASCII, not big5
        return false;
    }
    return false;
}

int ln_is_str_gbk(const char* str)
{
    ART_ASSERT(str);

    unsigned int i;
    unsigned int nBytes = 0;//GBK use 1-2 bytes to encode,2 bytes for chinese character and 1 byte for english
    unsigned char chr = *str;
    bool bAllAscii = true; //to check if the string is all ASCII,

    if(ln_is_str_utf8(str)){
        return false;
    }
    for (i = 0; str[i] != '\0'; ++i) {
        chr = *(str + i);
        if ((chr & 0x80) != 0 && nBytes == 0) { // if not asscii code,gbk is possible
            bAllAscii = false;
        }
        if (nBytes == 0) {
            if (chr >= 0x80) {
                if (chr >= 0x81 && chr <= 0xFE) {
                    nBytes = +2;
                } else {
                    return false;
                }
                nBytes--;
            }
        } else {
            if (chr < 0x40 || chr>0xFE) {
                return false;
            }
            nBytes--;
        }
    }
    if (nBytes != 0) {   //break the rule
        return false;
    }

    if (bAllAscii) { //if all character in string is ASCII, GBK is impossible.
        return false;
    }

    return true;
}

void encode_test(void)
{
    unsigned char str1[4]={0xE6,0xB0,0xB4,'\0'},i;
    unsigned char str2[4]={0xE6,0x98,0x9F,'\0'};
    unsigned char utf8[]={0xE6,0xB0,0xB4,0xE6,0x98,0x9F,0x5F,0x44,0x31,0x32,0x38,'\0'};
    unsigned char big5_encode[32]={0};
    unsigned short big5_1,big5_2;
    ln_encode_utf8_to_big5(str1,(unsigned char*)&big5_1);
    ln_encode_utf8_to_big5(str2,(unsigned char*)&big5_2);
    ln_encode_utf8_to_big5(utf8,big5_encode);
    LOG(LOG_LVL_DEBUG,"utf8[0xE6B0B4]=>[0x%x]big5,utf8[0xE6989F]=>[0x%x]big5", big5_1,big5_2);
    for (i=0;big5_encode[i]!='\0';i++)
        LOG(LOG_LVL_DEBUG, " 0x%x ", big5_encode[i]);
#if 0
    unsigned int i,big5_cnt=0,gbk_cnt=0,neither_of_2=0;
    unsigned char big5[3],gbk[3];
    for(i = 0; i < NUMOF_TAB_BIG5_TO_UCS2; i++) {
        big5[0] = (table_BIG5_to_UCS2[i][0]&0xff00)>>8;
        big5[1] = (table_BIG5_to_UCS2[i][0])&0xff;
        big5[2]='\0';
        if(ln_is_str_big5(big5))
            big5_cnt++;
        else if(ln_is_str_gbk(big5))
            gbk_cnt++;
        else
            neither_of_2++;
    }

    LOG(LOG_LVL_DEBUG, "big5 judge test: sum:%d,big5_cnt:%d,gbk_cnt:%d,neither_of_2=%d", NUMOF_TAB_BIG5_TO_UCS2,big5_cnt,gbk_cnt,neither_of_2);
                     // big5 judge test: sum:14558,big5_cnt:6987,gbk_cnt:6610,neither_of_2=961
    big5_cnt=0;
    gbk_cnt=0;
    neither_of_2=0;
    for(i = 0; i < NUMOF_TAB_GBK_TO_UCS2; i++) {
        gbk[0] = (table_GBK_to_UCS2[i][0]&0xff00)>>8;
        gbk[1] = table_GBK_to_UCS2[i][0]&0xff;
        gbk[2] ='\0';
        if(ln_is_str_big5(gbk))
            big5_cnt++;
        else if(ln_is_str_gbk(gbk))
            gbk_cnt++;
        else
            neither_of_2++;
    }
    LOG(LOG_LVL_DEBUG, "gbk judge test: sum:%d,big5_cnt:%d,gbk_cnt:%d,neither_of_2:%d", NUMOF_TAB_GBK_TO_UCS2,big5_cnt,gbk_cnt,neither_of_2);
                      //gbk judge test: sum:3493,big5_cnt:0,gbk_cnt:2811,neither_of_2:682
    big5_cnt=0;
    gbk_cnt=0;
    neither_of_2=0;
    for(i = 0; i < NUMOF_TAB_GBK_TO_UCS2; i++) {
        gbk[0] = (table_GBK_to_UCS2[i][0]&0xff00)>>8;
        gbk[1] = table_GBK_to_UCS2[i][0]&0xff;
        gbk[2] ='\0';
        if(ln_is_str_gbk(gbk))
            gbk_cnt++;
        else if(ln_is_str_big5(gbk))
            big5_cnt++;
        else
            neither_of_2++;
    }
    LOG(LOG_LVL_DEBUG, "gbk judge test222: sum:%d,big5_cnt:%d,gbk_cnt:%d,neither_of_2:%d", NUMOF_TAB_GBK_TO_UCS2,big5_cnt,gbk_cnt,neither_of_2);
                      //gbk judge test222: sum:3493,big5_cnt:0,gbk_cnt:2811,neither_of_2:682
#endif
}

