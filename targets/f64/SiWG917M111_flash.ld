/*******************************************************************************
 * GCC Linker script for Silicon Labs devices
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

ENTRY(Reset_Handler)

_stack_size = 0x200; /* Required amount of stack */

_flash_address_start = 0x8202000;
_flash_size = 0x1fe000;

MEMORY {
    FLASH (rx)  : ORIGIN = _flash_address_start, LENGTH = _flash_size
   /* RAM   (rwx) : ORIGIN = 0x000000c, LENGTH = 0x3fbf4
      UDMA0 (rwx) : ORIGIN = 0x003fc00, LENGTH = 0x400 */
    RAM   (rwx) : ORIGIN = 0x000000c, LENGTH = 0x4fbf4
    UDMA0 (rwx) : ORIGIN = 0x004fc00, LENGTH = 0x400
}

SECTIONS {
    .text : {
        KEEP(*(.isr_vector))
        KEEP(*(.reset_handler))
        /* Object files excluded from flash */
        *(EXCLUDE_FILE(
            /* FreeRTOS */
            *croutine.o
            *event_groups.o
            *heap_*.o
            *list.o
            *port.o
            *queue.o
            *stream_buffer.o
            *tasks.o
            *timers.o
            /* Furi layer */
            *check.o
            *critical.o
            *event_flag.o
            *event_loop*.o
            *memmgr*.o
            *message_queue.o
            *mutex.o
            *semaphore.o
            *string.o
            *thread*.o
            *timer.o
            *furi_hal_interrupt.o
            *furi_hal_os.o
            /* Wiseconnect */
            *rsi_hal_mcu_m4_ram.o
            *rsi_hal_mcu_m4_rom.o
            *sl_si91x_driver.o
            *sl_si91x_bus.o
            *sl_sleeptimer.o
            *sl_sleeptimer_hal_si91x_sysrtc.o
            *sl_utility.o
            *rsi_sysrtc.o
            *sl_si91x_low_power_tickless_mode.o
            *sli_si91x_multithreaded.o
            *rsi_deepsleep_soc.o
            *freertos_umm_malloc_host.o
            *malloc_buffers.o
            *sl_rsi_utility.o)
        .text*)

        /* .ctors */
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)

        /* .dtors */
        *crtbegin.o(.dtors)
        *crtbegin?.o(.dtors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
        *(SORT(.dtors.*))
        *(.dtors)
        *(.rodata*)

        KEEP(*(.eh_frame*))
    } > FLASH

    .ARM.extab : {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH

    __exidx_start = .;
    .ARM.exidx : {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > FLASH
    __exidx_end = .;
    __etext = .;

    /* _sidata is used in code startup code */
    _sidata = __etext;

    .data : {
        __data_start__ = .;
        /* _sdata is used in startup code */
        _sdata = __data_start__;
        KEEP(*(.ramVector))
        KEEP(*(.init))
        KEEP(*(.fini))
        *(.data*)
        /* Object files placed in RAM */
        /* FreeRTOS */
        *croutine.o(.text*)
        *event_groups.o(.text*)
        *heap_*.o(.text*)
        *list.o(.text*)
        *port.o(.text*)
        *queue.o(.text*)
        *stream_buffer.o(.text*)
        *tasks.o(.text*)
        *timers.o(.text*)
        /* Furi layer */
        *check.o(.text*)
        *critical.o(.text*)
        *event_flag.o(.text*)
        *event_loop*.o(.text*)
        *memmgr*.o(.text*)
        *message_queue.o(.text*)
        *mutex.o(.text*)
        *semaphore.o(.text*)
        *string.o(.text*)
        *thread*.o(.text*)
        *timer.o(.text*)
        *furi_hal_interrupt.o(.text*)
        *furi_hal_os.o(.text*)
        /* Wiseconnect */
        *rsi_hal_mcu_m4_ram.o(.text*)
        *rsi_hal_mcu_m4_rom.o(.text*)
        *sl_si91x_driver.o(.text*)
        *sl_si91x_bus.o(.text*)
        *sl_sleeptimer.o(.text*)
        *sl_sleeptimer_hal_si91x_sysrtc.o(.text*)
        *sl_utility.o(.text*)
        *rsi_sysrtc.o(.text*)
        *sl_si91x_low_power_tickless_mode.o(.text*)
        *sli_si91x_multithreaded.o(.text*)
        *rsi_deepsleep_soc.o(.text*)
        *freertos_umm_malloc_host.o(.text*)
        *malloc_buffers.o(.text*)
        *sl_rsi_utility.o(.text*)

        /* ipmu calibration data */
		*(.common_ipmu_ram*)
        
        . = ALIGN(4);
        /* preinit data */
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);

        . = ALIGN(4);
        /* init data */

        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);

        . = ALIGN(4);
        /* finit data */
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);

        KEEP(*(.jcr*))
        . = ALIGN(4);
        /* All data end */
        __data_end__ = .;

        /* _edata is used in startup code */
        _edata = __data_end__;
    } > RAM AT> FLASH

    .bss (NOLOAD) : {
        . = ALIGN(4);
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > RAM

    .heap (NOLOAD) : {
        . = ALIGN(4);
        __heap_start__ = .;
        . = ORIGIN(RAM) + LENGTH(RAM) - _stack_size;
        . = ALIGN(4);
        __heap_end__ = .;
    } > RAM

    /* Store remaining heap size as a symbol address */
    __heap_size__ = __heap_end__ - __heap_start__;

    .stack (NOLOAD) : {
        . = ALIGN(4);
        __StackLimit = .;
        . = . + _stack_size;
        . = ALIGN(4);
        __StackTop = .;
        PROVIDE(_stack_end = __StackTop);
    } > RAM

    /* Special section for UDMA0 descriptors */
    .udma_addr0 : {
        . = ALIGN(512);
        *(.udma_addr0*)
    } > UDMA0 AT> FLASH

    __main_flash_end__ = _flash_address_start + _flash_size;
        /* This is where we handle flash storage blocks. We use dummy sections for finding the configured
    * block sizes and then "place" them at the end of flash when the size is known. */
    .internal_storage1 (DSECT) : {
        KEEP(*(.internal_storage1*))
    } > FLASH
    .nvm (DSECT) : {
        KEEP(*(.simee*))
    } > FLASH
    
    linker_nvm_end = __main_flash_end__ - 4096;
    linker_nvm_begin = linker_nvm_end - SIZEOF(.nvm);
    linker_nvm_size = SIZEOF(.nvm);
    linker_storage_end = linker_nvm_begin;
    __nvm3Base = linker_nvm_begin;	
    linker_storage_begin = linker_storage_end - SIZEOF(.internal_storage1);
    linker_storage_size = SIZEOF(.internal_storage1);
    ASSERT((linker_storage_begin >= (__etext + SIZEOF(.data))), "FLASH memory overflowed !")   

}
