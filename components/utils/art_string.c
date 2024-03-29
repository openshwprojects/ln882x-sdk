#include "utils/art_string.h"
#include "ln_types.h"
#include "hal/hal_trng.h"


int substing(const char *strSource, char *strDest, const int startPos, const int endPos)
{
    int len = 0;

    if(strSource && strDest && (startPos <= endPos)) {
        len = ((strlen(strSource) - startPos) > (endPos - startPos)) ? (endPos - startPos) : (strlen(strSource) - startPos);
        strncpy(strDest, strSource + startPos, len);
        strDest[len] = '\0';
        return LN_TRUE;
    } else {
        return LN_FALSE;
    }
}

int art_string_extract_domain_content(char *domain_begin, char *domain_end, char domain_separator, char *flag, char *dest_buffer)
{
    char *domain_separator_addr = NULL, *base = domain_begin;
    int start, end;

    if (!(domain_begin && domain_end && dest_buffer && (domain_begin <= domain_end))) {
        return LN_FALSE;
    }

	if(domain_begin == domain_end){
		return LN_TRUE;
	}

    domain_separator_addr = strchr(domain_begin, domain_separator);

    if(!domain_separator_addr){
        domain_separator_addr = domain_end;
    }

	if(domain_separator_addr && domain_separator_addr == domain_end){
        if(flag){
            domain_begin = strchr(domain_begin, *flag);
            if(domain_begin > domain_separator_addr){
                goto out;
            }
            domain_begin = domain_begin + 1;
            if(domain_begin > domain_separator_addr){
                goto out;
            }
            domain_end = strchr(domain_begin, *flag);
            if(domain_begin > domain_separator_addr){
                goto out;
            }
        }
        start = domain_begin - base;
        end = domain_end - base;
        if(!substing(base, (char *)dest_buffer, start, end)){
            goto out;
        }
        return LN_TRUE;
    }
out:
    return LN_FALSE;
}

uint8_t ln_char2hex(char c)  
{
    if(c - '0' <= 9) {
        return (c - '0');
    } else if('f' - c <= 6) {
        return (c - 'a' + 10);
    } else {
        return (c - 'A' + 10);
    } 
}

int ln_char2hex_safe(char c, uint8_t *hex)
{
    if (!isxdigit(c)) {
        return LN_FALSE;
    }
    *hex = ln_char2hex(c);
    return LN_TRUE;
}

int ln_str2bytes(uint8_t *bytes, const char *str)
{
    uint8_t byte = 0;
    int i = 0;

    while ('\0' != str[i])
    {
        if (!isxdigit(str[i])) {
            return LN_FALSE;
        }
        byte = ln_char2hex(str[i]);
        if (0 == i++ % 2) {
            *bytes = (byte << 4) & 0xF0;
        } else {
            *bytes++ += byte & 0x0F;
        }
    }

    return LN_TRUE;
}

int ln_is_valid_mac_str(const char* mac_str)
{
    int i = 0, s = 0;

    if (mac_str == NULL) {
        return LN_FALSE;
    }
    if (strlen(mac_str) != 17) {
        return LN_FALSE;
    }

    while (*mac_str) 
    {
        if (isxdigit(*mac_str)) {
            i++;
        } else if ((*mac_str == ':') || (*mac_str == '-')) {
            if ((i == 0) || ((i/2 - 1) != s)) {
                break;
            }
            ++s;
        } else {
            s = -1;
        }
        ++mac_str;
    }

    return ((i == 12) && (s == 5 || s == 0) ? LN_TRUE : LN_FALSE);
}

int ln_mac_str2hex(const uint8_t *str, uint8_t *hex)
{
    const uint8_t *tmp = str;    
    if (LN_TRUE != ln_is_valid_mac_str((const char*)tmp)) {
        return LN_FALSE;
    }

    for (int i = 0; i < 6; ++i) {    
        hex[i]  = ln_char2hex(*tmp++) * 16;    
        hex[i] += ln_char2hex(*tmp++);
        tmp++;
    }       
    return LN_TRUE;
}

int ln_is_valid_mac(const char *mac)
{
    if (mac == NULL) {
        return LN_FALSE;
    }

    if(!memcmp(mac, "\x00\x00\x00\x00\x00\x00", 6)) {
        return LN_FALSE;
    }
    if(!memcmp(mac, "\xFF\xFF\xFF\xFF\xFF\xFF", 6)) {
        return LN_FALSE;
    }

    return LN_TRUE;
}

int ln_is_zero_mem(const void *addr, uint32_t size)
{
    uint8_t *_addr = (uint8_t *)addr;

    if (!addr) {
        return LN_FALSE;
    }
    
    for (uint32_t i = 0; i < size; i++) {
        if(_addr[i]) {
            return LN_FALSE;
        }
    }
    return LN_TRUE;
}

int ln_generate_true_random_words(uint32_t *words, uint32_t words_len)
{
    if(!words && ((((uint32_t)words) % sizeof(uint32_t)) != 0)){
        return LN_FALSE;
    }

    TRNG_InitStruct initStruct;
    initStruct.srcLength = Length_Long;
    initStruct.sampleCnt = 0x3F;

    HAL_TRNG_Init(TRNG, initStruct);
    HAL_TRNG_Start(TRNG);
    while(!HAL_TRNG_isDataReady(TRNG) || HAL_TRNG_isBusy(TRNG));

    for (uint32_t i = 0; i < words_len; i++) {
        *words++ = HAL_TRNG_GetRandomNumber(TRNG, i);
    }
    HAL_TRNG_Stop(TRNG);
    return LN_TRUE;
}

int ln_generate_random_mac(uint8_t *addr)
{
    #define LN_MAC_OUI    "00-50-C2"

    TRNG_InitStruct initStruct;
    uint32_t trngNumber[2] = {0};

    if(!addr){
        return -1;
    }
    initStruct.srcLength = Length_Long;
    initStruct.sampleCnt = 0x3F;
    HAL_TRNG_Init(TRNG, initStruct);
    HAL_TRNG_Start(TRNG);
    while(!HAL_TRNG_isDataReady(TRNG) || HAL_TRNG_isBusy(TRNG));
#ifdef LN_MAC_OUI
    uint32_t addr0, addr1, addr2;
    sscanf(LN_MAC_OUI, "%X-%X-%X", &addr0, &addr1, &addr2);
    *addr = addr0 & 0xFF;
    *(addr + 1) = addr1 & 0xFF;
    *(addr + 2) = addr2 & 0xFF;
    for (int i = 0; i < 1; i++) {
        trngNumber[i] = HAL_TRNG_GetRandomNumber(TRNG, i);
    }
    HAL_TRNG_Stop(TRNG);
    memcpy(addr + 3, trngNumber, 3);
#else
    for (int i = 0; i < 2; i++) {
        trngNumber[i] = HAL_TRNG_GetRandomNumber(TRNG, i);
    }
    HAL_TRNG_Stop(TRNG);
    memcpy(addr, trngNumber, 6);
    CLR_BIT(addr[0],0);
    CLR_BIT(addr[0],1);
#endif
    return 0;
}

