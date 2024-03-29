/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include <stdlib.h>

#include "os.h"
#include "passwd.h"
#include "awss_utils.h"
#include "awss_log.h"
#include "awss_packet.h"

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

uint8_t g_aes_random[RANDOM_MAX_LEN] = {0};
uint8_t g_token_type = TOKEN_TYPE_INVALID;

#ifndef CONFIG_BLDTIME_MUTE_DBGLOG
void _dump_hex(uint8_t *data, int len, int tab_num)
{
    int i;
    for (i = 0; i < len; i++) {
        HAL_Printf("%02x ", data[i]);

        if (!((i + 1) % tab_num)) {
            HAL_Printf("\r\n");
        }
    }

    HAL_Printf("\r\n");
}
#endif

int awss_set_token(uint8_t token[RANDOM_MAX_LEN], bind_token_type_t token_type)
{
    char rand_str[RANDOM_MAX_LEN * 2 + 1] = {0};
    if ((token == NULL) || (token_type >= TOKEN_TYPE_MAX)) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    memcpy(g_aes_random, token, RANDOM_MAX_LEN);
    g_token_type = token_type;
    utils_hex_to_str(g_aes_random, RANDOM_MAX_LEN, rand_str, sizeof(rand_str));
    dump_dev_bind_status(STATE_BIND_SET_APP_TOKEN, "bind: app token set (%d):%s", g_token_type, rand_str);
    return STATE_SUCCESS;
}

int awss_get_token(uint8_t token_buf[], int token_buf_len, bind_token_type_t *p_token_type)
{
    int i = 0;
    char token_str[RANDOM_STR_MAX_LEN] = {0};

    if (!token_buf || token_buf_len < RANDOM_STR_MAX_LEN) 
    {
        return STATE_USER_INPUT_INVALID;
    }

    for (i = 0; i < sizeof(g_aes_random); i ++)  // check g_aes_random is initialed or not
    {
        if (g_aes_random[i] != 0x00) {
            break;
        }
    }

    if (i >= sizeof(g_aes_random)) { // g_aes_random needs to be initialed
        produce_random(g_aes_random, sizeof(g_aes_random));
        awss_debug("produce random:");
        #ifndef CONFIG_BLDTIME_MUTE_DBGLOG
        _dump_hex((uint8_t *)g_aes_random, RANDOM_MAX_LEN, 24);
        #endif
    }

    utils_hex_to_str(g_aes_random, RANDOM_MAX_LEN, token_str, sizeof(token_str));
    memcpy(token_buf, token_str, RANDOM_STR_MAX_LEN);
    *p_token_type = g_token_type;

    return STATE_SUCCESS;
}
#ifdef WIFI_PROVISION_ENABLED
/*
 * 1. place 0 @ 0, because of java modified-UTF8
 * 2. translation follow utf8 stardard
 */
static const uint8_t ssid_dict_decode_table[] = {
    0x00, 0x0e, 0x6c, 0x3a, 0x6d, 0x44, 0x2a, 0x6f,
    0x4d, 0x05, 0x6b, 0x28, 0x08, 0x25, 0x5f, 0x2d,
    0x64, 0x76, 0x78, 0x37, 0x58, 0x60, 0x53, 0x31,
    0x36, 0x79, 0x43, 0x1a, 0x11, 0x72, 0x03, 0x59,
    0x50, 0x02, 0x71, 0x7c, 0x34, 0x3e, 0x23, 0x24,
    0x26, 0x5b, 0x73, 0x0f, 0x5e, 0x12, 0x54, 0x0b,
    0x61, 0x35, 0x3c, 0x57, 0x48, 0x55, 0x63, 0x4a,
    0x13, 0x75, 0x45, 0x70, 0x47, 0x0c, 0x2f, 0x21,
    0x17, 0x2e, 0x62, 0x49, 0x4b, 0x5c, 0x19, 0x51,
    0x69, 0x3b, 0x7e, 0x0d, 0x3d, 0x67, 0x2c, 0x22,
    0x14, 0x42, 0x5a, 0x7f, 0x32, 0x01, 0x07, 0x7b,
    0x15, 0x4f, 0x16, 0x29, 0x30, 0x27, 0x20, 0x18,
    0x65, 0x06, 0x1c, 0x3f, 0x68, 0x2b, 0x4c, 0x0a,
    0x1e, 0x46, 0x5d, 0x1f, 0x10, 0x6e, 0x56, 0x7a,
    0x1b, 0x09, 0x52, 0x38, 0x66, 0x7d, 0x41, 0x40,
    0x04, 0x6a, 0x39, 0x77, 0x33, 0x1d, 0x74, 0x4e,
    0xaf, 0xa6, 0x8c, 0xbd, 0x89, 0xa2, 0xa9, 0x9e,
    0xa1, 0x91, 0xb9, 0xad, 0xbf, 0xb7, 0x95, 0xa8,
    0xa5, 0x82, 0xaa, 0xa3, 0x94, 0x92, 0xb8, 0x87,
    0x88, 0xb1, 0x93, 0xbc, 0x80, 0xb5, 0xba, 0x99,
    0xab, 0xbe, 0x90, 0x8e, 0x83, 0x9f, 0x9a, 0x86,
    0x85, 0x98, 0xa4, 0xa0, 0xac, 0x9c, 0x96, 0x81,
    0xb0, 0x8d, 0xbb, 0xb2, 0x9d, 0xae, 0x84, 0x9b,
    0xb4, 0x8b, 0x97, 0xa7, 0xb3, 0x8a, 0x8f, 0xb6,
    0xc5, 0xc0, 0xc8, 0xd7, 0xde, 0xc4, 0xd1, 0xd2,
    0xd9, 0xcb, 0xcd, 0xd5, 0xcc, 0xc7, 0xdb, 0xdf,
    0xdc, 0xdd, 0xcf, 0xc6, 0xda, 0xc2, 0xc3, 0xc9,
    0xc1, 0xca, 0xd6, 0xd8, 0xce, 0xd3, 0xd0, 0xd4,
    0xe9, 0xe5, 0xe8, 0xe2, 0xe6, 0xeb, 0xe3, 0xec,
    0xed, 0xe7, 0xe1, 0xe4, 0xea, 0xef, 0xee, 0xe0,
    0xf6, 0xf0, 0xf4, 0xf5, 0xf2, 0xf3, 0xf7, 0xf1,
    0xfb, 0xf9, 0xfa, 0xf8, 0xfc, 0xfd, 0xfe, 0xff
};

