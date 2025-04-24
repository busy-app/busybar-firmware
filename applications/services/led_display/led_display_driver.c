#include "led_display_i.h"

#include <furi.h>
#include <stm32u5xx_ll_dma.h>

#define OCTOSPI_PRESCALLER  8
#define START_REFRESH_COUNT 10
#define START_VSYNC_COUNT   START_REFRESH_COUNT

typedef enum {
    LedDriverCmdNone = 0, // Placeholder
    LedDriverCmdDataLatch = 1, // Latch 16bit data and send it to SRAM
    LedDriverCmdWriteCfg5Dbg = 2, // Write debug register (DBG_MODE, GROUP_SEL)
    LedDriverCmdVsync = 3, // Update display data
    LedDriverCmdWriteCfg1 = 4, // Write configuration register 1
    LedDriverCmdReadCfg1 = 5, // Read configuration register 1
    LedDriverCmdWriteCfg2 = 6, // Write configuration register 2
    LedDriverCmdReadCfg2 = 7, // Read configuration register 2
    LedDriverCmdWriteCfg3 = 8, // Write configuration register 3
    LedDriverCmdReadCfg3 = 9, // Read configuration register 3
    LedDriverCmdWriteCfg4 = 10, // Write configuration register 4
    LedDriverCmdReadCfg4 = 11, // Read configuration register 4
    LedDriverCmdEnOp = 12, // Enable all output channels
    LedDriverCmdDisOp = 13, // Disable all output channels
    LedDriverCmdPreactive = 14, // Write enable command (Send before register writes)
    LedDriverCmdMbist = 15, // Enable SRAM checksum read status
} LedDriverCommand;

typedef union {
    uint16_t value;
    struct {
        uint16_t test_0_2        : 3;
        // Bit 3: Cross-version color difference optimization: (Default: 1'h0)
        uint16_t pwm_c           : 1;
        // Bits 5:4: DATA_MAPPING (1: Enable, Other: Disable, Default: 2'h0)
        uint16_t data_mapping_en : 2;
        // Bits 7:6: Low ash uniformity (Default: 2'h0)
        uint16_t opt_lvl         : 2;
        // Bits 12:8: Number of scan lines, (Default: 5'h1F)
        uint16_t scan_line       : 5;
        uint16_t test_13         : 1;
        // Bit 14: Enable open circuit detection (0: Disable, 1: enable, Default: 1'h0)
        uint16_t open_det_en     : 1;
        uint16_t reserved        : 1;
    } bits;
} LedDriverCfg1;
_Static_assert(sizeof(LedDriverCfg1) == sizeof(uint16_t), "LedDriverCfg1 size mismatch");

typedef union {
    uint16_t value;
    struct {
        // Bit 0: TEST (Text ghost optimization, 0=Open/Enable, 1=Close/Disable, Default: 1'h1)
        uint16_t text_ghost_opt_dis : 1;
        // Bits 8:1: IGAIN (Constant current gain, Range 64-255, Default: 8'hFF)
        uint16_t igain              : 8;
        // Bit 9: I_DIV4N (Current divisor select, 1=IOUT*=/256, 0=IOUT*=/1024, Default: 1'h1)
        uint16_t i_div4n            : 1;
        // Bits 14:10: ADJ Blanking level adjustment (Range 0-31, Default: 5'h1F)
        uint16_t adj                : 5;
        // Bit 15: Reserved
        uint16_t reserved           : 1;
    } bits;
} LedDriverCfg2;
_Static_assert(sizeof(LedDriverCfg2) == sizeof(uint16_t), "LedDriverCfg2 size mismatch");

typedef union {
    uint16_t value;
    struct {
        uint16_t test_cfg    : 2;
        // Bit 2: UP_SEL (Blanking level select, Default: 1'b1)
        uint16_t up_sel      : 1;
        uint16_t test_3      : 1;
        // Bits 7:4: PWM_ADD (Low gray color cast compensation level, Range 0-15, Default: 4'h0)
        uint16_t pwm_add     : 4;
        // Bit 8: Reg_EN (Register 5 write enable, 0: Disable, 1: Enable, Default: 1'h0)
        uint16_t reg_en      : 1;
        // Bit 9: (Register map select, 0: Write Reg1-4, 1: Write Debug Reg5, Default: 1'h1)
        uint16_t reg_map_sel : 1;
        uint16_t test_10_11  : 2;
        uint16_t test_12_14  : 3;
        uint16_t reserved_15 : 1;
    } bits;
} LedDriverCfg3;
_Static_assert(sizeof(LedDriverCfg3) == sizeof(uint16_t), "LedDriverCfg3 size mismatch");

