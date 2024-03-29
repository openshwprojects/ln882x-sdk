#ifndef __UNICODE_CONVERT_H__
#define __UNICODE_CONVERT_H__


int ln_is_str_utf8(const char* str);
int ln_is_str_big5(const char* str);
int ln_is_str_gbk(const char* str);
int ln_encode_gbk_to_utf8(unsigned char *gbk_str, unsigned char *utf8, int utf8_len_max);
int ln_encode_gbk_to_big5(unsigned char *gbk_str, unsigned char *big5, int big5_len_max);
int ln_encode_utf8_to_big5(unsigned char *utf8_str, unsigned char *big5);


#endif /* __UNICODE_CONVERT_H__ */
