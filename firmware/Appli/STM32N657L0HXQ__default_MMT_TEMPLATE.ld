/*
******************************************************************************
**
** @file        : STM32N6XX_LRUN.ld
**
** @author      : GPM Application Team
**
** @brief       : Linker script for STM32N6XX Device from STM32N6 series
**                      2048 KBytes RAM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used
**
**  Target      : STMicroelectronics STM32
**
**  Distribution: The file is distributed as is, without any warranty
**                of any kind.
**
******************************************************************************
** @attention
**
** Copyright (c) 2026 STMicroelectronics.
** All rights reserved.
**
** This software is licensed under terms that can be found in the LICENSE file
** in the root directory of this software component.
** If no LICENSE file comes with this software, it is provided AS-IS.
**
******************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = ORIGIN(RAM) + LENGTH(RAM); /* end of "RAM" Ram type memory */
_sstack = _estack - _Min_Stack_Size;

_Min_Heap_Size = 0x200; /* required amount of heap */
_Min_Stack_Size = 0x800; /* required amount of stack */
/* Start user variables */

/* End user variables */

/* Memories definition */
MEMORY
{
/* Start MX regions */
  RxDecripSection	(xrw)	: ORIGIN = 0x34064000, LENGTH = 192
  FLASH	(rx)	: ORIGIN = 0x34000400, LENGTH = 102K
  eth_dma_buffers	(xrw)	: ORIGIN = 0x34064180, LENGTH = 24704
  TxDecripSection	(xrw)	: ORIGIN = 0x340640c0, LENGTH = 192
  RAM	(xrw)	: ORIGIN = 0x34080000, LENGTH = 1536K
/* End MX regions */
/*removed by MX default:   ROM	(xrw)	: ORIGIN = 0x34000400, LENGTH = 511K */
}

/* Sections */
SECTIONS
{
  /* The startup code into "Rom" Rom type memory */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH

  /* The program code and other data into "ROM" Rom type memory */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)
    *(.RamFunc)        /* .RamFunc sections */
    *(.RamFunc*)       /* .RamFunc* sections */

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

  /* Constant data into "ROM" Rom type memory */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH

  .ARM.extab   : {
    . = ALIGN(4);
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    . = ALIGN(4);
  } >FLASH

  .ARM : {
    . = ALIGN(4);
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
    . = ALIGN(4);
  } >FLASH

  .preinit_array     :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
    . = ALIGN(4);
  } >FLASH

  .init_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
    . = ALIGN(4);
  } >FLASH

  .fini_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
    . = ALIGN(4);
  } >FLASH

  /* Used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections into "RAM" Ram type memory */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */

  } >RAM AT>FLASH

  .noncacheable :
  {
    . = ALIGN(8);
    __snoncacheable = .;/* create symbol for start of section */
    KEEP(*(.noncacheable))
    . = ALIGN(8);
    __enoncacheable = .;  /* create symbol for end of section */
  } > RAM

  .gnu.sgstubs :
  {
    _sNSCVeneer = .;
    . = ALIGN(4);
    *(.gnu.sgstubs*)   /* Secure Gateway stubs */
    . = ALIGN(4);
  } >FLASH
  _eNSCVeneer = .;

  /* Uninitialized data section into "RAM" Ram type memory */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss section */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM

  /* User_heap_stack section, used to check that there is enough "RAM" Ram  type memory left */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM

  /* Remove information from the compiler libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }

/* DEFAULT SECTION GENERATED BY MX */
 /*  Unitialized TxDescripSection section into "TxDescripSection" TxDescripSection type memory [MMT section] */
 .TxDescripSection :
  {
    . = ALIGN(4);
    KEEP (*(.TxDescripSection))
    . = ALIGN(4);
  } >TxDescripSection
/* DEFAULT SECTION GENERATED BY MX */
 /*  Unitialized RxDescripSection section into "RxDescripSection" RxDescripSection type memory [MMT section] */
 .RxDescripSection :
  {
    . = ALIGN(4);
    KEEP (*(.RxDescripSection))
    . = ALIGN(4);
  } >RxDescripSection
/* DEFAULT SECTION GENERATED BY MX */
 /*  Unitialized RxDecripSection section into "RxDecripSection" RxDecripSection type memory [MMT section] */
 .RxDecripSection :
  {
    . = ALIGN(4);
    KEEP (*(.RxDecripSection))
    . = ALIGN(4);
  } >RxDecripSection

/* DEFAULT SECTION GENERATED BY MX */
 /*  Unitialized eth_dma_buffers section into "eth_dma_buffers" eth_dma_buffers type memory [MMT section] */
 .eth_dma_buffers :
  {
    . = ALIGN(4);
    KEEP (*(.eth_dma_buffers))
    . = ALIGN(4);
  } >eth_dma_buffers

/* DEFAULT SECTION GENERATED BY MX */
 /*  Unitialized TxDecripSection section into "TxDecripSection" TxDecripSection type memory [MMT section] */
 .TxDecripSection :
  {
    . = ALIGN(4);
    KEEP (*(.TxDecripSection))
    . = ALIGN(4);
  } >TxDecripSection

}