typedef union {
    uint16_t value;
    struct {
        // Bit 0: Mapping_EN (Default: 1'h0)
        uint16_t mapping_en  : 1;
        // Bits 2:1: TRIM_ADJ (Constant current trimming value, Default: 2'h0)
        uint16_t trim_adj    : 2;
        // Bit 3: TRIM_ADD_EN (Trimming sign, 0: Subtract, 1: Add, Default: 1'h0)
        uint16_t trim_add_en : 1;
        // Bits 5:4: DN_SEL (First row dark compensation level, Range 0-3, Default: 2'h0)
        uint16_t dn_sel      : 2;
        // Bit 6: DN (First row dark compensation enable, Default: 1'h1)
        uint16_t dn_en       : 1;
        // Bit 7: OPEN_SCAN (Open circuit detection scan, 0: Off, 1: Reset & On, Default: 1'h0)
        uint16_t open_scan   : 1;
        uint16_t test_8_9    : 2;
        uint16_t test_10_11  : 2;
        uint16_t test_12     : 1;
        uint16_t test_13     : 1;
        // Bit 14: PWM_ADD_EN (Low gray compensation enable, Default: 1'h0)
        uint16_t pwm_add_en  : 1;
        uint16_t reserved    : 1;
    } bits;
} LedDriverCfg4;

_Static_assert(sizeof(LedDriverCfg4) == sizeof(uint16_t), "LedDriverCfg4 size mismatch");

// VSYNC command: LE high during 3x DCLK periods + 1 dummy clock
#define VSYNC_CMD ((1 << 3) | (1 << 5) | (1 << 7))

