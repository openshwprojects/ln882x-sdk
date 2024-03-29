/**
 * @file freertos_common.c
 * @author LightningSemi WLAN Team
 * Copyright (C) 2018 LightningSemi Technology Co., Ltd. All rights reserved.
 */
#include "proj_config.h"
#include "./FreeRTOS_Adapter/freertos_common.h"
#include "ln88xx.h"

#ifdef __CC_ARM
    extern unsigned int Image$$HEAP_SPACE0$$ZI$$Base;
    extern unsigned int Image$$HEAP_SPACE0$$ZI$$Limit;
    extern unsigned int Image$$HEAP_SPACE1$$ZI$$Base;
    extern unsigned int Image$$HEAP_SPACE1$$ZI$$Limit;
    extern unsigned int Image$$HEAP_SPACE2$$ZI$$Base;
    extern unsigned int Image$$HEAP_SPACE2$$ZI$$Limit;
    #define HEAP0_START                      (&Image$$HEAP_SPACE0$$ZI$$Base)
    #define HEAP0_END                        (&Image$$HEAP_SPACE0$$ZI$$Limit)
	#define HEAP0_LEN                        ((uint8_t *)HEAP0_END - (uint8_t *)HEAP0_START)
    #define HEAP1_START                      (&Image$$HEAP_SPACE1$$ZI$$Base)
    #define HEAP1_END                        (&Image$$HEAP_SPACE1$$ZI$$Limit)
	#define HEAP1_LEN                        ((uint8_t *)HEAP1_END - (uint8_t *)HEAP1_START)
    #define HEAP2_START                      (&Image$$HEAP_SPACE2$$ZI$$Base)
    #define HEAP2_END                        (&Image$$HEAP_SPACE2$$ZI$$Limit)
    #define HEAP2_LEN                        ((uint8_t *)HEAP2_END - (uint8_t *)HEAP2_START)
#elif __ICCARM__
    #error "TODO: support iar compiler!!!"
#elif __GNUC__
    extern void *heap0_start;
    extern void *heap0_end;
    extern void *heap0_len;
    extern void *heap1_start;
    extern void *heap1_end;
    extern void *heap1_len;
    extern void *heap2_start;
    extern void *heap2_end;
    extern void *heap2_len;
    #define HEAP0_START                      (&heap0_start)
    #define HEAP0_END                        (&heap0_end)
    #define HEAP0_LEN                        (&heap0_len)
    #define HEAP1_START                      (&heap1_start)
    #define HEAP1_END                        (&heap1_end)
    #define HEAP1_LEN                        (&heap1_len)
    #define HEAP2_START                      (&heap2_start)
    #define HEAP2_END                        (&heap2_end)
    #define HEAP2_LEN                        (&heap2_len)
#else
    #error "Unknown compiler!!!"
#endif


static HeapRegion_t xHeapRegions[] = {
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0}
};

void OS_HeapSizeConfig(void)
{
    xHeapRegions[0].pucStartAddress = (uint8_t *)(HEAP0_START);
    xHeapRegions[0].xSizeInBytes    = (size_t)HEAP0_LEN;

    xHeapRegions[1].pucStartAddress = (uint8_t *)(HEAP1_START);
    xHeapRegions[1].xSizeInBytes    = (size_t)HEAP1_LEN;

    xHeapRegions[2].pucStartAddress = (uint8_t *)(HEAP2_START);
    xHeapRegions[2].xSizeInBytes    = (size_t)HEAP2_LEN;
}

void OS_DefineHeapRegions(void)
{
    OS_HeapSizeConfig();
    vPortDefineHeapRegions(xHeapRegions);
}


