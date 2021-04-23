
/* Memory Section Lengths and Offsets */
_flash_origin = 0x08000000;
_flash_length = 1M;

_bootloader_origin = _flash_origin;
_bootloader_length = 16k;

/* 
    All shared data will be placed in the back 1K of bootloader FLASH 
    Shared data is fully encapsulated by the bootloader section of FLASH
*/
_shared_length = 16k;
_shared_origin = _bootloader_origin + _bootloader_length - _shared_length;

/* Application code is placed after bootloader block */
_app_origin = _bootloader_origin + _bootloader_length;
_app_length = _flash_length - (_bootloader_length);

/* Main entrypoint for ARM CMSIS */
ENTRY(Reset_Handler)

_estack = 0x20001800;

MEMORY 
{
    RAM (rwx)          : ORIGIN = 0x20000000, LENGTH = 8k
    BL_FLASH (rwx)     : ORIGIN = _bootloader_origin, LENGTH = _bootloader_length
    SHARED_FLASH (rw)  : ORIGIN = _shared_origin, LENGTH = _shared_length
    APP_FLASH (rwx)    : ORIGIN = _app_origin, LENGTH = _app_length
}

SECTIONS
{
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > BL_FLASH /* Default vector table goes into Bootloader flash*/

    .text :
    {
        . = ALIGN(4);
        *(.text*)
        *(.rodata*)

        KEEP(*(.init))

        . = ALIGN(4);
        _etext = .;
    } > BL_FLASH

    .shared_flash ORIGIN(SHARED_FLASH):
    {
        . = ALIGN(4);
        KEEP(*(.shared_flash))
        . = LENGTH(SHARED_FLASH);
    } > SHARED_FLASH

    .idata :
    {
        _sidata = .;
        *(.idata*)
        _eidata = .;
    } > RAM

    /* Data sections placed in RAM */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;

        *(.data)
        *(.data*)
        
        _edata = .;
    } > RAM

    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > RAM
}