// Display data load order look-up table
static const uint16_t tx_idx_lut[] = {
    1128, 1104, 1080, 1056, 1032, 1008, 984,  960, 936, 912, 888, 864, 840, 816, 792, 768,
    744,  720,  696,  672,  648,  624,  600,  576, 48,  24,  0,   120, 96,  72,  192, 168,
    144,  264,  240,  216,  336,  312,  288,  408, 384, 360, 480, 456, 432, 552, 528, 504,
    1129, 1105, 1081, 1057, 1033, 1009, 985,  961, 937, 913, 889, 865, 841, 817, 793, 769,
    745,  721,  697,  673,  649,  625,  601,  577, 49,  25,  1,   121, 97,  73,  193, 169,
    145,  265,  241,  217,  337,  313,  289,  409, 385, 361, 481, 457, 433, 553, 529, 505,
    1130, 1106, 1082, 1058, 1034, 1010, 986,  962, 938, 914, 890, 866, 842, 818, 794, 770,
    746,  722,  698,  674,  650,  626,  602,  578, 50,  26,  2,   122, 98,  74,  194, 170,
    146,  266,  242,  218,  338,  314,  290,  410, 386, 362, 482, 458, 434, 554, 530, 506,
    1131, 1107, 1083, 1059, 1035, 1011, 987,  963, 939, 915, 891, 867, 843, 819, 795, 771,
    747,  723,  699,  675,  651,  627,  603,  579, 51,  27,  3,   123, 99,  75,  195, 171,
    147,  267,  243,  219,  339,  315,  291,  411, 387, 363, 483, 459, 435, 555, 531, 507,
    1132, 1108, 1084, 1060, 1036, 1012, 988,  964, 940, 916, 892, 868, 844, 820, 796, 772,
    748,  724,  700,  676,  652,  628,  604,  580, 52,  28,  4,   124, 100, 76,  196, 172,
    148,  268,  244,  220,  340,  316,  292,  412, 388, 364, 484, 460, 436, 556, 532, 508,
    1133, 1109, 1085, 1061, 1037, 1013, 989,  965, 941, 917, 893, 869, 845, 821, 797, 773,
    749,  725,  701,  677,  653,  629,  605,  581, 53,  29,  5,   125, 101, 77,  197, 173,
    149,  269,  245,  221,  341,  317,  293,  413, 389, 365, 485, 461, 437, 557, 533, 509,
    1134, 1110, 1086, 1062, 1038, 1014, 990,  966, 942, 918, 894, 870, 846, 822, 798, 774,
    750,  726,  702,  678,  654,  630,  606,  582, 54,  30,  6,   126, 102, 78,  198, 174,
    150,  270,  246,  222,  342,  318,  294,  414, 390, 366, 486, 462, 438, 558, 534, 510,
    1135, 1111, 1087, 1063, 1039, 1015, 991,  967, 943, 919, 895, 871, 847, 823, 799, 775,
    751,  727,  703,  679,  655,  631,  607,  583, 55,  31,  7,   127, 103, 79,  199, 175,
    151,  271,  247,  223,  343,  319,  295,  415, 391, 367, 487, 463, 439, 559, 535, 511,
    1136, 1112, 1088, 1064, 1040, 1016, 992,  968, 944, 920, 896, 872, 848, 824, 800, 776,
    752,  728,  704,  680,  656,  632,  608,  584, 56,  32,  8,   128, 104, 80,  200, 176,
    152,  272,  248,  224,  344,  320,  296,  416, 392, 368, 488, 464, 440, 560, 536, 512,
    1137, 1113, 1089, 1065, 1041, 1017, 993,  969, 945, 921, 897, 873, 849, 825, 801, 777,
    753,  729,  705,  681,  657,  633,  609,  585, 57,  33,  9,   129, 105, 81,  201, 177,
    153,  273,  249,  225,  345,  321,  297,  417, 393, 369, 489, 465, 441, 561, 537, 513,
    1138, 1114, 1090, 1066, 1042, 1018, 994,  970, 946, 922, 898, 874, 850, 826, 802, 778,
    754,  730,  706,  682,  658,  634,  610,  586, 58,  34,  10,  130, 106, 82,  202, 178,
    154,  274,  250,  226,  346,  322,  298,  418, 394, 370, 490, 466, 442, 562, 538, 514,
    1139, 1115, 1091, 1067, 1043, 1019, 995,  971, 947, 923, 899, 875, 851, 827, 803, 779,
    755,  731,  707,  683,  659,  635,  611,  587, 59,  35,  11,  131, 107, 83,  203, 179,
    155,  275,  251,  227,  347,  323,  299,  419, 395, 371, 491, 467, 443, 563, 539, 515,
    1140, 1116, 1092, 1068, 1044, 1020, 996,  972, 948, 924, 900, 876, 852, 828, 804, 780,
    756,  732,  708,  684,  660,  636,  612,  588, 60,  36,  12,  132, 108, 84,  204, 180,
    156,  276,  252,  228,  348,  324,  300,  420, 396, 372, 492, 468, 444, 564, 540, 516,
    1141, 1117, 1093, 1069, 1045, 1021, 997,  973, 949, 925, 901, 877, 853, 829, 805, 781,
    757,  733,  709,  685,  661,  637,  613,  589, 61,  37,  13,  133, 109, 85,  205, 181,
    157,  277,  253,  229,  349,  325,  301,  421, 397, 373, 493, 469, 445, 565, 541, 517,
    1142, 1118, 1094, 1070, 1046, 1022, 998,  974, 950, 926, 902, 878, 854, 830, 806, 782,
    758,  734,  710,  686,  662,  638,  614,  590, 62,  38,  14,  134, 110, 86,  206, 182,
    158,  278,  254,  230,  350,  326,  302,  422, 398, 374, 494, 470, 446, 566, 542, 518,
    1143, 1119, 1095, 1071, 1047, 1023, 999,  975, 951, 927, 903, 879, 855, 831, 807, 783,
    759,  735,  711,  687,  663,  639,  615,  591, 63,  39,  15,  135, 111, 87,  207, 183,
    159,  279,  255,  231,  351,  327,  303,  423, 399, 375, 495, 471, 447, 567, 543, 519,
    1144, 1120, 1096, 1072, 1048, 1024, 1000, 976, 952, 928, 904, 880, 856, 832, 808, 784,
    760,  736,  712,  688,  664,  640,  616,  592, 64,  40,  16,  136, 112, 88,  208, 184,
    160,  280,  256,  232,  352,  328,  304,  424, 400, 376, 496, 472, 448, 568, 544, 520,
    1145, 1121, 1097, 1073, 1049, 1025, 1001, 977, 953, 929, 905, 881, 857, 833, 809, 785,
    761,  737,  713,  689,  665,  641,  617,  593, 65,  41,  17,  137, 113, 89,  209, 185,
    161,  281,  257,  233,  353,  329,  305,  425, 401, 377, 497, 473, 449, 569, 545, 521,
    1146, 1122, 1098, 1074, 1050, 1026, 1002, 978, 954, 930, 906, 882, 858, 834, 810, 786,
    762,  738,  714,  690,  666,  642,  618,  594, 66,  42,  18,  138, 114, 90,  210, 186,
    162,  282,  258,  234,  354,  330,  306,  426, 402, 378, 498, 474, 450, 570, 546, 522,
    1147, 1123, 1099, 1075, 1051, 1027, 1003, 979, 955, 931, 907, 883, 859, 835, 811, 787,
    763,  739,  715,  691,  667,  643,  619,  595, 67,  43,  19,  139, 115, 91,  211, 187,
    163,  283,  259,  235,  355,  331,  307,  427, 403, 379, 499, 475, 451, 571, 547, 523,
    1148, 1124, 1100, 1076, 1052, 1028, 1004, 980, 956, 932, 908, 884, 860, 836, 812, 788,
    764,  740,  716,  692,  668,  644,  620,  596, 68,  44,  20,  140, 116, 92,  212, 188,
    164,  284,  260,  236,  356,  332,  308,  428, 404, 380, 500, 476, 452, 572, 548, 524,
    1149, 1125, 1101, 1077, 1053, 1029, 1005, 981, 957, 933, 909, 885, 861, 837, 813, 789,
    765,  741,  717,  693,  669,  645,  621,  597, 69,  45,  21,  141, 117, 93,  213, 189,
    165,  285,  261,  237,  357,  333,  309,  429, 405, 381, 501, 477, 453, 573, 549, 525,
    1150, 1126, 1102, 1078, 1054, 1030, 1006, 982, 958, 934, 910, 886, 862, 838, 814, 790,
    766,  742,  718,  694,  670,  646,  622,  598, 70,  46,  22,  142, 118, 94,  214, 190,
    166,  286,  262,  238,  358,  334,  310,  430, 406, 382, 502, 478, 454, 574, 550, 526,
    1151, 1127, 1103, 1079, 1055, 1031, 1007, 983, 959, 935, 911, 887, 863, 839, 815, 791,
    767,  743,  719,  695,  671,  647,  623,  599, 71,  47,  23,  143, 119, 95,  215, 191,
    167,  287,  263,  239,  359,  335,  311,  431, 407, 383, 503, 479, 455, 575, 551, 527,
};