static const uint8_t notify_encode_table[] = {
    0x00, 0x71, 0x21, 0x1e, 0x78, 0x09, 0x61, 0x56,
    0x0c, 0x55, 0x67, 0x2f, 0x3d, 0x4b, 0x01, 0x2b,
    0x6c, 0x1c, 0x1b, 0x38, 0x50, 0x58, 0x5a, 0x40,
    0x5f, 0x46, 0x2d, 0x70, 0x62, 0x7d, 0x68, 0x6b,
    0x5e, 0x3f, 0x4f, 0x65, 0x27, 0x0d, 0x28, 0x5d,
    0x0b, 0x5b, 0x06, 0x26, 0x4e, 0x0f, 0x41, 0x3e,
    0x5c, 0x17, 0x54, 0x7c, 0x32, 0x31, 0x18, 0x13,
    0x73, 0x7a, 0x03, 0x49, 0x24, 0x4c, 0x25, 0x63,
    0x77, 0x76, 0x51, 0x1a, 0x05, 0x08, 0x69, 0x3c,
    0x34, 0x43, 0x37, 0x44, 0x66, 0x3a, 0x7f, 0x59,
    0x20, 0x47, 0x72, 0x16, 0x2e, 0x35, 0x2c, 0x33,
    0x14, 0x1f, 0x52, 0x29, 0x45, 0x6a, 0x6e, 0x0e,
    0x15, 0x30, 0x42, 0x36, 0x10, 0x60, 0x74, 0x07,
    0x64, 0x48, 0x79, 0x0a, 0x02, 0x04, 0x6d, 0x4d,
    0x3b, 0x22, 0x1d, 0x2a, 0x7e, 0x39, 0x1a, 0x7b,
    0x12, 0x19, 0x6f, 0x57, 0x23, 0x75, 0x41, 0x53,
    0x9c, 0xaf, 0x91, 0xa4, 0xb6, 0xb1, 0xa7, 0x97,
    0x98, 0x84, 0xbd, 0xb9, 0x82, 0xa8, 0xa3, 0xbe,
    0xa2, 0x89, 0x95, 0x9a, 0x94, 0x8e, 0xae, 0xba,
    0xa9, 0x9f, 0xa6, 0xb7, 0xad, 0xb4, 0x87, 0xa5,
    0xab, 0x88, 0x85, 0x93, 0xaa, 0x90, 0x81, 0xbb,
    0x8f, 0x86, 0x92, 0xa0, 0xac, 0x8b, 0xb5, 0x80,
    0xb0, 0x99, 0xb3, 0xbc, 0xb8, 0x9d, 0xbf, 0x8d,
    0x96, 0x8a, 0x9e, 0xb2, 0x9b, 0x83, 0xa1, 0x8c,
    0xc1, 0xd8, 0xd5, 0xd6, 0xc5, 0xc0, 0xd3, 0xcd,
    0xc2, 0xd7, 0xd9, 0xc9, 0xcc, 0xca, 0xdc, 0xd2,
    0xde, 0xc6, 0xc7, 0xdd, 0xdf, 0xcb, 0xda, 0xc3,
    0xdb, 0xc8, 0xd4, 0xce, 0xd0, 0xd1, 0xc4, 0xcf,
    0xef, 0xea, 0xe3, 0xe6, 0xeb, 0xe1, 0xe4, 0xe9,
    0xe2, 0xe0, 0xec, 0xe5, 0xe7, 0xe8, 0xee, 0xed,
    0xf1, 0xf7, 0xf4, 0xf5, 0xf2, 0xf3, 0xf0, 0xf6,
    0xfb, 0xf9, 0xfa, 0xf8, 0xfc, 0xfd, 0xfe, 0xff
};

int awss_dict_crypt(char tab_idx, uint8_t *data, uint8_t len)
{
    uint8_t i = 0;
    uint8_t *table = NULL;

    switch (tab_idx) {
        case SSID_DECODE_TABLE:
            table = (uint8_t *)ssid_dict_decode_table;
            break;
        case NOTIFY_ENCODE_TABLE:
            table = (uint8_t *)notify_encode_table;
            break;
        default:
            table = NULL;
            break;
    }

    if (table == NULL || data == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    for (i = 0; i < len; i ++) {
        data[i] = table[data[i]];
    }

    return 0;
}

int produce_signature(uint8_t *sign, uint8_t *txt,
                      uint32_t txt_len, const char *key)
{
    if (sign == NULL || txt == NULL || txt_len == 0 || key == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    utils_hmac_sha1_hex((const char *)txt, (int)txt_len,
                        (char *)sign, key, strlen(key));

    return 0;
}
#endif
#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif
