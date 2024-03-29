
#include "ln88xx.h"


/*----------------------------------------------------------------------------
  Linker generated Symbols
 *----------------------------------------------------------------------------*/
extern uint32_t __copysection_ram0_load;
extern uint32_t __copysection_ram0_start;
extern uint32_t __copysection_ram0_end;
extern uint32_t __etext;
extern uint32_t __bss_ram0_start__;
extern uint32_t __bss_ram0_end__;
extern uint32_t __bss_ram1_start__;
extern uint32_t __bss_ram1_end__;
extern uint32_t __StackTop;
extern uint32_t __retention_start__;
extern uint32_t __retention_end__;


/*----------------------------------------------------------------------------
  Exception / Interrupt Handler Function Prototype
 *----------------------------------------------------------------------------*/
typedef void( *pFunc )( void );


/*----------------------------------------------------------------------------
  External References
 *----------------------------------------------------------------------------*/
//extern void _start     (void) __attribute__((noreturn)); /* PreeMain (C library entry point) */
extern int main (int argc, char* argv[]);


/*----------------------------------------------------------------------------
  Internal References
 *----------------------------------------------------------------------------*/
void Default_Handler(void) __attribute__ ((noreturn));
void Reset_Handler  (void) __attribute__ ((noreturn));


/*----------------------------------------------------------------------------
  User Initial Stack & Heap
 *----------------------------------------------------------------------------*/
//<h> Stack Configuration
//  <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
//</h>
#define  __STACK_SIZE  0x00002000
static uint8_t stack[__STACK_SIZE] __attribute__ ((aligned(8), used, section(".stack")));

#if 0
//<h> Heap Configuration
//  <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
//</h>
#define  __HEAP_SIZE   0x00004000
#if __HEAP_SIZE > 0
static uint8_t heap[__HEAP_SIZE]   __attribute__ ((aligned(8), used, section(".heap")));
#endif
#endif


/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
/* Exceptions */
void NMI_Handler            (void) __attribute__ ((weak, alias("Default_Handler")));
void HardFault_Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void MemManage_Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void BusFault_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void UsageFault_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void SVC_Handler            (void) __attribute__ ((weak, alias("Default_Handler")));
void DebugMon_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void PendSV_Handler         (void) __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));

void WDT_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void EXTERNAL_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void RTC_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void SLEEP_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void MAC_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void QSPI_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void SDIO_FUN1_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void SDIO_FUN2_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void SDIO_FUN3_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void SDIO_FUN4_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void SDIO_FUN5_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void SDIO_FUN6_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void SDIO_FUN7_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void SDIO_ASYNC_HOST_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void SDIO_M2S_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void CM4_INTR0_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void CM4_INTR1_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void CM4_INTR2_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void CM4_INTR3_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void CM4_INTR4_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void CM4_INTR5_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void ADC_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C0_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C1_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void SPI0_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void SPI2_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void UART0_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void UART1_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void SPI1_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void I2S_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void PAOTD_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void PWM_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void TRNG_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void AES_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));