struct LedDisplayDriver {
    uint8_t spi_buf[DOT_MATRIX_W * DOT_MATRIX_H * PIXEL_BUF_LEN];
    uint16_t gamma_lut[256];
    uint32_t dma_channel;
    uint32_t refresh_count;
    uint32_t vsync_count;
    LedDisplayCallback load_done_callback;
    void* callback_context;
};

static LedDisplayDriver* led_driver;

// Send VSYNC command the fastest possible way
void led_display_driver_vsync_trig(void) {
    if(led_driver->vsync_count < START_VSYNC_COUNT) {
        *(uint8_t*)&OCTOSPI1->DR = (uint8_t)VSYNC_CMD;
        led_driver->vsync_count++;
    }
}

// Start OCTOSPI transfer
inline void led_display_driver_send_buf_start(void) {
    LL_DMA_EnableChannel(GPDMA1, led_driver->dma_channel);
}

// Prepare OCTOSPI transfer
static void octospi_send_buf_prepare(LedDisplayDriver* driver, uint8_t* buf, size_t len) {
    LL_DMA_DisableChannel(GPDMA1, driver->dma_channel);
    LL_DMA_SetSrcAddress(GPDMA1, driver->dma_channel, (uint32_t)(buf));
    LL_DMA_SetDestAddress(GPDMA1, driver->dma_channel, (uint32_t)&OCTOSPI1->DR);
    LL_DMA_SetBlkDataLength(GPDMA1, driver->dma_channel, len);
}

