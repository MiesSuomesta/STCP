;
MEMORY
{
  FLASH (rx) : ORIGIN = (0x0), LENGTH = (0x50000)
  RAM (rw) : ORIGIN = (0x20000000), LENGTH = (0x16000)
}
__msp_stack_size__ = (0x00000800);
ENTRY(Reset_Handler)
SECTIONS
{
    Image$$PT_RO_START$$Base = ADDR(.TFM_VECTORS);
    .TFM_VECTORS : ALIGN(4)
    {
        __vectors_start__ = .;
        KEEP(*(.vectors))
        . = ALIGN(4);
        __vectors_end__ = .;
    } > FLASH
    ASSERT(__vectors_start__ != __vectors_end__, ".vectors should not be empty")
    ASSERT(. <= ADDR(.TFM_VECTORS) + (0x144), ".TFM_VECTORS section size overflow.")
    . = ADDR(.TFM_VECTORS) + (0x144);
    .TFM_SP_LOAD_LIST ALIGN(4) :
    {
       KEEP(*(.part_load_priority_00))
       KEEP(*(.part_load_priority_01))
       KEEP(*(.part_load_priority_02))
       KEEP(*(.part_load_priority_03))
    } > FLASH
    Image$$TFM_SP_LOAD_LIST$$RO$$Base = ADDR(.TFM_SP_LOAD_LIST);
    Image$$TFM_SP_LOAD_LIST$$RO$$Limit = ADDR(.TFM_SP_LOAD_LIST) + SIZEOF(.TFM_SP_LOAD_LIST);
    . = ALIGN(32);
    Image$$TFM_PSA_CODE_START$$Base = .;
    .TFM_PSA_ROT_LINKER ALIGN(32) :
    {
        *tfm_psa_rot_partition*:(SORT_BY_ALIGNMENT(.text*))
        *tfm_psa_rot_partition*:(SORT_BY_ALIGNMENT(.rodata*))
        *tfm_*partition_its.*:(SORT_BY_ALIGNMENT(.text*))
        *tfm_*partition_its.*:(SORT_BY_ALIGNMENT(.rodata*))
        *(TFM_SP_ITS_PSA-ROT_ATTR_FN)
        *tfm_*partition_crypto.*:(SORT_BY_ALIGNMENT(.text*))
        *tfm_*partition_crypto.*:(SORT_BY_ALIGNMENT(.rodata*))
        *mbedcrypto.*:(SORT_BY_ALIGNMENT(.text*))
        *mbedcrypto.*:(SORT_BY_ALIGNMENT(.rodata*))
        *mbedtls*acceleration.*:(SORT_BY_ALIGNMENT(.text*))
        *mbedtls*acceleration.*:(SORT_BY_ALIGNMENT(.rodata*))
        *(TFM_SP_CRYPTO_PSA-ROT_ATTR_FN)
        *tfm_*partition_platform.*:(SORT_BY_ALIGNMENT(.text*))
        *tfm_*partition_platform.*:(SORT_BY_ALIGNMENT(.rodata*))
        *(TFM_SP_PLATFORM_PSA-ROT_ATTR_FN)
        *(TFM_*_PSA-ROT_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_PSA_ROT_LINKER$$RO$$Base = ADDR(.TFM_PSA_ROT_LINKER);
    Image$$TFM_PSA_ROT_LINKER$$RO$$Limit = ADDR(.TFM_PSA_ROT_LINKER) + SIZEOF(.TFM_PSA_ROT_LINKER);
    Image$$TFM_PSA_ROT_LINKER$$Base = ADDR(.TFM_PSA_ROT_LINKER);
    Image$$TFM_PSA_ROT_LINKER$$Limit = ADDR(.TFM_PSA_ROT_LINKER) + SIZEOF(.TFM_PSA_ROT_LINKER);
    Image$$TFM_PSA_CODE_END$$Base = .;
    Image$$TFM_APP_CODE_START$$Base = .;
    .TFM_APP_ROT_LINKER ALIGN(32) :
    {
        *tfm_app_rot_partition*:(SORT_BY_ALIGNMENT(.text*))
        *tfm_app_rot_partition*:(SORT_BY_ALIGNMENT(.rodata*))
        *tfm_*partition_ps.*:(SORT_BY_ALIGNMENT(.text*))
        *tfm_*partition_ps.*:(SORT_BY_ALIGNMENT(.rodata*))
        *(TFM_SP_PS_APP-ROT_ATTR_FN)
        *(TFM_*_APP-ROT_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_APP_ROT_LINKER$$RO$$Base = ADDR(.TFM_APP_ROT_LINKER);
    Image$$TFM_APP_ROT_LINKER$$RO$$Limit = ADDR(.TFM_APP_ROT_LINKER) + SIZEOF(.TFM_APP_ROT_LINKER);
    Image$$TFM_APP_ROT_LINKER$$Base = ADDR(.TFM_APP_ROT_LINKER);
    Image$$TFM_APP_ROT_LINKER$$Limit = ADDR(.TFM_APP_ROT_LINKER) + SIZEOF(.TFM_APP_ROT_LINKER);
    Image$$TFM_APP_CODE_END$$Base = .;
    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH
    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > FLASH
    __exidx_end = .;
    .ER_TFM_CODE ALIGN(4) (READONLY) :
    {
        . = ALIGN(4);
        PROVIDE_HIDDEN (__preinit_array_start = .);
        PROVIDE(__bothinit_array_start = __preinit_array_start);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);
        . = ALIGN(4);
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);
        PROVIDE(__bothinit_array_end = __init_array_end);
        . = ALIGN(4);
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);
        . = ALIGN(4);
        __copy_table_start__ = .;
        LONG (LOADADDR(.TFM_DATA))
        LONG (ADDR(.TFM_DATA))
        LONG (SIZEOF(.TFM_DATA) / 4)
        LONG (LOADADDR(.TFM_PSA_ROT_LINKER_DATA))
        LONG (ADDR(.TFM_PSA_ROT_LINKER_DATA))
        LONG (SIZEOF(.TFM_PSA_ROT_LINKER_DATA) / 4)
        LONG (LOADADDR(.TFM_APP_ROT_LINKER_DATA))
        LONG (ADDR(.TFM_APP_ROT_LINKER_DATA))
        LONG (SIZEOF(.TFM_APP_ROT_LINKER_DATA) / 4)
        __copy_table_end__ = .;
        . = ALIGN(4);
        __zero_table_start__ = .;
        LONG (ADDR(.TFM_BSS))
        LONG (SIZEOF(.TFM_BSS) / 4)
        LONG (ADDR(.TFM_PSA_ROT_LINKER_BSS))
        LONG (SIZEOF(.TFM_PSA_ROT_LINKER_BSS) / 4)
        LONG (ADDR(.TFM_APP_ROT_LINKER_BSS))
        LONG (SIZEOF(.TFM_APP_ROT_LINKER_BSS) / 4)
        LONG (ADDR(.TFM_SP_META_PTR))
        LONG (SIZEOF(.TFM_SP_META_PTR) / 4)
        __zero_table_end__ = .;
        *startup*(.text*)
        *libplatform_s*:(SORT_BY_ALIGNMENT(.text*))
        *libtfm_spm*:(SORT_BY_ALIGNMENT(.text*))
        *libplatform_s*:*(.rodata*)
        *libtfm_spm*:*(.rodata*)
    } > FLASH
    ASSERT((SIZEOF(.TFM_DATA)) % 4 == 0, "SIZEOF(.TFM_DATA)")
    ASSERT((SIZEOF(.TFM_PSA_ROT_LINKER_DATA)) % 4 == 0, "SIZEOF(.TFM_PSA_ROT_LINKER_DATA)")
    ASSERT((SIZEOF(.TFM_APP_ROT_LINKER_DATA)) % 4 == 0, "SIZEOF(.TFM_APP_ROT_LINKER_DATA)")
    ASSERT((SIZEOF(.TFM_PSA_ROT_LINKER_BSS)) % 4 == 0, "SIZEOF(.TFM_PSA_ROT_LINKER_BSS)")
    ASSERT((SIZEOF(.TFM_APP_ROT_LINKER_BSS)) % 4 == 0, "SIZEOF(.TFM_APP_ROT_LINKER_BSS)")
    ASSERT((SIZEOF(.TFM_SP_META_PTR)) % 4 == 0, "SIZEOF(.TFM_SP_META_PTR)")
    .TFM_UNPRIV_CODE ALIGN(32) :
    {
        *(SORT_BY_ALIGNMENT(.text*))
        KEEP(*(.init))
        KEEP(*(.fini))
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)
         *crtbegin.o(.dtors)
         *crtbegin?.o(.dtors)
         *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
         *(SORT(.dtors.*))
         *(.dtors)
        *(SORT_BY_ALIGNMENT(.rodata*))
        KEEP(*(.eh_frame*))
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_UNPRIV_CODE_START$$RO$$Base = ADDR(.TFM_UNPRIV_CODE);
    Image$$TFM_UNPRIV_CODE_END$$RO$$Limit = ADDR(.TFM_UNPRIV_CODE) + SIZEOF(.TFM_UNPRIV_CODE);
    .gnu.sgstubs (ALIGN(0x8000) - (0x400) + (. > (ALIGN(0x8000) - (0x400)) ? 0x8000 : 0)) : { *(.gnu.sgstubs*) } > FLASH .sgstubs_end : ALIGN(0x8000) { } > FLASH
    . = ALIGN(32);
    Image$$PT_RO_END$$Base = .;
    .tfm_secure_data_start :
    {
        . = ALIGN(4);
    } > RAM
    .tfm_bl2_shared_data ALIGN(32) :
    {
        . += (0x400);
    } > RAM
    .msp_stack ALIGN(32) :
    {
        . += __msp_stack_size__ - 0x8;
    } > RAM
    Image$$ARM_LIB_STACK$$ZI$$Base = ADDR(.msp_stack);
    Image$$ARM_LIB_STACK$$ZI$$Limit = ADDR(.msp_stack) + SIZEOF(.msp_stack);
    .msp_stack_seal_res :
    {
        . += 0x8;
    } > RAM
    __StackSeal = ADDR(.msp_stack_seal_res);
    __heap_size__ = (0x00001000);
    .heap ALIGN(8) :
    {
        __end__ = .;
        PROVIDE(end = .);
        __HeapBase = .;
        PROVIDE(__heap_start = __HeapBase);
        . += __heap_size__;
        __HeapLimit = .;
        PROVIDE(__heap_end = __HeapLimit);
        __heap_limit = .;
    } > RAM
    .TFM_SP_META_PTR ALIGN(32) (NOLOAD):
    {
        *(.bss.SP_META_PTR_SPRTL_INST)
        . = ALIGN(32);
    } > RAM
    Image$$TFM_SP_META_PTR$$ZI$$Base = ADDR(.TFM_SP_META_PTR);
    Image$$TFM_SP_META_PTR$$ZI$$Limit = ADDR(.TFM_SP_META_PTR) + SIZEOF(.TFM_SP_META_PTR);
    Image$$TFM_SP_META_PTR_END$$ZI$$Limit = Image$$TFM_SP_META_PTR$$ZI$$Limit;
    . = ALIGN(32);
    Image$$TFM_APP_RW_STACK_START$$Base = .;
    .TFM_APP_ROT_LINKER_DATA ALIGN(32) :
    {
        *tfm_app_rot_partition*:(SORT_BY_ALIGNMENT(.data*))
        *tfm_*partition_ps.*:(SORT_BY_ALIGNMENT(.data*))
        *(TFM_SP_PS_APP-ROT_ATTR_RW)
        *(TFM_*_APP-ROT_ATTR_RW)
        . = ALIGN(4);
    } > RAM AT> FLASH
    Image$$TFM_APP_ROT_LINKER_DATA$$RW$$Base = ADDR(.TFM_APP_ROT_LINKER_DATA);
    Image$$TFM_APP_ROT_LINKER_DATA$$RW$$Limit = ADDR(.TFM_APP_ROT_LINKER_DATA) + SIZEOF(.TFM_APP_ROT_LINKER_DATA);
    .TFM_APP_ROT_LINKER_BSS ALIGN(4) (NOLOAD) :
    {
        start_of_TFM_APP_ROT_LINKER = .;
        *tfm_app_rot_partition*:(SORT_BY_ALIGNMENT(.bss*))
        *tfm_app_rot_partition*:*(COMMON)
        *tfm_*partition_ps.*:(SORT_BY_ALIGNMENT(.bss*))
        *tfm_*partition_ps.*:*(COMMON)
        *(TFM_SP_PS_APP-ROT_ATTR_ZI)
        *(TFM_*_APP-ROT_ATTR_ZI)
        . += (. - start_of_TFM_APP_ROT_LINKER) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$TFM_APP_ROT_LINKER_DATA$$ZI$$Base = ADDR(.TFM_APP_ROT_LINKER_BSS);
    Image$$TFM_APP_ROT_LINKER_DATA$$ZI$$Limit = ADDR(.TFM_APP_ROT_LINKER_BSS) + SIZEOF(.TFM_APP_ROT_LINKER_BSS);
    Image$$TFM_APP_RW_STACK_END$$Base = .;
    Image$$TFM_PSA_RW_STACK_START$$Base = .;
    .TFM_PSA_ROT_LINKER_DATA ALIGN(32) :
    {
        *tfm_psa_rot_partition*:(SORT_BY_ALIGNMENT(.data*))
        *tfm_*partition_its.*:(SORT_BY_ALIGNMENT(.data*))
        *(TFM_SP_ITS_PSA-ROT_ATTR_RW)
        *tfm_*partition_crypto.*:(SORT_BY_ALIGNMENT(.data*))
        *mbedcrypto.*:(SORT_BY_ALIGNMENT(.data*))
        *mbedtls*acceleration.*:(SORT_BY_ALIGNMENT(.data*))
        *(TFM_SP_CRYPTO_PSA-ROT_ATTR_RW)
        *tfm_*partition_platform.*:(SORT_BY_ALIGNMENT(.data*))
        *(TFM_SP_PLATFORM_PSA-ROT_ATTR_RW)
        *(TFM_*_PSA-ROT_ATTR_RW)
        . = ALIGN(4);
    } > RAM AT> FLASH
    Image$$TFM_PSA_ROT_LINKER_DATA$$RW$$Base = ADDR(.TFM_PSA_ROT_LINKER_DATA);
    Image$$TFM_PSA_ROT_LINKER_DATA$$RW$$Limit = ADDR(.TFM_PSA_ROT_LINKER_DATA) + SIZEOF(.TFM_PSA_ROT_LINKER_DATA);
    .TFM_PSA_ROT_LINKER_BSS ALIGN(4) (NOLOAD) :
    {
        start_of_TFM_PSA_ROT_LINKER = .;
        *tfm_psa_rot_partition*:(SORT_BY_ALIGNMENT(.bss*))
        *tfm_psa_rot_partition*:*(COMMON)
        *tfm_*partition_its.*:(SORT_BY_ALIGNMENT(.bss*))
        *tfm_*partition_its.*:*(COMMON)
        *(TFM_SP_ITS_PSA-ROT_ATTR_ZI)
        *tfm_*partition_crypto.*:(SORT_BY_ALIGNMENT(.bss*))
        *tfm_*partition_crypto.*:*(COMMON)
        *mbedcrypto.*:(SORT_BY_ALIGNMENT(.bss*))
        *mbedcrypto.*:*(COMMON)
        *mbedtls*acceleration.*:(SORT_BY_ALIGNMENT(.bss*))
        *mbedtls*acceleration.*:*(COMMON)
        *(TFM_SP_CRYPTO_PSA-ROT_ATTR_ZI)
        *tfm_*partition_platform.*:(SORT_BY_ALIGNMENT(.bss*))
        *tfm_*partition_platform.*:*(COMMON)
        *(TFM_SP_PLATFORM_PSA-ROT_ATTR_ZI)
        *(TFM_*_PSA-ROT_ATTR_ZI)
        . += (. - start_of_TFM_PSA_ROT_LINKER) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$TFM_PSA_ROT_LINKER_DATA$$ZI$$Base = ADDR(.TFM_PSA_ROT_LINKER_BSS);
    Image$$TFM_PSA_ROT_LINKER_DATA$$ZI$$Limit = ADDR(.TFM_PSA_ROT_LINKER_BSS) + SIZEOF(.TFM_PSA_ROT_LINKER_BSS);
    Image$$TFM_PSA_RW_STACK_END$$Base = .;
    .TFM_DATA ALIGN(4) :
    {
        *(SORT_BY_ALIGNMENT(.data*))
        KEEP(*(.jcr*))
        . = ALIGN(4);
    } > RAM AT> FLASH
    Image$$ER_TFM_DATA$$RW$$Base = ADDR(.TFM_DATA);
    Image$$ER_TFM_DATA$$RW$$Limit = ADDR(.TFM_DATA) + SIZEOF(.TFM_DATA);
    .TFM_BSS ALIGN(4) (NOLOAD) :
    {
        __bss_start__ = .;
        __partition_runtime_start__ = .;
        KEEP(*(.bss.part_runtime_priority_00))
        KEEP(*(.bss.part_runtime_priority_01))
        KEEP(*(.bss.part_runtime_priority_02))
        KEEP(*(.bss.part_runtime_priority_03))
        __partition_runtime_end__ = .;
        . = ALIGN(4);
        __service_runtime_start__ = .;
        KEEP(*(.bss.serv_runtime_priority_00))
        KEEP(*(.bss.serv_runtime_priority_01))
        KEEP(*(.bss.serv_runtime_priority_02))
        KEEP(*(.bss.serv_runtime_priority_03))
        __service_runtime_end__ = .;
        *(SORT_BY_ALIGNMENT(.bss*))
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > RAM AT> RAM
    Image$$ER_TFM_DATA$$ZI$$Base = ADDR(.TFM_BSS);
    Image$$ER_TFM_DATA$$ZI$$Limit = ADDR(.TFM_BSS) + SIZEOF(.TFM_BSS);
    Image$$ER_PART_RT_POOL$$ZI$$Base = __partition_runtime_start__;
    Image$$ER_PART_RT_POOL$$ZI$$Limit = __partition_runtime_end__;
    Image$$ER_SERV_RT_POOL$$ZI$$Base = __service_runtime_start__;
    Image$$ER_SERV_RT_POOL$$ZI$$Limit = __service_runtime_end__;
    Image$$ER_TFM_DATA$$Base = ADDR(.TFM_DATA);
    Image$$ER_TFM_DATA$$Limit = ADDR(.TFM_DATA) + SIZEOF(.TFM_DATA) + SIZEOF(.TFM_BSS);
    Image$$ER_VENEER$$Base = ADDR(.gnu.sgstubs);
    Image$$VENEER_ALIGN$$Limit = ADDR(.sgstubs_end);
    ASSERT ((Image$$VENEER_ALIGN$$Limit - Image$$ER_VENEER$$Base) <= (0x400), "Veneer region overflowed")
    Load$$LR$$LR_NS_PARTITION$$Base = ((0x50000));
    PROVIDE(__stack = Image$$ARM_LIB_STACK$$ZI$$Limit);
}