/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/
extern const pFunc __Vectors[240];
const pFunc __Vectors[240] __attribute__ ((section(".vectors"))) = {
    (pFunc)(&__StackTop),                     /*     Initial Stack Pointer */
    Reset_Handler,                            /*     Reset Handler */
    NMI_Handler,                              /* -14 NMI Handler */
    HardFault_Handler,                        /* -13 Hard Fault Handler */
    MemManage_Handler,                        /* -12 MPU Fault Handler */
    BusFault_Handler,                         /* -11 Bus Fault Handler */
    UsageFault_Handler,                       /* -10 Usage Fault Handler */
    0,                                        /*     Reserved */
    0,                                        /*     Reserved */
    0,                                        /*     Reserved */
    0,                                        /*     Reserved */
    SVC_Handler,                              /*  -5 SVCall Handler */
    DebugMon_Handler,                         /*  -4 Debug Monitor Handler */
    0,                                        /*     Reserved */
    PendSV_Handler,                           /*  -2 PendSV Handler */
    SysTick_Handler,                          /*  -1 SysTick Handler */

    /* Interrupts */
    WDT_IRQHandler,                       /*   0 Interrupt 0 */
    EXTERNAL_IRQHandler,                       /*   1 Interrupt 1 */
    RTC_IRQHandler,                       /*   2 Interrupt 2 */
    SLEEP_IRQHandler,                       /*   3 Interrupt 3 */
    MAC_IRQHandler,                       /*   4 Interrupt 4 */
    DMA_IRQHandler,                       /*   5 Interrupt 5 */
    QSPI_IRQHandler,                       /*   6 Interrupt 6 */
    SDIO_FUN1_IRQHandler,                       /*   7 Interrupt 7 */
    SDIO_FUN2_IRQHandler,                       /*   8 Interrupt 8 */
    SDIO_FUN3_IRQHandler,                        /*   9 Interrupt 9 */
    SDIO_FUN4_IRQHandler,
    SDIO_FUN5_IRQHandler,
    SDIO_FUN6_IRQHandler,
    SDIO_FUN7_IRQHandler,
    SDIO_ASYNC_HOST_IRQHandler,
    SDIO_M2S_IRQHandler,
    CM4_INTR0_IRQHandler,
    CM4_INTR1_IRQHandler,
    CM4_INTR2_IRQHandler,
    CM4_INTR3_IRQHandler,
    CM4_INTR4_IRQHandler,
    CM4_INTR5_IRQHandler,
    ADC_IRQHandler,
    TIMER_IRQHandler,
    I2C0_IRQHandler,
    I2C1_IRQHandler,
    SPI0_IRQHandler,
    SPI2_IRQHandler,
    UART0_IRQHandler,
    UART1_IRQHandler,
    SPI1_IRQHandler,
    GPIO_IRQHandler,
    I2S_IRQHandler,               // 48 (0xC0) IRQ32
    PAOTD_IRQHandler,               // 48 (0xC4) IRQ33
    PWM_IRQHandler,
    TRNG_IRQHandler,
    AES_IRQHandler,
};

/*----------------------------------------------------------------------------
  Reset Handler called on controller reset
 *----------------------------------------------------------------------------*/
void Reset_Handler(void) {
    uint32_t *pSrc, *pDest;
    uint32_t *pTable __attribute__((unused));


    /* Firstly it copies data from read only memory to RAM.
     * There are two schemes to copy. One can copy more than one sections.
     * Another can copy only one section. The former scheme needs more
     * instructions and read-only data to implement than the latter.
     * Macro __STARTUP_COPY_MULTIPLE is used to choose between two schemes.
     */
    pSrc  = &__copysection_ram0_load;
    pDest = &__copysection_ram0_start;

    for ( ; (pDest < &__copysection_ram0_end); ) {
        *pDest++ = *pSrc++;
    }



    /* Single BSS section scheme.
     *
     * The BSS section is specified by following symbols
     *   __bss_start__: start of the BSS section.
     *   __bss_end__: end of the BSS section.
     *
     * Both addresses must be aligned to 4 bytes boundary.
     */
    pDest = &__bss_ram0_start__;
    for ( ; pDest < &__bss_ram0_end__ ; ) {
        *pDest++ = 0UL;
    }

    pDest = &__bss_ram1_start__;
    for ( ; pDest < &__bss_ram1_end__ ; ) {
        *pDest++ = 0UL;
    }

    pDest = &__retention_start__;
    for ( ; pDest < &__retention_end__ ; ) {
        *pDest++ = 0UL;
    }

    SystemInit();                             /* CMSIS System Initialization */
    main(0, NULL);
}


/*----------------------------------------------------------------------------
  Default Handler for Exceptions / Interrupts
 *----------------------------------------------------------------------------*/
void Default_Handler(void) {

  while(1);
}