// Wait for OCTOSPI transfer end TODO: wait for thread flag, set in TC IRQ
static void octospi_wait_end(LedDisplayDriver* driver) {
    while(LL_DMA_IsActiveFlag_IDLE(GPDMA1, driver->dma_channel) == 0) {
    }
    LL_DMA_ClearFlag_TC(GPDMA1, driver->dma_channel);
}

static void octospi_dma_tc_irq(void* context) {
    LedDisplayDriver* driver = context;

    if(LL_DMA_IsEnabledIT_TC(GPDMA1, driver->dma_channel) &&
       LL_DMA_IsActiveFlag_TC(GPDMA1, driver->dma_channel)) {
        LL_DMA_DisableIT_TC(GPDMA1, driver->dma_channel);
        if(driver->refresh_count < START_REFRESH_COUNT) {
            driver->refresh_count++;
        } else if(driver->refresh_count == START_REFRESH_COUNT) {
            led_display_scan_output_enable(true);
            driver->refresh_count++;
        } else if(driver->load_done_callback) {
            driver->load_done_callback(driver->callback_context);
        }
    }
}

static void octospi_dma_init(LedDisplayDriver* driver) {
    furi_hal_dma_allocate_gpdma_channel(&driver->dma_channel);

    LL_DMA_InitTypeDef tx_dma_cfg = {0};
    tx_dma_cfg.SrcAddress = 0;
    tx_dma_cfg.DestAddress = (uint32_t)&OCTOSPI1->DR;
    tx_dma_cfg.BlkDataLength = 0;
    tx_dma_cfg.Request = LL_GPDMA1_REQUEST_OCTOSPI1;

    tx_dma_cfg.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    tx_dma_cfg.BlkHWRequest = LL_DMA_HWREQUEST_BLK;
    tx_dma_cfg.DataAlignment = LL_DMA_DATA_ALIGN_ZEROPADD;

    tx_dma_cfg.SrcAllocatedPort = LL_DMA_SRC_ALLOCATED_PORT1;
    tx_dma_cfg.SrcBurstLength = 64; // DMA burst len = OCTOSPI tx FIFO size (64)
    tx_dma_cfg.SrcIncMode = LL_DMA_SRC_INCREMENT;
    tx_dma_cfg.SrcDataWidth = LL_DMA_SRC_DATAWIDTH_BYTE; // TODO: word + burst

    tx_dma_cfg.DestAllocatedPort = LL_DMA_DEST_ALLOCATED_PORT0;
    tx_dma_cfg.DestBurstLength = 64;
    tx_dma_cfg.DestIncMode = LL_DMA_DEST_FIXED;
    tx_dma_cfg.DestDataWidth = LL_DMA_DEST_DATAWIDTH_BYTE;

    tx_dma_cfg.TriggerMode = LL_DMA_TRIGM_BLK_TRANSFER;
    tx_dma_cfg.TriggerPolarity = LL_DMA_TRIG_POLARITY_MASKED;
    tx_dma_cfg.TriggerSelection = 0;

    tx_dma_cfg.TransferEventMode = LL_DMA_TCEM_BLK_TRANSFER;
    tx_dma_cfg.Priority = LL_DMA_HIGH_PRIORITY;
    tx_dma_cfg.LinkAllocatedPort = LL_DMA_LINK_ALLOCATED_PORT1;
    tx_dma_cfg.LinkStepMode = LL_DMA_LSM_FULL_EXECUTION;
    tx_dma_cfg.LinkedListBaseAddr = 0;
    tx_dma_cfg.LinkedListAddrOffset = 0;
    LL_DMA_Init(GPDMA1, driver->dma_channel, &tx_dma_cfg);
    LL_DMA_EnableCDARUpdate(GPDMA1, driver->dma_channel);
    LL_DMA_DisableChannel(GPDMA1, driver->dma_channel);

    furi_hal_interrupt_set_isr_ex(
        furi_hal_dma_get_gpdma_interrupt_id(driver->dma_channel),
        FuriHalInterruptPriorityHighest,
        octospi_dma_tc_irq,
        driver);
}

static void octospi_init(void) {
    furi_hal_bus_enable(FuriHalBusOCTOSPI1);
    furi_hal_bus_enable(FuriHalBusOCTOSPIM);

    OCTOSPI1->DCR1 = (2 << OCTOSPI_DCR1_MTYP_Pos) | (0x1F << OCTOSPI_DCR1_DEVSIZE_Pos);
    OCTOSPI1->DCR2 = ((OCTOSPI_PRESCALLER - 1) << OCTOSPI_DCR2_PRESCALER_Pos);
    OCTOSPI1->DCR3 = 0;
    OCTOSPI1->DCR4 = 0;

    OCTOSPI1->DLR = 0xFFFFFFFF; // Bypass memory size limit

    OCTOSPI1->CCR = (2 << OCTOSPI_CCR_DMODE_Pos);
    OCTOSPI1->WCCR = (2 << OCTOSPI_CCR_DMODE_Pos);
    OCTOSPI1->TCR = 0;
    OCTOSPI1->ABR = 0;

    OCTOSPI1->CR = OCTOSPI_CR_DMAEN;

    OCTOSPI1->CR |= OCTOSPI_CR_EN;

    OCTOSPIM->PCR[0] |= (OCTOSPIM_PCR_DQSEN | OCTOSPIM_PCR_CLKEN);

    furi_hal_gpio_init_ex(
        &gpio_led_sdi_ospi_d0,
        GpioModeAltFunctionPushPull,
        GpioPullNo,
        GpioSpeedMedium,
        GpioAltFn10OCTOSPI1);
    furi_hal_gpio_init_ex(
        &gpio_led_le_ospi_d1,
        GpioModeAltFunctionPushPull,
        GpioPullNo,
        GpioSpeedMedium,
        GpioAltFn10OCTOSPI1);
    furi_hal_gpio_init_ex(
        &gpio_led_dclk_ospi_clk,
        GpioModeAltFunctionPushPull,
        GpioPullNo,
        GpioSpeedMedium,
        GpioAltFn10OCTOSPI1);
}

static FURI_ALWAYS_INLINE void led_driver_add_le_cmd(uint8_t* tx_data, LedDriverCommand cmd) {
    uint32_t cmd_mask = 0;

    uint8_t bitcnt = 0;
    while(bitcnt < cmd) {
        cmd_mask |= (1 << (bitcnt * 2));
        bitcnt++;
    }

    cmd_mask <<= 1;
    tx_data[0] |= (cmd_mask >> 24);
    tx_data[1] |= (cmd_mask >> 16);
    tx_data[2] |= (cmd_mask >> 8);
    tx_data[3] |= cmd_mask;
}

static const uint8_t interleave_lut[16] =
    {0x00, 0x01, 0x04, 0x05, 0x10, 0x11, 0x14, 0x15, 0x40, 0x41, 0x44, 0x45, 0x50, 0x51, 0x54, 0x55};

static inline void led_driver_encode_byte(uint8_t* tx_data, uint8_t data) {
    tx_data[0] = interleave_lut[data >> 4];
    tx_data[1] = interleave_lut[data & 0x0f];
}

static FURI_ALWAYS_INLINE uint16_t
    led_display_gamma_apply(const uint16_t* gamma_lut, uint8_t in_val) {
    return (gamma_lut[in_val]);
}

static void
    led_display_gamma_lut_generate(uint16_t* gamma_lut, float gamma_val, uint8_t brightness) {
    if(brightness > BRIGHTNESS_VAL_MAX) {
        brightness = BRIGHTNESS_VAL_MAX;
    }

    uint32_t out_max = (brightness * 65535) / BRIGHTNESS_VAL_MAX;

    float inv_gamma = 1.f / (float)gamma_val;

    for(uint16_t i = 0; i < 256; i++) {
        float val_in = ((float)i) / 255.f;
        float val_out = powf(val_in, inv_gamma);
        gamma_lut[i] = (uint16_t)(val_out * out_max);
    }
}

static void
    led_driver_encode_pixel(uint8_t* tx_data, const uint8_t* pix_data, const uint16_t* gamma) {
    // Fast path for empty (black) pixels
    if(pix_data[0] == 0 && pix_data[1] == 0 && pix_data[2] == 0) {
        // Clear all bytes at once for empty pixels
        memset(tx_data, 0, 12);
        return;
    }

    uint16_t led_data = led_display_gamma_apply(gamma, pix_data[0]);
    led_driver_encode_byte(&tx_data[0], (led_data >> 8));
    led_driver_encode_byte(&tx_data[2], (led_data & 0xFF));

    led_data = led_display_gamma_apply(gamma, pix_data[1]);
    led_driver_encode_byte(&tx_data[4], (led_data >> 8));
    led_driver_encode_byte(&tx_data[6], (led_data & 0xFF));

    led_data = led_display_gamma_apply(gamma, pix_data[2]);
    led_driver_encode_byte(&tx_data[8], (led_data >> 8));
    led_driver_encode_byte(&tx_data[10], (led_data & 0xFF));
}

static void led_driver_encode_cmd_16(uint8_t* tx_buf, LedDriverCommand cmd, uint16_t data) {
    led_driver_encode_byte(&tx_buf[0], (data >> 8));
    led_driver_encode_byte(&tx_buf[2], (data & 0xFF));

    led_driver_add_le_cmd(tx_buf, cmd);
}

static void led_driver_encode_buffer(LedDisplayDriver* driver, const uint8_t* frame_buf) {
    size_t tx_idx_offset = 0;
    size_t buf_offset = 0;

    for(size_t transfer_n = 0; transfer_n < 16 * 24; transfer_n++) {
        for(size_t pixel_n = 0; pixel_n < LED_DRIVER_CHAIN; pixel_n++) {
            uint32_t fb_offset = tx_idx_lut[tx_idx_offset++];
            led_driver_encode_pixel(
                &(driver->spi_buf)[buf_offset], &frame_buf[fb_offset * 3], driver->gamma_lut);
            buf_offset += PIXEL_BUF_LEN;
        }

        led_driver_add_le_cmd(&(driver->spi_buf)[buf_offset - 4], LedDriverCmdDataLatch);
    }
}

static void led_driver_encode_empty_buffer(LedDisplayDriver* driver) {
    const uint8_t empty_pixel[3] = {0};

    for(size_t transfer_n = 0, buf_offset = 0; transfer_n < 16 * 24; transfer_n++) {
        for(size_t pixel_n = 0; pixel_n < LED_DRIVER_CHAIN; pixel_n++) {
            led_driver_encode_pixel(driver->spi_buf + buf_offset, empty_pixel, driver->gamma_lut);
            buf_offset += PIXEL_BUF_LEN;
        }

        led_driver_add_le_cmd(&(driver->spi_buf)[buf_offset - 4], LedDriverCmdDataLatch);
    }
}

static void led_driver_write_reg(LedDisplayDriver* driver, LedDriverCommand cmd, uint16_t data[]) {
    size_t tx_len = 4 * (1 + 3 * LED_DRIVER_CHAIN);
    memset(driver->spi_buf, 0, tx_len);
    size_t ptr = 0;

    led_driver_encode_cmd_16(&driver->spi_buf[4 * ptr++], LedDriverCmdPreactive, 0);
    for(size_t i = 0; i < LED_DRIVER_CHAIN; i++) {
        if(i != LED_DRIVER_CHAIN - 1) {
            led_driver_encode_cmd_16(&driver->spi_buf[4 * ptr++], LedDriverCmdNone, data[2]);
            led_driver_encode_cmd_16(&driver->spi_buf[4 * ptr++], LedDriverCmdNone, data[1]);
            led_driver_encode_cmd_16(&driver->spi_buf[4 * ptr++], LedDriverCmdNone, data[0]);
        } else {
            int8_t cmd_len_remain = cmd;
            LedDriverCommand cmd_len_cur = LedDriverCmdNone;
            if(cmd_len_remain > 32) {
                cmd_len_cur = cmd_len_remain - 32;
                cmd_len_remain = 32;
            } else {
                cmd_len_cur = LedDriverCmdNone;
            }
            led_driver_encode_cmd_16(&driver->spi_buf[4 * ptr++], cmd_len_cur, data[2]);
            if(cmd_len_remain > 16) {
                cmd_len_cur = cmd_len_remain - 16;
                cmd_len_remain = 16;
            } else {
                cmd_len_cur = LedDriverCmdNone;
            }
            led_driver_encode_cmd_16(&driver->spi_buf[4 * ptr++], cmd_len_cur, data[1]);
            led_driver_encode_cmd_16(&driver->spi_buf[4 * ptr++], cmd_len_remain, data[0]);
        }
    }

    octospi_send_buf_prepare(driver, driver->spi_buf, tx_len);
    led_display_driver_send_buf_start();
    octospi_wait_end(driver);
}

static void led_display_driver_send_init(LedDisplayDriver* driver) {
    LedDriverCfg1 cfg1 = {
        .bits.scan_line = (DOT_MATRIX_W / LED_DRIVER_CHAIN) - 1,
        .bits.data_mapping_en = 3, // Disable data mapping
    };
    led_driver_write_reg(
        driver, LedDriverCmdWriteCfg1, (uint16_t[]){cfg1.value, cfg1.value, cfg1.value});

    LedDriverCfg2 cfg2_r = {
        .bits.adj = 31,
        .bits.i_div4n = 1,
        .bits.igain = 255,
        .bits.text_ghost_opt_dis = 1,
    };
    LedDriverCfg2 cfg2_g = {
        .bits.adj = 28,
        .bits.i_div4n = 1,
        .bits.igain = 255,
        .bits.text_ghost_opt_dis = 1,
    };
    LedDriverCfg2 cfg2_b = {
        .bits.adj = 23,
        .bits.i_div4n = 1,
        .bits.igain = 255,
        .bits.text_ghost_opt_dis = 1,
    };
    led_driver_write_reg(
        driver, LedDriverCmdWriteCfg2, (uint16_t[]){cfg2_r.value, cfg2_g.value, cfg2_b.value});

    LedDriverCfg3 cfg3 = {
        .bits.test_12_14 = 4,
        .bits.reg_en = 1,
        .bits.pwm_add = 15,
        .bits.up_sel = 1,
        .bits.test_cfg = 3,
    };
    led_driver_write_reg(
        driver, LedDriverCmdWriteCfg3, (uint16_t[]){cfg3.value, cfg3.value, cfg3.value});

    LedDriverCfg4 cfg4 = {
        .bits.pwm_add_en = 1,
        .bits.mapping_en = 1,
    };
    led_driver_write_reg(
        driver, LedDriverCmdWriteCfg4, (uint16_t[]){cfg4.value, cfg4.value, cfg4.value});

    // Enable all output channels
    led_driver_write_reg(driver, LedDriverCmdEnOp, (uint16_t[]){0, 0, 0});
}

static void led_display_driver_send_buffer(LedDisplayDriver* driver) {
    LL_DMA_ClearFlag_TC(GPDMA1, driver->dma_channel);
    LL_DMA_EnableIT_TC(GPDMA1, driver->dma_channel);
    octospi_send_buf_prepare(driver, driver->spi_buf, sizeof(driver->spi_buf));
    led_display_scan_data_sync_enable();
}

void led_display_driver_send_frame(const uint8_t* frame_buf) {
    led_driver_encode_buffer(led_driver, frame_buf);
    led_display_driver_send_buffer(led_driver);
}

void led_display_driver_init(uint8_t initial_brightness) {
    led_driver = malloc(sizeof(LedDisplayDriver));
    led_display_gamma_lut_generate(led_driver->gamma_lut, DISPLAY_GAMMA, initial_brightness);

    octospi_init();
    octospi_dma_init(led_driver);

    led_display_driver_send_init(led_driver);
}

void led_display_driver_set_update_callback(LedDisplayCallback callback, void* context) {
    led_driver->load_done_callback = callback;
    led_driver->callback_context = context;
}

void led_display_driver_start(void) {
    led_driver_encode_empty_buffer(led_driver);

    while(led_driver->refresh_count < START_REFRESH_COUNT) {
        // TODO: Replace delay with proper synchronisation
        furi_delay_ms(5);
        led_display_driver_send_buffer(led_driver);
    }
}

void led_display_driver_set_brightness(uint8_t brightness) {
    led_display_gamma_lut_generate(led_driver->gamma_lut, DISPLAY_GAMMA, brightness);
}
