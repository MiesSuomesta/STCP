#ifndef __ASM_SYSREG_DEFS_H
#define __ASM_SYSREG_DEFS_H

/* Generated file - do not edit */

#define REG_OSDTRRX_EL1                                 S2_0_C0_C0_2
#define SYS_OSDTRRX_EL1                                 sys_reg(2, 0, 0, 0, 2)
#define SYS_OSDTRRX_EL1_Op0                             2
#define SYS_OSDTRRX_EL1_Op1                             0
#define SYS_OSDTRRX_EL1_CRn                             0
#define SYS_OSDTRRX_EL1_CRm                             0
#define SYS_OSDTRRX_EL1_Op2                             2

#define OSDTRRX_EL1_DTRRX                               GENMASK(31, 0)
#define OSDTRRX_EL1_DTRRX_MASK                          GENMASK(31, 0)
#define OSDTRRX_EL1_DTRRX_SHIFT                         0
#define OSDTRRX_EL1_DTRRX_WIDTH                         32

#define OSDTRRX_EL1_RES0                                (UL(0) | GENMASK_ULL(63, 32))
#define OSDTRRX_EL1_RES1                                (UL(0))
#define OSDTRRX_EL1_UNKN                                (UL(0))

#define REG_MDCCINT_EL1                                 S2_0_C0_C2_0
#define SYS_MDCCINT_EL1                                 sys_reg(2, 0, 0, 2, 0)
#define SYS_MDCCINT_EL1_Op0                             2
#define SYS_MDCCINT_EL1_Op1                             0
#define SYS_MDCCINT_EL1_CRn                             0
#define SYS_MDCCINT_EL1_CRm                             2
#define SYS_MDCCINT_EL1_Op2                             0

#define MDCCINT_EL1_RX                                  GENMASK(30, 30)
#define MDCCINT_EL1_RX_MASK                             GENMASK(30, 30)
#define MDCCINT_EL1_RX_SHIFT                            30
#define MDCCINT_EL1_RX_WIDTH                            1

#define MDCCINT_EL1_TX                                  GENMASK(29, 29)
#define MDCCINT_EL1_TX_MASK                             GENMASK(29, 29)
#define MDCCINT_EL1_TX_SHIFT                            29
#define MDCCINT_EL1_TX_WIDTH                            1

#define MDCCINT_EL1_RES0                                (UL(0) | GENMASK_ULL(63, 31) | GENMASK_ULL(28, 0))
#define MDCCINT_EL1_RES1                                (UL(0))
#define MDCCINT_EL1_UNKN                                (UL(0))

#define REG_MDSCR_EL1                                   S2_0_C0_C2_2
#define SYS_MDSCR_EL1                                   sys_reg(2, 0, 0, 2, 2)
#define SYS_MDSCR_EL1_Op0                               2
#define SYS_MDSCR_EL1_Op1                               0
#define SYS_MDSCR_EL1_CRn                               0
#define SYS_MDSCR_EL1_CRm                               2
#define SYS_MDSCR_EL1_Op2                               2

#define MDSCR_EL1_EHBWE                                 GENMASK(35, 35)
#define MDSCR_EL1_EHBWE_MASK                            GENMASK(35, 35)
#define MDSCR_EL1_EHBWE_SHIFT                           35
#define MDSCR_EL1_EHBWE_WIDTH                           1

#define MDSCR_EL1_EnSPM                                 GENMASK(34, 34)
#define MDSCR_EL1_EnSPM_MASK                            GENMASK(34, 34)
#define MDSCR_EL1_EnSPM_SHIFT                           34
#define MDSCR_EL1_EnSPM_WIDTH                           1

#define MDSCR_EL1_TTA                                   GENMASK(33, 33)
#define MDSCR_EL1_TTA_MASK                              GENMASK(33, 33)
#define MDSCR_EL1_TTA_SHIFT                             33
#define MDSCR_EL1_TTA_WIDTH                             1

#define MDSCR_EL1_EMBWE                                 GENMASK(32, 32)
#define MDSCR_EL1_EMBWE_MASK                            GENMASK(32, 32)
#define MDSCR_EL1_EMBWE_SHIFT                           32
#define MDSCR_EL1_EMBWE_WIDTH                           1

#define MDSCR_EL1_TFO                                   GENMASK(31, 31)
#define MDSCR_EL1_TFO_MASK                              GENMASK(31, 31)
#define MDSCR_EL1_TFO_SHIFT                             31
#define MDSCR_EL1_TFO_WIDTH                             1

#define MDSCR_EL1_RXfull                                GENMASK(30, 30)
#define MDSCR_EL1_RXfull_MASK                           GENMASK(30, 30)
#define MDSCR_EL1_RXfull_SHIFT                          30
#define MDSCR_EL1_RXfull_WIDTH                          1

#define MDSCR_EL1_TXfull                                GENMASK(29, 29)
#define MDSCR_EL1_TXfull_MASK                           GENMASK(29, 29)
#define MDSCR_EL1_TXfull_SHIFT                          29
#define MDSCR_EL1_TXfull_WIDTH                          1

#define MDSCR_EL1_RXO                                   GENMASK(27, 27)
#define MDSCR_EL1_RXO_MASK                              GENMASK(27, 27)
#define MDSCR_EL1_RXO_SHIFT                             27
#define MDSCR_EL1_RXO_WIDTH                             1

#define MDSCR_EL1_TXU                                   GENMASK(26, 26)
#define MDSCR_EL1_TXU_MASK                              GENMASK(26, 26)
#define MDSCR_EL1_TXU_SHIFT                             26
#define MDSCR_EL1_TXU_WIDTH                             1

#define MDSCR_EL1_INTdis                                GENMASK(23, 22)
#define MDSCR_EL1_INTdis_MASK                           GENMASK(23, 22)
#define MDSCR_EL1_INTdis_SHIFT                          22
#define MDSCR_EL1_INTdis_WIDTH                          2

#define MDSCR_EL1_TDA                                   GENMASK(21, 21)
#define MDSCR_EL1_TDA_MASK                              GENMASK(21, 21)
#define MDSCR_EL1_TDA_SHIFT                             21
#define MDSCR_EL1_TDA_WIDTH                             1

#define MDSCR_EL1_SC2                                   GENMASK(19, 19)
#define MDSCR_EL1_SC2_MASK                              GENMASK(19, 19)
#define MDSCR_EL1_SC2_SHIFT                             19
#define MDSCR_EL1_SC2_WIDTH                             1

#define MDSCR_EL1_MDE                                   GENMASK(15, 15)
#define MDSCR_EL1_MDE_MASK                              GENMASK(15, 15)
#define MDSCR_EL1_MDE_SHIFT                             15
#define MDSCR_EL1_MDE_WIDTH                             1

#define MDSCR_EL1_HDE                                   GENMASK(14, 14)
#define MDSCR_EL1_HDE_MASK                              GENMASK(14, 14)
#define MDSCR_EL1_HDE_SHIFT                             14
#define MDSCR_EL1_HDE_WIDTH                             1

#define MDSCR_EL1_KDE                                   GENMASK(13, 13)
#define MDSCR_EL1_KDE_MASK                              GENMASK(13, 13)
#define MDSCR_EL1_KDE_SHIFT                             13
#define MDSCR_EL1_KDE_WIDTH                             1

#define MDSCR_EL1_TDCC                                  GENMASK(12, 12)
#define MDSCR_EL1_TDCC_MASK                             GENMASK(12, 12)
#define MDSCR_EL1_TDCC_SHIFT                            12
#define MDSCR_EL1_TDCC_WIDTH                            1

#define MDSCR_EL1_ERR                                   GENMASK(6, 6)
#define MDSCR_EL1_ERR_MASK                              GENMASK(6, 6)
#define MDSCR_EL1_ERR_SHIFT                             6
#define MDSCR_EL1_ERR_WIDTH                             1

#define MDSCR_EL1_SS                                    GENMASK(0, 0)
#define MDSCR_EL1_SS_MASK                               GENMASK(0, 0)
#define MDSCR_EL1_SS_SHIFT                              0
#define MDSCR_EL1_SS_WIDTH                              1

#define MDSCR_EL1_RES0                                  (UL(0) | GENMASK_ULL(63, 36) | GENMASK_ULL(28, 28) | GENMASK_ULL(25, 24) | GENMASK_ULL(20, 20) | GENMASK_ULL(18, 16) | GENMASK_ULL(11, 7) | GENMASK_ULL(5, 1))
#define MDSCR_EL1_RES1                                  (UL(0))
#define MDSCR_EL1_UNKN                                  (UL(0))

#define REG_OSDTRTX_EL1                                 S2_0_C0_C3_2
#define SYS_OSDTRTX_EL1                                 sys_reg(2, 0, 0, 3, 2)
#define SYS_OSDTRTX_EL1_Op0                             2
#define SYS_OSDTRTX_EL1_Op1                             0
#define SYS_OSDTRTX_EL1_CRn                             0
#define SYS_OSDTRTX_EL1_CRm                             3
#define SYS_OSDTRTX_EL1_Op2                             2

#define OSDTRTX_EL1_DTRTX                               GENMASK(31, 0)
#define OSDTRTX_EL1_DTRTX_MASK                          GENMASK(31, 0)
#define OSDTRTX_EL1_DTRTX_SHIFT                         0
#define OSDTRTX_EL1_DTRTX_WIDTH                         32

#define OSDTRTX_EL1_RES0                                (UL(0) | GENMASK_ULL(63, 32))
#define OSDTRTX_EL1_RES1                                (UL(0))
#define OSDTRTX_EL1_UNKN                                (UL(0))

#define REG_MDSELR_EL1                                  S2_0_C0_C4_2
#define SYS_MDSELR_EL1                                  sys_reg(2, 0, 0, 4, 2)
#define SYS_MDSELR_EL1_Op0                              2
#define SYS_MDSELR_EL1_Op1                              0
#define SYS_MDSELR_EL1_CRn                              0
#define SYS_MDSELR_EL1_CRm                              4
#define SYS_MDSELR_EL1_Op2                              2

#define MDSELR_EL1_BANK                                 GENMASK(5, 4)
#define MDSELR_EL1_BANK_MASK                            GENMASK(5, 4)
#define MDSELR_EL1_BANK_SHIFT                           4
#define MDSELR_EL1_BANK_WIDTH                           2

#define MDSELR_EL1_RES0                                 (UL(0) | GENMASK_ULL(63, 6) | GENMASK_ULL(3, 0))
#define MDSELR_EL1_RES1                                 (UL(0))
#define MDSELR_EL1_UNKN                                 (UL(0))

#define REG_MDSTEPOP_EL1                                S2_0_C0_C5_2
#define SYS_MDSTEPOP_EL1                                sys_reg(2, 0, 0, 5, 2)
#define SYS_MDSTEPOP_EL1_Op0                            2
#define SYS_MDSTEPOP_EL1_Op1                            0
#define SYS_MDSTEPOP_EL1_CRn                            0
#define SYS_MDSTEPOP_EL1_CRm                            5
#define SYS_MDSTEPOP_EL1_Op2                            2

#define MDSTEPOP_EL1_OPCODE                             GENMASK(31, 0)
#define MDSTEPOP_EL1_OPCODE_MASK                        GENMASK(31, 0)
#define MDSTEPOP_EL1_OPCODE_SHIFT                       0
#define MDSTEPOP_EL1_OPCODE_WIDTH                       32

#define MDSTEPOP_EL1_RES0                               (UL(0) | GENMASK_ULL(63, 32))
#define MDSTEPOP_EL1_RES1                               (UL(0))
#define MDSTEPOP_EL1_UNKN                               (UL(0))

#define REG_OSECCR_EL1                                  S2_0_C0_C6_2
#define SYS_OSECCR_EL1                                  sys_reg(2, 0, 0, 6, 2)
#define SYS_OSECCR_EL1_Op0                              2
#define SYS_OSECCR_EL1_Op1                              0
#define SYS_OSECCR_EL1_CRn                              0
#define SYS_OSECCR_EL1_CRm                              6
#define SYS_OSECCR_EL1_Op2                              2

#define OSECCR_EL1_EDECCR                               GENMASK(31, 0)
#define OSECCR_EL1_EDECCR_MASK                          GENMASK(31, 0)
#define OSECCR_EL1_EDECCR_SHIFT                         0
#define OSECCR_EL1_EDECCR_WIDTH                         32

#define OSECCR_EL1_RES0                                 (UL(0) | GENMASK_ULL(63, 32))
#define OSECCR_EL1_RES1                                 (UL(0))
#define OSECCR_EL1_UNKN                                 (UL(0))

#define REG_OSLAR_EL1                                   S2_0_C1_C0_4
#define SYS_OSLAR_EL1                                   sys_reg(2, 0, 1, 0, 4)
#define SYS_OSLAR_EL1_Op0                               2
#define SYS_OSLAR_EL1_Op1                               0
#define SYS_OSLAR_EL1_CRn                               1
#define SYS_OSLAR_EL1_CRm                               0
#define SYS_OSLAR_EL1_Op2                               4

#define OSLAR_EL1_OSLK                                  GENMASK(0, 0)
#define OSLAR_EL1_OSLK_MASK                             GENMASK(0, 0)
#define OSLAR_EL1_OSLK_SHIFT                            0
#define OSLAR_EL1_OSLK_WIDTH                            1

#define OSLAR_EL1_RES0                                  (UL(0) | GENMASK_ULL(63, 1))
#define OSLAR_EL1_RES1                                  (UL(0))
#define OSLAR_EL1_UNKN                                  (UL(0))

#define REG_SPMACCESSR_EL1                              S2_0_C9_C13_3
#define SYS_SPMACCESSR_EL1                              sys_reg(2, 0, 9, 13, 3)
#define SYS_SPMACCESSR_EL1_Op0                          2
#define SYS_SPMACCESSR_EL1_Op1                          0
#define SYS_SPMACCESSR_EL1_CRn                          9
#define SYS_SPMACCESSR_EL1_CRm                          13
#define SYS_SPMACCESSR_EL1_Op2                          3

#define SPMACCESSR_EL1_P31                              GENMASK(63, 62)
#define SPMACCESSR_EL1_P31_MASK                         GENMASK(63, 62)
#define SPMACCESSR_EL1_P31_SHIFT                        62
#define SPMACCESSR_EL1_P31_WIDTH                        2
#define SPMACCESSR_EL1_P31_SIGNED                       false
#define SPMACCESSR_EL1_P31_TRAP_RW                      UL(0b00)
#define SPMACCESSR_EL1_P31_TRAP_W                       UL(0b01)
#define SPMACCESSR_EL1_P31_NOTRAP                       UL(0b11)

#define SPMACCESSR_EL1_P30                              GENMASK(61, 60)
#define SPMACCESSR_EL1_P30_MASK                         GENMASK(61, 60)
#define SPMACCESSR_EL1_P30_SHIFT                        60
#define SPMACCESSR_EL1_P30_WIDTH                        2
#define SPMACCESSR_EL1_P30_SIGNED                       false
#define SPMACCESSR_EL1_P30_TRAP_RW                      UL(0b00)
#define SPMACCESSR_EL1_P30_TRAP_W                       UL(0b01)
#define SPMACCESSR_EL1_P30_NOTRAP                       UL(0b11)

#define SPMACCESSR_EL1_P29                              GENMASK(59, 58)
#define SPMACCESSR_EL1_P29_MASK                         GENMASK(59, 58)
#define SPMACCESSR_EL1_P29_SHIFT                        58
#define SPMACCESSR_EL1_P29_WIDTH                        2
#define SPMACCESSR_EL1_P29_SIGNED                       false
#define SPMACCESSR_EL1_P29_TRAP_RW                      UL(0b00)
#define SPMACCESSR_EL1_P29_TRAP_W                       UL(0b01)
#define SPMACCESSR_EL1_P29_NOTRAP                       UL(0b11)

#define SPMACCESSR_EL1_P28                              GENMASK(57, 56)
#define SPMACCESSR_EL1_P28_MASK                         GENMASK(57, 56)
#define SPMACCESSR_EL1_P28_SHIFT                        56
#define SPMACCESSR_EL1_P28_WIDTH                        2
#define SPMACCESSR_EL1_P28_SIGNED                       false
#define SPMACCESSR_EL1_P28_TRAP_RW                      UL(0b00)
#define SPMACCESSR_EL1_P28_TRAP_W                       UL(0b01)
#define SPMACCESSR_EL1_P28_NOTRAP                       UL(0b11)

#define SPMACCESSR_EL1_P27                              GENMASK(55, 54)
#define SPMACCESSR_EL1_P27_MASK                         GENMASK(55, 54)
#define SPMACCESSR_EL1_P27_SHIFT                        54
#define SPMACCESSR_EL1_P27_WIDTH                        2
#define SPMACCESSR_EL1_P27_SIGNED                       false
#define SPMACCESSR_EL1_P27_TRAP_RW                      UL(0b00)
#define SPMACCESSR_EL1_P27_TRAP_W                       UL(0b01)
#define SPMACCESSR_EL1_P27_NOTRAP                       UL(0b11)

#define SPMACCESSR_EL1_P26                              GENMASK(53, 52)
#define SPMACCESSR_EL1_P26_MASK                         GENMASK(53, 52)
#define SPMACCESSR_EL1_P26_SHIFT                        52
#define SPMACCESSR_EL1_P26_WIDTH                        2
#define SPMACCESSR_EL1_P26_SIGNED                       false
#define SPMACCESSR_EL1_P26_TRAP_RW                      UL(0b00)
#define SPMACCESSR_EL1_P26_TRAP_W                       UL(0b01)
#define SPMACCESSR_EL1_P26_NOTRAP                       UL(0b11)

#define SPMACCESSR_EL1_P25                              GENMASK(51, 50)
#define SPMACCESSR_EL1_P25_MASK                         GENMASK(51, 50)
#define SPMACCESSR_EL1_P25_SHIFT                        50
#define SPMACCESSR_EL1_P25_WIDTH                        2
#define SPMACCESSR_EL1_P25_SIGNED                       false
#define SPMACCESSR_EL1_P25_TRAP_RW                      UL(0b00)
#define SPMACCESSR_EL1_P25_TRAP_W                       UL(0b01)
#define SPMACCESSR_EL1_P25_NOTRAP                       UL(0b11)

#define SPMACCESSR_EL1_P24                              GENMASK(49, 48)
#define SPMACCESSR_EL1_P24_MASK                         GENMASK(49, 48)
#define SPMACCESSR_EL1_P24_SHIFT                        48
#define SPMACCESSR_EL1_P24_WIDTH                        2
#define SPMACCESSR_EL1_P24_SIGNED                       false
#define SPMACCESSR_EL1_P24_TRAP_RW                      UL(0b00)
#define SPMACCESSR_EL1_P24_TRAP_W                       UL(0b01)
#define SPMACCESSR_EL1_P24_NOTRAP                       UL(0b11)

#define SPMACCESSR_EL1_P23                              GENMASK(47, 46)
#define SPMACCESSR_EL1_P23_MASK                         GENMASK(47, 46)
#define SPMACCESSR_EL1_P23_SHIFT                        46
#define SPMACCESSR_EL1_P23_WIDTH                        2
#define SPMACCESSR_EL1_P23_SIGNED                       false
#define SPMACCESSR_EL1_P23_TRAP_RW                      UL(0b00)
#define SPMACCESSR_EL1_P23_TRAP_W                       UL(0b01)
#define SPMACCESSR_EL1_P23_NOTRAP                       UL(0b11)

#define SPMACCESSR_EL1_P22                              GENMASK(45, 44)
#define SPMACCESSR_EL1_P22_MASK                         GENMASK(45, 44)
#define SPMACCESSR_EL1_P22_SHIFT                        44
#define SPMACCESSR_EL1_P22_WIDTH                        2
#define SPMACCESSR_EL1_P22_SIGNED                       false
#define SPMACCESSR_EL1_P22_TRAP_RW                      UL(0b00)
#define SPMACCESSR_EL1_P22_TRAP_W                       UL(0b01)
#define SPMACCESSR_EL1_P22_NOTRAP                       UL(0b11)

#define SPMACCESSR_EL1_P21                              GENMASK(43, 42)
#define SPMACCESSR_EL1_P21_MASK                         GENMASK(43, 42)
#define SPMACCESSR_EL1_P21_SHIFT                        42
#define SPMACCESSR_EL1_P21_WIDTH                        2
#define SPMACCESSR_EL1_P21_SIGNED                       false
#define SPMACCESSR_EL1_P21_TRAP_RW                      UL(0b00)
#define SPMACCESSR_EL1_P21_TRAP_W                       UL(0b01)
#define SPMACCESSR_EL1_P21_NOTRAP                       UL(0b11)

#define SPMACCESSR_EL1_P20                              GENMASK(41, 40)
#define SPMACCESSR_EL1_P20_MASK                         GENMASK(41, 40)
#define SPMACCESSR_EL1_P20_SHIFT                        40
#define SPMACCESSR_EL1_P20_WIDTH                        2
#define SPMACCESSR_EL1_P20_SIGNED                       false
#define SPMACCESSR_EL1_P20_TRAP_RW                      UL(0b00)
#define SPMACCESSR_EL1_P20_TRAP_W                       UL(0b01)
#define SPMACCESSR_EL1_P20_NOTRAP                       UL(0b11)

#define SPMACCESSR_EL1_P19                              GENMASK(39, 38)
#define SPMACCESSR_EL1_P19_MASK                         GENMASK(39, 38)
#define SPMACCESSR_EL1_P19_SHIFT                        38
#define SPMACCESSR_EL1_P19_WIDTH                        2
#define SPMACCESSR_EL1_P19_SIGNED                       false
#define SPMACCESSR_EL1_P19_TRAP_RW                      UL(0b00)
#define SPMACCESSR_EL1_P19_TRAP_W                       UL(0b01)
#define SPMACCESSR_EL1_P19_NOTRAP                       UL(0b11)

#define SPMACCESSR_EL1_P18                              GENMASK(37, 36)
#define SPMACCESSR_EL1_P18_MASK                         GENMASK(37, 36)
#define SPMACCESSR_EL1_P18_SHIFT                        36
#define SPMACCESSR_EL1_P18_WIDTH                        2
#define SPMACCESSR_EL1_P18_SIGNED                       false
#define SPMACCESSR_EL1_P18_TRAP_RW                      UL(0b00)
#define SPMACCESSR_EL1_P18_TRAP_W                       UL(0b01)
#define SPMACCESSR_EL1_P18_NOTRAP                       UL(0b11)

#define SPMACCESSR_EL1_P17                              GENMASK(35, 34)
#define SPMACCESSR_EL1_P17_MASK                         GENMASK(35, 34)
#define SPMACCESSR_EL1_P17_SHIFT                        34
#define SPMACCESSR_EL1_P17_WIDTH                        2
#define SPMACCESSR_EL1_P17_SIGNED                       false
#define SPMACCESSR_EL1_P17_TRAP_RW                      UL(0b00)
#define SPMACCESSR_EL1_P17_TRAP_W                       UL(0b01)
#define SPMACCESSR_EL1_P17_NOTRAP                       UL(0b11)

#define SPMACCESSR_EL1_P16                              GENMASK(33, 32)
#define SPMACCESSR_EL1_P16_MASK                         GENMASK(33, 32)
#define SPMACCESSR_EL1_P16_SHIFT                        32
#define SPMACCESSR_EL1_P16_WIDTH                        2
#define SPMACCESSR_EL1_P16_SIGNED                       false
#define SPMACCESSR_EL1_P16_TRAP_RW                      UL(0b00)
#define SPMACCESSR_EL1_P16_TRAP_W                       UL(0b01)
#define SPMACCESSR_EL1_P16_NOTRAP                       UL(0b11)

#define SPMACCESSR_EL1_P15                              GENMASK(31, 30)
#define SPMACCESSR_EL1_P15_MASK                         GENMASK(31, 30)
#define SPMACCESSR_EL1_P15_SHIFT                        30
#define SPMACCESSR_EL1_P15_WIDTH                        2
#define SPMACCESSR_EL1_P15_SIGNED                       false
#define SPMACCESSR_EL1_P15_TRAP_RW                      UL(0b00)
#define SPMACCESSR_EL1_P15_TRAP_W                       UL(0b01)
#define SPMACCESSR_EL1_P15_NOTRAP                       UL(0b11)

#define SPMACCESSR_EL1_P14                              GENMASK(29, 28)
#define SPMACCESSR_EL1_P14_MASK                         GENMASK(29, 28)
#define SPMACCESSR_EL1_P14_SHIFT                        28
#define SPMACCESSR_EL1_P14_WIDTH                        2
#define SPMACCESSR_EL1_P14_SIGNED                       false
#define SPMACCESSR_EL1_P14_TRAP_RW                      UL(0b00)
#define SPMACCESSR_EL1_P14_TRAP_W                       UL(0b01)
#define SPMACCESSR_EL1_P14_NOTRAP                       UL(0b11)

#define SPMACCESSR_EL1_P13                              GENMASK(27, 26)
#define SPMACCESSR_EL1_P13_MASK                         GENMASK(27, 26)
#define SPMACCESSR_EL1_P13_SHIFT                        26
#define SPMACCESSR_EL1_P13_WIDTH                        2
#define SPMACCESSR_EL1_P13_SIGNED                       false
#define SPMACCESSR_EL1_P13_TRAP_RW                      UL(0b00)
#define SPMACCESSR_EL1_P13_TRAP_W                       UL(0b01)
#define SPMACCESSR_EL1_P13_NOTRAP                       UL(0b11)

#define SPMACCESSR_EL1_P12                              GENMASK(25, 24)
#define SPMACCESSR_EL1_P12_MASK                         GENMASK(25, 24)
#define SPMACCESSR_EL1_P12_SHIFT                        24
#define SPMACCESSR_EL1_P12_WIDTH                        2
#define SPMACCESSR_EL1_P12_SIGNED                       false
#define SPMACCESSR_EL1_P12_TRAP_RW                      UL(0b00)
#define SPMACCESSR_EL1_P12_TRAP_W                       UL(0b01)
#define SPMACCESSR_EL1_P12_NOTRAP                       UL(0b11)

#define SPMACCESSR_EL1_P11                              GENMASK(23, 22)
#define SPMACCESSR_EL1_P11_MASK                         GENMASK(23, 22)
#define SPMACCESSR_EL1_P11_SHIFT                        22
#define SPMACCESSR_EL1_P11_WIDTH                        2
#define SPMACCESSR_EL1_P11_SIGNED                       false
#define SPMACCESSR_EL1_P11_TRAP_RW                      UL(0b00)
#define SPMACCESSR_EL1_P11_TRAP_W                       UL(0b01)
#define SPMACCESSR_EL1_P11_NOTRAP                       UL(0b11)

#define SPMACCESSR_EL1_P10                              GENMASK(21, 20)
#define SPMACCESSR_EL1_P10_MASK                         GENMASK(21, 20)
#define SPMACCESSR_EL1_P10_SHIFT                        20
#define SPMACCESSR_EL1_P10_WIDTH                        2
#define SPMACCESSR_EL1_P10_SIGNED                       false
#define SPMACCESSR_EL1_P10_TRAP_RW                      UL(0b00)
#define SPMACCESSR_EL1_P10_TRAP_W                       UL(0b01)
#define SPMACCESSR_EL1_P10_NOTRAP                       UL(0b11)

#define SPMACCESSR_EL1_P9                               GENMASK(19, 18)
#define SPMACCESSR_EL1_P9_MASK                          GENMASK(19, 18)
#define SPMACCESSR_EL1_P9_SHIFT                         18
#define SPMACCESSR_EL1_P9_WIDTH                         2
#define SPMACCESSR_EL1_P9_SIGNED                        false
#define SPMACCESSR_EL1_P9_TRAP_RW                       UL(0b00)
#define SPMACCESSR_EL1_P9_TRAP_W                        UL(0b01)
#define SPMACCESSR_EL1_P9_NOTRAP                        UL(0b11)

#define SPMACCESSR_EL1_P8                               GENMASK(17, 16)
#define SPMACCESSR_EL1_P8_MASK                          GENMASK(17, 16)
#define SPMACCESSR_EL1_P8_SHIFT                         16
#define SPMACCESSR_EL1_P8_WIDTH                         2
#define SPMACCESSR_EL1_P8_SIGNED                        false
#define SPMACCESSR_EL1_P8_TRAP_RW                       UL(0b00)
#define SPMACCESSR_EL1_P8_TRAP_W                        UL(0b01)
#define SPMACCESSR_EL1_P8_NOTRAP                        UL(0b11)

#define SPMACCESSR_EL1_P7                               GENMASK(15, 14)
#define SPMACCESSR_EL1_P7_MASK                          GENMASK(15, 14)
#define SPMACCESSR_EL1_P7_SHIFT                         14
#define SPMACCESSR_EL1_P7_WIDTH                         2
#define SPMACCESSR_EL1_P7_SIGNED                        false
#define SPMACCESSR_EL1_P7_TRAP_RW                       UL(0b00)
#define SPMACCESSR_EL1_P7_TRAP_W                        UL(0b01)
#define SPMACCESSR_EL1_P7_NOTRAP                        UL(0b11)

#define SPMACCESSR_EL1_P6                               GENMASK(13, 12)
#define SPMACCESSR_EL1_P6_MASK                          GENMASK(13, 12)
#define SPMACCESSR_EL1_P6_SHIFT                         12
#define SPMACCESSR_EL1_P6_WIDTH                         2
#define SPMACCESSR_EL1_P6_SIGNED                        false
#define SPMACCESSR_EL1_P6_TRAP_RW                       UL(0b00)
#define SPMACCESSR_EL1_P6_TRAP_W                        UL(0b01)
#define SPMACCESSR_EL1_P6_NOTRAP                        UL(0b11)

#define SPMACCESSR_EL1_P5                               GENMASK(11, 10)
#define SPMACCESSR_EL1_P5_MASK                          GENMASK(11, 10)
#define SPMACCESSR_EL1_P5_SHIFT                         10
#define SPMACCESSR_EL1_P5_WIDTH                         2
#define SPMACCESSR_EL1_P5_SIGNED                        false
#define SPMACCESSR_EL1_P5_TRAP_RW                       UL(0b00)
#define SPMACCESSR_EL1_P5_TRAP_W                        UL(0b01)
#define SPMACCESSR_EL1_P5_NOTRAP                        UL(0b11)

#define SPMACCESSR_EL1_P4                               GENMASK(9, 8)
#define SPMACCESSR_EL1_P4_MASK                          GENMASK(9, 8)
#define SPMACCESSR_EL1_P4_SHIFT                         8
#define SPMACCESSR_EL1_P4_WIDTH                         2
#define SPMACCESSR_EL1_P4_SIGNED                        false
#define SPMACCESSR_EL1_P4_TRAP_RW                       UL(0b00)
#define SPMACCESSR_EL1_P4_TRAP_W                        UL(0b01)
#define SPMACCESSR_EL1_P4_NOTRAP                        UL(0b11)

#define SPMACCESSR_EL1_P3                               GENMASK(7, 6)
#define SPMACCESSR_EL1_P3_MASK                          GENMASK(7, 6)
#define SPMACCESSR_EL1_P3_SHIFT                         6
#define SPMACCESSR_EL1_P3_WIDTH                         2
#define SPMACCESSR_EL1_P3_SIGNED                        false
#define SPMACCESSR_EL1_P3_TRAP_RW                       UL(0b00)
#define SPMACCESSR_EL1_P3_TRAP_W                        UL(0b01)
#define SPMACCESSR_EL1_P3_NOTRAP                        UL(0b11)

#define SPMACCESSR_EL1_P2                               GENMASK(5, 4)
#define SPMACCESSR_EL1_P2_MASK                          GENMASK(5, 4)
#define SPMACCESSR_EL1_P2_SHIFT                         4
#define SPMACCESSR_EL1_P2_WIDTH                         2
#define SPMACCESSR_EL1_P2_SIGNED                        false
#define SPMACCESSR_EL1_P2_TRAP_RW                       UL(0b00)
#define SPMACCESSR_EL1_P2_TRAP_W                        UL(0b01)
#define SPMACCESSR_EL1_P2_NOTRAP                        UL(0b11)

#define SPMACCESSR_EL1_P1                               GENMASK(3, 2)
#define SPMACCESSR_EL1_P1_MASK                          GENMASK(3, 2)
#define SPMACCESSR_EL1_P1_SHIFT                         2
#define SPMACCESSR_EL1_P1_WIDTH                         2
#define SPMACCESSR_EL1_P1_SIGNED                        false
#define SPMACCESSR_EL1_P1_TRAP_RW                       UL(0b00)
#define SPMACCESSR_EL1_P1_TRAP_W                        UL(0b01)
#define SPMACCESSR_EL1_P1_NOTRAP                        UL(0b11)

#define SPMACCESSR_EL1_P0                               GENMASK(1, 0)
#define SPMACCESSR_EL1_P0_MASK                          GENMASK(1, 0)
#define SPMACCESSR_EL1_P0_SHIFT                         0
#define SPMACCESSR_EL1_P0_WIDTH                         2
#define SPMACCESSR_EL1_P0_SIGNED                        false
#define SPMACCESSR_EL1_P0_TRAP_RW                       UL(0b00)
#define SPMACCESSR_EL1_P0_TRAP_W                        UL(0b01)
#define SPMACCESSR_EL1_P0_NOTRAP                        UL(0b11)

#define SPMACCESSR_EL1_RES0                             (UL(0))
#define SPMACCESSR_EL1_RES1                             (UL(0))
#define SPMACCESSR_EL1_UNKN                             (UL(0))

#define REG_SPMACCESSR_EL12                             S2_5_C9_C13_3
#define SYS_SPMACCESSR_EL12                             sys_reg(2, 5, 9, 13, 3)
#define SYS_SPMACCESSR_EL12_Op0                         2
#define SYS_SPMACCESSR_EL12_Op1                         5
#define SYS_SPMACCESSR_EL12_CRn                         9
#define SYS_SPMACCESSR_EL12_CRm                         13
#define SYS_SPMACCESSR_EL12_Op2                         3

/* For SPMACCESSR_EL12 fields see SPMACCESSR_EL1 */

#define REG_SPMIIDR_EL1                                 S2_0_C9_C13_4
#define SYS_SPMIIDR_EL1                                 sys_reg(2, 0, 9, 13, 4)
#define SYS_SPMIIDR_EL1_Op0                             2
#define SYS_SPMIIDR_EL1_Op1                             0
#define SYS_SPMIIDR_EL1_CRn                             9
#define SYS_SPMIIDR_EL1_CRm                             13
#define SYS_SPMIIDR_EL1_Op2                             4

#define SPMIIDR_EL1_ProductID                           GENMASK(31, 20)
#define SPMIIDR_EL1_ProductID_MASK                      GENMASK(31, 20)
#define SPMIIDR_EL1_ProductID_SHIFT                     20
#define SPMIIDR_EL1_ProductID_WIDTH                     12

#define SPMIIDR_EL1_Variant                             GENMASK(19, 16)
#define SPMIIDR_EL1_Variant_MASK                        GENMASK(19, 16)
#define SPMIIDR_EL1_Variant_SHIFT                       16
#define SPMIIDR_EL1_Variant_WIDTH                       4

#define SPMIIDR_EL1_Revision                            GENMASK(15, 12)
#define SPMIIDR_EL1_Revision_MASK                       GENMASK(15, 12)
#define SPMIIDR_EL1_Revision_SHIFT                      12
#define SPMIIDR_EL1_Revision_WIDTH                      4

#define SPMIIDR_EL1_Implementer                         GENMASK(11, 0)
#define SPMIIDR_EL1_Implementer_MASK                    GENMASK(11, 0)
#define SPMIIDR_EL1_Implementer_SHIFT                   0
#define SPMIIDR_EL1_Implementer_WIDTH                   12

#define SPMIIDR_EL1_RES0                                (UL(0) | GENMASK_ULL(63, 32))
#define SPMIIDR_EL1_RES1                                (UL(0))
#define SPMIIDR_EL1_UNKN                                (UL(0))

#define REG_SPMDEVARCH_EL1                              S2_0_C9_C13_5
#define SYS_SPMDEVARCH_EL1                              sys_reg(2, 0, 9, 13, 5)
#define SYS_SPMDEVARCH_EL1_Op0                          2
#define SYS_SPMDEVARCH_EL1_Op1                          0
#define SYS_SPMDEVARCH_EL1_CRn                          9
#define SYS_SPMDEVARCH_EL1_CRm                          13
#define SYS_SPMDEVARCH_EL1_Op2                          5

#define SPMDEVARCH_EL1_ARCHITECT                        GENMASK(31, 21)
#define SPMDEVARCH_EL1_ARCHITECT_MASK                   GENMASK(31, 21)
#define SPMDEVARCH_EL1_ARCHITECT_SHIFT                  21
#define SPMDEVARCH_EL1_ARCHITECT_WIDTH                  11

#define SPMDEVARCH_EL1_PRESENT                          GENMASK(20, 20)
#define SPMDEVARCH_EL1_PRESENT_MASK                     GENMASK(20, 20)
#define SPMDEVARCH_EL1_PRESENT_SHIFT                    20
#define SPMDEVARCH_EL1_PRESENT_WIDTH                    1

#define SPMDEVARCH_EL1_REVISION                         GENMASK(19, 16)
#define SPMDEVARCH_EL1_REVISION_MASK                    GENMASK(19, 16)
#define SPMDEVARCH_EL1_REVISION_SHIFT                   16
#define SPMDEVARCH_EL1_REVISION_WIDTH                   4

#define SPMDEVARCH_EL1_ARCHVER                          GENMASK(15, 12)
#define SPMDEVARCH_EL1_ARCHVER_MASK                     GENMASK(15, 12)
#define SPMDEVARCH_EL1_ARCHVER_SHIFT                    12
#define SPMDEVARCH_EL1_ARCHVER_WIDTH                    4

#define SPMDEVARCH_EL1_ARCHPART                         GENMASK(11, 0)
#define SPMDEVARCH_EL1_ARCHPART_MASK                    GENMASK(11, 0)
#define SPMDEVARCH_EL1_ARCHPART_SHIFT                   0
#define SPMDEVARCH_EL1_ARCHPART_WIDTH                   12

#define SPMDEVARCH_EL1_RES0                             (UL(0) | GENMASK_ULL(63, 32))
#define SPMDEVARCH_EL1_RES1                             (UL(0))
#define SPMDEVARCH_EL1_UNKN                             (UL(0))

#define REG_SPMDEVAFF_EL1                               S2_0_C9_C13_6
#define SYS_SPMDEVAFF_EL1                               sys_reg(2, 0, 9, 13, 6)
#define SYS_SPMDEVAFF_EL1_Op0                           2
#define SYS_SPMDEVAFF_EL1_Op1                           0
#define SYS_SPMDEVAFF_EL1_CRn                           9
#define SYS_SPMDEVAFF_EL1_CRm                           13
#define SYS_SPMDEVAFF_EL1_Op2                           6

#define SPMDEVAFF_EL1_Aff3                              GENMASK(39, 32)
#define SPMDEVAFF_EL1_Aff3_MASK                         GENMASK(39, 32)
#define SPMDEVAFF_EL1_Aff3_SHIFT                        32
#define SPMDEVAFF_EL1_Aff3_WIDTH                        8

#define SPMDEVAFF_EL1_F0V                               GENMASK(31, 31)
#define SPMDEVAFF_EL1_F0V_MASK                          GENMASK(31, 31)
#define SPMDEVAFF_EL1_F0V_SHIFT                         31
#define SPMDEVAFF_EL1_F0V_WIDTH                         1

#define SPMDEVAFF_EL1_U                                 GENMASK(30, 30)
#define SPMDEVAFF_EL1_U_MASK                            GENMASK(30, 30)
#define SPMDEVAFF_EL1_U_SHIFT                           30
#define SPMDEVAFF_EL1_U_WIDTH                           1

#define SPMDEVAFF_EL1_MT                                GENMASK(24, 24)
#define SPMDEVAFF_EL1_MT_MASK                           GENMASK(24, 24)
#define SPMDEVAFF_EL1_MT_SHIFT                          24
#define SPMDEVAFF_EL1_MT_WIDTH                          1

#define SPMDEVAFF_EL1_Aff2                              GENMASK(23, 16)
#define SPMDEVAFF_EL1_Aff2_MASK                         GENMASK(23, 16)
#define SPMDEVAFF_EL1_Aff2_SHIFT                        16
#define SPMDEVAFF_EL1_Aff2_WIDTH                        8

#define SPMDEVAFF_EL1_Aff1                              GENMASK(15, 8)
#define SPMDEVAFF_EL1_Aff1_MASK                         GENMASK(15, 8)
#define SPMDEVAFF_EL1_Aff1_SHIFT                        8
#define SPMDEVAFF_EL1_Aff1_WIDTH                        8

#define SPMDEVAFF_EL1_Aff0                              GENMASK(7, 0)
#define SPMDEVAFF_EL1_Aff0_MASK                         GENMASK(7, 0)
#define SPMDEVAFF_EL1_Aff0_SHIFT                        0
#define SPMDEVAFF_EL1_Aff0_WIDTH                        8

#define SPMDEVAFF_EL1_RES0                              (UL(0) | GENMASK_ULL(63, 40) | GENMASK_ULL(29, 25))
#define SPMDEVAFF_EL1_RES1                              (UL(0))
#define SPMDEVAFF_EL1_UNKN                              (UL(0))

#define REG_SPMCFGR_EL1                                 S2_0_C9_C13_7
#define SYS_SPMCFGR_EL1                                 sys_reg(2, 0, 9, 13, 7)
#define SYS_SPMCFGR_EL1_Op0                             2
#define SYS_SPMCFGR_EL1_Op1                             0
#define SYS_SPMCFGR_EL1_CRn                             9
#define SYS_SPMCFGR_EL1_CRm                             13
#define SYS_SPMCFGR_EL1_Op2                             7

#define SPMCFGR_EL1_NCG                                 GENMASK(31, 28)
#define SPMCFGR_EL1_NCG_MASK                            GENMASK(31, 28)
#define SPMCFGR_EL1_NCG_SHIFT                           28
#define SPMCFGR_EL1_NCG_WIDTH                           4

#define SPMCFGR_EL1_HDBG                                GENMASK(24, 24)
#define SPMCFGR_EL1_HDBG_MASK                           GENMASK(24, 24)
#define SPMCFGR_EL1_HDBG_SHIFT                          24
#define SPMCFGR_EL1_HDBG_WIDTH                          1

#define SPMCFGR_EL1_TRO                                 GENMASK(23, 23)
#define SPMCFGR_EL1_TRO_MASK                            GENMASK(23, 23)
#define SPMCFGR_EL1_TRO_SHIFT                           23
#define SPMCFGR_EL1_TRO_WIDTH                           1

#define SPMCFGR_EL1_SS                                  GENMASK(22, 22)
#define SPMCFGR_EL1_SS_MASK                             GENMASK(22, 22)
#define SPMCFGR_EL1_SS_SHIFT                            22
#define SPMCFGR_EL1_SS_WIDTH                            1

#define SPMCFGR_EL1_FZO                                 GENMASK(21, 21)
#define SPMCFGR_EL1_FZO_MASK                            GENMASK(21, 21)
#define SPMCFGR_EL1_FZO_SHIFT                           21
#define SPMCFGR_EL1_FZO_WIDTH                           1

#define SPMCFGR_EL1_MSI                                 GENMASK(20, 20)
#define SPMCFGR_EL1_MSI_MASK                            GENMASK(20, 20)
#define SPMCFGR_EL1_MSI_SHIFT                           20
#define SPMCFGR_EL1_MSI_WIDTH                           1

#define SPMCFGR_EL1_RAO                                 GENMASK(19, 19)
#define SPMCFGR_EL1_RAO_MASK                            GENMASK(19, 19)
#define SPMCFGR_EL1_RAO_SHIFT                           19
#define SPMCFGR_EL1_RAO_WIDTH                           1

#define SPMCFGR_EL1_NA                                  GENMASK(17, 17)
#define SPMCFGR_EL1_NA_MASK                             GENMASK(17, 17)
#define SPMCFGR_EL1_NA_SHIFT                            17
#define SPMCFGR_EL1_NA_WIDTH                            1

#define SPMCFGR_EL1_EX                                  GENMASK(16, 16)
#define SPMCFGR_EL1_EX_MASK                             GENMASK(16, 16)
#define SPMCFGR_EL1_EX_SHIFT                            16
#define SPMCFGR_EL1_EX_WIDTH                            1

#define SPMCFGR_EL1_RAZ                                 GENMASK(15, 14)
#define SPMCFGR_EL1_RAZ_MASK                            GENMASK(15, 14)
#define SPMCFGR_EL1_RAZ_SHIFT                           14
#define SPMCFGR_EL1_RAZ_WIDTH                           2

#define SPMCFGR_EL1_SIZE                                GENMASK(13, 8)
#define SPMCFGR_EL1_SIZE_MASK                           GENMASK(13, 8)
#define SPMCFGR_EL1_SIZE_SHIFT                          8
#define SPMCFGR_EL1_SIZE_WIDTH                          6

#define SPMCFGR_EL1_N                                   GENMASK(7, 0)
#define SPMCFGR_EL1_N_MASK                              GENMASK(7, 0)
#define SPMCFGR_EL1_N_SHIFT                             0
#define SPMCFGR_EL1_N_WIDTH                             8

#define SPMCFGR_EL1_RES0                                (UL(0) | GENMASK_ULL(63, 32) | GENMASK_ULL(27, 25) | GENMASK_ULL(18, 18))
#define SPMCFGR_EL1_RES1                                (UL(0))
#define SPMCFGR_EL1_UNKN                                (UL(0))

#define REG_SPMINTENSET_EL1                             S2_0_C9_C14_1
#define SYS_SPMINTENSET_EL1                             sys_reg(2, 0, 9, 14, 1)
#define SYS_SPMINTENSET_EL1_Op0                         2
#define SYS_SPMINTENSET_EL1_Op1                         0
#define SYS_SPMINTENSET_EL1_CRn                         9
#define SYS_SPMINTENSET_EL1_CRm                         14
#define SYS_SPMINTENSET_EL1_Op2                         1

#define SPMINTENSET_EL1_P                               GENMASK(63, 0)
#define SPMINTENSET_EL1_P_MASK                          GENMASK(63, 0)
#define SPMINTENSET_EL1_P_SHIFT                         0
#define SPMINTENSET_EL1_P_WIDTH                         64

#define SPMINTENSET_EL1_RES0                            (UL(0))
#define SPMINTENSET_EL1_RES1                            (UL(0))
#define SPMINTENSET_EL1_UNKN                            (UL(0))

#define REG_SPMINTENCLR_EL1                             S2_0_C9_C14_2
#define SYS_SPMINTENCLR_EL1                             sys_reg(2, 0, 9, 14, 2)
#define SYS_SPMINTENCLR_EL1_Op0                         2
#define SYS_SPMINTENCLR_EL1_Op1                         0
#define SYS_SPMINTENCLR_EL1_CRn                         9
#define SYS_SPMINTENCLR_EL1_CRm                         14
#define SYS_SPMINTENCLR_EL1_Op2                         2

#define SPMINTENCLR_EL1_P                               GENMASK(63, 0)
#define SPMINTENCLR_EL1_P_MASK                          GENMASK(63, 0)
#define SPMINTENCLR_EL1_P_SHIFT                         0
#define SPMINTENCLR_EL1_P_WIDTH                         64

#define SPMINTENCLR_EL1_RES0                            (UL(0))
#define SPMINTENCLR_EL1_RES1                            (UL(0))
#define SPMINTENCLR_EL1_UNKN                            (UL(0))

#define REG_PMCCNTSVR_EL1                               S2_0_C14_C11_7
#define SYS_PMCCNTSVR_EL1                               sys_reg(2, 0, 14, 11, 7)
#define SYS_PMCCNTSVR_EL1_Op0                           2
#define SYS_PMCCNTSVR_EL1_Op1                           0
#define SYS_PMCCNTSVR_EL1_CRn                           14
#define SYS_PMCCNTSVR_EL1_CRm                           11
#define SYS_PMCCNTSVR_EL1_Op2                           7

#define PMCCNTSVR_EL1_CCNT                              GENMASK(63, 0)
#define PMCCNTSVR_EL1_CCNT_MASK                         GENMASK(63, 0)
#define PMCCNTSVR_EL1_CCNT_SHIFT                        0
#define PMCCNTSVR_EL1_CCNT_WIDTH                        64

#define PMCCNTSVR_EL1_RES0                              (UL(0))
#define PMCCNTSVR_EL1_RES1                              (UL(0))
#define PMCCNTSVR_EL1_UNKN                              (UL(0))

#define REG_PMICNTSVR_EL1                               S2_0_C14_C12_0
#define SYS_PMICNTSVR_EL1                               sys_reg(2, 0, 14, 12, 0)
#define SYS_PMICNTSVR_EL1_Op0                           2
#define SYS_PMICNTSVR_EL1_Op1                           0
#define SYS_PMICNTSVR_EL1_CRn                           14
#define SYS_PMICNTSVR_EL1_CRm                           12
#define SYS_PMICNTSVR_EL1_Op2                           0

#define PMICNTSVR_EL1_ICNT                              GENMASK(63, 0)
#define PMICNTSVR_EL1_ICNT_MASK                         GENMASK(63, 0)
#define PMICNTSVR_EL1_ICNT_SHIFT                        0
#define PMICNTSVR_EL1_ICNT_WIDTH                        64

#define PMICNTSVR_EL1_RES0                              (UL(0))
#define PMICNTSVR_EL1_RES1                              (UL(0))
#define PMICNTSVR_EL1_UNKN                              (UL(0))

#define REG_SPMCR_EL0                                   S2_3_C9_C12_0
#define SYS_SPMCR_EL0                                   sys_reg(2, 3, 9, 12, 0)
#define SYS_SPMCR_EL0_Op0                               2
#define SYS_SPMCR_EL0_Op1                               3
#define SYS_SPMCR_EL0_CRn                               9
#define SYS_SPMCR_EL0_CRm                               12
#define SYS_SPMCR_EL0_Op2                               0

#define SPMCR_EL0_TRO                                   GENMASK(11, 11)
#define SPMCR_EL0_TRO_MASK                              GENMASK(11, 11)
#define SPMCR_EL0_TRO_SHIFT                             11
#define SPMCR_EL0_TRO_WIDTH                             1

#define SPMCR_EL0_HDBG                                  GENMASK(10, 10)
#define SPMCR_EL0_HDBG_MASK                             GENMASK(10, 10)
#define SPMCR_EL0_HDBG_SHIFT                            10
#define SPMCR_EL0_HDBG_WIDTH                            1

#define SPMCR_EL0_FZO                                   GENMASK(9, 9)
#define SPMCR_EL0_FZO_MASK                              GENMASK(9, 9)
#define SPMCR_EL0_FZO_SHIFT                             9
#define SPMCR_EL0_FZO_WIDTH                             1

#define SPMCR_EL0_NA                                    GENMASK(8, 8)
#define SPMCR_EL0_NA_MASK                               GENMASK(8, 8)
#define SPMCR_EL0_NA_SHIFT                              8
#define SPMCR_EL0_NA_WIDTH                              1

#define SPMCR_EL0_EX                                    GENMASK(4, 4)
#define SPMCR_EL0_EX_MASK                               GENMASK(4, 4)
#define SPMCR_EL0_EX_SHIFT                              4
#define SPMCR_EL0_EX_WIDTH                              1

#define SPMCR_EL0_P                                     GENMASK(1, 1)
#define SPMCR_EL0_P_MASK                                GENMASK(1, 1)
#define SPMCR_EL0_P_SHIFT                               1
#define SPMCR_EL0_P_WIDTH                               1

#define SPMCR_EL0_E                                     GENMASK(0, 0)
#define SPMCR_EL0_E_MASK                                GENMASK(0, 0)
#define SPMCR_EL0_E_SHIFT                               0
#define SPMCR_EL0_E_WIDTH                               1

#define SPMCR_EL0_RES0                                  (UL(0) | GENMASK_ULL(63, 12) | GENMASK_ULL(7, 5) | GENMASK_ULL(3, 2))
#define SPMCR_EL0_RES1                                  (UL(0))
#define SPMCR_EL0_UNKN                                  (UL(0))

#define REG_SPMCNTENSET_EL0                             S2_3_C9_C12_1
#define SYS_SPMCNTENSET_EL0                             sys_reg(2, 3, 9, 12, 1)
#define SYS_SPMCNTENSET_EL0_Op0                         2
#define SYS_SPMCNTENSET_EL0_Op1                         3
#define SYS_SPMCNTENSET_EL0_CRn                         9
#define SYS_SPMCNTENSET_EL0_CRm                         12
#define SYS_SPMCNTENSET_EL0_Op2                         1

#define SPMCNTENSET_EL0_P                               GENMASK(63, 0)
#define SPMCNTENSET_EL0_P_MASK                          GENMASK(63, 0)
#define SPMCNTENSET_EL0_P_SHIFT                         0
#define SPMCNTENSET_EL0_P_WIDTH                         64

#define SPMCNTENSET_EL0_RES0                            (UL(0))
#define SPMCNTENSET_EL0_RES1                            (UL(0))
#define SPMCNTENSET_EL0_UNKN                            (UL(0))

#define REG_SPMCNTENCLR_EL0                             S2_3_C9_C12_2
#define SYS_SPMCNTENCLR_EL0                             sys_reg(2, 3, 9, 12, 2)
#define SYS_SPMCNTENCLR_EL0_Op0                         2
#define SYS_SPMCNTENCLR_EL0_Op1                         3
#define SYS_SPMCNTENCLR_EL0_CRn                         9
#define SYS_SPMCNTENCLR_EL0_CRm                         12
#define SYS_SPMCNTENCLR_EL0_Op2                         2

#define SPMCNTENCLR_EL0_P                               GENMASK(63, 0)
#define SPMCNTENCLR_EL0_P_MASK                          GENMASK(63, 0)
#define SPMCNTENCLR_EL0_P_SHIFT                         0
#define SPMCNTENCLR_EL0_P_WIDTH                         64

#define SPMCNTENCLR_EL0_RES0                            (UL(0))
#define SPMCNTENCLR_EL0_RES1                            (UL(0))
#define SPMCNTENCLR_EL0_UNKN                            (UL(0))

#define REG_SPMOVSCLR_EL0                               S2_3_C9_C12_3
#define SYS_SPMOVSCLR_EL0                               sys_reg(2, 3, 9, 12, 3)
#define SYS_SPMOVSCLR_EL0_Op0                           2
#define SYS_SPMOVSCLR_EL0_Op1                           3
#define SYS_SPMOVSCLR_EL0_CRn                           9
#define SYS_SPMOVSCLR_EL0_CRm                           12
#define SYS_SPMOVSCLR_EL0_Op2                           3

#define SPMOVSCLR_EL0_P                                 GENMASK(63, 0)
#define SPMOVSCLR_EL0_P_MASK                            GENMASK(63, 0)
#define SPMOVSCLR_EL0_P_SHIFT                           0
#define SPMOVSCLR_EL0_P_WIDTH                           64

#define SPMOVSCLR_EL0_RES0                              (UL(0))
#define SPMOVSCLR_EL0_RES1                              (UL(0))
#define SPMOVSCLR_EL0_UNKN                              (UL(0))

#define REG_SPMZR_EL0                                   S2_3_C9_C12_4
#define SYS_SPMZR_EL0                                   sys_reg(2, 3, 9, 12, 4)
#define SYS_SPMZR_EL0_Op0                               2
#define SYS_SPMZR_EL0_Op1                               3
#define SYS_SPMZR_EL0_CRn                               9
#define SYS_SPMZR_EL0_CRm                               12
#define SYS_SPMZR_EL0_Op2                               4

#define SPMZR_EL0_P                                     GENMASK(63, 0)
#define SPMZR_EL0_P_MASK                                GENMASK(63, 0)
#define SPMZR_EL0_P_SHIFT                               0
#define SPMZR_EL0_P_WIDTH                               64

#define SPMZR_EL0_RES0                                  (UL(0))
#define SPMZR_EL0_RES1                                  (UL(0))
#define SPMZR_EL0_UNKN                                  (UL(0))

#define REG_SPMSELR_EL0                                 S2_3_C9_C12_5
#define SYS_SPMSELR_EL0                                 sys_reg(2, 3, 9, 12, 5)
#define SYS_SPMSELR_EL0_Op0                             2
#define SYS_SPMSELR_EL0_Op1                             3
#define SYS_SPMSELR_EL0_CRn                             9
#define SYS_SPMSELR_EL0_CRm                             12
#define SYS_SPMSELR_EL0_Op2                             5

#define SPMSELR_EL0_SYSPMUSEL                           GENMASK(9, 4)
#define SPMSELR_EL0_SYSPMUSEL_MASK                      GENMASK(9, 4)
#define SPMSELR_EL0_SYSPMUSEL_SHIFT                     4
#define SPMSELR_EL0_SYSPMUSEL_WIDTH                     6

#define SPMSELR_EL0_BANK                                GENMASK(1, 0)
#define SPMSELR_EL0_BANK_MASK                           GENMASK(1, 0)
#define SPMSELR_EL0_BANK_SHIFT                          0
#define SPMSELR_EL0_BANK_WIDTH                          2

#define SPMSELR_EL0_RES0                                (UL(0) | GENMASK_ULL(63, 10) | GENMASK_ULL(3, 2))
#define SPMSELR_EL0_RES1                                (UL(0))
#define SPMSELR_EL0_UNKN                                (UL(0))

#define REG_SPMOVSSET_EL0                               S2_3_C9_C14_3
#define SYS_SPMOVSSET_EL0                               sys_reg(2, 3, 9, 14, 3)
#define SYS_SPMOVSSET_EL0_Op0                           2
#define SYS_SPMOVSSET_EL0_Op1                           3
#define SYS_SPMOVSSET_EL0_CRn                           9
#define SYS_SPMOVSSET_EL0_CRm                           14
#define SYS_SPMOVSSET_EL0_Op2                           3

#define SPMOVSSET_EL0_P                                 GENMASK(63, 0)
#define SPMOVSSET_EL0_P_MASK                            GENMASK(63, 0)
#define SPMOVSSET_EL0_P_SHIFT                           0
#define SPMOVSSET_EL0_P_WIDTH                           64

#define SPMOVSSET_EL0_RES0                              (UL(0))
#define SPMOVSSET_EL0_RES1                              (UL(0))
#define SPMOVSSET_EL0_UNKN                              (UL(0))

#define REG_SPMSCR_EL1                                  S2_7_C9_C14_7
#define SYS_SPMSCR_EL1                                  sys_reg(2, 7, 9, 14, 7)
#define SYS_SPMSCR_EL1_Op0                              2
#define SYS_SPMSCR_EL1_Op1                              7
#define SYS_SPMSCR_EL1_CRn                              9
#define SYS_SPMSCR_EL1_CRm                              14
#define SYS_SPMSCR_EL1_Op2                              7

#define SPMSCR_EL1_IMPDEF                               GENMASK(63, 32)
#define SPMSCR_EL1_IMPDEF_MASK                          GENMASK(63, 32)
#define SPMSCR_EL1_IMPDEF_SHIFT                         32
#define SPMSCR_EL1_IMPDEF_WIDTH                         32

#define SPMSCR_EL1_RAO                                  GENMASK(31, 31)
#define SPMSCR_EL1_RAO_MASK                             GENMASK(31, 31)
#define SPMSCR_EL1_RAO_SHIFT                            31
#define SPMSCR_EL1_RAO_WIDTH                            1

#define SPMSCR_EL1_NAO                                  GENMASK(4, 4)
#define SPMSCR_EL1_NAO_MASK                             GENMASK(4, 4)
#define SPMSCR_EL1_NAO_SHIFT                            4
#define SPMSCR_EL1_NAO_WIDTH                            1

#define SPMSCR_EL1_SO                                   GENMASK(0, 0)
#define SPMSCR_EL1_SO_MASK                              GENMASK(0, 0)
#define SPMSCR_EL1_SO_SHIFT                             0
#define SPMSCR_EL1_SO_WIDTH                             1

#define SPMSCR_EL1_RES0                                 (UL(0) | GENMASK_ULL(30, 5) | GENMASK_ULL(3, 1))
#define SPMSCR_EL1_RES1                                 (UL(0))
#define SPMSCR_EL1_UNKN                                 (UL(0))

#define REG_ID_PFR0_EL1                                 S3_0_C0_C1_0
#define SYS_ID_PFR0_EL1                                 sys_reg(3, 0, 0, 1, 0)
#define SYS_ID_PFR0_EL1_Op0                             3
#define SYS_ID_PFR0_EL1_Op1                             0
#define SYS_ID_PFR0_EL1_CRn                             0
#define SYS_ID_PFR0_EL1_CRm                             1
#define SYS_ID_PFR0_EL1_Op2                             0

#define ID_PFR0_EL1_RAS                                 GENMASK(31, 28)
#define ID_PFR0_EL1_RAS_MASK                            GENMASK(31, 28)
#define ID_PFR0_EL1_RAS_SHIFT                           28
#define ID_PFR0_EL1_RAS_WIDTH                           4
#define ID_PFR0_EL1_RAS_SIGNED                          false
#define ID_PFR0_EL1_RAS_NI                              UL(0b0000)
#define ID_PFR0_EL1_RAS_RAS                             UL(0b0001)
#define ID_PFR0_EL1_RAS_RASv1p1                         UL(0b0010)

#define ID_PFR0_EL1_DIT                                 GENMASK(27, 24)
#define ID_PFR0_EL1_DIT_MASK                            GENMASK(27, 24)
#define ID_PFR0_EL1_DIT_SHIFT                           24
#define ID_PFR0_EL1_DIT_WIDTH                           4
#define ID_PFR0_EL1_DIT_SIGNED                          false
#define ID_PFR0_EL1_DIT_NI                              UL(0b0000)
#define ID_PFR0_EL1_DIT_IMP                             UL(0b0001)

#define ID_PFR0_EL1_AMU                                 GENMASK(23, 20)
#define ID_PFR0_EL1_AMU_MASK                            GENMASK(23, 20)
#define ID_PFR0_EL1_AMU_SHIFT                           20
#define ID_PFR0_EL1_AMU_WIDTH                           4
#define ID_PFR0_EL1_AMU_SIGNED                          false
#define ID_PFR0_EL1_AMU_NI                              UL(0b0000)
#define ID_PFR0_EL1_AMU_AMUv1                           UL(0b0001)
#define ID_PFR0_EL1_AMU_AMUv1p1                         UL(0b0010)

#define ID_PFR0_EL1_CSV2                                GENMASK(19, 16)
#define ID_PFR0_EL1_CSV2_MASK                           GENMASK(19, 16)
#define ID_PFR0_EL1_CSV2_SHIFT                          16
#define ID_PFR0_EL1_CSV2_WIDTH                          4
#define ID_PFR0_EL1_CSV2_SIGNED                         false
#define ID_PFR0_EL1_CSV2_UNDISCLOSED                    UL(0b0000)
#define ID_PFR0_EL1_CSV2_IMP                            UL(0b0001)
#define ID_PFR0_EL1_CSV2_CSV2p1                         UL(0b0010)

#define ID_PFR0_EL1_State3                              GENMASK(15, 12)
#define ID_PFR0_EL1_State3_MASK                         GENMASK(15, 12)
#define ID_PFR0_EL1_State3_SHIFT                        12
#define ID_PFR0_EL1_State3_WIDTH                        4
#define ID_PFR0_EL1_State3_SIGNED                       false
#define ID_PFR0_EL1_State3_NI                           UL(0b0000)
#define ID_PFR0_EL1_State3_IMP                          UL(0b0001)

#define ID_PFR0_EL1_State2                              GENMASK(11, 8)
#define ID_PFR0_EL1_State2_MASK                         GENMASK(11, 8)
#define ID_PFR0_EL1_State2_SHIFT                        8
#define ID_PFR0_EL1_State2_WIDTH                        4
#define ID_PFR0_EL1_State2_NI                           UL(0b0000)
#define ID_PFR0_EL1_State2_NO_CV                        UL(0b0001)
#define ID_PFR0_EL1_State2_CV                           UL(0b0010)

#define ID_PFR0_EL1_State1                              GENMASK(7, 4)
#define ID_PFR0_EL1_State1_MASK                         GENMASK(7, 4)
#define ID_PFR0_EL1_State1_SHIFT                        4
#define ID_PFR0_EL1_State1_WIDTH                        4
#define ID_PFR0_EL1_State1_SIGNED                       false
#define ID_PFR0_EL1_State1_NI                           UL(0b0000)
#define ID_PFR0_EL1_State1_THUMB                        UL(0b0001)
#define ID_PFR0_EL1_State1_THUMB2                       UL(0b0010)

#define ID_PFR0_EL1_State0                              GENMASK(3, 0)
#define ID_PFR0_EL1_State0_MASK                         GENMASK(3, 0)
#define ID_PFR0_EL1_State0_SHIFT                        0
#define ID_PFR0_EL1_State0_WIDTH                        4
#define ID_PFR0_EL1_State0_SIGNED                       false
#define ID_PFR0_EL1_State0_NI                           UL(0b0000)
#define ID_PFR0_EL1_State0_IMP                          UL(0b0001)

#define ID_PFR0_EL1_RES0                                (UL(0) | GENMASK_ULL(63, 32))
#define ID_PFR0_EL1_RES1                                (UL(0))
#define ID_PFR0_EL1_UNKN                                (UL(0))

#define REG_ID_PFR1_EL1                                 S3_0_C0_C1_1
#define SYS_ID_PFR1_EL1                                 sys_reg(3, 0, 0, 1, 1)
#define SYS_ID_PFR1_EL1_Op0                             3
#define SYS_ID_PFR1_EL1_Op1                             0
#define SYS_ID_PFR1_EL1_CRn                             0
#define SYS_ID_PFR1_EL1_CRm                             1
#define SYS_ID_PFR1_EL1_Op2                             1

#define ID_PFR1_EL1_GIC                                 GENMASK(31, 28)
#define ID_PFR1_EL1_GIC_MASK                            GENMASK(31, 28)
#define ID_PFR1_EL1_GIC_SHIFT                           28
#define ID_PFR1_EL1_GIC_WIDTH                           4
#define ID_PFR1_EL1_GIC_SIGNED                          false
#define ID_PFR1_EL1_GIC_NI                              UL(0b0000)
#define ID_PFR1_EL1_GIC_GICv3                           UL(0b0001)
#define ID_PFR1_EL1_GIC_GICv4p1                         UL(0b0011)

#define ID_PFR1_EL1_Virt_frac                           GENMASK(27, 24)
#define ID_PFR1_EL1_Virt_frac_MASK                      GENMASK(27, 24)
#define ID_PFR1_EL1_Virt_frac_SHIFT                     24
#define ID_PFR1_EL1_Virt_frac_WIDTH                     4
#define ID_PFR1_EL1_Virt_frac_SIGNED                    false
#define ID_PFR1_EL1_Virt_frac_NI                        UL(0b0000)
#define ID_PFR1_EL1_Virt_frac_IMP                       UL(0b0001)

#define ID_PFR1_EL1_Sec_frac                            GENMASK(23, 20)
#define ID_PFR1_EL1_Sec_frac_MASK                       GENMASK(23, 20)
#define ID_PFR1_EL1_Sec_frac_SHIFT                      20
#define ID_PFR1_EL1_Sec_frac_WIDTH                      4
#define ID_PFR1_EL1_Sec_frac_NI                         UL(0b0000)
#define ID_PFR1_EL1_Sec_frac_WALK_DISABLE               UL(0b0001)
#define ID_PFR1_EL1_Sec_frac_SECURE_MEMORY              UL(0b0010)

#define ID_PFR1_EL1_GenTimer                            GENMASK(19, 16)
#define ID_PFR1_EL1_GenTimer_MASK                       GENMASK(19, 16)
#define ID_PFR1_EL1_GenTimer_SHIFT                      16
#define ID_PFR1_EL1_GenTimer_WIDTH                      4
#define ID_PFR1_EL1_GenTimer_SIGNED                     false
#define ID_PFR1_EL1_GenTimer_NI                         UL(0b0000)
#define ID_PFR1_EL1_GenTimer_IMP                        UL(0b0001)
#define ID_PFR1_EL1_GenTimer_ECV                        UL(0b0010)

#define ID_PFR1_EL1_Virtualization                      GENMASK(15, 12)
#define ID_PFR1_EL1_Virtualization_MASK                 GENMASK(15, 12)
#define ID_PFR1_EL1_Virtualization_SHIFT                12
#define ID_PFR1_EL1_Virtualization_WIDTH                4
#define ID_PFR1_EL1_Virtualization_SIGNED               false
#define ID_PFR1_EL1_Virtualization_NI                   UL(0b0000)
#define ID_PFR1_EL1_Virtualization_IMP                  UL(0b0001)

#define ID_PFR1_EL1_MProgMod                            GENMASK(11, 8)
#define ID_PFR1_EL1_MProgMod_MASK                       GENMASK(11, 8)
#define ID_PFR1_EL1_MProgMod_SHIFT                      8
#define ID_PFR1_EL1_MProgMod_WIDTH                      4
#define ID_PFR1_EL1_MProgMod_SIGNED                     false
#define ID_PFR1_EL1_MProgMod_NI                         UL(0b0000)
#define ID_PFR1_EL1_MProgMod_IMP                        UL(0b0001)

#define ID_PFR1_EL1_Security                            GENMASK(7, 4)
#define ID_PFR1_EL1_Security_MASK                       GENMASK(7, 4)
#define ID_PFR1_EL1_Security_SHIFT                      4
#define ID_PFR1_EL1_Security_WIDTH                      4
#define ID_PFR1_EL1_Security_NI                         UL(0b0000)
#define ID_PFR1_EL1_Security_EL3                        UL(0b0001)
#define ID_PFR1_EL1_Security_NSACR_RFR                  UL(0b0010)

#define ID_PFR1_EL1_ProgMod                             GENMASK(3, 0)
#define ID_PFR1_EL1_ProgMod_MASK                        GENMASK(3, 0)
#define ID_PFR1_EL1_ProgMod_SHIFT                       0
#define ID_PFR1_EL1_ProgMod_WIDTH                       4
#define ID_PFR1_EL1_ProgMod_SIGNED                      false
#define ID_PFR1_EL1_ProgMod_NI                          UL(0b0000)
#define ID_PFR1_EL1_ProgMod_IMP                         UL(0b0001)

#define ID_PFR1_EL1_RES0                                (UL(0) | GENMASK_ULL(63, 32))
#define ID_PFR1_EL1_RES1                                (UL(0))
#define ID_PFR1_EL1_UNKN                                (UL(0))

#define REG_ID_DFR0_EL1                                 S3_0_C0_C1_2
#define SYS_ID_DFR0_EL1                                 sys_reg(3, 0, 0, 1, 2)
#define SYS_ID_DFR0_EL1_Op0                             3
#define SYS_ID_DFR0_EL1_Op1                             0
#define SYS_ID_DFR0_EL1_CRn                             0
#define SYS_ID_DFR0_EL1_CRm                             1
#define SYS_ID_DFR0_EL1_Op2                             2

#define ID_DFR0_EL1_TraceFilt                           GENMASK(31, 28)
#define ID_DFR0_EL1_TraceFilt_MASK                      GENMASK(31, 28)
#define ID_DFR0_EL1_TraceFilt_SHIFT                     28
#define ID_DFR0_EL1_TraceFilt_WIDTH                     4
#define ID_DFR0_EL1_TraceFilt_SIGNED                    false
#define ID_DFR0_EL1_TraceFilt_NI                        UL(0b0000)
#define ID_DFR0_EL1_TraceFilt_IMP                       UL(0b0001)

#define ID_DFR0_EL1_PerfMon                             GENMASK(27, 24)
#define ID_DFR0_EL1_PerfMon_MASK                        GENMASK(27, 24)
#define ID_DFR0_EL1_PerfMon_SHIFT                       24
#define ID_DFR0_EL1_PerfMon_WIDTH                       4
#define ID_DFR0_EL1_PerfMon_SIGNED                      false
#define ID_DFR0_EL1_PerfMon_NI                          UL(0b0000)
#define ID_DFR0_EL1_PerfMon_PMUv1                       UL(0b0001)
#define ID_DFR0_EL1_PerfMon_PMUv2                       UL(0b0010)
#define ID_DFR0_EL1_PerfMon_PMUv3                       UL(0b0011)
#define ID_DFR0_EL1_PerfMon_PMUv3p1                     UL(0b0100)
#define ID_DFR0_EL1_PerfMon_PMUv3p4                     UL(0b0101)
#define ID_DFR0_EL1_PerfMon_PMUv3p5                     UL(0b0110)
#define ID_DFR0_EL1_PerfMon_PMUv3p7                     UL(0b0111)
#define ID_DFR0_EL1_PerfMon_PMUv3p8                     UL(0b1000)
#define ID_DFR0_EL1_PerfMon_PMUv3p9                     UL(0b1001)
#define ID_DFR0_EL1_PerfMon_IMPDEF                      UL(0b1111)

#define ID_DFR0_EL1_MProfDbg                            GENMASK(23, 20)
#define ID_DFR0_EL1_MProfDbg_MASK                       GENMASK(23, 20)
#define ID_DFR0_EL1_MProfDbg_SHIFT                      20
#define ID_DFR0_EL1_MProfDbg_WIDTH                      4
#define ID_DFR0_EL1_MProfDbg_NI                         UL(0b0000)
#define ID_DFR0_EL1_MProfDbg_IMP                        UL(0b0001)

#define ID_DFR0_EL1_MMapTrc                             GENMASK(19, 16)
#define ID_DFR0_EL1_MMapTrc_MASK                        GENMASK(19, 16)
#define ID_DFR0_EL1_MMapTrc_SHIFT                       16
#define ID_DFR0_EL1_MMapTrc_WIDTH                       4
#define ID_DFR0_EL1_MMapTrc_NI                          UL(0b0000)
#define ID_DFR0_EL1_MMapTrc_IMP                         UL(0b0001)

#define ID_DFR0_EL1_CopTrc                              GENMASK(15, 12)
#define ID_DFR0_EL1_CopTrc_MASK                         GENMASK(15, 12)
#define ID_DFR0_EL1_CopTrc_SHIFT                        12
#define ID_DFR0_EL1_CopTrc_WIDTH                        4
#define ID_DFR0_EL1_CopTrc_NI                           UL(0b0000)
#define ID_DFR0_EL1_CopTrc_IMP                          UL(0b0001)

#define ID_DFR0_EL1_MMapDbg                             GENMASK(11, 8)
#define ID_DFR0_EL1_MMapDbg_MASK                        GENMASK(11, 8)
#define ID_DFR0_EL1_MMapDbg_SHIFT                       8
#define ID_DFR0_EL1_MMapDbg_WIDTH                       4
#define ID_DFR0_EL1_MMapDbg_NI                          UL(0b0000)
#define ID_DFR0_EL1_MMapDbg_Armv7                       UL(0b0100)
#define ID_DFR0_EL1_MMapDbg_Armv7p1                     UL(0b0101)

#define ID_DFR0_EL1_CopSDbg                             GENMASK(7, 4)
#define ID_DFR0_EL1_CopSDbg_MASK                        GENMASK(7, 4)
#define ID_DFR0_EL1_CopSDbg_SHIFT                       4
#define ID_DFR0_EL1_CopSDbg_WIDTH                       4

#define ID_DFR0_EL1_CopDbg                              GENMASK(3, 0)
#define ID_DFR0_EL1_CopDbg_MASK                         GENMASK(3, 0)
#define ID_DFR0_EL1_CopDbg_SHIFT                        0
#define ID_DFR0_EL1_CopDbg_WIDTH                        4
#define ID_DFR0_EL1_CopDbg_NI                           UL(0b0000)
#define ID_DFR0_EL1_CopDbg_Armv6                        UL(0b0010)
#define ID_DFR0_EL1_CopDbg_Armv6p1                      UL(0b0011)
#define ID_DFR0_EL1_CopDbg_Armv7                        UL(0b0100)
#define ID_DFR0_EL1_CopDbg_Armv7p1                      UL(0b0101)
#define ID_DFR0_EL1_CopDbg_Armv8                        UL(0b0110)
#define ID_DFR0_EL1_CopDbg_VHE                          UL(0b0111)
#define ID_DFR0_EL1_CopDbg_Debugv8p2                    UL(0b1000)
#define ID_DFR0_EL1_CopDbg_Debugv8p4                    UL(0b1001)
#define ID_DFR0_EL1_CopDbg_Debugv8p8                    UL(0b1010)
#define ID_DFR0_EL1_CopDbg_Debugv8p9                    UL(0b1011)

#define ID_DFR0_EL1_RES0                                (UL(0) | GENMASK_ULL(63, 32))
#define ID_DFR0_EL1_RES1                                (UL(0))
#define ID_DFR0_EL1_UNKN                                (UL(0))

#define REG_ID_AFR0_EL1                                 S3_0_C0_C1_3
#define SYS_ID_AFR0_EL1                                 sys_reg(3, 0, 0, 1, 3)
#define SYS_ID_AFR0_EL1_Op0                             3
#define SYS_ID_AFR0_EL1_Op1                             0
#define SYS_ID_AFR0_EL1_CRn                             0
#define SYS_ID_AFR0_EL1_CRm                             1
#define SYS_ID_AFR0_EL1_Op2                             3

#define ID_AFR0_EL1_IMPDEF3                             GENMASK(15, 12)
#define ID_AFR0_EL1_IMPDEF3_MASK                        GENMASK(15, 12)
#define ID_AFR0_EL1_IMPDEF3_SHIFT                       12
#define ID_AFR0_EL1_IMPDEF3_WIDTH                       4

#define ID_AFR0_EL1_IMPDEF2                             GENMASK(11, 8)
#define ID_AFR0_EL1_IMPDEF2_MASK                        GENMASK(11, 8)
#define ID_AFR0_EL1_IMPDEF2_SHIFT                       8
#define ID_AFR0_EL1_IMPDEF2_WIDTH                       4

#define ID_AFR0_EL1_IMPDEF1                             GENMASK(7, 4)
#define ID_AFR0_EL1_IMPDEF1_MASK                        GENMASK(7, 4)
#define ID_AFR0_EL1_IMPDEF1_SHIFT                       4
#define ID_AFR0_EL1_IMPDEF1_WIDTH                       4

#define ID_AFR0_EL1_IMPDEF0                             GENMASK(3, 0)
#define ID_AFR0_EL1_IMPDEF0_MASK                        GENMASK(3, 0)
#define ID_AFR0_EL1_IMPDEF0_SHIFT                       0
#define ID_AFR0_EL1_IMPDEF0_WIDTH                       4

#define ID_AFR0_EL1_RES0                                (UL(0) | GENMASK_ULL(63, 16))
#define ID_AFR0_EL1_RES1                                (UL(0))
#define ID_AFR0_EL1_UNKN                                (UL(0))

#define REG_ID_MMFR0_EL1                                S3_0_C0_C1_4
#define SYS_ID_MMFR0_EL1                                sys_reg(3, 0, 0, 1, 4)
#define SYS_ID_MMFR0_EL1_Op0                            3
#define SYS_ID_MMFR0_EL1_Op1                            0
#define SYS_ID_MMFR0_EL1_CRn                            0
#define SYS_ID_MMFR0_EL1_CRm                            1
#define SYS_ID_MMFR0_EL1_Op2                            4

#define ID_MMFR0_EL1_InnerShr                           GENMASK(31, 28)
#define ID_MMFR0_EL1_InnerShr_MASK                      GENMASK(31, 28)
#define ID_MMFR0_EL1_InnerShr_SHIFT                     28
#define ID_MMFR0_EL1_InnerShr_WIDTH                     4
#define ID_MMFR0_EL1_InnerShr_NC                        UL(0b0000)
#define ID_MMFR0_EL1_InnerShr_HW                        UL(0b0001)
#define ID_MMFR0_EL1_InnerShr_IGNORED                   UL(0b1111)

#define ID_MMFR0_EL1_FCSE                               GENMASK(27, 24)
#define ID_MMFR0_EL1_FCSE_MASK                          GENMASK(27, 24)
#define ID_MMFR0_EL1_FCSE_SHIFT                         24
#define ID_MMFR0_EL1_FCSE_WIDTH                         4
#define ID_MMFR0_EL1_FCSE_SIGNED                        false
#define ID_MMFR0_EL1_FCSE_NI                            UL(0b0000)
#define ID_MMFR0_EL1_FCSE_IMP                           UL(0b0001)

#define ID_MMFR0_EL1_AuxReg                             GENMASK(23, 20)
#define ID_MMFR0_EL1_AuxReg_MASK                        GENMASK(23, 20)
#define ID_MMFR0_EL1_AuxReg_SHIFT                       20
#define ID_MMFR0_EL1_AuxReg_WIDTH                       4
#define ID_MMFR0_EL1_AuxReg_NI                          UL(0b0000)
#define ID_MMFR0_EL1_AuxReg_ACTLR                       UL(0b0001)
#define ID_MMFR0_EL1_AuxReg_AIFSR                       UL(0b0010)

#define ID_MMFR0_EL1_TCM                                GENMASK(19, 16)
#define ID_MMFR0_EL1_TCM_MASK                           GENMASK(19, 16)
#define ID_MMFR0_EL1_TCM_SHIFT                          16
#define ID_MMFR0_EL1_TCM_WIDTH                          4
#define ID_MMFR0_EL1_TCM_NI                             UL(0b0000)
#define ID_MMFR0_EL1_TCM_IMPDEF                         UL(0b0001)
#define ID_MMFR0_EL1_TCM_TCM                            UL(0b0010)
#define ID_MMFR0_EL1_TCM_TCM_DMA                        UL(0b0011)

#define ID_MMFR0_EL1_ShareLvl                           GENMASK(15, 12)
#define ID_MMFR0_EL1_ShareLvl_MASK                      GENMASK(15, 12)
#define ID_MMFR0_EL1_ShareLvl_SHIFT                     12
#define ID_MMFR0_EL1_ShareLvl_WIDTH                     4
#define ID_MMFR0_EL1_ShareLvl_ONE                       UL(0b0000)
#define ID_MMFR0_EL1_ShareLvl_TWO                       UL(0b0001)

#define ID_MMFR0_EL1_OuterShr                           GENMASK(11, 8)
#define ID_MMFR0_EL1_OuterShr_MASK                      GENMASK(11, 8)
#define ID_MMFR0_EL1_OuterShr_SHIFT                     8
#define ID_MMFR0_EL1_OuterShr_WIDTH                     4
#define ID_MMFR0_EL1_OuterShr_NC                        UL(0b0000)
#define ID_MMFR0_EL1_OuterShr_HW                        UL(0b0001)
#define ID_MMFR0_EL1_OuterShr_IGNORED                   UL(0b1111)

#define ID_MMFR0_EL1_PMSA                               GENMASK(7, 4)
#define ID_MMFR0_EL1_PMSA_MASK                          GENMASK(7, 4)
#define ID_MMFR0_EL1_PMSA_SHIFT                         4
#define ID_MMFR0_EL1_PMSA_WIDTH                         4
#define ID_MMFR0_EL1_PMSA_NI                            UL(0b0000)
#define ID_MMFR0_EL1_PMSA_IMPDEF                        UL(0b0001)
#define ID_MMFR0_EL1_PMSA_PMSAv6                        UL(0b0010)
#define ID_MMFR0_EL1_PMSA_PMSAv7                        UL(0b0011)

#define ID_MMFR0_EL1_VMSA                               GENMASK(3, 0)
#define ID_MMFR0_EL1_VMSA_MASK                          GENMASK(3, 0)
#define ID_MMFR0_EL1_VMSA_SHIFT                         0
#define ID_MMFR0_EL1_VMSA_WIDTH                         4
#define ID_MMFR0_EL1_VMSA_NI                            UL(0b0000)
#define ID_MMFR0_EL1_VMSA_IMPDEF                        UL(0b0001)
#define ID_MMFR0_EL1_VMSA_VMSAv6                        UL(0b0010)
#define ID_MMFR0_EL1_VMSA_VMSAv7                        UL(0b0011)
#define ID_MMFR0_EL1_VMSA_VMSAv7_PXN                    UL(0b0100)
#define ID_MMFR0_EL1_VMSA_VMSAv7_LONG                   UL(0b0101)

#define ID_MMFR0_EL1_RES0                               (UL(0) | GENMASK_ULL(63, 32))
#define ID_MMFR0_EL1_RES1                               (UL(0))
#define ID_MMFR0_EL1_UNKN                               (UL(0))

#define REG_ID_MMFR1_EL1                                S3_0_C0_C1_5
#define SYS_ID_MMFR1_EL1                                sys_reg(3, 0, 0, 1, 5)
#define SYS_ID_MMFR1_EL1_Op0                            3
#define SYS_ID_MMFR1_EL1_Op1                            0
#define SYS_ID_MMFR1_EL1_CRn                            0
#define SYS_ID_MMFR1_EL1_CRm                            1
#define SYS_ID_MMFR1_EL1_Op2                            5

#define ID_MMFR1_EL1_BPred                              GENMASK(31, 28)
#define ID_MMFR1_EL1_BPred_MASK                         GENMASK(31, 28)
#define ID_MMFR1_EL1_BPred_SHIFT                        28
#define ID_MMFR1_EL1_BPred_WIDTH                        4
#define ID_MMFR1_EL1_BPred_NI                           UL(0b0000)
#define ID_MMFR1_EL1_BPred_BP_SW_MANGED                 UL(0b0001)
#define ID_MMFR1_EL1_BPred_BP_ASID_AWARE                UL(0b0010)
#define ID_MMFR1_EL1_BPred_BP_NOSNOOP                   UL(0b0011)
#define ID_MMFR1_EL1_BPred_BP_INVISIBLE                 UL(0b0100)

#define ID_MMFR1_EL1_L1TstCln                           GENMASK(27, 24)
#define ID_MMFR1_EL1_L1TstCln_MASK                      GENMASK(27, 24)
#define ID_MMFR1_EL1_L1TstCln_SHIFT                     24
#define ID_MMFR1_EL1_L1TstCln_WIDTH                     4
#define ID_MMFR1_EL1_L1TstCln_NI                        UL(0b0000)
#define ID_MMFR1_EL1_L1TstCln_NOINVALIDATE              UL(0b0001)
#define ID_MMFR1_EL1_L1TstCln_INVALIDATE                UL(0b0010)

#define ID_MMFR1_EL1_L1Uni                              GENMASK(23, 20)
#define ID_MMFR1_EL1_L1Uni_MASK                         GENMASK(23, 20)
#define ID_MMFR1_EL1_L1Uni_SHIFT                        20
#define ID_MMFR1_EL1_L1Uni_WIDTH                        4
#define ID_MMFR1_EL1_L1Uni_NI                           UL(0b0000)
#define ID_MMFR1_EL1_L1Uni_INVALIDATE                   UL(0b0001)
#define ID_MMFR1_EL1_L1Uni_CLEAN_AND_INVALIDATE         UL(0b0010)

#define ID_MMFR1_EL1_L1Hvd                              GENMASK(19, 16)
#define ID_MMFR1_EL1_L1Hvd_MASK                         GENMASK(19, 16)
#define ID_MMFR1_EL1_L1Hvd_SHIFT                        16
#define ID_MMFR1_EL1_L1Hvd_WIDTH                        4
#define ID_MMFR1_EL1_L1Hvd_NI                           UL(0b0000)
#define ID_MMFR1_EL1_L1Hvd_INVALIDATE_ISIDE_ONLY        UL(0b0001)
#define ID_MMFR1_EL1_L1Hvd_INVALIDATE                   UL(0b0010)
#define ID_MMFR1_EL1_L1Hvd_CLEAN_AND_INVALIDATE         UL(0b0011)

#define ID_MMFR1_EL1_L1UniSW                            GENMASK(15, 12)
#define ID_MMFR1_EL1_L1UniSW_MASK                       GENMASK(15, 12)
#define ID_MMFR1_EL1_L1UniSW_SHIFT                      12
#define ID_MMFR1_EL1_L1UniSW_WIDTH                      4
#define ID_MMFR1_EL1_L1UniSW_NI                         UL(0b0000)
#define ID_MMFR1_EL1_L1UniSW_CLEAN                      UL(0b0001)
#define ID_MMFR1_EL1_L1UniSW_CLEAN_AND_INVALIDATE       UL(0b0010)
#define ID_MMFR1_EL1_L1UniSW_INVALIDATE                 UL(0b0011)

#define ID_MMFR1_EL1_L1HvdSW                            GENMASK(11, 8)
#define ID_MMFR1_EL1_L1HvdSW_MASK                       GENMASK(11, 8)
#define ID_MMFR1_EL1_L1HvdSW_SHIFT                      8
#define ID_MMFR1_EL1_L1HvdSW_WIDTH                      4
#define ID_MMFR1_EL1_L1HvdSW_NI                         UL(0b0000)
#define ID_MMFR1_EL1_L1HvdSW_CLEAN_AND_INVALIDATE       UL(0b0001)
#define ID_MMFR1_EL1_L1HvdSW_INVALIDATE_DSIDE_ONLY      UL(0b0010)
#define ID_MMFR1_EL1_L1HvdSW_INVALIDATE                 UL(0b0011)

#define ID_MMFR1_EL1_L1UniVA                            GENMASK(7, 4)
#define ID_MMFR1_EL1_L1UniVA_MASK                       GENMASK(7, 4)
#define ID_MMFR1_EL1_L1UniVA_SHIFT                      4
#define ID_MMFR1_EL1_L1UniVA_WIDTH                      4
#define ID_MMFR1_EL1_L1UniVA_NI                         UL(0b0000)
#define ID_MMFR1_EL1_L1UniVA_CLEAN_AND_INVALIDATE       UL(0b0001)
#define ID_MMFR1_EL1_L1UniVA_INVALIDATE_BP              UL(0b0010)

#define ID_MMFR1_EL1_L1HvdVA                            GENMASK(3, 0)
#define ID_MMFR1_EL1_L1HvdVA_MASK                       GENMASK(3, 0)
#define ID_MMFR1_EL1_L1HvdVA_SHIFT                      0
#define ID_MMFR1_EL1_L1HvdVA_WIDTH                      4
#define ID_MMFR1_EL1_L1HvdVA_NI                         UL(0b0000)
#define ID_MMFR1_EL1_L1HvdVA_CLEAN_AND_INVALIDATE       UL(0b0001)
#define ID_MMFR1_EL1_L1HvdVA_INVALIDATE_BP              UL(0b0010)

#define ID_MMFR1_EL1_RES0                               (UL(0) | GENMASK_ULL(63, 32))
#define ID_MMFR1_EL1_RES1                               (UL(0))
#define ID_MMFR1_EL1_UNKN                               (UL(0))

#define REG_ID_MMFR2_EL1                                S3_0_C0_C1_6
#define SYS_ID_MMFR2_EL1                                sys_reg(3, 0, 0, 1, 6)
#define SYS_ID_MMFR2_EL1_Op0                            3
#define SYS_ID_MMFR2_EL1_Op1                            0
#define SYS_ID_MMFR2_EL1_CRn                            0
#define SYS_ID_MMFR2_EL1_CRm                            1
#define SYS_ID_MMFR2_EL1_Op2                            6

#define ID_MMFR2_EL1_HWAccFlg                           GENMASK(31, 28)
#define ID_MMFR2_EL1_HWAccFlg_MASK                      GENMASK(31, 28)
#define ID_MMFR2_EL1_HWAccFlg_SHIFT                     28
#define ID_MMFR2_EL1_HWAccFlg_WIDTH                     4
#define ID_MMFR2_EL1_HWAccFlg_NI                        UL(0b0000)
#define ID_MMFR2_EL1_HWAccFlg_IMP                       UL(0b0001)

#define ID_MMFR2_EL1_WFIStall                           GENMASK(27, 24)
#define ID_MMFR2_EL1_WFIStall_MASK                      GENMASK(27, 24)
#define ID_MMFR2_EL1_WFIStall_SHIFT                     24
#define ID_MMFR2_EL1_WFIStall_WIDTH                     4
#define ID_MMFR2_EL1_WFIStall_NI                        UL(0b0000)
#define ID_MMFR2_EL1_WFIStall_IMP                       UL(0b0001)

#define ID_MMFR2_EL1_MemBarr                            GENMASK(23, 20)
#define ID_MMFR2_EL1_MemBarr_MASK                       GENMASK(23, 20)
#define ID_MMFR2_EL1_MemBarr_SHIFT                      20
#define ID_MMFR2_EL1_MemBarr_WIDTH                      4
#define ID_MMFR2_EL1_MemBarr_NI                         UL(0b0000)
#define ID_MMFR2_EL1_MemBarr_DSB_ONLY                   UL(0b0001)
#define ID_MMFR2_EL1_MemBarr_IMP                        UL(0b0010)

#define ID_MMFR2_EL1_UniTLB                             GENMASK(19, 16)
#define ID_MMFR2_EL1_UniTLB_MASK                        GENMASK(19, 16)
#define ID_MMFR2_EL1_UniTLB_SHIFT                       16
#define ID_MMFR2_EL1_UniTLB_WIDTH                       4
#define ID_MMFR2_EL1_UniTLB_NI                          UL(0b0000)
#define ID_MMFR2_EL1_UniTLB_BY_VA                       UL(0b0001)
#define ID_MMFR2_EL1_UniTLB_BY_MATCH_ASID               UL(0b0010)
#define ID_MMFR2_EL1_UniTLB_BY_ALL_ASID                 UL(0b0011)
#define ID_MMFR2_EL1_UniTLB_OTHER_TLBS                  UL(0b0100)
#define ID_MMFR2_EL1_UniTLB_BROADCAST                   UL(0b0101)
#define ID_MMFR2_EL1_UniTLB_BY_IPA                      UL(0b0110)

#define ID_MMFR2_EL1_HvdTLB                             GENMASK(15, 12)
#define ID_MMFR2_EL1_HvdTLB_MASK                        GENMASK(15, 12)
#define ID_MMFR2_EL1_HvdTLB_SHIFT                       12
#define ID_MMFR2_EL1_HvdTLB_WIDTH                       4
#define ID_MMFR2_EL1_HvdTLB_NI                          UL(0b0000)

#define ID_MMFR2_EL1_L1HvdRng                           GENMASK(11, 8)
#define ID_MMFR2_EL1_L1HvdRng_MASK                      GENMASK(11, 8)
#define ID_MMFR2_EL1_L1HvdRng_SHIFT                     8
#define ID_MMFR2_EL1_L1HvdRng_WIDTH                     4
#define ID_MMFR2_EL1_L1HvdRng_NI                        UL(0b0000)
#define ID_MMFR2_EL1_L1HvdRng_IMP                       UL(0b0001)

#define ID_MMFR2_EL1_L1HvdBG                            GENMASK(7, 4)
#define ID_MMFR2_EL1_L1HvdBG_MASK                       GENMASK(7, 4)
#define ID_MMFR2_EL1_L1HvdBG_SHIFT                      4
#define ID_MMFR2_EL1_L1HvdBG_WIDTH                      4
#define ID_MMFR2_EL1_L1HvdBG_NI                         UL(0b0000)
#define ID_MMFR2_EL1_L1HvdBG_IMP                        UL(0b0001)

#define ID_MMFR2_EL1_L1HvdFG                            GENMASK(3, 0)
#define ID_MMFR2_EL1_L1HvdFG_MASK                       GENMASK(3, 0)
#define ID_MMFR2_EL1_L1HvdFG_SHIFT                      0
#define ID_MMFR2_EL1_L1HvdFG_WIDTH                      4
#define ID_MMFR2_EL1_L1HvdFG_NI                         UL(0b0000)
#define ID_MMFR2_EL1_L1HvdFG_IMP                        UL(0b0001)

#define ID_MMFR2_EL1_RES0                               (UL(0) | GENMASK_ULL(63, 32))
#define ID_MMFR2_EL1_RES1                               (UL(0))
#define ID_MMFR2_EL1_UNKN                               (UL(0))

#define REG_ID_MMFR3_EL1                                S3_0_C0_C1_7
#define SYS_ID_MMFR3_EL1                                sys_reg(3, 0, 0, 1, 7)
#define SYS_ID_MMFR3_EL1_Op0                            3
#define SYS_ID_MMFR3_EL1_Op1                            0
#define SYS_ID_MMFR3_EL1_CRn                            0
#define SYS_ID_MMFR3_EL1_CRm                            1
#define SYS_ID_MMFR3_EL1_Op2                            7

#define ID_MMFR3_EL1_Supersec                           GENMASK(31, 28)
#define ID_MMFR3_EL1_Supersec_MASK                      GENMASK(31, 28)
#define ID_MMFR3_EL1_Supersec_SHIFT                     28
#define ID_MMFR3_EL1_Supersec_WIDTH                     4
#define ID_MMFR3_EL1_Supersec_IMP                       UL(0b0000)
#define ID_MMFR3_EL1_Supersec_NI                        UL(0b1111)

#define ID_MMFR3_EL1_CMemSz                             GENMASK(27, 24)
#define ID_MMFR3_EL1_CMemSz_MASK                        GENMASK(27, 24)
#define ID_MMFR3_EL1_CMemSz_SHIFT                       24
#define ID_MMFR3_EL1_CMemSz_WIDTH                       4
#define ID_MMFR3_EL1_CMemSz_4GB                         UL(0b0000)
#define ID_MMFR3_EL1_CMemSz_64GB                        UL(0b0001)
#define ID_MMFR3_EL1_CMemSz_1TB                         UL(0b0010)

#define ID_MMFR3_EL1_CohWalk                            GENMASK(23, 20)
#define ID_MMFR3_EL1_CohWalk_MASK                       GENMASK(23, 20)
#define ID_MMFR3_EL1_CohWalk_SHIFT                      20
#define ID_MMFR3_EL1_CohWalk_WIDTH                      4
#define ID_MMFR3_EL1_CohWalk_NI                         UL(0b0000)
#define ID_MMFR3_EL1_CohWalk_IMP                        UL(0b0001)

#define ID_MMFR3_EL1_PAN                                GENMASK(19, 16)
#define ID_MMFR3_EL1_PAN_MASK                           GENMASK(19, 16)
#define ID_MMFR3_EL1_PAN_SHIFT                          16
#define ID_MMFR3_EL1_PAN_WIDTH                          4
#define ID_MMFR3_EL1_PAN_NI                             UL(0b0000)
#define ID_MMFR3_EL1_PAN_PAN                            UL(0b0001)
#define ID_MMFR3_EL1_PAN_PAN2                           UL(0b0010)

#define ID_MMFR3_EL1_MaintBcst                          GENMASK(15, 12)
#define ID_MMFR3_EL1_MaintBcst_MASK                     GENMASK(15, 12)
#define ID_MMFR3_EL1_MaintBcst_SHIFT                    12
#define ID_MMFR3_EL1_MaintBcst_WIDTH                    4
#define ID_MMFR3_EL1_MaintBcst_NI                       UL(0b0000)
#define ID_MMFR3_EL1_MaintBcst_NO_TLB                   UL(0b0001)
#define ID_MMFR3_EL1_MaintBcst_ALL                      UL(0b0010)

#define ID_MMFR3_EL1_BPMaint                            GENMASK(11, 8)
#define ID_MMFR3_EL1_BPMaint_MASK                       GENMASK(11, 8)
#define ID_MMFR3_EL1_BPMaint_SHIFT                      8
#define ID_MMFR3_EL1_BPMaint_WIDTH                      4
#define ID_MMFR3_EL1_BPMaint_NI                         UL(0b0000)
#define ID_MMFR3_EL1_BPMaint_ALL                        UL(0b0001)
#define ID_MMFR3_EL1_BPMaint_BY_VA                      UL(0b0010)

#define ID_MMFR3_EL1_CMaintSW                           GENMASK(7, 4)
#define ID_MMFR3_EL1_CMaintSW_MASK                      GENMASK(7, 4)
#define ID_MMFR3_EL1_CMaintSW_SHIFT                     4
#define ID_MMFR3_EL1_CMaintSW_WIDTH                     4
#define ID_MMFR3_EL1_CMaintSW_NI                        UL(0b0000)
#define ID_MMFR3_EL1_CMaintSW_IMP                       UL(0b0001)

#define ID_MMFR3_EL1_CMaintVA                           GENMASK(3, 0)
#define ID_MMFR3_EL1_CMaintVA_MASK                      GENMASK(3, 0)
#define ID_MMFR3_EL1_CMaintVA_SHIFT                     0
#define ID_MMFR3_EL1_CMaintVA_WIDTH                     4
#define ID_MMFR3_EL1_CMaintVA_NI                        UL(0b0000)
#define ID_MMFR3_EL1_CMaintVA_IMP                       UL(0b0001)

#define ID_MMFR3_EL1_RES0                               (UL(0) | GENMASK_ULL(63, 32))
#define ID_MMFR3_EL1_RES1                               (UL(0))
#define ID_MMFR3_EL1_UNKN                               (UL(0))

#define REG_ID_ISAR0_EL1                                S3_0_C0_C2_0
#define SYS_ID_ISAR0_EL1                                sys_reg(3, 0, 0, 2, 0)
#define SYS_ID_ISAR0_EL1_Op0                            3
#define SYS_ID_ISAR0_EL1_Op1                            0
#define SYS_ID_ISAR0_EL1_CRn                            0
#define SYS_ID_ISAR0_EL1_CRm                            2
#define SYS_ID_ISAR0_EL1_Op2                            0

#define ID_ISAR0_EL1_Divide                             GENMASK(27, 24)
#define ID_ISAR0_EL1_Divide_MASK                        GENMASK(27, 24)
#define ID_ISAR0_EL1_Divide_SHIFT                       24
#define ID_ISAR0_EL1_Divide_WIDTH                       4
#define ID_ISAR0_EL1_Divide_NI                          UL(0b0000)
#define ID_ISAR0_EL1_Divide_xDIV_T32                    UL(0b0001)
#define ID_ISAR0_EL1_Divide_xDIV_A32                    UL(0b0010)

#define ID_ISAR0_EL1_Debug                              GENMASK(23, 20)
#define ID_ISAR0_EL1_Debug_MASK                         GENMASK(23, 20)
#define ID_ISAR0_EL1_Debug_SHIFT                        20
#define ID_ISAR0_EL1_Debug_WIDTH                        4
#define ID_ISAR0_EL1_Debug_SIGNED                       false
#define ID_ISAR0_EL1_Debug_NI                           UL(0b0000)
#define ID_ISAR0_EL1_Debug_IMP                          UL(0b0001)

#define ID_ISAR0_EL1_Coproc                             GENMASK(19, 16)
#define ID_ISAR0_EL1_Coproc_MASK                        GENMASK(19, 16)
#define ID_ISAR0_EL1_Coproc_SHIFT                       16
#define ID_ISAR0_EL1_Coproc_WIDTH                       4
#define ID_ISAR0_EL1_Coproc_NI                          UL(0b0000)
#define ID_ISAR0_EL1_Coproc_MRC                         UL(0b0001)
#define ID_ISAR0_EL1_Coproc_MRC2                        UL(0b0010)
#define ID_ISAR0_EL1_Coproc_MRRC                        UL(0b0011)
#define ID_ISAR0_EL1_Coproc_MRRC2                       UL(0b0100)

#define ID_ISAR0_EL1_CmpBranch                          GENMASK(15, 12)
#define ID_ISAR0_EL1_CmpBranch_MASK                     GENMASK(15, 12)
#define ID_ISAR0_EL1_CmpBranch_SHIFT                    12
#define ID_ISAR0_EL1_CmpBranch_WIDTH                    4
#define ID_ISAR0_EL1_CmpBranch_SIGNED                   false
#define ID_ISAR0_EL1_CmpBranch_NI                       UL(0b0000)
#define ID_ISAR0_EL1_CmpBranch_IMP                      UL(0b0001)

#define ID_ISAR0_EL1_BitField                           GENMASK(11, 8)
#define ID_ISAR0_EL1_BitField_MASK                      GENMASK(11, 8)
#define ID_ISAR0_EL1_BitField_SHIFT                     8
#define ID_ISAR0_EL1_BitField_WIDTH                     4
#define ID_ISAR0_EL1_BitField_SIGNED                    false
#define ID_ISAR0_EL1_BitField_NI                        UL(0b0000)
#define ID_ISAR0_EL1_BitField_IMP                       UL(0b0001)

#define ID_ISAR0_EL1_BitCount                           GENMASK(7, 4)
#define ID_ISAR0_EL1_BitCount_MASK                      GENMASK(7, 4)
#define ID_ISAR0_EL1_BitCount_SHIFT                     4
#define ID_ISAR0_EL1_BitCount_WIDTH                     4
#define ID_ISAR0_EL1_BitCount_SIGNED                    false
#define ID_ISAR0_EL1_BitCount_NI                        UL(0b0000)
#define ID_ISAR0_EL1_BitCount_IMP                       UL(0b0001)

#define ID_ISAR0_EL1_Swap                               GENMASK(3, 0)
#define ID_ISAR0_EL1_Swap_MASK                          GENMASK(3, 0)
#define ID_ISAR0_EL1_Swap_SHIFT                         0
#define ID_ISAR0_EL1_Swap_WIDTH                         4
#define ID_ISAR0_EL1_Swap_SIGNED                        false
#define ID_ISAR0_EL1_Swap_NI                            UL(0b0000)
#define ID_ISAR0_EL1_Swap_IMP                           UL(0b0001)

#define ID_ISAR0_EL1_RES0                               (UL(0) | GENMASK_ULL(63, 28))
#define ID_ISAR0_EL1_RES1                               (UL(0))
#define ID_ISAR0_EL1_UNKN                               (UL(0))

#define REG_ID_ISAR1_EL1                                S3_0_C0_C2_1
#define SYS_ID_ISAR1_EL1                                sys_reg(3, 0, 0, 2, 1)
#define SYS_ID_ISAR1_EL1_Op0                            3
#define SYS_ID_ISAR1_EL1_Op1                            0
#define SYS_ID_ISAR1_EL1_CRn                            0
#define SYS_ID_ISAR1_EL1_CRm                            2
#define SYS_ID_ISAR1_EL1_Op2                            1

#define ID_ISAR1_EL1_Jazelle                            GENMASK(31, 28)
#define ID_ISAR1_EL1_Jazelle_MASK                       GENMASK(31, 28)
#define ID_ISAR1_EL1_Jazelle_SHIFT                      28
#define ID_ISAR1_EL1_Jazelle_WIDTH                      4
#define ID_ISAR1_EL1_Jazelle_NI                         UL(0b0000)
#define ID_ISAR1_EL1_Jazelle_IMP                        UL(0b0001)

#define ID_ISAR1_EL1_Interwork                          GENMASK(27, 24)
#define ID_ISAR1_EL1_Interwork_MASK                     GENMASK(27, 24)
#define ID_ISAR1_EL1_Interwork_SHIFT                    24
#define ID_ISAR1_EL1_Interwork_WIDTH                    4
#define ID_ISAR1_EL1_Interwork_NI                       UL(0b0000)
#define ID_ISAR1_EL1_Interwork_BX                       UL(0b0001)
#define ID_ISAR1_EL1_Interwork_BLX                      UL(0b0010)
#define ID_ISAR1_EL1_Interwork_A32_BX                   UL(0b0011)

#define ID_ISAR1_EL1_Immediate                          GENMASK(23, 20)
#define ID_ISAR1_EL1_Immediate_MASK                     GENMASK(23, 20)
#define ID_ISAR1_EL1_Immediate_SHIFT                    20
#define ID_ISAR1_EL1_Immediate_WIDTH                    4
#define ID_ISAR1_EL1_Immediate_NI                       UL(0b0000)
#define ID_ISAR1_EL1_Immediate_IMP                      UL(0b0001)

#define ID_ISAR1_EL1_IfThen                             GENMASK(19, 16)
#define ID_ISAR1_EL1_IfThen_MASK                        GENMASK(19, 16)
#define ID_ISAR1_EL1_IfThen_SHIFT                       16
#define ID_ISAR1_EL1_IfThen_WIDTH                       4
#define ID_ISAR1_EL1_IfThen_NI                          UL(0b0000)
#define ID_ISAR1_EL1_IfThen_IMP                         UL(0b0001)

#define ID_ISAR1_EL1_Extend                             GENMASK(15, 12)
#define ID_ISAR1_EL1_Extend_MASK                        GENMASK(15, 12)
#define ID_ISAR1_EL1_Extend_SHIFT                       12
#define ID_ISAR1_EL1_Extend_WIDTH                       4
#define ID_ISAR1_EL1_Extend_NI                          UL(0b0000)
#define ID_ISAR1_EL1_Extend_SXTB                        UL(0b0001)
#define ID_ISAR1_EL1_Extend_SXTB16                      UL(0b0010)

#define ID_ISAR1_EL1_Except_AR                          GENMASK(11, 8)
#define ID_ISAR1_EL1_Except_AR_MASK                     GENMASK(11, 8)
#define ID_ISAR1_EL1_Except_AR_SHIFT                    8
#define ID_ISAR1_EL1_Except_AR_WIDTH                    4
#define ID_ISAR1_EL1_Except_AR_NI                       UL(0b0000)
#define ID_ISAR1_EL1_Except_AR_IMP                      UL(0b0001)

#define ID_ISAR1_EL1_Except                             GENMASK(7, 4)
#define ID_ISAR1_EL1_Except_MASK                        GENMASK(7, 4)
#define ID_ISAR1_EL1_Except_SHIFT                       4
#define ID_ISAR1_EL1_Except_WIDTH                       4
#define ID_ISAR1_EL1_Except_NI                          UL(0b0000)
#define ID_ISAR1_EL1_Except_IMP                         UL(0b0001)

#define ID_ISAR1_EL1_Endian                             GENMASK(3, 0)
#define ID_ISAR1_EL1_Endian_MASK                        GENMASK(3, 0)
#define ID_ISAR1_EL1_Endian_SHIFT                       0
#define ID_ISAR1_EL1_Endian_WIDTH                       4
#define ID_ISAR1_EL1_Endian_NI                          UL(0b0000)
#define ID_ISAR1_EL1_Endian_IMP                         UL(0b0001)

#define ID_ISAR1_EL1_RES0                               (UL(0) | GENMASK_ULL(63, 32))
#define ID_ISAR1_EL1_RES1                               (UL(0))
#define ID_ISAR1_EL1_UNKN                               (UL(0))

#define REG_ID_ISAR2_EL1                                S3_0_C0_C2_2
#define SYS_ID_ISAR2_EL1                                sys_reg(3, 0, 0, 2, 2)
#define SYS_ID_ISAR2_EL1_Op0                            3
#define SYS_ID_ISAR2_EL1_Op1                            0
#define SYS_ID_ISAR2_EL1_CRn                            0
#define SYS_ID_ISAR2_EL1_CRm                            2
#define SYS_ID_ISAR2_EL1_Op2                            2

#define ID_ISAR2_EL1_Reversal                           GENMASK(31, 28)
#define ID_ISAR2_EL1_Reversal_MASK                      GENMASK(31, 28)
#define ID_ISAR2_EL1_Reversal_SHIFT                     28
#define ID_ISAR2_EL1_Reversal_WIDTH                     4
#define ID_ISAR2_EL1_Reversal_NI                        UL(0b0000)
#define ID_ISAR2_EL1_Reversal_REV                       UL(0b0001)
#define ID_ISAR2_EL1_Reversal_RBIT                      UL(0b0010)

#define ID_ISAR2_EL1_PSR_AR                             GENMASK(27, 24)
#define ID_ISAR2_EL1_PSR_AR_MASK                        GENMASK(27, 24)
#define ID_ISAR2_EL1_PSR_AR_SHIFT                       24
#define ID_ISAR2_EL1_PSR_AR_WIDTH                       4
#define ID_ISAR2_EL1_PSR_AR_NI                          UL(0b0000)
#define ID_ISAR2_EL1_PSR_AR_IMP                         UL(0b0001)

#define ID_ISAR2_EL1_MultU                              GENMASK(23, 20)
#define ID_ISAR2_EL1_MultU_MASK                         GENMASK(23, 20)
#define ID_ISAR2_EL1_MultU_SHIFT                        20
#define ID_ISAR2_EL1_MultU_WIDTH                        4
#define ID_ISAR2_EL1_MultU_NI                           UL(0b0000)
#define ID_ISAR2_EL1_MultU_UMULL                        UL(0b0001)
#define ID_ISAR2_EL1_MultU_UMAAL                        UL(0b0010)

#define ID_ISAR2_EL1_MultS                              GENMASK(19, 16)
#define ID_ISAR2_EL1_MultS_MASK                         GENMASK(19, 16)
#define ID_ISAR2_EL1_MultS_SHIFT                        16
#define ID_ISAR2_EL1_MultS_WIDTH                        4
#define ID_ISAR2_EL1_MultS_NI                           UL(0b0000)
#define ID_ISAR2_EL1_MultS_SMULL                        UL(0b0001)
#define ID_ISAR2_EL1_MultS_SMLABB                       UL(0b0010)
#define ID_ISAR2_EL1_MultS_SMLAD                        UL(0b0011)

#define ID_ISAR2_EL1_Mult                               GENMASK(15, 12)
#define ID_ISAR2_EL1_Mult_MASK                          GENMASK(15, 12)
#define ID_ISAR2_EL1_Mult_SHIFT                         12
#define ID_ISAR2_EL1_Mult_WIDTH                         4
#define ID_ISAR2_EL1_Mult_NI                            UL(0b0000)
#define ID_ISAR2_EL1_Mult_MLA                           UL(0b0001)
#define ID_ISAR2_EL1_Mult_MLS                           UL(0b0010)

#define ID_ISAR2_EL1_MultiAccessInt                     GENMASK(11, 8)
#define ID_ISAR2_EL1_MultiAccessInt_MASK                GENMASK(11, 8)
#define ID_ISAR2_EL1_MultiAccessInt_SHIFT               8
#define ID_ISAR2_EL1_MultiAccessInt_WIDTH               4
#define ID_ISAR2_EL1_MultiAccessInt_NI                  UL(0b0000)
#define ID_ISAR2_EL1_MultiAccessInt_RESTARTABLE         UL(0b0001)
#define ID_ISAR2_EL1_MultiAccessInt_CONTINUABLE         UL(0b0010)

#define ID_ISAR2_EL1_MemHint                            GENMASK(7, 4)
#define ID_ISAR2_EL1_MemHint_MASK                       GENMASK(7, 4)
#define ID_ISAR2_EL1_MemHint_SHIFT                      4
#define ID_ISAR2_EL1_MemHint_WIDTH                      4
#define ID_ISAR2_EL1_MemHint_NI                         UL(0b0000)
#define ID_ISAR2_EL1_MemHint_PLD                        UL(0b0001)
#define ID_ISAR2_EL1_MemHint_PLD2                       UL(0b0010)
#define ID_ISAR2_EL1_MemHint_PLI                        UL(0b0011)
#define ID_ISAR2_EL1_MemHint_PLDW                       UL(0b0100)

#define ID_ISAR2_EL1_LoadStore                          GENMASK(3, 0)
#define ID_ISAR2_EL1_LoadStore_MASK                     GENMASK(3, 0)
#define ID_ISAR2_EL1_LoadStore_SHIFT                    0
#define ID_ISAR2_EL1_LoadStore_WIDTH                    4
#define ID_ISAR2_EL1_LoadStore_NI                       UL(0b0000)
#define ID_ISAR2_EL1_LoadStore_DOUBLE                   UL(0b0001)
#define ID_ISAR2_EL1_LoadStore_ACQUIRE                  UL(0b0010)

#define ID_ISAR2_EL1_RES0                               (UL(0) | GENMASK_ULL(63, 32))
#define ID_ISAR2_EL1_RES1                               (UL(0))
#define ID_ISAR2_EL1_UNKN                               (UL(0))

#define REG_ID_ISAR3_EL1                                S3_0_C0_C2_3
#define SYS_ID_ISAR3_EL1                                sys_reg(3, 0, 0, 2, 3)
#define SYS_ID_ISAR3_EL1_Op0                            3
#define SYS_ID_ISAR3_EL1_Op1                            0
#define SYS_ID_ISAR3_EL1_CRn                            0
#define SYS_ID_ISAR3_EL1_CRm                            2
#define SYS_ID_ISAR3_EL1_Op2                            3

#define ID_ISAR3_EL1_T32EE                              GENMASK(31, 28)
#define ID_ISAR3_EL1_T32EE_MASK                         GENMASK(31, 28)
#define ID_ISAR3_EL1_T32EE_SHIFT                        28
#define ID_ISAR3_EL1_T32EE_WIDTH                        4
#define ID_ISAR3_EL1_T32EE_NI                           UL(0b0000)
#define ID_ISAR3_EL1_T32EE_IMP                          UL(0b0001)

#define ID_ISAR3_EL1_TrueNOP                            GENMASK(27, 24)
#define ID_ISAR3_EL1_TrueNOP_MASK                       GENMASK(27, 24)
#define ID_ISAR3_EL1_TrueNOP_SHIFT                      24
#define ID_ISAR3_EL1_TrueNOP_WIDTH                      4
#define ID_ISAR3_EL1_TrueNOP_NI                         UL(0b0000)
#define ID_ISAR3_EL1_TrueNOP_IMP                        UL(0b0001)

#define ID_ISAR3_EL1_T32Copy                            GENMASK(23, 20)
#define ID_ISAR3_EL1_T32Copy_MASK                       GENMASK(23, 20)
#define ID_ISAR3_EL1_T32Copy_SHIFT                      20
#define ID_ISAR3_EL1_T32Copy_WIDTH                      4
#define ID_ISAR3_EL1_T32Copy_NI                         UL(0b0000)
#define ID_ISAR3_EL1_T32Copy_IMP                        UL(0b0001)

#define ID_ISAR3_EL1_TabBranch                          GENMASK(19, 16)
#define ID_ISAR3_EL1_TabBranch_MASK                     GENMASK(19, 16)
#define ID_ISAR3_EL1_TabBranch_SHIFT                    16
#define ID_ISAR3_EL1_TabBranch_WIDTH                    4
#define ID_ISAR3_EL1_TabBranch_NI                       UL(0b0000)
#define ID_ISAR3_EL1_TabBranch_IMP                      UL(0b0001)

#define ID_ISAR3_EL1_SynchPrim                          GENMASK(15, 12)
#define ID_ISAR3_EL1_SynchPrim_MASK                     GENMASK(15, 12)
#define ID_ISAR3_EL1_SynchPrim_SHIFT                    12
#define ID_ISAR3_EL1_SynchPrim_WIDTH                    4
#define ID_ISAR3_EL1_SynchPrim_NI                       UL(0b0000)
#define ID_ISAR3_EL1_SynchPrim_EXCLUSIVE                UL(0b0001)
#define ID_ISAR3_EL1_SynchPrim_DOUBLE                   UL(0b0010)

#define ID_ISAR3_EL1_SVC                                GENMASK(11, 8)
#define ID_ISAR3_EL1_SVC_MASK                           GENMASK(11, 8)
#define ID_ISAR3_EL1_SVC_SHIFT                          8
#define ID_ISAR3_EL1_SVC_WIDTH                          4
#define ID_ISAR3_EL1_SVC_NI                             UL(0b0000)
#define ID_ISAR3_EL1_SVC_IMP                            UL(0b0001)

#define ID_ISAR3_EL1_SIMD                               GENMASK(7, 4)
#define ID_ISAR3_EL1_SIMD_MASK                          GENMASK(7, 4)
#define ID_ISAR3_EL1_SIMD_SHIFT                         4
#define ID_ISAR3_EL1_SIMD_WIDTH                         4
#define ID_ISAR3_EL1_SIMD_NI                            UL(0b0000)
#define ID_ISAR3_EL1_SIMD_SSAT                          UL(0b0001)
#define ID_ISAR3_EL1_SIMD_PKHBT                         UL(0b0011)

#define ID_ISAR3_EL1_Saturate                           GENMASK(3, 0)
#define ID_ISAR3_EL1_Saturate_MASK                      GENMASK(3, 0)
#define ID_ISAR3_EL1_Saturate_SHIFT                     0
#define ID_ISAR3_EL1_Saturate_WIDTH                     4
#define ID_ISAR3_EL1_Saturate_NI                        UL(0b0000)
#define ID_ISAR3_EL1_Saturate_IMP                       UL(0b0001)

#define ID_ISAR3_EL1_RES0                               (UL(0) | GENMASK_ULL(63, 32))
#define ID_ISAR3_EL1_RES1                               (UL(0))
#define ID_ISAR3_EL1_UNKN                               (UL(0))

#define REG_ID_ISAR4_EL1                                S3_0_C0_C2_4
#define SYS_ID_ISAR4_EL1                                sys_reg(3, 0, 0, 2, 4)
#define SYS_ID_ISAR4_EL1_Op0                            3
#define SYS_ID_ISAR4_EL1_Op1                            0
#define SYS_ID_ISAR4_EL1_CRn                            0
#define SYS_ID_ISAR4_EL1_CRm                            2
#define SYS_ID_ISAR4_EL1_Op2                            4

#define ID_ISAR4_EL1_SWP_frac                           GENMASK(31, 28)
#define ID_ISAR4_EL1_SWP_frac_MASK                      GENMASK(31, 28)
#define ID_ISAR4_EL1_SWP_frac_SHIFT                     28
#define ID_ISAR4_EL1_SWP_frac_WIDTH                     4
#define ID_ISAR4_EL1_SWP_frac_NI                        UL(0b0000)
#define ID_ISAR4_EL1_SWP_frac_IMP                       UL(0b0001)

#define ID_ISAR4_EL1_PSR_M                              GENMASK(27, 24)
#define ID_ISAR4_EL1_PSR_M_MASK                         GENMASK(27, 24)
#define ID_ISAR4_EL1_PSR_M_SHIFT                        24
#define ID_ISAR4_EL1_PSR_M_WIDTH                        4
#define ID_ISAR4_EL1_PSR_M_NI                           UL(0b0000)
#define ID_ISAR4_EL1_PSR_M_IMP                          UL(0b0001)

#define ID_ISAR4_EL1_SynchPrim_frac                     GENMASK(23, 20)
#define ID_ISAR4_EL1_SynchPrim_frac_MASK                GENMASK(23, 20)
#define ID_ISAR4_EL1_SynchPrim_frac_SHIFT               20
#define ID_ISAR4_EL1_SynchPrim_frac_WIDTH               4
#define ID_ISAR4_EL1_SynchPrim_frac_NI                  UL(0b0000)
#define ID_ISAR4_EL1_SynchPrim_frac_IMP                 UL(0b0011)

#define ID_ISAR4_EL1_Barrier                            GENMASK(19, 16)
#define ID_ISAR4_EL1_Barrier_MASK                       GENMASK(19, 16)
#define ID_ISAR4_EL1_Barrier_SHIFT                      16
#define ID_ISAR4_EL1_Barrier_WIDTH                      4
#define ID_ISAR4_EL1_Barrier_NI                         UL(0b0000)
#define ID_ISAR4_EL1_Barrier_IMP                        UL(0b0001)

#define ID_ISAR4_EL1_SMC                                GENMASK(15, 12)
#define ID_ISAR4_EL1_SMC_MASK                           GENMASK(15, 12)
#define ID_ISAR4_EL1_SMC_SHIFT                          12
#define ID_ISAR4_EL1_SMC_WIDTH                          4
#define ID_ISAR4_EL1_SMC_NI                             UL(0b0000)
#define ID_ISAR4_EL1_SMC_IMP                            UL(0b0001)

#define ID_ISAR4_EL1_Writeback                          GENMASK(11, 8)
#define ID_ISAR4_EL1_Writeback_MASK                     GENMASK(11, 8)
#define ID_ISAR4_EL1_Writeback_SHIFT                    8
#define ID_ISAR4_EL1_Writeback_WIDTH                    4
#define ID_ISAR4_EL1_Writeback_NI                       UL(0b0000)
#define ID_ISAR4_EL1_Writeback_IMP                      UL(0b0001)

#define ID_ISAR4_EL1_WithShifts                         GENMASK(7, 4)
#define ID_ISAR4_EL1_WithShifts_MASK                    GENMASK(7, 4)
#define ID_ISAR4_EL1_WithShifts_SHIFT                   4
#define ID_ISAR4_EL1_WithShifts_WIDTH                   4
#define ID_ISAR4_EL1_WithShifts_NI                      UL(0b0000)
#define ID_ISAR4_EL1_WithShifts_LSL3                    UL(0b0001)
#define ID_ISAR4_EL1_WithShifts_LS                      UL(0b0011)
#define ID_ISAR4_EL1_WithShifts_REG                     UL(0b0100)

#define ID_ISAR4_EL1_Unpriv                             GENMASK(3, 0)
#define ID_ISAR4_EL1_Unpriv_MASK                        GENMASK(3, 0)
#define ID_ISAR4_EL1_Unpriv_SHIFT                       0
#define ID_ISAR4_EL1_Unpriv_WIDTH                       4
#define ID_ISAR4_EL1_Unpriv_NI                          UL(0b0000)
#define ID_ISAR4_EL1_Unpriv_REG_BYTE                    UL(0b0001)
#define ID_ISAR4_EL1_Unpriv_SIGNED_HALFWORD             UL(0b0010)

#define ID_ISAR4_EL1_RES0                               (UL(0) | GENMASK_ULL(63, 32))
#define ID_ISAR4_EL1_RES1                               (UL(0))
#define ID_ISAR4_EL1_UNKN                               (UL(0))

#define REG_ID_ISAR5_EL1                                S3_0_C0_C2_5
#define SYS_ID_ISAR5_EL1                                sys_reg(3, 0, 0, 2, 5)
#define SYS_ID_ISAR5_EL1_Op0                            3
#define SYS_ID_ISAR5_EL1_Op1                            0
#define SYS_ID_ISAR5_EL1_CRn                            0
#define SYS_ID_ISAR5_EL1_CRm                            2
#define SYS_ID_ISAR5_EL1_Op2                            5

#define ID_ISAR5_EL1_VCMA                               GENMASK(31, 28)
#define ID_ISAR5_EL1_VCMA_MASK                          GENMASK(31, 28)
#define ID_ISAR5_EL1_VCMA_SHIFT                         28
#define ID_ISAR5_EL1_VCMA_WIDTH                         4
#define ID_ISAR5_EL1_VCMA_SIGNED                        false
#define ID_ISAR5_EL1_VCMA_NI                            UL(0b0000)
#define ID_ISAR5_EL1_VCMA_IMP                           UL(0b0001)

#define ID_ISAR5_EL1_RDM                                GENMASK(27, 24)
#define ID_ISAR5_EL1_RDM_MASK                           GENMASK(27, 24)
#define ID_ISAR5_EL1_RDM_SHIFT                          24
#define ID_ISAR5_EL1_RDM_WIDTH                          4
#define ID_ISAR5_EL1_RDM_SIGNED                         false
#define ID_ISAR5_EL1_RDM_NI                             UL(0b0000)
#define ID_ISAR5_EL1_RDM_IMP                            UL(0b0001)

#define ID_ISAR5_EL1_CRC32                              GENMASK(19, 16)
#define ID_ISAR5_EL1_CRC32_MASK                         GENMASK(19, 16)
#define ID_ISAR5_EL1_CRC32_SHIFT                        16
#define ID_ISAR5_EL1_CRC32_WIDTH                        4
#define ID_ISAR5_EL1_CRC32_SIGNED                       false
#define ID_ISAR5_EL1_CRC32_NI                           UL(0b0000)
#define ID_ISAR5_EL1_CRC32_IMP                          UL(0b0001)

#define ID_ISAR5_EL1_SHA2                               GENMASK(15, 12)
#define ID_ISAR5_EL1_SHA2_MASK                          GENMASK(15, 12)
#define ID_ISAR5_EL1_SHA2_SHIFT                         12
#define ID_ISAR5_EL1_SHA2_WIDTH                         4
#define ID_ISAR5_EL1_SHA2_SIGNED                        false
#define ID_ISAR5_EL1_SHA2_NI                            UL(0b0000)
#define ID_ISAR5_EL1_SHA2_IMP                           UL(0b0001)

#define ID_ISAR5_EL1_SHA1                               GENMASK(11, 8)
#define ID_ISAR5_EL1_SHA1_MASK                          GENMASK(11, 8)
#define ID_ISAR5_EL1_SHA1_SHIFT                         8
#define ID_ISAR5_EL1_SHA1_WIDTH                         4
#define ID_ISAR5_EL1_SHA1_SIGNED                        false
#define ID_ISAR5_EL1_SHA1_NI                            UL(0b0000)
#define ID_ISAR5_EL1_SHA1_IMP                           UL(0b0001)

#define ID_ISAR5_EL1_AES                                GENMASK(7, 4)
#define ID_ISAR5_EL1_AES_MASK                           GENMASK(7, 4)
#define ID_ISAR5_EL1_AES_SHIFT                          4
#define ID_ISAR5_EL1_AES_WIDTH                          4
#define ID_ISAR5_EL1_AES_SIGNED                         false
#define ID_ISAR5_EL1_AES_NI                             UL(0b0000)
#define ID_ISAR5_EL1_AES_IMP                            UL(0b0001)
#define ID_ISAR5_EL1_AES_VMULL                          UL(0b0010)

#define ID_ISAR5_EL1_SEVL                               GENMASK(3, 0)
#define ID_ISAR5_EL1_SEVL_MASK                          GENMASK(3, 0)
#define ID_ISAR5_EL1_SEVL_SHIFT                         0
#define ID_ISAR5_EL1_SEVL_WIDTH                         4
#define ID_ISAR5_EL1_SEVL_SIGNED                        false
#define ID_ISAR5_EL1_SEVL_NI                            UL(0b0000)
#define ID_ISAR5_EL1_SEVL_IMP                           UL(0b0001)

#define ID_ISAR5_EL1_RES0                               (UL(0) | GENMASK_ULL(63, 32) | GENMASK_ULL(23, 20))
#define ID_ISAR5_EL1_RES1                               (UL(0))
#define ID_ISAR5_EL1_UNKN                               (UL(0))

#define REG_ID_ISAR6_EL1                                S3_0_C0_C2_7
#define SYS_ID_ISAR6_EL1                                sys_reg(3, 0, 0, 2, 7)
#define SYS_ID_ISAR6_EL1_Op0                            3
#define SYS_ID_ISAR6_EL1_Op1                            0
#define SYS_ID_ISAR6_EL1_CRn                            0
#define SYS_ID_ISAR6_EL1_CRm                            2
#define SYS_ID_ISAR6_EL1_Op2                            7

#define ID_ISAR6_EL1_I8MM                               GENMASK(27, 24)
#define ID_ISAR6_EL1_I8MM_MASK                          GENMASK(27, 24)
#define ID_ISAR6_EL1_I8MM_SHIFT                         24
#define ID_ISAR6_EL1_I8MM_WIDTH                         4
#define ID_ISAR6_EL1_I8MM_SIGNED                        false
#define ID_ISAR6_EL1_I8MM_NI                            UL(0b0000)
#define ID_ISAR6_EL1_I8MM_IMP                           UL(0b0001)

#define ID_ISAR6_EL1_BF16                               GENMASK(23, 20)
#define ID_ISAR6_EL1_BF16_MASK                          GENMASK(23, 20)
#define ID_ISAR6_EL1_BF16_SHIFT                         20
#define ID_ISAR6_EL1_BF16_WIDTH                         4
#define ID_ISAR6_EL1_BF16_SIGNED                        false
#define ID_ISAR6_EL1_BF16_NI                            UL(0b0000)
#define ID_ISAR6_EL1_BF16_IMP                           UL(0b0001)

#define ID_ISAR6_EL1_SPECRES                            GENMASK(19, 16)
#define ID_ISAR6_EL1_SPECRES_MASK                       GENMASK(19, 16)
#define ID_ISAR6_EL1_SPECRES_SHIFT                      16
#define ID_ISAR6_EL1_SPECRES_WIDTH                      4
#define ID_ISAR6_EL1_SPECRES_SIGNED                     false
#define ID_ISAR6_EL1_SPECRES_NI                         UL(0b0000)
#define ID_ISAR6_EL1_SPECRES_IMP                        UL(0b0001)

#define ID_ISAR6_EL1_SB                                 GENMASK(15, 12)
#define ID_ISAR6_EL1_SB_MASK                            GENMASK(15, 12)
#define ID_ISAR6_EL1_SB_SHIFT                           12
#define ID_ISAR6_EL1_SB_WIDTH                           4
#define ID_ISAR6_EL1_SB_SIGNED                          false
#define ID_ISAR6_EL1_SB_NI                              UL(0b0000)
#define ID_ISAR6_EL1_SB_IMP                             UL(0b0001)

#define ID_ISAR6_EL1_FHM                                GENMASK(11, 8)
#define ID_ISAR6_EL1_FHM_MASK                           GENMASK(11, 8)
#define ID_ISAR6_EL1_FHM_SHIFT                          8
#define ID_ISAR6_EL1_FHM_WIDTH                          4
#define ID_ISAR6_EL1_FHM_SIGNED                         false
#define ID_ISAR6_EL1_FHM_NI                             UL(0b0000)
#define ID_ISAR6_EL1_FHM_IMP                            UL(0b0001)

#define ID_ISAR6_EL1_DP                                 GENMASK(7, 4)
#define ID_ISAR6_EL1_DP_MASK                            GENMASK(7, 4)
#define ID_ISAR6_EL1_DP_SHIFT                           4
#define ID_ISAR6_EL1_DP_WIDTH                           4
#define ID_ISAR6_EL1_DP_SIGNED                          false
#define ID_ISAR6_EL1_DP_NI                              UL(0b0000)
#define ID_ISAR6_EL1_DP_IMP                             UL(0b0001)

#define ID_ISAR6_EL1_JSCVT                              GENMASK(3, 0)
#define ID_ISAR6_EL1_JSCVT_MASK                         GENMASK(3, 0)
#define ID_ISAR6_EL1_JSCVT_SHIFT                        0
#define ID_ISAR6_EL1_JSCVT_WIDTH                        4
#define ID_ISAR6_EL1_JSCVT_SIGNED                       false
#define ID_ISAR6_EL1_JSCVT_NI                           UL(0b0000)
#define ID_ISAR6_EL1_JSCVT_IMP                          UL(0b0001)

#define ID_ISAR6_EL1_RES0                               (UL(0) | GENMASK_ULL(63, 28))
#define ID_ISAR6_EL1_RES1                               (UL(0))
#define ID_ISAR6_EL1_UNKN                               (UL(0))

#define REG_ID_MMFR4_EL1                                S3_0_C0_C2_6
#define SYS_ID_MMFR4_EL1                                sys_reg(3, 0, 0, 2, 6)
#define SYS_ID_MMFR4_EL1_Op0                            3
#define SYS_ID_MMFR4_EL1_Op1                            0
#define SYS_ID_MMFR4_EL1_CRn                            0
#define SYS_ID_MMFR4_EL1_CRm                            2
#define SYS_ID_MMFR4_EL1_Op2                            6

#define ID_MMFR4_EL1_EVT                                GENMASK(31, 28)
#define ID_MMFR4_EL1_EVT_MASK                           GENMASK(31, 28)
#define ID_MMFR4_EL1_EVT_SHIFT                          28
#define ID_MMFR4_EL1_EVT_WIDTH                          4
#define ID_MMFR4_EL1_EVT_SIGNED                         false
#define ID_MMFR4_EL1_EVT_NI                             UL(0b0000)
#define ID_MMFR4_EL1_EVT_NO_TLBIS                       UL(0b0001)
#define ID_MMFR4_EL1_EVT_TLBIS                          UL(0b0010)

#define ID_MMFR4_EL1_CCIDX                              GENMASK(27, 24)
#define ID_MMFR4_EL1_CCIDX_MASK                         GENMASK(27, 24)
#define ID_MMFR4_EL1_CCIDX_SHIFT                        24
#define ID_MMFR4_EL1_CCIDX_WIDTH                        4
#define ID_MMFR4_EL1_CCIDX_SIGNED                       false
#define ID_MMFR4_EL1_CCIDX_NI                           UL(0b0000)
#define ID_MMFR4_EL1_CCIDX_IMP                          UL(0b0001)

#define ID_MMFR4_EL1_LSM                                GENMASK(23, 20)
#define ID_MMFR4_EL1_LSM_MASK                           GENMASK(23, 20)
#define ID_MMFR4_EL1_LSM_SHIFT                          20
#define ID_MMFR4_EL1_LSM_WIDTH                          4
#define ID_MMFR4_EL1_LSM_SIGNED                         false
#define ID_MMFR4_EL1_LSM_NI                             UL(0b0000)
#define ID_MMFR4_EL1_LSM_IMP                            UL(0b0001)

#define ID_MMFR4_EL1_HPDS                               GENMASK(19, 16)
#define ID_MMFR4_EL1_HPDS_MASK                          GENMASK(19, 16)
#define ID_MMFR4_EL1_HPDS_SHIFT                         16
#define ID_MMFR4_EL1_HPDS_WIDTH                         4
#define ID_MMFR4_EL1_HPDS_SIGNED                        false
#define ID_MMFR4_EL1_HPDS_NI                            UL(0b0000)
#define ID_MMFR4_EL1_HPDS_AA32HPD                       UL(0b0001)
#define ID_MMFR4_EL1_HPDS_HPDS2                         UL(0b0010)

#define ID_MMFR4_EL1_CnP                                GENMASK(15, 12)
#define ID_MMFR4_EL1_CnP_MASK                           GENMASK(15, 12)
#define ID_MMFR4_EL1_CnP_SHIFT                          12
#define ID_MMFR4_EL1_CnP_WIDTH                          4
#define ID_MMFR4_EL1_CnP_SIGNED                         false
#define ID_MMFR4_EL1_CnP_NI                             UL(0b0000)
#define ID_MMFR4_EL1_CnP_IMP                            UL(0b0001)

#define ID_MMFR4_EL1_XNX                                GENMASK(11, 8)
#define ID_MMFR4_EL1_XNX_MASK                           GENMASK(11, 8)
#define ID_MMFR4_EL1_XNX_SHIFT                          8
#define ID_MMFR4_EL1_XNX_WIDTH                          4
#define ID_MMFR4_EL1_XNX_SIGNED                         false
#define ID_MMFR4_EL1_XNX_NI                             UL(0b0000)
#define ID_MMFR4_EL1_XNX_IMP                            UL(0b0001)

#define ID_MMFR4_EL1_AC2                                GENMASK(7, 4)
#define ID_MMFR4_EL1_AC2_MASK                           GENMASK(7, 4)
#define ID_MMFR4_EL1_AC2_SHIFT                          4
#define ID_MMFR4_EL1_AC2_WIDTH                          4
#define ID_MMFR4_EL1_AC2_SIGNED                         false
#define ID_MMFR4_EL1_AC2_NI                             UL(0b0000)
#define ID_MMFR4_EL1_AC2_IMP                            UL(0b0001)

#define ID_MMFR4_EL1_SpecSEI                            GENMASK(3, 0)
#define ID_MMFR4_EL1_SpecSEI_MASK                       GENMASK(3, 0)
#define ID_MMFR4_EL1_SpecSEI_SHIFT                      0
#define ID_MMFR4_EL1_SpecSEI_WIDTH                      4
#define ID_MMFR4_EL1_SpecSEI_SIGNED                     false
#define ID_MMFR4_EL1_SpecSEI_NI                         UL(0b0000)
#define ID_MMFR4_EL1_SpecSEI_IMP                        UL(0b0001)

#define ID_MMFR4_EL1_RES0                               (UL(0) | GENMASK_ULL(63, 32))
#define ID_MMFR4_EL1_RES1                               (UL(0))
#define ID_MMFR4_EL1_UNKN                               (UL(0))

#define REG_MVFR0_EL1                                   S3_0_C0_C3_0
#define SYS_MVFR0_EL1                                   sys_reg(3, 0, 0, 3, 0)
#define SYS_MVFR0_EL1_Op0                               3
#define SYS_MVFR0_EL1_Op1                               0
#define SYS_MVFR0_EL1_CRn                               0
#define SYS_MVFR0_EL1_CRm                               3
#define SYS_MVFR0_EL1_Op2                               0

#define MVFR0_EL1_FPRound                               GENMASK(31, 28)
#define MVFR0_EL1_FPRound_MASK                          GENMASK(31, 28)
#define MVFR0_EL1_FPRound_SHIFT                         28
#define MVFR0_EL1_FPRound_WIDTH                         4
#define MVFR0_EL1_FPRound_SIGNED                        false
#define MVFR0_EL1_FPRound_NI                            UL(0b0000)
#define MVFR0_EL1_FPRound_IMP                           UL(0b0001)

#define MVFR0_EL1_FPShVec                               GENMASK(27, 24)
#define MVFR0_EL1_FPShVec_MASK                          GENMASK(27, 24)
#define MVFR0_EL1_FPShVec_SHIFT                         24
#define MVFR0_EL1_FPShVec_WIDTH                         4
#define MVFR0_EL1_FPShVec_SIGNED                        false
#define MVFR0_EL1_FPShVec_NI                            UL(0b0000)
#define MVFR0_EL1_FPShVec_IMP                           UL(0b0001)

#define MVFR0_EL1_FPSqrt                                GENMASK(23, 20)
#define MVFR0_EL1_FPSqrt_MASK                           GENMASK(23, 20)
#define MVFR0_EL1_FPSqrt_SHIFT                          20
#define MVFR0_EL1_FPSqrt_WIDTH                          4
#define MVFR0_EL1_FPSqrt_SIGNED                         false
#define MVFR0_EL1_FPSqrt_NI                             UL(0b0000)
#define MVFR0_EL1_FPSqrt_IMP                            UL(0b0001)

#define MVFR0_EL1_FPDivide                              GENMASK(19, 16)
#define MVFR0_EL1_FPDivide_MASK                         GENMASK(19, 16)
#define MVFR0_EL1_FPDivide_SHIFT                        16
#define MVFR0_EL1_FPDivide_WIDTH                        4
#define MVFR0_EL1_FPDivide_SIGNED                       false
#define MVFR0_EL1_FPDivide_NI                           UL(0b0000)
#define MVFR0_EL1_FPDivide_IMP                          UL(0b0001)

#define MVFR0_EL1_FPTrap                                GENMASK(15, 12)
#define MVFR0_EL1_FPTrap_MASK                           GENMASK(15, 12)
#define MVFR0_EL1_FPTrap_SHIFT                          12
#define MVFR0_EL1_FPTrap_WIDTH                          4
#define MVFR0_EL1_FPTrap_SIGNED                         false
#define MVFR0_EL1_FPTrap_NI                             UL(0b0000)
#define MVFR0_EL1_FPTrap_IMP                            UL(0b0001)

#define MVFR0_EL1_FPDP                                  GENMASK(11, 8)
#define MVFR0_EL1_FPDP_MASK                             GENMASK(11, 8)
#define MVFR0_EL1_FPDP_SHIFT                            8
#define MVFR0_EL1_FPDP_WIDTH                            4
#define MVFR0_EL1_FPDP_SIGNED                           false
#define MVFR0_EL1_FPDP_NI                               UL(0b0000)
#define MVFR0_EL1_FPDP_VFPv2                            UL(0b0001)
#define MVFR0_EL1_FPDP_VFPv3                            UL(0b0010)

#define MVFR0_EL1_FPSP                                  GENMASK(7, 4)
#define MVFR0_EL1_FPSP_MASK                             GENMASK(7, 4)
#define MVFR0_EL1_FPSP_SHIFT                            4
#define MVFR0_EL1_FPSP_WIDTH                            4
#define MVFR0_EL1_FPSP_SIGNED                           false
#define MVFR0_EL1_FPSP_NI                               UL(0b0000)
#define MVFR0_EL1_FPSP_VFPv2                            UL(0b0001)
#define MVFR0_EL1_FPSP_VFPv3                            UL(0b0010)

#define MVFR0_EL1_SIMDReg                               GENMASK(3, 0)
#define MVFR0_EL1_SIMDReg_MASK                          GENMASK(3, 0)
#define MVFR0_EL1_SIMDReg_SHIFT                         0
#define MVFR0_EL1_SIMDReg_WIDTH                         4
#define MVFR0_EL1_SIMDReg_NI                            UL(0b0000)
#define MVFR0_EL1_SIMDReg_IMP_16x64                     UL(0b0001)
#define MVFR0_EL1_SIMDReg_IMP_32x64                     UL(0b0010)

#define MVFR0_EL1_RES0                                  (UL(0) | GENMASK_ULL(63, 32))
#define MVFR0_EL1_RES1                                  (UL(0))
#define MVFR0_EL1_UNKN                                  (UL(0))

#define REG_MVFR1_EL1                                   S3_0_C0_C3_1
#define SYS_MVFR1_EL1                                   sys_reg(3, 0, 0, 3, 1)
#define SYS_MVFR1_EL1_Op0                               3
#define SYS_MVFR1_EL1_Op1                               0
#define SYS_MVFR1_EL1_CRn                               0
#define SYS_MVFR1_EL1_CRm                               3
#define SYS_MVFR1_EL1_Op2                               1

#define MVFR1_EL1_SIMDFMAC                              GENMASK(31, 28)
#define MVFR1_EL1_SIMDFMAC_MASK                         GENMASK(31, 28)
#define MVFR1_EL1_SIMDFMAC_SHIFT                        28
#define MVFR1_EL1_SIMDFMAC_WIDTH                        4
#define MVFR1_EL1_SIMDFMAC_SIGNED                       false
#define MVFR1_EL1_SIMDFMAC_NI                           UL(0b0000)
#define MVFR1_EL1_SIMDFMAC_IMP                          UL(0b0001)

#define MVFR1_EL1_FPHP                                  GENMASK(27, 24)
#define MVFR1_EL1_FPHP_MASK                             GENMASK(27, 24)
#define MVFR1_EL1_FPHP_SHIFT                            24
#define MVFR1_EL1_FPHP_WIDTH                            4
#define MVFR1_EL1_FPHP_SIGNED                           false
#define MVFR1_EL1_FPHP_NI                               UL(0b0000)
#define MVFR1_EL1_FPHP_FPHP                             UL(0b0001)
#define MVFR1_EL1_FPHP_FPHP_CONV                        UL(0b0010)
#define MVFR1_EL1_FPHP_FP16                             UL(0b0011)

#define MVFR1_EL1_SIMDHP                                GENMASK(23, 20)
#define MVFR1_EL1_SIMDHP_MASK                           GENMASK(23, 20)
#define MVFR1_EL1_SIMDHP_SHIFT                          20
#define MVFR1_EL1_SIMDHP_WIDTH                          4
#define MVFR1_EL1_SIMDHP_SIGNED                         false
#define MVFR1_EL1_SIMDHP_NI                             UL(0b0000)
#define MVFR1_EL1_SIMDHP_SIMDHP                         UL(0b0001)
#define MVFR1_EL1_SIMDHP_SIMDHP_FLOAT                   UL(0b0010)

#define MVFR1_EL1_SIMDSP                                GENMASK(19, 16)
#define MVFR1_EL1_SIMDSP_MASK                           GENMASK(19, 16)
#define MVFR1_EL1_SIMDSP_SHIFT                          16
#define MVFR1_EL1_SIMDSP_WIDTH                          4
#define MVFR1_EL1_SIMDSP_SIGNED                         false
#define MVFR1_EL1_SIMDSP_NI                             UL(0b0000)
#define MVFR1_EL1_SIMDSP_IMP                            UL(0b0001)

#define MVFR1_EL1_SIMDInt                               GENMASK(15, 12)
#define MVFR1_EL1_SIMDInt_MASK                          GENMASK(15, 12)
#define MVFR1_EL1_SIMDInt_SHIFT                         12
#define MVFR1_EL1_SIMDInt_WIDTH                         4
#define MVFR1_EL1_SIMDInt_SIGNED                        false
#define MVFR1_EL1_SIMDInt_NI                            UL(0b0000)
#define MVFR1_EL1_SIMDInt_IMP                           UL(0b0001)

#define MVFR1_EL1_SIMDLS                                GENMASK(11, 8)
#define MVFR1_EL1_SIMDLS_MASK                           GENMASK(11, 8)
#define MVFR1_EL1_SIMDLS_SHIFT                          8
#define MVFR1_EL1_SIMDLS_WIDTH                          4
#define MVFR1_EL1_SIMDLS_SIGNED                         false
#define MVFR1_EL1_SIMDLS_NI                             UL(0b0000)
#define MVFR1_EL1_SIMDLS_IMP                            UL(0b0001)

#define MVFR1_EL1_FPDNaN                                GENMASK(7, 4)
#define MVFR1_EL1_FPDNaN_MASK                           GENMASK(7, 4)
#define MVFR1_EL1_FPDNaN_SHIFT                          4
#define MVFR1_EL1_FPDNaN_WIDTH                          4
#define MVFR1_EL1_FPDNaN_SIGNED                         false
#define MVFR1_EL1_FPDNaN_NI                             UL(0b0000)
#define MVFR1_EL1_FPDNaN_IMP                            UL(0b0001)

#define MVFR1_EL1_FPFtZ                                 GENMASK(3, 0)
#define MVFR1_EL1_FPFtZ_MASK                            GENMASK(3, 0)
#define MVFR1_EL1_FPFtZ_SHIFT                           0
#define MVFR1_EL1_FPFtZ_WIDTH                           4
#define MVFR1_EL1_FPFtZ_SIGNED                          false
#define MVFR1_EL1_FPFtZ_NI                              UL(0b0000)
#define MVFR1_EL1_FPFtZ_IMP                             UL(0b0001)

#define MVFR1_EL1_RES0                                  (UL(0) | GENMASK_ULL(63, 32))
#define MVFR1_EL1_RES1                                  (UL(0))
#define MVFR1_EL1_UNKN                                  (UL(0))

#define REG_MVFR2_EL1                                   S3_0_C0_C3_2
#define SYS_MVFR2_EL1                                   sys_reg(3, 0, 0, 3, 2)
#define SYS_MVFR2_EL1_Op0                               3
#define SYS_MVFR2_EL1_Op1                               0
#define SYS_MVFR2_EL1_CRn                               0
#define SYS_MVFR2_EL1_CRm                               3
#define SYS_MVFR2_EL1_Op2                               2

#define MVFR2_EL1_FPMisc                                GENMASK(7, 4)
#define MVFR2_EL1_FPMisc_MASK                           GENMASK(7, 4)
#define MVFR2_EL1_FPMisc_SHIFT                          4
#define MVFR2_EL1_FPMisc_WIDTH                          4
#define MVFR2_EL1_FPMisc_NI                             UL(0b0000)
#define MVFR2_EL1_FPMisc_FP                             UL(0b0001)
#define MVFR2_EL1_FPMisc_FP_DIRECTED_ROUNDING           UL(0b0010)
#define MVFR2_EL1_FPMisc_FP_ROUNDING                    UL(0b0011)
#define MVFR2_EL1_FPMisc_FP_MAX_MIN                     UL(0b0100)

#define MVFR2_EL1_SIMDMisc                              GENMASK(3, 0)
#define MVFR2_EL1_SIMDMisc_MASK                         GENMASK(3, 0)
#define MVFR2_EL1_SIMDMisc_SHIFT                        0
#define MVFR2_EL1_SIMDMisc_WIDTH                        4
#define MVFR2_EL1_SIMDMisc_NI                           UL(0b0000)
#define MVFR2_EL1_SIMDMisc_SIMD_DIRECTED_ROUNDING       UL(0b0001)
#define MVFR2_EL1_SIMDMisc_SIMD_ROUNDING                UL(0b0010)
#define MVFR2_EL1_SIMDMisc_SIMD_MAX_MIN                 UL(0b0011)

#define MVFR2_EL1_RES0                                  (UL(0) | GENMASK_ULL(63, 8))
#define MVFR2_EL1_RES1                                  (UL(0))
#define MVFR2_EL1_UNKN                                  (UL(0))

#define REG_ID_PFR2_EL1                                 S3_0_C0_C3_4
#define SYS_ID_PFR2_EL1                                 sys_reg(3, 0, 0, 3, 4)
#define SYS_ID_PFR2_EL1_Op0                             3
#define SYS_ID_PFR2_EL1_Op1                             0
#define SYS_ID_PFR2_EL1_CRn                             0
#define SYS_ID_PFR2_EL1_CRm                             3
#define SYS_ID_PFR2_EL1_Op2                             4

#define ID_PFR2_EL1_RAS_frac                            GENMASK(11, 8)
#define ID_PFR2_EL1_RAS_frac_MASK                       GENMASK(11, 8)
#define ID_PFR2_EL1_RAS_frac_SHIFT                      8
#define ID_PFR2_EL1_RAS_frac_WIDTH                      4
#define ID_PFR2_EL1_RAS_frac_SIGNED                     false
#define ID_PFR2_EL1_RAS_frac_NI                         UL(0b0000)
#define ID_PFR2_EL1_RAS_frac_RASv1p1                    UL(0b0001)

#define ID_PFR2_EL1_SSBS                                GENMASK(7, 4)
#define ID_PFR2_EL1_SSBS_MASK                           GENMASK(7, 4)
#define ID_PFR2_EL1_SSBS_SHIFT                          4
#define ID_PFR2_EL1_SSBS_WIDTH                          4
#define ID_PFR2_EL1_SSBS_SIGNED                         false
#define ID_PFR2_EL1_SSBS_NI                             UL(0b0000)
#define ID_PFR2_EL1_SSBS_IMP                            UL(0b0001)

#define ID_PFR2_EL1_CSV3                                GENMASK(3, 0)
#define ID_PFR2_EL1_CSV3_MASK                           GENMASK(3, 0)
#define ID_PFR2_EL1_CSV3_SHIFT                          0
#define ID_PFR2_EL1_CSV3_WIDTH                          4
#define ID_PFR2_EL1_CSV3_SIGNED                         false
#define ID_PFR2_EL1_CSV3_NI                             UL(0b0000)
#define ID_PFR2_EL1_CSV3_IMP                            UL(0b0001)

#define ID_PFR2_EL1_RES0                                (UL(0) | GENMASK_ULL(63, 12))
#define ID_PFR2_EL1_RES1                                (UL(0))
#define ID_PFR2_EL1_UNKN                                (UL(0))

#define REG_ID_DFR1_EL1                                 S3_0_C0_C3_5
#define SYS_ID_DFR1_EL1                                 sys_reg(3, 0, 0, 3, 5)
#define SYS_ID_DFR1_EL1_Op0                             3
#define SYS_ID_DFR1_EL1_Op1                             0
#define SYS_ID_DFR1_EL1_CRn                             0
#define SYS_ID_DFR1_EL1_CRm                             3
#define SYS_ID_DFR1_EL1_Op2                             5

#define ID_DFR1_EL1_HPMN0                               GENMASK(7, 4)
#define ID_DFR1_EL1_HPMN0_MASK                          GENMASK(7, 4)
#define ID_DFR1_EL1_HPMN0_SHIFT                         4
#define ID_DFR1_EL1_HPMN0_WIDTH                         4
#define ID_DFR1_EL1_HPMN0_SIGNED                        false
#define ID_DFR1_EL1_HPMN0_NI                            UL(0b0000)
#define ID_DFR1_EL1_HPMN0_IMP                           UL(0b0001)

#define ID_DFR1_EL1_MTPMU                               GENMASK(3, 0)
#define ID_DFR1_EL1_MTPMU_MASK                          GENMASK(3, 0)
#define ID_DFR1_EL1_MTPMU_SHIFT                         0
#define ID_DFR1_EL1_MTPMU_WIDTH                         4
#define ID_DFR1_EL1_MTPMU_IMPDEF                        UL(0b0000)
#define ID_DFR1_EL1_MTPMU_IMP                           UL(0b0001)
#define ID_DFR1_EL1_MTPMU_NI                            UL(0b1111)

#define ID_DFR1_EL1_RES0                                (UL(0) | GENMASK_ULL(63, 8))
#define ID_DFR1_EL1_RES1                                (UL(0))
#define ID_DFR1_EL1_UNKN                                (UL(0))

#define REG_ID_MMFR5_EL1                                S3_0_C0_C3_6
#define SYS_ID_MMFR5_EL1                                sys_reg(3, 0, 0, 3, 6)
#define SYS_ID_MMFR5_EL1_Op0                            3
#define SYS_ID_MMFR5_EL1_Op1                            0
#define SYS_ID_MMFR5_EL1_CRn                            0
#define SYS_ID_MMFR5_EL1_CRm                            3
#define SYS_ID_MMFR5_EL1_Op2                            6

#define ID_MMFR5_EL1_nTLBPA                             GENMASK(7, 4)
#define ID_MMFR5_EL1_nTLBPA_MASK                        GENMASK(7, 4)
#define ID_MMFR5_EL1_nTLBPA_SHIFT                       4
#define ID_MMFR5_EL1_nTLBPA_WIDTH                       4
#define ID_MMFR5_EL1_nTLBPA_SIGNED                      false
#define ID_MMFR5_EL1_nTLBPA_NI                          UL(0b0000)
#define ID_MMFR5_EL1_nTLBPA_IMP                         UL(0b0001)

#define ID_MMFR5_EL1_ETS                                GENMASK(3, 0)
#define ID_MMFR5_EL1_ETS_MASK                           GENMASK(3, 0)
#define ID_MMFR5_EL1_ETS_SHIFT                          0
#define ID_MMFR5_EL1_ETS_WIDTH                          4
#define ID_MMFR5_EL1_ETS_SIGNED                         false
#define ID_MMFR5_EL1_ETS_NI                             UL(0b0000)
#define ID_MMFR5_EL1_ETS_IMP                            UL(0b0001)

#define ID_MMFR5_EL1_RES0                               (UL(0) | GENMASK_ULL(63, 8))
#define ID_MMFR5_EL1_RES1                               (UL(0))
#define ID_MMFR5_EL1_UNKN                               (UL(0))

#define REG_ID_AA64PFR0_EL1                             S3_0_C0_C4_0
#define SYS_ID_AA64PFR0_EL1                             sys_reg(3, 0, 0, 4, 0)
#define SYS_ID_AA64PFR0_EL1_Op0                         3
#define SYS_ID_AA64PFR0_EL1_Op1                         0
#define SYS_ID_AA64PFR0_EL1_CRn                         0
#define SYS_ID_AA64PFR0_EL1_CRm                         4
#define SYS_ID_AA64PFR0_EL1_Op2                         0

#define ID_AA64PFR0_EL1_CSV3                            GENMASK(63, 60)
#define ID_AA64PFR0_EL1_CSV3_MASK                       GENMASK(63, 60)
#define ID_AA64PFR0_EL1_CSV3_SHIFT                      60
#define ID_AA64PFR0_EL1_CSV3_WIDTH                      4
#define ID_AA64PFR0_EL1_CSV3_SIGNED                     false
#define ID_AA64PFR0_EL1_CSV3_NI                         UL(0b0000)
#define ID_AA64PFR0_EL1_CSV3_IMP                        UL(0b0001)

#define ID_AA64PFR0_EL1_CSV2                            GENMASK(59, 56)
#define ID_AA64PFR0_EL1_CSV2_MASK                       GENMASK(59, 56)
#define ID_AA64PFR0_EL1_CSV2_SHIFT                      56
#define ID_AA64PFR0_EL1_CSV2_WIDTH                      4
#define ID_AA64PFR0_EL1_CSV2_SIGNED                     false
#define ID_AA64PFR0_EL1_CSV2_NI                         UL(0b0000)
#define ID_AA64PFR0_EL1_CSV2_IMP                        UL(0b0001)
#define ID_AA64PFR0_EL1_CSV2_CSV2_2                     UL(0b0010)
#define ID_AA64PFR0_EL1_CSV2_CSV2_3                     UL(0b0011)

#define ID_AA64PFR0_EL1_RME                             GENMASK(55, 52)
#define ID_AA64PFR0_EL1_RME_MASK                        GENMASK(55, 52)
#define ID_AA64PFR0_EL1_RME_SHIFT                       52
#define ID_AA64PFR0_EL1_RME_WIDTH                       4
#define ID_AA64PFR0_EL1_RME_SIGNED                      false
#define ID_AA64PFR0_EL1_RME_NI                          UL(0b0000)
#define ID_AA64PFR0_EL1_RME_IMP                         UL(0b0001)

#define ID_AA64PFR0_EL1_DIT                             GENMASK(51, 48)
#define ID_AA64PFR0_EL1_DIT_MASK                        GENMASK(51, 48)
#define ID_AA64PFR0_EL1_DIT_SHIFT                       48
#define ID_AA64PFR0_EL1_DIT_WIDTH                       4
#define ID_AA64PFR0_EL1_DIT_SIGNED                      false
#define ID_AA64PFR0_EL1_DIT_NI                          UL(0b0000)
#define ID_AA64PFR0_EL1_DIT_IMP                         UL(0b0001)

#define ID_AA64PFR0_EL1_AMU                             GENMASK(47, 44)
#define ID_AA64PFR0_EL1_AMU_MASK                        GENMASK(47, 44)
#define ID_AA64PFR0_EL1_AMU_SHIFT                       44
#define ID_AA64PFR0_EL1_AMU_WIDTH                       4
#define ID_AA64PFR0_EL1_AMU_SIGNED                      false
#define ID_AA64PFR0_EL1_AMU_NI                          UL(0b0000)
#define ID_AA64PFR0_EL1_AMU_IMP                         UL(0b0001)
#define ID_AA64PFR0_EL1_AMU_V1P1                        UL(0b0010)

#define ID_AA64PFR0_EL1_MPAM                            GENMASK(43, 40)
#define ID_AA64PFR0_EL1_MPAM_MASK                       GENMASK(43, 40)
#define ID_AA64PFR0_EL1_MPAM_SHIFT                      40
#define ID_AA64PFR0_EL1_MPAM_WIDTH                      4
#define ID_AA64PFR0_EL1_MPAM_SIGNED                     false
#define ID_AA64PFR0_EL1_MPAM_0                          UL(0b0000)
#define ID_AA64PFR0_EL1_MPAM_1                          UL(0b0001)

#define ID_AA64PFR0_EL1_SEL2                            GENMASK(39, 36)
#define ID_AA64PFR0_EL1_SEL2_MASK                       GENMASK(39, 36)
#define ID_AA64PFR0_EL1_SEL2_SHIFT                      36
#define ID_AA64PFR0_EL1_SEL2_WIDTH                      4
#define ID_AA64PFR0_EL1_SEL2_SIGNED                     false
#define ID_AA64PFR0_EL1_SEL2_NI                         UL(0b0000)
#define ID_AA64PFR0_EL1_SEL2_IMP                        UL(0b0001)

#define ID_AA64PFR0_EL1_SVE                             GENMASK(35, 32)
#define ID_AA64PFR0_EL1_SVE_MASK                        GENMASK(35, 32)
#define ID_AA64PFR0_EL1_SVE_SHIFT                       32
#define ID_AA64PFR0_EL1_SVE_WIDTH                       4
#define ID_AA64PFR0_EL1_SVE_SIGNED                      false
#define ID_AA64PFR0_EL1_SVE_NI                          UL(0b0000)
#define ID_AA64PFR0_EL1_SVE_IMP                         UL(0b0001)

#define ID_AA64PFR0_EL1_RAS                             GENMASK(31, 28)
#define ID_AA64PFR0_EL1_RAS_MASK                        GENMASK(31, 28)
#define ID_AA64PFR0_EL1_RAS_SHIFT                       28
#define ID_AA64PFR0_EL1_RAS_WIDTH                       4
#define ID_AA64PFR0_EL1_RAS_SIGNED                      false
#define ID_AA64PFR0_EL1_RAS_NI                          UL(0b0000)
#define ID_AA64PFR0_EL1_RAS_IMP                         UL(0b0001)
#define ID_AA64PFR0_EL1_RAS_V1P1                        UL(0b0010)
#define ID_AA64PFR0_EL1_RAS_V2                          UL(0b0011)

#define ID_AA64PFR0_EL1_GIC                             GENMASK(27, 24)
#define ID_AA64PFR0_EL1_GIC_MASK                        GENMASK(27, 24)
#define ID_AA64PFR0_EL1_GIC_SHIFT                       24
#define ID_AA64PFR0_EL1_GIC_WIDTH                       4
#define ID_AA64PFR0_EL1_GIC_SIGNED                      false
#define ID_AA64PFR0_EL1_GIC_NI                          UL(0b0000)
#define ID_AA64PFR0_EL1_GIC_IMP                         UL(0b0001)
#define ID_AA64PFR0_EL1_GIC_V4P1                        UL(0b0011)

#define ID_AA64PFR0_EL1_AdvSIMD                         GENMASK(23, 20)
#define ID_AA64PFR0_EL1_AdvSIMD_MASK                    GENMASK(23, 20)
#define ID_AA64PFR0_EL1_AdvSIMD_SHIFT                   20
#define ID_AA64PFR0_EL1_AdvSIMD_WIDTH                   4
#define ID_AA64PFR0_EL1_AdvSIMD_SIGNED                  true
#define ID_AA64PFR0_EL1_AdvSIMD_IMP                     UL(0b0000)
#define ID_AA64PFR0_EL1_AdvSIMD_FP16                    UL(0b0001)
#define ID_AA64PFR0_EL1_AdvSIMD_NI                      UL(0b1111)

#define ID_AA64PFR0_EL1_FP                              GENMASK(19, 16)
#define ID_AA64PFR0_EL1_FP_MASK                         GENMASK(19, 16)
#define ID_AA64PFR0_EL1_FP_SHIFT                        16
#define ID_AA64PFR0_EL1_FP_WIDTH                        4
#define ID_AA64PFR0_EL1_FP_SIGNED                       true
#define ID_AA64PFR0_EL1_FP_IMP                          UL(0b0000)
#define ID_AA64PFR0_EL1_FP_FP16                         UL(0b0001)
#define ID_AA64PFR0_EL1_FP_NI                           UL(0b1111)

#define ID_AA64PFR0_EL1_EL3                             GENMASK(15, 12)
#define ID_AA64PFR0_EL1_EL3_MASK                        GENMASK(15, 12)
#define ID_AA64PFR0_EL1_EL3_SHIFT                       12
#define ID_AA64PFR0_EL1_EL3_WIDTH                       4
#define ID_AA64PFR0_EL1_EL3_SIGNED                      false
#define ID_AA64PFR0_EL1_EL3_NI                          UL(0b0000)
#define ID_AA64PFR0_EL1_EL3_IMP                         UL(0b0001)
#define ID_AA64PFR0_EL1_EL3_AARCH32                     UL(0b0010)

#define ID_AA64PFR0_EL1_EL2                             GENMASK(11, 8)
#define ID_AA64PFR0_EL1_EL2_MASK                        GENMASK(11, 8)
#define ID_AA64PFR0_EL1_EL2_SHIFT                       8
#define ID_AA64PFR0_EL1_EL2_WIDTH                       4
#define ID_AA64PFR0_EL1_EL2_SIGNED                      false
#define ID_AA64PFR0_EL1_EL2_NI                          UL(0b0000)
#define ID_AA64PFR0_EL1_EL2_IMP                         UL(0b0001)
#define ID_AA64PFR0_EL1_EL2_AARCH32                     UL(0b0010)

#define ID_AA64PFR0_EL1_EL1                             GENMASK(7, 4)
#define ID_AA64PFR0_EL1_EL1_MASK                        GENMASK(7, 4)
#define ID_AA64PFR0_EL1_EL1_SHIFT                       4
#define ID_AA64PFR0_EL1_EL1_WIDTH                       4
#define ID_AA64PFR0_EL1_EL1_SIGNED                      false
#define ID_AA64PFR0_EL1_EL1_IMP                         UL(0b0001)
#define ID_AA64PFR0_EL1_EL1_AARCH32                     UL(0b0010)

#define ID_AA64PFR0_EL1_EL0                             GENMASK(3, 0)
#define ID_AA64PFR0_EL1_EL0_MASK                        GENMASK(3, 0)
#define ID_AA64PFR0_EL1_EL0_SHIFT                       0
#define ID_AA64PFR0_EL1_EL0_WIDTH                       4
#define ID_AA64PFR0_EL1_EL0_SIGNED                      false
#define ID_AA64PFR0_EL1_EL0_IMP                         UL(0b0001)
#define ID_AA64PFR0_EL1_EL0_AARCH32                     UL(0b0010)

#define ID_AA64PFR0_EL1_RES0                            (UL(0))
#define ID_AA64PFR0_EL1_RES1                            (UL(0))
#define ID_AA64PFR0_EL1_UNKN                            (UL(0))

#define REG_ID_AA64PFR1_EL1                             S3_0_C0_C4_1
#define SYS_ID_AA64PFR1_EL1                             sys_reg(3, 0, 0, 4, 1)
#define SYS_ID_AA64PFR1_EL1_Op0                         3
#define SYS_ID_AA64PFR1_EL1_Op1                         0
#define SYS_ID_AA64PFR1_EL1_CRn                         0
#define SYS_ID_AA64PFR1_EL1_CRm                         4
#define SYS_ID_AA64PFR1_EL1_Op2                         1

#define ID_AA64PFR1_EL1_PFAR                            GENMASK(63, 60)
#define ID_AA64PFR1_EL1_PFAR_MASK                       GENMASK(63, 60)
#define ID_AA64PFR1_EL1_PFAR_SHIFT                      60
#define ID_AA64PFR1_EL1_PFAR_WIDTH                      4
#define ID_AA64PFR1_EL1_PFAR_SIGNED                     false
#define ID_AA64PFR1_EL1_PFAR_NI                         UL(0b0000)
#define ID_AA64PFR1_EL1_PFAR_IMP                        UL(0b0001)

#define ID_AA64PFR1_EL1_DF2                             GENMASK(59, 56)
#define ID_AA64PFR1_EL1_DF2_MASK                        GENMASK(59, 56)
#define ID_AA64PFR1_EL1_DF2_SHIFT                       56
#define ID_AA64PFR1_EL1_DF2_WIDTH                       4
#define ID_AA64PFR1_EL1_DF2_SIGNED                      false
#define ID_AA64PFR1_EL1_DF2_NI                          UL(0b0000)
#define ID_AA64PFR1_EL1_DF2_IMP                         UL(0b0001)

#define ID_AA64PFR1_EL1_MTEX                            GENMASK(55, 52)
#define ID_AA64PFR1_EL1_MTEX_MASK                       GENMASK(55, 52)
#define ID_AA64PFR1_EL1_MTEX_SHIFT                      52
#define ID_AA64PFR1_EL1_MTEX_WIDTH                      4
#define ID_AA64PFR1_EL1_MTEX_SIGNED                     false
#define ID_AA64PFR1_EL1_MTEX_MTE                        UL(0b0000)
#define ID_AA64PFR1_EL1_MTEX_MTE4                       UL(0b0001)

#define ID_AA64PFR1_EL1_THE                             GENMASK(51, 48)
#define ID_AA64PFR1_EL1_THE_MASK                        GENMASK(51, 48)
#define ID_AA64PFR1_EL1_THE_SHIFT                       48
#define ID_AA64PFR1_EL1_THE_WIDTH                       4
#define ID_AA64PFR1_EL1_THE_SIGNED                      false
#define ID_AA64PFR1_EL1_THE_NI                          UL(0b0000)
#define ID_AA64PFR1_EL1_THE_IMP                         UL(0b0001)

#define ID_AA64PFR1_EL1_GCS                             GENMASK(47, 44)
#define ID_AA64PFR1_EL1_GCS_MASK                        GENMASK(47, 44)
#define ID_AA64PFR1_EL1_GCS_SHIFT                       44
#define ID_AA64PFR1_EL1_GCS_WIDTH                       4
#define ID_AA64PFR1_EL1_GCS_SIGNED                      false
#define ID_AA64PFR1_EL1_GCS_NI                          UL(0b0000)
#define ID_AA64PFR1_EL1_GCS_IMP                         UL(0b0001)

#define ID_AA64PFR1_EL1_MTE_frac                        GENMASK(43, 40)
#define ID_AA64PFR1_EL1_MTE_frac_MASK                   GENMASK(43, 40)
#define ID_AA64PFR1_EL1_MTE_frac_SHIFT                  40
#define ID_AA64PFR1_EL1_MTE_frac_WIDTH                  4
#define ID_AA64PFR1_EL1_MTE_frac_ASYNC                  UL(0b0000)
#define ID_AA64PFR1_EL1_MTE_frac_NI                     UL(0b1111)

#define ID_AA64PFR1_EL1_NMI                             GENMASK(39, 36)
#define ID_AA64PFR1_EL1_NMI_MASK                        GENMASK(39, 36)
#define ID_AA64PFR1_EL1_NMI_SHIFT                       36
#define ID_AA64PFR1_EL1_NMI_WIDTH                       4
#define ID_AA64PFR1_EL1_NMI_SIGNED                      false
#define ID_AA64PFR1_EL1_NMI_NI                          UL(0b0000)
#define ID_AA64PFR1_EL1_NMI_IMP                         UL(0b0001)

#define ID_AA64PFR1_EL1_CSV2_frac                       GENMASK(35, 32)
#define ID_AA64PFR1_EL1_CSV2_frac_MASK                  GENMASK(35, 32)
#define ID_AA64PFR1_EL1_CSV2_frac_SHIFT                 32
#define ID_AA64PFR1_EL1_CSV2_frac_WIDTH                 4
#define ID_AA64PFR1_EL1_CSV2_frac_SIGNED                false
#define ID_AA64PFR1_EL1_CSV2_frac_NI                    UL(0b0000)
#define ID_AA64PFR1_EL1_CSV2_frac_CSV2_1p1              UL(0b0001)
#define ID_AA64PFR1_EL1_CSV2_frac_CSV2_1p2              UL(0b0010)

#define ID_AA64PFR1_EL1_RNDR_trap                       GENMASK(31, 28)
#define ID_AA64PFR1_EL1_RNDR_trap_MASK                  GENMASK(31, 28)
#define ID_AA64PFR1_EL1_RNDR_trap_SHIFT                 28
#define ID_AA64PFR1_EL1_RNDR_trap_WIDTH                 4
#define ID_AA64PFR1_EL1_RNDR_trap_SIGNED                false
#define ID_AA64PFR1_EL1_RNDR_trap_NI                    UL(0b0000)
#define ID_AA64PFR1_EL1_RNDR_trap_IMP                   UL(0b0001)

#define ID_AA64PFR1_EL1_SME                             GENMASK(27, 24)
#define ID_AA64PFR1_EL1_SME_MASK                        GENMASK(27, 24)
#define ID_AA64PFR1_EL1_SME_SHIFT                       24
#define ID_AA64PFR1_EL1_SME_WIDTH                       4
#define ID_AA64PFR1_EL1_SME_SIGNED                      false
#define ID_AA64PFR1_EL1_SME_NI                          UL(0b0000)
#define ID_AA64PFR1_EL1_SME_IMP                         UL(0b0001)
#define ID_AA64PFR1_EL1_SME_SME2                        UL(0b0010)

#define ID_AA64PFR1_EL1_MPAM_frac                       GENMASK(19, 16)
#define ID_AA64PFR1_EL1_MPAM_frac_MASK                  GENMASK(19, 16)
#define ID_AA64PFR1_EL1_MPAM_frac_SHIFT                 16
#define ID_AA64PFR1_EL1_MPAM_frac_WIDTH                 4
#define ID_AA64PFR1_EL1_MPAM_frac_SIGNED                false
#define ID_AA64PFR1_EL1_MPAM_frac_MINOR_0               UL(0b0000)
#define ID_AA64PFR1_EL1_MPAM_frac_MINOR_1               UL(0b0001)

#define ID_AA64PFR1_EL1_RAS_frac                        GENMASK(15, 12)
#define ID_AA64PFR1_EL1_RAS_frac_MASK                   GENMASK(15, 12)
#define ID_AA64PFR1_EL1_RAS_frac_SHIFT                  12
#define ID_AA64PFR1_EL1_RAS_frac_WIDTH                  4
#define ID_AA64PFR1_EL1_RAS_frac_SIGNED                 false
#define ID_AA64PFR1_EL1_RAS_frac_NI                     UL(0b0000)
#define ID_AA64PFR1_EL1_RAS_frac_RASv1p1                UL(0b0001)

#define ID_AA64PFR1_EL1_MTE                             GENMASK(11, 8)
#define ID_AA64PFR1_EL1_MTE_MASK                        GENMASK(11, 8)
#define ID_AA64PFR1_EL1_MTE_SHIFT                       8
#define ID_AA64PFR1_EL1_MTE_WIDTH                       4
#define ID_AA64PFR1_EL1_MTE_SIGNED                      false
#define ID_AA64PFR1_EL1_MTE_NI                          UL(0b0000)
#define ID_AA64PFR1_EL1_MTE_IMP                         UL(0b0001)
#define ID_AA64PFR1_EL1_MTE_MTE2                        UL(0b0010)
#define ID_AA64PFR1_EL1_MTE_MTE3                        UL(0b0011)

#define ID_AA64PFR1_EL1_SSBS                            GENMASK(7, 4)
#define ID_AA64PFR1_EL1_SSBS_MASK                       GENMASK(7, 4)
#define ID_AA64PFR1_EL1_SSBS_SHIFT                      4
#define ID_AA64PFR1_EL1_SSBS_WIDTH                      4
#define ID_AA64PFR1_EL1_SSBS_SIGNED                     false
#define ID_AA64PFR1_EL1_SSBS_NI                         UL(0b0000)
#define ID_AA64PFR1_EL1_SSBS_IMP                        UL(0b0001)
#define ID_AA64PFR1_EL1_SSBS_SSBS2                      UL(0b0010)

#define ID_AA64PFR1_EL1_BT                              GENMASK(3, 0)
#define ID_AA64PFR1_EL1_BT_MASK                         GENMASK(3, 0)
#define ID_AA64PFR1_EL1_BT_SHIFT                        0
#define ID_AA64PFR1_EL1_BT_WIDTH                        4
#define ID_AA64PFR1_EL1_BT_SIGNED                       false
#define ID_AA64PFR1_EL1_BT_NI                           UL(0b0000)
#define ID_AA64PFR1_EL1_BT_IMP                          UL(0b0001)

#define ID_AA64PFR1_EL1_RES0                            (UL(0) | GENMASK_ULL(23, 20))
#define ID_AA64PFR1_EL1_RES1                            (UL(0))
#define ID_AA64PFR1_EL1_UNKN                            (UL(0))

#define REG_ID_AA64PFR2_EL1                             S3_0_C0_C4_2
#define SYS_ID_AA64PFR2_EL1                             sys_reg(3, 0, 0, 4, 2)
#define SYS_ID_AA64PFR2_EL1_Op0                         3
#define SYS_ID_AA64PFR2_EL1_Op1                         0
#define SYS_ID_AA64PFR2_EL1_CRn                         0
#define SYS_ID_AA64PFR2_EL1_CRm                         4
#define SYS_ID_AA64PFR2_EL1_Op2                         2

#define ID_AA64PFR2_EL1_FPMR                            GENMASK(35, 32)
#define ID_AA64PFR2_EL1_FPMR_MASK                       GENMASK(35, 32)
#define ID_AA64PFR2_EL1_FPMR_SHIFT                      32
#define ID_AA64PFR2_EL1_FPMR_WIDTH                      4
#define ID_AA64PFR2_EL1_FPMR_SIGNED                     false
#define ID_AA64PFR2_EL1_FPMR_NI                         UL(0b0000)
#define ID_AA64PFR2_EL1_FPMR_IMP                        UL(0b0001)

#define ID_AA64PFR2_EL1_UINJ                            GENMASK(19, 16)
#define ID_AA64PFR2_EL1_UINJ_MASK                       GENMASK(19, 16)
#define ID_AA64PFR2_EL1_UINJ_SHIFT                      16
#define ID_AA64PFR2_EL1_UINJ_WIDTH                      4
#define ID_AA64PFR2_EL1_UINJ_SIGNED                     false
#define ID_AA64PFR2_EL1_UINJ_NI                         UL(0b0000)
#define ID_AA64PFR2_EL1_UINJ_IMP                        UL(0b0001)

#define ID_AA64PFR2_EL1_GCIE                            GENMASK(15, 12)
#define ID_AA64PFR2_EL1_GCIE_MASK                       GENMASK(15, 12)
#define ID_AA64PFR2_EL1_GCIE_SHIFT                      12
#define ID_AA64PFR2_EL1_GCIE_WIDTH                      4
#define ID_AA64PFR2_EL1_GCIE_SIGNED                     false
#define ID_AA64PFR2_EL1_GCIE_NI                         UL(0b0000)
#define ID_AA64PFR2_EL1_GCIE_IMP                        UL(0b0001)

#define ID_AA64PFR2_EL1_MTEFAR                          GENMASK(11, 8)
#define ID_AA64PFR2_EL1_MTEFAR_MASK                     GENMASK(11, 8)
#define ID_AA64PFR2_EL1_MTEFAR_SHIFT                    8
#define ID_AA64PFR2_EL1_MTEFAR_WIDTH                    4
#define ID_AA64PFR2_EL1_MTEFAR_SIGNED                   false
#define ID_AA64PFR2_EL1_MTEFAR_NI                       UL(0b0000)
#define ID_AA64PFR2_EL1_MTEFAR_IMP                      UL(0b0001)

#define ID_AA64PFR2_EL1_MTESTOREONLY                    GENMASK(7, 4)
#define ID_AA64PFR2_EL1_MTESTOREONLY_MASK               GENMASK(7, 4)
#define ID_AA64PFR2_EL1_MTESTOREONLY_SHIFT              4
#define ID_AA64PFR2_EL1_MTESTOREONLY_WIDTH              4
#define ID_AA64PFR2_EL1_MTESTOREONLY_SIGNED             false
#define ID_AA64PFR2_EL1_MTESTOREONLY_NI                 UL(0b0000)
#define ID_AA64PFR2_EL1_MTESTOREONLY_IMP                UL(0b0001)

#define ID_AA64PFR2_EL1_MTEPERM                         GENMASK(3, 0)
#define ID_AA64PFR2_EL1_MTEPERM_MASK                    GENMASK(3, 0)
#define ID_AA64PFR2_EL1_MTEPERM_SHIFT                   0
#define ID_AA64PFR2_EL1_MTEPERM_WIDTH                   4
#define ID_AA64PFR2_EL1_MTEPERM_SIGNED                  false
#define ID_AA64PFR2_EL1_MTEPERM_NI                      UL(0b0000)
#define ID_AA64PFR2_EL1_MTEPERM_IMP                     UL(0b0001)

#define ID_AA64PFR2_EL1_RES0                            (UL(0) | GENMASK_ULL(63, 36) | GENMASK_ULL(31, 20))
#define ID_AA64PFR2_EL1_RES1                            (UL(0))
#define ID_AA64PFR2_EL1_UNKN                            (UL(0))

#define BRBINFx_EL1_CCU                                 GENMASK(46, 46)
#define BRBINFx_EL1_CCU_MASK                            GENMASK(46, 46)
#define BRBINFx_EL1_CCU_SHIFT                           46
#define BRBINFx_EL1_CCU_WIDTH                           1

#define BRBINFx_EL1_CC_EXP                              GENMASK(45, 40)
#define BRBINFx_EL1_CC_EXP_MASK                         GENMASK(45, 40)
#define BRBINFx_EL1_CC_EXP_SHIFT                        40
#define BRBINFx_EL1_CC_EXP_WIDTH                        6

#define BRBINFx_EL1_CC_MANT                             GENMASK(39, 32)
#define BRBINFx_EL1_CC_MANT_MASK                        GENMASK(39, 32)
#define BRBINFx_EL1_CC_MANT_SHIFT                       32
#define BRBINFx_EL1_CC_MANT_WIDTH                       8

#define BRBINFx_EL1_LASTFAILED                          GENMASK(17, 17)
#define BRBINFx_EL1_LASTFAILED_MASK                     GENMASK(17, 17)
#define BRBINFx_EL1_LASTFAILED_SHIFT                    17
#define BRBINFx_EL1_LASTFAILED_WIDTH                    1

#define BRBINFx_EL1_T                                   GENMASK(16, 16)
#define BRBINFx_EL1_T_MASK                              GENMASK(16, 16)
#define BRBINFx_EL1_T_SHIFT                             16
#define BRBINFx_EL1_T_WIDTH                             1

#define BRBINFx_EL1_TYPE                                GENMASK(13, 8)
#define BRBINFx_EL1_TYPE_MASK                           GENMASK(13, 8)
#define BRBINFx_EL1_TYPE_SHIFT                          8
#define BRBINFx_EL1_TYPE_WIDTH                          6
#define BRBINFx_EL1_TYPE_DIRECT_UNCOND                  UL(0b000000)
#define BRBINFx_EL1_TYPE_INDIRECT                       UL(0b000001)
#define BRBINFx_EL1_TYPE_DIRECT_LINK                    UL(0b000010)
#define BRBINFx_EL1_TYPE_INDIRECT_LINK                  UL(0b000011)
#define BRBINFx_EL1_TYPE_RET                            UL(0b000101)
#define BRBINFx_EL1_TYPE_ERET                           UL(0b000111)
#define BRBINFx_EL1_TYPE_DIRECT_COND                    UL(0b001000)
#define BRBINFx_EL1_TYPE_DEBUG_HALT                     UL(0b100001)
#define BRBINFx_EL1_TYPE_CALL                           UL(0b100010)
#define BRBINFx_EL1_TYPE_TRAP                           UL(0b100011)
#define BRBINFx_EL1_TYPE_SERROR                         UL(0b100100)
#define BRBINFx_EL1_TYPE_INSN_DEBUG                     UL(0b100110)
#define BRBINFx_EL1_TYPE_DATA_DEBUG                     UL(0b100111)
#define BRBINFx_EL1_TYPE_ALIGN_FAULT                    UL(0b101010)
#define BRBINFx_EL1_TYPE_INSN_FAULT                     UL(0b101011)
#define BRBINFx_EL1_TYPE_DATA_FAULT                     UL(0b101100)
#define BRBINFx_EL1_TYPE_IRQ                            UL(0b101110)
#define BRBINFx_EL1_TYPE_FIQ                            UL(0b101111)
#define BRBINFx_EL1_TYPE_IMPDEF_TRAP_EL3                UL(0b110000)
#define BRBINFx_EL1_TYPE_DEBUG_EXIT                     UL(0b111001)

#define BRBINFx_EL1_EL                                  GENMASK(7, 6)
#define BRBINFx_EL1_EL_MASK                             GENMASK(7, 6)
#define BRBINFx_EL1_EL_SHIFT                            6
#define BRBINFx_EL1_EL_WIDTH                            2
#define BRBINFx_EL1_EL_EL0                              UL(0b00)
#define BRBINFx_EL1_EL_EL1                              UL(0b01)
#define BRBINFx_EL1_EL_EL2                              UL(0b10)
#define BRBINFx_EL1_EL_EL3                              UL(0b11)

#define BRBINFx_EL1_MPRED                               GENMASK(5, 5)
#define BRBINFx_EL1_MPRED_MASK                          GENMASK(5, 5)
#define BRBINFx_EL1_MPRED_SHIFT                         5
#define BRBINFx_EL1_MPRED_WIDTH                         1

#define BRBINFx_EL1_VALID                               GENMASK(1, 0)
#define BRBINFx_EL1_VALID_MASK                          GENMASK(1, 0)
#define BRBINFx_EL1_VALID_SHIFT                         0
#define BRBINFx_EL1_VALID_WIDTH                         2
#define BRBINFx_EL1_VALID_NONE                          UL(0b00)
#define BRBINFx_EL1_VALID_TARGET                        UL(0b01)
#define BRBINFx_EL1_VALID_SOURCE                        UL(0b10)
#define BRBINFx_EL1_VALID_FULL                          UL(0b11)

#define BRBINFx_EL1_RES0                                (UL(0) | GENMASK_ULL(63, 47) | GENMASK_ULL(31, 18) | GENMASK_ULL(15, 14) | GENMASK_ULL(4, 2))
#define BRBINFx_EL1_RES1                                (UL(0))
#define BRBINFx_EL1_UNKN                                (UL(0))

#define BRBCR_ELx_EXCEPTION                             GENMASK(23, 23)
#define BRBCR_ELx_EXCEPTION_MASK                        GENMASK(23, 23)
#define BRBCR_ELx_EXCEPTION_SHIFT                       23
#define BRBCR_ELx_EXCEPTION_WIDTH                       1

#define BRBCR_ELx_ERTN                                  GENMASK(22, 22)
#define BRBCR_ELx_ERTN_MASK                             GENMASK(22, 22)
#define BRBCR_ELx_ERTN_SHIFT                            22
#define BRBCR_ELx_ERTN_WIDTH                            1

#define BRBCR_ELx_FZPSS                                 GENMASK(9, 9)
#define BRBCR_ELx_FZPSS_MASK                            GENMASK(9, 9)
#define BRBCR_ELx_FZPSS_SHIFT                           9
#define BRBCR_ELx_FZPSS_WIDTH                           1

#define BRBCR_ELx_FZP                                   GENMASK(8, 8)
#define BRBCR_ELx_FZP_MASK                              GENMASK(8, 8)
#define BRBCR_ELx_FZP_SHIFT                             8
#define BRBCR_ELx_FZP_WIDTH                             1

#define BRBCR_ELx_TS                                    GENMASK(6, 5)
#define BRBCR_ELx_TS_MASK                               GENMASK(6, 5)
#define BRBCR_ELx_TS_SHIFT                              5
#define BRBCR_ELx_TS_WIDTH                              2
#define BRBCR_ELx_TS_VIRTUAL                            UL(0b01)
#define BRBCR_ELx_TS_GUEST_PHYSICAL                     UL(0b10)
#define BRBCR_ELx_TS_PHYSICAL                           UL(0b11)

#define BRBCR_ELx_MPRED                                 GENMASK(4, 4)
#define BRBCR_ELx_MPRED_MASK                            GENMASK(4, 4)
#define BRBCR_ELx_MPRED_SHIFT                           4
#define BRBCR_ELx_MPRED_WIDTH                           1

#define BRBCR_ELx_CC                                    GENMASK(3, 3)
#define BRBCR_ELx_CC_MASK                               GENMASK(3, 3)
#define BRBCR_ELx_CC_SHIFT                              3
#define BRBCR_ELx_CC_WIDTH                              1

#define BRBCR_ELx_ExBRE                                 GENMASK(1, 1)
#define BRBCR_ELx_ExBRE_MASK                            GENMASK(1, 1)
#define BRBCR_ELx_ExBRE_SHIFT                           1
#define BRBCR_ELx_ExBRE_WIDTH                           1

#define BRBCR_ELx_E0BRE                                 GENMASK(0, 0)
#define BRBCR_ELx_E0BRE_MASK                            GENMASK(0, 0)
#define BRBCR_ELx_E0BRE_SHIFT                           0
#define BRBCR_ELx_E0BRE_WIDTH                           1

#define BRBCR_ELx_RES0                                  (UL(0) | GENMASK_ULL(63, 24) | GENMASK_ULL(21, 10) | GENMASK_ULL(7, 7) | GENMASK_ULL(2, 2))
#define BRBCR_ELx_RES1                                  (UL(0))
#define BRBCR_ELx_UNKN                                  (UL(0))

#define REG_BRBCR_EL1                                   S2_1_C9_C0_0
#define SYS_BRBCR_EL1                                   sys_reg(2, 1, 9, 0, 0)
#define SYS_BRBCR_EL1_Op0                               2
#define SYS_BRBCR_EL1_Op1                               1
#define SYS_BRBCR_EL1_CRn                               9
#define SYS_BRBCR_EL1_CRm                               0
#define SYS_BRBCR_EL1_Op2                               0

/* For BRBCR_EL1 fields see BRBCR_ELx */

#define REG_BRBFCR_EL1                                  S2_1_C9_C0_1
#define SYS_BRBFCR_EL1                                  sys_reg(2, 1, 9, 0, 1)
#define SYS_BRBFCR_EL1_Op0                              2
#define SYS_BRBFCR_EL1_Op1                              1
#define SYS_BRBFCR_EL1_CRn                              9
#define SYS_BRBFCR_EL1_CRm                              0
#define SYS_BRBFCR_EL1_Op2                              1

#define BRBFCR_EL1_BANK                                 GENMASK(29, 28)
#define BRBFCR_EL1_BANK_MASK                            GENMASK(29, 28)
#define BRBFCR_EL1_BANK_SHIFT                           28
#define BRBFCR_EL1_BANK_WIDTH                           2
#define BRBFCR_EL1_BANK_BANK_0                          UL(0b00)
#define BRBFCR_EL1_BANK_BANK_1                          UL(0b01)

#define BRBFCR_EL1_CONDDIR                              GENMASK(22, 22)
#define BRBFCR_EL1_CONDDIR_MASK                         GENMASK(22, 22)
#define BRBFCR_EL1_CONDDIR_SHIFT                        22
#define BRBFCR_EL1_CONDDIR_WIDTH                        1

#define BRBFCR_EL1_DIRCALL                              GENMASK(21, 21)
#define BRBFCR_EL1_DIRCALL_MASK                         GENMASK(21, 21)
#define BRBFCR_EL1_DIRCALL_SHIFT                        21
#define BRBFCR_EL1_DIRCALL_WIDTH                        1

#define BRBFCR_EL1_INDCALL                              GENMASK(20, 20)
#define BRBFCR_EL1_INDCALL_MASK                         GENMASK(20, 20)
#define BRBFCR_EL1_INDCALL_SHIFT                        20
#define BRBFCR_EL1_INDCALL_WIDTH                        1

#define BRBFCR_EL1_RTN                                  GENMASK(19, 19)
#define BRBFCR_EL1_RTN_MASK                             GENMASK(19, 19)
#define BRBFCR_EL1_RTN_SHIFT                            19
#define BRBFCR_EL1_RTN_WIDTH                            1

#define BRBFCR_EL1_INDIRECT                             GENMASK(18, 18)
#define BRBFCR_EL1_INDIRECT_MASK                        GENMASK(18, 18)
#define BRBFCR_EL1_INDIRECT_SHIFT                       18
#define BRBFCR_EL1_INDIRECT_WIDTH                       1

#define BRBFCR_EL1_DIRECT                               GENMASK(17, 17)
#define BRBFCR_EL1_DIRECT_MASK                          GENMASK(17, 17)
#define BRBFCR_EL1_DIRECT_SHIFT                         17
#define BRBFCR_EL1_DIRECT_WIDTH                         1

#define BRBFCR_EL1_EnI                                  GENMASK(16, 16)
#define BRBFCR_EL1_EnI_MASK                             GENMASK(16, 16)
#define BRBFCR_EL1_EnI_SHIFT                            16
#define BRBFCR_EL1_EnI_WIDTH                            1

#define BRBFCR_EL1_PAUSED                               GENMASK(7, 7)
#define BRBFCR_EL1_PAUSED_MASK                          GENMASK(7, 7)
#define BRBFCR_EL1_PAUSED_SHIFT                         7
#define BRBFCR_EL1_PAUSED_WIDTH                         1

#define BRBFCR_EL1_LASTFAILED                           GENMASK(6, 6)
#define BRBFCR_EL1_LASTFAILED_MASK                      GENMASK(6, 6)
#define BRBFCR_EL1_LASTFAILED_SHIFT                     6
#define BRBFCR_EL1_LASTFAILED_WIDTH                     1

#define BRBFCR_EL1_RES0                                 (UL(0) | GENMASK_ULL(63, 30) | GENMASK_ULL(27, 23) | GENMASK_ULL(15, 8) | GENMASK_ULL(5, 0))
#define BRBFCR_EL1_RES1                                 (UL(0))
#define BRBFCR_EL1_UNKN                                 (UL(0))

#define REG_BRBTS_EL1                                   S2_1_C9_C0_2
#define SYS_BRBTS_EL1                                   sys_reg(2, 1, 9, 0, 2)
#define SYS_BRBTS_EL1_Op0                               2
#define SYS_BRBTS_EL1_Op1                               1
#define SYS_BRBTS_EL1_CRn                               9
#define SYS_BRBTS_EL1_CRm                               0
#define SYS_BRBTS_EL1_Op2                               2

#define BRBTS_EL1_TS                                    GENMASK(63, 0)
#define BRBTS_EL1_TS_MASK                               GENMASK(63, 0)
#define BRBTS_EL1_TS_SHIFT                              0
#define BRBTS_EL1_TS_WIDTH                              64

#define BRBTS_EL1_RES0                                  (UL(0))
#define BRBTS_EL1_RES1                                  (UL(0))
#define BRBTS_EL1_UNKN                                  (UL(0))

#define REG_BRBINFINJ_EL1                               S2_1_C9_C1_0
#define SYS_BRBINFINJ_EL1                               sys_reg(2, 1, 9, 1, 0)
#define SYS_BRBINFINJ_EL1_Op0                           2
#define SYS_BRBINFINJ_EL1_Op1                           1
#define SYS_BRBINFINJ_EL1_CRn                           9
#define SYS_BRBINFINJ_EL1_CRm                           1
#define SYS_BRBINFINJ_EL1_Op2                           0

/* For BRBINFINJ_EL1 fields see BRBINFx_EL1 */

#define REG_BRBSRCINJ_EL1                               S2_1_C9_C1_1
#define SYS_BRBSRCINJ_EL1                               sys_reg(2, 1, 9, 1, 1)
#define SYS_BRBSRCINJ_EL1_Op0                           2
#define SYS_BRBSRCINJ_EL1_Op1                           1
#define SYS_BRBSRCINJ_EL1_CRn                           9
#define SYS_BRBSRCINJ_EL1_CRm                           1
#define SYS_BRBSRCINJ_EL1_Op2                           1

#define BRBSRCINJ_EL1_ADDRESS                           GENMASK(63, 0)
#define BRBSRCINJ_EL1_ADDRESS_MASK                      GENMASK(63, 0)
#define BRBSRCINJ_EL1_ADDRESS_SHIFT                     0
#define BRBSRCINJ_EL1_ADDRESS_WIDTH                     64

#define BRBSRCINJ_EL1_RES0                              (UL(0))
#define BRBSRCINJ_EL1_RES1                              (UL(0))
#define BRBSRCINJ_EL1_UNKN                              (UL(0))

#define REG_BRBTGTINJ_EL1                               S2_1_C9_C1_2
#define SYS_BRBTGTINJ_EL1                               sys_reg(2, 1, 9, 1, 2)
#define SYS_BRBTGTINJ_EL1_Op0                           2
#define SYS_BRBTGTINJ_EL1_Op1                           1
#define SYS_BRBTGTINJ_EL1_CRn                           9
#define SYS_BRBTGTINJ_EL1_CRm                           1
#define SYS_BRBTGTINJ_EL1_Op2                           2

#define BRBTGTINJ_EL1_ADDRESS                           GENMASK(63, 0)
#define BRBTGTINJ_EL1_ADDRESS_MASK                      GENMASK(63, 0)
#define BRBTGTINJ_EL1_ADDRESS_SHIFT                     0
#define BRBTGTINJ_EL1_ADDRESS_WIDTH                     64

#define BRBTGTINJ_EL1_RES0                              (UL(0))
#define BRBTGTINJ_EL1_RES1                              (UL(0))
#define BRBTGTINJ_EL1_UNKN                              (UL(0))

#define REG_BRBIDR0_EL1                                 S2_1_C9_C2_0
#define SYS_BRBIDR0_EL1                                 sys_reg(2, 1, 9, 2, 0)
#define SYS_BRBIDR0_EL1_Op0                             2
#define SYS_BRBIDR0_EL1_Op1                             1
#define SYS_BRBIDR0_EL1_CRn                             9
#define SYS_BRBIDR0_EL1_CRm                             2
#define SYS_BRBIDR0_EL1_Op2                             0

#define BRBIDR0_EL1_CC                                  GENMASK(15, 12)
#define BRBIDR0_EL1_CC_MASK                             GENMASK(15, 12)
#define BRBIDR0_EL1_CC_SHIFT                            12
#define BRBIDR0_EL1_CC_WIDTH                            4
#define BRBIDR0_EL1_CC_20_BIT                           UL(0b0101)

#define BRBIDR0_EL1_FORMAT                              GENMASK(11, 8)
#define BRBIDR0_EL1_FORMAT_MASK                         GENMASK(11, 8)
#define BRBIDR0_EL1_FORMAT_SHIFT                        8
#define BRBIDR0_EL1_FORMAT_WIDTH                        4
#define BRBIDR0_EL1_FORMAT_FORMAT_0                     UL(0b0000)

#define BRBIDR0_EL1_NUMREC                              GENMASK(7, 0)
#define BRBIDR0_EL1_NUMREC_MASK                         GENMASK(7, 0)
#define BRBIDR0_EL1_NUMREC_SHIFT                        0
#define BRBIDR0_EL1_NUMREC_WIDTH                        8
#define BRBIDR0_EL1_NUMREC_8                            UL(0b00001000)
#define BRBIDR0_EL1_NUMREC_16                           UL(0b00010000)
#define BRBIDR0_EL1_NUMREC_32                           UL(0b00100000)
#define BRBIDR0_EL1_NUMREC_64                           UL(0b01000000)

#define BRBIDR0_EL1_RES0                                (UL(0) | GENMASK_ULL(63, 16))
#define BRBIDR0_EL1_RES1                                (UL(0))
#define BRBIDR0_EL1_UNKN                                (UL(0))

#define REG_BRBCR_EL2                                   S2_4_C9_C0_0
#define SYS_BRBCR_EL2                                   sys_reg(2, 4, 9, 0, 0)
#define SYS_BRBCR_EL2_Op0                               2
#define SYS_BRBCR_EL2_Op1                               4
#define SYS_BRBCR_EL2_CRn                               9
#define SYS_BRBCR_EL2_CRm                               0
#define SYS_BRBCR_EL2_Op2                               0

/* For BRBCR_EL2 fields see BRBCR_ELx */

#define REG_BRBCR_EL12                                  S2_5_C9_C0_0
#define SYS_BRBCR_EL12                                  sys_reg(2, 5, 9, 0, 0)
#define SYS_BRBCR_EL12_Op0                              2
#define SYS_BRBCR_EL12_Op1                              5
#define SYS_BRBCR_EL12_CRn                              9
#define SYS_BRBCR_EL12_CRm                              0
#define SYS_BRBCR_EL12_Op2                              0

/* For BRBCR_EL12 fields see BRBCR_ELx */

#define REG_ID_AA64ZFR0_EL1                             S3_0_C0_C4_4
#define SYS_ID_AA64ZFR0_EL1                             sys_reg(3, 0, 0, 4, 4)
#define SYS_ID_AA64ZFR0_EL1_Op0                         3
#define SYS_ID_AA64ZFR0_EL1_Op1                         0
#define SYS_ID_AA64ZFR0_EL1_CRn                         0
#define SYS_ID_AA64ZFR0_EL1_CRm                         4
#define SYS_ID_AA64ZFR0_EL1_Op2                         4

#define ID_AA64ZFR0_EL1_F64MM                           GENMASK(59, 56)
#define ID_AA64ZFR0_EL1_F64MM_MASK                      GENMASK(59, 56)
#define ID_AA64ZFR0_EL1_F64MM_SHIFT                     56
#define ID_AA64ZFR0_EL1_F64MM_WIDTH                     4
#define ID_AA64ZFR0_EL1_F64MM_SIGNED                    false
#define ID_AA64ZFR0_EL1_F64MM_NI                        UL(0b0000)
#define ID_AA64ZFR0_EL1_F64MM_IMP                       UL(0b0001)

#define ID_AA64ZFR0_EL1_F32MM                           GENMASK(55, 52)
#define ID_AA64ZFR0_EL1_F32MM_MASK                      GENMASK(55, 52)
#define ID_AA64ZFR0_EL1_F32MM_SHIFT                     52
#define ID_AA64ZFR0_EL1_F32MM_WIDTH                     4
#define ID_AA64ZFR0_EL1_F32MM_SIGNED                    false
#define ID_AA64ZFR0_EL1_F32MM_NI                        UL(0b0000)
#define ID_AA64ZFR0_EL1_F32MM_IMP                       UL(0b0001)

#define ID_AA64ZFR0_EL1_F16MM                           GENMASK(51, 48)
#define ID_AA64ZFR0_EL1_F16MM_MASK                      GENMASK(51, 48)
#define ID_AA64ZFR0_EL1_F16MM_SHIFT                     48
#define ID_AA64ZFR0_EL1_F16MM_WIDTH                     4
#define ID_AA64ZFR0_EL1_F16MM_SIGNED                    false
#define ID_AA64ZFR0_EL1_F16MM_NI                        UL(0b0000)
#define ID_AA64ZFR0_EL1_F16MM_IMP                       UL(0b0001)

#define ID_AA64ZFR0_EL1_I8MM                            GENMASK(47, 44)
#define ID_AA64ZFR0_EL1_I8MM_MASK                       GENMASK(47, 44)
#define ID_AA64ZFR0_EL1_I8MM_SHIFT                      44
#define ID_AA64ZFR0_EL1_I8MM_WIDTH                      4
#define ID_AA64ZFR0_EL1_I8MM_SIGNED                     false
#define ID_AA64ZFR0_EL1_I8MM_NI                         UL(0b0000)
#define ID_AA64ZFR0_EL1_I8MM_IMP                        UL(0b0001)

#define ID_AA64ZFR0_EL1_SM4                             GENMASK(43, 40)
#define ID_AA64ZFR0_EL1_SM4_MASK                        GENMASK(43, 40)
#define ID_AA64ZFR0_EL1_SM4_SHIFT                       40
#define ID_AA64ZFR0_EL1_SM4_WIDTH                       4
#define ID_AA64ZFR0_EL1_SM4_SIGNED                      false
#define ID_AA64ZFR0_EL1_SM4_NI                          UL(0b0000)
#define ID_AA64ZFR0_EL1_SM4_IMP                         UL(0b0001)

#define ID_AA64ZFR0_EL1_SHA3                            GENMASK(35, 32)
#define ID_AA64ZFR0_EL1_SHA3_MASK                       GENMASK(35, 32)
#define ID_AA64ZFR0_EL1_SHA3_SHIFT                      32
#define ID_AA64ZFR0_EL1_SHA3_WIDTH                      4
#define ID_AA64ZFR0_EL1_SHA3_SIGNED                     false
#define ID_AA64ZFR0_EL1_SHA3_NI                         UL(0b0000)
#define ID_AA64ZFR0_EL1_SHA3_IMP                        UL(0b0001)

#define ID_AA64ZFR0_EL1_B16B16                          GENMASK(27, 24)
#define ID_AA64ZFR0_EL1_B16B16_MASK                     GENMASK(27, 24)
#define ID_AA64ZFR0_EL1_B16B16_SHIFT                    24
#define ID_AA64ZFR0_EL1_B16B16_WIDTH                    4
#define ID_AA64ZFR0_EL1_B16B16_SIGNED                   false
#define ID_AA64ZFR0_EL1_B16B16_NI                       UL(0b0000)
#define ID_AA64ZFR0_EL1_B16B16_IMP                      UL(0b0001)
#define ID_AA64ZFR0_EL1_B16B16_BFSCALE                  UL(0b0010)

#define ID_AA64ZFR0_EL1_BF16                            GENMASK(23, 20)
#define ID_AA64ZFR0_EL1_BF16_MASK                       GENMASK(23, 20)
#define ID_AA64ZFR0_EL1_BF16_SHIFT                      20
#define ID_AA64ZFR0_EL1_BF16_WIDTH                      4
#define ID_AA64ZFR0_EL1_BF16_SIGNED                     false
#define ID_AA64ZFR0_EL1_BF16_NI                         UL(0b0000)
#define ID_AA64ZFR0_EL1_BF16_IMP                        UL(0b0001)
#define ID_AA64ZFR0_EL1_BF16_EBF16                      UL(0b0010)

#define ID_AA64ZFR0_EL1_BitPerm                         GENMASK(19, 16)
#define ID_AA64ZFR0_EL1_BitPerm_MASK                    GENMASK(19, 16)
#define ID_AA64ZFR0_EL1_BitPerm_SHIFT                   16
#define ID_AA64ZFR0_EL1_BitPerm_WIDTH                   4
#define ID_AA64ZFR0_EL1_BitPerm_SIGNED                  false
#define ID_AA64ZFR0_EL1_BitPerm_NI                      UL(0b0000)
#define ID_AA64ZFR0_EL1_BitPerm_IMP                     UL(0b0001)

#define ID_AA64ZFR0_EL1_EltPerm                         GENMASK(15, 12)
#define ID_AA64ZFR0_EL1_EltPerm_MASK                    GENMASK(15, 12)
#define ID_AA64ZFR0_EL1_EltPerm_SHIFT                   12
#define ID_AA64ZFR0_EL1_EltPerm_WIDTH                   4
#define ID_AA64ZFR0_EL1_EltPerm_SIGNED                  false
#define ID_AA64ZFR0_EL1_EltPerm_NI                      UL(0b0000)
#define ID_AA64ZFR0_EL1_EltPerm_IMP                     UL(0b0001)

#define ID_AA64ZFR0_EL1_AES                             GENMASK(7, 4)
#define ID_AA64ZFR0_EL1_AES_MASK                        GENMASK(7, 4)
#define ID_AA64ZFR0_EL1_AES_SHIFT                       4
#define ID_AA64ZFR0_EL1_AES_WIDTH                       4
#define ID_AA64ZFR0_EL1_AES_SIGNED                      false
#define ID_AA64ZFR0_EL1_AES_NI                          UL(0b0000)
#define ID_AA64ZFR0_EL1_AES_IMP                         UL(0b0001)
#define ID_AA64ZFR0_EL1_AES_PMULL128                    UL(0b0010)
#define ID_AA64ZFR0_EL1_AES_AES2                        UL(0b0011)

#define ID_AA64ZFR0_EL1_SVEver                          GENMASK(3, 0)
#define ID_AA64ZFR0_EL1_SVEver_MASK                     GENMASK(3, 0)
#define ID_AA64ZFR0_EL1_SVEver_SHIFT                    0
#define ID_AA64ZFR0_EL1_SVEver_WIDTH                    4
#define ID_AA64ZFR0_EL1_SVEver_SIGNED                   false
#define ID_AA64ZFR0_EL1_SVEver_IMP                      UL(0b0000)
#define ID_AA64ZFR0_EL1_SVEver_SVE2                     UL(0b0001)
#define ID_AA64ZFR0_EL1_SVEver_SVE2p1                   UL(0b0010)
#define ID_AA64ZFR0_EL1_SVEver_SVE2p2                   UL(0b0011)

#define ID_AA64ZFR0_EL1_RES0                            (UL(0) | GENMASK_ULL(63, 60) | GENMASK_ULL(39, 36) | GENMASK_ULL(31, 28) | GENMASK_ULL(11, 8))
#define ID_AA64ZFR0_EL1_RES1                            (UL(0))
#define ID_AA64ZFR0_EL1_UNKN                            (UL(0))

#define REG_ID_AA64SMFR0_EL1                            S3_0_C0_C4_5
#define SYS_ID_AA64SMFR0_EL1                            sys_reg(3, 0, 0, 4, 5)
#define SYS_ID_AA64SMFR0_EL1_Op0                        3
#define SYS_ID_AA64SMFR0_EL1_Op1                        0
#define SYS_ID_AA64SMFR0_EL1_CRn                        0
#define SYS_ID_AA64SMFR0_EL1_CRm                        4
#define SYS_ID_AA64SMFR0_EL1_Op2                        5

#define ID_AA64SMFR0_EL1_FA64                           GENMASK(63, 63)
#define ID_AA64SMFR0_EL1_FA64_MASK                      GENMASK(63, 63)
#define ID_AA64SMFR0_EL1_FA64_SHIFT                     63
#define ID_AA64SMFR0_EL1_FA64_WIDTH                     1
#define ID_AA64SMFR0_EL1_FA64_SIGNED                    false
#define ID_AA64SMFR0_EL1_FA64_NI                        UL(0b0)
#define ID_AA64SMFR0_EL1_FA64_IMP                       UL(0b1)

#define ID_AA64SMFR0_EL1_LUTv2                          GENMASK(60, 60)
#define ID_AA64SMFR0_EL1_LUTv2_MASK                     GENMASK(60, 60)
#define ID_AA64SMFR0_EL1_LUTv2_SHIFT                    60
#define ID_AA64SMFR0_EL1_LUTv2_WIDTH                    1
#define ID_AA64SMFR0_EL1_LUTv2_SIGNED                   false
#define ID_AA64SMFR0_EL1_LUTv2_NI                       UL(0b0)
#define ID_AA64SMFR0_EL1_LUTv2_IMP                      UL(0b1)

#define ID_AA64SMFR0_EL1_SMEver                         GENMASK(59, 56)
#define ID_AA64SMFR0_EL1_SMEver_MASK                    GENMASK(59, 56)
#define ID_AA64SMFR0_EL1_SMEver_SHIFT                   56
#define ID_AA64SMFR0_EL1_SMEver_WIDTH                   4
#define ID_AA64SMFR0_EL1_SMEver_SIGNED                  false
#define ID_AA64SMFR0_EL1_SMEver_SME                     UL(0b0000)
#define ID_AA64SMFR0_EL1_SMEver_SME2                    UL(0b0001)
#define ID_AA64SMFR0_EL1_SMEver_SME2p1                  UL(0b0010)
#define ID_AA64SMFR0_EL1_SMEver_SME2p2                  UL(0b0011)

#define ID_AA64SMFR0_EL1_I16I64                         GENMASK(55, 52)
#define ID_AA64SMFR0_EL1_I16I64_MASK                    GENMASK(55, 52)
#define ID_AA64SMFR0_EL1_I16I64_SHIFT                   52
#define ID_AA64SMFR0_EL1_I16I64_WIDTH                   4
#define ID_AA64SMFR0_EL1_I16I64_SIGNED                  false
#define ID_AA64SMFR0_EL1_I16I64_NI                      UL(0b0000)
#define ID_AA64SMFR0_EL1_I16I64_IMP                     UL(0b1111)

#define ID_AA64SMFR0_EL1_F64F64                         GENMASK(48, 48)
#define ID_AA64SMFR0_EL1_F64F64_MASK                    GENMASK(48, 48)
#define ID_AA64SMFR0_EL1_F64F64_SHIFT                   48
#define ID_AA64SMFR0_EL1_F64F64_WIDTH                   1
#define ID_AA64SMFR0_EL1_F64F64_SIGNED                  false
#define ID_AA64SMFR0_EL1_F64F64_NI                      UL(0b0)
#define ID_AA64SMFR0_EL1_F64F64_IMP                     UL(0b1)

#define ID_AA64SMFR0_EL1_I16I32                         GENMASK(47, 44)
#define ID_AA64SMFR0_EL1_I16I32_MASK                    GENMASK(47, 44)
#define ID_AA64SMFR0_EL1_I16I32_SHIFT                   44
#define ID_AA64SMFR0_EL1_I16I32_WIDTH                   4
#define ID_AA64SMFR0_EL1_I16I32_SIGNED                  false
#define ID_AA64SMFR0_EL1_I16I32_NI                      UL(0b0000)
#define ID_AA64SMFR0_EL1_I16I32_IMP                     UL(0b0101)

#define ID_AA64SMFR0_EL1_B16B16                         GENMASK(43, 43)
#define ID_AA64SMFR0_EL1_B16B16_MASK                    GENMASK(43, 43)
#define ID_AA64SMFR0_EL1_B16B16_SHIFT                   43
#define ID_AA64SMFR0_EL1_B16B16_WIDTH                   1
#define ID_AA64SMFR0_EL1_B16B16_SIGNED                  false
#define ID_AA64SMFR0_EL1_B16B16_NI                      UL(0b0)
#define ID_AA64SMFR0_EL1_B16B16_IMP                     UL(0b1)

#define ID_AA64SMFR0_EL1_F16F16                         GENMASK(42, 42)
#define ID_AA64SMFR0_EL1_F16F16_MASK                    GENMASK(42, 42)
#define ID_AA64SMFR0_EL1_F16F16_SHIFT                   42
#define ID_AA64SMFR0_EL1_F16F16_WIDTH                   1
#define ID_AA64SMFR0_EL1_F16F16_SIGNED                  false
#define ID_AA64SMFR0_EL1_F16F16_NI                      UL(0b0)
#define ID_AA64SMFR0_EL1_F16F16_IMP                     UL(0b1)

#define ID_AA64SMFR0_EL1_F8F16                          GENMASK(41, 41)
#define ID_AA64SMFR0_EL1_F8F16_MASK                     GENMASK(41, 41)
#define ID_AA64SMFR0_EL1_F8F16_SHIFT                    41
#define ID_AA64SMFR0_EL1_F8F16_WIDTH                    1
#define ID_AA64SMFR0_EL1_F8F16_SIGNED                   false
#define ID_AA64SMFR0_EL1_F8F16_NI                       UL(0b0)
#define ID_AA64SMFR0_EL1_F8F16_IMP                      UL(0b1)

#define ID_AA64SMFR0_EL1_F8F32                          GENMASK(40, 40)
#define ID_AA64SMFR0_EL1_F8F32_MASK                     GENMASK(40, 40)
#define ID_AA64SMFR0_EL1_F8F32_SHIFT                    40
#define ID_AA64SMFR0_EL1_F8F32_WIDTH                    1
#define ID_AA64SMFR0_EL1_F8F32_SIGNED                   false
#define ID_AA64SMFR0_EL1_F8F32_NI                       UL(0b0)
#define ID_AA64SMFR0_EL1_F8F32_IMP                      UL(0b1)

#define ID_AA64SMFR0_EL1_I8I32                          GENMASK(39, 36)
#define ID_AA64SMFR0_EL1_I8I32_MASK                     GENMASK(39, 36)
#define ID_AA64SMFR0_EL1_I8I32_SHIFT                    36
#define ID_AA64SMFR0_EL1_I8I32_WIDTH                    4
#define ID_AA64SMFR0_EL1_I8I32_SIGNED                   false
#define ID_AA64SMFR0_EL1_I8I32_NI                       UL(0b0000)
#define ID_AA64SMFR0_EL1_I8I32_IMP                      UL(0b1111)

#define ID_AA64SMFR0_EL1_F16F32                         GENMASK(35, 35)
#define ID_AA64SMFR0_EL1_F16F32_MASK                    GENMASK(35, 35)
#define ID_AA64SMFR0_EL1_F16F32_SHIFT                   35
#define ID_AA64SMFR0_EL1_F16F32_WIDTH                   1
#define ID_AA64SMFR0_EL1_F16F32_SIGNED                  false
#define ID_AA64SMFR0_EL1_F16F32_NI                      UL(0b0)
#define ID_AA64SMFR0_EL1_F16F32_IMP                     UL(0b1)

#define ID_AA64SMFR0_EL1_B16F32                         GENMASK(34, 34)
#define ID_AA64SMFR0_EL1_B16F32_MASK                    GENMASK(34, 34)
#define ID_AA64SMFR0_EL1_B16F32_SHIFT                   34
#define ID_AA64SMFR0_EL1_B16F32_WIDTH                   1
#define ID_AA64SMFR0_EL1_B16F32_SIGNED                  false
#define ID_AA64SMFR0_EL1_B16F32_NI                      UL(0b0)
#define ID_AA64SMFR0_EL1_B16F32_IMP                     UL(0b1)

#define ID_AA64SMFR0_EL1_BI32I32                        GENMASK(33, 33)
#define ID_AA64SMFR0_EL1_BI32I32_MASK                   GENMASK(33, 33)
#define ID_AA64SMFR0_EL1_BI32I32_SHIFT                  33
#define ID_AA64SMFR0_EL1_BI32I32_WIDTH                  1
#define ID_AA64SMFR0_EL1_BI32I32_SIGNED                 false
#define ID_AA64SMFR0_EL1_BI32I32_NI                     UL(0b0)
#define ID_AA64SMFR0_EL1_BI32I32_IMP                    UL(0b1)

#define ID_AA64SMFR0_EL1_F32F32                         GENMASK(32, 32)
#define ID_AA64SMFR0_EL1_F32F32_MASK                    GENMASK(32, 32)
#define ID_AA64SMFR0_EL1_F32F32_SHIFT                   32
#define ID_AA64SMFR0_EL1_F32F32_WIDTH                   1
#define ID_AA64SMFR0_EL1_F32F32_SIGNED                  false
#define ID_AA64SMFR0_EL1_F32F32_NI                      UL(0b0)
#define ID_AA64SMFR0_EL1_F32F32_IMP                     UL(0b1)

#define ID_AA64SMFR0_EL1_SF8FMA                         GENMASK(30, 30)
#define ID_AA64SMFR0_EL1_SF8FMA_MASK                    GENMASK(30, 30)
#define ID_AA64SMFR0_EL1_SF8FMA_SHIFT                   30
#define ID_AA64SMFR0_EL1_SF8FMA_WIDTH                   1
#define ID_AA64SMFR0_EL1_SF8FMA_SIGNED                  false
#define ID_AA64SMFR0_EL1_SF8FMA_NI                      UL(0b0)
#define ID_AA64SMFR0_EL1_SF8FMA_IMP                     UL(0b1)

#define ID_AA64SMFR0_EL1_SF8DP4                         GENMASK(29, 29)
#define ID_AA64SMFR0_EL1_SF8DP4_MASK                    GENMASK(29, 29)
#define ID_AA64SMFR0_EL1_SF8DP4_SHIFT                   29
#define ID_AA64SMFR0_EL1_SF8DP4_WIDTH                   1
#define ID_AA64SMFR0_EL1_SF8DP4_SIGNED                  false
#define ID_AA64SMFR0_EL1_SF8DP4_NI                      UL(0b0)
#define ID_AA64SMFR0_EL1_SF8DP4_IMP                     UL(0b1)

#define ID_AA64SMFR0_EL1_SF8DP2                         GENMASK(28, 28)
#define ID_AA64SMFR0_EL1_SF8DP2_MASK                    GENMASK(28, 28)
#define ID_AA64SMFR0_EL1_SF8DP2_SHIFT                   28
#define ID_AA64SMFR0_EL1_SF8DP2_WIDTH                   1
#define ID_AA64SMFR0_EL1_SF8DP2_SIGNED                  false
#define ID_AA64SMFR0_EL1_SF8DP2_NI                      UL(0b0)
#define ID_AA64SMFR0_EL1_SF8DP2_IMP                     UL(0b1)

#define ID_AA64SMFR0_EL1_SBitPerm                       GENMASK(25, 25)
#define ID_AA64SMFR0_EL1_SBitPerm_MASK                  GENMASK(25, 25)
#define ID_AA64SMFR0_EL1_SBitPerm_SHIFT                 25
#define ID_AA64SMFR0_EL1_SBitPerm_WIDTH                 1
#define ID_AA64SMFR0_EL1_SBitPerm_SIGNED                false
#define ID_AA64SMFR0_EL1_SBitPerm_NI                    UL(0b0)
#define ID_AA64SMFR0_EL1_SBitPerm_IMP                   UL(0b1)

#define ID_AA64SMFR0_EL1_AES                            GENMASK(24, 24)
#define ID_AA64SMFR0_EL1_AES_MASK                       GENMASK(24, 24)
#define ID_AA64SMFR0_EL1_AES_SHIFT                      24
#define ID_AA64SMFR0_EL1_AES_WIDTH                      1
#define ID_AA64SMFR0_EL1_AES_SIGNED                     false
#define ID_AA64SMFR0_EL1_AES_NI                         UL(0b0)
#define ID_AA64SMFR0_EL1_AES_IMP                        UL(0b1)

#define ID_AA64SMFR0_EL1_SFEXPA                         GENMASK(23, 23)
#define ID_AA64SMFR0_EL1_SFEXPA_MASK                    GENMASK(23, 23)
#define ID_AA64SMFR0_EL1_SFEXPA_SHIFT                   23
#define ID_AA64SMFR0_EL1_SFEXPA_WIDTH                   1
#define ID_AA64SMFR0_EL1_SFEXPA_SIGNED                  false
#define ID_AA64SMFR0_EL1_SFEXPA_NI                      UL(0b0)
#define ID_AA64SMFR0_EL1_SFEXPA_IMP                     UL(0b1)

#define ID_AA64SMFR0_EL1_STMOP                          GENMASK(16, 16)
#define ID_AA64SMFR0_EL1_STMOP_MASK                     GENMASK(16, 16)
#define ID_AA64SMFR0_EL1_STMOP_SHIFT                    16
#define ID_AA64SMFR0_EL1_STMOP_WIDTH                    1
#define ID_AA64SMFR0_EL1_STMOP_SIGNED                   false
#define ID_AA64SMFR0_EL1_STMOP_NI                       UL(0b0)
#define ID_AA64SMFR0_EL1_STMOP_IMP                      UL(0b1)

#define ID_AA64SMFR0_EL1_SMOP4                          GENMASK(0, 0)
#define ID_AA64SMFR0_EL1_SMOP4_MASK                     GENMASK(0, 0)
#define ID_AA64SMFR0_EL1_SMOP4_SHIFT                    0
#define ID_AA64SMFR0_EL1_SMOP4_WIDTH                    1
#define ID_AA64SMFR0_EL1_SMOP4_SIGNED                   false
#define ID_AA64SMFR0_EL1_SMOP4_NI                       UL(0b0)
#define ID_AA64SMFR0_EL1_SMOP4_IMP                      UL(0b1)

#define ID_AA64SMFR0_EL1_RES0                           (UL(0) | GENMASK_ULL(62, 61) | GENMASK_ULL(51, 49) | GENMASK_ULL(31, 31) | GENMASK_ULL(27, 26) | GENMASK_ULL(22, 17) | GENMASK_ULL(15, 1))
#define ID_AA64SMFR0_EL1_RES1                           (UL(0))
#define ID_AA64SMFR0_EL1_UNKN                           (UL(0))

#define REG_ID_AA64FPFR0_EL1                            S3_0_C0_C4_7
#define SYS_ID_AA64FPFR0_EL1                            sys_reg(3, 0, 0, 4, 7)
#define SYS_ID_AA64FPFR0_EL1_Op0                        3
#define SYS_ID_AA64FPFR0_EL1_Op1                        0
#define SYS_ID_AA64FPFR0_EL1_CRn                        0
#define SYS_ID_AA64FPFR0_EL1_CRm                        4
#define SYS_ID_AA64FPFR0_EL1_Op2                        7

#define ID_AA64FPFR0_EL1_F8CVT                          GENMASK(31, 31)
#define ID_AA64FPFR0_EL1_F8CVT_MASK                     GENMASK(31, 31)
#define ID_AA64FPFR0_EL1_F8CVT_SHIFT                    31
#define ID_AA64FPFR0_EL1_F8CVT_WIDTH                    1
#define ID_AA64FPFR0_EL1_F8CVT_SIGNED                   false
#define ID_AA64FPFR0_EL1_F8CVT_NI                       UL(0b0)
#define ID_AA64FPFR0_EL1_F8CVT_IMP                      UL(0b1)

#define ID_AA64FPFR0_EL1_F8FMA                          GENMASK(30, 30)
#define ID_AA64FPFR0_EL1_F8FMA_MASK                     GENMASK(30, 30)
#define ID_AA64FPFR0_EL1_F8FMA_SHIFT                    30
#define ID_AA64FPFR0_EL1_F8FMA_WIDTH                    1
#define ID_AA64FPFR0_EL1_F8FMA_SIGNED                   false
#define ID_AA64FPFR0_EL1_F8FMA_NI                       UL(0b0)
#define ID_AA64FPFR0_EL1_F8FMA_IMP                      UL(0b1)

#define ID_AA64FPFR0_EL1_F8DP4                          GENMASK(29, 29)
#define ID_AA64FPFR0_EL1_F8DP4_MASK                     GENMASK(29, 29)
#define ID_AA64FPFR0_EL1_F8DP4_SHIFT                    29
#define ID_AA64FPFR0_EL1_F8DP4_WIDTH                    1
#define ID_AA64FPFR0_EL1_F8DP4_SIGNED                   false
#define ID_AA64FPFR0_EL1_F8DP4_NI                       UL(0b0)
#define ID_AA64FPFR0_EL1_F8DP4_IMP                      UL(0b1)

#define ID_AA64FPFR0_EL1_F8DP2                          GENMASK(28, 28)
#define ID_AA64FPFR0_EL1_F8DP2_MASK                     GENMASK(28, 28)
#define ID_AA64FPFR0_EL1_F8DP2_SHIFT                    28
#define ID_AA64FPFR0_EL1_F8DP2_WIDTH                    1
#define ID_AA64FPFR0_EL1_F8DP2_SIGNED                   false
#define ID_AA64FPFR0_EL1_F8DP2_NI                       UL(0b0)
#define ID_AA64FPFR0_EL1_F8DP2_IMP                      UL(0b1)

#define ID_AA64FPFR0_EL1_F8MM8                          GENMASK(27, 27)
#define ID_AA64FPFR0_EL1_F8MM8_MASK                     GENMASK(27, 27)
#define ID_AA64FPFR0_EL1_F8MM8_SHIFT                    27
#define ID_AA64FPFR0_EL1_F8MM8_WIDTH                    1
#define ID_AA64FPFR0_EL1_F8MM8_SIGNED                   false
#define ID_AA64FPFR0_EL1_F8MM8_NI                       UL(0b0)
#define ID_AA64FPFR0_EL1_F8MM8_IMP                      UL(0b1)

#define ID_AA64FPFR0_EL1_F8MM4                          GENMASK(26, 26)
#define ID_AA64FPFR0_EL1_F8MM4_MASK                     GENMASK(26, 26)
#define ID_AA64FPFR0_EL1_F8MM4_SHIFT                    26
#define ID_AA64FPFR0_EL1_F8MM4_WIDTH                    1
#define ID_AA64FPFR0_EL1_F8MM4_SIGNED                   false
#define ID_AA64FPFR0_EL1_F8MM4_NI                       UL(0b0)
#define ID_AA64FPFR0_EL1_F8MM4_IMP                      UL(0b1)

#define ID_AA64FPFR0_EL1_F8E4M3                         GENMASK(1, 1)
#define ID_AA64FPFR0_EL1_F8E4M3_MASK                    GENMASK(1, 1)
#define ID_AA64FPFR0_EL1_F8E4M3_SHIFT                   1
#define ID_AA64FPFR0_EL1_F8E4M3_WIDTH                   1
#define ID_AA64FPFR0_EL1_F8E4M3_SIGNED                  false
#define ID_AA64FPFR0_EL1_F8E4M3_NI                      UL(0b0)
#define ID_AA64FPFR0_EL1_F8E4M3_IMP                     UL(0b1)

#define ID_AA64FPFR0_EL1_F8E5M2                         GENMASK(0, 0)
#define ID_AA64FPFR0_EL1_F8E5M2_MASK                    GENMASK(0, 0)
#define ID_AA64FPFR0_EL1_F8E5M2_SHIFT                   0
#define ID_AA64FPFR0_EL1_F8E5M2_WIDTH                   1
#define ID_AA64FPFR0_EL1_F8E5M2_SIGNED                  false
#define ID_AA64FPFR0_EL1_F8E5M2_NI                      UL(0b0)
#define ID_AA64FPFR0_EL1_F8E5M2_IMP                     UL(0b1)

#define ID_AA64FPFR0_EL1_RES0                           (UL(0) | GENMASK_ULL(63, 32) | GENMASK_ULL(25, 2))
#define ID_AA64FPFR0_EL1_RES1                           (UL(0))
#define ID_AA64FPFR0_EL1_UNKN                           (UL(0))

#define REG_ID_AA64DFR0_EL1                             S3_0_C0_C5_0
#define SYS_ID_AA64DFR0_EL1                             sys_reg(3, 0, 0, 5, 0)
#define SYS_ID_AA64DFR0_EL1_Op0                         3
#define SYS_ID_AA64DFR0_EL1_Op1                         0
#define SYS_ID_AA64DFR0_EL1_CRn                         0
#define SYS_ID_AA64DFR0_EL1_CRm                         5
#define SYS_ID_AA64DFR0_EL1_Op2                         0

#define ID_AA64DFR0_EL1_HPMN0                           GENMASK(63, 60)
#define ID_AA64DFR0_EL1_HPMN0_MASK                      GENMASK(63, 60)
#define ID_AA64DFR0_EL1_HPMN0_SHIFT                     60
#define ID_AA64DFR0_EL1_HPMN0_WIDTH                     4
#define ID_AA64DFR0_EL1_HPMN0_UNPREDICTABLE             UL(0b0000)
#define ID_AA64DFR0_EL1_HPMN0_DEF                       UL(0b0001)

#define ID_AA64DFR0_EL1_ExtTrcBuff                      GENMASK(59, 56)
#define ID_AA64DFR0_EL1_ExtTrcBuff_MASK                 GENMASK(59, 56)
#define ID_AA64DFR0_EL1_ExtTrcBuff_SHIFT                56
#define ID_AA64DFR0_EL1_ExtTrcBuff_WIDTH                4
#define ID_AA64DFR0_EL1_ExtTrcBuff_SIGNED               false
#define ID_AA64DFR0_EL1_ExtTrcBuff_NI                   UL(0b0000)
#define ID_AA64DFR0_EL1_ExtTrcBuff_IMP                  UL(0b0001)

#define ID_AA64DFR0_EL1_BRBE                            GENMASK(55, 52)
#define ID_AA64DFR0_EL1_BRBE_MASK                       GENMASK(55, 52)
#define ID_AA64DFR0_EL1_BRBE_SHIFT                      52
#define ID_AA64DFR0_EL1_BRBE_WIDTH                      4
#define ID_AA64DFR0_EL1_BRBE_SIGNED                     false
#define ID_AA64DFR0_EL1_BRBE_NI                         UL(0b0000)
#define ID_AA64DFR0_EL1_BRBE_IMP                        UL(0b0001)
#define ID_AA64DFR0_EL1_BRBE_BRBE_V1P1                  UL(0b0010)

#define ID_AA64DFR0_EL1_MTPMU                           GENMASK(51, 48)
#define ID_AA64DFR0_EL1_MTPMU_MASK                      GENMASK(51, 48)
#define ID_AA64DFR0_EL1_MTPMU_SHIFT                     48
#define ID_AA64DFR0_EL1_MTPMU_WIDTH                     4
#define ID_AA64DFR0_EL1_MTPMU_SIGNED                    true
#define ID_AA64DFR0_EL1_MTPMU_NI_IMPDEF                 UL(0b0000)
#define ID_AA64DFR0_EL1_MTPMU_IMP                       UL(0b0001)
#define ID_AA64DFR0_EL1_MTPMU_NI                        UL(0b1111)

#define ID_AA64DFR0_EL1_TraceBuffer                     GENMASK(47, 44)
#define ID_AA64DFR0_EL1_TraceBuffer_MASK                GENMASK(47, 44)
#define ID_AA64DFR0_EL1_TraceBuffer_SHIFT               44
#define ID_AA64DFR0_EL1_TraceBuffer_WIDTH               4
#define ID_AA64DFR0_EL1_TraceBuffer_SIGNED              false
#define ID_AA64DFR0_EL1_TraceBuffer_NI                  UL(0b0000)
#define ID_AA64DFR0_EL1_TraceBuffer_IMP                 UL(0b0001)
#define ID_AA64DFR0_EL1_TraceBuffer_TRBE_V1P1           UL(0b0010)

#define ID_AA64DFR0_EL1_TraceFilt                       GENMASK(43, 40)
#define ID_AA64DFR0_EL1_TraceFilt_MASK                  GENMASK(43, 40)
#define ID_AA64DFR0_EL1_TraceFilt_SHIFT                 40
#define ID_AA64DFR0_EL1_TraceFilt_WIDTH                 4
#define ID_AA64DFR0_EL1_TraceFilt_SIGNED                false
#define ID_AA64DFR0_EL1_TraceFilt_NI                    UL(0b0000)
#define ID_AA64DFR0_EL1_TraceFilt_IMP                   UL(0b0001)

#define ID_AA64DFR0_EL1_DoubleLock                      GENMASK(39, 36)
#define ID_AA64DFR0_EL1_DoubleLock_MASK                 GENMASK(39, 36)
#define ID_AA64DFR0_EL1_DoubleLock_SHIFT                36
#define ID_AA64DFR0_EL1_DoubleLock_WIDTH                4
#define ID_AA64DFR0_EL1_DoubleLock_SIGNED               true
#define ID_AA64DFR0_EL1_DoubleLock_IMP                  UL(0b0000)
#define ID_AA64DFR0_EL1_DoubleLock_NI                   UL(0b1111)

#define ID_AA64DFR0_EL1_PMSVer                          GENMASK(35, 32)
#define ID_AA64DFR0_EL1_PMSVer_MASK                     GENMASK(35, 32)
#define ID_AA64DFR0_EL1_PMSVer_SHIFT                    32
#define ID_AA64DFR0_EL1_PMSVer_WIDTH                    4
#define ID_AA64DFR0_EL1_PMSVer_SIGNED                   false
#define ID_AA64DFR0_EL1_PMSVer_NI                       UL(0b0000)
#define ID_AA64DFR0_EL1_PMSVer_IMP                      UL(0b0001)
#define ID_AA64DFR0_EL1_PMSVer_V1P1                     UL(0b0010)
#define ID_AA64DFR0_EL1_PMSVer_V1P2                     UL(0b0011)
#define ID_AA64DFR0_EL1_PMSVer_V1P3                     UL(0b0100)
#define ID_AA64DFR0_EL1_PMSVer_V1P4                     UL(0b0101)
#define ID_AA64DFR0_EL1_PMSVer_V1P5                     UL(0b0110)

#define ID_AA64DFR0_EL1_CTX_CMPs                        GENMASK(31, 28)
#define ID_AA64DFR0_EL1_CTX_CMPs_MASK                   GENMASK(31, 28)
#define ID_AA64DFR0_EL1_CTX_CMPs_SHIFT                  28
#define ID_AA64DFR0_EL1_CTX_CMPs_WIDTH                  4

#define ID_AA64DFR0_EL1_SEBEP                           GENMASK(27, 24)
#define ID_AA64DFR0_EL1_SEBEP_MASK                      GENMASK(27, 24)
#define ID_AA64DFR0_EL1_SEBEP_SHIFT                     24
#define ID_AA64DFR0_EL1_SEBEP_WIDTH                     4
#define ID_AA64DFR0_EL1_SEBEP_SIGNED                    false
#define ID_AA64DFR0_EL1_SEBEP_NI                        UL(0b0000)
#define ID_AA64DFR0_EL1_SEBEP_IMP                       UL(0b0001)

#define ID_AA64DFR0_EL1_WRPs                            GENMASK(23, 20)
#define ID_AA64DFR0_EL1_WRPs_MASK                       GENMASK(23, 20)
#define ID_AA64DFR0_EL1_WRPs_SHIFT                      20
#define ID_AA64DFR0_EL1_WRPs_WIDTH                      4

#define ID_AA64DFR0_EL1_PMSS                            GENMASK(19, 16)
#define ID_AA64DFR0_EL1_PMSS_MASK                       GENMASK(19, 16)
#define ID_AA64DFR0_EL1_PMSS_SHIFT                      16
#define ID_AA64DFR0_EL1_PMSS_WIDTH                      4
#define ID_AA64DFR0_EL1_PMSS_SIGNED                     false
#define ID_AA64DFR0_EL1_PMSS_NI                         UL(0b0000)
#define ID_AA64DFR0_EL1_PMSS_IMP                        UL(0b0001)

#define ID_AA64DFR0_EL1_BRPs                            GENMASK(15, 12)
#define ID_AA64DFR0_EL1_BRPs_MASK                       GENMASK(15, 12)
#define ID_AA64DFR0_EL1_BRPs_SHIFT                      12
#define ID_AA64DFR0_EL1_BRPs_WIDTH                      4

#define ID_AA64DFR0_EL1_PMUVer                          GENMASK(11, 8)
#define ID_AA64DFR0_EL1_PMUVer_MASK                     GENMASK(11, 8)
#define ID_AA64DFR0_EL1_PMUVer_SHIFT                    8
#define ID_AA64DFR0_EL1_PMUVer_WIDTH                    4
#define ID_AA64DFR0_EL1_PMUVer_SIGNED                   false
#define ID_AA64DFR0_EL1_PMUVer_NI                       UL(0b0000)
#define ID_AA64DFR0_EL1_PMUVer_IMP                      UL(0b0001)
#define ID_AA64DFR0_EL1_PMUVer_V3P1                     UL(0b0100)
#define ID_AA64DFR0_EL1_PMUVer_V3P4                     UL(0b0101)
#define ID_AA64DFR0_EL1_PMUVer_V3P5                     UL(0b0110)
#define ID_AA64DFR0_EL1_PMUVer_V3P7                     UL(0b0111)
#define ID_AA64DFR0_EL1_PMUVer_V3P8                     UL(0b1000)
#define ID_AA64DFR0_EL1_PMUVer_V3P9                     UL(0b1001)
#define ID_AA64DFR0_EL1_PMUVer_IMP_DEF                  UL(0b1111)

#define ID_AA64DFR0_EL1_TraceVer                        GENMASK(7, 4)
#define ID_AA64DFR0_EL1_TraceVer_MASK                   GENMASK(7, 4)
#define ID_AA64DFR0_EL1_TraceVer_SHIFT                  4
#define ID_AA64DFR0_EL1_TraceVer_WIDTH                  4
#define ID_AA64DFR0_EL1_TraceVer_SIGNED                 false
#define ID_AA64DFR0_EL1_TraceVer_NI                     UL(0b0000)
#define ID_AA64DFR0_EL1_TraceVer_IMP                    UL(0b0001)

#define ID_AA64DFR0_EL1_DebugVer                        GENMASK(3, 0)
#define ID_AA64DFR0_EL1_DebugVer_MASK                   GENMASK(3, 0)
#define ID_AA64DFR0_EL1_DebugVer_SHIFT                  0
#define ID_AA64DFR0_EL1_DebugVer_WIDTH                  4
#define ID_AA64DFR0_EL1_DebugVer_SIGNED                 false
#define ID_AA64DFR0_EL1_DebugVer_IMP                    UL(0b0110)
#define ID_AA64DFR0_EL1_DebugVer_VHE                    UL(0b0111)
#define ID_AA64DFR0_EL1_DebugVer_V8P2                   UL(0b1000)
#define ID_AA64DFR0_EL1_DebugVer_V8P4                   UL(0b1001)
#define ID_AA64DFR0_EL1_DebugVer_V8P8                   UL(0b1010)
#define ID_AA64DFR0_EL1_DebugVer_V8P9                   UL(0b1011)

#define ID_AA64DFR0_EL1_RES0                            (UL(0))
#define ID_AA64DFR0_EL1_RES1                            (UL(0))
#define ID_AA64DFR0_EL1_UNKN                            (UL(0))

#define REG_ID_AA64DFR1_EL1                             S3_0_C0_C5_1
#define SYS_ID_AA64DFR1_EL1                             sys_reg(3, 0, 0, 5, 1)
#define SYS_ID_AA64DFR1_EL1_Op0                         3
#define SYS_ID_AA64DFR1_EL1_Op1                         0
#define SYS_ID_AA64DFR1_EL1_CRn                         0
#define SYS_ID_AA64DFR1_EL1_CRm                         5
#define SYS_ID_AA64DFR1_EL1_Op2                         1

#define ID_AA64DFR1_EL1_ABL_CMPs                        GENMASK(63, 56)
#define ID_AA64DFR1_EL1_ABL_CMPs_MASK                   GENMASK(63, 56)
#define ID_AA64DFR1_EL1_ABL_CMPs_SHIFT                  56
#define ID_AA64DFR1_EL1_ABL_CMPs_WIDTH                  8

#define ID_AA64DFR1_EL1_DPFZS                           GENMASK(55, 52)
#define ID_AA64DFR1_EL1_DPFZS_MASK                      GENMASK(55, 52)
#define ID_AA64DFR1_EL1_DPFZS_SHIFT                     52
#define ID_AA64DFR1_EL1_DPFZS_WIDTH                     4
#define ID_AA64DFR1_EL1_DPFZS_SIGNED                    false
#define ID_AA64DFR1_EL1_DPFZS_IGNR                      UL(0b0000)
#define ID_AA64DFR1_EL1_DPFZS_FRZN                      UL(0b0001)

#define ID_AA64DFR1_EL1_EBEP                            GENMASK(51, 48)
#define ID_AA64DFR1_EL1_EBEP_MASK                       GENMASK(51, 48)
#define ID_AA64DFR1_EL1_EBEP_SHIFT                      48
#define ID_AA64DFR1_EL1_EBEP_WIDTH                      4
#define ID_AA64DFR1_EL1_EBEP_SIGNED                     false
#define ID_AA64DFR1_EL1_EBEP_NI                         UL(0b0000)
#define ID_AA64DFR1_EL1_EBEP_IMP                        UL(0b0001)

#define ID_AA64DFR1_EL1_ITE                             GENMASK(47, 44)
#define ID_AA64DFR1_EL1_ITE_MASK                        GENMASK(47, 44)
#define ID_AA64DFR1_EL1_ITE_SHIFT                       44
#define ID_AA64DFR1_EL1_ITE_WIDTH                       4
#define ID_AA64DFR1_EL1_ITE_SIGNED                      false
#define ID_AA64DFR1_EL1_ITE_NI                          UL(0b0000)
#define ID_AA64DFR1_EL1_ITE_IMP                         UL(0b0001)

#define ID_AA64DFR1_EL1_ABLE                            GENMASK(43, 40)
#define ID_AA64DFR1_EL1_ABLE_MASK                       GENMASK(43, 40)
#define ID_AA64DFR1_EL1_ABLE_SHIFT                      40
#define ID_AA64DFR1_EL1_ABLE_WIDTH                      4
#define ID_AA64DFR1_EL1_ABLE_SIGNED                     false
#define ID_AA64DFR1_EL1_ABLE_NI                         UL(0b0000)
#define ID_AA64DFR1_EL1_ABLE_IMP                        UL(0b0001)

#define ID_AA64DFR1_EL1_PMICNTR                         GENMASK(39, 36)
#define ID_AA64DFR1_EL1_PMICNTR_MASK                    GENMASK(39, 36)
#define ID_AA64DFR1_EL1_PMICNTR_SHIFT                   36
#define ID_AA64DFR1_EL1_PMICNTR_WIDTH                   4
#define ID_AA64DFR1_EL1_PMICNTR_SIGNED                  false
#define ID_AA64DFR1_EL1_PMICNTR_NI                      UL(0b0000)
#define ID_AA64DFR1_EL1_PMICNTR_IMP                     UL(0b0001)

#define ID_AA64DFR1_EL1_SPMU                            GENMASK(35, 32)
#define ID_AA64DFR1_EL1_SPMU_MASK                       GENMASK(35, 32)
#define ID_AA64DFR1_EL1_SPMU_SHIFT                      32
#define ID_AA64DFR1_EL1_SPMU_WIDTH                      4
#define ID_AA64DFR1_EL1_SPMU_SIGNED                     false
#define ID_AA64DFR1_EL1_SPMU_NI                         UL(0b0000)
#define ID_AA64DFR1_EL1_SPMU_IMP                        UL(0b0001)
#define ID_AA64DFR1_EL1_SPMU_IMP_SPMZR                  UL(0b0010)

#define ID_AA64DFR1_EL1_CTX_CMPs                        GENMASK(31, 24)
#define ID_AA64DFR1_EL1_CTX_CMPs_MASK                   GENMASK(31, 24)
#define ID_AA64DFR1_EL1_CTX_CMPs_SHIFT                  24
#define ID_AA64DFR1_EL1_CTX_CMPs_WIDTH                  8

#define ID_AA64DFR1_EL1_WRPs                            GENMASK(23, 16)
#define ID_AA64DFR1_EL1_WRPs_MASK                       GENMASK(23, 16)
#define ID_AA64DFR1_EL1_WRPs_SHIFT                      16
#define ID_AA64DFR1_EL1_WRPs_WIDTH                      8

#define ID_AA64DFR1_EL1_BRPs                            GENMASK(15, 8)
#define ID_AA64DFR1_EL1_BRPs_MASK                       GENMASK(15, 8)
#define ID_AA64DFR1_EL1_BRPs_SHIFT                      8
#define ID_AA64DFR1_EL1_BRPs_WIDTH                      8

#define ID_AA64DFR1_EL1_SYSPMUID                        GENMASK(7, 0)
#define ID_AA64DFR1_EL1_SYSPMUID_MASK                   GENMASK(7, 0)
#define ID_AA64DFR1_EL1_SYSPMUID_SHIFT                  0
#define ID_AA64DFR1_EL1_SYSPMUID_WIDTH                  8

#define ID_AA64DFR1_EL1_RES0                            (UL(0))
#define ID_AA64DFR1_EL1_RES1                            (UL(0))
#define ID_AA64DFR1_EL1_UNKN                            (UL(0))

#define REG_ID_AA64DFR2_EL1                             S3_0_C0_C5_2
#define SYS_ID_AA64DFR2_EL1                             sys_reg(3, 0, 0, 5, 2)
#define SYS_ID_AA64DFR2_EL1_Op0                         3
#define SYS_ID_AA64DFR2_EL1_Op1                         0
#define SYS_ID_AA64DFR2_EL1_CRn                         0
#define SYS_ID_AA64DFR2_EL1_CRm                         5
#define SYS_ID_AA64DFR2_EL1_Op2                         2

#define ID_AA64DFR2_EL1_TRBE_EXC                        GENMASK(27, 24)
#define ID_AA64DFR2_EL1_TRBE_EXC_MASK                   GENMASK(27, 24)
#define ID_AA64DFR2_EL1_TRBE_EXC_SHIFT                  24
#define ID_AA64DFR2_EL1_TRBE_EXC_WIDTH                  4
#define ID_AA64DFR2_EL1_TRBE_EXC_SIGNED                 false
#define ID_AA64DFR2_EL1_TRBE_EXC_NI                     UL(0b0000)
#define ID_AA64DFR2_EL1_TRBE_EXC_IMP                    UL(0b0001)

#define ID_AA64DFR2_EL1_SPE_nVM                         GENMASK(23, 20)
#define ID_AA64DFR2_EL1_SPE_nVM_MASK                    GENMASK(23, 20)
#define ID_AA64DFR2_EL1_SPE_nVM_SHIFT                   20
#define ID_AA64DFR2_EL1_SPE_nVM_WIDTH                   4
#define ID_AA64DFR2_EL1_SPE_nVM_SIGNED                  false
#define ID_AA64DFR2_EL1_SPE_nVM_NI                      UL(0b0000)
#define ID_AA64DFR2_EL1_SPE_nVM_IMP                     UL(0b0001)

#define ID_AA64DFR2_EL1_SPE_EXC                         GENMASK(19, 16)
#define ID_AA64DFR2_EL1_SPE_EXC_MASK                    GENMASK(19, 16)
#define ID_AA64DFR2_EL1_SPE_EXC_SHIFT                   16
#define ID_AA64DFR2_EL1_SPE_EXC_WIDTH                   4
#define ID_AA64DFR2_EL1_SPE_EXC_SIGNED                  false
#define ID_AA64DFR2_EL1_SPE_EXC_NI                      UL(0b0000)
#define ID_AA64DFR2_EL1_SPE_EXC_IMP                     UL(0b0001)

#define ID_AA64DFR2_EL1_BWE                             GENMASK(7, 4)
#define ID_AA64DFR2_EL1_BWE_MASK                        GENMASK(7, 4)
#define ID_AA64DFR2_EL1_BWE_SHIFT                       4
#define ID_AA64DFR2_EL1_BWE_WIDTH                       4
#define ID_AA64DFR2_EL1_BWE_SIGNED                      false
#define ID_AA64DFR2_EL1_BWE_NI                          UL(0b0000)
#define ID_AA64DFR2_EL1_BWE_FEAT_BWE                    UL(0b0001)
#define ID_AA64DFR2_EL1_BWE_FEAT_BWE2                   UL(0b0010)

#define ID_AA64DFR2_EL1_STEP                            GENMASK(3, 0)
#define ID_AA64DFR2_EL1_STEP_MASK                       GENMASK(3, 0)
#define ID_AA64DFR2_EL1_STEP_SHIFT                      0
#define ID_AA64DFR2_EL1_STEP_WIDTH                      4
#define ID_AA64DFR2_EL1_STEP_SIGNED                     false
#define ID_AA64DFR2_EL1_STEP_NI                         UL(0b0000)
#define ID_AA64DFR2_EL1_STEP_IMP                        UL(0b0001)

#define ID_AA64DFR2_EL1_RES0                            (UL(0) | GENMASK_ULL(63, 28) | GENMASK_ULL(15, 8))
#define ID_AA64DFR2_EL1_RES1                            (UL(0))
#define ID_AA64DFR2_EL1_UNKN                            (UL(0))

#define REG_ID_AA64AFR0_EL1                             S3_0_C0_C5_4
#define SYS_ID_AA64AFR0_EL1                             sys_reg(3, 0, 0, 5, 4)
#define SYS_ID_AA64AFR0_EL1_Op0                         3
#define SYS_ID_AA64AFR0_EL1_Op1                         0
#define SYS_ID_AA64AFR0_EL1_CRn                         0
#define SYS_ID_AA64AFR0_EL1_CRm                         5
#define SYS_ID_AA64AFR0_EL1_Op2                         4

#define ID_AA64AFR0_EL1_IMPDEF7                         GENMASK(31, 28)
#define ID_AA64AFR0_EL1_IMPDEF7_MASK                    GENMASK(31, 28)
#define ID_AA64AFR0_EL1_IMPDEF7_SHIFT                   28
#define ID_AA64AFR0_EL1_IMPDEF7_WIDTH                   4

#define ID_AA64AFR0_EL1_IMPDEF6                         GENMASK(27, 24)
#define ID_AA64AFR0_EL1_IMPDEF6_MASK                    GENMASK(27, 24)
#define ID_AA64AFR0_EL1_IMPDEF6_SHIFT                   24
#define ID_AA64AFR0_EL1_IMPDEF6_WIDTH                   4

#define ID_AA64AFR0_EL1_IMPDEF5                         GENMASK(23, 20)
#define ID_AA64AFR0_EL1_IMPDEF5_MASK                    GENMASK(23, 20)
#define ID_AA64AFR0_EL1_IMPDEF5_SHIFT                   20
#define ID_AA64AFR0_EL1_IMPDEF5_WIDTH                   4

#define ID_AA64AFR0_EL1_IMPDEF4                         GENMASK(19, 16)
#define ID_AA64AFR0_EL1_IMPDEF4_MASK                    GENMASK(19, 16)
#define ID_AA64AFR0_EL1_IMPDEF4_SHIFT                   16
#define ID_AA64AFR0_EL1_IMPDEF4_WIDTH                   4

#define ID_AA64AFR0_EL1_IMPDEF3                         GENMASK(15, 12)
#define ID_AA64AFR0_EL1_IMPDEF3_MASK                    GENMASK(15, 12)
#define ID_AA64AFR0_EL1_IMPDEF3_SHIFT                   12
#define ID_AA64AFR0_EL1_IMPDEF3_WIDTH                   4

#define ID_AA64AFR0_EL1_IMPDEF2                         GENMASK(11, 8)
#define ID_AA64AFR0_EL1_IMPDEF2_MASK                    GENMASK(11, 8)
#define ID_AA64AFR0_EL1_IMPDEF2_SHIFT                   8
#define ID_AA64AFR0_EL1_IMPDEF2_WIDTH                   4

#define ID_AA64AFR0_EL1_IMPDEF1                         GENMASK(7, 4)
#define ID_AA64AFR0_EL1_IMPDEF1_MASK                    GENMASK(7, 4)
#define ID_AA64AFR0_EL1_IMPDEF1_SHIFT                   4
#define ID_AA64AFR0_EL1_IMPDEF1_WIDTH                   4

#define ID_AA64AFR0_EL1_IMPDEF0                         GENMASK(3, 0)
#define ID_AA64AFR0_EL1_IMPDEF0_MASK                    GENMASK(3, 0)
#define ID_AA64AFR0_EL1_IMPDEF0_SHIFT                   0
#define ID_AA64AFR0_EL1_IMPDEF0_WIDTH                   4

#define ID_AA64AFR0_EL1_RES0                            (UL(0) | GENMASK_ULL(63, 32))
#define ID_AA64AFR0_EL1_RES1                            (UL(0))
#define ID_AA64AFR0_EL1_UNKN                            (UL(0))

#define REG_ID_AA64AFR1_EL1                             S3_0_C0_C5_5
#define SYS_ID_AA64AFR1_EL1                             sys_reg(3, 0, 0, 5, 5)
#define SYS_ID_AA64AFR1_EL1_Op0                         3
#define SYS_ID_AA64AFR1_EL1_Op1                         0
#define SYS_ID_AA64AFR1_EL1_CRn                         0
#define SYS_ID_AA64AFR1_EL1_CRm                         5
#define SYS_ID_AA64AFR1_EL1_Op2                         5

#define ID_AA64AFR1_EL1_RES0                            (UL(0) | GENMASK_ULL(63, 0))
#define ID_AA64AFR1_EL1_RES1                            (UL(0))
#define ID_AA64AFR1_EL1_UNKN                            (UL(0))

#define REG_ID_AA64ISAR0_EL1                            S3_0_C0_C6_0
#define SYS_ID_AA64ISAR0_EL1                            sys_reg(3, 0, 0, 6, 0)
#define SYS_ID_AA64ISAR0_EL1_Op0                        3
#define SYS_ID_AA64ISAR0_EL1_Op1                        0
#define SYS_ID_AA64ISAR0_EL1_CRn                        0
#define SYS_ID_AA64ISAR0_EL1_CRm                        6
#define SYS_ID_AA64ISAR0_EL1_Op2                        0

#define ID_AA64ISAR0_EL1_RNDR                           GENMASK(63, 60)
#define ID_AA64ISAR0_EL1_RNDR_MASK                      GENMASK(63, 60)
#define ID_AA64ISAR0_EL1_RNDR_SHIFT                     60
#define ID_AA64ISAR0_EL1_RNDR_WIDTH                     4
#define ID_AA64ISAR0_EL1_RNDR_SIGNED                    false
#define ID_AA64ISAR0_EL1_RNDR_NI                        UL(0b0000)
#define ID_AA64ISAR0_EL1_RNDR_IMP                       UL(0b0001)

#define ID_AA64ISAR0_EL1_TLB                            GENMASK(59, 56)
#define ID_AA64ISAR0_EL1_TLB_MASK                       GENMASK(59, 56)
#define ID_AA64ISAR0_EL1_TLB_SHIFT                      56
#define ID_AA64ISAR0_EL1_TLB_WIDTH                      4
#define ID_AA64ISAR0_EL1_TLB_SIGNED                     false
#define ID_AA64ISAR0_EL1_TLB_NI                         UL(0b0000)
#define ID_AA64ISAR0_EL1_TLB_OS                         UL(0b0001)
#define ID_AA64ISAR0_EL1_TLB_RANGE                      UL(0b0010)

#define ID_AA64ISAR0_EL1_TS                             GENMASK(55, 52)
#define ID_AA64ISAR0_EL1_TS_MASK                        GENMASK(55, 52)
#define ID_AA64ISAR0_EL1_TS_SHIFT                       52
#define ID_AA64ISAR0_EL1_TS_WIDTH                       4
#define ID_AA64ISAR0_EL1_TS_SIGNED                      false
#define ID_AA64ISAR0_EL1_TS_NI                          UL(0b0000)
#define ID_AA64ISAR0_EL1_TS_FLAGM                       UL(0b0001)
#define ID_AA64ISAR0_EL1_TS_FLAGM2                      UL(0b0010)

#define ID_AA64ISAR0_EL1_FHM                            GENMASK(51, 48)
#define ID_AA64ISAR0_EL1_FHM_MASK                       GENMASK(51, 48)
#define ID_AA64ISAR0_EL1_FHM_SHIFT                      48
#define ID_AA64ISAR0_EL1_FHM_WIDTH                      4
#define ID_AA64ISAR0_EL1_FHM_SIGNED                     false
#define ID_AA64ISAR0_EL1_FHM_NI                         UL(0b0000)
#define ID_AA64ISAR0_EL1_FHM_IMP                        UL(0b0001)

#define ID_AA64ISAR0_EL1_DP                             GENMASK(47, 44)
#define ID_AA64ISAR0_EL1_DP_MASK                        GENMASK(47, 44)
#define ID_AA64ISAR0_EL1_DP_SHIFT                       44
#define ID_AA64ISAR0_EL1_DP_WIDTH                       4
#define ID_AA64ISAR0_EL1_DP_SIGNED                      false
#define ID_AA64ISAR0_EL1_DP_NI                          UL(0b0000)
#define ID_AA64ISAR0_EL1_DP_IMP                         UL(0b0001)

#define ID_AA64ISAR0_EL1_SM4                            GENMASK(43, 40)
#define ID_AA64ISAR0_EL1_SM4_MASK                       GENMASK(43, 40)
#define ID_AA64ISAR0_EL1_SM4_SHIFT                      40
#define ID_AA64ISAR0_EL1_SM4_WIDTH                      4
#define ID_AA64ISAR0_EL1_SM4_SIGNED                     false
#define ID_AA64ISAR0_EL1_SM4_NI                         UL(0b0000)
#define ID_AA64ISAR0_EL1_SM4_IMP                        UL(0b0001)

#define ID_AA64ISAR0_EL1_SM3                            GENMASK(39, 36)
#define ID_AA64ISAR0_EL1_SM3_MASK                       GENMASK(39, 36)
#define ID_AA64ISAR0_EL1_SM3_SHIFT                      36
#define ID_AA64ISAR0_EL1_SM3_WIDTH                      4
#define ID_AA64ISAR0_EL1_SM3_SIGNED                     false
#define ID_AA64ISAR0_EL1_SM3_NI                         UL(0b0000)
#define ID_AA64ISAR0_EL1_SM3_IMP                        UL(0b0001)

#define ID_AA64ISAR0_EL1_SHA3                           GENMASK(35, 32)
#define ID_AA64ISAR0_EL1_SHA3_MASK                      GENMASK(35, 32)
#define ID_AA64ISAR0_EL1_SHA3_SHIFT                     32
#define ID_AA64ISAR0_EL1_SHA3_WIDTH                     4
#define ID_AA64ISAR0_EL1_SHA3_SIGNED                    false
#define ID_AA64ISAR0_EL1_SHA3_NI                        UL(0b0000)
#define ID_AA64ISAR0_EL1_SHA3_IMP                       UL(0b0001)

#define ID_AA64ISAR0_EL1_RDM                            GENMASK(31, 28)
#define ID_AA64ISAR0_EL1_RDM_MASK                       GENMASK(31, 28)
#define ID_AA64ISAR0_EL1_RDM_SHIFT                      28
#define ID_AA64ISAR0_EL1_RDM_WIDTH                      4
#define ID_AA64ISAR0_EL1_RDM_SIGNED                     false
#define ID_AA64ISAR0_EL1_RDM_NI                         UL(0b0000)
#define ID_AA64ISAR0_EL1_RDM_IMP                        UL(0b0001)

#define ID_AA64ISAR0_EL1_TME                            GENMASK(27, 24)
#define ID_AA64ISAR0_EL1_TME_MASK                       GENMASK(27, 24)
#define ID_AA64ISAR0_EL1_TME_SHIFT                      24
#define ID_AA64ISAR0_EL1_TME_WIDTH                      4
#define ID_AA64ISAR0_EL1_TME_SIGNED                     false
#define ID_AA64ISAR0_EL1_TME_NI                         UL(0b0000)
#define ID_AA64ISAR0_EL1_TME_IMP                        UL(0b0001)

#define ID_AA64ISAR0_EL1_ATOMIC                         GENMASK(23, 20)
#define ID_AA64ISAR0_EL1_ATOMIC_MASK                    GENMASK(23, 20)
#define ID_AA64ISAR0_EL1_ATOMIC_SHIFT                   20
#define ID_AA64ISAR0_EL1_ATOMIC_WIDTH                   4
#define ID_AA64ISAR0_EL1_ATOMIC_SIGNED                  false
#define ID_AA64ISAR0_EL1_ATOMIC_NI                      UL(0b0000)
#define ID_AA64ISAR0_EL1_ATOMIC_IMP                     UL(0b0010)
#define ID_AA64ISAR0_EL1_ATOMIC_FEAT_LSE128             UL(0b0011)

#define ID_AA64ISAR0_EL1_CRC32                          GENMASK(19, 16)
#define ID_AA64ISAR0_EL1_CRC32_MASK                     GENMASK(19, 16)
#define ID_AA64ISAR0_EL1_CRC32_SHIFT                    16
#define ID_AA64ISAR0_EL1_CRC32_WIDTH                    4
#define ID_AA64ISAR0_EL1_CRC32_SIGNED                   false
#define ID_AA64ISAR0_EL1_CRC32_NI                       UL(0b0000)
#define ID_AA64ISAR0_EL1_CRC32_IMP                      UL(0b0001)

#define ID_AA64ISAR0_EL1_SHA2                           GENMASK(15, 12)
#define ID_AA64ISAR0_EL1_SHA2_MASK                      GENMASK(15, 12)
#define ID_AA64ISAR0_EL1_SHA2_SHIFT                     12
#define ID_AA64ISAR0_EL1_SHA2_WIDTH                     4
#define ID_AA64ISAR0_EL1_SHA2_SIGNED                    false
#define ID_AA64ISAR0_EL1_SHA2_NI                        UL(0b0000)
#define ID_AA64ISAR0_EL1_SHA2_SHA256                    UL(0b0001)
#define ID_AA64ISAR0_EL1_SHA2_SHA512                    UL(0b0010)

#define ID_AA64ISAR0_EL1_SHA1                           GENMASK(11, 8)
#define ID_AA64ISAR0_EL1_SHA1_MASK                      GENMASK(11, 8)
#define ID_AA64ISAR0_EL1_SHA1_SHIFT                     8
#define ID_AA64ISAR0_EL1_SHA1_WIDTH                     4
#define ID_AA64ISAR0_EL1_SHA1_SIGNED                    false
#define ID_AA64ISAR0_EL1_SHA1_NI                        UL(0b0000)
#define ID_AA64ISAR0_EL1_SHA1_IMP                       UL(0b0001)

#define ID_AA64ISAR0_EL1_AES                            GENMASK(7, 4)
#define ID_AA64ISAR0_EL1_AES_MASK                       GENMASK(7, 4)
#define ID_AA64ISAR0_EL1_AES_SHIFT                      4
#define ID_AA64ISAR0_EL1_AES_WIDTH                      4
#define ID_AA64ISAR0_EL1_AES_SIGNED                     false
#define ID_AA64ISAR0_EL1_AES_NI                         UL(0b0000)
#define ID_AA64ISAR0_EL1_AES_AES                        UL(0b0001)
#define ID_AA64ISAR0_EL1_AES_PMULL                      UL(0b0010)

#define ID_AA64ISAR0_EL1_RES0                           (UL(0) | GENMASK_ULL(3, 0))
#define ID_AA64ISAR0_EL1_RES1                           (UL(0))
#define ID_AA64ISAR0_EL1_UNKN                           (UL(0))

#define REG_ID_AA64ISAR1_EL1                            S3_0_C0_C6_1
#define SYS_ID_AA64ISAR1_EL1                            sys_reg(3, 0, 0, 6, 1)
#define SYS_ID_AA64ISAR1_EL1_Op0                        3
#define SYS_ID_AA64ISAR1_EL1_Op1                        0
#define SYS_ID_AA64ISAR1_EL1_CRn                        0
#define SYS_ID_AA64ISAR1_EL1_CRm                        6
#define SYS_ID_AA64ISAR1_EL1_Op2                        1

#define ID_AA64ISAR1_EL1_LS64                           GENMASK(63, 60)
#define ID_AA64ISAR1_EL1_LS64_MASK                      GENMASK(63, 60)
#define ID_AA64ISAR1_EL1_LS64_SHIFT                     60
#define ID_AA64ISAR1_EL1_LS64_WIDTH                     4
#define ID_AA64ISAR1_EL1_LS64_SIGNED                    false
#define ID_AA64ISAR1_EL1_LS64_NI                        UL(0b0000)
#define ID_AA64ISAR1_EL1_LS64_LS64                      UL(0b0001)
#define ID_AA64ISAR1_EL1_LS64_LS64_V                    UL(0b0010)
#define ID_AA64ISAR1_EL1_LS64_LS64_ACCDATA              UL(0b0011)
#define ID_AA64ISAR1_EL1_LS64_LS64WB                    UL(0b0100)

#define ID_AA64ISAR1_EL1_XS                             GENMASK(59, 56)
#define ID_AA64ISAR1_EL1_XS_MASK                        GENMASK(59, 56)
#define ID_AA64ISAR1_EL1_XS_SHIFT                       56
#define ID_AA64ISAR1_EL1_XS_WIDTH                       4
#define ID_AA64ISAR1_EL1_XS_SIGNED                      false
#define ID_AA64ISAR1_EL1_XS_NI                          UL(0b0000)
#define ID_AA64ISAR1_EL1_XS_IMP                         UL(0b0001)

#define ID_AA64ISAR1_EL1_I8MM                           GENMASK(55, 52)
#define ID_AA64ISAR1_EL1_I8MM_MASK                      GENMASK(55, 52)
#define ID_AA64ISAR1_EL1_I8MM_SHIFT                     52
#define ID_AA64ISAR1_EL1_I8MM_WIDTH                     4
#define ID_AA64ISAR1_EL1_I8MM_SIGNED                    false
#define ID_AA64ISAR1_EL1_I8MM_NI                        UL(0b0000)
#define ID_AA64ISAR1_EL1_I8MM_IMP                       UL(0b0001)

#define ID_AA64ISAR1_EL1_DGH                            GENMASK(51, 48)
#define ID_AA64ISAR1_EL1_DGH_MASK                       GENMASK(51, 48)
#define ID_AA64ISAR1_EL1_DGH_SHIFT                      48
#define ID_AA64ISAR1_EL1_DGH_WIDTH                      4
#define ID_AA64ISAR1_EL1_DGH_SIGNED                     false
#define ID_AA64ISAR1_EL1_DGH_NI                         UL(0b0000)
#define ID_AA64ISAR1_EL1_DGH_IMP                        UL(0b0001)

#define ID_AA64ISAR1_EL1_BF16                           GENMASK(47, 44)
#define ID_AA64ISAR1_EL1_BF16_MASK                      GENMASK(47, 44)
#define ID_AA64ISAR1_EL1_BF16_SHIFT                     44
#define ID_AA64ISAR1_EL1_BF16_WIDTH                     4
#define ID_AA64ISAR1_EL1_BF16_SIGNED                    false
#define ID_AA64ISAR1_EL1_BF16_NI                        UL(0b0000)
#define ID_AA64ISAR1_EL1_BF16_IMP                       UL(0b0001)
#define ID_AA64ISAR1_EL1_BF16_EBF16                     UL(0b0010)

#define ID_AA64ISAR1_EL1_SPECRES                        GENMASK(43, 40)
#define ID_AA64ISAR1_EL1_SPECRES_MASK                   GENMASK(43, 40)
#define ID_AA64ISAR1_EL1_SPECRES_SHIFT                  40
#define ID_AA64ISAR1_EL1_SPECRES_WIDTH                  4
#define ID_AA64ISAR1_EL1_SPECRES_SIGNED                 false
#define ID_AA64ISAR1_EL1_SPECRES_NI                     UL(0b0000)
#define ID_AA64ISAR1_EL1_SPECRES_IMP                    UL(0b0001)
#define ID_AA64ISAR1_EL1_SPECRES_COSP_RCTX              UL(0b0010)

#define ID_AA64ISAR1_EL1_SB                             GENMASK(39, 36)
#define ID_AA64ISAR1_EL1_SB_MASK                        GENMASK(39, 36)
#define ID_AA64ISAR1_EL1_SB_SHIFT                       36
#define ID_AA64ISAR1_EL1_SB_WIDTH                       4
#define ID_AA64ISAR1_EL1_SB_SIGNED                      false
#define ID_AA64ISAR1_EL1_SB_NI                          UL(0b0000)
#define ID_AA64ISAR1_EL1_SB_IMP                         UL(0b0001)

#define ID_AA64ISAR1_EL1_FRINTTS                        GENMASK(35, 32)
#define ID_AA64ISAR1_EL1_FRINTTS_MASK                   GENMASK(35, 32)
#define ID_AA64ISAR1_EL1_FRINTTS_SHIFT                  32
#define ID_AA64ISAR1_EL1_FRINTTS_WIDTH                  4
#define ID_AA64ISAR1_EL1_FRINTTS_SIGNED                 false
#define ID_AA64ISAR1_EL1_FRINTTS_NI                     UL(0b0000)
#define ID_AA64ISAR1_EL1_FRINTTS_IMP                    UL(0b0001)

#define ID_AA64ISAR1_EL1_GPI                            GENMASK(31, 28)
#define ID_AA64ISAR1_EL1_GPI_MASK                       GENMASK(31, 28)
#define ID_AA64ISAR1_EL1_GPI_SHIFT                      28
#define ID_AA64ISAR1_EL1_GPI_WIDTH                      4
#define ID_AA64ISAR1_EL1_GPI_SIGNED                     false
#define ID_AA64ISAR1_EL1_GPI_NI                         UL(0b0000)
#define ID_AA64ISAR1_EL1_GPI_IMP                        UL(0b0001)

#define ID_AA64ISAR1_EL1_GPA                            GENMASK(27, 24)
#define ID_AA64ISAR1_EL1_GPA_MASK                       GENMASK(27, 24)
#define ID_AA64ISAR1_EL1_GPA_SHIFT                      24
#define ID_AA64ISAR1_EL1_GPA_WIDTH                      4
#define ID_AA64ISAR1_EL1_GPA_SIGNED                     false
#define ID_AA64ISAR1_EL1_GPA_NI                         UL(0b0000)
#define ID_AA64ISAR1_EL1_GPA_IMP                        UL(0b0001)

#define ID_AA64ISAR1_EL1_LRCPC                          GENMASK(23, 20)
#define ID_AA64ISAR1_EL1_LRCPC_MASK                     GENMASK(23, 20)
#define ID_AA64ISAR1_EL1_LRCPC_SHIFT                    20
#define ID_AA64ISAR1_EL1_LRCPC_WIDTH                    4
#define ID_AA64ISAR1_EL1_LRCPC_SIGNED                   false
#define ID_AA64ISAR1_EL1_LRCPC_NI                       UL(0b0000)
#define ID_AA64ISAR1_EL1_LRCPC_IMP                      UL(0b0001)
#define ID_AA64ISAR1_EL1_LRCPC_LRCPC2                   UL(0b0010)
#define ID_AA64ISAR1_EL1_LRCPC_LRCPC3                   UL(0b0011)

#define ID_AA64ISAR1_EL1_FCMA                           GENMASK(19, 16)
#define ID_AA64ISAR1_EL1_FCMA_MASK                      GENMASK(19, 16)
#define ID_AA64ISAR1_EL1_FCMA_SHIFT                     16
#define ID_AA64ISAR1_EL1_FCMA_WIDTH                     4
#define ID_AA64ISAR1_EL1_FCMA_SIGNED                    false
#define ID_AA64ISAR1_EL1_FCMA_NI                        UL(0b0000)
#define ID_AA64ISAR1_EL1_FCMA_IMP                       UL(0b0001)

#define ID_AA64ISAR1_EL1_JSCVT                          GENMASK(15, 12)
#define ID_AA64ISAR1_EL1_JSCVT_MASK                     GENMASK(15, 12)
#define ID_AA64ISAR1_EL1_JSCVT_SHIFT                    12
#define ID_AA64ISAR1_EL1_JSCVT_WIDTH                    4
#define ID_AA64ISAR1_EL1_JSCVT_SIGNED                   false
#define ID_AA64ISAR1_EL1_JSCVT_NI                       UL(0b0000)
#define ID_AA64ISAR1_EL1_JSCVT_IMP                      UL(0b0001)

#define ID_AA64ISAR1_EL1_API                            GENMASK(11, 8)
#define ID_AA64ISAR1_EL1_API_MASK                       GENMASK(11, 8)
#define ID_AA64ISAR1_EL1_API_SHIFT                      8
#define ID_AA64ISAR1_EL1_API_WIDTH                      4
#define ID_AA64ISAR1_EL1_API_SIGNED                     false
#define ID_AA64ISAR1_EL1_API_NI                         UL(0b0000)
#define ID_AA64ISAR1_EL1_API_PAuth                      UL(0b0001)
#define ID_AA64ISAR1_EL1_API_EPAC                       UL(0b0010)
#define ID_AA64ISAR1_EL1_API_PAuth2                     UL(0b0011)
#define ID_AA64ISAR1_EL1_API_FPAC                       UL(0b0100)
#define ID_AA64ISAR1_EL1_API_FPACCOMBINE                UL(0b0101)
#define ID_AA64ISAR1_EL1_API_PAuth_LR                   UL(0b0110)

#define ID_AA64ISAR1_EL1_APA                            GENMASK(7, 4)
#define ID_AA64ISAR1_EL1_APA_MASK                       GENMASK(7, 4)
#define ID_AA64ISAR1_EL1_APA_SHIFT                      4
#define ID_AA64ISAR1_EL1_APA_WIDTH                      4
#define ID_AA64ISAR1_EL1_APA_SIGNED                     false
#define ID_AA64ISAR1_EL1_APA_NI                         UL(0b0000)
#define ID_AA64ISAR1_EL1_APA_PAuth                      UL(0b0001)
#define ID_AA64ISAR1_EL1_APA_EPAC                       UL(0b0010)
#define ID_AA64ISAR1_EL1_APA_PAuth2                     UL(0b0011)
#define ID_AA64ISAR1_EL1_APA_FPAC                       UL(0b0100)
#define ID_AA64ISAR1_EL1_APA_FPACCOMBINE                UL(0b0101)
#define ID_AA64ISAR1_EL1_APA_PAuth_LR                   UL(0b0110)

#define ID_AA64ISAR1_EL1_DPB                            GENMASK(3, 0)
#define ID_AA64ISAR1_EL1_DPB_MASK                       GENMASK(3, 0)
#define ID_AA64ISAR1_EL1_DPB_SHIFT                      0
#define ID_AA64ISAR1_EL1_DPB_WIDTH                      4
#define ID_AA64ISAR1_EL1_DPB_SIGNED                     false
#define ID_AA64ISAR1_EL1_DPB_NI                         UL(0b0000)
#define ID_AA64ISAR1_EL1_DPB_IMP                        UL(0b0001)
#define ID_AA64ISAR1_EL1_DPB_DPB2                       UL(0b0010)

#define ID_AA64ISAR1_EL1_RES0                           (UL(0))
#define ID_AA64ISAR1_EL1_RES1                           (UL(0))
#define ID_AA64ISAR1_EL1_UNKN                           (UL(0))

#define REG_ID_AA64ISAR2_EL1                            S3_0_C0_C6_2
#define SYS_ID_AA64ISAR2_EL1                            sys_reg(3, 0, 0, 6, 2)
#define SYS_ID_AA64ISAR2_EL1_Op0                        3
#define SYS_ID_AA64ISAR2_EL1_Op1                        0
#define SYS_ID_AA64ISAR2_EL1_CRn                        0
#define SYS_ID_AA64ISAR2_EL1_CRm                        6
#define SYS_ID_AA64ISAR2_EL1_Op2                        2

#define ID_AA64ISAR2_EL1_ATS1A                          GENMASK(63, 60)
#define ID_AA64ISAR2_EL1_ATS1A_MASK                     GENMASK(63, 60)
#define ID_AA64ISAR2_EL1_ATS1A_SHIFT                    60
#define ID_AA64ISAR2_EL1_ATS1A_WIDTH                    4
#define ID_AA64ISAR2_EL1_ATS1A_SIGNED                   false
#define ID_AA64ISAR2_EL1_ATS1A_NI                       UL(0b0000)
#define ID_AA64ISAR2_EL1_ATS1A_IMP                      UL(0b0001)

#define ID_AA64ISAR2_EL1_LUT                            GENMASK(59, 56)
#define ID_AA64ISAR2_EL1_LUT_MASK                       GENMASK(59, 56)
#define ID_AA64ISAR2_EL1_LUT_SHIFT                      56
#define ID_AA64ISAR2_EL1_LUT_WIDTH                      4
#define ID_AA64ISAR2_EL1_LUT_SIGNED                     false
#define ID_AA64ISAR2_EL1_LUT_NI                         UL(0b0000)
#define ID_AA64ISAR2_EL1_LUT_IMP                        UL(0b0001)

#define ID_AA64ISAR2_EL1_CSSC                           GENMASK(55, 52)
#define ID_AA64ISAR2_EL1_CSSC_MASK                      GENMASK(55, 52)
#define ID_AA64ISAR2_EL1_CSSC_SHIFT                     52
#define ID_AA64ISAR2_EL1_CSSC_WIDTH                     4
#define ID_AA64ISAR2_EL1_CSSC_SIGNED                    false
#define ID_AA64ISAR2_EL1_CSSC_NI                        UL(0b0000)
#define ID_AA64ISAR2_EL1_CSSC_IMP                       UL(0b0001)
#define ID_AA64ISAR2_EL1_CSSC_CMPBR                     UL(0b0010)

#define ID_AA64ISAR2_EL1_RPRFM                          GENMASK(51, 48)
#define ID_AA64ISAR2_EL1_RPRFM_MASK                     GENMASK(51, 48)
#define ID_AA64ISAR2_EL1_RPRFM_SHIFT                    48
#define ID_AA64ISAR2_EL1_RPRFM_WIDTH                    4
#define ID_AA64ISAR2_EL1_RPRFM_SIGNED                   false
#define ID_AA64ISAR2_EL1_RPRFM_NI                       UL(0b0000)
#define ID_AA64ISAR2_EL1_RPRFM_IMP                      UL(0b0001)

#define ID_AA64ISAR2_EL1_PCDPHINT                       GENMASK(47, 44)
#define ID_AA64ISAR2_EL1_PCDPHINT_MASK                  GENMASK(47, 44)
#define ID_AA64ISAR2_EL1_PCDPHINT_SHIFT                 44
#define ID_AA64ISAR2_EL1_PCDPHINT_WIDTH                 4
#define ID_AA64ISAR2_EL1_PCDPHINT_SIGNED                false
#define ID_AA64ISAR2_EL1_PCDPHINT_NI                    UL(0b0000)
#define ID_AA64ISAR2_EL1_PCDPHINT_IMP                   UL(0b0001)

#define ID_AA64ISAR2_EL1_PRFMSLC                        GENMASK(43, 40)
#define ID_AA64ISAR2_EL1_PRFMSLC_MASK                   GENMASK(43, 40)
#define ID_AA64ISAR2_EL1_PRFMSLC_SHIFT                  40
#define ID_AA64ISAR2_EL1_PRFMSLC_WIDTH                  4
#define ID_AA64ISAR2_EL1_PRFMSLC_SIGNED                 false
#define ID_AA64ISAR2_EL1_PRFMSLC_NI                     UL(0b0000)
#define ID_AA64ISAR2_EL1_PRFMSLC_IMP                    UL(0b0001)

#define ID_AA64ISAR2_EL1_SYSINSTR_128                   GENMASK(39, 36)
#define ID_AA64ISAR2_EL1_SYSINSTR_128_MASK              GENMASK(39, 36)
#define ID_AA64ISAR2_EL1_SYSINSTR_128_SHIFT             36
#define ID_AA64ISAR2_EL1_SYSINSTR_128_WIDTH             4
#define ID_AA64ISAR2_EL1_SYSINSTR_128_SIGNED            false
#define ID_AA64ISAR2_EL1_SYSINSTR_128_NI                UL(0b0000)
#define ID_AA64ISAR2_EL1_SYSINSTR_128_IMP               UL(0b0001)

#define ID_AA64ISAR2_EL1_SYSREG_128                     GENMASK(35, 32)
#define ID_AA64ISAR2_EL1_SYSREG_128_MASK                GENMASK(35, 32)
#define ID_AA64ISAR2_EL1_SYSREG_128_SHIFT               32
#define ID_AA64ISAR2_EL1_SYSREG_128_WIDTH               4
#define ID_AA64ISAR2_EL1_SYSREG_128_SIGNED              false
#define ID_AA64ISAR2_EL1_SYSREG_128_NI                  UL(0b0000)
#define ID_AA64ISAR2_EL1_SYSREG_128_IMP                 UL(0b0001)

#define ID_AA64ISAR2_EL1_CLRBHB                         GENMASK(31, 28)
#define ID_AA64ISAR2_EL1_CLRBHB_MASK                    GENMASK(31, 28)
#define ID_AA64ISAR2_EL1_CLRBHB_SHIFT                   28
#define ID_AA64ISAR2_EL1_CLRBHB_WIDTH                   4
#define ID_AA64ISAR2_EL1_CLRBHB_SIGNED                  false
#define ID_AA64ISAR2_EL1_CLRBHB_NI                      UL(0b0000)
#define ID_AA64ISAR2_EL1_CLRBHB_IMP                     UL(0b0001)

#define ID_AA64ISAR2_EL1_PAC_frac                       GENMASK(27, 24)
#define ID_AA64ISAR2_EL1_PAC_frac_MASK                  GENMASK(27, 24)
#define ID_AA64ISAR2_EL1_PAC_frac_SHIFT                 24
#define ID_AA64ISAR2_EL1_PAC_frac_WIDTH                 4
#define ID_AA64ISAR2_EL1_PAC_frac_SIGNED                false
#define ID_AA64ISAR2_EL1_PAC_frac_NI                    UL(0b0000)
#define ID_AA64ISAR2_EL1_PAC_frac_IMP                   UL(0b0001)

#define ID_AA64ISAR2_EL1_BC                             GENMASK(23, 20)
#define ID_AA64ISAR2_EL1_BC_MASK                        GENMASK(23, 20)
#define ID_AA64ISAR2_EL1_BC_SHIFT                       20
#define ID_AA64ISAR2_EL1_BC_WIDTH                       4
#define ID_AA64ISAR2_EL1_BC_SIGNED                      false
#define ID_AA64ISAR2_EL1_BC_NI                          UL(0b0000)
#define ID_AA64ISAR2_EL1_BC_IMP                         UL(0b0001)

#define ID_AA64ISAR2_EL1_MOPS                           GENMASK(19, 16)
#define ID_AA64ISAR2_EL1_MOPS_MASK                      GENMASK(19, 16)
#define ID_AA64ISAR2_EL1_MOPS_SHIFT                     16
#define ID_AA64ISAR2_EL1_MOPS_WIDTH                     4
#define ID_AA64ISAR2_EL1_MOPS_SIGNED                    false
#define ID_AA64ISAR2_EL1_MOPS_NI                        UL(0b0000)
#define ID_AA64ISAR2_EL1_MOPS_IMP                       UL(0b0001)

#define ID_AA64ISAR2_EL1_APA3                           GENMASK(15, 12)
#define ID_AA64ISAR2_EL1_APA3_MASK                      GENMASK(15, 12)
#define ID_AA64ISAR2_EL1_APA3_SHIFT                     12
#define ID_AA64ISAR2_EL1_APA3_WIDTH                     4
#define ID_AA64ISAR2_EL1_APA3_SIGNED                    false
#define ID_AA64ISAR2_EL1_APA3_NI                        UL(0b0000)
#define ID_AA64ISAR2_EL1_APA3_PAuth                     UL(0b0001)
#define ID_AA64ISAR2_EL1_APA3_EPAC                      UL(0b0010)
#define ID_AA64ISAR2_EL1_APA3_PAuth2                    UL(0b0011)
#define ID_AA64ISAR2_EL1_APA3_FPAC                      UL(0b0100)
#define ID_AA64ISAR2_EL1_APA3_FPACCOMBINE               UL(0b0101)
#define ID_AA64ISAR2_EL1_APA3_PAuth_LR                  UL(0b0110)

#define ID_AA64ISAR2_EL1_GPA3                           GENMASK(11, 8)
#define ID_AA64ISAR2_EL1_GPA3_MASK                      GENMASK(11, 8)
#define ID_AA64ISAR2_EL1_GPA3_SHIFT                     8
#define ID_AA64ISAR2_EL1_GPA3_WIDTH                     4
#define ID_AA64ISAR2_EL1_GPA3_SIGNED                    false
#define ID_AA64ISAR2_EL1_GPA3_NI                        UL(0b0000)
#define ID_AA64ISAR2_EL1_GPA3_IMP                       UL(0b0001)

#define ID_AA64ISAR2_EL1_RPRES                          GENMASK(7, 4)
#define ID_AA64ISAR2_EL1_RPRES_MASK                     GENMASK(7, 4)
#define ID_AA64ISAR2_EL1_RPRES_SHIFT                    4
#define ID_AA64ISAR2_EL1_RPRES_WIDTH                    4
#define ID_AA64ISAR2_EL1_RPRES_SIGNED                   false
#define ID_AA64ISAR2_EL1_RPRES_NI                       UL(0b0000)
#define ID_AA64ISAR2_EL1_RPRES_IMP                      UL(0b0001)

#define ID_AA64ISAR2_EL1_WFxT                           GENMASK(3, 0)
#define ID_AA64ISAR2_EL1_WFxT_MASK                      GENMASK(3, 0)
#define ID_AA64ISAR2_EL1_WFxT_SHIFT                     0
#define ID_AA64ISAR2_EL1_WFxT_WIDTH                     4
#define ID_AA64ISAR2_EL1_WFxT_SIGNED                    false
#define ID_AA64ISAR2_EL1_WFxT_NI                        UL(0b0000)
#define ID_AA64ISAR2_EL1_WFxT_IMP                       UL(0b0010)

#define ID_AA64ISAR2_EL1_RES0                           (UL(0))
#define ID_AA64ISAR2_EL1_RES1                           (UL(0))
#define ID_AA64ISAR2_EL1_UNKN                           (UL(0))

#define REG_ID_AA64ISAR3_EL1                            S3_0_C0_C6_3
#define SYS_ID_AA64ISAR3_EL1                            sys_reg(3, 0, 0, 6, 3)
#define SYS_ID_AA64ISAR3_EL1_Op0                        3
#define SYS_ID_AA64ISAR3_EL1_Op1                        0
#define SYS_ID_AA64ISAR3_EL1_CRn                        0
#define SYS_ID_AA64ISAR3_EL1_CRm                        6
#define SYS_ID_AA64ISAR3_EL1_Op2                        3

#define ID_AA64ISAR3_EL1_FPRCVT                         GENMASK(31, 28)
#define ID_AA64ISAR3_EL1_FPRCVT_MASK                    GENMASK(31, 28)
#define ID_AA64ISAR3_EL1_FPRCVT_SHIFT                   28
#define ID_AA64ISAR3_EL1_FPRCVT_WIDTH                   4
#define ID_AA64ISAR3_EL1_FPRCVT_SIGNED                  false
#define ID_AA64ISAR3_EL1_FPRCVT_NI                      UL(0b0000)
#define ID_AA64ISAR3_EL1_FPRCVT_IMP                     UL(0b0001)

#define ID_AA64ISAR3_EL1_LSUI                           GENMASK(27, 24)
#define ID_AA64ISAR3_EL1_LSUI_MASK                      GENMASK(27, 24)
#define ID_AA64ISAR3_EL1_LSUI_SHIFT                     24
#define ID_AA64ISAR3_EL1_LSUI_WIDTH                     4
#define ID_AA64ISAR3_EL1_LSUI_SIGNED                    false
#define ID_AA64ISAR3_EL1_LSUI_NI                        UL(0b0000)
#define ID_AA64ISAR3_EL1_LSUI_IMP                       UL(0b0001)

#define ID_AA64ISAR3_EL1_OCCMO                          GENMASK(23, 20)
#define ID_AA64ISAR3_EL1_OCCMO_MASK                     GENMASK(23, 20)
#define ID_AA64ISAR3_EL1_OCCMO_SHIFT                    20
#define ID_AA64ISAR3_EL1_OCCMO_WIDTH                    4
#define ID_AA64ISAR3_EL1_OCCMO_SIGNED                   false
#define ID_AA64ISAR3_EL1_OCCMO_NI                       UL(0b0000)
#define ID_AA64ISAR3_EL1_OCCMO_IMP                      UL(0b0001)

#define ID_AA64ISAR3_EL1_LSFE                           GENMASK(19, 16)
#define ID_AA64ISAR3_EL1_LSFE_MASK                      GENMASK(19, 16)
#define ID_AA64ISAR3_EL1_LSFE_SHIFT                     16
#define ID_AA64ISAR3_EL1_LSFE_WIDTH                     4
#define ID_AA64ISAR3_EL1_LSFE_SIGNED                    false
#define ID_AA64ISAR3_EL1_LSFE_NI                        UL(0b0000)
#define ID_AA64ISAR3_EL1_LSFE_IMP                       UL(0b0001)

#define ID_AA64ISAR3_EL1_PACM                           GENMASK(15, 12)
#define ID_AA64ISAR3_EL1_PACM_MASK                      GENMASK(15, 12)
#define ID_AA64ISAR3_EL1_PACM_SHIFT                     12
#define ID_AA64ISAR3_EL1_PACM_WIDTH                     4
#define ID_AA64ISAR3_EL1_PACM_SIGNED                    false
#define ID_AA64ISAR3_EL1_PACM_NI                        UL(0b0000)
#define ID_AA64ISAR3_EL1_PACM_TRIVIAL_IMP               UL(0b0001)
#define ID_AA64ISAR3_EL1_PACM_FULL_IMP                  UL(0b0010)

#define ID_AA64ISAR3_EL1_TLBIW                          GENMASK(11, 8)
#define ID_AA64ISAR3_EL1_TLBIW_MASK                     GENMASK(11, 8)
#define ID_AA64ISAR3_EL1_TLBIW_SHIFT                    8
#define ID_AA64ISAR3_EL1_TLBIW_WIDTH                    4
#define ID_AA64ISAR3_EL1_TLBIW_SIGNED                   false
#define ID_AA64ISAR3_EL1_TLBIW_NI                       UL(0b0000)
#define ID_AA64ISAR3_EL1_TLBIW_IMP                      UL(0b0001)

#define ID_AA64ISAR3_EL1_FAMINMAX                       GENMASK(7, 4)
#define ID_AA64ISAR3_EL1_FAMINMAX_MASK                  GENMASK(7, 4)
#define ID_AA64ISAR3_EL1_FAMINMAX_SHIFT                 4
#define ID_AA64ISAR3_EL1_FAMINMAX_WIDTH                 4
#define ID_AA64ISAR3_EL1_FAMINMAX_SIGNED                false
#define ID_AA64ISAR3_EL1_FAMINMAX_NI                    UL(0b0000)
#define ID_AA64ISAR3_EL1_FAMINMAX_IMP                   UL(0b0001)

#define ID_AA64ISAR3_EL1_CPA                            GENMASK(3, 0)
#define ID_AA64ISAR3_EL1_CPA_MASK                       GENMASK(3, 0)
#define ID_AA64ISAR3_EL1_CPA_SHIFT                      0
#define ID_AA64ISAR3_EL1_CPA_WIDTH                      4
#define ID_AA64ISAR3_EL1_CPA_SIGNED                     false
#define ID_AA64ISAR3_EL1_CPA_NI                         UL(0b0000)
#define ID_AA64ISAR3_EL1_CPA_IMP                        UL(0b0001)
#define ID_AA64ISAR3_EL1_CPA_CPA2                       UL(0b0010)

#define ID_AA64ISAR3_EL1_RES0                           (UL(0) | GENMASK_ULL(63, 32))
#define ID_AA64ISAR3_EL1_RES1                           (UL(0))
#define ID_AA64ISAR3_EL1_UNKN                           (UL(0))

#define REG_ID_AA64MMFR0_EL1                            S3_0_C0_C7_0
#define SYS_ID_AA64MMFR0_EL1                            sys_reg(3, 0, 0, 7, 0)
#define SYS_ID_AA64MMFR0_EL1_Op0                        3
#define SYS_ID_AA64MMFR0_EL1_Op1                        0
#define SYS_ID_AA64MMFR0_EL1_CRn                        0
#define SYS_ID_AA64MMFR0_EL1_CRm                        7
#define SYS_ID_AA64MMFR0_EL1_Op2                        0

#define ID_AA64MMFR0_EL1_ECV                            GENMASK(63, 60)
#define ID_AA64MMFR0_EL1_ECV_MASK                       GENMASK(63, 60)
#define ID_AA64MMFR0_EL1_ECV_SHIFT                      60
#define ID_AA64MMFR0_EL1_ECV_WIDTH                      4
#define ID_AA64MMFR0_EL1_ECV_SIGNED                     false
#define ID_AA64MMFR0_EL1_ECV_NI                         UL(0b0000)
#define ID_AA64MMFR0_EL1_ECV_IMP                        UL(0b0001)
#define ID_AA64MMFR0_EL1_ECV_CNTPOFF                    UL(0b0010)

#define ID_AA64MMFR0_EL1_FGT                            GENMASK(59, 56)
#define ID_AA64MMFR0_EL1_FGT_MASK                       GENMASK(59, 56)
#define ID_AA64MMFR0_EL1_FGT_SHIFT                      56
#define ID_AA64MMFR0_EL1_FGT_WIDTH                      4
#define ID_AA64MMFR0_EL1_FGT_SIGNED                     false
#define ID_AA64MMFR0_EL1_FGT_NI                         UL(0b0000)
#define ID_AA64MMFR0_EL1_FGT_IMP                        UL(0b0001)
#define ID_AA64MMFR0_EL1_FGT_FGT2                       UL(0b0010)

#define ID_AA64MMFR0_EL1_EXS                            GENMASK(47, 44)
#define ID_AA64MMFR0_EL1_EXS_MASK                       GENMASK(47, 44)
#define ID_AA64MMFR0_EL1_EXS_SHIFT                      44
#define ID_AA64MMFR0_EL1_EXS_WIDTH                      4
#define ID_AA64MMFR0_EL1_EXS_SIGNED                     false
#define ID_AA64MMFR0_EL1_EXS_NI                         UL(0b0000)
#define ID_AA64MMFR0_EL1_EXS_IMP                        UL(0b0001)

#define ID_AA64MMFR0_EL1_TGRAN4_2                       GENMASK(43, 40)
#define ID_AA64MMFR0_EL1_TGRAN4_2_MASK                  GENMASK(43, 40)
#define ID_AA64MMFR0_EL1_TGRAN4_2_SHIFT                 40
#define ID_AA64MMFR0_EL1_TGRAN4_2_WIDTH                 4
#define ID_AA64MMFR0_EL1_TGRAN4_2_TGRAN4                UL(0b0000)
#define ID_AA64MMFR0_EL1_TGRAN4_2_NI                    UL(0b0001)
#define ID_AA64MMFR0_EL1_TGRAN4_2_IMP                   UL(0b0010)
#define ID_AA64MMFR0_EL1_TGRAN4_2_52_BIT                UL(0b0011)

#define ID_AA64MMFR0_EL1_TGRAN64_2                      GENMASK(39, 36)
#define ID_AA64MMFR0_EL1_TGRAN64_2_MASK                 GENMASK(39, 36)
#define ID_AA64MMFR0_EL1_TGRAN64_2_SHIFT                36
#define ID_AA64MMFR0_EL1_TGRAN64_2_WIDTH                4
#define ID_AA64MMFR0_EL1_TGRAN64_2_TGRAN64              UL(0b0000)
#define ID_AA64MMFR0_EL1_TGRAN64_2_NI                   UL(0b0001)
#define ID_AA64MMFR0_EL1_TGRAN64_2_IMP                  UL(0b0010)

#define ID_AA64MMFR0_EL1_TGRAN16_2                      GENMASK(35, 32)
#define ID_AA64MMFR0_EL1_TGRAN16_2_MASK                 GENMASK(35, 32)
#define ID_AA64MMFR0_EL1_TGRAN16_2_SHIFT                32
#define ID_AA64MMFR0_EL1_TGRAN16_2_WIDTH                4
#define ID_AA64MMFR0_EL1_TGRAN16_2_TGRAN16              UL(0b0000)
#define ID_AA64MMFR0_EL1_TGRAN16_2_NI                   UL(0b0001)
#define ID_AA64MMFR0_EL1_TGRAN16_2_IMP                  UL(0b0010)
#define ID_AA64MMFR0_EL1_TGRAN16_2_52_BIT               UL(0b0011)

#define ID_AA64MMFR0_EL1_TGRAN4                         GENMASK(31, 28)
#define ID_AA64MMFR0_EL1_TGRAN4_MASK                    GENMASK(31, 28)
#define ID_AA64MMFR0_EL1_TGRAN4_SHIFT                   28
#define ID_AA64MMFR0_EL1_TGRAN4_WIDTH                   4
#define ID_AA64MMFR0_EL1_TGRAN4_SIGNED                  true
#define ID_AA64MMFR0_EL1_TGRAN4_IMP                     UL(0b0000)
#define ID_AA64MMFR0_EL1_TGRAN4_52_BIT                  UL(0b0001)
#define ID_AA64MMFR0_EL1_TGRAN4_NI                      UL(0b1111)

#define ID_AA64MMFR0_EL1_TGRAN64                        GENMASK(27, 24)
#define ID_AA64MMFR0_EL1_TGRAN64_MASK                   GENMASK(27, 24)
#define ID_AA64MMFR0_EL1_TGRAN64_SHIFT                  24
#define ID_AA64MMFR0_EL1_TGRAN64_WIDTH                  4
#define ID_AA64MMFR0_EL1_TGRAN64_SIGNED                 true
#define ID_AA64MMFR0_EL1_TGRAN64_IMP                    UL(0b0000)
#define ID_AA64MMFR0_EL1_TGRAN64_NI                     UL(0b1111)

#define ID_AA64MMFR0_EL1_TGRAN16                        GENMASK(23, 20)
#define ID_AA64MMFR0_EL1_TGRAN16_MASK                   GENMASK(23, 20)
#define ID_AA64MMFR0_EL1_TGRAN16_SHIFT                  20
#define ID_AA64MMFR0_EL1_TGRAN16_WIDTH                  4
#define ID_AA64MMFR0_EL1_TGRAN16_SIGNED                 false
#define ID_AA64MMFR0_EL1_TGRAN16_NI                     UL(0b0000)
#define ID_AA64MMFR0_EL1_TGRAN16_IMP                    UL(0b0001)
#define ID_AA64MMFR0_EL1_TGRAN16_52_BIT                 UL(0b0010)

#define ID_AA64MMFR0_EL1_BIGENDEL0                      GENMASK(19, 16)
#define ID_AA64MMFR0_EL1_BIGENDEL0_MASK                 GENMASK(19, 16)
#define ID_AA64MMFR0_EL1_BIGENDEL0_SHIFT                16
#define ID_AA64MMFR0_EL1_BIGENDEL0_WIDTH                4
#define ID_AA64MMFR0_EL1_BIGENDEL0_SIGNED               false
#define ID_AA64MMFR0_EL1_BIGENDEL0_NI                   UL(0b0000)
#define ID_AA64MMFR0_EL1_BIGENDEL0_IMP                  UL(0b0001)

#define ID_AA64MMFR0_EL1_SNSMEM                         GENMASK(15, 12)
#define ID_AA64MMFR0_EL1_SNSMEM_MASK                    GENMASK(15, 12)
#define ID_AA64MMFR0_EL1_SNSMEM_SHIFT                   12
#define ID_AA64MMFR0_EL1_SNSMEM_WIDTH                   4
#define ID_AA64MMFR0_EL1_SNSMEM_SIGNED                  false
#define ID_AA64MMFR0_EL1_SNSMEM_NI                      UL(0b0000)
#define ID_AA64MMFR0_EL1_SNSMEM_IMP                     UL(0b0001)

#define ID_AA64MMFR0_EL1_BIGEND                         GENMASK(11, 8)
#define ID_AA64MMFR0_EL1_BIGEND_MASK                    GENMASK(11, 8)
#define ID_AA64MMFR0_EL1_BIGEND_SHIFT                   8
#define ID_AA64MMFR0_EL1_BIGEND_WIDTH                   4
#define ID_AA64MMFR0_EL1_BIGEND_SIGNED                  false
#define ID_AA64MMFR0_EL1_BIGEND_NI                      UL(0b0000)
#define ID_AA64MMFR0_EL1_BIGEND_IMP                     UL(0b0001)

#define ID_AA64MMFR0_EL1_ASIDBITS                       GENMASK(7, 4)
#define ID_AA64MMFR0_EL1_ASIDBITS_MASK                  GENMASK(7, 4)
#define ID_AA64MMFR0_EL1_ASIDBITS_SHIFT                 4
#define ID_AA64MMFR0_EL1_ASIDBITS_WIDTH                 4
#define ID_AA64MMFR0_EL1_ASIDBITS_8                     UL(0b0000)
#define ID_AA64MMFR0_EL1_ASIDBITS_16                    UL(0b0010)

#define ID_AA64MMFR0_EL1_PARANGE                        GENMASK(3, 0)
#define ID_AA64MMFR0_EL1_PARANGE_MASK                   GENMASK(3, 0)
#define ID_AA64MMFR0_EL1_PARANGE_SHIFT                  0
#define ID_AA64MMFR0_EL1_PARANGE_WIDTH                  4
#define ID_AA64MMFR0_EL1_PARANGE_32                     UL(0b0000)
#define ID_AA64MMFR0_EL1_PARANGE_36                     UL(0b0001)
#define ID_AA64MMFR0_EL1_PARANGE_40                     UL(0b0010)
#define ID_AA64MMFR0_EL1_PARANGE_42                     UL(0b0011)
#define ID_AA64MMFR0_EL1_PARANGE_44                     UL(0b0100)
#define ID_AA64MMFR0_EL1_PARANGE_48                     UL(0b0101)
#define ID_AA64MMFR0_EL1_PARANGE_52                     UL(0b0110)
#define ID_AA64MMFR0_EL1_PARANGE_56                     UL(0b0111)

#define ID_AA64MMFR0_EL1_RES0                           (UL(0) | GENMASK_ULL(55, 48))
#define ID_AA64MMFR0_EL1_RES1                           (UL(0))
#define ID_AA64MMFR0_EL1_UNKN                           (UL(0))

#define REG_ID_AA64MMFR1_EL1                            S3_0_C0_C7_1
#define SYS_ID_AA64MMFR1_EL1                            sys_reg(3, 0, 0, 7, 1)
#define SYS_ID_AA64MMFR1_EL1_Op0                        3
#define SYS_ID_AA64MMFR1_EL1_Op1                        0
#define SYS_ID_AA64MMFR1_EL1_CRn                        0
#define SYS_ID_AA64MMFR1_EL1_CRm                        7
#define SYS_ID_AA64MMFR1_EL1_Op2                        1

#define ID_AA64MMFR1_EL1_ECBHB                          GENMASK(63, 60)
#define ID_AA64MMFR1_EL1_ECBHB_MASK                     GENMASK(63, 60)
#define ID_AA64MMFR1_EL1_ECBHB_SHIFT                    60
#define ID_AA64MMFR1_EL1_ECBHB_WIDTH                    4
#define ID_AA64MMFR1_EL1_ECBHB_SIGNED                   false
#define ID_AA64MMFR1_EL1_ECBHB_NI                       UL(0b0000)
#define ID_AA64MMFR1_EL1_ECBHB_IMP                      UL(0b0001)

#define ID_AA64MMFR1_EL1_CMOW                           GENMASK(59, 56)
#define ID_AA64MMFR1_EL1_CMOW_MASK                      GENMASK(59, 56)
#define ID_AA64MMFR1_EL1_CMOW_SHIFT                     56
#define ID_AA64MMFR1_EL1_CMOW_WIDTH                     4
#define ID_AA64MMFR1_EL1_CMOW_SIGNED                    false
#define ID_AA64MMFR1_EL1_CMOW_NI                        UL(0b0000)
#define ID_AA64MMFR1_EL1_CMOW_IMP                       UL(0b0001)

#define ID_AA64MMFR1_EL1_TIDCP1                         GENMASK(55, 52)
#define ID_AA64MMFR1_EL1_TIDCP1_MASK                    GENMASK(55, 52)
#define ID_AA64MMFR1_EL1_TIDCP1_SHIFT                   52
#define ID_AA64MMFR1_EL1_TIDCP1_WIDTH                   4
#define ID_AA64MMFR1_EL1_TIDCP1_SIGNED                  false
#define ID_AA64MMFR1_EL1_TIDCP1_NI                      UL(0b0000)
#define ID_AA64MMFR1_EL1_TIDCP1_IMP                     UL(0b0001)

#define ID_AA64MMFR1_EL1_nTLBPA                         GENMASK(51, 48)
#define ID_AA64MMFR1_EL1_nTLBPA_MASK                    GENMASK(51, 48)
#define ID_AA64MMFR1_EL1_nTLBPA_SHIFT                   48
#define ID_AA64MMFR1_EL1_nTLBPA_WIDTH                   4
#define ID_AA64MMFR1_EL1_nTLBPA_SIGNED                  false
#define ID_AA64MMFR1_EL1_nTLBPA_NI                      UL(0b0000)
#define ID_AA64MMFR1_EL1_nTLBPA_IMP                     UL(0b0001)

#define ID_AA64MMFR1_EL1_AFP                            GENMASK(47, 44)
#define ID_AA64MMFR1_EL1_AFP_MASK                       GENMASK(47, 44)
#define ID_AA64MMFR1_EL1_AFP_SHIFT                      44
#define ID_AA64MMFR1_EL1_AFP_WIDTH                      4
#define ID_AA64MMFR1_EL1_AFP_SIGNED                     false
#define ID_AA64MMFR1_EL1_AFP_NI                         UL(0b0000)
#define ID_AA64MMFR1_EL1_AFP_IMP                        UL(0b0001)

#define ID_AA64MMFR1_EL1_HCX                            GENMASK(43, 40)
#define ID_AA64MMFR1_EL1_HCX_MASK                       GENMASK(43, 40)
#define ID_AA64MMFR1_EL1_HCX_SHIFT                      40
#define ID_AA64MMFR1_EL1_HCX_WIDTH                      4
#define ID_AA64MMFR1_EL1_HCX_SIGNED                     false
#define ID_AA64MMFR1_EL1_HCX_NI                         UL(0b0000)
#define ID_AA64MMFR1_EL1_HCX_IMP                        UL(0b0001)

#define ID_AA64MMFR1_EL1_ETS                            GENMASK(39, 36)
#define ID_AA64MMFR1_EL1_ETS_MASK                       GENMASK(39, 36)
#define ID_AA64MMFR1_EL1_ETS_SHIFT                      36
#define ID_AA64MMFR1_EL1_ETS_WIDTH                      4
#define ID_AA64MMFR1_EL1_ETS_SIGNED                     false
#define ID_AA64MMFR1_EL1_ETS_NI                         UL(0b0000)
#define ID_AA64MMFR1_EL1_ETS_IMP                        UL(0b0001)
#define ID_AA64MMFR1_EL1_ETS_ETS2                       UL(0b0010)
#define ID_AA64MMFR1_EL1_ETS_ETS3                       UL(0b0011)

#define ID_AA64MMFR1_EL1_TWED                           GENMASK(35, 32)
#define ID_AA64MMFR1_EL1_TWED_MASK                      GENMASK(35, 32)
#define ID_AA64MMFR1_EL1_TWED_SHIFT                     32
#define ID_AA64MMFR1_EL1_TWED_WIDTH                     4
#define ID_AA64MMFR1_EL1_TWED_SIGNED                    false
#define ID_AA64MMFR1_EL1_TWED_NI                        UL(0b0000)
#define ID_AA64MMFR1_EL1_TWED_IMP                       UL(0b0001)

#define ID_AA64MMFR1_EL1_XNX                            GENMASK(31, 28)
#define ID_AA64MMFR1_EL1_XNX_MASK                       GENMASK(31, 28)
#define ID_AA64MMFR1_EL1_XNX_SHIFT                      28
#define ID_AA64MMFR1_EL1_XNX_WIDTH                      4
#define ID_AA64MMFR1_EL1_XNX_SIGNED                     false
#define ID_AA64MMFR1_EL1_XNX_NI                         UL(0b0000)
#define ID_AA64MMFR1_EL1_XNX_IMP                        UL(0b0001)

#define ID_AA64MMFR1_EL1_SpecSEI                        GENMASK(27, 24)
#define ID_AA64MMFR1_EL1_SpecSEI_MASK                   GENMASK(27, 24)
#define ID_AA64MMFR1_EL1_SpecSEI_SHIFT                  24
#define ID_AA64MMFR1_EL1_SpecSEI_WIDTH                  4
#define ID_AA64MMFR1_EL1_SpecSEI_SIGNED                 false
#define ID_AA64MMFR1_EL1_SpecSEI_NI                     UL(0b0000)
#define ID_AA64MMFR1_EL1_SpecSEI_IMP                    UL(0b0001)

#define ID_AA64MMFR1_EL1_PAN                            GENMASK(23, 20)
#define ID_AA64MMFR1_EL1_PAN_MASK                       GENMASK(23, 20)
#define ID_AA64MMFR1_EL1_PAN_SHIFT                      20
#define ID_AA64MMFR1_EL1_PAN_WIDTH                      4
#define ID_AA64MMFR1_EL1_PAN_SIGNED                     false
#define ID_AA64MMFR1_EL1_PAN_NI                         UL(0b0000)
#define ID_AA64MMFR1_EL1_PAN_IMP                        UL(0b0001)
#define ID_AA64MMFR1_EL1_PAN_PAN2                       UL(0b0010)
#define ID_AA64MMFR1_EL1_PAN_PAN3                       UL(0b0011)

#define ID_AA64MMFR1_EL1_LO                             GENMASK(19, 16)
#define ID_AA64MMFR1_EL1_LO_MASK                        GENMASK(19, 16)
#define ID_AA64MMFR1_EL1_LO_SHIFT                       16
#define ID_AA64MMFR1_EL1_LO_WIDTH                       4
#define ID_AA64MMFR1_EL1_LO_SIGNED                      false
#define ID_AA64MMFR1_EL1_LO_NI                          UL(0b0000)
#define ID_AA64MMFR1_EL1_LO_IMP                         UL(0b0001)

#define ID_AA64MMFR1_EL1_HPDS                           GENMASK(15, 12)
#define ID_AA64MMFR1_EL1_HPDS_MASK                      GENMASK(15, 12)
#define ID_AA64MMFR1_EL1_HPDS_SHIFT                     12
#define ID_AA64MMFR1_EL1_HPDS_WIDTH                     4
#define ID_AA64MMFR1_EL1_HPDS_SIGNED                    false
#define ID_AA64MMFR1_EL1_HPDS_NI                        UL(0b0000)
#define ID_AA64MMFR1_EL1_HPDS_IMP                       UL(0b0001)
#define ID_AA64MMFR1_EL1_HPDS_HPDS2                     UL(0b0010)

#define ID_AA64MMFR1_EL1_VH                             GENMASK(11, 8)
#define ID_AA64MMFR1_EL1_VH_MASK                        GENMASK(11, 8)
#define ID_AA64MMFR1_EL1_VH_SHIFT                       8
#define ID_AA64MMFR1_EL1_VH_WIDTH                       4
#define ID_AA64MMFR1_EL1_VH_SIGNED                      false
#define ID_AA64MMFR1_EL1_VH_NI                          UL(0b0000)
#define ID_AA64MMFR1_EL1_VH_IMP                         UL(0b0001)

#define ID_AA64MMFR1_EL1_VMIDBits                       GENMASK(7, 4)
#define ID_AA64MMFR1_EL1_VMIDBits_MASK                  GENMASK(7, 4)
#define ID_AA64MMFR1_EL1_VMIDBits_SHIFT                 4
#define ID_AA64MMFR1_EL1_VMIDBits_WIDTH                 4
#define ID_AA64MMFR1_EL1_VMIDBits_8                     UL(0b0000)
#define ID_AA64MMFR1_EL1_VMIDBits_16                    UL(0b0010)

#define ID_AA64MMFR1_EL1_HAFDBS                         GENMASK(3, 0)
#define ID_AA64MMFR1_EL1_HAFDBS_MASK                    GENMASK(3, 0)
#define ID_AA64MMFR1_EL1_HAFDBS_SHIFT                   0
#define ID_AA64MMFR1_EL1_HAFDBS_WIDTH                   4
#define ID_AA64MMFR1_EL1_HAFDBS_SIGNED                  false
#define ID_AA64MMFR1_EL1_HAFDBS_NI                      UL(0b0000)
#define ID_AA64MMFR1_EL1_HAFDBS_AF                      UL(0b0001)
#define ID_AA64MMFR1_EL1_HAFDBS_DBM                     UL(0b0010)
#define ID_AA64MMFR1_EL1_HAFDBS_HAFT                    UL(0b0011)
#define ID_AA64MMFR1_EL1_HAFDBS_HDBSS                   UL(0b0100)

#define ID_AA64MMFR1_EL1_RES0                           (UL(0))
#define ID_AA64MMFR1_EL1_RES1                           (UL(0))
#define ID_AA64MMFR1_EL1_UNKN                           (UL(0))

#define REG_ID_AA64MMFR2_EL1                            S3_0_C0_C7_2
#define SYS_ID_AA64MMFR2_EL1                            sys_reg(3, 0, 0, 7, 2)
#define SYS_ID_AA64MMFR2_EL1_Op0                        3
#define SYS_ID_AA64MMFR2_EL1_Op1                        0
#define SYS_ID_AA64MMFR2_EL1_CRn                        0
#define SYS_ID_AA64MMFR2_EL1_CRm                        7
#define SYS_ID_AA64MMFR2_EL1_Op2                        2

#define ID_AA64MMFR2_EL1_E0PD                           GENMASK(63, 60)
#define ID_AA64MMFR2_EL1_E0PD_MASK                      GENMASK(63, 60)
#define ID_AA64MMFR2_EL1_E0PD_SHIFT                     60
#define ID_AA64MMFR2_EL1_E0PD_WIDTH                     4
#define ID_AA64MMFR2_EL1_E0PD_SIGNED                    false
#define ID_AA64MMFR2_EL1_E0PD_NI                        UL(0b0000)
#define ID_AA64MMFR2_EL1_E0PD_IMP                       UL(0b0001)

#define ID_AA64MMFR2_EL1_EVT                            GENMASK(59, 56)
#define ID_AA64MMFR2_EL1_EVT_MASK                       GENMASK(59, 56)
#define ID_AA64MMFR2_EL1_EVT_SHIFT                      56
#define ID_AA64MMFR2_EL1_EVT_WIDTH                      4
#define ID_AA64MMFR2_EL1_EVT_SIGNED                     false
#define ID_AA64MMFR2_EL1_EVT_NI                         UL(0b0000)
#define ID_AA64MMFR2_EL1_EVT_IMP                        UL(0b0001)
#define ID_AA64MMFR2_EL1_EVT_TTLBxS                     UL(0b0010)

#define ID_AA64MMFR2_EL1_BBM                            GENMASK(55, 52)
#define ID_AA64MMFR2_EL1_BBM_MASK                       GENMASK(55, 52)
#define ID_AA64MMFR2_EL1_BBM_SHIFT                      52
#define ID_AA64MMFR2_EL1_BBM_WIDTH                      4
#define ID_AA64MMFR2_EL1_BBM_SIGNED                     false
#define ID_AA64MMFR2_EL1_BBM_0                          UL(0b0000)
#define ID_AA64MMFR2_EL1_BBM_1                          UL(0b0001)
#define ID_AA64MMFR2_EL1_BBM_2                          UL(0b0010)

#define ID_AA64MMFR2_EL1_TTL                            GENMASK(51, 48)
#define ID_AA64MMFR2_EL1_TTL_MASK                       GENMASK(51, 48)
#define ID_AA64MMFR2_EL1_TTL_SHIFT                      48
#define ID_AA64MMFR2_EL1_TTL_WIDTH                      4
#define ID_AA64MMFR2_EL1_TTL_SIGNED                     false
#define ID_AA64MMFR2_EL1_TTL_NI                         UL(0b0000)
#define ID_AA64MMFR2_EL1_TTL_IMP                        UL(0b0001)

#define ID_AA64MMFR2_EL1_FWB                            GENMASK(43, 40)
#define ID_AA64MMFR2_EL1_FWB_MASK                       GENMASK(43, 40)
#define ID_AA64MMFR2_EL1_FWB_SHIFT                      40
#define ID_AA64MMFR2_EL1_FWB_WIDTH                      4
#define ID_AA64MMFR2_EL1_FWB_SIGNED                     false
#define ID_AA64MMFR2_EL1_FWB_NI                         UL(0b0000)
#define ID_AA64MMFR2_EL1_FWB_IMP                        UL(0b0001)

#define ID_AA64MMFR2_EL1_IDS                            GENMASK(39, 36)
#define ID_AA64MMFR2_EL1_IDS_MASK                       GENMASK(39, 36)
#define ID_AA64MMFR2_EL1_IDS_SHIFT                      36
#define ID_AA64MMFR2_EL1_IDS_WIDTH                      4
#define ID_AA64MMFR2_EL1_IDS_0x0                        UL(0b0000)
#define ID_AA64MMFR2_EL1_IDS_0x18                       UL(0b0001)

#define ID_AA64MMFR2_EL1_AT                             GENMASK(35, 32)
#define ID_AA64MMFR2_EL1_AT_MASK                        GENMASK(35, 32)
#define ID_AA64MMFR2_EL1_AT_SHIFT                       32
#define ID_AA64MMFR2_EL1_AT_WIDTH                       4
#define ID_AA64MMFR2_EL1_AT_SIGNED                      false
#define ID_AA64MMFR2_EL1_AT_NI                          UL(0b0000)
#define ID_AA64MMFR2_EL1_AT_IMP                         UL(0b0001)

#define ID_AA64MMFR2_EL1_ST                             GENMASK(31, 28)
#define ID_AA64MMFR2_EL1_ST_MASK                        GENMASK(31, 28)
#define ID_AA64MMFR2_EL1_ST_SHIFT                       28
#define ID_AA64MMFR2_EL1_ST_WIDTH                       4
#define ID_AA64MMFR2_EL1_ST_39                          UL(0b0000)
#define ID_AA64MMFR2_EL1_ST_48_47                       UL(0b0001)

#define ID_AA64MMFR2_EL1_NV                             GENMASK(27, 24)
#define ID_AA64MMFR2_EL1_NV_MASK                        GENMASK(27, 24)
#define ID_AA64MMFR2_EL1_NV_SHIFT                       24
#define ID_AA64MMFR2_EL1_NV_WIDTH                       4
#define ID_AA64MMFR2_EL1_NV_SIGNED                      false
#define ID_AA64MMFR2_EL1_NV_NI                          UL(0b0000)
#define ID_AA64MMFR2_EL1_NV_IMP                         UL(0b0001)
#define ID_AA64MMFR2_EL1_NV_NV2                         UL(0b0010)

#define ID_AA64MMFR2_EL1_CCIDX                          GENMASK(23, 20)
#define ID_AA64MMFR2_EL1_CCIDX_MASK                     GENMASK(23, 20)
#define ID_AA64MMFR2_EL1_CCIDX_SHIFT                    20
#define ID_AA64MMFR2_EL1_CCIDX_WIDTH                    4
#define ID_AA64MMFR2_EL1_CCIDX_32                       UL(0b0000)
#define ID_AA64MMFR2_EL1_CCIDX_64                       UL(0b0001)

#define ID_AA64MMFR2_EL1_VARange                        GENMASK(19, 16)
#define ID_AA64MMFR2_EL1_VARange_MASK                   GENMASK(19, 16)
#define ID_AA64MMFR2_EL1_VARange_SHIFT                  16
#define ID_AA64MMFR2_EL1_VARange_WIDTH                  4
#define ID_AA64MMFR2_EL1_VARange_SIGNED                 false
#define ID_AA64MMFR2_EL1_VARange_48                     UL(0b0000)
#define ID_AA64MMFR2_EL1_VARange_52                     UL(0b0001)

#define ID_AA64MMFR2_EL1_IESB                           GENMASK(15, 12)
#define ID_AA64MMFR2_EL1_IESB_MASK                      GENMASK(15, 12)
#define ID_AA64MMFR2_EL1_IESB_SHIFT                     12
#define ID_AA64MMFR2_EL1_IESB_WIDTH                     4
#define ID_AA64MMFR2_EL1_IESB_SIGNED                    false
#define ID_AA64MMFR2_EL1_IESB_NI                        UL(0b0000)
#define ID_AA64MMFR2_EL1_IESB_IMP                       UL(0b0001)

#define ID_AA64MMFR2_EL1_LSM                            GENMASK(11, 8)
#define ID_AA64MMFR2_EL1_LSM_MASK                       GENMASK(11, 8)
#define ID_AA64MMFR2_EL1_LSM_SHIFT                      8
#define ID_AA64MMFR2_EL1_LSM_WIDTH                      4
#define ID_AA64MMFR2_EL1_LSM_SIGNED                     false
#define ID_AA64MMFR2_EL1_LSM_NI                         UL(0b0000)
#define ID_AA64MMFR2_EL1_LSM_IMP                        UL(0b0001)

#define ID_AA64MMFR2_EL1_UAO                            GENMASK(7, 4)
#define ID_AA64MMFR2_EL1_UAO_MASK                       GENMASK(7, 4)
#define ID_AA64MMFR2_EL1_UAO_SHIFT                      4
#define ID_AA64MMFR2_EL1_UAO_WIDTH                      4
#define ID_AA64MMFR2_EL1_UAO_SIGNED                     false
#define ID_AA64MMFR2_EL1_UAO_NI                         UL(0b0000)
#define ID_AA64MMFR2_EL1_UAO_IMP                        UL(0b0001)

#define ID_AA64MMFR2_EL1_CnP                            GENMASK(3, 0)
#define ID_AA64MMFR2_EL1_CnP_MASK                       GENMASK(3, 0)
#define ID_AA64MMFR2_EL1_CnP_SHIFT                      0
#define ID_AA64MMFR2_EL1_CnP_WIDTH                      4
#define ID_AA64MMFR2_EL1_CnP_SIGNED                     false
#define ID_AA64MMFR2_EL1_CnP_NI                         UL(0b0000)
#define ID_AA64MMFR2_EL1_CnP_IMP                        UL(0b0001)

#define ID_AA64MMFR2_EL1_RES0                           (UL(0) | GENMASK_ULL(47, 44))
#define ID_AA64MMFR2_EL1_RES1                           (UL(0))
#define ID_AA64MMFR2_EL1_UNKN                           (UL(0))

#define REG_ID_AA64MMFR3_EL1                            S3_0_C0_C7_3
#define SYS_ID_AA64MMFR3_EL1                            sys_reg(3, 0, 0, 7, 3)
#define SYS_ID_AA64MMFR3_EL1_Op0                        3
#define SYS_ID_AA64MMFR3_EL1_Op1                        0
#define SYS_ID_AA64MMFR3_EL1_CRn                        0
#define SYS_ID_AA64MMFR3_EL1_CRm                        7
#define SYS_ID_AA64MMFR3_EL1_Op2                        3

#define ID_AA64MMFR3_EL1_Spec_FPACC                     GENMASK(63, 60)
#define ID_AA64MMFR3_EL1_Spec_FPACC_MASK                GENMASK(63, 60)
#define ID_AA64MMFR3_EL1_Spec_FPACC_SHIFT               60
#define ID_AA64MMFR3_EL1_Spec_FPACC_WIDTH               4
#define ID_AA64MMFR3_EL1_Spec_FPACC_SIGNED              false
#define ID_AA64MMFR3_EL1_Spec_FPACC_NI                  UL(0b0000)
#define ID_AA64MMFR3_EL1_Spec_FPACC_IMP                 UL(0b0001)

#define ID_AA64MMFR3_EL1_ADERR                          GENMASK(59, 56)
#define ID_AA64MMFR3_EL1_ADERR_MASK                     GENMASK(59, 56)
#define ID_AA64MMFR3_EL1_ADERR_SHIFT                    56
#define ID_AA64MMFR3_EL1_ADERR_WIDTH                    4
#define ID_AA64MMFR3_EL1_ADERR_SIGNED                   false
#define ID_AA64MMFR3_EL1_ADERR_NI                       UL(0b0000)
#define ID_AA64MMFR3_EL1_ADERR_DEV_ASYNC                UL(0b0001)
#define ID_AA64MMFR3_EL1_ADERR_FEAT_ADERR               UL(0b0010)
#define ID_AA64MMFR3_EL1_ADERR_FEAT_ADERR_IND           UL(0b0011)

#define ID_AA64MMFR3_EL1_SDERR                          GENMASK(55, 52)
#define ID_AA64MMFR3_EL1_SDERR_MASK                     GENMASK(55, 52)
#define ID_AA64MMFR3_EL1_SDERR_SHIFT                    52
#define ID_AA64MMFR3_EL1_SDERR_WIDTH                    4
#define ID_AA64MMFR3_EL1_SDERR_SIGNED                   false
#define ID_AA64MMFR3_EL1_SDERR_NI                       UL(0b0000)
#define ID_AA64MMFR3_EL1_SDERR_DEV_SYNC                 UL(0b0001)
#define ID_AA64MMFR3_EL1_SDERR_FEAT_ADERR               UL(0b0010)
#define ID_AA64MMFR3_EL1_SDERR_FEAT_ADERR_IND           UL(0b0011)

#define ID_AA64MMFR3_EL1_ANERR                          GENMASK(47, 44)
#define ID_AA64MMFR3_EL1_ANERR_MASK                     GENMASK(47, 44)
#define ID_AA64MMFR3_EL1_ANERR_SHIFT                    44
#define ID_AA64MMFR3_EL1_ANERR_WIDTH                    4
#define ID_AA64MMFR3_EL1_ANERR_SIGNED                   false
#define ID_AA64MMFR3_EL1_ANERR_NI                       UL(0b0000)
#define ID_AA64MMFR3_EL1_ANERR_ASYNC                    UL(0b0001)
#define ID_AA64MMFR3_EL1_ANERR_FEAT_ANERR               UL(0b0010)
#define ID_AA64MMFR3_EL1_ANERR_FEAT_ANERR_IND           UL(0b0011)

#define ID_AA64MMFR3_EL1_SNERR                          GENMASK(43, 40)
#define ID_AA64MMFR3_EL1_SNERR_MASK                     GENMASK(43, 40)
#define ID_AA64MMFR3_EL1_SNERR_SHIFT                    40
#define ID_AA64MMFR3_EL1_SNERR_WIDTH                    4
#define ID_AA64MMFR3_EL1_SNERR_SIGNED                   false
#define ID_AA64MMFR3_EL1_SNERR_NI                       UL(0b0000)
#define ID_AA64MMFR3_EL1_SNERR_SYNC                     UL(0b0001)
#define ID_AA64MMFR3_EL1_SNERR_FEAT_ANERR               UL(0b0010)
#define ID_AA64MMFR3_EL1_SNERR_FEAT_ANERR_IND           UL(0b0011)

#define ID_AA64MMFR3_EL1_D128_2                         GENMASK(39, 36)
#define ID_AA64MMFR3_EL1_D128_2_MASK                    GENMASK(39, 36)
#define ID_AA64MMFR3_EL1_D128_2_SHIFT                   36
#define ID_AA64MMFR3_EL1_D128_2_WIDTH                   4
#define ID_AA64MMFR3_EL1_D128_2_SIGNED                  false
#define ID_AA64MMFR3_EL1_D128_2_NI                      UL(0b0000)
#define ID_AA64MMFR3_EL1_D128_2_IMP                     UL(0b0001)

#define ID_AA64MMFR3_EL1_D128                           GENMASK(35, 32)
#define ID_AA64MMFR3_EL1_D128_MASK                      GENMASK(35, 32)
#define ID_AA64MMFR3_EL1_D128_SHIFT                     32
#define ID_AA64MMFR3_EL1_D128_WIDTH                     4
#define ID_AA64MMFR3_EL1_D128_SIGNED                    false
#define ID_AA64MMFR3_EL1_D128_NI                        UL(0b0000)
#define ID_AA64MMFR3_EL1_D128_IMP                       UL(0b0001)

#define ID_AA64MMFR3_EL1_MEC                            GENMASK(31, 28)
#define ID_AA64MMFR3_EL1_MEC_MASK                       GENMASK(31, 28)
#define ID_AA64MMFR3_EL1_MEC_SHIFT                      28
#define ID_AA64MMFR3_EL1_MEC_WIDTH                      4
#define ID_AA64MMFR3_EL1_MEC_SIGNED                     false
#define ID_AA64MMFR3_EL1_MEC_NI                         UL(0b0000)
#define ID_AA64MMFR3_EL1_MEC_IMP                        UL(0b0001)

#define ID_AA64MMFR3_EL1_AIE                            GENMASK(27, 24)
#define ID_AA64MMFR3_EL1_AIE_MASK                       GENMASK(27, 24)
#define ID_AA64MMFR3_EL1_AIE_SHIFT                      24
#define ID_AA64MMFR3_EL1_AIE_WIDTH                      4
#define ID_AA64MMFR3_EL1_AIE_SIGNED                     false
#define ID_AA64MMFR3_EL1_AIE_NI                         UL(0b0000)
#define ID_AA64MMFR3_EL1_AIE_IMP                        UL(0b0001)

#define ID_AA64MMFR3_EL1_S2POE                          GENMASK(23, 20)
#define ID_AA64MMFR3_EL1_S2POE_MASK                     GENMASK(23, 20)
#define ID_AA64MMFR3_EL1_S2POE_SHIFT                    20
#define ID_AA64MMFR3_EL1_S2POE_WIDTH                    4
#define ID_AA64MMFR3_EL1_S2POE_SIGNED                   false
#define ID_AA64MMFR3_EL1_S2POE_NI                       UL(0b0000)
#define ID_AA64MMFR3_EL1_S2POE_IMP                      UL(0b0001)

#define ID_AA64MMFR3_EL1_S1POE                          GENMASK(19, 16)
#define ID_AA64MMFR3_EL1_S1POE_MASK                     GENMASK(19, 16)
#define ID_AA64MMFR3_EL1_S1POE_SHIFT                    16
#define ID_AA64MMFR3_EL1_S1POE_WIDTH                    4
#define ID_AA64MMFR3_EL1_S1POE_SIGNED                   false
#define ID_AA64MMFR3_EL1_S1POE_NI                       UL(0b0000)
#define ID_AA64MMFR3_EL1_S1POE_IMP                      UL(0b0001)

#define ID_AA64MMFR3_EL1_S2PIE                          GENMASK(15, 12)
#define ID_AA64MMFR3_EL1_S2PIE_MASK                     GENMASK(15, 12)
#define ID_AA64MMFR3_EL1_S2PIE_SHIFT                    12
#define ID_AA64MMFR3_EL1_S2PIE_WIDTH                    4
#define ID_AA64MMFR3_EL1_S2PIE_SIGNED                   false
#define ID_AA64MMFR3_EL1_S2PIE_NI                       UL(0b0000)
#define ID_AA64MMFR3_EL1_S2PIE_IMP                      UL(0b0001)

#define ID_AA64MMFR3_EL1_S1PIE                          GENMASK(11, 8)
#define ID_AA64MMFR3_EL1_S1PIE_MASK                     GENMASK(11, 8)
#define ID_AA64MMFR3_EL1_S1PIE_SHIFT                    8
#define ID_AA64MMFR3_EL1_S1PIE_WIDTH                    4
#define ID_AA64MMFR3_EL1_S1PIE_SIGNED                   false
#define ID_AA64MMFR3_EL1_S1PIE_NI                       UL(0b0000)
#define ID_AA64MMFR3_EL1_S1PIE_IMP                      UL(0b0001)

#define ID_AA64MMFR3_EL1_SCTLRX                         GENMASK(7, 4)
#define ID_AA64MMFR3_EL1_SCTLRX_MASK                    GENMASK(7, 4)
#define ID_AA64MMFR3_EL1_SCTLRX_SHIFT                   4
#define ID_AA64MMFR3_EL1_SCTLRX_WIDTH                   4
#define ID_AA64MMFR3_EL1_SCTLRX_SIGNED                  false
#define ID_AA64MMFR3_EL1_SCTLRX_NI                      UL(0b0000)
#define ID_AA64MMFR3_EL1_SCTLRX_IMP                     UL(0b0001)

#define ID_AA64MMFR3_EL1_TCRX                           GENMASK(3, 0)
#define ID_AA64MMFR3_EL1_TCRX_MASK                      GENMASK(3, 0)
#define ID_AA64MMFR3_EL1_TCRX_SHIFT                     0
#define ID_AA64MMFR3_EL1_TCRX_WIDTH                     4
#define ID_AA64MMFR3_EL1_TCRX_SIGNED                    false
#define ID_AA64MMFR3_EL1_TCRX_NI                        UL(0b0000)
#define ID_AA64MMFR3_EL1_TCRX_IMP                       UL(0b0001)

#define ID_AA64MMFR3_EL1_RES0                           (UL(0) | GENMASK_ULL(51, 48))
#define ID_AA64MMFR3_EL1_RES1                           (UL(0))
#define ID_AA64MMFR3_EL1_UNKN                           (UL(0))

#define REG_ID_AA64MMFR4_EL1                            S3_0_C0_C7_4
#define SYS_ID_AA64MMFR4_EL1                            sys_reg(3, 0, 0, 7, 4)
#define SYS_ID_AA64MMFR4_EL1_Op0                        3
#define SYS_ID_AA64MMFR4_EL1_Op1                        0
#define SYS_ID_AA64MMFR4_EL1_CRn                        0
#define SYS_ID_AA64MMFR4_EL1_CRm                        7
#define SYS_ID_AA64MMFR4_EL1_Op2                        4

#define ID_AA64MMFR4_EL1_SRMASK                         GENMASK(47, 44)
#define ID_AA64MMFR4_EL1_SRMASK_MASK                    GENMASK(47, 44)
#define ID_AA64MMFR4_EL1_SRMASK_SHIFT                   44
#define ID_AA64MMFR4_EL1_SRMASK_WIDTH                   4
#define ID_AA64MMFR4_EL1_SRMASK_SIGNED                  false
#define ID_AA64MMFR4_EL1_SRMASK_NI                      UL(0b0000)
#define ID_AA64MMFR4_EL1_SRMASK_IMP                     UL(0b0001)

#define ID_AA64MMFR4_EL1_E3DSE                          GENMASK(39, 36)
#define ID_AA64MMFR4_EL1_E3DSE_MASK                     GENMASK(39, 36)
#define ID_AA64MMFR4_EL1_E3DSE_SHIFT                    36
#define ID_AA64MMFR4_EL1_E3DSE_WIDTH                    4
#define ID_AA64MMFR4_EL1_E3DSE_SIGNED                   false
#define ID_AA64MMFR4_EL1_E3DSE_NI                       UL(0b0000)
#define ID_AA64MMFR4_EL1_E3DSE_IMP                      UL(0b0001)

#define ID_AA64MMFR4_EL1_RMEGDI                         GENMASK(31, 28)
#define ID_AA64MMFR4_EL1_RMEGDI_MASK                    GENMASK(31, 28)
#define ID_AA64MMFR4_EL1_RMEGDI_SHIFT                   28
#define ID_AA64MMFR4_EL1_RMEGDI_WIDTH                   4
#define ID_AA64MMFR4_EL1_RMEGDI_SIGNED                  false
#define ID_AA64MMFR4_EL1_RMEGDI_NI                      UL(0b0000)
#define ID_AA64MMFR4_EL1_RMEGDI_IMP                     UL(0b0001)

#define ID_AA64MMFR4_EL1_E2H0                           GENMASK(27, 24)
#define ID_AA64MMFR4_EL1_E2H0_MASK                      GENMASK(27, 24)
#define ID_AA64MMFR4_EL1_E2H0_SHIFT                     24
#define ID_AA64MMFR4_EL1_E2H0_WIDTH                     4
#define ID_AA64MMFR4_EL1_E2H0_SIGNED                    true
#define ID_AA64MMFR4_EL1_E2H0_IMP                       UL(0b0000)
#define ID_AA64MMFR4_EL1_E2H0_NI_NV1                    UL(0b1110)
#define ID_AA64MMFR4_EL1_E2H0_NI                        UL(0b1111)

#define ID_AA64MMFR4_EL1_NV_frac                        GENMASK(23, 20)
#define ID_AA64MMFR4_EL1_NV_frac_MASK                   GENMASK(23, 20)
#define ID_AA64MMFR4_EL1_NV_frac_SHIFT                  20
#define ID_AA64MMFR4_EL1_NV_frac_WIDTH                  4
#define ID_AA64MMFR4_EL1_NV_frac_SIGNED                 false
#define ID_AA64MMFR4_EL1_NV_frac_NV_NV2                 UL(0b0000)
#define ID_AA64MMFR4_EL1_NV_frac_NV2_ONLY               UL(0b0001)
#define ID_AA64MMFR4_EL1_NV_frac_NV2P1                  UL(0b0010)

#define ID_AA64MMFR4_EL1_FGWTE3                         GENMASK(19, 16)
#define ID_AA64MMFR4_EL1_FGWTE3_MASK                    GENMASK(19, 16)
#define ID_AA64MMFR4_EL1_FGWTE3_SHIFT                   16
#define ID_AA64MMFR4_EL1_FGWTE3_WIDTH                   4
#define ID_AA64MMFR4_EL1_FGWTE3_SIGNED                  false
#define ID_AA64MMFR4_EL1_FGWTE3_NI                      UL(0b0000)
#define ID_AA64MMFR4_EL1_FGWTE3_IMP                     UL(0b0001)

#define ID_AA64MMFR4_EL1_HACDBS                         GENMASK(15, 12)
#define ID_AA64MMFR4_EL1_HACDBS_MASK                    GENMASK(15, 12)
#define ID_AA64MMFR4_EL1_HACDBS_SHIFT                   12
#define ID_AA64MMFR4_EL1_HACDBS_WIDTH                   4
#define ID_AA64MMFR4_EL1_HACDBS_SIGNED                  false
#define ID_AA64MMFR4_EL1_HACDBS_NI                      UL(0b0000)
#define ID_AA64MMFR4_EL1_HACDBS_IMP                     UL(0b0001)

#define ID_AA64MMFR4_EL1_ASID2                          GENMASK(11, 8)
#define ID_AA64MMFR4_EL1_ASID2_MASK                     GENMASK(11, 8)
#define ID_AA64MMFR4_EL1_ASID2_SHIFT                    8
#define ID_AA64MMFR4_EL1_ASID2_WIDTH                    4
#define ID_AA64MMFR4_EL1_ASID2_SIGNED                   false
#define ID_AA64MMFR4_EL1_ASID2_NI                       UL(0b0000)
#define ID_AA64MMFR4_EL1_ASID2_IMP                      UL(0b0001)

#define ID_AA64MMFR4_EL1_EIESB                          GENMASK(7, 4)
#define ID_AA64MMFR4_EL1_EIESB_MASK                     GENMASK(7, 4)
#define ID_AA64MMFR4_EL1_EIESB_SHIFT                    4
#define ID_AA64MMFR4_EL1_EIESB_WIDTH                    4
#define ID_AA64MMFR4_EL1_EIESB_SIGNED                   false
#define ID_AA64MMFR4_EL1_EIESB_NI                       UL(0b0000)
#define ID_AA64MMFR4_EL1_EIESB_ToEL3                    UL(0b0001)
#define ID_AA64MMFR4_EL1_EIESB_ToELx                    UL(0b0010)
#define ID_AA64MMFR4_EL1_EIESB_ANY                      UL(0b1111)

#define ID_AA64MMFR4_EL1_PoPS                           GENMASK(3, 0)
#define ID_AA64MMFR4_EL1_PoPS_MASK                      GENMASK(3, 0)
#define ID_AA64MMFR4_EL1_PoPS_SHIFT                     0
#define ID_AA64MMFR4_EL1_PoPS_WIDTH                     4
#define ID_AA64MMFR4_EL1_PoPS_SIGNED                    false
#define ID_AA64MMFR4_EL1_PoPS_NI                        UL(0b0000)
#define ID_AA64MMFR4_EL1_PoPS_IMP                       UL(0b0001)

#define ID_AA64MMFR4_EL1_RES0                           (UL(0) | GENMASK_ULL(63, 48) | GENMASK_ULL(43, 40) | GENMASK_ULL(35, 32))
#define ID_AA64MMFR4_EL1_RES1                           (UL(0))
#define ID_AA64MMFR4_EL1_UNKN                           (UL(0))

#define REG_SCTLR_EL1                                   S3_0_C1_C0_0
#define SYS_SCTLR_EL1                                   sys_reg(3, 0, 1, 0, 0)
#define SYS_SCTLR_EL1_Op0                               3
#define SYS_SCTLR_EL1_Op1                               0
#define SYS_SCTLR_EL1_CRn                               1
#define SYS_SCTLR_EL1_CRm                               0
#define SYS_SCTLR_EL1_Op2                               0

#define SCTLR_EL1_TIDCP                                 GENMASK(63, 63)
#define SCTLR_EL1_TIDCP_MASK                            GENMASK(63, 63)
#define SCTLR_EL1_TIDCP_SHIFT                           63
#define SCTLR_EL1_TIDCP_WIDTH                           1

#define SCTLR_EL1_SPINTMASK                             GENMASK(62, 62)
#define SCTLR_EL1_SPINTMASK_MASK                        GENMASK(62, 62)
#define SCTLR_EL1_SPINTMASK_SHIFT                       62
#define SCTLR_EL1_SPINTMASK_WIDTH                       1

#define SCTLR_EL1_NMI                                   GENMASK(61, 61)
#define SCTLR_EL1_NMI_MASK                              GENMASK(61, 61)
#define SCTLR_EL1_NMI_SHIFT                             61
#define SCTLR_EL1_NMI_WIDTH                             1

#define SCTLR_EL1_EnTP2                                 GENMASK(60, 60)
#define SCTLR_EL1_EnTP2_MASK                            GENMASK(60, 60)
#define SCTLR_EL1_EnTP2_SHIFT                           60
#define SCTLR_EL1_EnTP2_WIDTH                           1

#define SCTLR_EL1_TCSO                                  GENMASK(59, 59)
#define SCTLR_EL1_TCSO_MASK                             GENMASK(59, 59)
#define SCTLR_EL1_TCSO_SHIFT                            59
#define SCTLR_EL1_TCSO_WIDTH                            1

#define SCTLR_EL1_TCSO0                                 GENMASK(58, 58)
#define SCTLR_EL1_TCSO0_MASK                            GENMASK(58, 58)
#define SCTLR_EL1_TCSO0_SHIFT                           58
#define SCTLR_EL1_TCSO0_WIDTH                           1

#define SCTLR_EL1_EPAN                                  GENMASK(57, 57)
#define SCTLR_EL1_EPAN_MASK                             GENMASK(57, 57)
#define SCTLR_EL1_EPAN_SHIFT                            57
#define SCTLR_EL1_EPAN_WIDTH                            1

#define SCTLR_EL1_EnALS                                 GENMASK(56, 56)
#define SCTLR_EL1_EnALS_MASK                            GENMASK(56, 56)
#define SCTLR_EL1_EnALS_SHIFT                           56
#define SCTLR_EL1_EnALS_WIDTH                           1

#define SCTLR_EL1_EnAS0                                 GENMASK(55, 55)
#define SCTLR_EL1_EnAS0_MASK                            GENMASK(55, 55)
#define SCTLR_EL1_EnAS0_SHIFT                           55
#define SCTLR_EL1_EnAS0_WIDTH                           1

#define SCTLR_EL1_EnASR                                 GENMASK(54, 54)
#define SCTLR_EL1_EnASR_MASK                            GENMASK(54, 54)
#define SCTLR_EL1_EnASR_SHIFT                           54
#define SCTLR_EL1_EnASR_WIDTH                           1

#define SCTLR_EL1_TME                                   GENMASK(53, 53)
#define SCTLR_EL1_TME_MASK                              GENMASK(53, 53)
#define SCTLR_EL1_TME_SHIFT                             53
#define SCTLR_EL1_TME_WIDTH                             1

#define SCTLR_EL1_TME0                                  GENMASK(52, 52)
#define SCTLR_EL1_TME0_MASK                             GENMASK(52, 52)
#define SCTLR_EL1_TME0_SHIFT                            52
#define SCTLR_EL1_TME0_WIDTH                            1

#define SCTLR_EL1_TMT                                   GENMASK(51, 51)
#define SCTLR_EL1_TMT_MASK                              GENMASK(51, 51)
#define SCTLR_EL1_TMT_SHIFT                             51
#define SCTLR_EL1_TMT_WIDTH                             1

#define SCTLR_EL1_TMT0                                  GENMASK(50, 50)
#define SCTLR_EL1_TMT0_MASK                             GENMASK(50, 50)
#define SCTLR_EL1_TMT0_SHIFT                            50
#define SCTLR_EL1_TMT0_WIDTH                            1

#define SCTLR_EL1_TWEDEL                                GENMASK(49, 46)
#define SCTLR_EL1_TWEDEL_MASK                           GENMASK(49, 46)
#define SCTLR_EL1_TWEDEL_SHIFT                          46
#define SCTLR_EL1_TWEDEL_WIDTH                          4

#define SCTLR_EL1_TWEDEn                                GENMASK(45, 45)
#define SCTLR_EL1_TWEDEn_MASK                           GENMASK(45, 45)
#define SCTLR_EL1_TWEDEn_SHIFT                          45
#define SCTLR_EL1_TWEDEn_WIDTH                          1

#define SCTLR_EL1_DSSBS                                 GENMASK(44, 44)
#define SCTLR_EL1_DSSBS_MASK                            GENMASK(44, 44)
#define SCTLR_EL1_DSSBS_SHIFT                           44
#define SCTLR_EL1_DSSBS_WIDTH                           1

#define SCTLR_EL1_ATA                                   GENMASK(43, 43)
#define SCTLR_EL1_ATA_MASK                              GENMASK(43, 43)
#define SCTLR_EL1_ATA_SHIFT                             43
#define SCTLR_EL1_ATA_WIDTH                             1

#define SCTLR_EL1_ATA0                                  GENMASK(42, 42)
#define SCTLR_EL1_ATA0_MASK                             GENMASK(42, 42)
#define SCTLR_EL1_ATA0_SHIFT                            42
#define SCTLR_EL1_ATA0_WIDTH                            1

#define SCTLR_EL1_TCF                                   GENMASK(41, 40)
#define SCTLR_EL1_TCF_MASK                              GENMASK(41, 40)
#define SCTLR_EL1_TCF_SHIFT                             40
#define SCTLR_EL1_TCF_WIDTH                             2
#define SCTLR_EL1_TCF_NONE                              UL(0b00)
#define SCTLR_EL1_TCF_SYNC                              UL(0b01)
#define SCTLR_EL1_TCF_ASYNC                             UL(0b10)
#define SCTLR_EL1_TCF_ASYMM                             UL(0b11)

#define SCTLR_EL1_TCF0                                  GENMASK(39, 38)
#define SCTLR_EL1_TCF0_MASK                             GENMASK(39, 38)
#define SCTLR_EL1_TCF0_SHIFT                            38
#define SCTLR_EL1_TCF0_WIDTH                            2
#define SCTLR_EL1_TCF0_NONE                             UL(0b00)
#define SCTLR_EL1_TCF0_SYNC                             UL(0b01)
#define SCTLR_EL1_TCF0_ASYNC                            UL(0b10)
#define SCTLR_EL1_TCF0_ASYMM                            UL(0b11)

#define SCTLR_EL1_ITFSB                                 GENMASK(37, 37)
#define SCTLR_EL1_ITFSB_MASK                            GENMASK(37, 37)
#define SCTLR_EL1_ITFSB_SHIFT                           37
#define SCTLR_EL1_ITFSB_WIDTH                           1

#define SCTLR_EL1_BT1                                   GENMASK(36, 36)
#define SCTLR_EL1_BT1_MASK                              GENMASK(36, 36)
#define SCTLR_EL1_BT1_SHIFT                             36
#define SCTLR_EL1_BT1_WIDTH                             1

#define SCTLR_EL1_BT0                                   GENMASK(35, 35)
#define SCTLR_EL1_BT0_MASK                              GENMASK(35, 35)
#define SCTLR_EL1_BT0_SHIFT                             35
#define SCTLR_EL1_BT0_WIDTH                             1

#define SCTLR_EL1_EnFPM                                 GENMASK(34, 34)
#define SCTLR_EL1_EnFPM_MASK                            GENMASK(34, 34)
#define SCTLR_EL1_EnFPM_SHIFT                           34
#define SCTLR_EL1_EnFPM_WIDTH                           1

#define SCTLR_EL1_MSCEn                                 GENMASK(33, 33)
#define SCTLR_EL1_MSCEn_MASK                            GENMASK(33, 33)
#define SCTLR_EL1_MSCEn_SHIFT                           33
#define SCTLR_EL1_MSCEn_WIDTH                           1

#define SCTLR_EL1_CMOW                                  GENMASK(32, 32)
#define SCTLR_EL1_CMOW_MASK                             GENMASK(32, 32)
#define SCTLR_EL1_CMOW_SHIFT                            32
#define SCTLR_EL1_CMOW_WIDTH                            1

#define SCTLR_EL1_EnIA                                  GENMASK(31, 31)
#define SCTLR_EL1_EnIA_MASK                             GENMASK(31, 31)
#define SCTLR_EL1_EnIA_SHIFT                            31
#define SCTLR_EL1_EnIA_WIDTH                            1

#define SCTLR_EL1_EnIB                                  GENMASK(30, 30)
#define SCTLR_EL1_EnIB_MASK                             GENMASK(30, 30)
#define SCTLR_EL1_EnIB_SHIFT                            30
#define SCTLR_EL1_EnIB_WIDTH                            1

#define SCTLR_EL1_LSMAOE                                GENMASK(29, 29)
#define SCTLR_EL1_LSMAOE_MASK                           GENMASK(29, 29)
#define SCTLR_EL1_LSMAOE_SHIFT                          29
#define SCTLR_EL1_LSMAOE_WIDTH                          1

#define SCTLR_EL1_nTLSMD                                GENMASK(28, 28)
#define SCTLR_EL1_nTLSMD_MASK                           GENMASK(28, 28)
#define SCTLR_EL1_nTLSMD_SHIFT                          28
#define SCTLR_EL1_nTLSMD_WIDTH                          1

#define SCTLR_EL1_EnDA                                  GENMASK(27, 27)
#define SCTLR_EL1_EnDA_MASK                             GENMASK(27, 27)
#define SCTLR_EL1_EnDA_SHIFT                            27
#define SCTLR_EL1_EnDA_WIDTH                            1

#define SCTLR_EL1_UCI                                   GENMASK(26, 26)
#define SCTLR_EL1_UCI_MASK                              GENMASK(26, 26)
#define SCTLR_EL1_UCI_SHIFT                             26
#define SCTLR_EL1_UCI_WIDTH                             1

#define SCTLR_EL1_EE                                    GENMASK(25, 25)
#define SCTLR_EL1_EE_MASK                               GENMASK(25, 25)
#define SCTLR_EL1_EE_SHIFT                              25
#define SCTLR_EL1_EE_WIDTH                              1

#define SCTLR_EL1_E0E                                   GENMASK(24, 24)
#define SCTLR_EL1_E0E_MASK                              GENMASK(24, 24)
#define SCTLR_EL1_E0E_SHIFT                             24
#define SCTLR_EL1_E0E_WIDTH                             1

#define SCTLR_EL1_SPAN                                  GENMASK(23, 23)
#define SCTLR_EL1_SPAN_MASK                             GENMASK(23, 23)
#define SCTLR_EL1_SPAN_SHIFT                            23
#define SCTLR_EL1_SPAN_WIDTH                            1

#define SCTLR_EL1_EIS                                   GENMASK(22, 22)
#define SCTLR_EL1_EIS_MASK                              GENMASK(22, 22)
#define SCTLR_EL1_EIS_SHIFT                             22
#define SCTLR_EL1_EIS_WIDTH                             1

#define SCTLR_EL1_IESB                                  GENMASK(21, 21)
#define SCTLR_EL1_IESB_MASK                             GENMASK(21, 21)
#define SCTLR_EL1_IESB_SHIFT                            21
#define SCTLR_EL1_IESB_WIDTH                            1

#define SCTLR_EL1_TSCXT                                 GENMASK(20, 20)
#define SCTLR_EL1_TSCXT_MASK                            GENMASK(20, 20)
#define SCTLR_EL1_TSCXT_SHIFT                           20
#define SCTLR_EL1_TSCXT_WIDTH                           1

#define SCTLR_EL1_WXN                                   GENMASK(19, 19)
#define SCTLR_EL1_WXN_MASK                              GENMASK(19, 19)
#define SCTLR_EL1_WXN_SHIFT                             19
#define SCTLR_EL1_WXN_WIDTH                             1

#define SCTLR_EL1_nTWE                                  GENMASK(18, 18)
#define SCTLR_EL1_nTWE_MASK                             GENMASK(18, 18)
#define SCTLR_EL1_nTWE_SHIFT                            18
#define SCTLR_EL1_nTWE_WIDTH                            1

#define SCTLR_EL1_nTWI                                  GENMASK(16, 16)
#define SCTLR_EL1_nTWI_MASK                             GENMASK(16, 16)
#define SCTLR_EL1_nTWI_SHIFT                            16
#define SCTLR_EL1_nTWI_WIDTH                            1

#define SCTLR_EL1_UCT                                   GENMASK(15, 15)
#define SCTLR_EL1_UCT_MASK                              GENMASK(15, 15)
#define SCTLR_EL1_UCT_SHIFT                             15
#define SCTLR_EL1_UCT_WIDTH                             1

#define SCTLR_EL1_DZE                                   GENMASK(14, 14)
#define SCTLR_EL1_DZE_MASK                              GENMASK(14, 14)
#define SCTLR_EL1_DZE_SHIFT                             14
#define SCTLR_EL1_DZE_WIDTH                             1

#define SCTLR_EL1_EnDB                                  GENMASK(13, 13)
#define SCTLR_EL1_EnDB_MASK                             GENMASK(13, 13)
#define SCTLR_EL1_EnDB_SHIFT                            13
#define SCTLR_EL1_EnDB_WIDTH                            1

#define SCTLR_EL1_I                                     GENMASK(12, 12)
#define SCTLR_EL1_I_MASK                                GENMASK(12, 12)
#define SCTLR_EL1_I_SHIFT                               12
#define SCTLR_EL1_I_WIDTH                               1

#define SCTLR_EL1_EOS                                   GENMASK(11, 11)
#define SCTLR_EL1_EOS_MASK                              GENMASK(11, 11)
#define SCTLR_EL1_EOS_SHIFT                             11
#define SCTLR_EL1_EOS_WIDTH                             1

#define SCTLR_EL1_EnRCTX                                GENMASK(10, 10)
#define SCTLR_EL1_EnRCTX_MASK                           GENMASK(10, 10)
#define SCTLR_EL1_EnRCTX_SHIFT                          10
#define SCTLR_EL1_EnRCTX_WIDTH                          1

#define SCTLR_EL1_UMA                                   GENMASK(9, 9)
#define SCTLR_EL1_UMA_MASK                              GENMASK(9, 9)
#define SCTLR_EL1_UMA_SHIFT                             9
#define SCTLR_EL1_UMA_WIDTH                             1

#define SCTLR_EL1_SED                                   GENMASK(8, 8)
#define SCTLR_EL1_SED_MASK                              GENMASK(8, 8)
#define SCTLR_EL1_SED_SHIFT                             8
#define SCTLR_EL1_SED_WIDTH                             1

#define SCTLR_EL1_ITD                                   GENMASK(7, 7)
#define SCTLR_EL1_ITD_MASK                              GENMASK(7, 7)
#define SCTLR_EL1_ITD_SHIFT                             7
#define SCTLR_EL1_ITD_WIDTH                             1

#define SCTLR_EL1_nAA                                   GENMASK(6, 6)
#define SCTLR_EL1_nAA_MASK                              GENMASK(6, 6)
#define SCTLR_EL1_nAA_SHIFT                             6
#define SCTLR_EL1_nAA_WIDTH                             1

#define SCTLR_EL1_CP15BEN                               GENMASK(5, 5)
#define SCTLR_EL1_CP15BEN_MASK                          GENMASK(5, 5)
#define SCTLR_EL1_CP15BEN_SHIFT                         5
#define SCTLR_EL1_CP15BEN_WIDTH                         1

#define SCTLR_EL1_SA0                                   GENMASK(4, 4)
#define SCTLR_EL1_SA0_MASK                              GENMASK(4, 4)
#define SCTLR_EL1_SA0_SHIFT                             4
#define SCTLR_EL1_SA0_WIDTH                             1

#define SCTLR_EL1_SA                                    GENMASK(3, 3)
#define SCTLR_EL1_SA_MASK                               GENMASK(3, 3)
#define SCTLR_EL1_SA_SHIFT                              3
#define SCTLR_EL1_SA_WIDTH                              1

#define SCTLR_EL1_C                                     GENMASK(2, 2)
#define SCTLR_EL1_C_MASK                                GENMASK(2, 2)
#define SCTLR_EL1_C_SHIFT                               2
#define SCTLR_EL1_C_WIDTH                               1

#define SCTLR_EL1_A                                     GENMASK(1, 1)
#define SCTLR_EL1_A_MASK                                GENMASK(1, 1)
#define SCTLR_EL1_A_SHIFT                               1
#define SCTLR_EL1_A_WIDTH                               1

#define SCTLR_EL1_M                                     GENMASK(0, 0)
#define SCTLR_EL1_M_MASK                                GENMASK(0, 0)
#define SCTLR_EL1_M_SHIFT                               0
#define SCTLR_EL1_M_WIDTH                               1

#define SCTLR_EL1_RES0                                  (UL(0) | GENMASK_ULL(17, 17))
#define SCTLR_EL1_RES1                                  (UL(0))
#define SCTLR_EL1_UNKN                                  (UL(0))

#define REG_SCTLR_EL12                                  S3_5_C1_C0_0
#define SYS_SCTLR_EL12                                  sys_reg(3, 5, 1, 0, 0)
#define SYS_SCTLR_EL12_Op0                              3
#define SYS_SCTLR_EL12_Op1                              5
#define SYS_SCTLR_EL12_CRn                              1
#define SYS_SCTLR_EL12_CRm                              0
#define SYS_SCTLR_EL12_Op2                              0

/* For SCTLR_EL12 fields see SCTLR_EL1 */

#define REG_SCTLRALIAS_EL1                              S3_0_C1_C4_6
#define SYS_SCTLRALIAS_EL1                              sys_reg(3, 0, 1, 4, 6)
#define SYS_SCTLRALIAS_EL1_Op0                          3
#define SYS_SCTLRALIAS_EL1_Op1                          0
#define SYS_SCTLRALIAS_EL1_CRn                          1
#define SYS_SCTLRALIAS_EL1_CRm                          4
#define SYS_SCTLRALIAS_EL1_Op2                          6

/* For SCTLRALIAS_EL1 fields see SCTLR_EL1 */

#define REG_ACTLR_EL1                                   S3_0_C1_C0_1
#define SYS_ACTLR_EL1                                   sys_reg(3, 0, 1, 0, 1)
#define SYS_ACTLR_EL1_Op0                               3
#define SYS_ACTLR_EL1_Op1                               0
#define SYS_ACTLR_EL1_CRn                               1
#define SYS_ACTLR_EL1_CRm                               0
#define SYS_ACTLR_EL1_Op2                               1

#define ACTLR_EL1_IMPDEF                                GENMASK(63, 0)
#define ACTLR_EL1_IMPDEF_MASK                           GENMASK(63, 0)
#define ACTLR_EL1_IMPDEF_SHIFT                          0
#define ACTLR_EL1_IMPDEF_WIDTH                          64

#define ACTLR_EL1_RES0                                  (UL(0))
#define ACTLR_EL1_RES1                                  (UL(0))
#define ACTLR_EL1_UNKN                                  (UL(0))

#define REG_ACTLR_EL12                                  S3_5_C1_C0_1
#define SYS_ACTLR_EL12                                  sys_reg(3, 5, 1, 0, 1)
#define SYS_ACTLR_EL12_Op0                              3
#define SYS_ACTLR_EL12_Op1                              5
#define SYS_ACTLR_EL12_CRn                              1
#define SYS_ACTLR_EL12_CRm                              0
#define SYS_ACTLR_EL12_Op2                              1

/* For ACTLR_EL12 fields see ACTLR_EL1 */

#define REG_ACTLRALIAS_EL1                              S3_0_C1_C4_5
#define SYS_ACTLRALIAS_EL1                              sys_reg(3, 0, 1, 4, 5)
#define SYS_ACTLRALIAS_EL1_Op0                          3
#define SYS_ACTLRALIAS_EL1_Op1                          0
#define SYS_ACTLRALIAS_EL1_CRn                          1
#define SYS_ACTLRALIAS_EL1_CRm                          4
#define SYS_ACTLRALIAS_EL1_Op2                          5

/* For ACTLRALIAS_EL1 fields see ACTLR_EL1 */

#define REG_CPACR_EL1                                   S3_0_C1_C0_2
#define SYS_CPACR_EL1                                   sys_reg(3, 0, 1, 0, 2)
#define SYS_CPACR_EL1_Op0                               3
#define SYS_CPACR_EL1_Op1                               0
#define SYS_CPACR_EL1_CRn                               1
#define SYS_CPACR_EL1_CRm                               0
#define SYS_CPACR_EL1_Op2                               2

#define CPACR_EL1_TCPAC                                 GENMASK(31, 31)
#define CPACR_EL1_TCPAC_MASK                            GENMASK(31, 31)
#define CPACR_EL1_TCPAC_SHIFT                           31
#define CPACR_EL1_TCPAC_WIDTH                           1

#define CPACR_EL1_TAM                                   GENMASK(30, 30)
#define CPACR_EL1_TAM_MASK                              GENMASK(30, 30)
#define CPACR_EL1_TAM_SHIFT                             30
#define CPACR_EL1_TAM_WIDTH                             1

#define CPACR_EL1_E0POE                                 GENMASK(29, 29)
#define CPACR_EL1_E0POE_MASK                            GENMASK(29, 29)
#define CPACR_EL1_E0POE_SHIFT                           29
#define CPACR_EL1_E0POE_WIDTH                           1

#define CPACR_EL1_TTA                                   GENMASK(28, 28)
#define CPACR_EL1_TTA_MASK                              GENMASK(28, 28)
#define CPACR_EL1_TTA_SHIFT                             28
#define CPACR_EL1_TTA_WIDTH                             1

#define CPACR_EL1_SMEN                                  GENMASK(25, 24)
#define CPACR_EL1_SMEN_MASK                             GENMASK(25, 24)
#define CPACR_EL1_SMEN_SHIFT                            24
#define CPACR_EL1_SMEN_WIDTH                            2

#define CPACR_EL1_FPEN                                  GENMASK(21, 20)
#define CPACR_EL1_FPEN_MASK                             GENMASK(21, 20)
#define CPACR_EL1_FPEN_SHIFT                            20
#define CPACR_EL1_FPEN_WIDTH                            2

#define CPACR_EL1_ZEN                                   GENMASK(17, 16)
#define CPACR_EL1_ZEN_MASK                              GENMASK(17, 16)
#define CPACR_EL1_ZEN_SHIFT                             16
#define CPACR_EL1_ZEN_WIDTH                             2

#define CPACR_EL1_RES0                                  (UL(0) | GENMASK_ULL(63, 32) | GENMASK_ULL(27, 26) | GENMASK_ULL(23, 22) | GENMASK_ULL(19, 18) | GENMASK_ULL(15, 0))
#define CPACR_EL1_RES1                                  (UL(0))
#define CPACR_EL1_UNKN                                  (UL(0))

#define REG_CPACRALIAS_EL1                              S3_0_C1_C4_4
#define SYS_CPACRALIAS_EL1                              sys_reg(3, 0, 1, 4, 4)
#define SYS_CPACRALIAS_EL1_Op0                          3
#define SYS_CPACRALIAS_EL1_Op1                          0
#define SYS_CPACRALIAS_EL1_CRn                          1
#define SYS_CPACRALIAS_EL1_CRm                          4
#define SYS_CPACRALIAS_EL1_Op2                          4

/* For CPACRALIAS_EL1 fields see CPACR_EL1 */

#define REG_ACTLRMASK_EL1                               S3_0_C1_C4_1
#define SYS_ACTLRMASK_EL1                               sys_reg(3, 0, 1, 4, 1)
#define SYS_ACTLRMASK_EL1_Op0                           3
#define SYS_ACTLRMASK_EL1_Op1                           0
#define SYS_ACTLRMASK_EL1_CRn                           1
#define SYS_ACTLRMASK_EL1_CRm                           4
#define SYS_ACTLRMASK_EL1_Op2                           1

#define ACTLRMASK_EL1_IMPDEF                            GENMASK(63, 0)
#define ACTLRMASK_EL1_IMPDEF_MASK                       GENMASK(63, 0)
#define ACTLRMASK_EL1_IMPDEF_SHIFT                      0
#define ACTLRMASK_EL1_IMPDEF_WIDTH                      64

#define ACTLRMASK_EL1_RES0                              (UL(0))
#define ACTLRMASK_EL1_RES1                              (UL(0))
#define ACTLRMASK_EL1_UNKN                              (UL(0))

#define REG_ACTLRMASK_EL12                              S3_5_C1_C4_1
#define SYS_ACTLRMASK_EL12                              sys_reg(3, 5, 1, 4, 1)
#define SYS_ACTLRMASK_EL12_Op0                          3
#define SYS_ACTLRMASK_EL12_Op1                          5
#define SYS_ACTLRMASK_EL12_CRn                          1
#define SYS_ACTLRMASK_EL12_CRm                          4
#define SYS_ACTLRMASK_EL12_Op2                          1

/* For ACTLRMASK_EL12 fields see ACTLRMASK_EL1 */

#define REG_CPACRMASK_EL1                               S3_0_C1_C4_2
#define SYS_CPACRMASK_EL1                               sys_reg(3, 0, 1, 4, 2)
#define SYS_CPACRMASK_EL1_Op0                           3
#define SYS_CPACRMASK_EL1_Op1                           0
#define SYS_CPACRMASK_EL1_CRn                           1
#define SYS_CPACRMASK_EL1_CRm                           4
#define SYS_CPACRMASK_EL1_Op2                           2

#define CPACRMASK_EL1_TCPAC                             GENMASK(31, 31)
#define CPACRMASK_EL1_TCPAC_MASK                        GENMASK(31, 31)
#define CPACRMASK_EL1_TCPAC_SHIFT                       31
#define CPACRMASK_EL1_TCPAC_WIDTH                       1

#define CPACRMASK_EL1_TAM                               GENMASK(30, 30)
#define CPACRMASK_EL1_TAM_MASK                          GENMASK(30, 30)
#define CPACRMASK_EL1_TAM_SHIFT                         30
#define CPACRMASK_EL1_TAM_WIDTH                         1

#define CPACRMASK_EL1_E0POE                             GENMASK(29, 29)
#define CPACRMASK_EL1_E0POE_MASK                        GENMASK(29, 29)
#define CPACRMASK_EL1_E0POE_SHIFT                       29
#define CPACRMASK_EL1_E0POE_WIDTH                       1

#define CPACRMASK_EL1_TTA                               GENMASK(28, 28)
#define CPACRMASK_EL1_TTA_MASK                          GENMASK(28, 28)
#define CPACRMASK_EL1_TTA_SHIFT                         28
#define CPACRMASK_EL1_TTA_WIDTH                         1

#define CPACRMASK_EL1_SMEN                              GENMASK(24, 24)
#define CPACRMASK_EL1_SMEN_MASK                         GENMASK(24, 24)
#define CPACRMASK_EL1_SMEN_SHIFT                        24
#define CPACRMASK_EL1_SMEN_WIDTH                        1

#define CPACRMASK_EL1_FPEN                              GENMASK(20, 20)
#define CPACRMASK_EL1_FPEN_MASK                         GENMASK(20, 20)
#define CPACRMASK_EL1_FPEN_SHIFT                        20
#define CPACRMASK_EL1_FPEN_WIDTH                        1

#define CPACRMASK_EL1_ZEN                               GENMASK(16, 16)
#define CPACRMASK_EL1_ZEN_MASK                          GENMASK(16, 16)
#define CPACRMASK_EL1_ZEN_SHIFT                         16
#define CPACRMASK_EL1_ZEN_WIDTH                         1

#define CPACRMASK_EL1_RES0                              (UL(0) | GENMASK_ULL(63, 32) | GENMASK_ULL(27, 25) | GENMASK_ULL(23, 21) | GENMASK_ULL(19, 17) | GENMASK_ULL(15, 0))
#define CPACRMASK_EL1_RES1                              (UL(0))
#define CPACRMASK_EL1_UNKN                              (UL(0))

#define REG_CPACRMASK_EL12                              S3_5_C1_C4_2
#define SYS_CPACRMASK_EL12                              sys_reg(3, 5, 1, 4, 2)
#define SYS_CPACRMASK_EL12_Op0                          3
#define SYS_CPACRMASK_EL12_Op1                          5
#define SYS_CPACRMASK_EL12_CRn                          1
#define SYS_CPACRMASK_EL12_CRm                          4
#define SYS_CPACRMASK_EL12_Op2                          2

/* For CPACRMASK_EL12 fields see CPACRMASK_EL1 */

#define REG_PFAR_EL1                                    S3_0_C6_C0_5
#define SYS_PFAR_EL1                                    sys_reg(3, 0, 6, 0, 5)
#define SYS_PFAR_EL1_Op0                                3
#define SYS_PFAR_EL1_Op1                                0
#define SYS_PFAR_EL1_CRn                                6
#define SYS_PFAR_EL1_CRm                                0
#define SYS_PFAR_EL1_Op2                                5

#define PFAR_EL1_NS                                     GENMASK(63, 63)
#define PFAR_EL1_NS_MASK                                GENMASK(63, 63)
#define PFAR_EL1_NS_SHIFT                               63
#define PFAR_EL1_NS_WIDTH                               1

#define PFAR_EL1_NSE                                    GENMASK(62, 62)
#define PFAR_EL1_NSE_MASK                               GENMASK(62, 62)
#define PFAR_EL1_NSE_SHIFT                              62
#define PFAR_EL1_NSE_WIDTH                              1

#define PFAR_EL1_PA_55_52                               GENMASK(55, 52)
#define PFAR_EL1_PA_55_52_MASK                          GENMASK(55, 52)
#define PFAR_EL1_PA_55_52_SHIFT                         52
#define PFAR_EL1_PA_55_52_WIDTH                         4

#define PFAR_EL1_PA_51_48                               GENMASK(51, 48)
#define PFAR_EL1_PA_51_48_MASK                          GENMASK(51, 48)
#define PFAR_EL1_PA_51_48_SHIFT                         48
#define PFAR_EL1_PA_51_48_WIDTH                         4

#define PFAR_EL1_PA                                     GENMASK(47, 0)
#define PFAR_EL1_PA_MASK                                GENMASK(47, 0)
#define PFAR_EL1_PA_SHIFT                               0
#define PFAR_EL1_PA_WIDTH                               48

#define PFAR_EL1_RES0                                   (UL(0) | GENMASK_ULL(61, 56))
#define PFAR_EL1_RES1                                   (UL(0))
#define PFAR_EL1_UNKN                                   (UL(0))

#define REG_PFAR_EL12                                   S3_5_C6_C0_5
#define SYS_PFAR_EL12                                   sys_reg(3, 5, 6, 0, 5)
#define SYS_PFAR_EL12_Op0                               3
#define SYS_PFAR_EL12_Op1                               5
#define SYS_PFAR_EL12_CRn                               6
#define SYS_PFAR_EL12_CRm                               0
#define SYS_PFAR_EL12_Op2                               5

/* For PFAR_EL12 fields see PFAR_EL1 */

#define REG_SCTLR2_EL1                                  S3_0_C1_C0_3
#define SYS_SCTLR2_EL1                                  sys_reg(3, 0, 1, 0, 3)
#define SYS_SCTLR2_EL1_Op0                              3
#define SYS_SCTLR2_EL1_Op1                              0
#define SYS_SCTLR2_EL1_CRn                              1
#define SYS_SCTLR2_EL1_CRm                              0
#define SYS_SCTLR2_EL1_Op2                              3

#define SCTLR2_EL1_CPTM0                                GENMASK(12, 12)
#define SCTLR2_EL1_CPTM0_MASK                           GENMASK(12, 12)
#define SCTLR2_EL1_CPTM0_SHIFT                          12
#define SCTLR2_EL1_CPTM0_WIDTH                          1

#define SCTLR2_EL1_CPTM                                 GENMASK(11, 11)
#define SCTLR2_EL1_CPTM_MASK                            GENMASK(11, 11)
#define SCTLR2_EL1_CPTM_SHIFT                           11
#define SCTLR2_EL1_CPTM_WIDTH                           1

#define SCTLR2_EL1_CPTA0                                GENMASK(10, 10)
#define SCTLR2_EL1_CPTA0_MASK                           GENMASK(10, 10)
#define SCTLR2_EL1_CPTA0_SHIFT                          10
#define SCTLR2_EL1_CPTA0_WIDTH                          1

#define SCTLR2_EL1_CPTA                                 GENMASK(9, 9)
#define SCTLR2_EL1_CPTA_MASK                            GENMASK(9, 9)
#define SCTLR2_EL1_CPTA_SHIFT                           9
#define SCTLR2_EL1_CPTA_WIDTH                           1

#define SCTLR2_EL1_EnPACM0                              GENMASK(8, 8)
#define SCTLR2_EL1_EnPACM0_MASK                         GENMASK(8, 8)
#define SCTLR2_EL1_EnPACM0_SHIFT                        8
#define SCTLR2_EL1_EnPACM0_WIDTH                        1

#define SCTLR2_EL1_EnPACM                               GENMASK(7, 7)
#define SCTLR2_EL1_EnPACM_MASK                          GENMASK(7, 7)
#define SCTLR2_EL1_EnPACM_SHIFT                         7
#define SCTLR2_EL1_EnPACM_WIDTH                         1

#define SCTLR2_EL1_EnIDCP128                            GENMASK(6, 6)
#define SCTLR2_EL1_EnIDCP128_MASK                       GENMASK(6, 6)
#define SCTLR2_EL1_EnIDCP128_SHIFT                      6
#define SCTLR2_EL1_EnIDCP128_WIDTH                      1

#define SCTLR2_EL1_EASE                                 GENMASK(5, 5)
#define SCTLR2_EL1_EASE_MASK                            GENMASK(5, 5)
#define SCTLR2_EL1_EASE_SHIFT                           5
#define SCTLR2_EL1_EASE_WIDTH                           1

#define SCTLR2_EL1_EnANERR                              GENMASK(4, 4)
#define SCTLR2_EL1_EnANERR_MASK                         GENMASK(4, 4)
#define SCTLR2_EL1_EnANERR_SHIFT                        4
#define SCTLR2_EL1_EnANERR_WIDTH                        1

#define SCTLR2_EL1_EnADERR                              GENMASK(3, 3)
#define SCTLR2_EL1_EnADERR_MASK                         GENMASK(3, 3)
#define SCTLR2_EL1_EnADERR_SHIFT                        3
#define SCTLR2_EL1_EnADERR_WIDTH                        1

#define SCTLR2_EL1_NMEA                                 GENMASK(2, 2)
#define SCTLR2_EL1_NMEA_MASK                            GENMASK(2, 2)
#define SCTLR2_EL1_NMEA_SHIFT                           2
#define SCTLR2_EL1_NMEA_WIDTH                           1

#define SCTLR2_EL1_RES0                                 (UL(0) | GENMASK_ULL(63, 13) | GENMASK_ULL(1, 0))
#define SCTLR2_EL1_RES1                                 (UL(0))
#define SCTLR2_EL1_UNKN                                 (UL(0))

#define REG_SCTLR2_EL12                                 S3_5_C1_C0_3
#define SYS_SCTLR2_EL12                                 sys_reg(3, 5, 1, 0, 3)
#define SYS_SCTLR2_EL12_Op0                             3
#define SYS_SCTLR2_EL12_Op1                             5
#define SYS_SCTLR2_EL12_CRn                             1
#define SYS_SCTLR2_EL12_CRm                             0
#define SYS_SCTLR2_EL12_Op2                             3

/* For SCTLR2_EL12 fields see SCTLR2_EL1 */

#define REG_SCTLR2ALIAS_EL1                             S3_0_C1_C4_7
#define SYS_SCTLR2ALIAS_EL1                             sys_reg(3, 0, 1, 4, 7)
#define SYS_SCTLR2ALIAS_EL1_Op0                         3
#define SYS_SCTLR2ALIAS_EL1_Op1                         0
#define SYS_SCTLR2ALIAS_EL1_CRn                         1
#define SYS_SCTLR2ALIAS_EL1_CRm                         4
#define SYS_SCTLR2ALIAS_EL1_Op2                         7

/* For SCTLR2ALIAS_EL1 fields see SCTLR2_EL1 */

#define REG_SCTLR2MASK_EL1                              S3_0_C1_C4_3
#define SYS_SCTLR2MASK_EL1                              sys_reg(3, 0, 1, 4, 3)
#define SYS_SCTLR2MASK_EL1_Op0                          3
#define SYS_SCTLR2MASK_EL1_Op1                          0
#define SYS_SCTLR2MASK_EL1_CRn                          1
#define SYS_SCTLR2MASK_EL1_CRm                          4
#define SYS_SCTLR2MASK_EL1_Op2                          3

#define SCTLR2MASK_EL1_CPTM0                            GENMASK(12, 12)
#define SCTLR2MASK_EL1_CPTM0_MASK                       GENMASK(12, 12)
#define SCTLR2MASK_EL1_CPTM0_SHIFT                      12
#define SCTLR2MASK_EL1_CPTM0_WIDTH                      1

#define SCTLR2MASK_EL1_CPTM                             GENMASK(11, 11)
#define SCTLR2MASK_EL1_CPTM_MASK                        GENMASK(11, 11)
#define SCTLR2MASK_EL1_CPTM_SHIFT                       11
#define SCTLR2MASK_EL1_CPTM_WIDTH                       1

#define SCTLR2MASK_EL1_CPTA0                            GENMASK(10, 10)
#define SCTLR2MASK_EL1_CPTA0_MASK                       GENMASK(10, 10)
#define SCTLR2MASK_EL1_CPTA0_SHIFT                      10
#define SCTLR2MASK_EL1_CPTA0_WIDTH                      1

#define SCTLR2MASK_EL1_CPTA                             GENMASK(9, 9)
#define SCTLR2MASK_EL1_CPTA_MASK                        GENMASK(9, 9)
#define SCTLR2MASK_EL1_CPTA_SHIFT                       9
#define SCTLR2MASK_EL1_CPTA_WIDTH                       1

#define SCTLR2MASK_EL1_EnPACM0                          GENMASK(8, 8)
#define SCTLR2MASK_EL1_EnPACM0_MASK                     GENMASK(8, 8)
#define SCTLR2MASK_EL1_EnPACM0_SHIFT                    8
#define SCTLR2MASK_EL1_EnPACM0_WIDTH                    1

#define SCTLR2MASK_EL1_EnPACM                           GENMASK(7, 7)
#define SCTLR2MASK_EL1_EnPACM_MASK                      GENMASK(7, 7)
#define SCTLR2MASK_EL1_EnPACM_SHIFT                     7
#define SCTLR2MASK_EL1_EnPACM_WIDTH                     1

#define SCTLR2MASK_EL1_EnIDCP128                        GENMASK(6, 6)
#define SCTLR2MASK_EL1_EnIDCP128_MASK                   GENMASK(6, 6)
#define SCTLR2MASK_EL1_EnIDCP128_SHIFT                  6
#define SCTLR2MASK_EL1_EnIDCP128_WIDTH                  1

#define SCTLR2MASK_EL1_EASE                             GENMASK(5, 5)
#define SCTLR2MASK_EL1_EASE_MASK                        GENMASK(5, 5)
#define SCTLR2MASK_EL1_EASE_SHIFT                       5
#define SCTLR2MASK_EL1_EASE_WIDTH                       1

#define SCTLR2MASK_EL1_EnANERR                          GENMASK(4, 4)
#define SCTLR2MASK_EL1_EnANERR_MASK                     GENMASK(4, 4)
#define SCTLR2MASK_EL1_EnANERR_SHIFT                    4
#define SCTLR2MASK_EL1_EnANERR_WIDTH                    1

#define SCTLR2MASK_EL1_EnADERR                          GENMASK(3, 3)
#define SCTLR2MASK_EL1_EnADERR_MASK                     GENMASK(3, 3)
#define SCTLR2MASK_EL1_EnADERR_SHIFT                    3
#define SCTLR2MASK_EL1_EnADERR_WIDTH                    1

#define SCTLR2MASK_EL1_NMEA                             GENMASK(2, 2)
#define SCTLR2MASK_EL1_NMEA_MASK                        GENMASK(2, 2)
#define SCTLR2MASK_EL1_NMEA_SHIFT                       2
#define SCTLR2MASK_EL1_NMEA_WIDTH                       1

#define SCTLR2MASK_EL1_RES0                             (UL(0) | GENMASK_ULL(63, 13) | GENMASK_ULL(1, 0))
#define SCTLR2MASK_EL1_RES1                             (UL(0))
#define SCTLR2MASK_EL1_UNKN                             (UL(0))

#define REG_SCTLR2MASK_EL12                             S3_5_C1_C4_3
#define SYS_SCTLR2MASK_EL12                             sys_reg(3, 5, 1, 4, 3)
#define SYS_SCTLR2MASK_EL12_Op0                         3
#define SYS_SCTLR2MASK_EL12_Op1                         5
#define SYS_SCTLR2MASK_EL12_CRn                         1
#define SYS_SCTLR2MASK_EL12_CRm                         4
#define SYS_SCTLR2MASK_EL12_Op2                         3

/* For SCTLR2MASK_EL12 fields see SCTLR2MASK_EL1 */

#define REG_SCTLRMASK_EL1                               S3_0_C1_C4_0
#define SYS_SCTLRMASK_EL1                               sys_reg(3, 0, 1, 4, 0)
#define SYS_SCTLRMASK_EL1_Op0                           3
#define SYS_SCTLRMASK_EL1_Op1                           0
#define SYS_SCTLRMASK_EL1_CRn                           1
#define SYS_SCTLRMASK_EL1_CRm                           4
#define SYS_SCTLRMASK_EL1_Op2                           0

#define SCTLRMASK_EL1_TIDCP                             GENMASK(63, 63)
#define SCTLRMASK_EL1_TIDCP_MASK                        GENMASK(63, 63)
#define SCTLRMASK_EL1_TIDCP_SHIFT                       63
#define SCTLRMASK_EL1_TIDCP_WIDTH                       1

#define SCTLRMASK_EL1_SPINTMASK                         GENMASK(62, 62)
#define SCTLRMASK_EL1_SPINTMASK_MASK                    GENMASK(62, 62)
#define SCTLRMASK_EL1_SPINTMASK_SHIFT                   62
#define SCTLRMASK_EL1_SPINTMASK_WIDTH                   1

#define SCTLRMASK_EL1_NMI                               GENMASK(61, 61)
#define SCTLRMASK_EL1_NMI_MASK                          GENMASK(61, 61)
#define SCTLRMASK_EL1_NMI_SHIFT                         61
#define SCTLRMASK_EL1_NMI_WIDTH                         1

#define SCTLRMASK_EL1_EnTP2                             GENMASK(60, 60)
#define SCTLRMASK_EL1_EnTP2_MASK                        GENMASK(60, 60)
#define SCTLRMASK_EL1_EnTP2_SHIFT                       60
#define SCTLRMASK_EL1_EnTP2_WIDTH                       1

#define SCTLRMASK_EL1_TCSO                              GENMASK(59, 59)
#define SCTLRMASK_EL1_TCSO_MASK                         GENMASK(59, 59)
#define SCTLRMASK_EL1_TCSO_SHIFT                        59
#define SCTLRMASK_EL1_TCSO_WIDTH                        1

#define SCTLRMASK_EL1_TCSO0                             GENMASK(58, 58)
#define SCTLRMASK_EL1_TCSO0_MASK                        GENMASK(58, 58)
#define SCTLRMASK_EL1_TCSO0_SHIFT                       58
#define SCTLRMASK_EL1_TCSO0_WIDTH                       1

#define SCTLRMASK_EL1_EPAN                              GENMASK(57, 57)
#define SCTLRMASK_EL1_EPAN_MASK                         GENMASK(57, 57)
#define SCTLRMASK_EL1_EPAN_SHIFT                        57
#define SCTLRMASK_EL1_EPAN_WIDTH                        1

#define SCTLRMASK_EL1_EnALS                             GENMASK(56, 56)
#define SCTLRMASK_EL1_EnALS_MASK                        GENMASK(56, 56)
#define SCTLRMASK_EL1_EnALS_SHIFT                       56
#define SCTLRMASK_EL1_EnALS_WIDTH                       1

#define SCTLRMASK_EL1_EnAS0                             GENMASK(55, 55)
#define SCTLRMASK_EL1_EnAS0_MASK                        GENMASK(55, 55)
#define SCTLRMASK_EL1_EnAS0_SHIFT                       55
#define SCTLRMASK_EL1_EnAS0_WIDTH                       1

#define SCTLRMASK_EL1_EnASR                             GENMASK(54, 54)
#define SCTLRMASK_EL1_EnASR_MASK                        GENMASK(54, 54)
#define SCTLRMASK_EL1_EnASR_SHIFT                       54
#define SCTLRMASK_EL1_EnASR_WIDTH                       1

#define SCTLRMASK_EL1_TME                               GENMASK(53, 53)
#define SCTLRMASK_EL1_TME_MASK                          GENMASK(53, 53)
#define SCTLRMASK_EL1_TME_SHIFT                         53
#define SCTLRMASK_EL1_TME_WIDTH                         1

#define SCTLRMASK_EL1_TME0                              GENMASK(52, 52)
#define SCTLRMASK_EL1_TME0_MASK                         GENMASK(52, 52)
#define SCTLRMASK_EL1_TME0_SHIFT                        52
#define SCTLRMASK_EL1_TME0_WIDTH                        1

#define SCTLRMASK_EL1_TMT                               GENMASK(51, 51)
#define SCTLRMASK_EL1_TMT_MASK                          GENMASK(51, 51)
#define SCTLRMASK_EL1_TMT_SHIFT                         51
#define SCTLRMASK_EL1_TMT_WIDTH                         1

#define SCTLRMASK_EL1_TMT0                              GENMASK(50, 50)
#define SCTLRMASK_EL1_TMT0_MASK                         GENMASK(50, 50)
#define SCTLRMASK_EL1_TMT0_SHIFT                        50
#define SCTLRMASK_EL1_TMT0_WIDTH                        1

#define SCTLRMASK_EL1_TWEDEL                            GENMASK(46, 46)
#define SCTLRMASK_EL1_TWEDEL_MASK                       GENMASK(46, 46)
#define SCTLRMASK_EL1_TWEDEL_SHIFT                      46
#define SCTLRMASK_EL1_TWEDEL_WIDTH                      1

#define SCTLRMASK_EL1_TWEDEn                            GENMASK(45, 45)
#define SCTLRMASK_EL1_TWEDEn_MASK                       GENMASK(45, 45)
#define SCTLRMASK_EL1_TWEDEn_SHIFT                      45
#define SCTLRMASK_EL1_TWEDEn_WIDTH                      1

#define SCTLRMASK_EL1_DSSBS                             GENMASK(44, 44)
#define SCTLRMASK_EL1_DSSBS_MASK                        GENMASK(44, 44)
#define SCTLRMASK_EL1_DSSBS_SHIFT                       44
#define SCTLRMASK_EL1_DSSBS_WIDTH                       1

#define SCTLRMASK_EL1_ATA                               GENMASK(43, 43)
#define SCTLRMASK_EL1_ATA_MASK                          GENMASK(43, 43)
#define SCTLRMASK_EL1_ATA_SHIFT                         43
#define SCTLRMASK_EL1_ATA_WIDTH                         1

#define SCTLRMASK_EL1_ATA0                              GENMASK(42, 42)
#define SCTLRMASK_EL1_ATA0_MASK                         GENMASK(42, 42)
#define SCTLRMASK_EL1_ATA0_SHIFT                        42
#define SCTLRMASK_EL1_ATA0_WIDTH                        1

#define SCTLRMASK_EL1_TCF                               GENMASK(40, 40)
#define SCTLRMASK_EL1_TCF_MASK                          GENMASK(40, 40)
#define SCTLRMASK_EL1_TCF_SHIFT                         40
#define SCTLRMASK_EL1_TCF_WIDTH                         1

#define SCTLRMASK_EL1_TCF0                              GENMASK(38, 38)
#define SCTLRMASK_EL1_TCF0_MASK                         GENMASK(38, 38)
#define SCTLRMASK_EL1_TCF0_SHIFT                        38
#define SCTLRMASK_EL1_TCF0_WIDTH                        1

#define SCTLRMASK_EL1_ITFSB                             GENMASK(37, 37)
#define SCTLRMASK_EL1_ITFSB_MASK                        GENMASK(37, 37)
#define SCTLRMASK_EL1_ITFSB_SHIFT                       37
#define SCTLRMASK_EL1_ITFSB_WIDTH                       1

#define SCTLRMASK_EL1_BT1                               GENMASK(36, 36)
#define SCTLRMASK_EL1_BT1_MASK                          GENMASK(36, 36)
#define SCTLRMASK_EL1_BT1_SHIFT                         36
#define SCTLRMASK_EL1_BT1_WIDTH                         1

#define SCTLRMASK_EL1_BT0                               GENMASK(35, 35)
#define SCTLRMASK_EL1_BT0_MASK                          GENMASK(35, 35)
#define SCTLRMASK_EL1_BT0_SHIFT                         35
#define SCTLRMASK_EL1_BT0_WIDTH                         1

#define SCTLRMASK_EL1_EnFPM                             GENMASK(34, 34)
#define SCTLRMASK_EL1_EnFPM_MASK                        GENMASK(34, 34)
#define SCTLRMASK_EL1_EnFPM_SHIFT                       34
#define SCTLRMASK_EL1_EnFPM_WIDTH                       1

#define SCTLRMASK_EL1_MSCEn                             GENMASK(33, 33)
#define SCTLRMASK_EL1_MSCEn_MASK                        GENMASK(33, 33)
#define SCTLRMASK_EL1_MSCEn_SHIFT                       33
#define SCTLRMASK_EL1_MSCEn_WIDTH                       1

#define SCTLRMASK_EL1_CMOW                              GENMASK(32, 32)
#define SCTLRMASK_EL1_CMOW_MASK                         GENMASK(32, 32)
#define SCTLRMASK_EL1_CMOW_SHIFT                        32
#define SCTLRMASK_EL1_CMOW_WIDTH                        1

#define SCTLRMASK_EL1_EnIA                              GENMASK(31, 31)
#define SCTLRMASK_EL1_EnIA_MASK                         GENMASK(31, 31)
#define SCTLRMASK_EL1_EnIA_SHIFT                        31
#define SCTLRMASK_EL1_EnIA_WIDTH                        1

#define SCTLRMASK_EL1_EnIB                              GENMASK(30, 30)
#define SCTLRMASK_EL1_EnIB_MASK                         GENMASK(30, 30)
#define SCTLRMASK_EL1_EnIB_SHIFT                        30
#define SCTLRMASK_EL1_EnIB_WIDTH                        1

#define SCTLRMASK_EL1_LSMAOE                            GENMASK(29, 29)
#define SCTLRMASK_EL1_LSMAOE_MASK                       GENMASK(29, 29)
#define SCTLRMASK_EL1_LSMAOE_SHIFT                      29
#define SCTLRMASK_EL1_LSMAOE_WIDTH                      1

#define SCTLRMASK_EL1_nTLSMD                            GENMASK(28, 28)
#define SCTLRMASK_EL1_nTLSMD_MASK                       GENMASK(28, 28)
#define SCTLRMASK_EL1_nTLSMD_SHIFT                      28
#define SCTLRMASK_EL1_nTLSMD_WIDTH                      1

#define SCTLRMASK_EL1_EnDA                              GENMASK(27, 27)
#define SCTLRMASK_EL1_EnDA_MASK                         GENMASK(27, 27)
#define SCTLRMASK_EL1_EnDA_SHIFT                        27
#define SCTLRMASK_EL1_EnDA_WIDTH                        1

#define SCTLRMASK_EL1_UCI                               GENMASK(26, 26)
#define SCTLRMASK_EL1_UCI_MASK                          GENMASK(26, 26)
#define SCTLRMASK_EL1_UCI_SHIFT                         26
#define SCTLRMASK_EL1_UCI_WIDTH                         1

#define SCTLRMASK_EL1_EE                                GENMASK(25, 25)
#define SCTLRMASK_EL1_EE_MASK                           GENMASK(25, 25)
#define SCTLRMASK_EL1_EE_SHIFT                          25
#define SCTLRMASK_EL1_EE_WIDTH                          1

#define SCTLRMASK_EL1_E0E                               GENMASK(24, 24)
#define SCTLRMASK_EL1_E0E_MASK                          GENMASK(24, 24)
#define SCTLRMASK_EL1_E0E_SHIFT                         24
#define SCTLRMASK_EL1_E0E_WIDTH                         1

#define SCTLRMASK_EL1_SPAN                              GENMASK(23, 23)
#define SCTLRMASK_EL1_SPAN_MASK                         GENMASK(23, 23)
#define SCTLRMASK_EL1_SPAN_SHIFT                        23
#define SCTLRMASK_EL1_SPAN_WIDTH                        1

#define SCTLRMASK_EL1_EIS                               GENMASK(22, 22)
#define SCTLRMASK_EL1_EIS_MASK                          GENMASK(22, 22)
#define SCTLRMASK_EL1_EIS_SHIFT                         22
#define SCTLRMASK_EL1_EIS_WIDTH                         1

#define SCTLRMASK_EL1_IESB                              GENMASK(21, 21)
#define SCTLRMASK_EL1_IESB_MASK                         GENMASK(21, 21)
#define SCTLRMASK_EL1_IESB_SHIFT                        21
#define SCTLRMASK_EL1_IESB_WIDTH                        1

#define SCTLRMASK_EL1_TSCXT                             GENMASK(20, 20)
#define SCTLRMASK_EL1_TSCXT_MASK                        GENMASK(20, 20)
#define SCTLRMASK_EL1_TSCXT_SHIFT                       20
#define SCTLRMASK_EL1_TSCXT_WIDTH                       1

#define SCTLRMASK_EL1_WXN                               GENMASK(19, 19)
#define SCTLRMASK_EL1_WXN_MASK                          GENMASK(19, 19)
#define SCTLRMASK_EL1_WXN_SHIFT                         19
#define SCTLRMASK_EL1_WXN_WIDTH                         1

#define SCTLRMASK_EL1_nTWE                              GENMASK(18, 18)
#define SCTLRMASK_EL1_nTWE_MASK                         GENMASK(18, 18)
#define SCTLRMASK_EL1_nTWE_SHIFT                        18
#define SCTLRMASK_EL1_nTWE_WIDTH                        1

#define SCTLRMASK_EL1_nTWI                              GENMASK(16, 16)
#define SCTLRMASK_EL1_nTWI_MASK                         GENMASK(16, 16)
#define SCTLRMASK_EL1_nTWI_SHIFT                        16
#define SCTLRMASK_EL1_nTWI_WIDTH                        1

#define SCTLRMASK_EL1_UCT                               GENMASK(15, 15)
#define SCTLRMASK_EL1_UCT_MASK                          GENMASK(15, 15)
#define SCTLRMASK_EL1_UCT_SHIFT                         15
#define SCTLRMASK_EL1_UCT_WIDTH                         1

#define SCTLRMASK_EL1_DZE                               GENMASK(14, 14)
#define SCTLRMASK_EL1_DZE_MASK                          GENMASK(14, 14)
#define SCTLRMASK_EL1_DZE_SHIFT                         14
#define SCTLRMASK_EL1_DZE_WIDTH                         1

#define SCTLRMASK_EL1_EnDB                              GENMASK(13, 13)
#define SCTLRMASK_EL1_EnDB_MASK                         GENMASK(13, 13)
#define SCTLRMASK_EL1_EnDB_SHIFT                        13
#define SCTLRMASK_EL1_EnDB_WIDTH                        1

#define SCTLRMASK_EL1_I                                 GENMASK(12, 12)
#define SCTLRMASK_EL1_I_MASK                            GENMASK(12, 12)
#define SCTLRMASK_EL1_I_SHIFT                           12
#define SCTLRMASK_EL1_I_WIDTH                           1

#define SCTLRMASK_EL1_EOS                               GENMASK(11, 11)
#define SCTLRMASK_EL1_EOS_MASK                          GENMASK(11, 11)
#define SCTLRMASK_EL1_EOS_SHIFT                         11
#define SCTLRMASK_EL1_EOS_WIDTH                         1

#define SCTLRMASK_EL1_EnRCTX                            GENMASK(10, 10)
#define SCTLRMASK_EL1_EnRCTX_MASK                       GENMASK(10, 10)
#define SCTLRMASK_EL1_EnRCTX_SHIFT                      10
#define SCTLRMASK_EL1_EnRCTX_WIDTH                      1

#define SCTLRMASK_EL1_UMA                               GENMASK(9, 9)
#define SCTLRMASK_EL1_UMA_MASK                          GENMASK(9, 9)
#define SCTLRMASK_EL1_UMA_SHIFT                         9
#define SCTLRMASK_EL1_UMA_WIDTH                         1

#define SCTLRMASK_EL1_SED                               GENMASK(8, 8)
#define SCTLRMASK_EL1_SED_MASK                          GENMASK(8, 8)
#define SCTLRMASK_EL1_SED_SHIFT                         8
#define SCTLRMASK_EL1_SED_WIDTH                         1

#define SCTLRMASK_EL1_ITD                               GENMASK(7, 7)
#define SCTLRMASK_EL1_ITD_MASK                          GENMASK(7, 7)
#define SCTLRMASK_EL1_ITD_SHIFT                         7
#define SCTLRMASK_EL1_ITD_WIDTH                         1

#define SCTLRMASK_EL1_nAA                               GENMASK(6, 6)
#define SCTLRMASK_EL1_nAA_MASK                          GENMASK(6, 6)
#define SCTLRMASK_EL1_nAA_SHIFT                         6
#define SCTLRMASK_EL1_nAA_WIDTH                         1

#define SCTLRMASK_EL1_CP15BEN                           GENMASK(5, 5)
#define SCTLRMASK_EL1_CP15BEN_MASK                      GENMASK(5, 5)
#define SCTLRMASK_EL1_CP15BEN_SHIFT                     5
#define SCTLRMASK_EL1_CP15BEN_WIDTH                     1

#define SCTLRMASK_EL1_SA0                               GENMASK(4, 4)
#define SCTLRMASK_EL1_SA0_MASK                          GENMASK(4, 4)
#define SCTLRMASK_EL1_SA0_SHIFT                         4
#define SCTLRMASK_EL1_SA0_WIDTH                         1

#define SCTLRMASK_EL1_SA                                GENMASK(3, 3)
#define SCTLRMASK_EL1_SA_MASK                           GENMASK(3, 3)
#define SCTLRMASK_EL1_SA_SHIFT                          3
#define SCTLRMASK_EL1_SA_WIDTH                          1

#define SCTLRMASK_EL1_C                                 GENMASK(2, 2)
#define SCTLRMASK_EL1_C_MASK                            GENMASK(2, 2)
#define SCTLRMASK_EL1_C_SHIFT                           2
#define SCTLRMASK_EL1_C_WIDTH                           1

#define SCTLRMASK_EL1_A                                 GENMASK(1, 1)
#define SCTLRMASK_EL1_A_MASK                            GENMASK(1, 1)
#define SCTLRMASK_EL1_A_SHIFT                           1
#define SCTLRMASK_EL1_A_WIDTH                           1

#define SCTLRMASK_EL1_M                                 GENMASK(0, 0)
#define SCTLRMASK_EL1_M_MASK                            GENMASK(0, 0)
#define SCTLRMASK_EL1_M_SHIFT                           0
#define SCTLRMASK_EL1_M_WIDTH                           1

#define SCTLRMASK_EL1_RES0                              (UL(0) | GENMASK_ULL(49, 47) | GENMASK_ULL(41, 41) | GENMASK_ULL(39, 39) | GENMASK_ULL(17, 17))
#define SCTLRMASK_EL1_RES1                              (UL(0))
#define SCTLRMASK_EL1_UNKN                              (UL(0))

#define REG_SCTLRMASK_EL12                              S3_5_C1_C4_0
#define SYS_SCTLRMASK_EL12                              sys_reg(3, 5, 1, 4, 0)
#define SYS_SCTLRMASK_EL12_Op0                          3
#define SYS_SCTLRMASK_EL12_Op1                          5
#define SYS_SCTLRMASK_EL12_CRn                          1
#define SYS_SCTLRMASK_EL12_CRm                          4
#define SYS_SCTLRMASK_EL12_Op2                          0

/* For SCTLRMASK_EL12 fields see SCTLRMASK_EL1 */

#define REG_TCR2MASK_EL1                                S3_0_C2_C7_3
#define SYS_TCR2MASK_EL1                                sys_reg(3, 0, 2, 7, 3)
#define SYS_TCR2MASK_EL1_Op0                            3
#define SYS_TCR2MASK_EL1_Op1                            0
#define SYS_TCR2MASK_EL1_CRn                            2
#define SYS_TCR2MASK_EL1_CRm                            7
#define SYS_TCR2MASK_EL1_Op2                            3

#define TCR2MASK_EL1_FNGNA1                             GENMASK(21, 21)
#define TCR2MASK_EL1_FNGNA1_MASK                        GENMASK(21, 21)
#define TCR2MASK_EL1_FNGNA1_SHIFT                       21
#define TCR2MASK_EL1_FNGNA1_WIDTH                       1

#define TCR2MASK_EL1_FNGNA0                             GENMASK(20, 20)
#define TCR2MASK_EL1_FNGNA0_MASK                        GENMASK(20, 20)
#define TCR2MASK_EL1_FNGNA0_SHIFT                       20
#define TCR2MASK_EL1_FNGNA0_WIDTH                       1

#define TCR2MASK_EL1_FNG1                               GENMASK(18, 18)
#define TCR2MASK_EL1_FNG1_MASK                          GENMASK(18, 18)
#define TCR2MASK_EL1_FNG1_SHIFT                         18
#define TCR2MASK_EL1_FNG1_WIDTH                         1

#define TCR2MASK_EL1_FNG0                               GENMASK(17, 17)
#define TCR2MASK_EL1_FNG0_MASK                          GENMASK(17, 17)
#define TCR2MASK_EL1_FNG0_SHIFT                         17
#define TCR2MASK_EL1_FNG0_WIDTH                         1

#define TCR2MASK_EL1_A2                                 GENMASK(16, 16)
#define TCR2MASK_EL1_A2_MASK                            GENMASK(16, 16)
#define TCR2MASK_EL1_A2_SHIFT                           16
#define TCR2MASK_EL1_A2_WIDTH                           1

#define TCR2MASK_EL1_DisCH1                             GENMASK(15, 15)
#define TCR2MASK_EL1_DisCH1_MASK                        GENMASK(15, 15)
#define TCR2MASK_EL1_DisCH1_SHIFT                       15
#define TCR2MASK_EL1_DisCH1_WIDTH                       1

#define TCR2MASK_EL1_DisCH0                             GENMASK(14, 14)
#define TCR2MASK_EL1_DisCH0_MASK                        GENMASK(14, 14)
#define TCR2MASK_EL1_DisCH0_SHIFT                       14
#define TCR2MASK_EL1_DisCH0_WIDTH                       1

#define TCR2MASK_EL1_HAFT                               GENMASK(11, 11)
#define TCR2MASK_EL1_HAFT_MASK                          GENMASK(11, 11)
#define TCR2MASK_EL1_HAFT_SHIFT                         11
#define TCR2MASK_EL1_HAFT_WIDTH                         1

#define TCR2MASK_EL1_PTTWI                              GENMASK(10, 10)
#define TCR2MASK_EL1_PTTWI_MASK                         GENMASK(10, 10)
#define TCR2MASK_EL1_PTTWI_SHIFT                        10
#define TCR2MASK_EL1_PTTWI_WIDTH                        1

#define TCR2MASK_EL1_D128                               GENMASK(5, 5)
#define TCR2MASK_EL1_D128_MASK                          GENMASK(5, 5)
#define TCR2MASK_EL1_D128_SHIFT                         5
#define TCR2MASK_EL1_D128_WIDTH                         1

#define TCR2MASK_EL1_AIE                                GENMASK(4, 4)
#define TCR2MASK_EL1_AIE_MASK                           GENMASK(4, 4)
#define TCR2MASK_EL1_AIE_SHIFT                          4
#define TCR2MASK_EL1_AIE_WIDTH                          1

#define TCR2MASK_EL1_POE                                GENMASK(3, 3)
#define TCR2MASK_EL1_POE_MASK                           GENMASK(3, 3)
#define TCR2MASK_EL1_POE_SHIFT                          3
#define TCR2MASK_EL1_POE_WIDTH                          1

#define TCR2MASK_EL1_E0POE                              GENMASK(2, 2)
#define TCR2MASK_EL1_E0POE_MASK                         GENMASK(2, 2)
#define TCR2MASK_EL1_E0POE_SHIFT                        2
#define TCR2MASK_EL1_E0POE_WIDTH                        1

#define TCR2MASK_EL1_PIE                                GENMASK(1, 1)
#define TCR2MASK_EL1_PIE_MASK                           GENMASK(1, 1)
#define TCR2MASK_EL1_PIE_SHIFT                          1
#define TCR2MASK_EL1_PIE_WIDTH                          1

#define TCR2MASK_EL1_PnCH                               GENMASK(0, 0)
#define TCR2MASK_EL1_PnCH_MASK                          GENMASK(0, 0)
#define TCR2MASK_EL1_PnCH_SHIFT                         0
#define TCR2MASK_EL1_PnCH_WIDTH                         1

#define TCR2MASK_EL1_RES0                               (UL(0) | GENMASK_ULL(63, 22) | GENMASK_ULL(19, 19) | GENMASK_ULL(13, 12) | GENMASK_ULL(9, 6))
#define TCR2MASK_EL1_RES1                               (UL(0))
#define TCR2MASK_EL1_UNKN                               (UL(0))

#define REG_TCR2MASK_EL12                               S3_5_C2_C7_3
#define SYS_TCR2MASK_EL12                               sys_reg(3, 5, 2, 7, 3)
#define SYS_TCR2MASK_EL12_Op0                           3
#define SYS_TCR2MASK_EL12_Op1                           5
#define SYS_TCR2MASK_EL12_CRn                           2
#define SYS_TCR2MASK_EL12_CRm                           7
#define SYS_TCR2MASK_EL12_Op2                           3

/* For TCR2MASK_EL12 fields see TCR2MASK_EL1 */

#define REG_TCRMASK_EL1                                 S3_0_C2_C7_2
#define SYS_TCRMASK_EL1                                 sys_reg(3, 0, 2, 7, 2)
#define SYS_TCRMASK_EL1_Op0                             3
#define SYS_TCRMASK_EL1_Op1                             0
#define SYS_TCRMASK_EL1_CRn                             2
#define SYS_TCRMASK_EL1_CRm                             7
#define SYS_TCRMASK_EL1_Op2                             2

#define TCRMASK_EL1_MTX1                                GENMASK(61, 61)
#define TCRMASK_EL1_MTX1_MASK                           GENMASK(61, 61)
#define TCRMASK_EL1_MTX1_SHIFT                          61
#define TCRMASK_EL1_MTX1_WIDTH                          1

#define TCRMASK_EL1_MTX0                                GENMASK(60, 60)
#define TCRMASK_EL1_MTX0_MASK                           GENMASK(60, 60)
#define TCRMASK_EL1_MTX0_SHIFT                          60
#define TCRMASK_EL1_MTX0_WIDTH                          1

#define TCRMASK_EL1_DS                                  GENMASK(59, 59)
#define TCRMASK_EL1_DS_MASK                             GENMASK(59, 59)
#define TCRMASK_EL1_DS_SHIFT                            59
#define TCRMASK_EL1_DS_WIDTH                            1

#define TCRMASK_EL1_TCMA1                               GENMASK(58, 58)
#define TCRMASK_EL1_TCMA1_MASK                          GENMASK(58, 58)
#define TCRMASK_EL1_TCMA1_SHIFT                         58
#define TCRMASK_EL1_TCMA1_WIDTH                         1

#define TCRMASK_EL1_TCMA0                               GENMASK(57, 57)
#define TCRMASK_EL1_TCMA0_MASK                          GENMASK(57, 57)
#define TCRMASK_EL1_TCMA0_SHIFT                         57
#define TCRMASK_EL1_TCMA0_WIDTH                         1

#define TCRMASK_EL1_E0PD1                               GENMASK(56, 56)
#define TCRMASK_EL1_E0PD1_MASK                          GENMASK(56, 56)
#define TCRMASK_EL1_E0PD1_SHIFT                         56
#define TCRMASK_EL1_E0PD1_WIDTH                         1

#define TCRMASK_EL1_E0PD0                               GENMASK(55, 55)
#define TCRMASK_EL1_E0PD0_MASK                          GENMASK(55, 55)
#define TCRMASK_EL1_E0PD0_SHIFT                         55
#define TCRMASK_EL1_E0PD0_WIDTH                         1

#define TCRMASK_EL1_NFD1                                GENMASK(54, 54)
#define TCRMASK_EL1_NFD1_MASK                           GENMASK(54, 54)
#define TCRMASK_EL1_NFD1_SHIFT                          54
#define TCRMASK_EL1_NFD1_WIDTH                          1

#define TCRMASK_EL1_NFD0                                GENMASK(53, 53)
#define TCRMASK_EL1_NFD0_MASK                           GENMASK(53, 53)
#define TCRMASK_EL1_NFD0_SHIFT                          53
#define TCRMASK_EL1_NFD0_WIDTH                          1

#define TCRMASK_EL1_TBID1                               GENMASK(52, 52)
#define TCRMASK_EL1_TBID1_MASK                          GENMASK(52, 52)
#define TCRMASK_EL1_TBID1_SHIFT                         52
#define TCRMASK_EL1_TBID1_WIDTH                         1

#define TCRMASK_EL1_TBID0                               GENMASK(51, 51)
#define TCRMASK_EL1_TBID0_MASK                          GENMASK(51, 51)
#define TCRMASK_EL1_TBID0_SHIFT                         51
#define TCRMASK_EL1_TBID0_WIDTH                         1

#define TCRMASK_EL1_HWU162                              GENMASK(50, 50)
#define TCRMASK_EL1_HWU162_MASK                         GENMASK(50, 50)
#define TCRMASK_EL1_HWU162_SHIFT                        50
#define TCRMASK_EL1_HWU162_WIDTH                        1

#define TCRMASK_EL1_HWU161                              GENMASK(49, 49)
#define TCRMASK_EL1_HWU161_MASK                         GENMASK(49, 49)
#define TCRMASK_EL1_HWU161_SHIFT                        49
#define TCRMASK_EL1_HWU161_WIDTH                        1

#define TCRMASK_EL1_HWU160                              GENMASK(48, 48)
#define TCRMASK_EL1_HWU160_MASK                         GENMASK(48, 48)
#define TCRMASK_EL1_HWU160_SHIFT                        48
#define TCRMASK_EL1_HWU160_WIDTH                        1

#define TCRMASK_EL1_HWU159                              GENMASK(47, 47)
#define TCRMASK_EL1_HWU159_MASK                         GENMASK(47, 47)
#define TCRMASK_EL1_HWU159_SHIFT                        47
#define TCRMASK_EL1_HWU159_WIDTH                        1

#define TCRMASK_EL1_HWU062                              GENMASK(46, 46)
#define TCRMASK_EL1_HWU062_MASK                         GENMASK(46, 46)
#define TCRMASK_EL1_HWU062_SHIFT                        46
#define TCRMASK_EL1_HWU062_WIDTH                        1

#define TCRMASK_EL1_HWU061                              GENMASK(45, 45)
#define TCRMASK_EL1_HWU061_MASK                         GENMASK(45, 45)
#define TCRMASK_EL1_HWU061_SHIFT                        45
#define TCRMASK_EL1_HWU061_WIDTH                        1

#define TCRMASK_EL1_HWU060                              GENMASK(44, 44)
#define TCRMASK_EL1_HWU060_MASK                         GENMASK(44, 44)
#define TCRMASK_EL1_HWU060_SHIFT                        44
#define TCRMASK_EL1_HWU060_WIDTH                        1

#define TCRMASK_EL1_HWU059                              GENMASK(43, 43)
#define TCRMASK_EL1_HWU059_MASK                         GENMASK(43, 43)
#define TCRMASK_EL1_HWU059_SHIFT                        43
#define TCRMASK_EL1_HWU059_WIDTH                        1

#define TCRMASK_EL1_HPD1                                GENMASK(42, 42)
#define TCRMASK_EL1_HPD1_MASK                           GENMASK(42, 42)
#define TCRMASK_EL1_HPD1_SHIFT                          42
#define TCRMASK_EL1_HPD1_WIDTH                          1

#define TCRMASK_EL1_HPD0                                GENMASK(41, 41)
#define TCRMASK_EL1_HPD0_MASK                           GENMASK(41, 41)
#define TCRMASK_EL1_HPD0_SHIFT                          41
#define TCRMASK_EL1_HPD0_WIDTH                          1

#define TCRMASK_EL1_HD                                  GENMASK(40, 40)
#define TCRMASK_EL1_HD_MASK                             GENMASK(40, 40)
#define TCRMASK_EL1_HD_SHIFT                            40
#define TCRMASK_EL1_HD_WIDTH                            1

#define TCRMASK_EL1_HA                                  GENMASK(39, 39)
#define TCRMASK_EL1_HA_MASK                             GENMASK(39, 39)
#define TCRMASK_EL1_HA_SHIFT                            39
#define TCRMASK_EL1_HA_WIDTH                            1

#define TCRMASK_EL1_TBI1                                GENMASK(38, 38)
#define TCRMASK_EL1_TBI1_MASK                           GENMASK(38, 38)
#define TCRMASK_EL1_TBI1_SHIFT                          38
#define TCRMASK_EL1_TBI1_WIDTH                          1

#define TCRMASK_EL1_TBI0                                GENMASK(37, 37)
#define TCRMASK_EL1_TBI0_MASK                           GENMASK(37, 37)
#define TCRMASK_EL1_TBI0_SHIFT                          37
#define TCRMASK_EL1_TBI0_WIDTH                          1

#define TCRMASK_EL1_AS                                  GENMASK(36, 36)
#define TCRMASK_EL1_AS_MASK                             GENMASK(36, 36)
#define TCRMASK_EL1_AS_SHIFT                            36
#define TCRMASK_EL1_AS_WIDTH                            1

#define TCRMASK_EL1_IPS                                 GENMASK(32, 32)
#define TCRMASK_EL1_IPS_MASK                            GENMASK(32, 32)
#define TCRMASK_EL1_IPS_SHIFT                           32
#define TCRMASK_EL1_IPS_WIDTH                           1

#define TCRMASK_EL1_TG1                                 GENMASK(30, 30)
#define TCRMASK_EL1_TG1_MASK                            GENMASK(30, 30)
#define TCRMASK_EL1_TG1_SHIFT                           30
#define TCRMASK_EL1_TG1_WIDTH                           1

#define TCRMASK_EL1_SH1                                 GENMASK(28, 28)
#define TCRMASK_EL1_SH1_MASK                            GENMASK(28, 28)
#define TCRMASK_EL1_SH1_SHIFT                           28
#define TCRMASK_EL1_SH1_WIDTH                           1

#define TCRMASK_EL1_ORGN1                               GENMASK(26, 26)
#define TCRMASK_EL1_ORGN1_MASK                          GENMASK(26, 26)
#define TCRMASK_EL1_ORGN1_SHIFT                         26
#define TCRMASK_EL1_ORGN1_WIDTH                         1

#define TCRMASK_EL1_IRGN1                               GENMASK(24, 24)
#define TCRMASK_EL1_IRGN1_MASK                          GENMASK(24, 24)
#define TCRMASK_EL1_IRGN1_SHIFT                         24
#define TCRMASK_EL1_IRGN1_WIDTH                         1

#define TCRMASK_EL1_EPD1                                GENMASK(23, 23)
#define TCRMASK_EL1_EPD1_MASK                           GENMASK(23, 23)
#define TCRMASK_EL1_EPD1_SHIFT                          23
#define TCRMASK_EL1_EPD1_WIDTH                          1

#define TCRMASK_EL1_A1                                  GENMASK(22, 22)
#define TCRMASK_EL1_A1_MASK                             GENMASK(22, 22)
#define TCRMASK_EL1_A1_SHIFT                            22
#define TCRMASK_EL1_A1_WIDTH                            1

#define TCRMASK_EL1_T1SZ                                GENMASK(16, 16)
#define TCRMASK_EL1_T1SZ_MASK                           GENMASK(16, 16)
#define TCRMASK_EL1_T1SZ_SHIFT                          16
#define TCRMASK_EL1_T1SZ_WIDTH                          1

#define TCRMASK_EL1_TG0                                 GENMASK(14, 14)
#define TCRMASK_EL1_TG0_MASK                            GENMASK(14, 14)
#define TCRMASK_EL1_TG0_SHIFT                           14
#define TCRMASK_EL1_TG0_WIDTH                           1

#define TCRMASK_EL1_SH0                                 GENMASK(12, 12)
#define TCRMASK_EL1_SH0_MASK                            GENMASK(12, 12)
#define TCRMASK_EL1_SH0_SHIFT                           12
#define TCRMASK_EL1_SH0_WIDTH                           1

#define TCRMASK_EL1_ORGN0                               GENMASK(10, 10)
#define TCRMASK_EL1_ORGN0_MASK                          GENMASK(10, 10)
#define TCRMASK_EL1_ORGN0_SHIFT                         10
#define TCRMASK_EL1_ORGN0_WIDTH                         1

#define TCRMASK_EL1_IRGN0                               GENMASK(8, 8)
#define TCRMASK_EL1_IRGN0_MASK                          GENMASK(8, 8)
#define TCRMASK_EL1_IRGN0_SHIFT                         8
#define TCRMASK_EL1_IRGN0_WIDTH                         1

#define TCRMASK_EL1_EPD0                                GENMASK(7, 7)
#define TCRMASK_EL1_EPD0_MASK                           GENMASK(7, 7)
#define TCRMASK_EL1_EPD0_SHIFT                          7
#define TCRMASK_EL1_EPD0_WIDTH                          1

#define TCRMASK_EL1_T0SZ                                GENMASK(0, 0)
#define TCRMASK_EL1_T0SZ_MASK                           GENMASK(0, 0)
#define TCRMASK_EL1_T0SZ_SHIFT                          0
#define TCRMASK_EL1_T0SZ_WIDTH                          1

#define TCRMASK_EL1_RES0                                (UL(0) | GENMASK_ULL(63, 62) | GENMASK_ULL(35, 33) | GENMASK_ULL(31, 31) | GENMASK_ULL(29, 29) | GENMASK_ULL(27, 27) | GENMASK_ULL(25, 25) | GENMASK_ULL(21, 17) | GENMASK_ULL(15, 15) | GENMASK_ULL(13, 13) | GENMASK_ULL(11, 11) | GENMASK_ULL(9, 9) | GENMASK_ULL(6, 1))
#define TCRMASK_EL1_RES1                                (UL(0))
#define TCRMASK_EL1_UNKN                                (UL(0))

#define REG_TCRMASK_EL12                                S3_5_C2_C7_2
#define SYS_TCRMASK_EL12                                sys_reg(3, 5, 2, 7, 2)
#define SYS_TCRMASK_EL12_Op0                            3
#define SYS_TCRMASK_EL12_Op1                            5
#define SYS_TCRMASK_EL12_CRn                            2
#define SYS_TCRMASK_EL12_CRm                            7
#define SYS_TCRMASK_EL12_Op2                            2

/* For TCRMASK_EL12 fields see TCRMASK_EL1 */

#define REG_ERXGSR_EL1                                  S3_0_C5_C3_2
#define SYS_ERXGSR_EL1                                  sys_reg(3, 0, 5, 3, 2)
#define SYS_ERXGSR_EL1_Op0                              3
#define SYS_ERXGSR_EL1_Op1                              0
#define SYS_ERXGSR_EL1_CRn                              5
#define SYS_ERXGSR_EL1_CRm                              3
#define SYS_ERXGSR_EL1_Op2                              2

#define ERXGSR_EL1_S63                                  GENMASK(63, 63)
#define ERXGSR_EL1_S63_MASK                             GENMASK(63, 63)
#define ERXGSR_EL1_S63_SHIFT                            63
#define ERXGSR_EL1_S63_WIDTH                            1

#define ERXGSR_EL1_S62                                  GENMASK(62, 62)
#define ERXGSR_EL1_S62_MASK                             GENMASK(62, 62)
#define ERXGSR_EL1_S62_SHIFT                            62
#define ERXGSR_EL1_S62_WIDTH                            1

#define ERXGSR_EL1_S61                                  GENMASK(61, 61)
#define ERXGSR_EL1_S61_MASK                             GENMASK(61, 61)
#define ERXGSR_EL1_S61_SHIFT                            61
#define ERXGSR_EL1_S61_WIDTH                            1

#define ERXGSR_EL1_S60                                  GENMASK(60, 60)
#define ERXGSR_EL1_S60_MASK                             GENMASK(60, 60)
#define ERXGSR_EL1_S60_SHIFT                            60
#define ERXGSR_EL1_S60_WIDTH                            1

#define ERXGSR_EL1_S59                                  GENMASK(59, 59)
#define ERXGSR_EL1_S59_MASK                             GENMASK(59, 59)
#define ERXGSR_EL1_S59_SHIFT                            59
#define ERXGSR_EL1_S59_WIDTH                            1

#define ERXGSR_EL1_S58                                  GENMASK(58, 58)
#define ERXGSR_EL1_S58_MASK                             GENMASK(58, 58)
#define ERXGSR_EL1_S58_SHIFT                            58
#define ERXGSR_EL1_S58_WIDTH                            1

#define ERXGSR_EL1_S57                                  GENMASK(57, 57)
#define ERXGSR_EL1_S57_MASK                             GENMASK(57, 57)
#define ERXGSR_EL1_S57_SHIFT                            57
#define ERXGSR_EL1_S57_WIDTH                            1

#define ERXGSR_EL1_S56                                  GENMASK(56, 56)
#define ERXGSR_EL1_S56_MASK                             GENMASK(56, 56)
#define ERXGSR_EL1_S56_SHIFT                            56
#define ERXGSR_EL1_S56_WIDTH                            1

#define ERXGSR_EL1_S55                                  GENMASK(55, 55)
#define ERXGSR_EL1_S55_MASK                             GENMASK(55, 55)
#define ERXGSR_EL1_S55_SHIFT                            55
#define ERXGSR_EL1_S55_WIDTH                            1

#define ERXGSR_EL1_S54                                  GENMASK(54, 54)
#define ERXGSR_EL1_S54_MASK                             GENMASK(54, 54)
#define ERXGSR_EL1_S54_SHIFT                            54
#define ERXGSR_EL1_S54_WIDTH                            1

#define ERXGSR_EL1_S53                                  GENMASK(53, 53)
#define ERXGSR_EL1_S53_MASK                             GENMASK(53, 53)
#define ERXGSR_EL1_S53_SHIFT                            53
#define ERXGSR_EL1_S53_WIDTH                            1

#define ERXGSR_EL1_S52                                  GENMASK(52, 52)
#define ERXGSR_EL1_S52_MASK                             GENMASK(52, 52)
#define ERXGSR_EL1_S52_SHIFT                            52
#define ERXGSR_EL1_S52_WIDTH                            1

#define ERXGSR_EL1_S51                                  GENMASK(51, 51)
#define ERXGSR_EL1_S51_MASK                             GENMASK(51, 51)
#define ERXGSR_EL1_S51_SHIFT                            51
#define ERXGSR_EL1_S51_WIDTH                            1

#define ERXGSR_EL1_S50                                  GENMASK(50, 50)
#define ERXGSR_EL1_S50_MASK                             GENMASK(50, 50)
#define ERXGSR_EL1_S50_SHIFT                            50
#define ERXGSR_EL1_S50_WIDTH                            1

#define ERXGSR_EL1_S49                                  GENMASK(49, 49)
#define ERXGSR_EL1_S49_MASK                             GENMASK(49, 49)
#define ERXGSR_EL1_S49_SHIFT                            49
#define ERXGSR_EL1_S49_WIDTH                            1

#define ERXGSR_EL1_S48                                  GENMASK(48, 48)
#define ERXGSR_EL1_S48_MASK                             GENMASK(48, 48)
#define ERXGSR_EL1_S48_SHIFT                            48
#define ERXGSR_EL1_S48_WIDTH                            1

#define ERXGSR_EL1_S47                                  GENMASK(47, 47)
#define ERXGSR_EL1_S47_MASK                             GENMASK(47, 47)
#define ERXGSR_EL1_S47_SHIFT                            47
#define ERXGSR_EL1_S47_WIDTH                            1

#define ERXGSR_EL1_S46                                  GENMASK(46, 46)
#define ERXGSR_EL1_S46_MASK                             GENMASK(46, 46)
#define ERXGSR_EL1_S46_SHIFT                            46
#define ERXGSR_EL1_S46_WIDTH                            1

#define ERXGSR_EL1_S45                                  GENMASK(45, 45)
#define ERXGSR_EL1_S45_MASK                             GENMASK(45, 45)
#define ERXGSR_EL1_S45_SHIFT                            45
#define ERXGSR_EL1_S45_WIDTH                            1

#define ERXGSR_EL1_S44                                  GENMASK(44, 44)
#define ERXGSR_EL1_S44_MASK                             GENMASK(44, 44)
#define ERXGSR_EL1_S44_SHIFT                            44
#define ERXGSR_EL1_S44_WIDTH                            1

#define ERXGSR_EL1_S43                                  GENMASK(43, 43)
#define ERXGSR_EL1_S43_MASK                             GENMASK(43, 43)
#define ERXGSR_EL1_S43_SHIFT                            43
#define ERXGSR_EL1_S43_WIDTH                            1

#define ERXGSR_EL1_S42                                  GENMASK(42, 42)
#define ERXGSR_EL1_S42_MASK                             GENMASK(42, 42)
#define ERXGSR_EL1_S42_SHIFT                            42
#define ERXGSR_EL1_S42_WIDTH                            1

#define ERXGSR_EL1_S41                                  GENMASK(41, 41)
#define ERXGSR_EL1_S41_MASK                             GENMASK(41, 41)
#define ERXGSR_EL1_S41_SHIFT                            41
#define ERXGSR_EL1_S41_WIDTH                            1

#define ERXGSR_EL1_S40                                  GENMASK(40, 40)
#define ERXGSR_EL1_S40_MASK                             GENMASK(40, 40)
#define ERXGSR_EL1_S40_SHIFT                            40
#define ERXGSR_EL1_S40_WIDTH                            1

#define ERXGSR_EL1_S39                                  GENMASK(39, 39)
#define ERXGSR_EL1_S39_MASK                             GENMASK(39, 39)
#define ERXGSR_EL1_S39_SHIFT                            39
#define ERXGSR_EL1_S39_WIDTH                            1

#define ERXGSR_EL1_S38                                  GENMASK(38, 38)
#define ERXGSR_EL1_S38_MASK                             GENMASK(38, 38)
#define ERXGSR_EL1_S38_SHIFT                            38
#define ERXGSR_EL1_S38_WIDTH                            1

#define ERXGSR_EL1_S37                                  GENMASK(37, 37)
#define ERXGSR_EL1_S37_MASK                             GENMASK(37, 37)
#define ERXGSR_EL1_S37_SHIFT                            37
#define ERXGSR_EL1_S37_WIDTH                            1

#define ERXGSR_EL1_S36                                  GENMASK(36, 36)
#define ERXGSR_EL1_S36_MASK                             GENMASK(36, 36)
#define ERXGSR_EL1_S36_SHIFT                            36
#define ERXGSR_EL1_S36_WIDTH                            1

#define ERXGSR_EL1_S35                                  GENMASK(35, 35)
#define ERXGSR_EL1_S35_MASK                             GENMASK(35, 35)
#define ERXGSR_EL1_S35_SHIFT                            35
#define ERXGSR_EL1_S35_WIDTH                            1

#define ERXGSR_EL1_S34                                  GENMASK(34, 34)
#define ERXGSR_EL1_S34_MASK                             GENMASK(34, 34)
#define ERXGSR_EL1_S34_SHIFT                            34
#define ERXGSR_EL1_S34_WIDTH                            1

#define ERXGSR_EL1_S33                                  GENMASK(33, 33)
#define ERXGSR_EL1_S33_MASK                             GENMASK(33, 33)
#define ERXGSR_EL1_S33_SHIFT                            33
#define ERXGSR_EL1_S33_WIDTH                            1

#define ERXGSR_EL1_S32                                  GENMASK(32, 32)
#define ERXGSR_EL1_S32_MASK                             GENMASK(32, 32)
#define ERXGSR_EL1_S32_SHIFT                            32
#define ERXGSR_EL1_S32_WIDTH                            1

#define ERXGSR_EL1_S31                                  GENMASK(31, 31)
#define ERXGSR_EL1_S31_MASK                             GENMASK(31, 31)
#define ERXGSR_EL1_S31_SHIFT                            31
#define ERXGSR_EL1_S31_WIDTH                            1

#define ERXGSR_EL1_S30                                  GENMASK(30, 30)
#define ERXGSR_EL1_S30_MASK                             GENMASK(30, 30)
#define ERXGSR_EL1_S30_SHIFT                            30
#define ERXGSR_EL1_S30_WIDTH                            1

#define ERXGSR_EL1_S29                                  GENMASK(29, 29)
#define ERXGSR_EL1_S29_MASK                             GENMASK(29, 29)
#define ERXGSR_EL1_S29_SHIFT                            29
#define ERXGSR_EL1_S29_WIDTH                            1

#define ERXGSR_EL1_S28                                  GENMASK(28, 28)
#define ERXGSR_EL1_S28_MASK                             GENMASK(28, 28)
#define ERXGSR_EL1_S28_SHIFT                            28
#define ERXGSR_EL1_S28_WIDTH                            1

#define ERXGSR_EL1_S27                                  GENMASK(27, 27)
#define ERXGSR_EL1_S27_MASK                             GENMASK(27, 27)
#define ERXGSR_EL1_S27_SHIFT                            27
#define ERXGSR_EL1_S27_WIDTH                            1

#define ERXGSR_EL1_S26                                  GENMASK(26, 26)
#define ERXGSR_EL1_S26_MASK                             GENMASK(26, 26)
#define ERXGSR_EL1_S26_SHIFT                            26
#define ERXGSR_EL1_S26_WIDTH                            1

#define ERXGSR_EL1_S25                                  GENMASK(25, 25)
#define ERXGSR_EL1_S25_MASK                             GENMASK(25, 25)
#define ERXGSR_EL1_S25_SHIFT                            25
#define ERXGSR_EL1_S25_WIDTH                            1

#define ERXGSR_EL1_S24                                  GENMASK(24, 24)
#define ERXGSR_EL1_S24_MASK                             GENMASK(24, 24)
#define ERXGSR_EL1_S24_SHIFT                            24
#define ERXGSR_EL1_S24_WIDTH                            1

#define ERXGSR_EL1_S23                                  GENMASK(23, 23)
#define ERXGSR_EL1_S23_MASK                             GENMASK(23, 23)
#define ERXGSR_EL1_S23_SHIFT                            23
#define ERXGSR_EL1_S23_WIDTH                            1

#define ERXGSR_EL1_S22                                  GENMASK(22, 22)
#define ERXGSR_EL1_S22_MASK                             GENMASK(22, 22)
#define ERXGSR_EL1_S22_SHIFT                            22
#define ERXGSR_EL1_S22_WIDTH                            1

#define ERXGSR_EL1_S21                                  GENMASK(21, 21)
#define ERXGSR_EL1_S21_MASK                             GENMASK(21, 21)
#define ERXGSR_EL1_S21_SHIFT                            21
#define ERXGSR_EL1_S21_WIDTH                            1

#define ERXGSR_EL1_S20                                  GENMASK(20, 20)
#define ERXGSR_EL1_S20_MASK                             GENMASK(20, 20)
#define ERXGSR_EL1_S20_SHIFT                            20
#define ERXGSR_EL1_S20_WIDTH                            1

#define ERXGSR_EL1_S19                                  GENMASK(19, 19)
#define ERXGSR_EL1_S19_MASK                             GENMASK(19, 19)
#define ERXGSR_EL1_S19_SHIFT                            19
#define ERXGSR_EL1_S19_WIDTH                            1

#define ERXGSR_EL1_S18                                  GENMASK(18, 18)
#define ERXGSR_EL1_S18_MASK                             GENMASK(18, 18)
#define ERXGSR_EL1_S18_SHIFT                            18
#define ERXGSR_EL1_S18_WIDTH                            1

#define ERXGSR_EL1_S17                                  GENMASK(17, 17)
#define ERXGSR_EL1_S17_MASK                             GENMASK(17, 17)
#define ERXGSR_EL1_S17_SHIFT                            17
#define ERXGSR_EL1_S17_WIDTH                            1

#define ERXGSR_EL1_S16                                  GENMASK(16, 16)
#define ERXGSR_EL1_S16_MASK                             GENMASK(16, 16)
#define ERXGSR_EL1_S16_SHIFT                            16
#define ERXGSR_EL1_S16_WIDTH                            1

#define ERXGSR_EL1_S15                                  GENMASK(15, 15)
#define ERXGSR_EL1_S15_MASK                             GENMASK(15, 15)
#define ERXGSR_EL1_S15_SHIFT                            15
#define ERXGSR_EL1_S15_WIDTH                            1

#define ERXGSR_EL1_S14                                  GENMASK(14, 14)
#define ERXGSR_EL1_S14_MASK                             GENMASK(14, 14)
#define ERXGSR_EL1_S14_SHIFT                            14
#define ERXGSR_EL1_S14_WIDTH                            1

#define ERXGSR_EL1_S13                                  GENMASK(13, 13)
#define ERXGSR_EL1_S13_MASK                             GENMASK(13, 13)
#define ERXGSR_EL1_S13_SHIFT                            13
#define ERXGSR_EL1_S13_WIDTH                            1

#define ERXGSR_EL1_S12                                  GENMASK(12, 12)
#define ERXGSR_EL1_S12_MASK                             GENMASK(12, 12)
#define ERXGSR_EL1_S12_SHIFT                            12
#define ERXGSR_EL1_S12_WIDTH                            1

#define ERXGSR_EL1_S11                                  GENMASK(11, 11)
#define ERXGSR_EL1_S11_MASK                             GENMASK(11, 11)
#define ERXGSR_EL1_S11_SHIFT                            11
#define ERXGSR_EL1_S11_WIDTH                            1

#define ERXGSR_EL1_S10                                  GENMASK(10, 10)
#define ERXGSR_EL1_S10_MASK                             GENMASK(10, 10)
#define ERXGSR_EL1_S10_SHIFT                            10
#define ERXGSR_EL1_S10_WIDTH                            1

#define ERXGSR_EL1_S9                                   GENMASK(9, 9)
#define ERXGSR_EL1_S9_MASK                              GENMASK(9, 9)
#define ERXGSR_EL1_S9_SHIFT                             9
#define ERXGSR_EL1_S9_WIDTH                             1

#define ERXGSR_EL1_S8                                   GENMASK(8, 8)
#define ERXGSR_EL1_S8_MASK                              GENMASK(8, 8)
#define ERXGSR_EL1_S8_SHIFT                             8
#define ERXGSR_EL1_S8_WIDTH                             1

#define ERXGSR_EL1_S7                                   GENMASK(7, 7)
#define ERXGSR_EL1_S7_MASK                              GENMASK(7, 7)
#define ERXGSR_EL1_S7_SHIFT                             7
#define ERXGSR_EL1_S7_WIDTH                             1

#define ERXGSR_EL1_S6                                   GENMASK(6, 6)
#define ERXGSR_EL1_S6_MASK                              GENMASK(6, 6)
#define ERXGSR_EL1_S6_SHIFT                             6
#define ERXGSR_EL1_S6_WIDTH                             1

#define ERXGSR_EL1_S5                                   GENMASK(5, 5)
#define ERXGSR_EL1_S5_MASK                              GENMASK(5, 5)
#define ERXGSR_EL1_S5_SHIFT                             5
#define ERXGSR_EL1_S5_WIDTH                             1

#define ERXGSR_EL1_S4                                   GENMASK(4, 4)
#define ERXGSR_EL1_S4_MASK                              GENMASK(4, 4)
#define ERXGSR_EL1_S4_SHIFT                             4
#define ERXGSR_EL1_S4_WIDTH                             1

#define ERXGSR_EL1_S3                                   GENMASK(3, 3)
#define ERXGSR_EL1_S3_MASK                              GENMASK(3, 3)
#define ERXGSR_EL1_S3_SHIFT                             3
#define ERXGSR_EL1_S3_WIDTH                             1

#define ERXGSR_EL1_S2                                   GENMASK(2, 2)
#define ERXGSR_EL1_S2_MASK                              GENMASK(2, 2)
#define ERXGSR_EL1_S2_SHIFT                             2
#define ERXGSR_EL1_S2_WIDTH                             1

#define ERXGSR_EL1_S1                                   GENMASK(1, 1)
#define ERXGSR_EL1_S1_MASK                              GENMASK(1, 1)
#define ERXGSR_EL1_S1_SHIFT                             1
#define ERXGSR_EL1_S1_WIDTH                             1

#define ERXGSR_EL1_S0                                   GENMASK(0, 0)
#define ERXGSR_EL1_S0_MASK                              GENMASK(0, 0)
#define ERXGSR_EL1_S0_SHIFT                             0
#define ERXGSR_EL1_S0_WIDTH                             1

#define ERXGSR_EL1_RES0                                 (UL(0))
#define ERXGSR_EL1_RES1                                 (UL(0))
#define ERXGSR_EL1_UNKN                                 (UL(0))

#define REG_TRFCR_EL1                                   S3_0_C1_C2_1
#define SYS_TRFCR_EL1                                   sys_reg(3, 0, 1, 2, 1)
#define SYS_TRFCR_EL1_Op0                               3
#define SYS_TRFCR_EL1_Op1                               0
#define SYS_TRFCR_EL1_CRn                               1
#define SYS_TRFCR_EL1_CRm                               2
#define SYS_TRFCR_EL1_Op2                               1

#define TRFCR_EL1_TS                                    GENMASK(6, 5)
#define TRFCR_EL1_TS_MASK                               GENMASK(6, 5)
#define TRFCR_EL1_TS_SHIFT                              5
#define TRFCR_EL1_TS_WIDTH                              2
#define TRFCR_EL1_TS_SIGNED                             false
#define TRFCR_EL1_TS_VIRTUAL                            UL(0b0001)
#define TRFCR_EL1_TS_GUEST_PHYSICAL                     UL(0b0010)
#define TRFCR_EL1_TS_PHYSICAL                           UL(0b0011)

#define TRFCR_EL1_ExTRE                                 GENMASK(1, 1)
#define TRFCR_EL1_ExTRE_MASK                            GENMASK(1, 1)
#define TRFCR_EL1_ExTRE_SHIFT                           1
#define TRFCR_EL1_ExTRE_WIDTH                           1

#define TRFCR_EL1_E0TRE                                 GENMASK(0, 0)
#define TRFCR_EL1_E0TRE_MASK                            GENMASK(0, 0)
#define TRFCR_EL1_E0TRE_SHIFT                           0
#define TRFCR_EL1_E0TRE_WIDTH                           1

#define TRFCR_EL1_RES0                                  (UL(0) | GENMASK_ULL(63, 7) | GENMASK_ULL(4, 2))
#define TRFCR_EL1_RES1                                  (UL(0))
#define TRFCR_EL1_UNKN                                  (UL(0))

#define REG_TRCITECR_EL1                                S3_0_C1_C2_3
#define SYS_TRCITECR_EL1                                sys_reg(3, 0, 1, 2, 3)
#define SYS_TRCITECR_EL1_Op0                            3
#define SYS_TRCITECR_EL1_Op1                            0
#define SYS_TRCITECR_EL1_CRn                            1
#define SYS_TRCITECR_EL1_CRm                            2
#define SYS_TRCITECR_EL1_Op2                            3

#define TRCITECR_EL1_E1E                                GENMASK(1, 1)
#define TRCITECR_EL1_E1E_MASK                           GENMASK(1, 1)
#define TRCITECR_EL1_E1E_SHIFT                          1
#define TRCITECR_EL1_E1E_WIDTH                          1

#define TRCITECR_EL1_E0E                                GENMASK(0, 0)
#define TRCITECR_EL1_E0E_MASK                           GENMASK(0, 0)
#define TRCITECR_EL1_E0E_SHIFT                          0
#define TRCITECR_EL1_E0E_WIDTH                          1

#define TRCITECR_EL1_RES0                               (UL(0) | GENMASK_ULL(63, 2))
#define TRCITECR_EL1_RES1                               (UL(0))
#define TRCITECR_EL1_UNKN                               (UL(0))

#define REG_TRCITECR_EL12                               S3_5_C1_C2_3
#define SYS_TRCITECR_EL12                               sys_reg(3, 5, 1, 2, 3)
#define SYS_TRCITECR_EL12_Op0                           3
#define SYS_TRCITECR_EL12_Op1                           5
#define SYS_TRCITECR_EL12_CRn                           1
#define SYS_TRCITECR_EL12_CRm                           2
#define SYS_TRCITECR_EL12_Op2                           3

/* For TRCITECR_EL12 fields see TRCITECR_EL1 */

#define REG_SMPRI_EL1                                   S3_0_C1_C2_4
#define SYS_SMPRI_EL1                                   sys_reg(3, 0, 1, 2, 4)
#define SYS_SMPRI_EL1_Op0                               3
#define SYS_SMPRI_EL1_Op1                               0
#define SYS_SMPRI_EL1_CRn                               1
#define SYS_SMPRI_EL1_CRm                               2
#define SYS_SMPRI_EL1_Op2                               4

#define SMPRI_EL1_PRIORITY                              GENMASK(3, 0)
#define SMPRI_EL1_PRIORITY_MASK                         GENMASK(3, 0)
#define SMPRI_EL1_PRIORITY_SHIFT                        0
#define SMPRI_EL1_PRIORITY_WIDTH                        4

#define SMPRI_EL1_RES0                                  (UL(0) | GENMASK_ULL(63, 4))
#define SMPRI_EL1_RES1                                  (UL(0))
#define SMPRI_EL1_UNKN                                  (UL(0))

#define ZCR_ELx_LEN                                     GENMASK(3, 0)
#define ZCR_ELx_LEN_MASK                                GENMASK(3, 0)
#define ZCR_ELx_LEN_SHIFT                               0
#define ZCR_ELx_LEN_WIDTH                               4

#define ZCR_ELx_RES0                                    (UL(0) | GENMASK_ULL(63, 9))
#define ZCR_ELx_RES1                                    (UL(0))
#define ZCR_ELx_UNKN                                    (UL(0))

#define REG_ZCR_EL1                                     S3_0_C1_C2_0
#define SYS_ZCR_EL1                                     sys_reg(3, 0, 1, 2, 0)
#define SYS_ZCR_EL1_Op0                                 3
#define SYS_ZCR_EL1_Op1                                 0
#define SYS_ZCR_EL1_CRn                                 1
#define SYS_ZCR_EL1_CRm                                 2
#define SYS_ZCR_EL1_Op2                                 0

/* For ZCR_EL1 fields see ZCR_ELx */

#define SMCR_ELx_FA64                                   GENMASK(31, 31)
#define SMCR_ELx_FA64_MASK                              GENMASK(31, 31)
#define SMCR_ELx_FA64_SHIFT                             31
#define SMCR_ELx_FA64_WIDTH                             1

#define SMCR_ELx_EZT0                                   GENMASK(30, 30)
#define SMCR_ELx_EZT0_MASK                              GENMASK(30, 30)
#define SMCR_ELx_EZT0_SHIFT                             30
#define SMCR_ELx_EZT0_WIDTH                             1

#define SMCR_ELx_LEN                                    GENMASK(3, 0)
#define SMCR_ELx_LEN_MASK                               GENMASK(3, 0)
#define SMCR_ELx_LEN_SHIFT                              0
#define SMCR_ELx_LEN_WIDTH                              4

#define SMCR_ELx_RES0                                   (UL(0) | GENMASK_ULL(63, 32) | GENMASK_ULL(29, 9))
#define SMCR_ELx_RES1                                   (UL(0))
#define SMCR_ELx_UNKN                                   (UL(0))

#define REG_SMCR_EL1                                    S3_0_C1_C2_6
#define SYS_SMCR_EL1                                    sys_reg(3, 0, 1, 2, 6)
#define SYS_SMCR_EL1_Op0                                3
#define SYS_SMCR_EL1_Op1                                0
#define SYS_SMCR_EL1_CRn                                1
#define SYS_SMCR_EL1_CRm                                2
#define SYS_SMCR_EL1_Op2                                6

/* For SMCR_EL1 fields see SMCR_ELx */

#define GCSCR_ELx_STREn                                 GENMASK(9, 9)
#define GCSCR_ELx_STREn_MASK                            GENMASK(9, 9)
#define GCSCR_ELx_STREn_SHIFT                           9
#define GCSCR_ELx_STREn_WIDTH                           1

#define GCSCR_ELx_PUSHMEn                               GENMASK(8, 8)
#define GCSCR_ELx_PUSHMEn_MASK                          GENMASK(8, 8)
#define GCSCR_ELx_PUSHMEn_SHIFT                         8
#define GCSCR_ELx_PUSHMEn_WIDTH                         1

#define GCSCR_ELx_EXLOCKEN                              GENMASK(6, 6)
#define GCSCR_ELx_EXLOCKEN_MASK                         GENMASK(6, 6)
#define GCSCR_ELx_EXLOCKEN_SHIFT                        6
#define GCSCR_ELx_EXLOCKEN_WIDTH                        1

#define GCSCR_ELx_RVCHKEN                               GENMASK(5, 5)
#define GCSCR_ELx_RVCHKEN_MASK                          GENMASK(5, 5)
#define GCSCR_ELx_RVCHKEN_SHIFT                         5
#define GCSCR_ELx_RVCHKEN_WIDTH                         1

#define GCSCR_ELx_PCRSEL                                GENMASK(0, 0)
#define GCSCR_ELx_PCRSEL_MASK                           GENMASK(0, 0)
#define GCSCR_ELx_PCRSEL_SHIFT                          0
#define GCSCR_ELx_PCRSEL_WIDTH                          1

#define GCSCR_ELx_RES0                                  (UL(0) | GENMASK_ULL(63, 10) | GENMASK_ULL(7, 7) | GENMASK_ULL(4, 1))
#define GCSCR_ELx_RES1                                  (UL(0))
#define GCSCR_ELx_UNKN                                  (UL(0))

#define REG_GCSCR_EL1                                   S3_0_C2_C5_0
#define SYS_GCSCR_EL1                                   sys_reg(3, 0, 2, 5, 0)
#define SYS_GCSCR_EL1_Op0                               3
#define SYS_GCSCR_EL1_Op1                               0
#define SYS_GCSCR_EL1_CRn                               2
#define SYS_GCSCR_EL1_CRm                               5
#define SYS_GCSCR_EL1_Op2                               0

/* For GCSCR_EL1 fields see GCSCR_ELx */

#define GCSPR_ELx_PTR                                   GENMASK(63, 3)
#define GCSPR_ELx_PTR_MASK                              GENMASK(63, 3)
#define GCSPR_ELx_PTR_SHIFT                             3
#define GCSPR_ELx_PTR_WIDTH                             61

#define GCSPR_ELx_RES0                                  (UL(0) | GENMASK_ULL(2, 0))
#define GCSPR_ELx_RES1                                  (UL(0))
#define GCSPR_ELx_UNKN                                  (UL(0))

#define REG_GCSPR_EL1                                   S3_0_C2_C5_1
#define SYS_GCSPR_EL1                                   sys_reg(3, 0, 2, 5, 1)
#define SYS_GCSPR_EL1_Op0                               3
#define SYS_GCSPR_EL1_Op1                               0
#define SYS_GCSPR_EL1_CRn                               2
#define SYS_GCSPR_EL1_CRm                               5
#define SYS_GCSPR_EL1_Op2                               1

/* For GCSPR_EL1 fields see GCSPR_ELx */

#define REG_GCSCRE0_EL1                                 S3_0_C2_C5_2
#define SYS_GCSCRE0_EL1                                 sys_reg(3, 0, 2, 5, 2)
#define SYS_GCSCRE0_EL1_Op0                             3
#define SYS_GCSCRE0_EL1_Op1                             0
#define SYS_GCSCRE0_EL1_CRn                             2
#define SYS_GCSCRE0_EL1_CRm                             5
#define SYS_GCSCRE0_EL1_Op2                             2

#define GCSCRE0_EL1_nTR                                 GENMASK(10, 10)
#define GCSCRE0_EL1_nTR_MASK                            GENMASK(10, 10)
#define GCSCRE0_EL1_nTR_SHIFT                           10
#define GCSCRE0_EL1_nTR_WIDTH                           1

#define GCSCRE0_EL1_STREn                               GENMASK(9, 9)
#define GCSCRE0_EL1_STREn_MASK                          GENMASK(9, 9)
#define GCSCRE0_EL1_STREn_SHIFT                         9
#define GCSCRE0_EL1_STREn_WIDTH                         1

#define GCSCRE0_EL1_PUSHMEn                             GENMASK(8, 8)
#define GCSCRE0_EL1_PUSHMEn_MASK                        GENMASK(8, 8)
#define GCSCRE0_EL1_PUSHMEn_SHIFT                       8
#define GCSCRE0_EL1_PUSHMEn_WIDTH                       1

#define GCSCRE0_EL1_RVCHKEN                             GENMASK(5, 5)
#define GCSCRE0_EL1_RVCHKEN_MASK                        GENMASK(5, 5)
#define GCSCRE0_EL1_RVCHKEN_SHIFT                       5
#define GCSCRE0_EL1_RVCHKEN_WIDTH                       1

#define GCSCRE0_EL1_PCRSEL                              GENMASK(0, 0)
#define GCSCRE0_EL1_PCRSEL_MASK                         GENMASK(0, 0)
#define GCSCRE0_EL1_PCRSEL_SHIFT                        0
#define GCSCRE0_EL1_PCRSEL_WIDTH                        1

#define GCSCRE0_EL1_RES0                                (UL(0) | GENMASK_ULL(63, 11) | GENMASK_ULL(7, 6) | GENMASK_ULL(4, 1))
#define GCSCRE0_EL1_RES1                                (UL(0))
#define GCSCRE0_EL1_UNKN                                (UL(0))

#define REG_ALLINT                                      S3_0_C4_C3_0
#define SYS_ALLINT                                      sys_reg(3, 0, 4, 3, 0)
#define SYS_ALLINT_Op0                                  3
#define SYS_ALLINT_Op1                                  0
#define SYS_ALLINT_CRn                                  4
#define SYS_ALLINT_CRm                                  3
#define SYS_ALLINT_Op2                                  0

#define ALLINT_ALLINT                                   GENMASK(13, 13)
#define ALLINT_ALLINT_MASK                              GENMASK(13, 13)
#define ALLINT_ALLINT_SHIFT                             13
#define ALLINT_ALLINT_WIDTH                             1

#define ALLINT_RES0                                     (UL(0) | GENMASK_ULL(63, 14) | GENMASK_ULL(12, 0))
#define ALLINT_RES1                                     (UL(0))
#define ALLINT_UNKN                                     (UL(0))

#define REG_FAR_EL1                                     S3_0_C6_C0_0
#define SYS_FAR_EL1                                     sys_reg(3, 0, 6, 0, 0)
#define SYS_FAR_EL1_Op0                                 3
#define SYS_FAR_EL1_Op1                                 0
#define SYS_FAR_EL1_CRn                                 6
#define SYS_FAR_EL1_CRm                                 0
#define SYS_FAR_EL1_Op2                                 0

#define FAR_EL1_ADDR                                    GENMASK(63, 0)
#define FAR_EL1_ADDR_MASK                               GENMASK(63, 0)
#define FAR_EL1_ADDR_SHIFT                              0
#define FAR_EL1_ADDR_WIDTH                              64

#define FAR_EL1_RES0                                    (UL(0))
#define FAR_EL1_RES1                                    (UL(0))
#define FAR_EL1_UNKN                                    (UL(0))

#define REG_PMICNTR_EL0                                 S3_3_C9_C4_0
#define SYS_PMICNTR_EL0                                 sys_reg(3, 3, 9, 4, 0)
#define SYS_PMICNTR_EL0_Op0                             3
#define SYS_PMICNTR_EL0_Op1                             3
#define SYS_PMICNTR_EL0_CRn                             9
#define SYS_PMICNTR_EL0_CRm                             4
#define SYS_PMICNTR_EL0_Op2                             0

#define PMICNTR_EL0_ICNT                                GENMASK(63, 0)
#define PMICNTR_EL0_ICNT_MASK                           GENMASK(63, 0)
#define PMICNTR_EL0_ICNT_SHIFT                          0
#define PMICNTR_EL0_ICNT_WIDTH                          64

#define PMICNTR_EL0_RES0                                (UL(0))
#define PMICNTR_EL0_RES1                                (UL(0))
#define PMICNTR_EL0_UNKN                                (UL(0))

#define REG_PMICFILTR_EL0                               S3_3_C9_C6_0
#define SYS_PMICFILTR_EL0                               sys_reg(3, 3, 9, 6, 0)
#define SYS_PMICFILTR_EL0_Op0                           3
#define SYS_PMICFILTR_EL0_Op1                           3
#define SYS_PMICFILTR_EL0_CRn                           9
#define SYS_PMICFILTR_EL0_CRm                           6
#define SYS_PMICFILTR_EL0_Op2                           0

#define PMICFILTR_EL0_SYNC                              GENMASK(58, 58)
#define PMICFILTR_EL0_SYNC_MASK                         GENMASK(58, 58)
#define PMICFILTR_EL0_SYNC_SHIFT                        58
#define PMICFILTR_EL0_SYNC_WIDTH                        1

#define PMICFILTR_EL0_VS                                GENMASK(57, 56)
#define PMICFILTR_EL0_VS_MASK                           GENMASK(57, 56)
#define PMICFILTR_EL0_VS_SHIFT                          56
#define PMICFILTR_EL0_VS_WIDTH                          2

#define PMICFILTR_EL0_P                                 GENMASK(31, 31)
#define PMICFILTR_EL0_P_MASK                            GENMASK(31, 31)
#define PMICFILTR_EL0_P_SHIFT                           31
#define PMICFILTR_EL0_P_WIDTH                           1

#define PMICFILTR_EL0_U                                 GENMASK(30, 30)
#define PMICFILTR_EL0_U_MASK                            GENMASK(30, 30)
#define PMICFILTR_EL0_U_SHIFT                           30
#define PMICFILTR_EL0_U_WIDTH                           1

#define PMICFILTR_EL0_NSK                               GENMASK(29, 29)
#define PMICFILTR_EL0_NSK_MASK                          GENMASK(29, 29)
#define PMICFILTR_EL0_NSK_SHIFT                         29
#define PMICFILTR_EL0_NSK_WIDTH                         1

#define PMICFILTR_EL0_NSU                               GENMASK(28, 28)
#define PMICFILTR_EL0_NSU_MASK                          GENMASK(28, 28)
#define PMICFILTR_EL0_NSU_SHIFT                         28
#define PMICFILTR_EL0_NSU_WIDTH                         1

#define PMICFILTR_EL0_NSH                               GENMASK(27, 27)
#define PMICFILTR_EL0_NSH_MASK                          GENMASK(27, 27)
#define PMICFILTR_EL0_NSH_SHIFT                         27
#define PMICFILTR_EL0_NSH_WIDTH                         1

#define PMICFILTR_EL0_M                                 GENMASK(26, 26)
#define PMICFILTR_EL0_M_MASK                            GENMASK(26, 26)
#define PMICFILTR_EL0_M_SHIFT                           26
#define PMICFILTR_EL0_M_WIDTH                           1

#define PMICFILTR_EL0_SH                                GENMASK(24, 24)
#define PMICFILTR_EL0_SH_MASK                           GENMASK(24, 24)
#define PMICFILTR_EL0_SH_SHIFT                          24
#define PMICFILTR_EL0_SH_WIDTH                          1

#define PMICFILTR_EL0_T                                 GENMASK(23, 23)
#define PMICFILTR_EL0_T_MASK                            GENMASK(23, 23)
#define PMICFILTR_EL0_T_SHIFT                           23
#define PMICFILTR_EL0_T_WIDTH                           1

#define PMICFILTR_EL0_RLK                               GENMASK(22, 22)
#define PMICFILTR_EL0_RLK_MASK                          GENMASK(22, 22)
#define PMICFILTR_EL0_RLK_SHIFT                         22
#define PMICFILTR_EL0_RLK_WIDTH                         1

#define PMICFILTR_EL0_RLU                               GENMASK(21, 21)
#define PMICFILTR_EL0_RLU_MASK                          GENMASK(21, 21)
#define PMICFILTR_EL0_RLU_SHIFT                         21
#define PMICFILTR_EL0_RLU_WIDTH                         1

#define PMICFILTR_EL0_RLH                               GENMASK(20, 20)
#define PMICFILTR_EL0_RLH_MASK                          GENMASK(20, 20)
#define PMICFILTR_EL0_RLH_SHIFT                         20
#define PMICFILTR_EL0_RLH_WIDTH                         1

#define PMICFILTR_EL0_evtCount                          GENMASK(15, 0)
#define PMICFILTR_EL0_evtCount_MASK                     GENMASK(15, 0)
#define PMICFILTR_EL0_evtCount_SHIFT                    0
#define PMICFILTR_EL0_evtCount_WIDTH                    16

#define PMICFILTR_EL0_RES0                              (UL(0) | GENMASK_ULL(63, 59) | GENMASK_ULL(55, 32) | GENMASK_ULL(25, 25) | GENMASK_ULL(19, 16))
#define PMICFILTR_EL0_RES1                              (UL(0))
#define PMICFILTR_EL0_UNKN                              (UL(0))

#define REG_PMSCR_EL1                                   S3_0_C9_C9_0
#define SYS_PMSCR_EL1                                   sys_reg(3, 0, 9, 9, 0)
#define SYS_PMSCR_EL1_Op0                               3
#define SYS_PMSCR_EL1_Op1                               0
#define SYS_PMSCR_EL1_CRn                               9
#define SYS_PMSCR_EL1_CRm                               9
#define SYS_PMSCR_EL1_Op2                               0

#define PMSCR_EL1_PCT                                   GENMASK(7, 6)
#define PMSCR_EL1_PCT_MASK                              GENMASK(7, 6)
#define PMSCR_EL1_PCT_SHIFT                             6
#define PMSCR_EL1_PCT_WIDTH                             2

#define PMSCR_EL1_TS                                    GENMASK(5, 5)
#define PMSCR_EL1_TS_MASK                               GENMASK(5, 5)
#define PMSCR_EL1_TS_SHIFT                              5
#define PMSCR_EL1_TS_WIDTH                              1

#define PMSCR_EL1_PA                                    GENMASK(4, 4)
#define PMSCR_EL1_PA_MASK                               GENMASK(4, 4)
#define PMSCR_EL1_PA_SHIFT                              4
#define PMSCR_EL1_PA_WIDTH                              1

#define PMSCR_EL1_CX                                    GENMASK(3, 3)
#define PMSCR_EL1_CX_MASK                               GENMASK(3, 3)
#define PMSCR_EL1_CX_SHIFT                              3
#define PMSCR_EL1_CX_WIDTH                              1

#define PMSCR_EL1_E1SPE                                 GENMASK(1, 1)
#define PMSCR_EL1_E1SPE_MASK                            GENMASK(1, 1)
#define PMSCR_EL1_E1SPE_SHIFT                           1
#define PMSCR_EL1_E1SPE_WIDTH                           1

#define PMSCR_EL1_E0SPE                                 GENMASK(0, 0)
#define PMSCR_EL1_E0SPE_MASK                            GENMASK(0, 0)
#define PMSCR_EL1_E0SPE_SHIFT                           0
#define PMSCR_EL1_E0SPE_WIDTH                           1

#define PMSCR_EL1_RES0                                  (UL(0) | GENMASK_ULL(63, 8) | GENMASK_ULL(2, 2))
#define PMSCR_EL1_RES1                                  (UL(0))
#define PMSCR_EL1_UNKN                                  (UL(0))

#define REG_PMSNEVFR_EL1                                S3_0_C9_C9_1
#define SYS_PMSNEVFR_EL1                                sys_reg(3, 0, 9, 9, 1)
#define SYS_PMSNEVFR_EL1_Op0                            3
#define SYS_PMSNEVFR_EL1_Op1                            0
#define SYS_PMSNEVFR_EL1_CRn                            9
#define SYS_PMSNEVFR_EL1_CRm                            9
#define SYS_PMSNEVFR_EL1_Op2                            1

#define PMSNEVFR_EL1_E                                  GENMASK(63, 0)
#define PMSNEVFR_EL1_E_MASK                             GENMASK(63, 0)
#define PMSNEVFR_EL1_E_SHIFT                            0
#define PMSNEVFR_EL1_E_WIDTH                            64

#define PMSNEVFR_EL1_RES0                               (UL(0))
#define PMSNEVFR_EL1_RES1                               (UL(0))
#define PMSNEVFR_EL1_UNKN                               (UL(0))

#define REG_PMSICR_EL1                                  S3_0_C9_C9_2
#define SYS_PMSICR_EL1                                  sys_reg(3, 0, 9, 9, 2)
#define SYS_PMSICR_EL1_Op0                              3
#define SYS_PMSICR_EL1_Op1                              0
#define SYS_PMSICR_EL1_CRn                              9
#define SYS_PMSICR_EL1_CRm                              9
#define SYS_PMSICR_EL1_Op2                              2

#define PMSICR_EL1_ECOUNT                               GENMASK(63, 56)
#define PMSICR_EL1_ECOUNT_MASK                          GENMASK(63, 56)
#define PMSICR_EL1_ECOUNT_SHIFT                         56
#define PMSICR_EL1_ECOUNT_WIDTH                         8

#define PMSICR_EL1_COUNT                                GENMASK(31, 0)
#define PMSICR_EL1_COUNT_MASK                           GENMASK(31, 0)
#define PMSICR_EL1_COUNT_SHIFT                          0
#define PMSICR_EL1_COUNT_WIDTH                          32

#define PMSICR_EL1_RES0                                 (UL(0) | GENMASK_ULL(55, 32))
#define PMSICR_EL1_RES1                                 (UL(0))
#define PMSICR_EL1_UNKN                                 (UL(0))

#define REG_PMSIRR_EL1                                  S3_0_C9_C9_3
#define SYS_PMSIRR_EL1                                  sys_reg(3, 0, 9, 9, 3)
#define SYS_PMSIRR_EL1_Op0                              3
#define SYS_PMSIRR_EL1_Op1                              0
#define SYS_PMSIRR_EL1_CRn                              9
#define SYS_PMSIRR_EL1_CRm                              9
#define SYS_PMSIRR_EL1_Op2                              3

#define PMSIRR_EL1_INTERVAL                             GENMASK(31, 8)
#define PMSIRR_EL1_INTERVAL_MASK                        GENMASK(31, 8)
#define PMSIRR_EL1_INTERVAL_SHIFT                       8
#define PMSIRR_EL1_INTERVAL_WIDTH                       24

#define PMSIRR_EL1_RND                                  GENMASK(0, 0)
#define PMSIRR_EL1_RND_MASK                             GENMASK(0, 0)
#define PMSIRR_EL1_RND_SHIFT                            0
#define PMSIRR_EL1_RND_WIDTH                            1

#define PMSIRR_EL1_RES0                                 (UL(0) | GENMASK_ULL(63, 32) | GENMASK_ULL(7, 1))
#define PMSIRR_EL1_RES1                                 (UL(0))
#define PMSIRR_EL1_UNKN                                 (UL(0))

#define REG_PMSFCR_EL1                                  S3_0_C9_C9_4
#define SYS_PMSFCR_EL1                                  sys_reg(3, 0, 9, 9, 4)
#define SYS_PMSFCR_EL1_Op0                              3
#define SYS_PMSFCR_EL1_Op1                              0
#define SYS_PMSFCR_EL1_CRn                              9
#define SYS_PMSFCR_EL1_CRm                              9
#define SYS_PMSFCR_EL1_Op2                              4

#define PMSFCR_EL1_SIMDm                                GENMASK(52, 52)
#define PMSFCR_EL1_SIMDm_MASK                           GENMASK(52, 52)
#define PMSFCR_EL1_SIMDm_SHIFT                          52
#define PMSFCR_EL1_SIMDm_WIDTH                          1

#define PMSFCR_EL1_FPm                                  GENMASK(51, 51)
#define PMSFCR_EL1_FPm_MASK                             GENMASK(51, 51)
#define PMSFCR_EL1_FPm_SHIFT                            51
#define PMSFCR_EL1_FPm_WIDTH                            1

#define PMSFCR_EL1_STm                                  GENMASK(50, 50)
#define PMSFCR_EL1_STm_MASK                             GENMASK(50, 50)
#define PMSFCR_EL1_STm_SHIFT                            50
#define PMSFCR_EL1_STm_WIDTH                            1

#define PMSFCR_EL1_LDm                                  GENMASK(49, 49)
#define PMSFCR_EL1_LDm_MASK                             GENMASK(49, 49)
#define PMSFCR_EL1_LDm_SHIFT                            49
#define PMSFCR_EL1_LDm_WIDTH                            1

#define PMSFCR_EL1_Bm                                   GENMASK(48, 48)
#define PMSFCR_EL1_Bm_MASK                              GENMASK(48, 48)
#define PMSFCR_EL1_Bm_SHIFT                             48
#define PMSFCR_EL1_Bm_WIDTH                             1

#define PMSFCR_EL1_SIMD                                 GENMASK(20, 20)
#define PMSFCR_EL1_SIMD_MASK                            GENMASK(20, 20)
#define PMSFCR_EL1_SIMD_SHIFT                           20
#define PMSFCR_EL1_SIMD_WIDTH                           1

#define PMSFCR_EL1_FP                                   GENMASK(19, 19)
#define PMSFCR_EL1_FP_MASK                              GENMASK(19, 19)
#define PMSFCR_EL1_FP_SHIFT                             19
#define PMSFCR_EL1_FP_WIDTH                             1

#define PMSFCR_EL1_ST                                   GENMASK(18, 18)
#define PMSFCR_EL1_ST_MASK                              GENMASK(18, 18)
#define PMSFCR_EL1_ST_SHIFT                             18
#define PMSFCR_EL1_ST_WIDTH                             1

#define PMSFCR_EL1_LD                                   GENMASK(17, 17)
#define PMSFCR_EL1_LD_MASK                              GENMASK(17, 17)
#define PMSFCR_EL1_LD_SHIFT                             17
#define PMSFCR_EL1_LD_WIDTH                             1

#define PMSFCR_EL1_B                                    GENMASK(16, 16)
#define PMSFCR_EL1_B_MASK                               GENMASK(16, 16)
#define PMSFCR_EL1_B_SHIFT                              16
#define PMSFCR_EL1_B_WIDTH                              1

#define PMSFCR_EL1_FDS                                  GENMASK(4, 4)
#define PMSFCR_EL1_FDS_MASK                             GENMASK(4, 4)
#define PMSFCR_EL1_FDS_SHIFT                            4
#define PMSFCR_EL1_FDS_WIDTH                            1

#define PMSFCR_EL1_FnE                                  GENMASK(3, 3)
#define PMSFCR_EL1_FnE_MASK                             GENMASK(3, 3)
#define PMSFCR_EL1_FnE_SHIFT                            3
#define PMSFCR_EL1_FnE_WIDTH                            1

#define PMSFCR_EL1_FL                                   GENMASK(2, 2)
#define PMSFCR_EL1_FL_MASK                              GENMASK(2, 2)
#define PMSFCR_EL1_FL_SHIFT                             2
#define PMSFCR_EL1_FL_WIDTH                             1

#define PMSFCR_EL1_FT                                   GENMASK(1, 1)
#define PMSFCR_EL1_FT_MASK                              GENMASK(1, 1)
#define PMSFCR_EL1_FT_SHIFT                             1
#define PMSFCR_EL1_FT_WIDTH                             1

#define PMSFCR_EL1_FE                                   GENMASK(0, 0)
#define PMSFCR_EL1_FE_MASK                              GENMASK(0, 0)
#define PMSFCR_EL1_FE_SHIFT                             0
#define PMSFCR_EL1_FE_WIDTH                             1

#define PMSFCR_EL1_RES0                                 (UL(0) | GENMASK_ULL(63, 53) | GENMASK_ULL(47, 21) | GENMASK_ULL(15, 5))
#define PMSFCR_EL1_RES1                                 (UL(0))
#define PMSFCR_EL1_UNKN                                 (UL(0))

#define REG_PMSEVFR_EL1                                 S3_0_C9_C9_5
#define SYS_PMSEVFR_EL1                                 sys_reg(3, 0, 9, 9, 5)
#define SYS_PMSEVFR_EL1_Op0                             3
#define SYS_PMSEVFR_EL1_Op1                             0
#define SYS_PMSEVFR_EL1_CRn                             9
#define SYS_PMSEVFR_EL1_CRm                             9
#define SYS_PMSEVFR_EL1_Op2                             5

#define PMSEVFR_EL1_E                                   GENMASK(63, 0)
#define PMSEVFR_EL1_E_MASK                              GENMASK(63, 0)
#define PMSEVFR_EL1_E_SHIFT                             0
#define PMSEVFR_EL1_E_WIDTH                             64

#define PMSEVFR_EL1_RES0                                (UL(0))
#define PMSEVFR_EL1_RES1                                (UL(0))
#define PMSEVFR_EL1_UNKN                                (UL(0))

#define REG_PMSLATFR_EL1                                S3_0_C9_C9_6
#define SYS_PMSLATFR_EL1                                sys_reg(3, 0, 9, 9, 6)
#define SYS_PMSLATFR_EL1_Op0                            3
#define SYS_PMSLATFR_EL1_Op1                            0
#define SYS_PMSLATFR_EL1_CRn                            9
#define SYS_PMSLATFR_EL1_CRm                            9
#define SYS_PMSLATFR_EL1_Op2                            6

#define PMSLATFR_EL1_MINLAT                             GENMASK(15, 0)
#define PMSLATFR_EL1_MINLAT_MASK                        GENMASK(15, 0)
#define PMSLATFR_EL1_MINLAT_SHIFT                       0
#define PMSLATFR_EL1_MINLAT_WIDTH                       16

#define PMSLATFR_EL1_RES0                               (UL(0) | GENMASK_ULL(63, 16))
#define PMSLATFR_EL1_RES1                               (UL(0))
#define PMSLATFR_EL1_UNKN                               (UL(0))

#define REG_PMSIDR_EL1                                  S3_0_C9_C9_7
#define SYS_PMSIDR_EL1                                  sys_reg(3, 0, 9, 9, 7)
#define SYS_PMSIDR_EL1_Op0                              3
#define SYS_PMSIDR_EL1_Op1                              0
#define SYS_PMSIDR_EL1_CRn                              9
#define SYS_PMSIDR_EL1_CRm                              9
#define SYS_PMSIDR_EL1_Op2                              7

#define PMSIDR_EL1_SME                                  GENMASK(32, 32)
#define PMSIDR_EL1_SME_MASK                             GENMASK(32, 32)
#define PMSIDR_EL1_SME_SHIFT                            32
#define PMSIDR_EL1_SME_WIDTH                            1
#define PMSIDR_EL1_SME_SIGNED                           false
#define PMSIDR_EL1_SME_NI                               UL(0b0)
#define PMSIDR_EL1_SME_IMP                              UL(0b1)

#define PMSIDR_EL1_ALTCLK                               GENMASK(31, 28)
#define PMSIDR_EL1_ALTCLK_MASK                          GENMASK(31, 28)
#define PMSIDR_EL1_ALTCLK_SHIFT                         28
#define PMSIDR_EL1_ALTCLK_WIDTH                         4
#define PMSIDR_EL1_ALTCLK_SIGNED                        false
#define PMSIDR_EL1_ALTCLK_NI                            UL(0b0000)
#define PMSIDR_EL1_ALTCLK_IMP                           UL(0b0001)
#define PMSIDR_EL1_ALTCLK_IMPDEF                        UL(0b1111)

#define PMSIDR_EL1_FPF                                  GENMASK(27, 27)
#define PMSIDR_EL1_FPF_MASK                             GENMASK(27, 27)
#define PMSIDR_EL1_FPF_SHIFT                            27
#define PMSIDR_EL1_FPF_WIDTH                            1
#define PMSIDR_EL1_FPF_SIGNED                           false
#define PMSIDR_EL1_FPF_NI                               UL(0b0)
#define PMSIDR_EL1_FPF_IMP                              UL(0b1)

#define PMSIDR_EL1_EFT                                  GENMASK(26, 26)
#define PMSIDR_EL1_EFT_MASK                             GENMASK(26, 26)
#define PMSIDR_EL1_EFT_SHIFT                            26
#define PMSIDR_EL1_EFT_WIDTH                            1
#define PMSIDR_EL1_EFT_SIGNED                           false
#define PMSIDR_EL1_EFT_NI                               UL(0b0)
#define PMSIDR_EL1_EFT_IMP                              UL(0b1)

#define PMSIDR_EL1_CRR                                  GENMASK(25, 25)
#define PMSIDR_EL1_CRR_MASK                             GENMASK(25, 25)
#define PMSIDR_EL1_CRR_SHIFT                            25
#define PMSIDR_EL1_CRR_WIDTH                            1
#define PMSIDR_EL1_CRR_SIGNED                           false
#define PMSIDR_EL1_CRR_NI                               UL(0b0)
#define PMSIDR_EL1_CRR_IMP                              UL(0b1)

#define PMSIDR_EL1_PBT                                  GENMASK(24, 24)
#define PMSIDR_EL1_PBT_MASK                             GENMASK(24, 24)
#define PMSIDR_EL1_PBT_SHIFT                            24
#define PMSIDR_EL1_PBT_WIDTH                            1

#define PMSIDR_EL1_FORMAT                               GENMASK(23, 20)
#define PMSIDR_EL1_FORMAT_MASK                          GENMASK(23, 20)
#define PMSIDR_EL1_FORMAT_SHIFT                         20
#define PMSIDR_EL1_FORMAT_WIDTH                         4

#define PMSIDR_EL1_COUNTSIZE                            GENMASK(19, 16)
#define PMSIDR_EL1_COUNTSIZE_MASK                       GENMASK(19, 16)
#define PMSIDR_EL1_COUNTSIZE_SHIFT                      16
#define PMSIDR_EL1_COUNTSIZE_WIDTH                      4
#define PMSIDR_EL1_COUNTSIZE_12_BIT_SAT                 UL(0b0010)
#define PMSIDR_EL1_COUNTSIZE_16_BIT_SAT                 UL(0b0011)

#define PMSIDR_EL1_MAXSIZE                              GENMASK(15, 12)
#define PMSIDR_EL1_MAXSIZE_MASK                         GENMASK(15, 12)
#define PMSIDR_EL1_MAXSIZE_SHIFT                        12
#define PMSIDR_EL1_MAXSIZE_WIDTH                        4

#define PMSIDR_EL1_INTERVAL                             GENMASK(11, 8)
#define PMSIDR_EL1_INTERVAL_MASK                        GENMASK(11, 8)
#define PMSIDR_EL1_INTERVAL_SHIFT                       8
#define PMSIDR_EL1_INTERVAL_WIDTH                       4
#define PMSIDR_EL1_INTERVAL_256                         UL(0b0000)
#define PMSIDR_EL1_INTERVAL_512                         UL(0b0010)
#define PMSIDR_EL1_INTERVAL_768                         UL(0b0011)
#define PMSIDR_EL1_INTERVAL_1024                        UL(0b0100)
#define PMSIDR_EL1_INTERVAL_1536                        UL(0b0101)
#define PMSIDR_EL1_INTERVAL_2048                        UL(0b0110)
#define PMSIDR_EL1_INTERVAL_3072                        UL(0b0111)
#define PMSIDR_EL1_INTERVAL_4096                        UL(0b1000)

#define PMSIDR_EL1_FDS                                  GENMASK(7, 7)
#define PMSIDR_EL1_FDS_MASK                             GENMASK(7, 7)
#define PMSIDR_EL1_FDS_SHIFT                            7
#define PMSIDR_EL1_FDS_WIDTH                            1
#define PMSIDR_EL1_FDS_SIGNED                           false
#define PMSIDR_EL1_FDS_NI                               UL(0b0)
#define PMSIDR_EL1_FDS_IMP                              UL(0b1)

#define PMSIDR_EL1_FnE                                  GENMASK(6, 6)
#define PMSIDR_EL1_FnE_MASK                             GENMASK(6, 6)
#define PMSIDR_EL1_FnE_SHIFT                            6
#define PMSIDR_EL1_FnE_WIDTH                            1

#define PMSIDR_EL1_ERND                                 GENMASK(5, 5)
#define PMSIDR_EL1_ERND_MASK                            GENMASK(5, 5)
#define PMSIDR_EL1_ERND_SHIFT                           5
#define PMSIDR_EL1_ERND_WIDTH                           1

#define PMSIDR_EL1_LDS                                  GENMASK(4, 4)
#define PMSIDR_EL1_LDS_MASK                             GENMASK(4, 4)
#define PMSIDR_EL1_LDS_SHIFT                            4
#define PMSIDR_EL1_LDS_WIDTH                            1

#define PMSIDR_EL1_ARCHINST                             GENMASK(3, 3)
#define PMSIDR_EL1_ARCHINST_MASK                        GENMASK(3, 3)
#define PMSIDR_EL1_ARCHINST_SHIFT                       3
#define PMSIDR_EL1_ARCHINST_WIDTH                       1

#define PMSIDR_EL1_FL                                   GENMASK(2, 2)
#define PMSIDR_EL1_FL_MASK                              GENMASK(2, 2)
#define PMSIDR_EL1_FL_SHIFT                             2
#define PMSIDR_EL1_FL_WIDTH                             1

#define PMSIDR_EL1_FT                                   GENMASK(1, 1)
#define PMSIDR_EL1_FT_MASK                              GENMASK(1, 1)
#define PMSIDR_EL1_FT_SHIFT                             1
#define PMSIDR_EL1_FT_WIDTH                             1

#define PMSIDR_EL1_FE                                   GENMASK(0, 0)
#define PMSIDR_EL1_FE_MASK                              GENMASK(0, 0)
#define PMSIDR_EL1_FE_SHIFT                             0
#define PMSIDR_EL1_FE_WIDTH                             1

#define PMSIDR_EL1_RES0                                 (UL(0) | GENMASK_ULL(63, 33))
#define PMSIDR_EL1_RES1                                 (UL(0))
#define PMSIDR_EL1_UNKN                                 (UL(0))

#define REG_PMBLIMITR_EL1                               S3_0_C9_C10_0
#define SYS_PMBLIMITR_EL1                               sys_reg(3, 0, 9, 10, 0)
#define SYS_PMBLIMITR_EL1_Op0                           3
#define SYS_PMBLIMITR_EL1_Op1                           0
#define SYS_PMBLIMITR_EL1_CRn                           9
#define SYS_PMBLIMITR_EL1_CRm                           10
#define SYS_PMBLIMITR_EL1_Op2                           0

#define PMBLIMITR_EL1_LIMIT                             GENMASK(63, 12)
#define PMBLIMITR_EL1_LIMIT_MASK                        GENMASK(63, 12)
#define PMBLIMITR_EL1_LIMIT_SHIFT                       12
#define PMBLIMITR_EL1_LIMIT_WIDTH                       52

#define PMBLIMITR_EL1_PMFZ                              GENMASK(5, 5)
#define PMBLIMITR_EL1_PMFZ_MASK                         GENMASK(5, 5)
#define PMBLIMITR_EL1_PMFZ_SHIFT                        5
#define PMBLIMITR_EL1_PMFZ_WIDTH                        1

#define PMBLIMITR_EL1_FM                                GENMASK(2, 1)
#define PMBLIMITR_EL1_FM_MASK                           GENMASK(2, 1)
#define PMBLIMITR_EL1_FM_SHIFT                          1
#define PMBLIMITR_EL1_FM_WIDTH                          2
#define PMBLIMITR_EL1_FM_FILL                           UL(0b00)
#define PMBLIMITR_EL1_FM_DISCARD                        UL(0b10)

#define PMBLIMITR_EL1_E                                 GENMASK(0, 0)
#define PMBLIMITR_EL1_E_MASK                            GENMASK(0, 0)
#define PMBLIMITR_EL1_E_SHIFT                           0
#define PMBLIMITR_EL1_E_WIDTH                           1

#define PMBLIMITR_EL1_RES0                              (UL(0) | GENMASK_ULL(11, 6) | GENMASK_ULL(4, 3))
#define PMBLIMITR_EL1_RES1                              (UL(0))
#define PMBLIMITR_EL1_UNKN                              (UL(0))

#define REG_PMBPTR_EL1                                  S3_0_C9_C10_1
#define SYS_PMBPTR_EL1                                  sys_reg(3, 0, 9, 10, 1)
#define SYS_PMBPTR_EL1_Op0                              3
#define SYS_PMBPTR_EL1_Op1                              0
#define SYS_PMBPTR_EL1_CRn                              9
#define SYS_PMBPTR_EL1_CRm                              10
#define SYS_PMBPTR_EL1_Op2                              1

#define PMBPTR_EL1_PTR                                  GENMASK(63, 0)
#define PMBPTR_EL1_PTR_MASK                             GENMASK(63, 0)
#define PMBPTR_EL1_PTR_SHIFT                            0
#define PMBPTR_EL1_PTR_WIDTH                            64

#define PMBPTR_EL1_RES0                                 (UL(0))
#define PMBPTR_EL1_RES1                                 (UL(0))
#define PMBPTR_EL1_UNKN                                 (UL(0))

#define REG_PMBSR_EL1                                   S3_0_C9_C10_3
#define SYS_PMBSR_EL1                                   sys_reg(3, 0, 9, 10, 3)
#define SYS_PMBSR_EL1_Op0                               3
#define SYS_PMBSR_EL1_Op1                               0
#define SYS_PMBSR_EL1_CRn                               9
#define SYS_PMBSR_EL1_CRm                               10
#define SYS_PMBSR_EL1_Op2                               3

#define PMBSR_EL1_EC                                    GENMASK(31, 26)
#define PMBSR_EL1_EC_MASK                               GENMASK(31, 26)
#define PMBSR_EL1_EC_SHIFT                              26
#define PMBSR_EL1_EC_WIDTH                              6
#define PMBSR_EL1_EC_BUF                                UL(0b000000)
#define PMBSR_EL1_EC_FAULT_S1                           UL(0b100100)
#define PMBSR_EL1_EC_FAULT_S2                           UL(0b100101)
#define PMBSR_EL1_EC_FAULT_GPC                          UL(0b011110)
#define PMBSR_EL1_EC_IMP_DEF                            UL(0b011111)

#define PMBSR_EL1_DL                                    GENMASK(19, 19)
#define PMBSR_EL1_DL_MASK                               GENMASK(19, 19)
#define PMBSR_EL1_DL_SHIFT                              19
#define PMBSR_EL1_DL_WIDTH                              1

#define PMBSR_EL1_EA                                    GENMASK(18, 18)
#define PMBSR_EL1_EA_MASK                               GENMASK(18, 18)
#define PMBSR_EL1_EA_SHIFT                              18
#define PMBSR_EL1_EA_WIDTH                              1

#define PMBSR_EL1_S                                     GENMASK(17, 17)
#define PMBSR_EL1_S_MASK                                GENMASK(17, 17)
#define PMBSR_EL1_S_SHIFT                               17
#define PMBSR_EL1_S_WIDTH                               1

#define PMBSR_EL1_COLL                                  GENMASK(16, 16)
#define PMBSR_EL1_COLL_MASK                             GENMASK(16, 16)
#define PMBSR_EL1_COLL_SHIFT                            16
#define PMBSR_EL1_COLL_WIDTH                            1

#define PMBSR_EL1_MSS                                   GENMASK(15, 0)
#define PMBSR_EL1_MSS_MASK                              GENMASK(15, 0)
#define PMBSR_EL1_MSS_SHIFT                             0
#define PMBSR_EL1_MSS_WIDTH                             16

#define PMBSR_EL1_RES0                                  (UL(0) | GENMASK_ULL(63, 32) | GENMASK_ULL(25, 20))
#define PMBSR_EL1_RES1                                  (UL(0))
#define PMBSR_EL1_UNKN                                  (UL(0))

#define REG_PMSDSFR_EL1                                 S3_0_C9_C10_4
#define SYS_PMSDSFR_EL1                                 sys_reg(3, 0, 9, 10, 4)
#define SYS_PMSDSFR_EL1_Op0                             3
#define SYS_PMSDSFR_EL1_Op1                             0
#define SYS_PMSDSFR_EL1_CRn                             9
#define SYS_PMSDSFR_EL1_CRm                             10
#define SYS_PMSDSFR_EL1_Op2                             4

#define PMSDSFR_EL1_S                                   GENMASK(63, 0)
#define PMSDSFR_EL1_S_MASK                              GENMASK(63, 0)
#define PMSDSFR_EL1_S_SHIFT                             0
#define PMSDSFR_EL1_S_WIDTH                             64

#define PMSDSFR_EL1_RES0                                (UL(0))
#define PMSDSFR_EL1_RES1                                (UL(0))
#define PMSDSFR_EL1_UNKN                                (UL(0))

#define REG_PMBMAR_EL1                                  S3_0_C9_C10_5
#define SYS_PMBMAR_EL1                                  sys_reg(3, 0, 9, 10, 5)
#define SYS_PMBMAR_EL1_Op0                              3
#define SYS_PMBMAR_EL1_Op1                              0
#define SYS_PMBMAR_EL1_CRn                              9
#define SYS_PMBMAR_EL1_CRm                              10
#define SYS_PMBMAR_EL1_Op2                              5

#define PMBMAR_EL1_SH                                   GENMASK(9, 8)
#define PMBMAR_EL1_SH_MASK                              GENMASK(9, 8)
#define PMBMAR_EL1_SH_SHIFT                             8
#define PMBMAR_EL1_SH_WIDTH                             2

#define PMBMAR_EL1_Attr                                 GENMASK(7, 0)
#define PMBMAR_EL1_Attr_MASK                            GENMASK(7, 0)
#define PMBMAR_EL1_Attr_SHIFT                           0
#define PMBMAR_EL1_Attr_WIDTH                           8

#define PMBMAR_EL1_RES0                                 (UL(0) | GENMASK_ULL(63, 10))
#define PMBMAR_EL1_RES1                                 (UL(0))
#define PMBMAR_EL1_UNKN                                 (UL(0))

#define REG_PMBIDR_EL1                                  S3_0_C9_C10_7
#define SYS_PMBIDR_EL1                                  sys_reg(3, 0, 9, 10, 7)
#define SYS_PMBIDR_EL1_Op0                              3
#define SYS_PMBIDR_EL1_Op1                              0
#define SYS_PMBIDR_EL1_CRn                              9
#define SYS_PMBIDR_EL1_CRm                              10
#define SYS_PMBIDR_EL1_Op2                              7

#define PMBIDR_EL1_EA                                   GENMASK(11, 8)
#define PMBIDR_EL1_EA_MASK                              GENMASK(11, 8)
#define PMBIDR_EL1_EA_SHIFT                             8
#define PMBIDR_EL1_EA_WIDTH                             4
#define PMBIDR_EL1_EA_NotDescribed                      UL(0b0000)
#define PMBIDR_EL1_EA_Ignored                           UL(0b0001)
#define PMBIDR_EL1_EA_SError                            UL(0b0010)

#define PMBIDR_EL1_F                                    GENMASK(5, 5)
#define PMBIDR_EL1_F_MASK                               GENMASK(5, 5)
#define PMBIDR_EL1_F_SHIFT                              5
#define PMBIDR_EL1_F_WIDTH                              1

#define PMBIDR_EL1_P                                    GENMASK(4, 4)
#define PMBIDR_EL1_P_MASK                               GENMASK(4, 4)
#define PMBIDR_EL1_P_SHIFT                              4
#define PMBIDR_EL1_P_WIDTH                              1

#define PMBIDR_EL1_ALIGN                                GENMASK(3, 0)
#define PMBIDR_EL1_ALIGN_MASK                           GENMASK(3, 0)
#define PMBIDR_EL1_ALIGN_SHIFT                          0
#define PMBIDR_EL1_ALIGN_WIDTH                          4

#define PMBIDR_EL1_RES0                                 (UL(0) | GENMASK_ULL(63, 12) | GENMASK_ULL(7, 6))
#define PMBIDR_EL1_RES1                                 (UL(0))
#define PMBIDR_EL1_UNKN                                 (UL(0))

#define REG_TRBMPAM_EL1                                 S3_0_C9_C11_5
#define SYS_TRBMPAM_EL1                                 sys_reg(3, 0, 9, 11, 5)
#define SYS_TRBMPAM_EL1_Op0                             3
#define SYS_TRBMPAM_EL1_Op1                             0
#define SYS_TRBMPAM_EL1_CRn                             9
#define SYS_TRBMPAM_EL1_CRm                             11
#define SYS_TRBMPAM_EL1_Op2                             5

#define TRBMPAM_EL1_EN                                  GENMASK(26, 26)
#define TRBMPAM_EL1_EN_MASK                             GENMASK(26, 26)
#define TRBMPAM_EL1_EN_SHIFT                            26
#define TRBMPAM_EL1_EN_WIDTH                            1

#define TRBMPAM_EL1_MPAM_SP                             GENMASK(25, 24)
#define TRBMPAM_EL1_MPAM_SP_MASK                        GENMASK(25, 24)
#define TRBMPAM_EL1_MPAM_SP_SHIFT                       24
#define TRBMPAM_EL1_MPAM_SP_WIDTH                       2

#define TRBMPAM_EL1_PMG                                 GENMASK(23, 16)
#define TRBMPAM_EL1_PMG_MASK                            GENMASK(23, 16)
#define TRBMPAM_EL1_PMG_SHIFT                           16
#define TRBMPAM_EL1_PMG_WIDTH                           8

#define TRBMPAM_EL1_PARTID                              GENMASK(15, 0)
#define TRBMPAM_EL1_PARTID_MASK                         GENMASK(15, 0)
#define TRBMPAM_EL1_PARTID_SHIFT                        0
#define TRBMPAM_EL1_PARTID_WIDTH                        16

#define TRBMPAM_EL1_RES0                                (UL(0) | GENMASK_ULL(63, 27))
#define TRBMPAM_EL1_RES1                                (UL(0))
#define TRBMPAM_EL1_UNKN                                (UL(0))

#define REG_PMSSCR_EL1                                  S3_0_C9_C13_3
#define SYS_PMSSCR_EL1                                  sys_reg(3, 0, 9, 13, 3)
#define SYS_PMSSCR_EL1_Op0                              3
#define SYS_PMSSCR_EL1_Op1                              0
#define SYS_PMSSCR_EL1_CRn                              9
#define SYS_PMSSCR_EL1_CRm                              13
#define SYS_PMSSCR_EL1_Op2                              3

#define PMSSCR_EL1_NC                                   GENMASK(32, 32)
#define PMSSCR_EL1_NC_MASK                              GENMASK(32, 32)
#define PMSSCR_EL1_NC_SHIFT                             32
#define PMSSCR_EL1_NC_WIDTH                             1

#define PMSSCR_EL1_SS                                   GENMASK(0, 0)
#define PMSSCR_EL1_SS_MASK                              GENMASK(0, 0)
#define PMSSCR_EL1_SS_SHIFT                             0
#define PMSSCR_EL1_SS_WIDTH                             1

#define PMSSCR_EL1_RES0                                 (UL(0) | GENMASK_ULL(63, 33) | GENMASK_ULL(31, 1))
#define PMSSCR_EL1_RES1                                 (UL(0))
#define PMSSCR_EL1_UNKN                                 (UL(0))

#define REG_PMUACR_EL1                                  S3_0_C9_C14_4
#define SYS_PMUACR_EL1                                  sys_reg(3, 0, 9, 14, 4)
#define SYS_PMUACR_EL1_Op0                              3
#define SYS_PMUACR_EL1_Op1                              0
#define SYS_PMUACR_EL1_CRn                              9
#define SYS_PMUACR_EL1_CRm                              14
#define SYS_PMUACR_EL1_Op2                              4

#define PMUACR_EL1_F0                                   GENMASK(32, 32)
#define PMUACR_EL1_F0_MASK                              GENMASK(32, 32)
#define PMUACR_EL1_F0_SHIFT                             32
#define PMUACR_EL1_F0_WIDTH                             1

#define PMUACR_EL1_C                                    GENMASK(31, 31)
#define PMUACR_EL1_C_MASK                               GENMASK(31, 31)
#define PMUACR_EL1_C_SHIFT                              31
#define PMUACR_EL1_C_WIDTH                              1

#define PMUACR_EL1_P                                    GENMASK(30, 0)
#define PMUACR_EL1_P_MASK                               GENMASK(30, 0)
#define PMUACR_EL1_P_SHIFT                              0
#define PMUACR_EL1_P_WIDTH                              31

#define PMUACR_EL1_RES0                                 (UL(0) | GENMASK_ULL(63, 33))
#define PMUACR_EL1_RES1                                 (UL(0))
#define PMUACR_EL1_UNKN                                 (UL(0))

#define REG_PMECR_EL1                                   S3_0_C9_C14_5
#define SYS_PMECR_EL1                                   sys_reg(3, 0, 9, 14, 5)
#define SYS_PMECR_EL1_Op0                               3
#define SYS_PMECR_EL1_Op1                               0
#define SYS_PMECR_EL1_CRn                               9
#define SYS_PMECR_EL1_CRm                               14
#define SYS_PMECR_EL1_Op2                               5

#define PMECR_EL1_SSE                                   GENMASK(4, 3)
#define PMECR_EL1_SSE_MASK                              GENMASK(4, 3)
#define PMECR_EL1_SSE_SHIFT                             3
#define PMECR_EL1_SSE_WIDTH                             2

#define PMECR_EL1_KPME                                  GENMASK(2, 2)
#define PMECR_EL1_KPME_MASK                             GENMASK(2, 2)
#define PMECR_EL1_KPME_SHIFT                            2
#define PMECR_EL1_KPME_WIDTH                            1

#define PMECR_EL1_PMEE                                  GENMASK(1, 0)
#define PMECR_EL1_PMEE_MASK                             GENMASK(1, 0)
#define PMECR_EL1_PMEE_SHIFT                            0
#define PMECR_EL1_PMEE_WIDTH                            2

#define PMECR_EL1_RES0                                  (UL(0) | GENMASK_ULL(63, 5))
#define PMECR_EL1_RES1                                  (UL(0))
#define PMECR_EL1_UNKN                                  (UL(0))

#define REG_PMIAR_EL1                                   S3_0_C9_C14_7
#define SYS_PMIAR_EL1                                   sys_reg(3, 0, 9, 14, 7)
#define SYS_PMIAR_EL1_Op0                               3
#define SYS_PMIAR_EL1_Op1                               0
#define SYS_PMIAR_EL1_CRn                               9
#define SYS_PMIAR_EL1_CRm                               14
#define SYS_PMIAR_EL1_Op2                               7

#define PMIAR_EL1_ADDRESS                               GENMASK(63, 0)
#define PMIAR_EL1_ADDRESS_MASK                          GENMASK(63, 0)
#define PMIAR_EL1_ADDRESS_SHIFT                         0
#define PMIAR_EL1_ADDRESS_WIDTH                         64

#define PMIAR_EL1_RES0                                  (UL(0))
#define PMIAR_EL1_RES1                                  (UL(0))
#define PMIAR_EL1_UNKN                                  (UL(0))

#define ICC_PPI_HMRx_EL1_HM63                           GENMASK(63, 63)
#define ICC_PPI_HMRx_EL1_HM63_MASK                      GENMASK(63, 63)
#define ICC_PPI_HMRx_EL1_HM63_SHIFT                     63
#define ICC_PPI_HMRx_EL1_HM63_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM62                           GENMASK(62, 62)
#define ICC_PPI_HMRx_EL1_HM62_MASK                      GENMASK(62, 62)
#define ICC_PPI_HMRx_EL1_HM62_SHIFT                     62
#define ICC_PPI_HMRx_EL1_HM62_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM61                           GENMASK(61, 61)
#define ICC_PPI_HMRx_EL1_HM61_MASK                      GENMASK(61, 61)
#define ICC_PPI_HMRx_EL1_HM61_SHIFT                     61
#define ICC_PPI_HMRx_EL1_HM61_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM60                           GENMASK(60, 60)
#define ICC_PPI_HMRx_EL1_HM60_MASK                      GENMASK(60, 60)
#define ICC_PPI_HMRx_EL1_HM60_SHIFT                     60
#define ICC_PPI_HMRx_EL1_HM60_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM59                           GENMASK(59, 59)
#define ICC_PPI_HMRx_EL1_HM59_MASK                      GENMASK(59, 59)
#define ICC_PPI_HMRx_EL1_HM59_SHIFT                     59
#define ICC_PPI_HMRx_EL1_HM59_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM58                           GENMASK(58, 58)
#define ICC_PPI_HMRx_EL1_HM58_MASK                      GENMASK(58, 58)
#define ICC_PPI_HMRx_EL1_HM58_SHIFT                     58
#define ICC_PPI_HMRx_EL1_HM58_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM57                           GENMASK(57, 57)
#define ICC_PPI_HMRx_EL1_HM57_MASK                      GENMASK(57, 57)
#define ICC_PPI_HMRx_EL1_HM57_SHIFT                     57
#define ICC_PPI_HMRx_EL1_HM57_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM56                           GENMASK(56, 56)
#define ICC_PPI_HMRx_EL1_HM56_MASK                      GENMASK(56, 56)
#define ICC_PPI_HMRx_EL1_HM56_SHIFT                     56
#define ICC_PPI_HMRx_EL1_HM56_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM55                           GENMASK(55, 55)
#define ICC_PPI_HMRx_EL1_HM55_MASK                      GENMASK(55, 55)
#define ICC_PPI_HMRx_EL1_HM55_SHIFT                     55
#define ICC_PPI_HMRx_EL1_HM55_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM54                           GENMASK(54, 54)
#define ICC_PPI_HMRx_EL1_HM54_MASK                      GENMASK(54, 54)
#define ICC_PPI_HMRx_EL1_HM54_SHIFT                     54
#define ICC_PPI_HMRx_EL1_HM54_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM53                           GENMASK(53, 53)
#define ICC_PPI_HMRx_EL1_HM53_MASK                      GENMASK(53, 53)
#define ICC_PPI_HMRx_EL1_HM53_SHIFT                     53
#define ICC_PPI_HMRx_EL1_HM53_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM52                           GENMASK(52, 52)
#define ICC_PPI_HMRx_EL1_HM52_MASK                      GENMASK(52, 52)
#define ICC_PPI_HMRx_EL1_HM52_SHIFT                     52
#define ICC_PPI_HMRx_EL1_HM52_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM51                           GENMASK(51, 51)
#define ICC_PPI_HMRx_EL1_HM51_MASK                      GENMASK(51, 51)
#define ICC_PPI_HMRx_EL1_HM51_SHIFT                     51
#define ICC_PPI_HMRx_EL1_HM51_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM50                           GENMASK(50, 50)
#define ICC_PPI_HMRx_EL1_HM50_MASK                      GENMASK(50, 50)
#define ICC_PPI_HMRx_EL1_HM50_SHIFT                     50
#define ICC_PPI_HMRx_EL1_HM50_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM49                           GENMASK(49, 49)
#define ICC_PPI_HMRx_EL1_HM49_MASK                      GENMASK(49, 49)
#define ICC_PPI_HMRx_EL1_HM49_SHIFT                     49
#define ICC_PPI_HMRx_EL1_HM49_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM48                           GENMASK(48, 48)
#define ICC_PPI_HMRx_EL1_HM48_MASK                      GENMASK(48, 48)
#define ICC_PPI_HMRx_EL1_HM48_SHIFT                     48
#define ICC_PPI_HMRx_EL1_HM48_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM47                           GENMASK(47, 47)
#define ICC_PPI_HMRx_EL1_HM47_MASK                      GENMASK(47, 47)
#define ICC_PPI_HMRx_EL1_HM47_SHIFT                     47
#define ICC_PPI_HMRx_EL1_HM47_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM46                           GENMASK(46, 46)
#define ICC_PPI_HMRx_EL1_HM46_MASK                      GENMASK(46, 46)
#define ICC_PPI_HMRx_EL1_HM46_SHIFT                     46
#define ICC_PPI_HMRx_EL1_HM46_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM45                           GENMASK(45, 45)
#define ICC_PPI_HMRx_EL1_HM45_MASK                      GENMASK(45, 45)
#define ICC_PPI_HMRx_EL1_HM45_SHIFT                     45
#define ICC_PPI_HMRx_EL1_HM45_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM44                           GENMASK(44, 44)
#define ICC_PPI_HMRx_EL1_HM44_MASK                      GENMASK(44, 44)
#define ICC_PPI_HMRx_EL1_HM44_SHIFT                     44
#define ICC_PPI_HMRx_EL1_HM44_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM43                           GENMASK(43, 43)
#define ICC_PPI_HMRx_EL1_HM43_MASK                      GENMASK(43, 43)
#define ICC_PPI_HMRx_EL1_HM43_SHIFT                     43
#define ICC_PPI_HMRx_EL1_HM43_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM42                           GENMASK(42, 42)
#define ICC_PPI_HMRx_EL1_HM42_MASK                      GENMASK(42, 42)
#define ICC_PPI_HMRx_EL1_HM42_SHIFT                     42
#define ICC_PPI_HMRx_EL1_HM42_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM41                           GENMASK(41, 41)
#define ICC_PPI_HMRx_EL1_HM41_MASK                      GENMASK(41, 41)
#define ICC_PPI_HMRx_EL1_HM41_SHIFT                     41
#define ICC_PPI_HMRx_EL1_HM41_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM40                           GENMASK(40, 40)
#define ICC_PPI_HMRx_EL1_HM40_MASK                      GENMASK(40, 40)
#define ICC_PPI_HMRx_EL1_HM40_SHIFT                     40
#define ICC_PPI_HMRx_EL1_HM40_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM39                           GENMASK(39, 39)
#define ICC_PPI_HMRx_EL1_HM39_MASK                      GENMASK(39, 39)
#define ICC_PPI_HMRx_EL1_HM39_SHIFT                     39
#define ICC_PPI_HMRx_EL1_HM39_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM38                           GENMASK(38, 38)
#define ICC_PPI_HMRx_EL1_HM38_MASK                      GENMASK(38, 38)
#define ICC_PPI_HMRx_EL1_HM38_SHIFT                     38
#define ICC_PPI_HMRx_EL1_HM38_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM37                           GENMASK(37, 37)
#define ICC_PPI_HMRx_EL1_HM37_MASK                      GENMASK(37, 37)
#define ICC_PPI_HMRx_EL1_HM37_SHIFT                     37
#define ICC_PPI_HMRx_EL1_HM37_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM36                           GENMASK(36, 36)
#define ICC_PPI_HMRx_EL1_HM36_MASK                      GENMASK(36, 36)
#define ICC_PPI_HMRx_EL1_HM36_SHIFT                     36
#define ICC_PPI_HMRx_EL1_HM36_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM35                           GENMASK(35, 35)
#define ICC_PPI_HMRx_EL1_HM35_MASK                      GENMASK(35, 35)
#define ICC_PPI_HMRx_EL1_HM35_SHIFT                     35
#define ICC_PPI_HMRx_EL1_HM35_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM34                           GENMASK(34, 34)
#define ICC_PPI_HMRx_EL1_HM34_MASK                      GENMASK(34, 34)
#define ICC_PPI_HMRx_EL1_HM34_SHIFT                     34
#define ICC_PPI_HMRx_EL1_HM34_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM33                           GENMASK(33, 33)
#define ICC_PPI_HMRx_EL1_HM33_MASK                      GENMASK(33, 33)
#define ICC_PPI_HMRx_EL1_HM33_SHIFT                     33
#define ICC_PPI_HMRx_EL1_HM33_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM32                           GENMASK(32, 32)
#define ICC_PPI_HMRx_EL1_HM32_MASK                      GENMASK(32, 32)
#define ICC_PPI_HMRx_EL1_HM32_SHIFT                     32
#define ICC_PPI_HMRx_EL1_HM32_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM31                           GENMASK(31, 31)
#define ICC_PPI_HMRx_EL1_HM31_MASK                      GENMASK(31, 31)
#define ICC_PPI_HMRx_EL1_HM31_SHIFT                     31
#define ICC_PPI_HMRx_EL1_HM31_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM30                           GENMASK(30, 30)
#define ICC_PPI_HMRx_EL1_HM30_MASK                      GENMASK(30, 30)
#define ICC_PPI_HMRx_EL1_HM30_SHIFT                     30
#define ICC_PPI_HMRx_EL1_HM30_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM29                           GENMASK(29, 29)
#define ICC_PPI_HMRx_EL1_HM29_MASK                      GENMASK(29, 29)
#define ICC_PPI_HMRx_EL1_HM29_SHIFT                     29
#define ICC_PPI_HMRx_EL1_HM29_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM28                           GENMASK(28, 28)
#define ICC_PPI_HMRx_EL1_HM28_MASK                      GENMASK(28, 28)
#define ICC_PPI_HMRx_EL1_HM28_SHIFT                     28
#define ICC_PPI_HMRx_EL1_HM28_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM27                           GENMASK(27, 27)
#define ICC_PPI_HMRx_EL1_HM27_MASK                      GENMASK(27, 27)
#define ICC_PPI_HMRx_EL1_HM27_SHIFT                     27
#define ICC_PPI_HMRx_EL1_HM27_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM26                           GENMASK(26, 26)
#define ICC_PPI_HMRx_EL1_HM26_MASK                      GENMASK(26, 26)
#define ICC_PPI_HMRx_EL1_HM26_SHIFT                     26
#define ICC_PPI_HMRx_EL1_HM26_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM25                           GENMASK(25, 25)
#define ICC_PPI_HMRx_EL1_HM25_MASK                      GENMASK(25, 25)
#define ICC_PPI_HMRx_EL1_HM25_SHIFT                     25
#define ICC_PPI_HMRx_EL1_HM25_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM24                           GENMASK(24, 24)
#define ICC_PPI_HMRx_EL1_HM24_MASK                      GENMASK(24, 24)
#define ICC_PPI_HMRx_EL1_HM24_SHIFT                     24
#define ICC_PPI_HMRx_EL1_HM24_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM23                           GENMASK(23, 23)
#define ICC_PPI_HMRx_EL1_HM23_MASK                      GENMASK(23, 23)
#define ICC_PPI_HMRx_EL1_HM23_SHIFT                     23
#define ICC_PPI_HMRx_EL1_HM23_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM22                           GENMASK(22, 22)
#define ICC_PPI_HMRx_EL1_HM22_MASK                      GENMASK(22, 22)
#define ICC_PPI_HMRx_EL1_HM22_SHIFT                     22
#define ICC_PPI_HMRx_EL1_HM22_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM21                           GENMASK(21, 21)
#define ICC_PPI_HMRx_EL1_HM21_MASK                      GENMASK(21, 21)
#define ICC_PPI_HMRx_EL1_HM21_SHIFT                     21
#define ICC_PPI_HMRx_EL1_HM21_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM20                           GENMASK(20, 20)
#define ICC_PPI_HMRx_EL1_HM20_MASK                      GENMASK(20, 20)
#define ICC_PPI_HMRx_EL1_HM20_SHIFT                     20
#define ICC_PPI_HMRx_EL1_HM20_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM19                           GENMASK(19, 19)
#define ICC_PPI_HMRx_EL1_HM19_MASK                      GENMASK(19, 19)
#define ICC_PPI_HMRx_EL1_HM19_SHIFT                     19
#define ICC_PPI_HMRx_EL1_HM19_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM18                           GENMASK(18, 18)
#define ICC_PPI_HMRx_EL1_HM18_MASK                      GENMASK(18, 18)
#define ICC_PPI_HMRx_EL1_HM18_SHIFT                     18
#define ICC_PPI_HMRx_EL1_HM18_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM17                           GENMASK(17, 17)
#define ICC_PPI_HMRx_EL1_HM17_MASK                      GENMASK(17, 17)
#define ICC_PPI_HMRx_EL1_HM17_SHIFT                     17
#define ICC_PPI_HMRx_EL1_HM17_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM16                           GENMASK(16, 16)
#define ICC_PPI_HMRx_EL1_HM16_MASK                      GENMASK(16, 16)
#define ICC_PPI_HMRx_EL1_HM16_SHIFT                     16
#define ICC_PPI_HMRx_EL1_HM16_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM15                           GENMASK(15, 15)
#define ICC_PPI_HMRx_EL1_HM15_MASK                      GENMASK(15, 15)
#define ICC_PPI_HMRx_EL1_HM15_SHIFT                     15
#define ICC_PPI_HMRx_EL1_HM15_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM14                           GENMASK(14, 14)
#define ICC_PPI_HMRx_EL1_HM14_MASK                      GENMASK(14, 14)
#define ICC_PPI_HMRx_EL1_HM14_SHIFT                     14
#define ICC_PPI_HMRx_EL1_HM14_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM13                           GENMASK(13, 13)
#define ICC_PPI_HMRx_EL1_HM13_MASK                      GENMASK(13, 13)
#define ICC_PPI_HMRx_EL1_HM13_SHIFT                     13
#define ICC_PPI_HMRx_EL1_HM13_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM12                           GENMASK(12, 12)
#define ICC_PPI_HMRx_EL1_HM12_MASK                      GENMASK(12, 12)
#define ICC_PPI_HMRx_EL1_HM12_SHIFT                     12
#define ICC_PPI_HMRx_EL1_HM12_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM11                           GENMASK(11, 11)
#define ICC_PPI_HMRx_EL1_HM11_MASK                      GENMASK(11, 11)
#define ICC_PPI_HMRx_EL1_HM11_SHIFT                     11
#define ICC_PPI_HMRx_EL1_HM11_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM10                           GENMASK(10, 10)
#define ICC_PPI_HMRx_EL1_HM10_MASK                      GENMASK(10, 10)
#define ICC_PPI_HMRx_EL1_HM10_SHIFT                     10
#define ICC_PPI_HMRx_EL1_HM10_WIDTH                     1

#define ICC_PPI_HMRx_EL1_HM9                            GENMASK(9, 9)
#define ICC_PPI_HMRx_EL1_HM9_MASK                       GENMASK(9, 9)
#define ICC_PPI_HMRx_EL1_HM9_SHIFT                      9
#define ICC_PPI_HMRx_EL1_HM9_WIDTH                      1

#define ICC_PPI_HMRx_EL1_HM8                            GENMASK(8, 8)
#define ICC_PPI_HMRx_EL1_HM8_MASK                       GENMASK(8, 8)
#define ICC_PPI_HMRx_EL1_HM8_SHIFT                      8
#define ICC_PPI_HMRx_EL1_HM8_WIDTH                      1

#define ICC_PPI_HMRx_EL1_HM7                            GENMASK(7, 7)
#define ICC_PPI_HMRx_EL1_HM7_MASK                       GENMASK(7, 7)
#define ICC_PPI_HMRx_EL1_HM7_SHIFT                      7
#define ICC_PPI_HMRx_EL1_HM7_WIDTH                      1

#define ICC_PPI_HMRx_EL1_HM6                            GENMASK(6, 6)
#define ICC_PPI_HMRx_EL1_HM6_MASK                       GENMASK(6, 6)
#define ICC_PPI_HMRx_EL1_HM6_SHIFT                      6
#define ICC_PPI_HMRx_EL1_HM6_WIDTH                      1

#define ICC_PPI_HMRx_EL1_HM5                            GENMASK(5, 5)
#define ICC_PPI_HMRx_EL1_HM5_MASK                       GENMASK(5, 5)
#define ICC_PPI_HMRx_EL1_HM5_SHIFT                      5
#define ICC_PPI_HMRx_EL1_HM5_WIDTH                      1

#define ICC_PPI_HMRx_EL1_HM4                            GENMASK(4, 4)
#define ICC_PPI_HMRx_EL1_HM4_MASK                       GENMASK(4, 4)
#define ICC_PPI_HMRx_EL1_HM4_SHIFT                      4
#define ICC_PPI_HMRx_EL1_HM4_WIDTH                      1

#define ICC_PPI_HMRx_EL1_HM3                            GENMASK(3, 3)
#define ICC_PPI_HMRx_EL1_HM3_MASK                       GENMASK(3, 3)
#define ICC_PPI_HMRx_EL1_HM3_SHIFT                      3
#define ICC_PPI_HMRx_EL1_HM3_WIDTH                      1

#define ICC_PPI_HMRx_EL1_HM2                            GENMASK(2, 2)
#define ICC_PPI_HMRx_EL1_HM2_MASK                       GENMASK(2, 2)
#define ICC_PPI_HMRx_EL1_HM2_SHIFT                      2
#define ICC_PPI_HMRx_EL1_HM2_WIDTH                      1

#define ICC_PPI_HMRx_EL1_HM1                            GENMASK(1, 1)
#define ICC_PPI_HMRx_EL1_HM1_MASK                       GENMASK(1, 1)
#define ICC_PPI_HMRx_EL1_HM1_SHIFT                      1
#define ICC_PPI_HMRx_EL1_HM1_WIDTH                      1

#define ICC_PPI_HMRx_EL1_HM0                            GENMASK(0, 0)
#define ICC_PPI_HMRx_EL1_HM0_MASK                       GENMASK(0, 0)
#define ICC_PPI_HMRx_EL1_HM0_SHIFT                      0
#define ICC_PPI_HMRx_EL1_HM0_WIDTH                      1

#define ICC_PPI_HMRx_EL1_RES0                           (UL(0))
#define ICC_PPI_HMRx_EL1_RES1                           (UL(0))
#define ICC_PPI_HMRx_EL1_UNKN                           (UL(0))

#define REG_ICC_PPI_HMR0_EL1                            S3_0_C12_C10_0
#define SYS_ICC_PPI_HMR0_EL1                            sys_reg(3, 0, 12, 10, 0)
#define SYS_ICC_PPI_HMR0_EL1_Op0                        3
#define SYS_ICC_PPI_HMR0_EL1_Op1                        0
#define SYS_ICC_PPI_HMR0_EL1_CRn                        12
#define SYS_ICC_PPI_HMR0_EL1_CRm                        10
#define SYS_ICC_PPI_HMR0_EL1_Op2                        0

/* For ICC_PPI_HMR0_EL1 fields see ICC_PPI_HMRx_EL1 */

#define REG_ICC_PPI_HMR1_EL1                            S3_0_C12_C10_1
#define SYS_ICC_PPI_HMR1_EL1                            sys_reg(3, 0, 12, 10, 1)
#define SYS_ICC_PPI_HMR1_EL1_Op0                        3
#define SYS_ICC_PPI_HMR1_EL1_Op1                        0
#define SYS_ICC_PPI_HMR1_EL1_CRn                        12
#define SYS_ICC_PPI_HMR1_EL1_CRm                        10
#define SYS_ICC_PPI_HMR1_EL1_Op2                        1

/* For ICC_PPI_HMR1_EL1 fields see ICC_PPI_HMRx_EL1 */

#define REG_ICC_IDR0_EL1                                S3_0_C12_C10_2
#define SYS_ICC_IDR0_EL1                                sys_reg(3, 0, 12, 10, 2)
#define SYS_ICC_IDR0_EL1_Op0                            3
#define SYS_ICC_IDR0_EL1_Op1                            0
#define SYS_ICC_IDR0_EL1_CRn                            12
#define SYS_ICC_IDR0_EL1_CRm                            10
#define SYS_ICC_IDR0_EL1_Op2                            2

#define ICC_IDR0_EL1_GCIE_LEGACY                        GENMASK(11, 8)
#define ICC_IDR0_EL1_GCIE_LEGACY_MASK                   GENMASK(11, 8)
#define ICC_IDR0_EL1_GCIE_LEGACY_SHIFT                  8
#define ICC_IDR0_EL1_GCIE_LEGACY_WIDTH                  4
#define ICC_IDR0_EL1_GCIE_LEGACY_SIGNED                 false
#define ICC_IDR0_EL1_GCIE_LEGACY_NI                     UL(0b0000)
#define ICC_IDR0_EL1_GCIE_LEGACY_IMP                    UL(0b0001)

#define ICC_IDR0_EL1_PRI_BITS                           GENMASK(7, 4)
#define ICC_IDR0_EL1_PRI_BITS_MASK                      GENMASK(7, 4)
#define ICC_IDR0_EL1_PRI_BITS_SHIFT                     4
#define ICC_IDR0_EL1_PRI_BITS_WIDTH                     4
#define ICC_IDR0_EL1_PRI_BITS_SIGNED                    false
#define ICC_IDR0_EL1_PRI_BITS_4BITS                     UL(0b0011)
#define ICC_IDR0_EL1_PRI_BITS_5BITS                     UL(0b0100)

#define ICC_IDR0_EL1_ID_BITS                            GENMASK(3, 0)
#define ICC_IDR0_EL1_ID_BITS_MASK                       GENMASK(3, 0)
#define ICC_IDR0_EL1_ID_BITS_SHIFT                      0
#define ICC_IDR0_EL1_ID_BITS_WIDTH                      4
#define ICC_IDR0_EL1_ID_BITS_SIGNED                     false
#define ICC_IDR0_EL1_ID_BITS_16BITS                     UL(0b0000)
#define ICC_IDR0_EL1_ID_BITS_24BITS                     UL(0b0001)

#define ICC_IDR0_EL1_RES0                               (UL(0) | GENMASK_ULL(63, 12))
#define ICC_IDR0_EL1_RES1                               (UL(0))
#define ICC_IDR0_EL1_UNKN                               (UL(0))

#define REG_ICC_ICSR_EL1                                S3_0_C12_C10_4
#define SYS_ICC_ICSR_EL1                                sys_reg(3, 0, 12, 10, 4)
#define SYS_ICC_ICSR_EL1_Op0                            3
#define SYS_ICC_ICSR_EL1_Op1                            0
#define SYS_ICC_ICSR_EL1_CRn                            12
#define SYS_ICC_ICSR_EL1_CRm                            10
#define SYS_ICC_ICSR_EL1_Op2                            4

#define ICC_ICSR_EL1_IAFFID                             GENMASK(47, 32)
#define ICC_ICSR_EL1_IAFFID_MASK                        GENMASK(47, 32)
#define ICC_ICSR_EL1_IAFFID_SHIFT                       32
#define ICC_ICSR_EL1_IAFFID_WIDTH                       16

#define ICC_ICSR_EL1_Priority                           GENMASK(15, 11)
#define ICC_ICSR_EL1_Priority_MASK                      GENMASK(15, 11)
#define ICC_ICSR_EL1_Priority_SHIFT                     11
#define ICC_ICSR_EL1_Priority_WIDTH                     5

#define ICC_ICSR_EL1_HM                                 GENMASK(5, 5)
#define ICC_ICSR_EL1_HM_MASK                            GENMASK(5, 5)
#define ICC_ICSR_EL1_HM_SHIFT                           5
#define ICC_ICSR_EL1_HM_WIDTH                           1

#define ICC_ICSR_EL1_Active                             GENMASK(4, 4)
#define ICC_ICSR_EL1_Active_MASK                        GENMASK(4, 4)
#define ICC_ICSR_EL1_Active_SHIFT                       4
#define ICC_ICSR_EL1_Active_WIDTH                       1

#define ICC_ICSR_EL1_IRM                                GENMASK(3, 3)
#define ICC_ICSR_EL1_IRM_MASK                           GENMASK(3, 3)
#define ICC_ICSR_EL1_IRM_SHIFT                          3
#define ICC_ICSR_EL1_IRM_WIDTH                          1

#define ICC_ICSR_EL1_Pending                            GENMASK(2, 2)
#define ICC_ICSR_EL1_Pending_MASK                       GENMASK(2, 2)
#define ICC_ICSR_EL1_Pending_SHIFT                      2
#define ICC_ICSR_EL1_Pending_WIDTH                      1

#define ICC_ICSR_EL1_Enabled                            GENMASK(1, 1)
#define ICC_ICSR_EL1_Enabled_MASK                       GENMASK(1, 1)
#define ICC_ICSR_EL1_Enabled_SHIFT                      1
#define ICC_ICSR_EL1_Enabled_WIDTH                      1

#define ICC_ICSR_EL1_F                                  GENMASK(0, 0)
#define ICC_ICSR_EL1_F_MASK                             GENMASK(0, 0)
#define ICC_ICSR_EL1_F_SHIFT                            0
#define ICC_ICSR_EL1_F_WIDTH                            1

#define ICC_ICSR_EL1_RES0                               (UL(0) | GENMASK_ULL(63, 48) | GENMASK_ULL(31, 16) | GENMASK_ULL(10, 6))
#define ICC_ICSR_EL1_RES1                               (UL(0))
#define ICC_ICSR_EL1_UNKN                               (UL(0))

#define ICC_PPI_ENABLERx_EL1_EN63                       GENMASK(63, 63)
#define ICC_PPI_ENABLERx_EL1_EN63_MASK                  GENMASK(63, 63)
#define ICC_PPI_ENABLERx_EL1_EN63_SHIFT                 63
#define ICC_PPI_ENABLERx_EL1_EN63_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN62                       GENMASK(62, 62)
#define ICC_PPI_ENABLERx_EL1_EN62_MASK                  GENMASK(62, 62)
#define ICC_PPI_ENABLERx_EL1_EN62_SHIFT                 62
#define ICC_PPI_ENABLERx_EL1_EN62_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN61                       GENMASK(61, 61)
#define ICC_PPI_ENABLERx_EL1_EN61_MASK                  GENMASK(61, 61)
#define ICC_PPI_ENABLERx_EL1_EN61_SHIFT                 61
#define ICC_PPI_ENABLERx_EL1_EN61_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN60                       GENMASK(60, 60)
#define ICC_PPI_ENABLERx_EL1_EN60_MASK                  GENMASK(60, 60)
#define ICC_PPI_ENABLERx_EL1_EN60_SHIFT                 60
#define ICC_PPI_ENABLERx_EL1_EN60_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN59                       GENMASK(59, 59)
#define ICC_PPI_ENABLERx_EL1_EN59_MASK                  GENMASK(59, 59)
#define ICC_PPI_ENABLERx_EL1_EN59_SHIFT                 59
#define ICC_PPI_ENABLERx_EL1_EN59_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN58                       GENMASK(58, 58)
#define ICC_PPI_ENABLERx_EL1_EN58_MASK                  GENMASK(58, 58)
#define ICC_PPI_ENABLERx_EL1_EN58_SHIFT                 58
#define ICC_PPI_ENABLERx_EL1_EN58_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN57                       GENMASK(57, 57)
#define ICC_PPI_ENABLERx_EL1_EN57_MASK                  GENMASK(57, 57)
#define ICC_PPI_ENABLERx_EL1_EN57_SHIFT                 57
#define ICC_PPI_ENABLERx_EL1_EN57_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN56                       GENMASK(56, 56)
#define ICC_PPI_ENABLERx_EL1_EN56_MASK                  GENMASK(56, 56)
#define ICC_PPI_ENABLERx_EL1_EN56_SHIFT                 56
#define ICC_PPI_ENABLERx_EL1_EN56_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN55                       GENMASK(55, 55)
#define ICC_PPI_ENABLERx_EL1_EN55_MASK                  GENMASK(55, 55)
#define ICC_PPI_ENABLERx_EL1_EN55_SHIFT                 55
#define ICC_PPI_ENABLERx_EL1_EN55_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN54                       GENMASK(54, 54)
#define ICC_PPI_ENABLERx_EL1_EN54_MASK                  GENMASK(54, 54)
#define ICC_PPI_ENABLERx_EL1_EN54_SHIFT                 54
#define ICC_PPI_ENABLERx_EL1_EN54_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN53                       GENMASK(53, 53)
#define ICC_PPI_ENABLERx_EL1_EN53_MASK                  GENMASK(53, 53)
#define ICC_PPI_ENABLERx_EL1_EN53_SHIFT                 53
#define ICC_PPI_ENABLERx_EL1_EN53_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN52                       GENMASK(52, 52)
#define ICC_PPI_ENABLERx_EL1_EN52_MASK                  GENMASK(52, 52)
#define ICC_PPI_ENABLERx_EL1_EN52_SHIFT                 52
#define ICC_PPI_ENABLERx_EL1_EN52_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN51                       GENMASK(51, 51)
#define ICC_PPI_ENABLERx_EL1_EN51_MASK                  GENMASK(51, 51)
#define ICC_PPI_ENABLERx_EL1_EN51_SHIFT                 51
#define ICC_PPI_ENABLERx_EL1_EN51_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN50                       GENMASK(50, 50)
#define ICC_PPI_ENABLERx_EL1_EN50_MASK                  GENMASK(50, 50)
#define ICC_PPI_ENABLERx_EL1_EN50_SHIFT                 50
#define ICC_PPI_ENABLERx_EL1_EN50_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN49                       GENMASK(49, 49)
#define ICC_PPI_ENABLERx_EL1_EN49_MASK                  GENMASK(49, 49)
#define ICC_PPI_ENABLERx_EL1_EN49_SHIFT                 49
#define ICC_PPI_ENABLERx_EL1_EN49_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN48                       GENMASK(48, 48)
#define ICC_PPI_ENABLERx_EL1_EN48_MASK                  GENMASK(48, 48)
#define ICC_PPI_ENABLERx_EL1_EN48_SHIFT                 48
#define ICC_PPI_ENABLERx_EL1_EN48_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN47                       GENMASK(47, 47)
#define ICC_PPI_ENABLERx_EL1_EN47_MASK                  GENMASK(47, 47)
#define ICC_PPI_ENABLERx_EL1_EN47_SHIFT                 47
#define ICC_PPI_ENABLERx_EL1_EN47_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN46                       GENMASK(46, 46)
#define ICC_PPI_ENABLERx_EL1_EN46_MASK                  GENMASK(46, 46)
#define ICC_PPI_ENABLERx_EL1_EN46_SHIFT                 46
#define ICC_PPI_ENABLERx_EL1_EN46_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN45                       GENMASK(45, 45)
#define ICC_PPI_ENABLERx_EL1_EN45_MASK                  GENMASK(45, 45)
#define ICC_PPI_ENABLERx_EL1_EN45_SHIFT                 45
#define ICC_PPI_ENABLERx_EL1_EN45_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN44                       GENMASK(44, 44)
#define ICC_PPI_ENABLERx_EL1_EN44_MASK                  GENMASK(44, 44)
#define ICC_PPI_ENABLERx_EL1_EN44_SHIFT                 44
#define ICC_PPI_ENABLERx_EL1_EN44_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN43                       GENMASK(43, 43)
#define ICC_PPI_ENABLERx_EL1_EN43_MASK                  GENMASK(43, 43)
#define ICC_PPI_ENABLERx_EL1_EN43_SHIFT                 43
#define ICC_PPI_ENABLERx_EL1_EN43_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN42                       GENMASK(42, 42)
#define ICC_PPI_ENABLERx_EL1_EN42_MASK                  GENMASK(42, 42)
#define ICC_PPI_ENABLERx_EL1_EN42_SHIFT                 42
#define ICC_PPI_ENABLERx_EL1_EN42_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN41                       GENMASK(41, 41)
#define ICC_PPI_ENABLERx_EL1_EN41_MASK                  GENMASK(41, 41)
#define ICC_PPI_ENABLERx_EL1_EN41_SHIFT                 41
#define ICC_PPI_ENABLERx_EL1_EN41_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN40                       GENMASK(40, 40)
#define ICC_PPI_ENABLERx_EL1_EN40_MASK                  GENMASK(40, 40)
#define ICC_PPI_ENABLERx_EL1_EN40_SHIFT                 40
#define ICC_PPI_ENABLERx_EL1_EN40_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN39                       GENMASK(39, 39)
#define ICC_PPI_ENABLERx_EL1_EN39_MASK                  GENMASK(39, 39)
#define ICC_PPI_ENABLERx_EL1_EN39_SHIFT                 39
#define ICC_PPI_ENABLERx_EL1_EN39_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN38                       GENMASK(38, 38)
#define ICC_PPI_ENABLERx_EL1_EN38_MASK                  GENMASK(38, 38)
#define ICC_PPI_ENABLERx_EL1_EN38_SHIFT                 38
#define ICC_PPI_ENABLERx_EL1_EN38_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN37                       GENMASK(37, 37)
#define ICC_PPI_ENABLERx_EL1_EN37_MASK                  GENMASK(37, 37)
#define ICC_PPI_ENABLERx_EL1_EN37_SHIFT                 37
#define ICC_PPI_ENABLERx_EL1_EN37_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN36                       GENMASK(36, 36)
#define ICC_PPI_ENABLERx_EL1_EN36_MASK                  GENMASK(36, 36)
#define ICC_PPI_ENABLERx_EL1_EN36_SHIFT                 36
#define ICC_PPI_ENABLERx_EL1_EN36_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN35                       GENMASK(35, 35)
#define ICC_PPI_ENABLERx_EL1_EN35_MASK                  GENMASK(35, 35)
#define ICC_PPI_ENABLERx_EL1_EN35_SHIFT                 35
#define ICC_PPI_ENABLERx_EL1_EN35_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN34                       GENMASK(34, 34)
#define ICC_PPI_ENABLERx_EL1_EN34_MASK                  GENMASK(34, 34)
#define ICC_PPI_ENABLERx_EL1_EN34_SHIFT                 34
#define ICC_PPI_ENABLERx_EL1_EN34_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN33                       GENMASK(33, 33)
#define ICC_PPI_ENABLERx_EL1_EN33_MASK                  GENMASK(33, 33)
#define ICC_PPI_ENABLERx_EL1_EN33_SHIFT                 33
#define ICC_PPI_ENABLERx_EL1_EN33_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN32                       GENMASK(32, 32)
#define ICC_PPI_ENABLERx_EL1_EN32_MASK                  GENMASK(32, 32)
#define ICC_PPI_ENABLERx_EL1_EN32_SHIFT                 32
#define ICC_PPI_ENABLERx_EL1_EN32_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN31                       GENMASK(31, 31)
#define ICC_PPI_ENABLERx_EL1_EN31_MASK                  GENMASK(31, 31)
#define ICC_PPI_ENABLERx_EL1_EN31_SHIFT                 31
#define ICC_PPI_ENABLERx_EL1_EN31_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN30                       GENMASK(30, 30)
#define ICC_PPI_ENABLERx_EL1_EN30_MASK                  GENMASK(30, 30)
#define ICC_PPI_ENABLERx_EL1_EN30_SHIFT                 30
#define ICC_PPI_ENABLERx_EL1_EN30_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN29                       GENMASK(29, 29)
#define ICC_PPI_ENABLERx_EL1_EN29_MASK                  GENMASK(29, 29)
#define ICC_PPI_ENABLERx_EL1_EN29_SHIFT                 29
#define ICC_PPI_ENABLERx_EL1_EN29_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN28                       GENMASK(28, 28)
#define ICC_PPI_ENABLERx_EL1_EN28_MASK                  GENMASK(28, 28)
#define ICC_PPI_ENABLERx_EL1_EN28_SHIFT                 28
#define ICC_PPI_ENABLERx_EL1_EN28_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN27                       GENMASK(27, 27)
#define ICC_PPI_ENABLERx_EL1_EN27_MASK                  GENMASK(27, 27)
#define ICC_PPI_ENABLERx_EL1_EN27_SHIFT                 27
#define ICC_PPI_ENABLERx_EL1_EN27_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN26                       GENMASK(26, 26)
#define ICC_PPI_ENABLERx_EL1_EN26_MASK                  GENMASK(26, 26)
#define ICC_PPI_ENABLERx_EL1_EN26_SHIFT                 26
#define ICC_PPI_ENABLERx_EL1_EN26_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN25                       GENMASK(25, 25)
#define ICC_PPI_ENABLERx_EL1_EN25_MASK                  GENMASK(25, 25)
#define ICC_PPI_ENABLERx_EL1_EN25_SHIFT                 25
#define ICC_PPI_ENABLERx_EL1_EN25_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN24                       GENMASK(24, 24)
#define ICC_PPI_ENABLERx_EL1_EN24_MASK                  GENMASK(24, 24)
#define ICC_PPI_ENABLERx_EL1_EN24_SHIFT                 24
#define ICC_PPI_ENABLERx_EL1_EN24_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN23                       GENMASK(23, 23)
#define ICC_PPI_ENABLERx_EL1_EN23_MASK                  GENMASK(23, 23)
#define ICC_PPI_ENABLERx_EL1_EN23_SHIFT                 23
#define ICC_PPI_ENABLERx_EL1_EN23_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN22                       GENMASK(22, 22)
#define ICC_PPI_ENABLERx_EL1_EN22_MASK                  GENMASK(22, 22)
#define ICC_PPI_ENABLERx_EL1_EN22_SHIFT                 22
#define ICC_PPI_ENABLERx_EL1_EN22_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN21                       GENMASK(21, 21)
#define ICC_PPI_ENABLERx_EL1_EN21_MASK                  GENMASK(21, 21)
#define ICC_PPI_ENABLERx_EL1_EN21_SHIFT                 21
#define ICC_PPI_ENABLERx_EL1_EN21_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN20                       GENMASK(20, 20)
#define ICC_PPI_ENABLERx_EL1_EN20_MASK                  GENMASK(20, 20)
#define ICC_PPI_ENABLERx_EL1_EN20_SHIFT                 20
#define ICC_PPI_ENABLERx_EL1_EN20_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN19                       GENMASK(19, 19)
#define ICC_PPI_ENABLERx_EL1_EN19_MASK                  GENMASK(19, 19)
#define ICC_PPI_ENABLERx_EL1_EN19_SHIFT                 19
#define ICC_PPI_ENABLERx_EL1_EN19_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN18                       GENMASK(18, 18)
#define ICC_PPI_ENABLERx_EL1_EN18_MASK                  GENMASK(18, 18)
#define ICC_PPI_ENABLERx_EL1_EN18_SHIFT                 18
#define ICC_PPI_ENABLERx_EL1_EN18_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN17                       GENMASK(17, 17)
#define ICC_PPI_ENABLERx_EL1_EN17_MASK                  GENMASK(17, 17)
#define ICC_PPI_ENABLERx_EL1_EN17_SHIFT                 17
#define ICC_PPI_ENABLERx_EL1_EN17_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN16                       GENMASK(16, 16)
#define ICC_PPI_ENABLERx_EL1_EN16_MASK                  GENMASK(16, 16)
#define ICC_PPI_ENABLERx_EL1_EN16_SHIFT                 16
#define ICC_PPI_ENABLERx_EL1_EN16_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN15                       GENMASK(15, 15)
#define ICC_PPI_ENABLERx_EL1_EN15_MASK                  GENMASK(15, 15)
#define ICC_PPI_ENABLERx_EL1_EN15_SHIFT                 15
#define ICC_PPI_ENABLERx_EL1_EN15_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN14                       GENMASK(14, 14)
#define ICC_PPI_ENABLERx_EL1_EN14_MASK                  GENMASK(14, 14)
#define ICC_PPI_ENABLERx_EL1_EN14_SHIFT                 14
#define ICC_PPI_ENABLERx_EL1_EN14_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN13                       GENMASK(13, 13)
#define ICC_PPI_ENABLERx_EL1_EN13_MASK                  GENMASK(13, 13)
#define ICC_PPI_ENABLERx_EL1_EN13_SHIFT                 13
#define ICC_PPI_ENABLERx_EL1_EN13_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN12                       GENMASK(12, 12)
#define ICC_PPI_ENABLERx_EL1_EN12_MASK                  GENMASK(12, 12)
#define ICC_PPI_ENABLERx_EL1_EN12_SHIFT                 12
#define ICC_PPI_ENABLERx_EL1_EN12_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN11                       GENMASK(11, 11)
#define ICC_PPI_ENABLERx_EL1_EN11_MASK                  GENMASK(11, 11)
#define ICC_PPI_ENABLERx_EL1_EN11_SHIFT                 11
#define ICC_PPI_ENABLERx_EL1_EN11_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN10                       GENMASK(10, 10)
#define ICC_PPI_ENABLERx_EL1_EN10_MASK                  GENMASK(10, 10)
#define ICC_PPI_ENABLERx_EL1_EN10_SHIFT                 10
#define ICC_PPI_ENABLERx_EL1_EN10_WIDTH                 1

#define ICC_PPI_ENABLERx_EL1_EN9                        GENMASK(9, 9)
#define ICC_PPI_ENABLERx_EL1_EN9_MASK                   GENMASK(9, 9)
#define ICC_PPI_ENABLERx_EL1_EN9_SHIFT                  9
#define ICC_PPI_ENABLERx_EL1_EN9_WIDTH                  1

#define ICC_PPI_ENABLERx_EL1_EN8                        GENMASK(8, 8)
#define ICC_PPI_ENABLERx_EL1_EN8_MASK                   GENMASK(8, 8)
#define ICC_PPI_ENABLERx_EL1_EN8_SHIFT                  8
#define ICC_PPI_ENABLERx_EL1_EN8_WIDTH                  1

#define ICC_PPI_ENABLERx_EL1_EN7                        GENMASK(7, 7)
#define ICC_PPI_ENABLERx_EL1_EN7_MASK                   GENMASK(7, 7)
#define ICC_PPI_ENABLERx_EL1_EN7_SHIFT                  7
#define ICC_PPI_ENABLERx_EL1_EN7_WIDTH                  1

#define ICC_PPI_ENABLERx_EL1_EN6                        GENMASK(6, 6)
#define ICC_PPI_ENABLERx_EL1_EN6_MASK                   GENMASK(6, 6)
#define ICC_PPI_ENABLERx_EL1_EN6_SHIFT                  6
#define ICC_PPI_ENABLERx_EL1_EN6_WIDTH                  1

#define ICC_PPI_ENABLERx_EL1_EN5                        GENMASK(5, 5)
#define ICC_PPI_ENABLERx_EL1_EN5_MASK                   GENMASK(5, 5)
#define ICC_PPI_ENABLERx_EL1_EN5_SHIFT                  5
#define ICC_PPI_ENABLERx_EL1_EN5_WIDTH                  1

#define ICC_PPI_ENABLERx_EL1_EN4                        GENMASK(4, 4)
#define ICC_PPI_ENABLERx_EL1_EN4_MASK                   GENMASK(4, 4)
#define ICC_PPI_ENABLERx_EL1_EN4_SHIFT                  4
#define ICC_PPI_ENABLERx_EL1_EN4_WIDTH                  1

#define ICC_PPI_ENABLERx_EL1_EN3                        GENMASK(3, 3)
#define ICC_PPI_ENABLERx_EL1_EN3_MASK                   GENMASK(3, 3)
#define ICC_PPI_ENABLERx_EL1_EN3_SHIFT                  3
#define ICC_PPI_ENABLERx_EL1_EN3_WIDTH                  1

#define ICC_PPI_ENABLERx_EL1_EN2                        GENMASK(2, 2)
#define ICC_PPI_ENABLERx_EL1_EN2_MASK                   GENMASK(2, 2)
#define ICC_PPI_ENABLERx_EL1_EN2_SHIFT                  2
#define ICC_PPI_ENABLERx_EL1_EN2_WIDTH                  1

#define ICC_PPI_ENABLERx_EL1_EN1                        GENMASK(1, 1)
#define ICC_PPI_ENABLERx_EL1_EN1_MASK                   GENMASK(1, 1)
#define ICC_PPI_ENABLERx_EL1_EN1_SHIFT                  1
#define ICC_PPI_ENABLERx_EL1_EN1_WIDTH                  1

#define ICC_PPI_ENABLERx_EL1_EN0                        GENMASK(0, 0)
#define ICC_PPI_ENABLERx_EL1_EN0_MASK                   GENMASK(0, 0)
#define ICC_PPI_ENABLERx_EL1_EN0_SHIFT                  0
#define ICC_PPI_ENABLERx_EL1_EN0_WIDTH                  1

#define ICC_PPI_ENABLERx_EL1_RES0                       (UL(0))
#define ICC_PPI_ENABLERx_EL1_RES1                       (UL(0))
#define ICC_PPI_ENABLERx_EL1_UNKN                       (UL(0))

#define REG_ICC_PPI_ENABLER0_EL1                        S3_0_C12_C10_6
#define SYS_ICC_PPI_ENABLER0_EL1                        sys_reg(3, 0, 12, 10, 6)
#define SYS_ICC_PPI_ENABLER0_EL1_Op0                    3
#define SYS_ICC_PPI_ENABLER0_EL1_Op1                    0
#define SYS_ICC_PPI_ENABLER0_EL1_CRn                    12
#define SYS_ICC_PPI_ENABLER0_EL1_CRm                    10
#define SYS_ICC_PPI_ENABLER0_EL1_Op2                    6

/* For ICC_PPI_ENABLER0_EL1 fields see ICC_PPI_ENABLERx_EL1 */

#define REG_ICC_PPI_ENABLER1_EL1                        S3_0_C12_C10_7
#define SYS_ICC_PPI_ENABLER1_EL1                        sys_reg(3, 0, 12, 10, 7)
#define SYS_ICC_PPI_ENABLER1_EL1_Op0                    3
#define SYS_ICC_PPI_ENABLER1_EL1_Op1                    0
#define SYS_ICC_PPI_ENABLER1_EL1_CRn                    12
#define SYS_ICC_PPI_ENABLER1_EL1_CRm                    10
#define SYS_ICC_PPI_ENABLER1_EL1_Op2                    7

/* For ICC_PPI_ENABLER1_EL1 fields see ICC_PPI_ENABLERx_EL1 */

#define ICC_PPI_ACTIVERx_EL1_Active63                   GENMASK(63, 63)
#define ICC_PPI_ACTIVERx_EL1_Active63_MASK              GENMASK(63, 63)
#define ICC_PPI_ACTIVERx_EL1_Active63_SHIFT             63
#define ICC_PPI_ACTIVERx_EL1_Active63_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active62                   GENMASK(62, 62)
#define ICC_PPI_ACTIVERx_EL1_Active62_MASK              GENMASK(62, 62)
#define ICC_PPI_ACTIVERx_EL1_Active62_SHIFT             62
#define ICC_PPI_ACTIVERx_EL1_Active62_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active61                   GENMASK(61, 61)
#define ICC_PPI_ACTIVERx_EL1_Active61_MASK              GENMASK(61, 61)
#define ICC_PPI_ACTIVERx_EL1_Active61_SHIFT             61
#define ICC_PPI_ACTIVERx_EL1_Active61_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active60                   GENMASK(60, 60)
#define ICC_PPI_ACTIVERx_EL1_Active60_MASK              GENMASK(60, 60)
#define ICC_PPI_ACTIVERx_EL1_Active60_SHIFT             60
#define ICC_PPI_ACTIVERx_EL1_Active60_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active59                   GENMASK(59, 59)
#define ICC_PPI_ACTIVERx_EL1_Active59_MASK              GENMASK(59, 59)
#define ICC_PPI_ACTIVERx_EL1_Active59_SHIFT             59
#define ICC_PPI_ACTIVERx_EL1_Active59_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active58                   GENMASK(58, 58)
#define ICC_PPI_ACTIVERx_EL1_Active58_MASK              GENMASK(58, 58)
#define ICC_PPI_ACTIVERx_EL1_Active58_SHIFT             58
#define ICC_PPI_ACTIVERx_EL1_Active58_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active57                   GENMASK(57, 57)
#define ICC_PPI_ACTIVERx_EL1_Active57_MASK              GENMASK(57, 57)
#define ICC_PPI_ACTIVERx_EL1_Active57_SHIFT             57
#define ICC_PPI_ACTIVERx_EL1_Active57_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active56                   GENMASK(56, 56)
#define ICC_PPI_ACTIVERx_EL1_Active56_MASK              GENMASK(56, 56)
#define ICC_PPI_ACTIVERx_EL1_Active56_SHIFT             56
#define ICC_PPI_ACTIVERx_EL1_Active56_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active55                   GENMASK(55, 55)
#define ICC_PPI_ACTIVERx_EL1_Active55_MASK              GENMASK(55, 55)
#define ICC_PPI_ACTIVERx_EL1_Active55_SHIFT             55
#define ICC_PPI_ACTIVERx_EL1_Active55_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active54                   GENMASK(54, 54)
#define ICC_PPI_ACTIVERx_EL1_Active54_MASK              GENMASK(54, 54)
#define ICC_PPI_ACTIVERx_EL1_Active54_SHIFT             54
#define ICC_PPI_ACTIVERx_EL1_Active54_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active53                   GENMASK(53, 53)
#define ICC_PPI_ACTIVERx_EL1_Active53_MASK              GENMASK(53, 53)
#define ICC_PPI_ACTIVERx_EL1_Active53_SHIFT             53
#define ICC_PPI_ACTIVERx_EL1_Active53_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active52                   GENMASK(52, 52)
#define ICC_PPI_ACTIVERx_EL1_Active52_MASK              GENMASK(52, 52)
#define ICC_PPI_ACTIVERx_EL1_Active52_SHIFT             52
#define ICC_PPI_ACTIVERx_EL1_Active52_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active51                   GENMASK(51, 51)
#define ICC_PPI_ACTIVERx_EL1_Active51_MASK              GENMASK(51, 51)
#define ICC_PPI_ACTIVERx_EL1_Active51_SHIFT             51
#define ICC_PPI_ACTIVERx_EL1_Active51_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active50                   GENMASK(50, 50)
#define ICC_PPI_ACTIVERx_EL1_Active50_MASK              GENMASK(50, 50)
#define ICC_PPI_ACTIVERx_EL1_Active50_SHIFT             50
#define ICC_PPI_ACTIVERx_EL1_Active50_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active49                   GENMASK(49, 49)
#define ICC_PPI_ACTIVERx_EL1_Active49_MASK              GENMASK(49, 49)
#define ICC_PPI_ACTIVERx_EL1_Active49_SHIFT             49
#define ICC_PPI_ACTIVERx_EL1_Active49_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active48                   GENMASK(48, 48)
#define ICC_PPI_ACTIVERx_EL1_Active48_MASK              GENMASK(48, 48)
#define ICC_PPI_ACTIVERx_EL1_Active48_SHIFT             48
#define ICC_PPI_ACTIVERx_EL1_Active48_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active47                   GENMASK(47, 47)
#define ICC_PPI_ACTIVERx_EL1_Active47_MASK              GENMASK(47, 47)
#define ICC_PPI_ACTIVERx_EL1_Active47_SHIFT             47
#define ICC_PPI_ACTIVERx_EL1_Active47_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active46                   GENMASK(46, 46)
#define ICC_PPI_ACTIVERx_EL1_Active46_MASK              GENMASK(46, 46)
#define ICC_PPI_ACTIVERx_EL1_Active46_SHIFT             46
#define ICC_PPI_ACTIVERx_EL1_Active46_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active45                   GENMASK(45, 45)
#define ICC_PPI_ACTIVERx_EL1_Active45_MASK              GENMASK(45, 45)
#define ICC_PPI_ACTIVERx_EL1_Active45_SHIFT             45
#define ICC_PPI_ACTIVERx_EL1_Active45_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active44                   GENMASK(44, 44)
#define ICC_PPI_ACTIVERx_EL1_Active44_MASK              GENMASK(44, 44)
#define ICC_PPI_ACTIVERx_EL1_Active44_SHIFT             44
#define ICC_PPI_ACTIVERx_EL1_Active44_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active43                   GENMASK(43, 43)
#define ICC_PPI_ACTIVERx_EL1_Active43_MASK              GENMASK(43, 43)
#define ICC_PPI_ACTIVERx_EL1_Active43_SHIFT             43
#define ICC_PPI_ACTIVERx_EL1_Active43_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active42                   GENMASK(42, 42)
#define ICC_PPI_ACTIVERx_EL1_Active42_MASK              GENMASK(42, 42)
#define ICC_PPI_ACTIVERx_EL1_Active42_SHIFT             42
#define ICC_PPI_ACTIVERx_EL1_Active42_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active41                   GENMASK(41, 41)
#define ICC_PPI_ACTIVERx_EL1_Active41_MASK              GENMASK(41, 41)
#define ICC_PPI_ACTIVERx_EL1_Active41_SHIFT             41
#define ICC_PPI_ACTIVERx_EL1_Active41_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active40                   GENMASK(40, 40)
#define ICC_PPI_ACTIVERx_EL1_Active40_MASK              GENMASK(40, 40)
#define ICC_PPI_ACTIVERx_EL1_Active40_SHIFT             40
#define ICC_PPI_ACTIVERx_EL1_Active40_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active39                   GENMASK(39, 39)
#define ICC_PPI_ACTIVERx_EL1_Active39_MASK              GENMASK(39, 39)
#define ICC_PPI_ACTIVERx_EL1_Active39_SHIFT             39
#define ICC_PPI_ACTIVERx_EL1_Active39_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active38                   GENMASK(38, 38)
#define ICC_PPI_ACTIVERx_EL1_Active38_MASK              GENMASK(38, 38)
#define ICC_PPI_ACTIVERx_EL1_Active38_SHIFT             38
#define ICC_PPI_ACTIVERx_EL1_Active38_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active37                   GENMASK(37, 37)
#define ICC_PPI_ACTIVERx_EL1_Active37_MASK              GENMASK(37, 37)
#define ICC_PPI_ACTIVERx_EL1_Active37_SHIFT             37
#define ICC_PPI_ACTIVERx_EL1_Active37_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active36                   GENMASK(36, 36)
#define ICC_PPI_ACTIVERx_EL1_Active36_MASK              GENMASK(36, 36)
#define ICC_PPI_ACTIVERx_EL1_Active36_SHIFT             36
#define ICC_PPI_ACTIVERx_EL1_Active36_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active35                   GENMASK(35, 35)
#define ICC_PPI_ACTIVERx_EL1_Active35_MASK              GENMASK(35, 35)
#define ICC_PPI_ACTIVERx_EL1_Active35_SHIFT             35
#define ICC_PPI_ACTIVERx_EL1_Active35_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active34                   GENMASK(34, 34)
#define ICC_PPI_ACTIVERx_EL1_Active34_MASK              GENMASK(34, 34)
#define ICC_PPI_ACTIVERx_EL1_Active34_SHIFT             34
#define ICC_PPI_ACTIVERx_EL1_Active34_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active33                   GENMASK(33, 33)
#define ICC_PPI_ACTIVERx_EL1_Active33_MASK              GENMASK(33, 33)
#define ICC_PPI_ACTIVERx_EL1_Active33_SHIFT             33
#define ICC_PPI_ACTIVERx_EL1_Active33_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active32                   GENMASK(32, 32)
#define ICC_PPI_ACTIVERx_EL1_Active32_MASK              GENMASK(32, 32)
#define ICC_PPI_ACTIVERx_EL1_Active32_SHIFT             32
#define ICC_PPI_ACTIVERx_EL1_Active32_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active31                   GENMASK(31, 31)
#define ICC_PPI_ACTIVERx_EL1_Active31_MASK              GENMASK(31, 31)
#define ICC_PPI_ACTIVERx_EL1_Active31_SHIFT             31
#define ICC_PPI_ACTIVERx_EL1_Active31_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active30                   GENMASK(30, 30)
#define ICC_PPI_ACTIVERx_EL1_Active30_MASK              GENMASK(30, 30)
#define ICC_PPI_ACTIVERx_EL1_Active30_SHIFT             30
#define ICC_PPI_ACTIVERx_EL1_Active30_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active29                   GENMASK(29, 29)
#define ICC_PPI_ACTIVERx_EL1_Active29_MASK              GENMASK(29, 29)
#define ICC_PPI_ACTIVERx_EL1_Active29_SHIFT             29
#define ICC_PPI_ACTIVERx_EL1_Active29_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active28                   GENMASK(28, 28)
#define ICC_PPI_ACTIVERx_EL1_Active28_MASK              GENMASK(28, 28)
#define ICC_PPI_ACTIVERx_EL1_Active28_SHIFT             28
#define ICC_PPI_ACTIVERx_EL1_Active28_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active27                   GENMASK(27, 27)
#define ICC_PPI_ACTIVERx_EL1_Active27_MASK              GENMASK(27, 27)
#define ICC_PPI_ACTIVERx_EL1_Active27_SHIFT             27
#define ICC_PPI_ACTIVERx_EL1_Active27_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active26                   GENMASK(26, 26)
#define ICC_PPI_ACTIVERx_EL1_Active26_MASK              GENMASK(26, 26)
#define ICC_PPI_ACTIVERx_EL1_Active26_SHIFT             26
#define ICC_PPI_ACTIVERx_EL1_Active26_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active25                   GENMASK(25, 25)
#define ICC_PPI_ACTIVERx_EL1_Active25_MASK              GENMASK(25, 25)
#define ICC_PPI_ACTIVERx_EL1_Active25_SHIFT             25
#define ICC_PPI_ACTIVERx_EL1_Active25_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active24                   GENMASK(24, 24)
#define ICC_PPI_ACTIVERx_EL1_Active24_MASK              GENMASK(24, 24)
#define ICC_PPI_ACTIVERx_EL1_Active24_SHIFT             24
#define ICC_PPI_ACTIVERx_EL1_Active24_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active23                   GENMASK(23, 23)
#define ICC_PPI_ACTIVERx_EL1_Active23_MASK              GENMASK(23, 23)
#define ICC_PPI_ACTIVERx_EL1_Active23_SHIFT             23
#define ICC_PPI_ACTIVERx_EL1_Active23_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active22                   GENMASK(22, 22)
#define ICC_PPI_ACTIVERx_EL1_Active22_MASK              GENMASK(22, 22)
#define ICC_PPI_ACTIVERx_EL1_Active22_SHIFT             22
#define ICC_PPI_ACTIVERx_EL1_Active22_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active21                   GENMASK(21, 21)
#define ICC_PPI_ACTIVERx_EL1_Active21_MASK              GENMASK(21, 21)
#define ICC_PPI_ACTIVERx_EL1_Active21_SHIFT             21
#define ICC_PPI_ACTIVERx_EL1_Active21_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active20                   GENMASK(20, 20)
#define ICC_PPI_ACTIVERx_EL1_Active20_MASK              GENMASK(20, 20)
#define ICC_PPI_ACTIVERx_EL1_Active20_SHIFT             20
#define ICC_PPI_ACTIVERx_EL1_Active20_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active19                   GENMASK(19, 19)
#define ICC_PPI_ACTIVERx_EL1_Active19_MASK              GENMASK(19, 19)
#define ICC_PPI_ACTIVERx_EL1_Active19_SHIFT             19
#define ICC_PPI_ACTIVERx_EL1_Active19_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active18                   GENMASK(18, 18)
#define ICC_PPI_ACTIVERx_EL1_Active18_MASK              GENMASK(18, 18)
#define ICC_PPI_ACTIVERx_EL1_Active18_SHIFT             18
#define ICC_PPI_ACTIVERx_EL1_Active18_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active17                   GENMASK(17, 17)
#define ICC_PPI_ACTIVERx_EL1_Active17_MASK              GENMASK(17, 17)
#define ICC_PPI_ACTIVERx_EL1_Active17_SHIFT             17
#define ICC_PPI_ACTIVERx_EL1_Active17_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active16                   GENMASK(16, 16)
#define ICC_PPI_ACTIVERx_EL1_Active16_MASK              GENMASK(16, 16)
#define ICC_PPI_ACTIVERx_EL1_Active16_SHIFT             16
#define ICC_PPI_ACTIVERx_EL1_Active16_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active15                   GENMASK(15, 15)
#define ICC_PPI_ACTIVERx_EL1_Active15_MASK              GENMASK(15, 15)
#define ICC_PPI_ACTIVERx_EL1_Active15_SHIFT             15
#define ICC_PPI_ACTIVERx_EL1_Active15_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active14                   GENMASK(14, 14)
#define ICC_PPI_ACTIVERx_EL1_Active14_MASK              GENMASK(14, 14)
#define ICC_PPI_ACTIVERx_EL1_Active14_SHIFT             14
#define ICC_PPI_ACTIVERx_EL1_Active14_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active13                   GENMASK(13, 13)
#define ICC_PPI_ACTIVERx_EL1_Active13_MASK              GENMASK(13, 13)
#define ICC_PPI_ACTIVERx_EL1_Active13_SHIFT             13
#define ICC_PPI_ACTIVERx_EL1_Active13_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active12                   GENMASK(12, 12)
#define ICC_PPI_ACTIVERx_EL1_Active12_MASK              GENMASK(12, 12)
#define ICC_PPI_ACTIVERx_EL1_Active12_SHIFT             12
#define ICC_PPI_ACTIVERx_EL1_Active12_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active11                   GENMASK(11, 11)
#define ICC_PPI_ACTIVERx_EL1_Active11_MASK              GENMASK(11, 11)
#define ICC_PPI_ACTIVERx_EL1_Active11_SHIFT             11
#define ICC_PPI_ACTIVERx_EL1_Active11_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active10                   GENMASK(10, 10)
#define ICC_PPI_ACTIVERx_EL1_Active10_MASK              GENMASK(10, 10)
#define ICC_PPI_ACTIVERx_EL1_Active10_SHIFT             10
#define ICC_PPI_ACTIVERx_EL1_Active10_WIDTH             1

#define ICC_PPI_ACTIVERx_EL1_Active9                    GENMASK(9, 9)
#define ICC_PPI_ACTIVERx_EL1_Active9_MASK               GENMASK(9, 9)
#define ICC_PPI_ACTIVERx_EL1_Active9_SHIFT              9
#define ICC_PPI_ACTIVERx_EL1_Active9_WIDTH              1

#define ICC_PPI_ACTIVERx_EL1_Active8                    GENMASK(8, 8)
#define ICC_PPI_ACTIVERx_EL1_Active8_MASK               GENMASK(8, 8)
#define ICC_PPI_ACTIVERx_EL1_Active8_SHIFT              8
#define ICC_PPI_ACTIVERx_EL1_Active8_WIDTH              1

#define ICC_PPI_ACTIVERx_EL1_Active7                    GENMASK(7, 7)
#define ICC_PPI_ACTIVERx_EL1_Active7_MASK               GENMASK(7, 7)
#define ICC_PPI_ACTIVERx_EL1_Active7_SHIFT              7
#define ICC_PPI_ACTIVERx_EL1_Active7_WIDTH              1

#define ICC_PPI_ACTIVERx_EL1_Active6                    GENMASK(6, 6)
#define ICC_PPI_ACTIVERx_EL1_Active6_MASK               GENMASK(6, 6)
#define ICC_PPI_ACTIVERx_EL1_Active6_SHIFT              6
#define ICC_PPI_ACTIVERx_EL1_Active6_WIDTH              1

#define ICC_PPI_ACTIVERx_EL1_Active5                    GENMASK(5, 5)
#define ICC_PPI_ACTIVERx_EL1_Active5_MASK               GENMASK(5, 5)
#define ICC_PPI_ACTIVERx_EL1_Active5_SHIFT              5
#define ICC_PPI_ACTIVERx_EL1_Active5_WIDTH              1

#define ICC_PPI_ACTIVERx_EL1_Active4                    GENMASK(4, 4)
#define ICC_PPI_ACTIVERx_EL1_Active4_MASK               GENMASK(4, 4)
#define ICC_PPI_ACTIVERx_EL1_Active4_SHIFT              4
#define ICC_PPI_ACTIVERx_EL1_Active4_WIDTH              1

#define ICC_PPI_ACTIVERx_EL1_Active3                    GENMASK(3, 3)
#define ICC_PPI_ACTIVERx_EL1_Active3_MASK               GENMASK(3, 3)
#define ICC_PPI_ACTIVERx_EL1_Active3_SHIFT              3
#define ICC_PPI_ACTIVERx_EL1_Active3_WIDTH              1

#define ICC_PPI_ACTIVERx_EL1_Active2                    GENMASK(2, 2)
#define ICC_PPI_ACTIVERx_EL1_Active2_MASK               GENMASK(2, 2)
#define ICC_PPI_ACTIVERx_EL1_Active2_SHIFT              2
#define ICC_PPI_ACTIVERx_EL1_Active2_WIDTH              1

#define ICC_PPI_ACTIVERx_EL1_Active1                    GENMASK(1, 1)
#define ICC_PPI_ACTIVERx_EL1_Active1_MASK               GENMASK(1, 1)
#define ICC_PPI_ACTIVERx_EL1_Active1_SHIFT              1
#define ICC_PPI_ACTIVERx_EL1_Active1_WIDTH              1

#define ICC_PPI_ACTIVERx_EL1_Active0                    GENMASK(0, 0)
#define ICC_PPI_ACTIVERx_EL1_Active0_MASK               GENMASK(0, 0)
#define ICC_PPI_ACTIVERx_EL1_Active0_SHIFT              0
#define ICC_PPI_ACTIVERx_EL1_Active0_WIDTH              1

#define ICC_PPI_ACTIVERx_EL1_RES0                       (UL(0))
#define ICC_PPI_ACTIVERx_EL1_RES1                       (UL(0))
#define ICC_PPI_ACTIVERx_EL1_UNKN                       (UL(0))

#define REG_ICC_PPI_CACTIVER0_EL1                       S3_0_C12_C13_0
#define SYS_ICC_PPI_CACTIVER0_EL1                       sys_reg(3, 0, 12, 13, 0)
#define SYS_ICC_PPI_CACTIVER0_EL1_Op0                   3
#define SYS_ICC_PPI_CACTIVER0_EL1_Op1                   0
#define SYS_ICC_PPI_CACTIVER0_EL1_CRn                   12
#define SYS_ICC_PPI_CACTIVER0_EL1_CRm                   13
#define SYS_ICC_PPI_CACTIVER0_EL1_Op2                   0

/* For ICC_PPI_CACTIVER0_EL1 fields see ICC_PPI_ACTIVERx_EL1 */

#define REG_ICC_PPI_CACTIVER1_EL1                       S3_0_C12_C13_1
#define SYS_ICC_PPI_CACTIVER1_EL1                       sys_reg(3, 0, 12, 13, 1)
#define SYS_ICC_PPI_CACTIVER1_EL1_Op0                   3
#define SYS_ICC_PPI_CACTIVER1_EL1_Op1                   0
#define SYS_ICC_PPI_CACTIVER1_EL1_CRn                   12
#define SYS_ICC_PPI_CACTIVER1_EL1_CRm                   13
#define SYS_ICC_PPI_CACTIVER1_EL1_Op2                   1

/* For ICC_PPI_CACTIVER1_EL1 fields see ICC_PPI_ACTIVERx_EL1 */

#define REG_ICC_PPI_SACTIVER0_EL1                       S3_0_C12_C13_2
#define SYS_ICC_PPI_SACTIVER0_EL1                       sys_reg(3, 0, 12, 13, 2)
#define SYS_ICC_PPI_SACTIVER0_EL1_Op0                   3
#define SYS_ICC_PPI_SACTIVER0_EL1_Op1                   0
#define SYS_ICC_PPI_SACTIVER0_EL1_CRn                   12
#define SYS_ICC_PPI_SACTIVER0_EL1_CRm                   13
#define SYS_ICC_PPI_SACTIVER0_EL1_Op2                   2

/* For ICC_PPI_SACTIVER0_EL1 fields see ICC_PPI_ACTIVERx_EL1 */

#define REG_ICC_PPI_SACTIVER1_EL1                       S3_0_C12_C13_3
#define SYS_ICC_PPI_SACTIVER1_EL1                       sys_reg(3, 0, 12, 13, 3)
#define SYS_ICC_PPI_SACTIVER1_EL1_Op0                   3
#define SYS_ICC_PPI_SACTIVER1_EL1_Op1                   0
#define SYS_ICC_PPI_SACTIVER1_EL1_CRn                   12
#define SYS_ICC_PPI_SACTIVER1_EL1_CRm                   13
#define SYS_ICC_PPI_SACTIVER1_EL1_Op2                   3

/* For ICC_PPI_SACTIVER1_EL1 fields see ICC_PPI_ACTIVERx_EL1 */

#define ICC_PPI_PENDRx_EL1_Pend63                       GENMASK(63, 63)
#define ICC_PPI_PENDRx_EL1_Pend63_MASK                  GENMASK(63, 63)
#define ICC_PPI_PENDRx_EL1_Pend63_SHIFT                 63
#define ICC_PPI_PENDRx_EL1_Pend63_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend62                       GENMASK(62, 62)
#define ICC_PPI_PENDRx_EL1_Pend62_MASK                  GENMASK(62, 62)
#define ICC_PPI_PENDRx_EL1_Pend62_SHIFT                 62
#define ICC_PPI_PENDRx_EL1_Pend62_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend61                       GENMASK(61, 61)
#define ICC_PPI_PENDRx_EL1_Pend61_MASK                  GENMASK(61, 61)
#define ICC_PPI_PENDRx_EL1_Pend61_SHIFT                 61
#define ICC_PPI_PENDRx_EL1_Pend61_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend60                       GENMASK(60, 60)
#define ICC_PPI_PENDRx_EL1_Pend60_MASK                  GENMASK(60, 60)
#define ICC_PPI_PENDRx_EL1_Pend60_SHIFT                 60
#define ICC_PPI_PENDRx_EL1_Pend60_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend59                       GENMASK(59, 59)
#define ICC_PPI_PENDRx_EL1_Pend59_MASK                  GENMASK(59, 59)
#define ICC_PPI_PENDRx_EL1_Pend59_SHIFT                 59
#define ICC_PPI_PENDRx_EL1_Pend59_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend58                       GENMASK(58, 58)
#define ICC_PPI_PENDRx_EL1_Pend58_MASK                  GENMASK(58, 58)
#define ICC_PPI_PENDRx_EL1_Pend58_SHIFT                 58
#define ICC_PPI_PENDRx_EL1_Pend58_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend57                       GENMASK(57, 57)
#define ICC_PPI_PENDRx_EL1_Pend57_MASK                  GENMASK(57, 57)
#define ICC_PPI_PENDRx_EL1_Pend57_SHIFT                 57
#define ICC_PPI_PENDRx_EL1_Pend57_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend56                       GENMASK(56, 56)
#define ICC_PPI_PENDRx_EL1_Pend56_MASK                  GENMASK(56, 56)
#define ICC_PPI_PENDRx_EL1_Pend56_SHIFT                 56
#define ICC_PPI_PENDRx_EL1_Pend56_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend55                       GENMASK(55, 55)
#define ICC_PPI_PENDRx_EL1_Pend55_MASK                  GENMASK(55, 55)
#define ICC_PPI_PENDRx_EL1_Pend55_SHIFT                 55
#define ICC_PPI_PENDRx_EL1_Pend55_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend54                       GENMASK(54, 54)
#define ICC_PPI_PENDRx_EL1_Pend54_MASK                  GENMASK(54, 54)
#define ICC_PPI_PENDRx_EL1_Pend54_SHIFT                 54
#define ICC_PPI_PENDRx_EL1_Pend54_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend53                       GENMASK(53, 53)
#define ICC_PPI_PENDRx_EL1_Pend53_MASK                  GENMASK(53, 53)
#define ICC_PPI_PENDRx_EL1_Pend53_SHIFT                 53
#define ICC_PPI_PENDRx_EL1_Pend53_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend52                       GENMASK(52, 52)
#define ICC_PPI_PENDRx_EL1_Pend52_MASK                  GENMASK(52, 52)
#define ICC_PPI_PENDRx_EL1_Pend52_SHIFT                 52
#define ICC_PPI_PENDRx_EL1_Pend52_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend51                       GENMASK(51, 51)
#define ICC_PPI_PENDRx_EL1_Pend51_MASK                  GENMASK(51, 51)
#define ICC_PPI_PENDRx_EL1_Pend51_SHIFT                 51
#define ICC_PPI_PENDRx_EL1_Pend51_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend50                       GENMASK(50, 50)
#define ICC_PPI_PENDRx_EL1_Pend50_MASK                  GENMASK(50, 50)
#define ICC_PPI_PENDRx_EL1_Pend50_SHIFT                 50
#define ICC_PPI_PENDRx_EL1_Pend50_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend49                       GENMASK(49, 49)
#define ICC_PPI_PENDRx_EL1_Pend49_MASK                  GENMASK(49, 49)
#define ICC_PPI_PENDRx_EL1_Pend49_SHIFT                 49
#define ICC_PPI_PENDRx_EL1_Pend49_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend48                       GENMASK(48, 48)
#define ICC_PPI_PENDRx_EL1_Pend48_MASK                  GENMASK(48, 48)
#define ICC_PPI_PENDRx_EL1_Pend48_SHIFT                 48
#define ICC_PPI_PENDRx_EL1_Pend48_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend47                       GENMASK(47, 47)
#define ICC_PPI_PENDRx_EL1_Pend47_MASK                  GENMASK(47, 47)
#define ICC_PPI_PENDRx_EL1_Pend47_SHIFT                 47
#define ICC_PPI_PENDRx_EL1_Pend47_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend46                       GENMASK(46, 46)
#define ICC_PPI_PENDRx_EL1_Pend46_MASK                  GENMASK(46, 46)
#define ICC_PPI_PENDRx_EL1_Pend46_SHIFT                 46
#define ICC_PPI_PENDRx_EL1_Pend46_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend45                       GENMASK(45, 45)
#define ICC_PPI_PENDRx_EL1_Pend45_MASK                  GENMASK(45, 45)
#define ICC_PPI_PENDRx_EL1_Pend45_SHIFT                 45
#define ICC_PPI_PENDRx_EL1_Pend45_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend44                       GENMASK(44, 44)
#define ICC_PPI_PENDRx_EL1_Pend44_MASK                  GENMASK(44, 44)
#define ICC_PPI_PENDRx_EL1_Pend44_SHIFT                 44
#define ICC_PPI_PENDRx_EL1_Pend44_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend43                       GENMASK(43, 43)
#define ICC_PPI_PENDRx_EL1_Pend43_MASK                  GENMASK(43, 43)
#define ICC_PPI_PENDRx_EL1_Pend43_SHIFT                 43
#define ICC_PPI_PENDRx_EL1_Pend43_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend42                       GENMASK(42, 42)
#define ICC_PPI_PENDRx_EL1_Pend42_MASK                  GENMASK(42, 42)
#define ICC_PPI_PENDRx_EL1_Pend42_SHIFT                 42
#define ICC_PPI_PENDRx_EL1_Pend42_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend41                       GENMASK(41, 41)
#define ICC_PPI_PENDRx_EL1_Pend41_MASK                  GENMASK(41, 41)
#define ICC_PPI_PENDRx_EL1_Pend41_SHIFT                 41
#define ICC_PPI_PENDRx_EL1_Pend41_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend40                       GENMASK(40, 40)
#define ICC_PPI_PENDRx_EL1_Pend40_MASK                  GENMASK(40, 40)
#define ICC_PPI_PENDRx_EL1_Pend40_SHIFT                 40
#define ICC_PPI_PENDRx_EL1_Pend40_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend39                       GENMASK(39, 39)
#define ICC_PPI_PENDRx_EL1_Pend39_MASK                  GENMASK(39, 39)
#define ICC_PPI_PENDRx_EL1_Pend39_SHIFT                 39
#define ICC_PPI_PENDRx_EL1_Pend39_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend38                       GENMASK(38, 38)
#define ICC_PPI_PENDRx_EL1_Pend38_MASK                  GENMASK(38, 38)
#define ICC_PPI_PENDRx_EL1_Pend38_SHIFT                 38
#define ICC_PPI_PENDRx_EL1_Pend38_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend37                       GENMASK(37, 37)
#define ICC_PPI_PENDRx_EL1_Pend37_MASK                  GENMASK(37, 37)
#define ICC_PPI_PENDRx_EL1_Pend37_SHIFT                 37
#define ICC_PPI_PENDRx_EL1_Pend37_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend36                       GENMASK(36, 36)
#define ICC_PPI_PENDRx_EL1_Pend36_MASK                  GENMASK(36, 36)
#define ICC_PPI_PENDRx_EL1_Pend36_SHIFT                 36
#define ICC_PPI_PENDRx_EL1_Pend36_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend35                       GENMASK(35, 35)
#define ICC_PPI_PENDRx_EL1_Pend35_MASK                  GENMASK(35, 35)
#define ICC_PPI_PENDRx_EL1_Pend35_SHIFT                 35
#define ICC_PPI_PENDRx_EL1_Pend35_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend34                       GENMASK(34, 34)
#define ICC_PPI_PENDRx_EL1_Pend34_MASK                  GENMASK(34, 34)
#define ICC_PPI_PENDRx_EL1_Pend34_SHIFT                 34
#define ICC_PPI_PENDRx_EL1_Pend34_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend33                       GENMASK(33, 33)
#define ICC_PPI_PENDRx_EL1_Pend33_MASK                  GENMASK(33, 33)
#define ICC_PPI_PENDRx_EL1_Pend33_SHIFT                 33
#define ICC_PPI_PENDRx_EL1_Pend33_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend32                       GENMASK(32, 32)
#define ICC_PPI_PENDRx_EL1_Pend32_MASK                  GENMASK(32, 32)
#define ICC_PPI_PENDRx_EL1_Pend32_SHIFT                 32
#define ICC_PPI_PENDRx_EL1_Pend32_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend31                       GENMASK(31, 31)
#define ICC_PPI_PENDRx_EL1_Pend31_MASK                  GENMASK(31, 31)
#define ICC_PPI_PENDRx_EL1_Pend31_SHIFT                 31
#define ICC_PPI_PENDRx_EL1_Pend31_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend30                       GENMASK(30, 30)
#define ICC_PPI_PENDRx_EL1_Pend30_MASK                  GENMASK(30, 30)
#define ICC_PPI_PENDRx_EL1_Pend30_SHIFT                 30
#define ICC_PPI_PENDRx_EL1_Pend30_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend29                       GENMASK(29, 29)
#define ICC_PPI_PENDRx_EL1_Pend29_MASK                  GENMASK(29, 29)
#define ICC_PPI_PENDRx_EL1_Pend29_SHIFT                 29
#define ICC_PPI_PENDRx_EL1_Pend29_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend28                       GENMASK(28, 28)
#define ICC_PPI_PENDRx_EL1_Pend28_MASK                  GENMASK(28, 28)
#define ICC_PPI_PENDRx_EL1_Pend28_SHIFT                 28
#define ICC_PPI_PENDRx_EL1_Pend28_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend27                       GENMASK(27, 27)
#define ICC_PPI_PENDRx_EL1_Pend27_MASK                  GENMASK(27, 27)
#define ICC_PPI_PENDRx_EL1_Pend27_SHIFT                 27
#define ICC_PPI_PENDRx_EL1_Pend27_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend26                       GENMASK(26, 26)
#define ICC_PPI_PENDRx_EL1_Pend26_MASK                  GENMASK(26, 26)
#define ICC_PPI_PENDRx_EL1_Pend26_SHIFT                 26
#define ICC_PPI_PENDRx_EL1_Pend26_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend25                       GENMASK(25, 25)
#define ICC_PPI_PENDRx_EL1_Pend25_MASK                  GENMASK(25, 25)
#define ICC_PPI_PENDRx_EL1_Pend25_SHIFT                 25
#define ICC_PPI_PENDRx_EL1_Pend25_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend24                       GENMASK(24, 24)
#define ICC_PPI_PENDRx_EL1_Pend24_MASK                  GENMASK(24, 24)
#define ICC_PPI_PENDRx_EL1_Pend24_SHIFT                 24
#define ICC_PPI_PENDRx_EL1_Pend24_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend23                       GENMASK(23, 23)
#define ICC_PPI_PENDRx_EL1_Pend23_MASK                  GENMASK(23, 23)
#define ICC_PPI_PENDRx_EL1_Pend23_SHIFT                 23
#define ICC_PPI_PENDRx_EL1_Pend23_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend22                       GENMASK(22, 22)
#define ICC_PPI_PENDRx_EL1_Pend22_MASK                  GENMASK(22, 22)
#define ICC_PPI_PENDRx_EL1_Pend22_SHIFT                 22
#define ICC_PPI_PENDRx_EL1_Pend22_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend21                       GENMASK(21, 21)
#define ICC_PPI_PENDRx_EL1_Pend21_MASK                  GENMASK(21, 21)
#define ICC_PPI_PENDRx_EL1_Pend21_SHIFT                 21
#define ICC_PPI_PENDRx_EL1_Pend21_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend20                       GENMASK(20, 20)
#define ICC_PPI_PENDRx_EL1_Pend20_MASK                  GENMASK(20, 20)
#define ICC_PPI_PENDRx_EL1_Pend20_SHIFT                 20
#define ICC_PPI_PENDRx_EL1_Pend20_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend19                       GENMASK(19, 19)
#define ICC_PPI_PENDRx_EL1_Pend19_MASK                  GENMASK(19, 19)
#define ICC_PPI_PENDRx_EL1_Pend19_SHIFT                 19
#define ICC_PPI_PENDRx_EL1_Pend19_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend18                       GENMASK(18, 18)
#define ICC_PPI_PENDRx_EL1_Pend18_MASK                  GENMASK(18, 18)
#define ICC_PPI_PENDRx_EL1_Pend18_SHIFT                 18
#define ICC_PPI_PENDRx_EL1_Pend18_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend17                       GENMASK(17, 17)
#define ICC_PPI_PENDRx_EL1_Pend17_MASK                  GENMASK(17, 17)
#define ICC_PPI_PENDRx_EL1_Pend17_SHIFT                 17
#define ICC_PPI_PENDRx_EL1_Pend17_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend16                       GENMASK(16, 16)
#define ICC_PPI_PENDRx_EL1_Pend16_MASK                  GENMASK(16, 16)
#define ICC_PPI_PENDRx_EL1_Pend16_SHIFT                 16
#define ICC_PPI_PENDRx_EL1_Pend16_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend15                       GENMASK(15, 15)
#define ICC_PPI_PENDRx_EL1_Pend15_MASK                  GENMASK(15, 15)
#define ICC_PPI_PENDRx_EL1_Pend15_SHIFT                 15
#define ICC_PPI_PENDRx_EL1_Pend15_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend14                       GENMASK(14, 14)
#define ICC_PPI_PENDRx_EL1_Pend14_MASK                  GENMASK(14, 14)
#define ICC_PPI_PENDRx_EL1_Pend14_SHIFT                 14
#define ICC_PPI_PENDRx_EL1_Pend14_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend13                       GENMASK(13, 13)
#define ICC_PPI_PENDRx_EL1_Pend13_MASK                  GENMASK(13, 13)
#define ICC_PPI_PENDRx_EL1_Pend13_SHIFT                 13
#define ICC_PPI_PENDRx_EL1_Pend13_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend12                       GENMASK(12, 12)
#define ICC_PPI_PENDRx_EL1_Pend12_MASK                  GENMASK(12, 12)
#define ICC_PPI_PENDRx_EL1_Pend12_SHIFT                 12
#define ICC_PPI_PENDRx_EL1_Pend12_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend11                       GENMASK(11, 11)
#define ICC_PPI_PENDRx_EL1_Pend11_MASK                  GENMASK(11, 11)
#define ICC_PPI_PENDRx_EL1_Pend11_SHIFT                 11
#define ICC_PPI_PENDRx_EL1_Pend11_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend10                       GENMASK(10, 10)
#define ICC_PPI_PENDRx_EL1_Pend10_MASK                  GENMASK(10, 10)
#define ICC_PPI_PENDRx_EL1_Pend10_SHIFT                 10
#define ICC_PPI_PENDRx_EL1_Pend10_WIDTH                 1

#define ICC_PPI_PENDRx_EL1_Pend9                        GENMASK(9, 9)
#define ICC_PPI_PENDRx_EL1_Pend9_MASK                   GENMASK(9, 9)
#define ICC_PPI_PENDRx_EL1_Pend9_SHIFT                  9
#define ICC_PPI_PENDRx_EL1_Pend9_WIDTH                  1

#define ICC_PPI_PENDRx_EL1_Pend8                        GENMASK(8, 8)
#define ICC_PPI_PENDRx_EL1_Pend8_MASK                   GENMASK(8, 8)
#define ICC_PPI_PENDRx_EL1_Pend8_SHIFT                  8
#define ICC_PPI_PENDRx_EL1_Pend8_WIDTH                  1

#define ICC_PPI_PENDRx_EL1_Pend7                        GENMASK(7, 7)
#define ICC_PPI_PENDRx_EL1_Pend7_MASK                   GENMASK(7, 7)
#define ICC_PPI_PENDRx_EL1_Pend7_SHIFT                  7
#define ICC_PPI_PENDRx_EL1_Pend7_WIDTH                  1

#define ICC_PPI_PENDRx_EL1_Pend6                        GENMASK(6, 6)
#define ICC_PPI_PENDRx_EL1_Pend6_MASK                   GENMASK(6, 6)
#define ICC_PPI_PENDRx_EL1_Pend6_SHIFT                  6
#define ICC_PPI_PENDRx_EL1_Pend6_WIDTH                  1

#define ICC_PPI_PENDRx_EL1_Pend5                        GENMASK(5, 5)
#define ICC_PPI_PENDRx_EL1_Pend5_MASK                   GENMASK(5, 5)
#define ICC_PPI_PENDRx_EL1_Pend5_SHIFT                  5
#define ICC_PPI_PENDRx_EL1_Pend5_WIDTH                  1

#define ICC_PPI_PENDRx_EL1_Pend4                        GENMASK(4, 4)
#define ICC_PPI_PENDRx_EL1_Pend4_MASK                   GENMASK(4, 4)
#define ICC_PPI_PENDRx_EL1_Pend4_SHIFT                  4
#define ICC_PPI_PENDRx_EL1_Pend4_WIDTH                  1

#define ICC_PPI_PENDRx_EL1_Pend3                        GENMASK(3, 3)
#define ICC_PPI_PENDRx_EL1_Pend3_MASK                   GENMASK(3, 3)
#define ICC_PPI_PENDRx_EL1_Pend3_SHIFT                  3
#define ICC_PPI_PENDRx_EL1_Pend3_WIDTH                  1

#define ICC_PPI_PENDRx_EL1_Pend2                        GENMASK(2, 2)
#define ICC_PPI_PENDRx_EL1_Pend2_MASK                   GENMASK(2, 2)
#define ICC_PPI_PENDRx_EL1_Pend2_SHIFT                  2
#define ICC_PPI_PENDRx_EL1_Pend2_WIDTH                  1

#define ICC_PPI_PENDRx_EL1_Pend1                        GENMASK(1, 1)
#define ICC_PPI_PENDRx_EL1_Pend1_MASK                   GENMASK(1, 1)
#define ICC_PPI_PENDRx_EL1_Pend1_SHIFT                  1
#define ICC_PPI_PENDRx_EL1_Pend1_WIDTH                  1

#define ICC_PPI_PENDRx_EL1_Pend0                        GENMASK(0, 0)
#define ICC_PPI_PENDRx_EL1_Pend0_MASK                   GENMASK(0, 0)
#define ICC_PPI_PENDRx_EL1_Pend0_SHIFT                  0
#define ICC_PPI_PENDRx_EL1_Pend0_WIDTH                  1

#define ICC_PPI_PENDRx_EL1_RES0                         (UL(0))
#define ICC_PPI_PENDRx_EL1_RES1                         (UL(0))
#define ICC_PPI_PENDRx_EL1_UNKN                         (UL(0))

#define REG_ICC_PPI_CPENDR0_EL1                         S3_0_C12_C13_4
#define SYS_ICC_PPI_CPENDR0_EL1                         sys_reg(3, 0, 12, 13, 4)
#define SYS_ICC_PPI_CPENDR0_EL1_Op0                     3
#define SYS_ICC_PPI_CPENDR0_EL1_Op1                     0
#define SYS_ICC_PPI_CPENDR0_EL1_CRn                     12
#define SYS_ICC_PPI_CPENDR0_EL1_CRm                     13
#define SYS_ICC_PPI_CPENDR0_EL1_Op2                     4

/* For ICC_PPI_CPENDR0_EL1 fields see ICC_PPI_PENDRx_EL1 */

#define REG_ICC_PPI_CPENDR1_EL1                         S3_0_C12_C13_5
#define SYS_ICC_PPI_CPENDR1_EL1                         sys_reg(3, 0, 12, 13, 5)
#define SYS_ICC_PPI_CPENDR1_EL1_Op0                     3
#define SYS_ICC_PPI_CPENDR1_EL1_Op1                     0
#define SYS_ICC_PPI_CPENDR1_EL1_CRn                     12
#define SYS_ICC_PPI_CPENDR1_EL1_CRm                     13
#define SYS_ICC_PPI_CPENDR1_EL1_Op2                     5

/* For ICC_PPI_CPENDR1_EL1 fields see ICC_PPI_PENDRx_EL1 */

#define REG_ICC_PPI_SPENDR0_EL1                         S3_0_C12_C13_6
#define SYS_ICC_PPI_SPENDR0_EL1                         sys_reg(3, 0, 12, 13, 6)
#define SYS_ICC_PPI_SPENDR0_EL1_Op0                     3
#define SYS_ICC_PPI_SPENDR0_EL1_Op1                     0
#define SYS_ICC_PPI_SPENDR0_EL1_CRn                     12
#define SYS_ICC_PPI_SPENDR0_EL1_CRm                     13
#define SYS_ICC_PPI_SPENDR0_EL1_Op2                     6

/* For ICC_PPI_SPENDR0_EL1 fields see ICC_PPI_PENDRx_EL1 */

#define REG_ICC_PPI_SPENDR1_EL1                         S3_0_C12_C13_7
#define SYS_ICC_PPI_SPENDR1_EL1                         sys_reg(3, 0, 12, 13, 7)
#define SYS_ICC_PPI_SPENDR1_EL1_Op0                     3
#define SYS_ICC_PPI_SPENDR1_EL1_Op1                     0
#define SYS_ICC_PPI_SPENDR1_EL1_CRn                     12
#define SYS_ICC_PPI_SPENDR1_EL1_CRm                     13
#define SYS_ICC_PPI_SPENDR1_EL1_Op2                     7

/* For ICC_PPI_SPENDR1_EL1 fields see ICC_PPI_PENDRx_EL1 */

#define ICC_PPI_PRIORITYRx_EL1_Priority7                GENMASK(60, 56)
#define ICC_PPI_PRIORITYRx_EL1_Priority7_MASK           GENMASK(60, 56)
#define ICC_PPI_PRIORITYRx_EL1_Priority7_SHIFT          56
#define ICC_PPI_PRIORITYRx_EL1_Priority7_WIDTH          5

#define ICC_PPI_PRIORITYRx_EL1_Priority6                GENMASK(52, 48)
#define ICC_PPI_PRIORITYRx_EL1_Priority6_MASK           GENMASK(52, 48)
#define ICC_PPI_PRIORITYRx_EL1_Priority6_SHIFT          48
#define ICC_PPI_PRIORITYRx_EL1_Priority6_WIDTH          5

#define ICC_PPI_PRIORITYRx_EL1_Priority5                GENMASK(44, 40)
#define ICC_PPI_PRIORITYRx_EL1_Priority5_MASK           GENMASK(44, 40)
#define ICC_PPI_PRIORITYRx_EL1_Priority5_SHIFT          40
#define ICC_PPI_PRIORITYRx_EL1_Priority5_WIDTH          5

#define ICC_PPI_PRIORITYRx_EL1_Priority4                GENMASK(36, 32)
#define ICC_PPI_PRIORITYRx_EL1_Priority4_MASK           GENMASK(36, 32)
#define ICC_PPI_PRIORITYRx_EL1_Priority4_SHIFT          32
#define ICC_PPI_PRIORITYRx_EL1_Priority4_WIDTH          5

#define ICC_PPI_PRIORITYRx_EL1_Priority3                GENMASK(28, 24)
#define ICC_PPI_PRIORITYRx_EL1_Priority3_MASK           GENMASK(28, 24)
#define ICC_PPI_PRIORITYRx_EL1_Priority3_SHIFT          24
#define ICC_PPI_PRIORITYRx_EL1_Priority3_WIDTH          5

#define ICC_PPI_PRIORITYRx_EL1_Priority2                GENMASK(20, 16)
#define ICC_PPI_PRIORITYRx_EL1_Priority2_MASK           GENMASK(20, 16)
#define ICC_PPI_PRIORITYRx_EL1_Priority2_SHIFT          16
#define ICC_PPI_PRIORITYRx_EL1_Priority2_WIDTH          5

#define ICC_PPI_PRIORITYRx_EL1_Priority1                GENMASK(12, 8)
#define ICC_PPI_PRIORITYRx_EL1_Priority1_MASK           GENMASK(12, 8)
#define ICC_PPI_PRIORITYRx_EL1_Priority1_SHIFT          8
#define ICC_PPI_PRIORITYRx_EL1_Priority1_WIDTH          5

#define ICC_PPI_PRIORITYRx_EL1_Priority0                GENMASK(4, 0)
#define ICC_PPI_PRIORITYRx_EL1_Priority0_MASK           GENMASK(4, 0)
#define ICC_PPI_PRIORITYRx_EL1_Priority0_SHIFT          0
#define ICC_PPI_PRIORITYRx_EL1_Priority0_WIDTH          5

#define ICC_PPI_PRIORITYRx_EL1_RES0                     (UL(0) | GENMASK_ULL(63, 61) | GENMASK_ULL(55, 53) | GENMASK_ULL(47, 45) | GENMASK_ULL(39, 37) | GENMASK_ULL(31, 29) | GENMASK_ULL(23, 21) | GENMASK_ULL(15, 13) | GENMASK_ULL(7, 5))
#define ICC_PPI_PRIORITYRx_EL1_RES1                     (UL(0))
#define ICC_PPI_PRIORITYRx_EL1_UNKN                     (UL(0))

#define REG_ICC_PPI_PRIORITYR0_EL1                      S3_0_C12_C14_0
#define SYS_ICC_PPI_PRIORITYR0_EL1                      sys_reg(3, 0, 12, 14, 0)
#define SYS_ICC_PPI_PRIORITYR0_EL1_Op0                  3
#define SYS_ICC_PPI_PRIORITYR0_EL1_Op1                  0
#define SYS_ICC_PPI_PRIORITYR0_EL1_CRn                  12
#define SYS_ICC_PPI_PRIORITYR0_EL1_CRm                  14
#define SYS_ICC_PPI_PRIORITYR0_EL1_Op2                  0

/* For ICC_PPI_PRIORITYR0_EL1 fields see ICC_PPI_PRIORITYRx_EL1 */

#define REG_ICC_PPI_PRIORITYR1_EL1                      S3_0_C12_C14_1
#define SYS_ICC_PPI_PRIORITYR1_EL1                      sys_reg(3, 0, 12, 14, 1)
#define SYS_ICC_PPI_PRIORITYR1_EL1_Op0                  3
#define SYS_ICC_PPI_PRIORITYR1_EL1_Op1                  0
#define SYS_ICC_PPI_PRIORITYR1_EL1_CRn                  12
#define SYS_ICC_PPI_PRIORITYR1_EL1_CRm                  14
#define SYS_ICC_PPI_PRIORITYR1_EL1_Op2                  1

/* For ICC_PPI_PRIORITYR1_EL1 fields see ICC_PPI_PRIORITYRx_EL1 */

#define REG_ICC_PPI_PRIORITYR2_EL1                      S3_0_C12_C14_2
#define SYS_ICC_PPI_PRIORITYR2_EL1                      sys_reg(3, 0, 12, 14, 2)
#define SYS_ICC_PPI_PRIORITYR2_EL1_Op0                  3
#define SYS_ICC_PPI_PRIORITYR2_EL1_Op1                  0
#define SYS_ICC_PPI_PRIORITYR2_EL1_CRn                  12
#define SYS_ICC_PPI_PRIORITYR2_EL1_CRm                  14
#define SYS_ICC_PPI_PRIORITYR2_EL1_Op2                  2

/* For ICC_PPI_PRIORITYR2_EL1 fields see ICC_PPI_PRIORITYRx_EL1 */

#define REG_ICC_PPI_PRIORITYR3_EL1                      S3_0_C12_C14_3
#define SYS_ICC_PPI_PRIORITYR3_EL1                      sys_reg(3, 0, 12, 14, 3)
#define SYS_ICC_PPI_PRIORITYR3_EL1_Op0                  3
#define SYS_ICC_PPI_PRIORITYR3_EL1_Op1                  0
#define SYS_ICC_PPI_PRIORITYR3_EL1_CRn                  12
#define SYS_ICC_PPI_PRIORITYR3_EL1_CRm                  14
#define SYS_ICC_PPI_PRIORITYR3_EL1_Op2                  3

/* For ICC_PPI_PRIORITYR3_EL1 fields see ICC_PPI_PRIORITYRx_EL1 */

#define REG_ICC_PPI_PRIORITYR4_EL1                      S3_0_C12_C14_4
#define SYS_ICC_PPI_PRIORITYR4_EL1                      sys_reg(3, 0, 12, 14, 4)
#define SYS_ICC_PPI_PRIORITYR4_EL1_Op0                  3
#define SYS_ICC_PPI_PRIORITYR4_EL1_Op1                  0
#define SYS_ICC_PPI_PRIORITYR4_EL1_CRn                  12
#define SYS_ICC_PPI_PRIORITYR4_EL1_CRm                  14
#define SYS_ICC_PPI_PRIORITYR4_EL1_Op2                  4

/* For ICC_PPI_PRIORITYR4_EL1 fields see ICC_PPI_PRIORITYRx_EL1 */

#define REG_ICC_PPI_PRIORITYR5_EL1                      S3_0_C12_C14_5
#define SYS_ICC_PPI_PRIORITYR5_EL1                      sys_reg(3, 0, 12, 14, 5)
#define SYS_ICC_PPI_PRIORITYR5_EL1_Op0                  3
#define SYS_ICC_PPI_PRIORITYR5_EL1_Op1                  0
#define SYS_ICC_PPI_PRIORITYR5_EL1_CRn                  12
#define SYS_ICC_PPI_PRIORITYR5_EL1_CRm                  14
#define SYS_ICC_PPI_PRIORITYR5_EL1_Op2                  5

/* For ICC_PPI_PRIORITYR5_EL1 fields see ICC_PPI_PRIORITYRx_EL1 */

#define REG_ICC_PPI_PRIORITYR6_EL1                      S3_0_C12_C14_6
#define SYS_ICC_PPI_PRIORITYR6_EL1                      sys_reg(3, 0, 12, 14, 6)
#define SYS_ICC_PPI_PRIORITYR6_EL1_Op0                  3
#define SYS_ICC_PPI_PRIORITYR6_EL1_Op1                  0
#define SYS_ICC_PPI_PRIORITYR6_EL1_CRn                  12
#define SYS_ICC_PPI_PRIORITYR6_EL1_CRm                  14
#define SYS_ICC_PPI_PRIORITYR6_EL1_Op2                  6

/* For ICC_PPI_PRIORITYR6_EL1 fields see ICC_PPI_PRIORITYRx_EL1 */

#define REG_ICC_PPI_PRIORITYR7_EL1                      S3_0_C12_C14_7
#define SYS_ICC_PPI_PRIORITYR7_EL1                      sys_reg(3, 0, 12, 14, 7)
#define SYS_ICC_PPI_PRIORITYR7_EL1_Op0                  3
#define SYS_ICC_PPI_PRIORITYR7_EL1_Op1                  0
#define SYS_ICC_PPI_PRIORITYR7_EL1_CRn                  12
#define SYS_ICC_PPI_PRIORITYR7_EL1_CRm                  14
#define SYS_ICC_PPI_PRIORITYR7_EL1_Op2                  7

/* For ICC_PPI_PRIORITYR7_EL1 fields see ICC_PPI_PRIORITYRx_EL1 */

#define REG_ICC_PPI_PRIORITYR8_EL1                      S3_0_C12_C15_0
#define SYS_ICC_PPI_PRIORITYR8_EL1                      sys_reg(3, 0, 12, 15, 0)
#define SYS_ICC_PPI_PRIORITYR8_EL1_Op0                  3
#define SYS_ICC_PPI_PRIORITYR8_EL1_Op1                  0
#define SYS_ICC_PPI_PRIORITYR8_EL1_CRn                  12
#define SYS_ICC_PPI_PRIORITYR8_EL1_CRm                  15
#define SYS_ICC_PPI_PRIORITYR8_EL1_Op2                  0

/* For ICC_PPI_PRIORITYR8_EL1 fields see ICC_PPI_PRIORITYRx_EL1 */

#define REG_ICC_PPI_PRIORITYR9_EL1                      S3_0_C12_C15_1
#define SYS_ICC_PPI_PRIORITYR9_EL1                      sys_reg(3, 0, 12, 15, 1)
#define SYS_ICC_PPI_PRIORITYR9_EL1_Op0                  3
#define SYS_ICC_PPI_PRIORITYR9_EL1_Op1                  0
#define SYS_ICC_PPI_PRIORITYR9_EL1_CRn                  12
#define SYS_ICC_PPI_PRIORITYR9_EL1_CRm                  15
#define SYS_ICC_PPI_PRIORITYR9_EL1_Op2                  1

/* For ICC_PPI_PRIORITYR9_EL1 fields see ICC_PPI_PRIORITYRx_EL1 */

#define REG_ICC_PPI_PRIORITYR10_EL1                     S3_0_C12_C15_2
#define SYS_ICC_PPI_PRIORITYR10_EL1                     sys_reg(3, 0, 12, 15, 2)
#define SYS_ICC_PPI_PRIORITYR10_EL1_Op0                 3
#define SYS_ICC_PPI_PRIORITYR10_EL1_Op1                 0
#define SYS_ICC_PPI_PRIORITYR10_EL1_CRn                 12
#define SYS_ICC_PPI_PRIORITYR10_EL1_CRm                 15
#define SYS_ICC_PPI_PRIORITYR10_EL1_Op2                 2

/* For ICC_PPI_PRIORITYR10_EL1 fields see ICC_PPI_PRIORITYRx_EL1 */

#define REG_ICC_PPI_PRIORITYR11_EL1                     S3_0_C12_C15_3
#define SYS_ICC_PPI_PRIORITYR11_EL1                     sys_reg(3, 0, 12, 15, 3)
#define SYS_ICC_PPI_PRIORITYR11_EL1_Op0                 3
#define SYS_ICC_PPI_PRIORITYR11_EL1_Op1                 0
#define SYS_ICC_PPI_PRIORITYR11_EL1_CRn                 12
#define SYS_ICC_PPI_PRIORITYR11_EL1_CRm                 15
#define SYS_ICC_PPI_PRIORITYR11_EL1_Op2                 3

/* For ICC_PPI_PRIORITYR11_EL1 fields see ICC_PPI_PRIORITYRx_EL1 */

#define REG_ICC_PPI_PRIORITYR12_EL1                     S3_0_C12_C15_4
#define SYS_ICC_PPI_PRIORITYR12_EL1                     sys_reg(3, 0, 12, 15, 4)
#define SYS_ICC_PPI_PRIORITYR12_EL1_Op0                 3
#define SYS_ICC_PPI_PRIORITYR12_EL1_Op1                 0
#define SYS_ICC_PPI_PRIORITYR12_EL1_CRn                 12
#define SYS_ICC_PPI_PRIORITYR12_EL1_CRm                 15
#define SYS_ICC_PPI_PRIORITYR12_EL1_Op2                 4

/* For ICC_PPI_PRIORITYR12_EL1 fields see ICC_PPI_PRIORITYRx_EL1 */

#define REG_ICC_PPI_PRIORITYR13_EL1                     S3_0_C12_C15_5
#define SYS_ICC_PPI_PRIORITYR13_EL1                     sys_reg(3, 0, 12, 15, 5)
#define SYS_ICC_PPI_PRIORITYR13_EL1_Op0                 3
#define SYS_ICC_PPI_PRIORITYR13_EL1_Op1                 0
#define SYS_ICC_PPI_PRIORITYR13_EL1_CRn                 12
#define SYS_ICC_PPI_PRIORITYR13_EL1_CRm                 15
#define SYS_ICC_PPI_PRIORITYR13_EL1_Op2                 5

/* For ICC_PPI_PRIORITYR13_EL1 fields see ICC_PPI_PRIORITYRx_EL1 */

#define REG_ICC_PPI_PRIORITYR14_EL1                     S3_0_C12_C15_6
#define SYS_ICC_PPI_PRIORITYR14_EL1                     sys_reg(3, 0, 12, 15, 6)
#define SYS_ICC_PPI_PRIORITYR14_EL1_Op0                 3
#define SYS_ICC_PPI_PRIORITYR14_EL1_Op1                 0
#define SYS_ICC_PPI_PRIORITYR14_EL1_CRn                 12
#define SYS_ICC_PPI_PRIORITYR14_EL1_CRm                 15
#define SYS_ICC_PPI_PRIORITYR14_EL1_Op2                 6

/* For ICC_PPI_PRIORITYR14_EL1 fields see ICC_PPI_PRIORITYRx_EL1 */

#define REG_ICC_PPI_PRIORITYR15_EL1                     S3_0_C12_C15_7
#define SYS_ICC_PPI_PRIORITYR15_EL1                     sys_reg(3, 0, 12, 15, 7)
#define SYS_ICC_PPI_PRIORITYR15_EL1_Op0                 3
#define SYS_ICC_PPI_PRIORITYR15_EL1_Op1                 0
#define SYS_ICC_PPI_PRIORITYR15_EL1_CRn                 12
#define SYS_ICC_PPI_PRIORITYR15_EL1_CRm                 15
#define SYS_ICC_PPI_PRIORITYR15_EL1_Op2                 7

/* For ICC_PPI_PRIORITYR15_EL1 fields see ICC_PPI_PRIORITYRx_EL1 */

#define REG_PMSELR_EL0                                  S3_3_C9_C12_5
#define SYS_PMSELR_EL0                                  sys_reg(3, 3, 9, 12, 5)
#define SYS_PMSELR_EL0_Op0                              3
#define SYS_PMSELR_EL0_Op1                              3
#define SYS_PMSELR_EL0_CRn                              9
#define SYS_PMSELR_EL0_CRm                              12
#define SYS_PMSELR_EL0_Op2                              5

#define PMSELR_EL0_SEL                                  GENMASK(4, 0)
#define PMSELR_EL0_SEL_MASK                             GENMASK(4, 0)
#define PMSELR_EL0_SEL_SHIFT                            0
#define PMSELR_EL0_SEL_WIDTH                            5

#define PMSELR_EL0_RES0                                 (UL(0) | GENMASK_ULL(63, 5))
#define PMSELR_EL0_RES1                                 (UL(0))
#define PMSELR_EL0_UNKN                                 (UL(0))

#define REG_PMZR_EL0                                    S3_3_C9_C13_4
#define SYS_PMZR_EL0                                    sys_reg(3, 3, 9, 13, 4)
#define SYS_PMZR_EL0_Op0                                3
#define SYS_PMZR_EL0_Op1                                3
#define SYS_PMZR_EL0_CRn                                9
#define SYS_PMZR_EL0_CRm                                13
#define SYS_PMZR_EL0_Op2                                4

#define PMZR_EL0_F0                                     GENMASK(32, 32)
#define PMZR_EL0_F0_MASK                                GENMASK(32, 32)
#define PMZR_EL0_F0_SHIFT                               32
#define PMZR_EL0_F0_WIDTH                               1

#define PMZR_EL0_C                                      GENMASK(31, 31)
#define PMZR_EL0_C_MASK                                 GENMASK(31, 31)
#define PMZR_EL0_C_SHIFT                                31
#define PMZR_EL0_C_WIDTH                                1

#define PMZR_EL0_P                                      GENMASK(30, 0)
#define PMZR_EL0_P_MASK                                 GENMASK(30, 0)
#define PMZR_EL0_P_SHIFT                                0
#define PMZR_EL0_P_WIDTH                                31

#define PMZR_EL0_RES0                                   (UL(0) | GENMASK_ULL(63, 33))
#define PMZR_EL0_RES1                                   (UL(0))
#define PMZR_EL0_UNKN                                   (UL(0))

#define CONTEXTIDR_ELx_PROCID                           GENMASK(31, 0)
#define CONTEXTIDR_ELx_PROCID_MASK                      GENMASK(31, 0)
#define CONTEXTIDR_ELx_PROCID_SHIFT                     0
#define CONTEXTIDR_ELx_PROCID_WIDTH                     32

#define CONTEXTIDR_ELx_RES0                             (UL(0) | GENMASK_ULL(63, 32))
#define CONTEXTIDR_ELx_RES1                             (UL(0))
#define CONTEXTIDR_ELx_UNKN                             (UL(0))

#define REG_CONTEXTIDR_EL1                              S3_0_C13_C0_1
#define SYS_CONTEXTIDR_EL1                              sys_reg(3, 0, 13, 0, 1)
#define SYS_CONTEXTIDR_EL1_Op0                          3
#define SYS_CONTEXTIDR_EL1_Op1                          0
#define SYS_CONTEXTIDR_EL1_CRn                          13
#define SYS_CONTEXTIDR_EL1_CRm                          0
#define SYS_CONTEXTIDR_EL1_Op2                          1

/* For CONTEXTIDR_EL1 fields see CONTEXTIDR_ELx */

#define REG_RCWSMASK_EL1                                S3_0_C13_C0_3
#define SYS_RCWSMASK_EL1                                sys_reg(3, 0, 13, 0, 3)
#define SYS_RCWSMASK_EL1_Op0                            3
#define SYS_RCWSMASK_EL1_Op1                            0
#define SYS_RCWSMASK_EL1_CRn                            13
#define SYS_RCWSMASK_EL1_CRm                            0
#define SYS_RCWSMASK_EL1_Op2                            3

#define RCWSMASK_EL1_RCWSMASK                           GENMASK(63, 0)
#define RCWSMASK_EL1_RCWSMASK_MASK                      GENMASK(63, 0)
#define RCWSMASK_EL1_RCWSMASK_SHIFT                     0
#define RCWSMASK_EL1_RCWSMASK_WIDTH                     64

#define RCWSMASK_EL1_RES0                               (UL(0))
#define RCWSMASK_EL1_RES1                               (UL(0))
#define RCWSMASK_EL1_UNKN                               (UL(0))

#define REG_TPIDR_EL1                                   S3_0_C13_C0_4
#define SYS_TPIDR_EL1                                   sys_reg(3, 0, 13, 0, 4)
#define SYS_TPIDR_EL1_Op0                               3
#define SYS_TPIDR_EL1_Op1                               0
#define SYS_TPIDR_EL1_CRn                               13
#define SYS_TPIDR_EL1_CRm                               0
#define SYS_TPIDR_EL1_Op2                               4

#define TPIDR_EL1_ThreadID                              GENMASK(63, 0)
#define TPIDR_EL1_ThreadID_MASK                         GENMASK(63, 0)
#define TPIDR_EL1_ThreadID_SHIFT                        0
#define TPIDR_EL1_ThreadID_WIDTH                        64

#define TPIDR_EL1_RES0                                  (UL(0))
#define TPIDR_EL1_RES1                                  (UL(0))
#define TPIDR_EL1_UNKN                                  (UL(0))

#define REG_RCWMASK_EL1                                 S3_0_C13_C0_6
#define SYS_RCWMASK_EL1                                 sys_reg(3, 0, 13, 0, 6)
#define SYS_RCWMASK_EL1_Op0                             3
#define SYS_RCWMASK_EL1_Op1                             0
#define SYS_RCWMASK_EL1_CRn                             13
#define SYS_RCWMASK_EL1_CRm                             0
#define SYS_RCWMASK_EL1_Op2                             6

#define RCWMASK_EL1_RCWMASK                             GENMASK(63, 0)
#define RCWMASK_EL1_RCWMASK_MASK                        GENMASK(63, 0)
#define RCWMASK_EL1_RCWMASK_SHIFT                       0
#define RCWMASK_EL1_RCWMASK_WIDTH                       64

#define RCWMASK_EL1_RES0                                (UL(0))
#define RCWMASK_EL1_RES1                                (UL(0))
#define RCWMASK_EL1_UNKN                                (UL(0))

#define REG_SCXTNUM_EL1                                 S3_0_C13_C0_7
#define SYS_SCXTNUM_EL1                                 sys_reg(3, 0, 13, 0, 7)
#define SYS_SCXTNUM_EL1_Op0                             3
#define SYS_SCXTNUM_EL1_Op1                             0
#define SYS_SCXTNUM_EL1_CRn                             13
#define SYS_SCXTNUM_EL1_CRm                             0
#define SYS_SCXTNUM_EL1_Op2                             7

#define SCXTNUM_EL1_SoftwareContextNumber               GENMASK(63, 0)
#define SCXTNUM_EL1_SoftwareContextNumber_MASK          GENMASK(63, 0)
#define SCXTNUM_EL1_SoftwareContextNumber_SHIFT         0
#define SCXTNUM_EL1_SoftwareContextNumber_WIDTH         64

#define SCXTNUM_EL1_RES0                                (UL(0))
#define SCXTNUM_EL1_RES1                                (UL(0))
#define SCXTNUM_EL1_UNKN                                (UL(0))

#define REG_CCSIDR_EL1                                  S3_1_C0_C0_0
#define SYS_CCSIDR_EL1                                  sys_reg(3, 1, 0, 0, 0)
#define SYS_CCSIDR_EL1_Op0                              3
#define SYS_CCSIDR_EL1_Op1                              1
#define SYS_CCSIDR_EL1_CRn                              0
#define SYS_CCSIDR_EL1_CRm                              0
#define SYS_CCSIDR_EL1_Op2                              0

#define CCSIDR_EL1_NumSets                              GENMASK(27, 13)
#define CCSIDR_EL1_NumSets_MASK                         GENMASK(27, 13)
#define CCSIDR_EL1_NumSets_SHIFT                        13
#define CCSIDR_EL1_NumSets_WIDTH                        15

#define CCSIDR_EL1_Associativity                        GENMASK(12, 3)
#define CCSIDR_EL1_Associativity_MASK                   GENMASK(12, 3)
#define CCSIDR_EL1_Associativity_SHIFT                  3
#define CCSIDR_EL1_Associativity_WIDTH                  10

#define CCSIDR_EL1_LineSize                             GENMASK(2, 0)
#define CCSIDR_EL1_LineSize_MASK                        GENMASK(2, 0)
#define CCSIDR_EL1_LineSize_SHIFT                       0
#define CCSIDR_EL1_LineSize_WIDTH                       3

#define CCSIDR_EL1_RES0                                 (UL(0) | GENMASK_ULL(63, 32))
#define CCSIDR_EL1_RES1                                 (UL(0))
#define CCSIDR_EL1_UNKN                                 (UL(0) | GENMASK_ULL(31, 28))

#define REG_CLIDR_EL1                                   S3_1_C0_C0_1
#define SYS_CLIDR_EL1                                   sys_reg(3, 1, 0, 0, 1)
#define SYS_CLIDR_EL1_Op0                               3
#define SYS_CLIDR_EL1_Op1                               1
#define SYS_CLIDR_EL1_CRn                               0
#define SYS_CLIDR_EL1_CRm                               0
#define SYS_CLIDR_EL1_Op2                               1

#define CLIDR_EL1_Ttypen                                GENMASK(46, 33)
#define CLIDR_EL1_Ttypen_MASK                           GENMASK(46, 33)
#define CLIDR_EL1_Ttypen_SHIFT                          33
#define CLIDR_EL1_Ttypen_WIDTH                          14

#define CLIDR_EL1_ICB                                   GENMASK(32, 30)
#define CLIDR_EL1_ICB_MASK                              GENMASK(32, 30)
#define CLIDR_EL1_ICB_SHIFT                             30
#define CLIDR_EL1_ICB_WIDTH                             3

#define CLIDR_EL1_LoUU                                  GENMASK(29, 27)
#define CLIDR_EL1_LoUU_MASK                             GENMASK(29, 27)
#define CLIDR_EL1_LoUU_SHIFT                            27
#define CLIDR_EL1_LoUU_WIDTH                            3

#define CLIDR_EL1_LoC                                   GENMASK(26, 24)
#define CLIDR_EL1_LoC_MASK                              GENMASK(26, 24)
#define CLIDR_EL1_LoC_SHIFT                             24
#define CLIDR_EL1_LoC_WIDTH                             3

#define CLIDR_EL1_LoUIS                                 GENMASK(23, 21)
#define CLIDR_EL1_LoUIS_MASK                            GENMASK(23, 21)
#define CLIDR_EL1_LoUIS_SHIFT                           21
#define CLIDR_EL1_LoUIS_WIDTH                           3

#define CLIDR_EL1_Ctype7                                GENMASK(20, 18)
#define CLIDR_EL1_Ctype7_MASK                           GENMASK(20, 18)
#define CLIDR_EL1_Ctype7_SHIFT                          18
#define CLIDR_EL1_Ctype7_WIDTH                          3

#define CLIDR_EL1_Ctype6                                GENMASK(17, 15)
#define CLIDR_EL1_Ctype6_MASK                           GENMASK(17, 15)
#define CLIDR_EL1_Ctype6_SHIFT                          15
#define CLIDR_EL1_Ctype6_WIDTH                          3

#define CLIDR_EL1_Ctype5                                GENMASK(14, 12)
#define CLIDR_EL1_Ctype5_MASK                           GENMASK(14, 12)
#define CLIDR_EL1_Ctype5_SHIFT                          12
#define CLIDR_EL1_Ctype5_WIDTH                          3

#define CLIDR_EL1_Ctype4                                GENMASK(11, 9)
#define CLIDR_EL1_Ctype4_MASK                           GENMASK(11, 9)
#define CLIDR_EL1_Ctype4_SHIFT                          9
#define CLIDR_EL1_Ctype4_WIDTH                          3

#define CLIDR_EL1_Ctype3                                GENMASK(8, 6)
#define CLIDR_EL1_Ctype3_MASK                           GENMASK(8, 6)
#define CLIDR_EL1_Ctype3_SHIFT                          6
#define CLIDR_EL1_Ctype3_WIDTH                          3

#define CLIDR_EL1_Ctype2                                GENMASK(5, 3)
#define CLIDR_EL1_Ctype2_MASK                           GENMASK(5, 3)
#define CLIDR_EL1_Ctype2_SHIFT                          3
#define CLIDR_EL1_Ctype2_WIDTH                          3

#define CLIDR_EL1_Ctype1                                GENMASK(2, 0)
#define CLIDR_EL1_Ctype1_MASK                           GENMASK(2, 0)
#define CLIDR_EL1_Ctype1_SHIFT                          0
#define CLIDR_EL1_Ctype1_WIDTH                          3

#define CLIDR_EL1_RES0                                  (UL(0) | GENMASK_ULL(63, 47))
#define CLIDR_EL1_RES1                                  (UL(0))
#define CLIDR_EL1_UNKN                                  (UL(0))

#define REG_CCSIDR2_EL1                                 S3_1_C0_C0_2
#define SYS_CCSIDR2_EL1                                 sys_reg(3, 1, 0, 0, 2)
#define SYS_CCSIDR2_EL1_Op0                             3
#define SYS_CCSIDR2_EL1_Op1                             1
#define SYS_CCSIDR2_EL1_CRn                             0
#define SYS_CCSIDR2_EL1_CRm                             0
#define SYS_CCSIDR2_EL1_Op2                             2

#define CCSIDR2_EL1_NumSets                             GENMASK(23, 0)
#define CCSIDR2_EL1_NumSets_MASK                        GENMASK(23, 0)
#define CCSIDR2_EL1_NumSets_SHIFT                       0
#define CCSIDR2_EL1_NumSets_WIDTH                       24

#define CCSIDR2_EL1_RES0                                (UL(0) | GENMASK_ULL(63, 24))
#define CCSIDR2_EL1_RES1                                (UL(0))
#define CCSIDR2_EL1_UNKN                                (UL(0))

#define REG_GMID_EL1                                    S3_1_C0_C0_4
#define SYS_GMID_EL1                                    sys_reg(3, 1, 0, 0, 4)
#define SYS_GMID_EL1_Op0                                3
#define SYS_GMID_EL1_Op1                                1
#define SYS_GMID_EL1_CRn                                0
#define SYS_GMID_EL1_CRm                                0
#define SYS_GMID_EL1_Op2                                4

#define GMID_EL1_BS                                     GENMASK(3, 0)
#define GMID_EL1_BS_MASK                                GENMASK(3, 0)
#define GMID_EL1_BS_SHIFT                               0
#define GMID_EL1_BS_WIDTH                               4

#define GMID_EL1_RES0                                   (UL(0) | GENMASK_ULL(63, 4))
#define GMID_EL1_RES1                                   (UL(0))
#define GMID_EL1_UNKN                                   (UL(0))

#define REG_SMIDR_EL1                                   S3_1_C0_C0_6
#define SYS_SMIDR_EL1                                   sys_reg(3, 1, 0, 0, 6)
#define SYS_SMIDR_EL1_Op0                               3
#define SYS_SMIDR_EL1_Op1                               1
#define SYS_SMIDR_EL1_CRn                               0
#define SYS_SMIDR_EL1_CRm                               0
#define SYS_SMIDR_EL1_Op2                               6

#define SMIDR_EL1_IMPLEMENTER                           GENMASK(31, 24)
#define SMIDR_EL1_IMPLEMENTER_MASK                      GENMASK(31, 24)
#define SMIDR_EL1_IMPLEMENTER_SHIFT                     24
#define SMIDR_EL1_IMPLEMENTER_WIDTH                     8

#define SMIDR_EL1_REVISION                              GENMASK(23, 16)
#define SMIDR_EL1_REVISION_MASK                         GENMASK(23, 16)
#define SMIDR_EL1_REVISION_SHIFT                        16
#define SMIDR_EL1_REVISION_WIDTH                        8

#define SMIDR_EL1_SMPS                                  GENMASK(15, 15)
#define SMIDR_EL1_SMPS_MASK                             GENMASK(15, 15)
#define SMIDR_EL1_SMPS_SHIFT                            15
#define SMIDR_EL1_SMPS_WIDTH                            1

#define SMIDR_EL1_AFFINITY                              GENMASK(11, 0)
#define SMIDR_EL1_AFFINITY_MASK                         GENMASK(11, 0)
#define SMIDR_EL1_AFFINITY_SHIFT                        0
#define SMIDR_EL1_AFFINITY_WIDTH                        12

#define SMIDR_EL1_RES0                                  (UL(0) | GENMASK_ULL(63, 32) | GENMASK_ULL(14, 12))
#define SMIDR_EL1_RES1                                  (UL(0))
#define SMIDR_EL1_UNKN                                  (UL(0))

#define REG_ICC_CR0_EL1                                 S3_1_C12_C0_1
#define SYS_ICC_CR0_EL1                                 sys_reg(3, 1, 12, 0, 1)
#define SYS_ICC_CR0_EL1_Op0                             3
#define SYS_ICC_CR0_EL1_Op1                             1
#define SYS_ICC_CR0_EL1_CRn                             12
#define SYS_ICC_CR0_EL1_CRm                             0
#define SYS_ICC_CR0_EL1_Op2                             1

#define ICC_CR0_EL1_PID                                 GENMASK(38, 38)
#define ICC_CR0_EL1_PID_MASK                            GENMASK(38, 38)
#define ICC_CR0_EL1_PID_SHIFT                           38
#define ICC_CR0_EL1_PID_WIDTH                           1

#define ICC_CR0_EL1_IPPT                                GENMASK(37, 32)
#define ICC_CR0_EL1_IPPT_MASK                           GENMASK(37, 32)
#define ICC_CR0_EL1_IPPT_SHIFT                          32
#define ICC_CR0_EL1_IPPT_WIDTH                          6

#define ICC_CR0_EL1_EN                                  GENMASK(0, 0)
#define ICC_CR0_EL1_EN_MASK                             GENMASK(0, 0)
#define ICC_CR0_EL1_EN_SHIFT                            0
#define ICC_CR0_EL1_EN_WIDTH                            1

#define ICC_CR0_EL1_RES0                                (UL(0) | GENMASK_ULL(63, 39) | GENMASK_ULL(31, 1))
#define ICC_CR0_EL1_RES1                                (UL(0))
#define ICC_CR0_EL1_UNKN                                (UL(0))

#define REG_ICC_PCR_EL1                                 S3_1_C12_C0_2
#define SYS_ICC_PCR_EL1                                 sys_reg(3, 1, 12, 0, 2)
#define SYS_ICC_PCR_EL1_Op0                             3
#define SYS_ICC_PCR_EL1_Op1                             1
#define SYS_ICC_PCR_EL1_CRn                             12
#define SYS_ICC_PCR_EL1_CRm                             0
#define SYS_ICC_PCR_EL1_Op2                             2

#define ICC_PCR_EL1_PRIORITY                            GENMASK(4, 0)
#define ICC_PCR_EL1_PRIORITY_MASK                       GENMASK(4, 0)
#define ICC_PCR_EL1_PRIORITY_SHIFT                      0
#define ICC_PCR_EL1_PRIORITY_WIDTH                      5

#define ICC_PCR_EL1_RES0                                (UL(0) | GENMASK_ULL(63, 5))
#define ICC_PCR_EL1_RES1                                (UL(0))
#define ICC_PCR_EL1_UNKN                                (UL(0))

#define REG_CSSELR_EL1                                  S3_2_C0_C0_0
#define SYS_CSSELR_EL1                                  sys_reg(3, 2, 0, 0, 0)
#define SYS_CSSELR_EL1_Op0                              3
#define SYS_CSSELR_EL1_Op1                              2
#define SYS_CSSELR_EL1_CRn                              0
#define SYS_CSSELR_EL1_CRm                              0
#define SYS_CSSELR_EL1_Op2                              0

#define CSSELR_EL1_TnD                                  GENMASK(4, 4)
#define CSSELR_EL1_TnD_MASK                             GENMASK(4, 4)
#define CSSELR_EL1_TnD_SHIFT                            4
#define CSSELR_EL1_TnD_WIDTH                            1

#define CSSELR_EL1_Level                                GENMASK(3, 1)
#define CSSELR_EL1_Level_MASK                           GENMASK(3, 1)
#define CSSELR_EL1_Level_SHIFT                          1
#define CSSELR_EL1_Level_WIDTH                          3

#define CSSELR_EL1_InD                                  GENMASK(0, 0)
#define CSSELR_EL1_InD_MASK                             GENMASK(0, 0)
#define CSSELR_EL1_InD_SHIFT                            0
#define CSSELR_EL1_InD_WIDTH                            1

#define CSSELR_EL1_RES0                                 (UL(0) | GENMASK_ULL(63, 5))
#define CSSELR_EL1_RES1                                 (UL(0))
#define CSSELR_EL1_UNKN                                 (UL(0))

#define REG_CTR_EL0                                     S3_3_C0_C0_1
#define SYS_CTR_EL0                                     sys_reg(3, 3, 0, 0, 1)
#define SYS_CTR_EL0_Op0                                 3
#define SYS_CTR_EL0_Op1                                 3
#define SYS_CTR_EL0_CRn                                 0
#define SYS_CTR_EL0_CRm                                 0
#define SYS_CTR_EL0_Op2                                 1

#define CTR_EL0_TminLine                                GENMASK(37, 32)
#define CTR_EL0_TminLine_MASK                           GENMASK(37, 32)
#define CTR_EL0_TminLine_SHIFT                          32
#define CTR_EL0_TminLine_WIDTH                          6

#define CTR_EL0_DIC                                     GENMASK(29, 29)
#define CTR_EL0_DIC_MASK                                GENMASK(29, 29)
#define CTR_EL0_DIC_SHIFT                               29
#define CTR_EL0_DIC_WIDTH                               1

#define CTR_EL0_IDC                                     GENMASK(28, 28)
#define CTR_EL0_IDC_MASK                                GENMASK(28, 28)
#define CTR_EL0_IDC_SHIFT                               28
#define CTR_EL0_IDC_WIDTH                               1

#define CTR_EL0_CWG                                     GENMASK(27, 24)
#define CTR_EL0_CWG_MASK                                GENMASK(27, 24)
#define CTR_EL0_CWG_SHIFT                               24
#define CTR_EL0_CWG_WIDTH                               4

#define CTR_EL0_ERG                                     GENMASK(23, 20)
#define CTR_EL0_ERG_MASK                                GENMASK(23, 20)
#define CTR_EL0_ERG_SHIFT                               20
#define CTR_EL0_ERG_WIDTH                               4

#define CTR_EL0_DminLine                                GENMASK(19, 16)
#define CTR_EL0_DminLine_MASK                           GENMASK(19, 16)
#define CTR_EL0_DminLine_SHIFT                          16
#define CTR_EL0_DminLine_WIDTH                          4

#define CTR_EL0_L1Ip                                    GENMASK(15, 14)
#define CTR_EL0_L1Ip_MASK                               GENMASK(15, 14)
#define CTR_EL0_L1Ip_SHIFT                              14
#define CTR_EL0_L1Ip_WIDTH                              2
#define CTR_EL0_L1Ip_RESERVED_VPIPT                     UL(0b00)
#define CTR_EL0_L1Ip_RESERVED_AIVIVT                    UL(0b01)
#define CTR_EL0_L1Ip_VIPT                               UL(0b10)
#define CTR_EL0_L1Ip_PIPT                               UL(0b11)

#define CTR_EL0_IminLine                                GENMASK(3, 0)
#define CTR_EL0_IminLine_MASK                           GENMASK(3, 0)
#define CTR_EL0_IminLine_SHIFT                          0
#define CTR_EL0_IminLine_WIDTH                          4

#define CTR_EL0_RES0                                    (UL(0) | GENMASK_ULL(63, 38) | GENMASK_ULL(30, 30) | GENMASK_ULL(13, 4))
#define CTR_EL0_RES1                                    (UL(0) | GENMASK_ULL(31, 31))
#define CTR_EL0_UNKN                                    (UL(0))

#define REG_DCZID_EL0                                   S3_3_C0_C0_7
#define SYS_DCZID_EL0                                   sys_reg(3, 3, 0, 0, 7)
#define SYS_DCZID_EL0_Op0                               3
#define SYS_DCZID_EL0_Op1                               3
#define SYS_DCZID_EL0_CRn                               0
#define SYS_DCZID_EL0_CRm                               0
#define SYS_DCZID_EL0_Op2                               7

#define DCZID_EL0_DZP                                   GENMASK(4, 4)
#define DCZID_EL0_DZP_MASK                              GENMASK(4, 4)
#define DCZID_EL0_DZP_SHIFT                             4
#define DCZID_EL0_DZP_WIDTH                             1

#define DCZID_EL0_BS                                    GENMASK(3, 0)
#define DCZID_EL0_BS_MASK                               GENMASK(3, 0)
#define DCZID_EL0_BS_SHIFT                              0
#define DCZID_EL0_BS_WIDTH                              4

#define DCZID_EL0_RES0                                  (UL(0) | GENMASK_ULL(63, 5))
#define DCZID_EL0_RES1                                  (UL(0))
#define DCZID_EL0_UNKN                                  (UL(0))

#define REG_GCSPR_EL0                                   S3_3_C2_C5_1
#define SYS_GCSPR_EL0                                   sys_reg(3, 3, 2, 5, 1)
#define SYS_GCSPR_EL0_Op0                               3
#define SYS_GCSPR_EL0_Op1                               3
#define SYS_GCSPR_EL0_CRn                               2
#define SYS_GCSPR_EL0_CRm                               5
#define SYS_GCSPR_EL0_Op2                               1

/* For GCSPR_EL0 fields see GCSPR_ELx */

#define REG_SVCR                                        S3_3_C4_C2_2
#define SYS_SVCR                                        sys_reg(3, 3, 4, 2, 2)
#define SYS_SVCR_Op0                                    3
#define SYS_SVCR_Op1                                    3
#define SYS_SVCR_CRn                                    4
#define SYS_SVCR_CRm                                    2
#define SYS_SVCR_Op2                                    2

#define SVCR_ZA                                         GENMASK(1, 1)
#define SVCR_ZA_MASK                                    GENMASK(1, 1)
#define SVCR_ZA_SHIFT                                   1
#define SVCR_ZA_WIDTH                                   1

#define SVCR_SM                                         GENMASK(0, 0)
#define SVCR_SM_MASK                                    GENMASK(0, 0)
#define SVCR_SM_SHIFT                                   0
#define SVCR_SM_WIDTH                                   1

#define SVCR_RES0                                       (UL(0) | GENMASK_ULL(63, 2))
#define SVCR_RES1                                       (UL(0))
#define SVCR_UNKN                                       (UL(0))

#define REG_FPMR                                        S3_3_C4_C4_2
#define SYS_FPMR                                        sys_reg(3, 3, 4, 4, 2)
#define SYS_FPMR_Op0                                    3
#define SYS_FPMR_Op1                                    3
#define SYS_FPMR_CRn                                    4
#define SYS_FPMR_CRm                                    4
#define SYS_FPMR_Op2                                    2

#define FPMR_LSCALE2                                    GENMASK(37, 32)
#define FPMR_LSCALE2_MASK                               GENMASK(37, 32)
#define FPMR_LSCALE2_SHIFT                              32
#define FPMR_LSCALE2_WIDTH                              6

#define FPMR_NSCALE                                     GENMASK(31, 24)
#define FPMR_NSCALE_MASK                                GENMASK(31, 24)
#define FPMR_NSCALE_SHIFT                               24
#define FPMR_NSCALE_WIDTH                               8

#define FPMR_LSCALE                                     GENMASK(22, 16)
#define FPMR_LSCALE_MASK                                GENMASK(22, 16)
#define FPMR_LSCALE_SHIFT                               16
#define FPMR_LSCALE_WIDTH                               7

#define FPMR_OSC                                        GENMASK(15, 15)
#define FPMR_OSC_MASK                                   GENMASK(15, 15)
#define FPMR_OSC_SHIFT                                  15
#define FPMR_OSC_WIDTH                                  1

#define FPMR_OSM                                        GENMASK(14, 14)
#define FPMR_OSM_MASK                                   GENMASK(14, 14)
#define FPMR_OSM_SHIFT                                  14
#define FPMR_OSM_WIDTH                                  1

#define FPMR_F8D                                        GENMASK(8, 6)
#define FPMR_F8D_MASK                                   GENMASK(8, 6)
#define FPMR_F8D_SHIFT                                  6
#define FPMR_F8D_WIDTH                                  3
#define FPMR_F8D_SIGNED                                 false
#define FPMR_F8D_E5M2                                   UL(0b000)
#define FPMR_F8D_E4M3                                   UL(0b001)

#define FPMR_F8S2                                       GENMASK(5, 3)
#define FPMR_F8S2_MASK                                  GENMASK(5, 3)
#define FPMR_F8S2_SHIFT                                 3
#define FPMR_F8S2_WIDTH                                 3
#define FPMR_F8S2_SIGNED                                false
#define FPMR_F8S2_E5M2                                  UL(0b000)
#define FPMR_F8S2_E4M3                                  UL(0b001)

#define FPMR_F8S1                                       GENMASK(2, 0)
#define FPMR_F8S1_MASK                                  GENMASK(2, 0)
#define FPMR_F8S1_SHIFT                                 0
#define FPMR_F8S1_WIDTH                                 3
#define FPMR_F8S1_SIGNED                                false
#define FPMR_F8S1_E5M2                                  UL(0b000)
#define FPMR_F8S1_E4M3                                  UL(0b001)

#define FPMR_RES0                                       (UL(0) | GENMASK_ULL(63, 38) | GENMASK_ULL(23, 23) | GENMASK_ULL(13, 9))
#define FPMR_RES1                                       (UL(0))
#define FPMR_UNKN                                       (UL(0))

#define REG_HCR_EL2                                     S3_4_C1_C1_0
#define SYS_HCR_EL2                                     sys_reg(3, 4, 1, 1, 0)
#define SYS_HCR_EL2_Op0                                 3
#define SYS_HCR_EL2_Op1                                 4
#define SYS_HCR_EL2_CRn                                 1
#define SYS_HCR_EL2_CRm                                 1
#define SYS_HCR_EL2_Op2                                 0

#define HCR_EL2_TWEDEL                                  GENMASK(63, 60)
#define HCR_EL2_TWEDEL_MASK                             GENMASK(63, 60)
#define HCR_EL2_TWEDEL_SHIFT                            60
#define HCR_EL2_TWEDEL_WIDTH                            4

#define HCR_EL2_TWEDEn                                  GENMASK(59, 59)
#define HCR_EL2_TWEDEn_MASK                             GENMASK(59, 59)
#define HCR_EL2_TWEDEn_SHIFT                            59
#define HCR_EL2_TWEDEn_WIDTH                            1

#define HCR_EL2_TID5                                    GENMASK(58, 58)
#define HCR_EL2_TID5_MASK                               GENMASK(58, 58)
#define HCR_EL2_TID5_SHIFT                              58
#define HCR_EL2_TID5_WIDTH                              1

#define HCR_EL2_DCT                                     GENMASK(57, 57)
#define HCR_EL2_DCT_MASK                                GENMASK(57, 57)
#define HCR_EL2_DCT_SHIFT                               57
#define HCR_EL2_DCT_WIDTH                               1

#define HCR_EL2_ATA                                     GENMASK(56, 56)
#define HCR_EL2_ATA_MASK                                GENMASK(56, 56)
#define HCR_EL2_ATA_SHIFT                               56
#define HCR_EL2_ATA_WIDTH                               1

#define HCR_EL2_TTLBOS                                  GENMASK(55, 55)
#define HCR_EL2_TTLBOS_MASK                             GENMASK(55, 55)
#define HCR_EL2_TTLBOS_SHIFT                            55
#define HCR_EL2_TTLBOS_WIDTH                            1

#define HCR_EL2_TTLBIS                                  GENMASK(54, 54)
#define HCR_EL2_TTLBIS_MASK                             GENMASK(54, 54)
#define HCR_EL2_TTLBIS_SHIFT                            54
#define HCR_EL2_TTLBIS_WIDTH                            1

#define HCR_EL2_EnSCXT                                  GENMASK(53, 53)
#define HCR_EL2_EnSCXT_MASK                             GENMASK(53, 53)
#define HCR_EL2_EnSCXT_SHIFT                            53
#define HCR_EL2_EnSCXT_WIDTH                            1

#define HCR_EL2_TOCU                                    GENMASK(52, 52)
#define HCR_EL2_TOCU_MASK                               GENMASK(52, 52)
#define HCR_EL2_TOCU_SHIFT                              52
#define HCR_EL2_TOCU_WIDTH                              1

#define HCR_EL2_AMVOFFEN                                GENMASK(51, 51)
#define HCR_EL2_AMVOFFEN_MASK                           GENMASK(51, 51)
#define HCR_EL2_AMVOFFEN_SHIFT                          51
#define HCR_EL2_AMVOFFEN_WIDTH                          1

#define HCR_EL2_TICAB                                   GENMASK(50, 50)
#define HCR_EL2_TICAB_MASK                              GENMASK(50, 50)
#define HCR_EL2_TICAB_SHIFT                             50
#define HCR_EL2_TICAB_WIDTH                             1

#define HCR_EL2_TID4                                    GENMASK(49, 49)
#define HCR_EL2_TID4_MASK                               GENMASK(49, 49)
#define HCR_EL2_TID4_SHIFT                              49
#define HCR_EL2_TID4_WIDTH                              1

#define HCR_EL2_GPF                                     GENMASK(48, 48)
#define HCR_EL2_GPF_MASK                                GENMASK(48, 48)
#define HCR_EL2_GPF_SHIFT                               48
#define HCR_EL2_GPF_WIDTH                               1

#define HCR_EL2_FIEN                                    GENMASK(47, 47)
#define HCR_EL2_FIEN_MASK                               GENMASK(47, 47)
#define HCR_EL2_FIEN_SHIFT                              47
#define HCR_EL2_FIEN_WIDTH                              1

#define HCR_EL2_FWB                                     GENMASK(46, 46)
#define HCR_EL2_FWB_MASK                                GENMASK(46, 46)
#define HCR_EL2_FWB_SHIFT                               46
#define HCR_EL2_FWB_WIDTH                               1

#define HCR_EL2_NV2                                     GENMASK(45, 45)
#define HCR_EL2_NV2_MASK                                GENMASK(45, 45)
#define HCR_EL2_NV2_SHIFT                               45
#define HCR_EL2_NV2_WIDTH                               1

#define HCR_EL2_AT                                      GENMASK(44, 44)
#define HCR_EL2_AT_MASK                                 GENMASK(44, 44)
#define HCR_EL2_AT_SHIFT                                44
#define HCR_EL2_AT_WIDTH                                1

#define HCR_EL2_NV1                                     GENMASK(43, 43)
#define HCR_EL2_NV1_MASK                                GENMASK(43, 43)
#define HCR_EL2_NV1_SHIFT                               43
#define HCR_EL2_NV1_WIDTH                               1

#define HCR_EL2_NV                                      GENMASK(42, 42)
#define HCR_EL2_NV_MASK                                 GENMASK(42, 42)
#define HCR_EL2_NV_SHIFT                                42
#define HCR_EL2_NV_WIDTH                                1

#define HCR_EL2_API                                     GENMASK(41, 41)
#define HCR_EL2_API_MASK                                GENMASK(41, 41)
#define HCR_EL2_API_SHIFT                               41
#define HCR_EL2_API_WIDTH                               1

#define HCR_EL2_APK                                     GENMASK(40, 40)
#define HCR_EL2_APK_MASK                                GENMASK(40, 40)
#define HCR_EL2_APK_SHIFT                               40
#define HCR_EL2_APK_WIDTH                               1

#define HCR_EL2_TME                                     GENMASK(39, 39)
#define HCR_EL2_TME_MASK                                GENMASK(39, 39)
#define HCR_EL2_TME_SHIFT                               39
#define HCR_EL2_TME_WIDTH                               1

#define HCR_EL2_MIOCNCE                                 GENMASK(38, 38)
#define HCR_EL2_MIOCNCE_MASK                            GENMASK(38, 38)
#define HCR_EL2_MIOCNCE_SHIFT                           38
#define HCR_EL2_MIOCNCE_WIDTH                           1

#define HCR_EL2_TEA                                     GENMASK(37, 37)
#define HCR_EL2_TEA_MASK                                GENMASK(37, 37)
#define HCR_EL2_TEA_SHIFT                               37
#define HCR_EL2_TEA_WIDTH                               1

#define HCR_EL2_TERR                                    GENMASK(36, 36)
#define HCR_EL2_TERR_MASK                               GENMASK(36, 36)
#define HCR_EL2_TERR_SHIFT                              36
#define HCR_EL2_TERR_WIDTH                              1

#define HCR_EL2_TLOR                                    GENMASK(35, 35)
#define HCR_EL2_TLOR_MASK                               GENMASK(35, 35)
#define HCR_EL2_TLOR_SHIFT                              35
#define HCR_EL2_TLOR_WIDTH                              1

#define HCR_EL2_E2H                                     GENMASK(34, 34)
#define HCR_EL2_E2H_MASK                                GENMASK(34, 34)
#define HCR_EL2_E2H_SHIFT                               34
#define HCR_EL2_E2H_WIDTH                               1

#define HCR_EL2_ID                                      GENMASK(33, 33)
#define HCR_EL2_ID_MASK                                 GENMASK(33, 33)
#define HCR_EL2_ID_SHIFT                                33
#define HCR_EL2_ID_WIDTH                                1

#define HCR_EL2_CD                                      GENMASK(32, 32)
#define HCR_EL2_CD_MASK                                 GENMASK(32, 32)
#define HCR_EL2_CD_SHIFT                                32
#define HCR_EL2_CD_WIDTH                                1

#define HCR_EL2_RW                                      GENMASK(31, 31)
#define HCR_EL2_RW_MASK                                 GENMASK(31, 31)
#define HCR_EL2_RW_SHIFT                                31
#define HCR_EL2_RW_WIDTH                                1

#define HCR_EL2_TRVM                                    GENMASK(30, 30)
#define HCR_EL2_TRVM_MASK                               GENMASK(30, 30)
#define HCR_EL2_TRVM_SHIFT                              30
#define HCR_EL2_TRVM_WIDTH                              1

#define HCR_EL2_HCD                                     GENMASK(29, 29)
#define HCR_EL2_HCD_MASK                                GENMASK(29, 29)
#define HCR_EL2_HCD_SHIFT                               29
#define HCR_EL2_HCD_WIDTH                               1

#define HCR_EL2_TDZ                                     GENMASK(28, 28)
#define HCR_EL2_TDZ_MASK                                GENMASK(28, 28)
#define HCR_EL2_TDZ_SHIFT                               28
#define HCR_EL2_TDZ_WIDTH                               1

#define HCR_EL2_TGE                                     GENMASK(27, 27)
#define HCR_EL2_TGE_MASK                                GENMASK(27, 27)
#define HCR_EL2_TGE_SHIFT                               27
#define HCR_EL2_TGE_WIDTH                               1

#define HCR_EL2_TVM                                     GENMASK(26, 26)
#define HCR_EL2_TVM_MASK                                GENMASK(26, 26)
#define HCR_EL2_TVM_SHIFT                               26
#define HCR_EL2_TVM_WIDTH                               1

#define HCR_EL2_TTLB                                    GENMASK(25, 25)
#define HCR_EL2_TTLB_MASK                               GENMASK(25, 25)
#define HCR_EL2_TTLB_SHIFT                              25
#define HCR_EL2_TTLB_WIDTH                              1

#define HCR_EL2_TPU                                     GENMASK(24, 24)
#define HCR_EL2_TPU_MASK                                GENMASK(24, 24)
#define HCR_EL2_TPU_SHIFT                               24
#define HCR_EL2_TPU_WIDTH                               1

#define HCR_EL2_TPCP                                    GENMASK(23, 23)
#define HCR_EL2_TPCP_MASK                               GENMASK(23, 23)
#define HCR_EL2_TPCP_SHIFT                              23
#define HCR_EL2_TPCP_WIDTH                              1

#define HCR_EL2_TSW                                     GENMASK(22, 22)
#define HCR_EL2_TSW_MASK                                GENMASK(22, 22)
#define HCR_EL2_TSW_SHIFT                               22
#define HCR_EL2_TSW_WIDTH                               1

#define HCR_EL2_TACR                                    GENMASK(21, 21)
#define HCR_EL2_TACR_MASK                               GENMASK(21, 21)
#define HCR_EL2_TACR_SHIFT                              21
#define HCR_EL2_TACR_WIDTH                              1

#define HCR_EL2_TIDCP                                   GENMASK(20, 20)
#define HCR_EL2_TIDCP_MASK                              GENMASK(20, 20)
#define HCR_EL2_TIDCP_SHIFT                             20
#define HCR_EL2_TIDCP_WIDTH                             1

#define HCR_EL2_TSC                                     GENMASK(19, 19)
#define HCR_EL2_TSC_MASK                                GENMASK(19, 19)
#define HCR_EL2_TSC_SHIFT                               19
#define HCR_EL2_TSC_WIDTH                               1

#define HCR_EL2_TID3                                    GENMASK(18, 18)
#define HCR_EL2_TID3_MASK                               GENMASK(18, 18)
#define HCR_EL2_TID3_SHIFT                              18
#define HCR_EL2_TID3_WIDTH                              1

#define HCR_EL2_TID2                                    GENMASK(17, 17)
#define HCR_EL2_TID2_MASK                               GENMASK(17, 17)
#define HCR_EL2_TID2_SHIFT                              17
#define HCR_EL2_TID2_WIDTH                              1

#define HCR_EL2_TID1                                    GENMASK(16, 16)
#define HCR_EL2_TID1_MASK                               GENMASK(16, 16)
#define HCR_EL2_TID1_SHIFT                              16
#define HCR_EL2_TID1_WIDTH                              1

#define HCR_EL2_TID0                                    GENMASK(15, 15)
#define HCR_EL2_TID0_MASK                               GENMASK(15, 15)
#define HCR_EL2_TID0_SHIFT                              15
#define HCR_EL2_TID0_WIDTH                              1

#define HCR_EL2_TWE                                     GENMASK(14, 14)
#define HCR_EL2_TWE_MASK                                GENMASK(14, 14)
#define HCR_EL2_TWE_SHIFT                               14
#define HCR_EL2_TWE_WIDTH                               1

#define HCR_EL2_TWI                                     GENMASK(13, 13)
#define HCR_EL2_TWI_MASK                                GENMASK(13, 13)
#define HCR_EL2_TWI_SHIFT                               13
#define HCR_EL2_TWI_WIDTH                               1

#define HCR_EL2_DC                                      GENMASK(12, 12)
#define HCR_EL2_DC_MASK                                 GENMASK(12, 12)
#define HCR_EL2_DC_SHIFT                                12
#define HCR_EL2_DC_WIDTH                                1

#define HCR_EL2_BSU                                     GENMASK(11, 10)
#define HCR_EL2_BSU_MASK                                GENMASK(11, 10)
#define HCR_EL2_BSU_SHIFT                               10
#define HCR_EL2_BSU_WIDTH                               2
#define HCR_EL2_BSU_SIGNED                              false
#define HCR_EL2_BSU_NONE                                UL(0b00)
#define HCR_EL2_BSU_IS                                  UL(0b01)
#define HCR_EL2_BSU_OS                                  UL(0b10)
#define HCR_EL2_BSU_FS                                  UL(0b11)

#define HCR_EL2_FB                                      GENMASK(9, 9)
#define HCR_EL2_FB_MASK                                 GENMASK(9, 9)
#define HCR_EL2_FB_SHIFT                                9
#define HCR_EL2_FB_WIDTH                                1

#define HCR_EL2_VSE                                     GENMASK(8, 8)
#define HCR_EL2_VSE_MASK                                GENMASK(8, 8)
#define HCR_EL2_VSE_SHIFT                               8
#define HCR_EL2_VSE_WIDTH                               1

#define HCR_EL2_VI                                      GENMASK(7, 7)
#define HCR_EL2_VI_MASK                                 GENMASK(7, 7)
#define HCR_EL2_VI_SHIFT                                7
#define HCR_EL2_VI_WIDTH                                1

#define HCR_EL2_VF                                      GENMASK(6, 6)
#define HCR_EL2_VF_MASK                                 GENMASK(6, 6)
#define HCR_EL2_VF_SHIFT                                6
#define HCR_EL2_VF_WIDTH                                1

#define HCR_EL2_AMO                                     GENMASK(5, 5)
#define HCR_EL2_AMO_MASK                                GENMASK(5, 5)
#define HCR_EL2_AMO_SHIFT                               5
#define HCR_EL2_AMO_WIDTH                               1

#define HCR_EL2_IMO                                     GENMASK(4, 4)
#define HCR_EL2_IMO_MASK                                GENMASK(4, 4)
#define HCR_EL2_IMO_SHIFT                               4
#define HCR_EL2_IMO_WIDTH                               1

#define HCR_EL2_FMO                                     GENMASK(3, 3)
#define HCR_EL2_FMO_MASK                                GENMASK(3, 3)
#define HCR_EL2_FMO_SHIFT                               3
#define HCR_EL2_FMO_WIDTH                               1

#define HCR_EL2_PTW                                     GENMASK(2, 2)
#define HCR_EL2_PTW_MASK                                GENMASK(2, 2)
#define HCR_EL2_PTW_SHIFT                               2
#define HCR_EL2_PTW_WIDTH                               1

#define HCR_EL2_SWIO                                    GENMASK(1, 1)
#define HCR_EL2_SWIO_MASK                               GENMASK(1, 1)
#define HCR_EL2_SWIO_SHIFT                              1
#define HCR_EL2_SWIO_WIDTH                              1

#define HCR_EL2_VM                                      GENMASK(0, 0)
#define HCR_EL2_VM_MASK                                 GENMASK(0, 0)
#define HCR_EL2_VM_SHIFT                                0
#define HCR_EL2_VM_WIDTH                                1

#define HCR_EL2_RES0                                    (UL(0))
#define HCR_EL2_RES1                                    (UL(0))
#define HCR_EL2_UNKN                                    (UL(0))

#define REG_MDCR_EL2                                    S3_4_C1_C1_1
#define SYS_MDCR_EL2                                    sys_reg(3, 4, 1, 1, 1)
#define SYS_MDCR_EL2_Op0                                3
#define SYS_MDCR_EL2_Op1                                4
#define SYS_MDCR_EL2_CRn                                1
#define SYS_MDCR_EL2_CRm                                1
#define SYS_MDCR_EL2_Op2                                1

#define MDCR_EL2_EnSTEPOP                               GENMASK(50, 50)
#define MDCR_EL2_EnSTEPOP_MASK                          GENMASK(50, 50)
#define MDCR_EL2_EnSTEPOP_SHIFT                         50
#define MDCR_EL2_EnSTEPOP_WIDTH                         1

#define MDCR_EL2_EBWE                                   GENMASK(43, 43)
#define MDCR_EL2_EBWE_MASK                              GENMASK(43, 43)
#define MDCR_EL2_EBWE_SHIFT                             43
#define MDCR_EL2_EBWE_WIDTH                             1

#define MDCR_EL2_PMEE                                   GENMASK(41, 40)
#define MDCR_EL2_PMEE_MASK                              GENMASK(41, 40)
#define MDCR_EL2_PMEE_SHIFT                             40
#define MDCR_EL2_PMEE_WIDTH                             2

#define MDCR_EL2_HPMFZS                                 GENMASK(36, 36)
#define MDCR_EL2_HPMFZS_MASK                            GENMASK(36, 36)
#define MDCR_EL2_HPMFZS_SHIFT                           36
#define MDCR_EL2_HPMFZS_WIDTH                           1

#define MDCR_EL2_PMSSE                                  GENMASK(31, 30)
#define MDCR_EL2_PMSSE_MASK                             GENMASK(31, 30)
#define MDCR_EL2_PMSSE_SHIFT                            30
#define MDCR_EL2_PMSSE_WIDTH                            2

#define MDCR_EL2_HPMFZO                                 GENMASK(29, 29)
#define MDCR_EL2_HPMFZO_MASK                            GENMASK(29, 29)
#define MDCR_EL2_HPMFZO_SHIFT                           29
#define MDCR_EL2_HPMFZO_WIDTH                           1

#define MDCR_EL2_MTPME                                  GENMASK(28, 28)
#define MDCR_EL2_MTPME_MASK                             GENMASK(28, 28)
#define MDCR_EL2_MTPME_SHIFT                            28
#define MDCR_EL2_MTPME_WIDTH                            1

#define MDCR_EL2_TDCC                                   GENMASK(27, 27)
#define MDCR_EL2_TDCC_MASK                              GENMASK(27, 27)
#define MDCR_EL2_TDCC_SHIFT                             27
#define MDCR_EL2_TDCC_WIDTH                             1

#define MDCR_EL2_HLP                                    GENMASK(26, 26)
#define MDCR_EL2_HLP_MASK                               GENMASK(26, 26)
#define MDCR_EL2_HLP_SHIFT                              26
#define MDCR_EL2_HLP_WIDTH                              1

#define MDCR_EL2_E2TB                                   GENMASK(25, 24)
#define MDCR_EL2_E2TB_MASK                              GENMASK(25, 24)
#define MDCR_EL2_E2TB_SHIFT                             24
#define MDCR_EL2_E2TB_WIDTH                             2

#define MDCR_EL2_HCCD                                   GENMASK(23, 23)
#define MDCR_EL2_HCCD_MASK                              GENMASK(23, 23)
#define MDCR_EL2_HCCD_SHIFT                             23
#define MDCR_EL2_HCCD_WIDTH                             1

#define MDCR_EL2_TTRF                                   GENMASK(19, 19)
#define MDCR_EL2_TTRF_MASK                              GENMASK(19, 19)
#define MDCR_EL2_TTRF_SHIFT                             19
#define MDCR_EL2_TTRF_WIDTH                             1

#define MDCR_EL2_HPMD                                   GENMASK(17, 17)
#define MDCR_EL2_HPMD_MASK                              GENMASK(17, 17)
#define MDCR_EL2_HPMD_SHIFT                             17
#define MDCR_EL2_HPMD_WIDTH                             1

#define MDCR_EL2_EnSPM                                  GENMASK(15, 15)
#define MDCR_EL2_EnSPM_MASK                             GENMASK(15, 15)
#define MDCR_EL2_EnSPM_SHIFT                            15
#define MDCR_EL2_EnSPM_WIDTH                            1

#define MDCR_EL2_TPMS                                   GENMASK(14, 14)
#define MDCR_EL2_TPMS_MASK                              GENMASK(14, 14)
#define MDCR_EL2_TPMS_SHIFT                             14
#define MDCR_EL2_TPMS_WIDTH                             1

#define MDCR_EL2_E2PB                                   GENMASK(13, 12)
#define MDCR_EL2_E2PB_MASK                              GENMASK(13, 12)
#define MDCR_EL2_E2PB_SHIFT                             12
#define MDCR_EL2_E2PB_WIDTH                             2

#define MDCR_EL2_TDRA                                   GENMASK(11, 11)
#define MDCR_EL2_TDRA_MASK                              GENMASK(11, 11)
#define MDCR_EL2_TDRA_SHIFT                             11
#define MDCR_EL2_TDRA_WIDTH                             1

#define MDCR_EL2_TDOSA                                  GENMASK(10, 10)
#define MDCR_EL2_TDOSA_MASK                             GENMASK(10, 10)
#define MDCR_EL2_TDOSA_SHIFT                            10
#define MDCR_EL2_TDOSA_WIDTH                            1

#define MDCR_EL2_TDA                                    GENMASK(9, 9)
#define MDCR_EL2_TDA_MASK                               GENMASK(9, 9)
#define MDCR_EL2_TDA_SHIFT                              9
#define MDCR_EL2_TDA_WIDTH                              1

#define MDCR_EL2_TDE                                    GENMASK(8, 8)
#define MDCR_EL2_TDE_MASK                               GENMASK(8, 8)
#define MDCR_EL2_TDE_SHIFT                              8
#define MDCR_EL2_TDE_WIDTH                              1

#define MDCR_EL2_HPME                                   GENMASK(7, 7)
#define MDCR_EL2_HPME_MASK                              GENMASK(7, 7)
#define MDCR_EL2_HPME_SHIFT                             7
#define MDCR_EL2_HPME_WIDTH                             1

#define MDCR_EL2_TPM                                    GENMASK(6, 6)
#define MDCR_EL2_TPM_MASK                               GENMASK(6, 6)
#define MDCR_EL2_TPM_SHIFT                              6
#define MDCR_EL2_TPM_WIDTH                              1

#define MDCR_EL2_TPMCR                                  GENMASK(5, 5)
#define MDCR_EL2_TPMCR_MASK                             GENMASK(5, 5)
#define MDCR_EL2_TPMCR_SHIFT                            5
#define MDCR_EL2_TPMCR_WIDTH                            1

#define MDCR_EL2_HPMN                                   GENMASK(4, 0)
#define MDCR_EL2_HPMN_MASK                              GENMASK(4, 0)
#define MDCR_EL2_HPMN_SHIFT                             0
#define MDCR_EL2_HPMN_WIDTH                             5

#define MDCR_EL2_RES0                                   (UL(0) | GENMASK_ULL(63, 51) | GENMASK_ULL(49, 44) | GENMASK_ULL(42, 42) | GENMASK_ULL(39, 37) | GENMASK_ULL(35, 32) | GENMASK_ULL(22, 20) | GENMASK_ULL(18, 18) | GENMASK_ULL(16, 16))
#define MDCR_EL2_RES1                                   (UL(0))
#define MDCR_EL2_UNKN                                   (UL(0))

#define REG_HFGRTR_EL2                                  S3_4_C1_C1_4
#define SYS_HFGRTR_EL2                                  sys_reg(3, 4, 1, 1, 4)
#define SYS_HFGRTR_EL2_Op0                              3
#define SYS_HFGRTR_EL2_Op1                              4
#define SYS_HFGRTR_EL2_CRn                              1
#define SYS_HFGRTR_EL2_CRm                              1
#define SYS_HFGRTR_EL2_Op2                              4

#define HFGRTR_EL2_nAMAIR2_EL1                          GENMASK(63, 63)
#define HFGRTR_EL2_nAMAIR2_EL1_MASK                     GENMASK(63, 63)
#define HFGRTR_EL2_nAMAIR2_EL1_SHIFT                    63
#define HFGRTR_EL2_nAMAIR2_EL1_WIDTH                    1

#define HFGRTR_EL2_nMAIR2_EL1                           GENMASK(62, 62)
#define HFGRTR_EL2_nMAIR2_EL1_MASK                      GENMASK(62, 62)
#define HFGRTR_EL2_nMAIR2_EL1_SHIFT                     62
#define HFGRTR_EL2_nMAIR2_EL1_WIDTH                     1

#define HFGRTR_EL2_nS2POR_EL1                           GENMASK(61, 61)
#define HFGRTR_EL2_nS2POR_EL1_MASK                      GENMASK(61, 61)
#define HFGRTR_EL2_nS2POR_EL1_SHIFT                     61
#define HFGRTR_EL2_nS2POR_EL1_WIDTH                     1

#define HFGRTR_EL2_nPOR_EL1                             GENMASK(60, 60)
#define HFGRTR_EL2_nPOR_EL1_MASK                        GENMASK(60, 60)
#define HFGRTR_EL2_nPOR_EL1_SHIFT                       60
#define HFGRTR_EL2_nPOR_EL1_WIDTH                       1

#define HFGRTR_EL2_nPOR_EL0                             GENMASK(59, 59)
#define HFGRTR_EL2_nPOR_EL0_MASK                        GENMASK(59, 59)
#define HFGRTR_EL2_nPOR_EL0_SHIFT                       59
#define HFGRTR_EL2_nPOR_EL0_WIDTH                       1

#define HFGRTR_EL2_nPIR_EL1                             GENMASK(58, 58)
#define HFGRTR_EL2_nPIR_EL1_MASK                        GENMASK(58, 58)
#define HFGRTR_EL2_nPIR_EL1_SHIFT                       58
#define HFGRTR_EL2_nPIR_EL1_WIDTH                       1

#define HFGRTR_EL2_nPIRE0_EL1                           GENMASK(57, 57)
#define HFGRTR_EL2_nPIRE0_EL1_MASK                      GENMASK(57, 57)
#define HFGRTR_EL2_nPIRE0_EL1_SHIFT                     57
#define HFGRTR_EL2_nPIRE0_EL1_WIDTH                     1

#define HFGRTR_EL2_nRCWMASK_EL1                         GENMASK(56, 56)
#define HFGRTR_EL2_nRCWMASK_EL1_MASK                    GENMASK(56, 56)
#define HFGRTR_EL2_nRCWMASK_EL1_SHIFT                   56
#define HFGRTR_EL2_nRCWMASK_EL1_WIDTH                   1

#define HFGRTR_EL2_nTPIDR2_EL0                          GENMASK(55, 55)
#define HFGRTR_EL2_nTPIDR2_EL0_MASK                     GENMASK(55, 55)
#define HFGRTR_EL2_nTPIDR2_EL0_SHIFT                    55
#define HFGRTR_EL2_nTPIDR2_EL0_WIDTH                    1

#define HFGRTR_EL2_nSMPRI_EL1                           GENMASK(54, 54)
#define HFGRTR_EL2_nSMPRI_EL1_MASK                      GENMASK(54, 54)
#define HFGRTR_EL2_nSMPRI_EL1_SHIFT                     54
#define HFGRTR_EL2_nSMPRI_EL1_WIDTH                     1

#define HFGRTR_EL2_nGCS_EL1                             GENMASK(53, 53)
#define HFGRTR_EL2_nGCS_EL1_MASK                        GENMASK(53, 53)
#define HFGRTR_EL2_nGCS_EL1_SHIFT                       53
#define HFGRTR_EL2_nGCS_EL1_WIDTH                       1

#define HFGRTR_EL2_nGCS_EL0                             GENMASK(52, 52)
#define HFGRTR_EL2_nGCS_EL0_MASK                        GENMASK(52, 52)
#define HFGRTR_EL2_nGCS_EL0_SHIFT                       52
#define HFGRTR_EL2_nGCS_EL0_WIDTH                       1

#define HFGRTR_EL2_nACCDATA_EL1                         GENMASK(50, 50)
#define HFGRTR_EL2_nACCDATA_EL1_MASK                    GENMASK(50, 50)
#define HFGRTR_EL2_nACCDATA_EL1_SHIFT                   50
#define HFGRTR_EL2_nACCDATA_EL1_WIDTH                   1

#define HFGRTR_EL2_ERXADDR_EL1                          GENMASK(49, 49)
#define HFGRTR_EL2_ERXADDR_EL1_MASK                     GENMASK(49, 49)
#define HFGRTR_EL2_ERXADDR_EL1_SHIFT                    49
#define HFGRTR_EL2_ERXADDR_EL1_WIDTH                    1

#define HFGRTR_EL2_ERXPFGCDN_EL1                        GENMASK(48, 48)
#define HFGRTR_EL2_ERXPFGCDN_EL1_MASK                   GENMASK(48, 48)
#define HFGRTR_EL2_ERXPFGCDN_EL1_SHIFT                  48
#define HFGRTR_EL2_ERXPFGCDN_EL1_WIDTH                  1

#define HFGRTR_EL2_ERXPFGCTL_EL1                        GENMASK(47, 47)
#define HFGRTR_EL2_ERXPFGCTL_EL1_MASK                   GENMASK(47, 47)
#define HFGRTR_EL2_ERXPFGCTL_EL1_SHIFT                  47
#define HFGRTR_EL2_ERXPFGCTL_EL1_WIDTH                  1

#define HFGRTR_EL2_ERXPFGF_EL1                          GENMASK(46, 46)
#define HFGRTR_EL2_ERXPFGF_EL1_MASK                     GENMASK(46, 46)
#define HFGRTR_EL2_ERXPFGF_EL1_SHIFT                    46
#define HFGRTR_EL2_ERXPFGF_EL1_WIDTH                    1

#define HFGRTR_EL2_ERXMISCn_EL1                         GENMASK(45, 45)
#define HFGRTR_EL2_ERXMISCn_EL1_MASK                    GENMASK(45, 45)
#define HFGRTR_EL2_ERXMISCn_EL1_SHIFT                   45
#define HFGRTR_EL2_ERXMISCn_EL1_WIDTH                   1

#define HFGRTR_EL2_ERXSTATUS_EL1                        GENMASK(44, 44)
#define HFGRTR_EL2_ERXSTATUS_EL1_MASK                   GENMASK(44, 44)
#define HFGRTR_EL2_ERXSTATUS_EL1_SHIFT                  44
#define HFGRTR_EL2_ERXSTATUS_EL1_WIDTH                  1

#define HFGRTR_EL2_ERXCTLR_EL1                          GENMASK(43, 43)
#define HFGRTR_EL2_ERXCTLR_EL1_MASK                     GENMASK(43, 43)
#define HFGRTR_EL2_ERXCTLR_EL1_SHIFT                    43
#define HFGRTR_EL2_ERXCTLR_EL1_WIDTH                    1

#define HFGRTR_EL2_ERXFR_EL1                            GENMASK(42, 42)
#define HFGRTR_EL2_ERXFR_EL1_MASK                       GENMASK(42, 42)
#define HFGRTR_EL2_ERXFR_EL1_SHIFT                      42
#define HFGRTR_EL2_ERXFR_EL1_WIDTH                      1

#define HFGRTR_EL2_ERRSELR_EL1                          GENMASK(41, 41)
#define HFGRTR_EL2_ERRSELR_EL1_MASK                     GENMASK(41, 41)
#define HFGRTR_EL2_ERRSELR_EL1_SHIFT                    41
#define HFGRTR_EL2_ERRSELR_EL1_WIDTH                    1

#define HFGRTR_EL2_ERRIDR_EL1                           GENMASK(40, 40)
#define HFGRTR_EL2_ERRIDR_EL1_MASK                      GENMASK(40, 40)
#define HFGRTR_EL2_ERRIDR_EL1_SHIFT                     40
#define HFGRTR_EL2_ERRIDR_EL1_WIDTH                     1

#define HFGRTR_EL2_ICC_IGRPENn_EL1                      GENMASK(39, 39)
#define HFGRTR_EL2_ICC_IGRPENn_EL1_MASK                 GENMASK(39, 39)
#define HFGRTR_EL2_ICC_IGRPENn_EL1_SHIFT                39
#define HFGRTR_EL2_ICC_IGRPENn_EL1_WIDTH                1

#define HFGRTR_EL2_VBAR_EL1                             GENMASK(38, 38)
#define HFGRTR_EL2_VBAR_EL1_MASK                        GENMASK(38, 38)
#define HFGRTR_EL2_VBAR_EL1_SHIFT                       38
#define HFGRTR_EL2_VBAR_EL1_WIDTH                       1

#define HFGRTR_EL2_TTBR1_EL1                            GENMASK(37, 37)
#define HFGRTR_EL2_TTBR1_EL1_MASK                       GENMASK(37, 37)
#define HFGRTR_EL2_TTBR1_EL1_SHIFT                      37
#define HFGRTR_EL2_TTBR1_EL1_WIDTH                      1

#define HFGRTR_EL2_TTBR0_EL1                            GENMASK(36, 36)
#define HFGRTR_EL2_TTBR0_EL1_MASK                       GENMASK(36, 36)
#define HFGRTR_EL2_TTBR0_EL1_SHIFT                      36
#define HFGRTR_EL2_TTBR0_EL1_WIDTH                      1

#define HFGRTR_EL2_TPIDR_EL0                            GENMASK(35, 35)
#define HFGRTR_EL2_TPIDR_EL0_MASK                       GENMASK(35, 35)
#define HFGRTR_EL2_TPIDR_EL0_SHIFT                      35
#define HFGRTR_EL2_TPIDR_EL0_WIDTH                      1

#define HFGRTR_EL2_TPIDRRO_EL0                          GENMASK(34, 34)
#define HFGRTR_EL2_TPIDRRO_EL0_MASK                     GENMASK(34, 34)
#define HFGRTR_EL2_TPIDRRO_EL0_SHIFT                    34
#define HFGRTR_EL2_TPIDRRO_EL0_WIDTH                    1

#define HFGRTR_EL2_TPIDR_EL1                            GENMASK(33, 33)
#define HFGRTR_EL2_TPIDR_EL1_MASK                       GENMASK(33, 33)
#define HFGRTR_EL2_TPIDR_EL1_SHIFT                      33
#define HFGRTR_EL2_TPIDR_EL1_WIDTH                      1

#define HFGRTR_EL2_TCR_EL1                              GENMASK(32, 32)
#define HFGRTR_EL2_TCR_EL1_MASK                         GENMASK(32, 32)
#define HFGRTR_EL2_TCR_EL1_SHIFT                        32
#define HFGRTR_EL2_TCR_EL1_WIDTH                        1

#define HFGRTR_EL2_SCXTNUM_EL0                          GENMASK(31, 31)
#define HFGRTR_EL2_SCXTNUM_EL0_MASK                     GENMASK(31, 31)
#define HFGRTR_EL2_SCXTNUM_EL0_SHIFT                    31
#define HFGRTR_EL2_SCXTNUM_EL0_WIDTH                    1

#define HFGRTR_EL2_SCXTNUM_EL1                          GENMASK(30, 30)
#define HFGRTR_EL2_SCXTNUM_EL1_MASK                     GENMASK(30, 30)
#define HFGRTR_EL2_SCXTNUM_EL1_SHIFT                    30
#define HFGRTR_EL2_SCXTNUM_EL1_WIDTH                    1

#define HFGRTR_EL2_SCTLR_EL1                            GENMASK(29, 29)
#define HFGRTR_EL2_SCTLR_EL1_MASK                       GENMASK(29, 29)
#define HFGRTR_EL2_SCTLR_EL1_SHIFT                      29
#define HFGRTR_EL2_SCTLR_EL1_WIDTH                      1

#define HFGRTR_EL2_REVIDR_EL1                           GENMASK(28, 28)
#define HFGRTR_EL2_REVIDR_EL1_MASK                      GENMASK(28, 28)
#define HFGRTR_EL2_REVIDR_EL1_SHIFT                     28
#define HFGRTR_EL2_REVIDR_EL1_WIDTH                     1

#define HFGRTR_EL2_PAR_EL1                              GENMASK(27, 27)
#define HFGRTR_EL2_PAR_EL1_MASK                         GENMASK(27, 27)
#define HFGRTR_EL2_PAR_EL1_SHIFT                        27
#define HFGRTR_EL2_PAR_EL1_WIDTH                        1

#define HFGRTR_EL2_MPIDR_EL1                            GENMASK(26, 26)
#define HFGRTR_EL2_MPIDR_EL1_MASK                       GENMASK(26, 26)
#define HFGRTR_EL2_MPIDR_EL1_SHIFT                      26
#define HFGRTR_EL2_MPIDR_EL1_WIDTH                      1

#define HFGRTR_EL2_MIDR_EL1                             GENMASK(25, 25)
#define HFGRTR_EL2_MIDR_EL1_MASK                        GENMASK(25, 25)
#define HFGRTR_EL2_MIDR_EL1_SHIFT                       25
#define HFGRTR_EL2_MIDR_EL1_WIDTH                       1

#define HFGRTR_EL2_MAIR_EL1                             GENMASK(24, 24)
#define HFGRTR_EL2_MAIR_EL1_MASK                        GENMASK(24, 24)
#define HFGRTR_EL2_MAIR_EL1_SHIFT                       24
#define HFGRTR_EL2_MAIR_EL1_WIDTH                       1

#define HFGRTR_EL2_LORSA_EL1                            GENMASK(23, 23)
#define HFGRTR_EL2_LORSA_EL1_MASK                       GENMASK(23, 23)
#define HFGRTR_EL2_LORSA_EL1_SHIFT                      23
#define HFGRTR_EL2_LORSA_EL1_WIDTH                      1

#define HFGRTR_EL2_LORN_EL1                             GENMASK(22, 22)
#define HFGRTR_EL2_LORN_EL1_MASK                        GENMASK(22, 22)
#define HFGRTR_EL2_LORN_EL1_SHIFT                       22
#define HFGRTR_EL2_LORN_EL1_WIDTH                       1

#define HFGRTR_EL2_LORID_EL1                            GENMASK(21, 21)
#define HFGRTR_EL2_LORID_EL1_MASK                       GENMASK(21, 21)
#define HFGRTR_EL2_LORID_EL1_SHIFT                      21
#define HFGRTR_EL2_LORID_EL1_WIDTH                      1

#define HFGRTR_EL2_LOREA_EL1                            GENMASK(20, 20)
#define HFGRTR_EL2_LOREA_EL1_MASK                       GENMASK(20, 20)
#define HFGRTR_EL2_LOREA_EL1_SHIFT                      20
#define HFGRTR_EL2_LOREA_EL1_WIDTH                      1

#define HFGRTR_EL2_LORC_EL1                             GENMASK(19, 19)
#define HFGRTR_EL2_LORC_EL1_MASK                        GENMASK(19, 19)
#define HFGRTR_EL2_LORC_EL1_SHIFT                       19
#define HFGRTR_EL2_LORC_EL1_WIDTH                       1

#define HFGRTR_EL2_ISR_EL1                              GENMASK(18, 18)
#define HFGRTR_EL2_ISR_EL1_MASK                         GENMASK(18, 18)
#define HFGRTR_EL2_ISR_EL1_SHIFT                        18
#define HFGRTR_EL2_ISR_EL1_WIDTH                        1

#define HFGRTR_EL2_FAR_EL1                              GENMASK(17, 17)
#define HFGRTR_EL2_FAR_EL1_MASK                         GENMASK(17, 17)
#define HFGRTR_EL2_FAR_EL1_SHIFT                        17
#define HFGRTR_EL2_FAR_EL1_WIDTH                        1

#define HFGRTR_EL2_ESR_EL1                              GENMASK(16, 16)
#define HFGRTR_EL2_ESR_EL1_MASK                         GENMASK(16, 16)
#define HFGRTR_EL2_ESR_EL1_SHIFT                        16
#define HFGRTR_EL2_ESR_EL1_WIDTH                        1

#define HFGRTR_EL2_DCZID_EL0                            GENMASK(15, 15)
#define HFGRTR_EL2_DCZID_EL0_MASK                       GENMASK(15, 15)
#define HFGRTR_EL2_DCZID_EL0_SHIFT                      15
#define HFGRTR_EL2_DCZID_EL0_WIDTH                      1

#define HFGRTR_EL2_CTR_EL0                              GENMASK(14, 14)
#define HFGRTR_EL2_CTR_EL0_MASK                         GENMASK(14, 14)
#define HFGRTR_EL2_CTR_EL0_SHIFT                        14
#define HFGRTR_EL2_CTR_EL0_WIDTH                        1

#define HFGRTR_EL2_CSSELR_EL1                           GENMASK(13, 13)
#define HFGRTR_EL2_CSSELR_EL1_MASK                      GENMASK(13, 13)
#define HFGRTR_EL2_CSSELR_EL1_SHIFT                     13
#define HFGRTR_EL2_CSSELR_EL1_WIDTH                     1

#define HFGRTR_EL2_CPACR_EL1                            GENMASK(12, 12)
#define HFGRTR_EL2_CPACR_EL1_MASK                       GENMASK(12, 12)
#define HFGRTR_EL2_CPACR_EL1_SHIFT                      12
#define HFGRTR_EL2_CPACR_EL1_WIDTH                      1

#define HFGRTR_EL2_CONTEXTIDR_EL1                       GENMASK(11, 11)
#define HFGRTR_EL2_CONTEXTIDR_EL1_MASK                  GENMASK(11, 11)
#define HFGRTR_EL2_CONTEXTIDR_EL1_SHIFT                 11
#define HFGRTR_EL2_CONTEXTIDR_EL1_WIDTH                 1

#define HFGRTR_EL2_CLIDR_EL1                            GENMASK(10, 10)
#define HFGRTR_EL2_CLIDR_EL1_MASK                       GENMASK(10, 10)
#define HFGRTR_EL2_CLIDR_EL1_SHIFT                      10
#define HFGRTR_EL2_CLIDR_EL1_WIDTH                      1

#define HFGRTR_EL2_CCSIDR_EL1                           GENMASK(9, 9)
#define HFGRTR_EL2_CCSIDR_EL1_MASK                      GENMASK(9, 9)
#define HFGRTR_EL2_CCSIDR_EL1_SHIFT                     9
#define HFGRTR_EL2_CCSIDR_EL1_WIDTH                     1

#define HFGRTR_EL2_APIBKey                              GENMASK(8, 8)
#define HFGRTR_EL2_APIBKey_MASK                         GENMASK(8, 8)
#define HFGRTR_EL2_APIBKey_SHIFT                        8
#define HFGRTR_EL2_APIBKey_WIDTH                        1

#define HFGRTR_EL2_APIAKey                              GENMASK(7, 7)
#define HFGRTR_EL2_APIAKey_MASK                         GENMASK(7, 7)
#define HFGRTR_EL2_APIAKey_SHIFT                        7
#define HFGRTR_EL2_APIAKey_WIDTH                        1

#define HFGRTR_EL2_APGAKey                              GENMASK(6, 6)
#define HFGRTR_EL2_APGAKey_MASK                         GENMASK(6, 6)
#define HFGRTR_EL2_APGAKey_SHIFT                        6
#define HFGRTR_EL2_APGAKey_WIDTH                        1

#define HFGRTR_EL2_APDBKey                              GENMASK(5, 5)
#define HFGRTR_EL2_APDBKey_MASK                         GENMASK(5, 5)
#define HFGRTR_EL2_APDBKey_SHIFT                        5
#define HFGRTR_EL2_APDBKey_WIDTH                        1

#define HFGRTR_EL2_APDAKey                              GENMASK(4, 4)
#define HFGRTR_EL2_APDAKey_MASK                         GENMASK(4, 4)
#define HFGRTR_EL2_APDAKey_SHIFT                        4
#define HFGRTR_EL2_APDAKey_WIDTH                        1

#define HFGRTR_EL2_AMAIR_EL1                            GENMASK(3, 3)
#define HFGRTR_EL2_AMAIR_EL1_MASK                       GENMASK(3, 3)
#define HFGRTR_EL2_AMAIR_EL1_SHIFT                      3
#define HFGRTR_EL2_AMAIR_EL1_WIDTH                      1

#define HFGRTR_EL2_AIDR_EL1                             GENMASK(2, 2)
#define HFGRTR_EL2_AIDR_EL1_MASK                        GENMASK(2, 2)
#define HFGRTR_EL2_AIDR_EL1_SHIFT                       2
#define HFGRTR_EL2_AIDR_EL1_WIDTH                       1

#define HFGRTR_EL2_AFSR1_EL1                            GENMASK(1, 1)
#define HFGRTR_EL2_AFSR1_EL1_MASK                       GENMASK(1, 1)
#define HFGRTR_EL2_AFSR1_EL1_SHIFT                      1
#define HFGRTR_EL2_AFSR1_EL1_WIDTH                      1

#define HFGRTR_EL2_AFSR0_EL1                            GENMASK(0, 0)
#define HFGRTR_EL2_AFSR0_EL1_MASK                       GENMASK(0, 0)
#define HFGRTR_EL2_AFSR0_EL1_SHIFT                      0
#define HFGRTR_EL2_AFSR0_EL1_WIDTH                      1

#define HFGRTR_EL2_RES0                                 (UL(0) | GENMASK_ULL(51, 51))
#define HFGRTR_EL2_RES1                                 (UL(0))
#define HFGRTR_EL2_UNKN                                 (UL(0))

#define REG_HFGWTR_EL2                                  S3_4_C1_C1_5
#define SYS_HFGWTR_EL2                                  sys_reg(3, 4, 1, 1, 5)
#define SYS_HFGWTR_EL2_Op0                              3
#define SYS_HFGWTR_EL2_Op1                              4
#define SYS_HFGWTR_EL2_CRn                              1
#define SYS_HFGWTR_EL2_CRm                              1
#define SYS_HFGWTR_EL2_Op2                              5

#define HFGWTR_EL2_nAMAIR2_EL1                          GENMASK(63, 63)
#define HFGWTR_EL2_nAMAIR2_EL1_MASK                     GENMASK(63, 63)
#define HFGWTR_EL2_nAMAIR2_EL1_SHIFT                    63
#define HFGWTR_EL2_nAMAIR2_EL1_WIDTH                    1

#define HFGWTR_EL2_nMAIR2_EL1                           GENMASK(62, 62)
#define HFGWTR_EL2_nMAIR2_EL1_MASK                      GENMASK(62, 62)
#define HFGWTR_EL2_nMAIR2_EL1_SHIFT                     62
#define HFGWTR_EL2_nMAIR2_EL1_WIDTH                     1

#define HFGWTR_EL2_nS2POR_EL1                           GENMASK(61, 61)
#define HFGWTR_EL2_nS2POR_EL1_MASK                      GENMASK(61, 61)
#define HFGWTR_EL2_nS2POR_EL1_SHIFT                     61
#define HFGWTR_EL2_nS2POR_EL1_WIDTH                     1

#define HFGWTR_EL2_nPOR_EL1                             GENMASK(60, 60)
#define HFGWTR_EL2_nPOR_EL1_MASK                        GENMASK(60, 60)
#define HFGWTR_EL2_nPOR_EL1_SHIFT                       60
#define HFGWTR_EL2_nPOR_EL1_WIDTH                       1

#define HFGWTR_EL2_nPOR_EL0                             GENMASK(59, 59)
#define HFGWTR_EL2_nPOR_EL0_MASK                        GENMASK(59, 59)
#define HFGWTR_EL2_nPOR_EL0_SHIFT                       59
#define HFGWTR_EL2_nPOR_EL0_WIDTH                       1

#define HFGWTR_EL2_nPIR_EL1                             GENMASK(58, 58)
#define HFGWTR_EL2_nPIR_EL1_MASK                        GENMASK(58, 58)
#define HFGWTR_EL2_nPIR_EL1_SHIFT                       58
#define HFGWTR_EL2_nPIR_EL1_WIDTH                       1

#define HFGWTR_EL2_nPIRE0_EL1                           GENMASK(57, 57)
#define HFGWTR_EL2_nPIRE0_EL1_MASK                      GENMASK(57, 57)
#define HFGWTR_EL2_nPIRE0_EL1_SHIFT                     57
#define HFGWTR_EL2_nPIRE0_EL1_WIDTH                     1

#define HFGWTR_EL2_nRCWMASK_EL1                         GENMASK(56, 56)
#define HFGWTR_EL2_nRCWMASK_EL1_MASK                    GENMASK(56, 56)
#define HFGWTR_EL2_nRCWMASK_EL1_SHIFT                   56
#define HFGWTR_EL2_nRCWMASK_EL1_WIDTH                   1

#define HFGWTR_EL2_nTPIDR2_EL0                          GENMASK(55, 55)
#define HFGWTR_EL2_nTPIDR2_EL0_MASK                     GENMASK(55, 55)
#define HFGWTR_EL2_nTPIDR2_EL0_SHIFT                    55
#define HFGWTR_EL2_nTPIDR2_EL0_WIDTH                    1

#define HFGWTR_EL2_nSMPRI_EL1                           GENMASK(54, 54)
#define HFGWTR_EL2_nSMPRI_EL1_MASK                      GENMASK(54, 54)
#define HFGWTR_EL2_nSMPRI_EL1_SHIFT                     54
#define HFGWTR_EL2_nSMPRI_EL1_WIDTH                     1

#define HFGWTR_EL2_nGCS_EL1                             GENMASK(53, 53)
#define HFGWTR_EL2_nGCS_EL1_MASK                        GENMASK(53, 53)
#define HFGWTR_EL2_nGCS_EL1_SHIFT                       53
#define HFGWTR_EL2_nGCS_EL1_WIDTH                       1

#define HFGWTR_EL2_nGCS_EL0                             GENMASK(52, 52)
#define HFGWTR_EL2_nGCS_EL0_MASK                        GENMASK(52, 52)
#define HFGWTR_EL2_nGCS_EL0_SHIFT                       52
#define HFGWTR_EL2_nGCS_EL0_WIDTH                       1

#define HFGWTR_EL2_nACCDATA_EL1                         GENMASK(50, 50)
#define HFGWTR_EL2_nACCDATA_EL1_MASK                    GENMASK(50, 50)
#define HFGWTR_EL2_nACCDATA_EL1_SHIFT                   50
#define HFGWTR_EL2_nACCDATA_EL1_WIDTH                   1

#define HFGWTR_EL2_ERXADDR_EL1                          GENMASK(49, 49)
#define HFGWTR_EL2_ERXADDR_EL1_MASK                     GENMASK(49, 49)
#define HFGWTR_EL2_ERXADDR_EL1_SHIFT                    49
#define HFGWTR_EL2_ERXADDR_EL1_WIDTH                    1

#define HFGWTR_EL2_ERXPFGCDN_EL1                        GENMASK(48, 48)
#define HFGWTR_EL2_ERXPFGCDN_EL1_MASK                   GENMASK(48, 48)
#define HFGWTR_EL2_ERXPFGCDN_EL1_SHIFT                  48
#define HFGWTR_EL2_ERXPFGCDN_EL1_WIDTH                  1

#define HFGWTR_EL2_ERXPFGCTL_EL1                        GENMASK(47, 47)
#define HFGWTR_EL2_ERXPFGCTL_EL1_MASK                   GENMASK(47, 47)
#define HFGWTR_EL2_ERXPFGCTL_EL1_SHIFT                  47
#define HFGWTR_EL2_ERXPFGCTL_EL1_WIDTH                  1

#define HFGWTR_EL2_ERXMISCn_EL1                         GENMASK(45, 45)
#define HFGWTR_EL2_ERXMISCn_EL1_MASK                    GENMASK(45, 45)
#define HFGWTR_EL2_ERXMISCn_EL1_SHIFT                   45
#define HFGWTR_EL2_ERXMISCn_EL1_WIDTH                   1

#define HFGWTR_EL2_ERXSTATUS_EL1                        GENMASK(44, 44)
#define HFGWTR_EL2_ERXSTATUS_EL1_MASK                   GENMASK(44, 44)
#define HFGWTR_EL2_ERXSTATUS_EL1_SHIFT                  44
#define HFGWTR_EL2_ERXSTATUS_EL1_WIDTH                  1

#define HFGWTR_EL2_ERXCTLR_EL1                          GENMASK(43, 43)
#define HFGWTR_EL2_ERXCTLR_EL1_MASK                     GENMASK(43, 43)
#define HFGWTR_EL2_ERXCTLR_EL1_SHIFT                    43
#define HFGWTR_EL2_ERXCTLR_EL1_WIDTH                    1

#define HFGWTR_EL2_ERRSELR_EL1                          GENMASK(41, 41)
#define HFGWTR_EL2_ERRSELR_EL1_MASK                     GENMASK(41, 41)
#define HFGWTR_EL2_ERRSELR_EL1_SHIFT                    41
#define HFGWTR_EL2_ERRSELR_EL1_WIDTH                    1

#define HFGWTR_EL2_ICC_IGRPENn_EL1                      GENMASK(39, 39)
#define HFGWTR_EL2_ICC_IGRPENn_EL1_MASK                 GENMASK(39, 39)
#define HFGWTR_EL2_ICC_IGRPENn_EL1_SHIFT                39
#define HFGWTR_EL2_ICC_IGRPENn_EL1_WIDTH                1

#define HFGWTR_EL2_VBAR_EL1                             GENMASK(38, 38)
#define HFGWTR_EL2_VBAR_EL1_MASK                        GENMASK(38, 38)
#define HFGWTR_EL2_VBAR_EL1_SHIFT                       38
#define HFGWTR_EL2_VBAR_EL1_WIDTH                       1

#define HFGWTR_EL2_TTBR1_EL1                            GENMASK(37, 37)
#define HFGWTR_EL2_TTBR1_EL1_MASK                       GENMASK(37, 37)
#define HFGWTR_EL2_TTBR1_EL1_SHIFT                      37
#define HFGWTR_EL2_TTBR1_EL1_WIDTH                      1

#define HFGWTR_EL2_TTBR0_EL1                            GENMASK(36, 36)
#define HFGWTR_EL2_TTBR0_EL1_MASK                       GENMASK(36, 36)
#define HFGWTR_EL2_TTBR0_EL1_SHIFT                      36
#define HFGWTR_EL2_TTBR0_EL1_WIDTH                      1

#define HFGWTR_EL2_TPIDR_EL0                            GENMASK(35, 35)
#define HFGWTR_EL2_TPIDR_EL0_MASK                       GENMASK(35, 35)
#define HFGWTR_EL2_TPIDR_EL0_SHIFT                      35
#define HFGWTR_EL2_TPIDR_EL0_WIDTH                      1

#define HFGWTR_EL2_TPIDRRO_EL0                          GENMASK(34, 34)
#define HFGWTR_EL2_TPIDRRO_EL0_MASK                     GENMASK(34, 34)
#define HFGWTR_EL2_TPIDRRO_EL0_SHIFT                    34
#define HFGWTR_EL2_TPIDRRO_EL0_WIDTH                    1

#define HFGWTR_EL2_TPIDR_EL1                            GENMASK(33, 33)
#define HFGWTR_EL2_TPIDR_EL1_MASK                       GENMASK(33, 33)
#define HFGWTR_EL2_TPIDR_EL1_SHIFT                      33
#define HFGWTR_EL2_TPIDR_EL1_WIDTH                      1

#define HFGWTR_EL2_TCR_EL1                              GENMASK(32, 32)
#define HFGWTR_EL2_TCR_EL1_MASK                         GENMASK(32, 32)
#define HFGWTR_EL2_TCR_EL1_SHIFT                        32
#define HFGWTR_EL2_TCR_EL1_WIDTH                        1

#define HFGWTR_EL2_SCXTNUM_EL0                          GENMASK(31, 31)
#define HFGWTR_EL2_SCXTNUM_EL0_MASK                     GENMASK(31, 31)
#define HFGWTR_EL2_SCXTNUM_EL0_SHIFT                    31
#define HFGWTR_EL2_SCXTNUM_EL0_WIDTH                    1

#define HFGWTR_EL2_SCXTNUM_EL1                          GENMASK(30, 30)
#define HFGWTR_EL2_SCXTNUM_EL1_MASK                     GENMASK(30, 30)
#define HFGWTR_EL2_SCXTNUM_EL1_SHIFT                    30
#define HFGWTR_EL2_SCXTNUM_EL1_WIDTH                    1

#define HFGWTR_EL2_SCTLR_EL1                            GENMASK(29, 29)
#define HFGWTR_EL2_SCTLR_EL1_MASK                       GENMASK(29, 29)
#define HFGWTR_EL2_SCTLR_EL1_SHIFT                      29
#define HFGWTR_EL2_SCTLR_EL1_WIDTH                      1

#define HFGWTR_EL2_PAR_EL1                              GENMASK(27, 27)
#define HFGWTR_EL2_PAR_EL1_MASK                         GENMASK(27, 27)
#define HFGWTR_EL2_PAR_EL1_SHIFT                        27
#define HFGWTR_EL2_PAR_EL1_WIDTH                        1

#define HFGWTR_EL2_MAIR_EL1                             GENMASK(24, 24)
#define HFGWTR_EL2_MAIR_EL1_MASK                        GENMASK(24, 24)
#define HFGWTR_EL2_MAIR_EL1_SHIFT                       24
#define HFGWTR_EL2_MAIR_EL1_WIDTH                       1

#define HFGWTR_EL2_LORSA_EL1                            GENMASK(23, 23)
#define HFGWTR_EL2_LORSA_EL1_MASK                       GENMASK(23, 23)
#define HFGWTR_EL2_LORSA_EL1_SHIFT                      23
#define HFGWTR_EL2_LORSA_EL1_WIDTH                      1

#define HFGWTR_EL2_LORN_EL1                             GENMASK(22, 22)
#define HFGWTR_EL2_LORN_EL1_MASK                        GENMASK(22, 22)
#define HFGWTR_EL2_LORN_EL1_SHIFT                       22
#define HFGWTR_EL2_LORN_EL1_WIDTH                       1

#define HFGWTR_EL2_LOREA_EL1                            GENMASK(20, 20)
#define HFGWTR_EL2_LOREA_EL1_MASK                       GENMASK(20, 20)
#define HFGWTR_EL2_LOREA_EL1_SHIFT                      20
#define HFGWTR_EL2_LOREA_EL1_WIDTH                      1

#define HFGWTR_EL2_LORC_EL1                             GENMASK(19, 19)
#define HFGWTR_EL2_LORC_EL1_MASK                        GENMASK(19, 19)
#define HFGWTR_EL2_LORC_EL1_SHIFT                       19
#define HFGWTR_EL2_LORC_EL1_WIDTH                       1

#define HFGWTR_EL2_FAR_EL1                              GENMASK(17, 17)
#define HFGWTR_EL2_FAR_EL1_MASK                         GENMASK(17, 17)
#define HFGWTR_EL2_FAR_EL1_SHIFT                        17
#define HFGWTR_EL2_FAR_EL1_WIDTH                        1

#define HFGWTR_EL2_ESR_EL1                              GENMASK(16, 16)
#define HFGWTR_EL2_ESR_EL1_MASK                         GENMASK(16, 16)
#define HFGWTR_EL2_ESR_EL1_SHIFT                        16
#define HFGWTR_EL2_ESR_EL1_WIDTH                        1

#define HFGWTR_EL2_CSSELR_EL1                           GENMASK(13, 13)
#define HFGWTR_EL2_CSSELR_EL1_MASK                      GENMASK(13, 13)
#define HFGWTR_EL2_CSSELR_EL1_SHIFT                     13
#define HFGWTR_EL2_CSSELR_EL1_WIDTH                     1

#define HFGWTR_EL2_CPACR_EL1                            GENMASK(12, 12)
#define HFGWTR_EL2_CPACR_EL1_MASK                       GENMASK(12, 12)
#define HFGWTR_EL2_CPACR_EL1_SHIFT                      12
#define HFGWTR_EL2_CPACR_EL1_WIDTH                      1

#define HFGWTR_EL2_CONTEXTIDR_EL1                       GENMASK(11, 11)
#define HFGWTR_EL2_CONTEXTIDR_EL1_MASK                  GENMASK(11, 11)
#define HFGWTR_EL2_CONTEXTIDR_EL1_SHIFT                 11
#define HFGWTR_EL2_CONTEXTIDR_EL1_WIDTH                 1

#define HFGWTR_EL2_APIBKey                              GENMASK(8, 8)
#define HFGWTR_EL2_APIBKey_MASK                         GENMASK(8, 8)
#define HFGWTR_EL2_APIBKey_SHIFT                        8
#define HFGWTR_EL2_APIBKey_WIDTH                        1

#define HFGWTR_EL2_APIAKey                              GENMASK(7, 7)
#define HFGWTR_EL2_APIAKey_MASK                         GENMASK(7, 7)
#define HFGWTR_EL2_APIAKey_SHIFT                        7
#define HFGWTR_EL2_APIAKey_WIDTH                        1

#define HFGWTR_EL2_APGAKey                              GENMASK(6, 6)
#define HFGWTR_EL2_APGAKey_MASK                         GENMASK(6, 6)
#define HFGWTR_EL2_APGAKey_SHIFT                        6
#define HFGWTR_EL2_APGAKey_WIDTH                        1

#define HFGWTR_EL2_APDBKey                              GENMASK(5, 5)
#define HFGWTR_EL2_APDBKey_MASK                         GENMASK(5, 5)
#define HFGWTR_EL2_APDBKey_SHIFT                        5
#define HFGWTR_EL2_APDBKey_WIDTH                        1

#define HFGWTR_EL2_APDAKey                              GENMASK(4, 4)
#define HFGWTR_EL2_APDAKey_MASK                         GENMASK(4, 4)
#define HFGWTR_EL2_APDAKey_SHIFT                        4
#define HFGWTR_EL2_APDAKey_WIDTH                        1

#define HFGWTR_EL2_AMAIR_EL1                            GENMASK(3, 3)
#define HFGWTR_EL2_AMAIR_EL1_MASK                       GENMASK(3, 3)
#define HFGWTR_EL2_AMAIR_EL1_SHIFT                      3
#define HFGWTR_EL2_AMAIR_EL1_WIDTH                      1

#define HFGWTR_EL2_AFSR1_EL1                            GENMASK(1, 1)
#define HFGWTR_EL2_AFSR1_EL1_MASK                       GENMASK(1, 1)
#define HFGWTR_EL2_AFSR1_EL1_SHIFT                      1
#define HFGWTR_EL2_AFSR1_EL1_WIDTH                      1

#define HFGWTR_EL2_AFSR0_EL1                            GENMASK(0, 0)
#define HFGWTR_EL2_AFSR0_EL1_MASK                       GENMASK(0, 0)
#define HFGWTR_EL2_AFSR0_EL1_SHIFT                      0
#define HFGWTR_EL2_AFSR0_EL1_WIDTH                      1

#define HFGWTR_EL2_RES0                                 (UL(0) | GENMASK_ULL(51, 51) | GENMASK_ULL(46, 46) | GENMASK_ULL(42, 42) | GENMASK_ULL(40, 40) | GENMASK_ULL(28, 28) | GENMASK_ULL(26, 25) | GENMASK_ULL(21, 21) | GENMASK_ULL(18, 18) | GENMASK_ULL(15, 14) | GENMASK_ULL(10, 9) | GENMASK_ULL(2, 2))
#define HFGWTR_EL2_RES1                                 (UL(0))
#define HFGWTR_EL2_UNKN                                 (UL(0))

#define REG_HFGITR_EL2                                  S3_4_C1_C1_6
#define SYS_HFGITR_EL2                                  sys_reg(3, 4, 1, 1, 6)
#define SYS_HFGITR_EL2_Op0                              3
#define SYS_HFGITR_EL2_Op1                              4
#define SYS_HFGITR_EL2_CRn                              1
#define SYS_HFGITR_EL2_CRm                              1
#define SYS_HFGITR_EL2_Op2                              6

#define HFGITR_EL2_PSBCSYNC                             GENMASK(63, 63)
#define HFGITR_EL2_PSBCSYNC_MASK                        GENMASK(63, 63)
#define HFGITR_EL2_PSBCSYNC_SHIFT                       63
#define HFGITR_EL2_PSBCSYNC_WIDTH                       1

#define HFGITR_EL2_ATS1E1A                              GENMASK(62, 62)
#define HFGITR_EL2_ATS1E1A_MASK                         GENMASK(62, 62)
#define HFGITR_EL2_ATS1E1A_SHIFT                        62
#define HFGITR_EL2_ATS1E1A_WIDTH                        1

#define HFGITR_EL2_COSPRCTX                             GENMASK(60, 60)
#define HFGITR_EL2_COSPRCTX_MASK                        GENMASK(60, 60)
#define HFGITR_EL2_COSPRCTX_SHIFT                       60
#define HFGITR_EL2_COSPRCTX_WIDTH                       1

#define HFGITR_EL2_nGCSEPP                              GENMASK(59, 59)
#define HFGITR_EL2_nGCSEPP_MASK                         GENMASK(59, 59)
#define HFGITR_EL2_nGCSEPP_SHIFT                        59
#define HFGITR_EL2_nGCSEPP_WIDTH                        1

#define HFGITR_EL2_nGCSSTR_EL1                          GENMASK(58, 58)
#define HFGITR_EL2_nGCSSTR_EL1_MASK                     GENMASK(58, 58)
#define HFGITR_EL2_nGCSSTR_EL1_SHIFT                    58
#define HFGITR_EL2_nGCSSTR_EL1_WIDTH                    1

#define HFGITR_EL2_nGCSPUSHM_EL1                        GENMASK(57, 57)
#define HFGITR_EL2_nGCSPUSHM_EL1_MASK                   GENMASK(57, 57)
#define HFGITR_EL2_nGCSPUSHM_EL1_SHIFT                  57
#define HFGITR_EL2_nGCSPUSHM_EL1_WIDTH                  1

#define HFGITR_EL2_nBRBIALL                             GENMASK(56, 56)
#define HFGITR_EL2_nBRBIALL_MASK                        GENMASK(56, 56)
#define HFGITR_EL2_nBRBIALL_SHIFT                       56
#define HFGITR_EL2_nBRBIALL_WIDTH                       1

#define HFGITR_EL2_nBRBINJ                              GENMASK(55, 55)
#define HFGITR_EL2_nBRBINJ_MASK                         GENMASK(55, 55)
#define HFGITR_EL2_nBRBINJ_SHIFT                        55
#define HFGITR_EL2_nBRBINJ_WIDTH                        1

#define HFGITR_EL2_DCCVAC                               GENMASK(54, 54)
#define HFGITR_EL2_DCCVAC_MASK                          GENMASK(54, 54)
#define HFGITR_EL2_DCCVAC_SHIFT                         54
#define HFGITR_EL2_DCCVAC_WIDTH                         1

#define HFGITR_EL2_SVC_EL1                              GENMASK(53, 53)
#define HFGITR_EL2_SVC_EL1_MASK                         GENMASK(53, 53)
#define HFGITR_EL2_SVC_EL1_SHIFT                        53
#define HFGITR_EL2_SVC_EL1_WIDTH                        1

#define HFGITR_EL2_SVC_EL0                              GENMASK(52, 52)
#define HFGITR_EL2_SVC_EL0_MASK                         GENMASK(52, 52)
#define HFGITR_EL2_SVC_EL0_SHIFT                        52
#define HFGITR_EL2_SVC_EL0_WIDTH                        1

#define HFGITR_EL2_ERET                                 GENMASK(51, 51)
#define HFGITR_EL2_ERET_MASK                            GENMASK(51, 51)
#define HFGITR_EL2_ERET_SHIFT                           51
#define HFGITR_EL2_ERET_WIDTH                           1

#define HFGITR_EL2_CPPRCTX                              GENMASK(50, 50)
#define HFGITR_EL2_CPPRCTX_MASK                         GENMASK(50, 50)
#define HFGITR_EL2_CPPRCTX_SHIFT                        50
#define HFGITR_EL2_CPPRCTX_WIDTH                        1

#define HFGITR_EL2_DVPRCTX                              GENMASK(49, 49)
#define HFGITR_EL2_DVPRCTX_MASK                         GENMASK(49, 49)
#define HFGITR_EL2_DVPRCTX_SHIFT                        49
#define HFGITR_EL2_DVPRCTX_WIDTH                        1

#define HFGITR_EL2_CFPRCTX                              GENMASK(48, 48)
#define HFGITR_EL2_CFPRCTX_MASK                         GENMASK(48, 48)
#define HFGITR_EL2_CFPRCTX_SHIFT                        48
#define HFGITR_EL2_CFPRCTX_WIDTH                        1

#define HFGITR_EL2_TLBIVAALE1                           GENMASK(47, 47)
#define HFGITR_EL2_TLBIVAALE1_MASK                      GENMASK(47, 47)
#define HFGITR_EL2_TLBIVAALE1_SHIFT                     47
#define HFGITR_EL2_TLBIVAALE1_WIDTH                     1

#define HFGITR_EL2_TLBIVALE1                            GENMASK(46, 46)
#define HFGITR_EL2_TLBIVALE1_MASK                       GENMASK(46, 46)
#define HFGITR_EL2_TLBIVALE1_SHIFT                      46
#define HFGITR_EL2_TLBIVALE1_WIDTH                      1

#define HFGITR_EL2_TLBIVAAE1                            GENMASK(45, 45)
#define HFGITR_EL2_TLBIVAAE1_MASK                       GENMASK(45, 45)
#define HFGITR_EL2_TLBIVAAE1_SHIFT                      45
#define HFGITR_EL2_TLBIVAAE1_WIDTH                      1

#define HFGITR_EL2_TLBIASIDE1                           GENMASK(44, 44)
#define HFGITR_EL2_TLBIASIDE1_MASK                      GENMASK(44, 44)
#define HFGITR_EL2_TLBIASIDE1_SHIFT                     44
#define HFGITR_EL2_TLBIASIDE1_WIDTH                     1

#define HFGITR_EL2_TLBIVAE1                             GENMASK(43, 43)
#define HFGITR_EL2_TLBIVAE1_MASK                        GENMASK(43, 43)
#define HFGITR_EL2_TLBIVAE1_SHIFT                       43
#define HFGITR_EL2_TLBIVAE1_WIDTH                       1

#define HFGITR_EL2_TLBIVMALLE1                          GENMASK(42, 42)
#define HFGITR_EL2_TLBIVMALLE1_MASK                     GENMASK(42, 42)
#define HFGITR_EL2_TLBIVMALLE1_SHIFT                    42
#define HFGITR_EL2_TLBIVMALLE1_WIDTH                    1

#define HFGITR_EL2_TLBIRVAALE1                          GENMASK(41, 41)
#define HFGITR_EL2_TLBIRVAALE1_MASK                     GENMASK(41, 41)
#define HFGITR_EL2_TLBIRVAALE1_SHIFT                    41
#define HFGITR_EL2_TLBIRVAALE1_WIDTH                    1

#define HFGITR_EL2_TLBIRVALE1                           GENMASK(40, 40)
#define HFGITR_EL2_TLBIRVALE1_MASK                      GENMASK(40, 40)
#define HFGITR_EL2_TLBIRVALE1_SHIFT                     40
#define HFGITR_EL2_TLBIRVALE1_WIDTH                     1

#define HFGITR_EL2_TLBIRVAAE1                           GENMASK(39, 39)
#define HFGITR_EL2_TLBIRVAAE1_MASK                      GENMASK(39, 39)
#define HFGITR_EL2_TLBIRVAAE1_SHIFT                     39
#define HFGITR_EL2_TLBIRVAAE1_WIDTH                     1

#define HFGITR_EL2_TLBIRVAE1                            GENMASK(38, 38)
#define HFGITR_EL2_TLBIRVAE1_MASK                       GENMASK(38, 38)
#define HFGITR_EL2_TLBIRVAE1_SHIFT                      38
#define HFGITR_EL2_TLBIRVAE1_WIDTH                      1

#define HFGITR_EL2_TLBIRVAALE1IS                        GENMASK(37, 37)
#define HFGITR_EL2_TLBIRVAALE1IS_MASK                   GENMASK(37, 37)
#define HFGITR_EL2_TLBIRVAALE1IS_SHIFT                  37
#define HFGITR_EL2_TLBIRVAALE1IS_WIDTH                  1

#define HFGITR_EL2_TLBIRVALE1IS                         GENMASK(36, 36)
#define HFGITR_EL2_TLBIRVALE1IS_MASK                    GENMASK(36, 36)
#define HFGITR_EL2_TLBIRVALE1IS_SHIFT                   36
#define HFGITR_EL2_TLBIRVALE1IS_WIDTH                   1

#define HFGITR_EL2_TLBIRVAAE1IS                         GENMASK(35, 35)
#define HFGITR_EL2_TLBIRVAAE1IS_MASK                    GENMASK(35, 35)
#define HFGITR_EL2_TLBIRVAAE1IS_SHIFT                   35
#define HFGITR_EL2_TLBIRVAAE1IS_WIDTH                   1

#define HFGITR_EL2_TLBIRVAE1IS                          GENMASK(34, 34)
#define HFGITR_EL2_TLBIRVAE1IS_MASK                     GENMASK(34, 34)
#define HFGITR_EL2_TLBIRVAE1IS_SHIFT                    34
#define HFGITR_EL2_TLBIRVAE1IS_WIDTH                    1

#define HFGITR_EL2_TLBIVAALE1IS                         GENMASK(33, 33)
#define HFGITR_EL2_TLBIVAALE1IS_MASK                    GENMASK(33, 33)
#define HFGITR_EL2_TLBIVAALE1IS_SHIFT                   33
#define HFGITR_EL2_TLBIVAALE1IS_WIDTH                   1

#define HFGITR_EL2_TLBIVALE1IS                          GENMASK(32, 32)
#define HFGITR_EL2_TLBIVALE1IS_MASK                     GENMASK(32, 32)
#define HFGITR_EL2_TLBIVALE1IS_SHIFT                    32
#define HFGITR_EL2_TLBIVALE1IS_WIDTH                    1

#define HFGITR_EL2_TLBIVAAE1IS                          GENMASK(31, 31)
#define HFGITR_EL2_TLBIVAAE1IS_MASK                     GENMASK(31, 31)
#define HFGITR_EL2_TLBIVAAE1IS_SHIFT                    31
#define HFGITR_EL2_TLBIVAAE1IS_WIDTH                    1

#define HFGITR_EL2_TLBIASIDE1IS                         GENMASK(30, 30)
#define HFGITR_EL2_TLBIASIDE1IS_MASK                    GENMASK(30, 30)
#define HFGITR_EL2_TLBIASIDE1IS_SHIFT                   30
#define HFGITR_EL2_TLBIASIDE1IS_WIDTH                   1

#define HFGITR_EL2_TLBIVAE1IS                           GENMASK(29, 29)
#define HFGITR_EL2_TLBIVAE1IS_MASK                      GENMASK(29, 29)
#define HFGITR_EL2_TLBIVAE1IS_SHIFT                     29
#define HFGITR_EL2_TLBIVAE1IS_WIDTH                     1

#define HFGITR_EL2_TLBIVMALLE1IS                        GENMASK(28, 28)
#define HFGITR_EL2_TLBIVMALLE1IS_MASK                   GENMASK(28, 28)
#define HFGITR_EL2_TLBIVMALLE1IS_SHIFT                  28
#define HFGITR_EL2_TLBIVMALLE1IS_WIDTH                  1

#define HFGITR_EL2_TLBIRVAALE1OS                        GENMASK(27, 27)
#define HFGITR_EL2_TLBIRVAALE1OS_MASK                   GENMASK(27, 27)
#define HFGITR_EL2_TLBIRVAALE1OS_SHIFT                  27
#define HFGITR_EL2_TLBIRVAALE1OS_WIDTH                  1

#define HFGITR_EL2_TLBIRVALE1OS                         GENMASK(26, 26)
#define HFGITR_EL2_TLBIRVALE1OS_MASK                    GENMASK(26, 26)
#define HFGITR_EL2_TLBIRVALE1OS_SHIFT                   26
#define HFGITR_EL2_TLBIRVALE1OS_WIDTH                   1

#define HFGITR_EL2_TLBIRVAAE1OS                         GENMASK(25, 25)
#define HFGITR_EL2_TLBIRVAAE1OS_MASK                    GENMASK(25, 25)
#define HFGITR_EL2_TLBIRVAAE1OS_SHIFT                   25
#define HFGITR_EL2_TLBIRVAAE1OS_WIDTH                   1

#define HFGITR_EL2_TLBIRVAE1OS                          GENMASK(24, 24)
#define HFGITR_EL2_TLBIRVAE1OS_MASK                     GENMASK(24, 24)
#define HFGITR_EL2_TLBIRVAE1OS_SHIFT                    24
#define HFGITR_EL2_TLBIRVAE1OS_WIDTH                    1

#define HFGITR_EL2_TLBIVAALE1OS                         GENMASK(23, 23)
#define HFGITR_EL2_TLBIVAALE1OS_MASK                    GENMASK(23, 23)
#define HFGITR_EL2_TLBIVAALE1OS_SHIFT                   23
#define HFGITR_EL2_TLBIVAALE1OS_WIDTH                   1

#define HFGITR_EL2_TLBIVALE1OS                          GENMASK(22, 22)
#define HFGITR_EL2_TLBIVALE1OS_MASK                     GENMASK(22, 22)
#define HFGITR_EL2_TLBIVALE1OS_SHIFT                    22
#define HFGITR_EL2_TLBIVALE1OS_WIDTH                    1

#define HFGITR_EL2_TLBIVAAE1OS                          GENMASK(21, 21)
#define HFGITR_EL2_TLBIVAAE1OS_MASK                     GENMASK(21, 21)
#define HFGITR_EL2_TLBIVAAE1OS_SHIFT                    21
#define HFGITR_EL2_TLBIVAAE1OS_WIDTH                    1

#define HFGITR_EL2_TLBIASIDE1OS                         GENMASK(20, 20)
#define HFGITR_EL2_TLBIASIDE1OS_MASK                    GENMASK(20, 20)
#define HFGITR_EL2_TLBIASIDE1OS_SHIFT                   20
#define HFGITR_EL2_TLBIASIDE1OS_WIDTH                   1

#define HFGITR_EL2_TLBIVAE1OS                           GENMASK(19, 19)
#define HFGITR_EL2_TLBIVAE1OS_MASK                      GENMASK(19, 19)
#define HFGITR_EL2_TLBIVAE1OS_SHIFT                     19
#define HFGITR_EL2_TLBIVAE1OS_WIDTH                     1

#define HFGITR_EL2_TLBIVMALLE1OS                        GENMASK(18, 18)
#define HFGITR_EL2_TLBIVMALLE1OS_MASK                   GENMASK(18, 18)
#define HFGITR_EL2_TLBIVMALLE1OS_SHIFT                  18
#define HFGITR_EL2_TLBIVMALLE1OS_WIDTH                  1

#define HFGITR_EL2_ATS1E1WP                             GENMASK(17, 17)
#define HFGITR_EL2_ATS1E1WP_MASK                        GENMASK(17, 17)
#define HFGITR_EL2_ATS1E1WP_SHIFT                       17
#define HFGITR_EL2_ATS1E1WP_WIDTH                       1

#define HFGITR_EL2_ATS1E1RP                             GENMASK(16, 16)
#define HFGITR_EL2_ATS1E1RP_MASK                        GENMASK(16, 16)
#define HFGITR_EL2_ATS1E1RP_SHIFT                       16
#define HFGITR_EL2_ATS1E1RP_WIDTH                       1

#define HFGITR_EL2_ATS1E0W                              GENMASK(15, 15)
#define HFGITR_EL2_ATS1E0W_MASK                         GENMASK(15, 15)
#define HFGITR_EL2_ATS1E0W_SHIFT                        15
#define HFGITR_EL2_ATS1E0W_WIDTH                        1

#define HFGITR_EL2_ATS1E0R                              GENMASK(14, 14)
#define HFGITR_EL2_ATS1E0R_MASK                         GENMASK(14, 14)
#define HFGITR_EL2_ATS1E0R_SHIFT                        14
#define HFGITR_EL2_ATS1E0R_WIDTH                        1

#define HFGITR_EL2_ATS1E1W                              GENMASK(13, 13)
#define HFGITR_EL2_ATS1E1W_MASK                         GENMASK(13, 13)
#define HFGITR_EL2_ATS1E1W_SHIFT                        13
#define HFGITR_EL2_ATS1E1W_WIDTH                        1

#define HFGITR_EL2_ATS1E1R                              GENMASK(12, 12)
#define HFGITR_EL2_ATS1E1R_MASK                         GENMASK(12, 12)
#define HFGITR_EL2_ATS1E1R_SHIFT                        12
#define HFGITR_EL2_ATS1E1R_WIDTH                        1

#define HFGITR_EL2_DCZVA                                GENMASK(11, 11)
#define HFGITR_EL2_DCZVA_MASK                           GENMASK(11, 11)
#define HFGITR_EL2_DCZVA_SHIFT                          11
#define HFGITR_EL2_DCZVA_WIDTH                          1

#define HFGITR_EL2_DCCIVAC                              GENMASK(10, 10)
#define HFGITR_EL2_DCCIVAC_MASK                         GENMASK(10, 10)
#define HFGITR_EL2_DCCIVAC_SHIFT                        10
#define HFGITR_EL2_DCCIVAC_WIDTH                        1

#define HFGITR_EL2_DCCVADP                              GENMASK(9, 9)
#define HFGITR_EL2_DCCVADP_MASK                         GENMASK(9, 9)
#define HFGITR_EL2_DCCVADP_SHIFT                        9
#define HFGITR_EL2_DCCVADP_WIDTH                        1

#define HFGITR_EL2_DCCVAP                               GENMASK(8, 8)
#define HFGITR_EL2_DCCVAP_MASK                          GENMASK(8, 8)
#define HFGITR_EL2_DCCVAP_SHIFT                         8
#define HFGITR_EL2_DCCVAP_WIDTH                         1

#define HFGITR_EL2_DCCVAU                               GENMASK(7, 7)
#define HFGITR_EL2_DCCVAU_MASK                          GENMASK(7, 7)
#define HFGITR_EL2_DCCVAU_SHIFT                         7
#define HFGITR_EL2_DCCVAU_WIDTH                         1

#define HFGITR_EL2_DCCISW                               GENMASK(6, 6)
#define HFGITR_EL2_DCCISW_MASK                          GENMASK(6, 6)
#define HFGITR_EL2_DCCISW_SHIFT                         6
#define HFGITR_EL2_DCCISW_WIDTH                         1

#define HFGITR_EL2_DCCSW                                GENMASK(5, 5)
#define HFGITR_EL2_DCCSW_MASK                           GENMASK(5, 5)
#define HFGITR_EL2_DCCSW_SHIFT                          5
#define HFGITR_EL2_DCCSW_WIDTH                          1

#define HFGITR_EL2_DCISW                                GENMASK(4, 4)
#define HFGITR_EL2_DCISW_MASK                           GENMASK(4, 4)
#define HFGITR_EL2_DCISW_SHIFT                          4
#define HFGITR_EL2_DCISW_WIDTH                          1

#define HFGITR_EL2_DCIVAC                               GENMASK(3, 3)
#define HFGITR_EL2_DCIVAC_MASK                          GENMASK(3, 3)
#define HFGITR_EL2_DCIVAC_SHIFT                         3
#define HFGITR_EL2_DCIVAC_WIDTH                         1

#define HFGITR_EL2_ICIVAU                               GENMASK(2, 2)
#define HFGITR_EL2_ICIVAU_MASK                          GENMASK(2, 2)
#define HFGITR_EL2_ICIVAU_SHIFT                         2
#define HFGITR_EL2_ICIVAU_WIDTH                         1

#define HFGITR_EL2_ICIALLU                              GENMASK(1, 1)
#define HFGITR_EL2_ICIALLU_MASK                         GENMASK(1, 1)
#define HFGITR_EL2_ICIALLU_SHIFT                        1
#define HFGITR_EL2_ICIALLU_WIDTH                        1

#define HFGITR_EL2_ICIALLUIS                            GENMASK(0, 0)
#define HFGITR_EL2_ICIALLUIS_MASK                       GENMASK(0, 0)
#define HFGITR_EL2_ICIALLUIS_SHIFT                      0
#define HFGITR_EL2_ICIALLUIS_WIDTH                      1

#define HFGITR_EL2_RES0                                 (UL(0) | GENMASK_ULL(61, 61))
#define HFGITR_EL2_RES1                                 (UL(0))
#define HFGITR_EL2_UNKN                                 (UL(0))

#define REG_TRFCR_EL2                                   S3_4_C1_C2_1
#define SYS_TRFCR_EL2                                   sys_reg(3, 4, 1, 2, 1)
#define SYS_TRFCR_EL2_Op0                               3
#define SYS_TRFCR_EL2_Op1                               4
#define SYS_TRFCR_EL2_CRn                               1
#define SYS_TRFCR_EL2_CRm                               2
#define SYS_TRFCR_EL2_Op2                               1

#define TRFCR_EL2_TS                                    GENMASK(6, 5)
#define TRFCR_EL2_TS_MASK                               GENMASK(6, 5)
#define TRFCR_EL2_TS_SHIFT                              5
#define TRFCR_EL2_TS_WIDTH                              2
#define TRFCR_EL2_TS_SIGNED                             false
#define TRFCR_EL2_TS_USE_TRFCR_EL1_TS                   UL(0b0000)
#define TRFCR_EL2_TS_VIRTUAL                            UL(0b0001)
#define TRFCR_EL2_TS_GUEST_PHYSICAL                     UL(0b0010)
#define TRFCR_EL2_TS_PHYSICAL                           UL(0b0011)

#define TRFCR_EL2_CX                                    GENMASK(3, 3)
#define TRFCR_EL2_CX_MASK                               GENMASK(3, 3)
#define TRFCR_EL2_CX_SHIFT                              3
#define TRFCR_EL2_CX_WIDTH                              1

#define TRFCR_EL2_E2TRE                                 GENMASK(1, 1)
#define TRFCR_EL2_E2TRE_MASK                            GENMASK(1, 1)
#define TRFCR_EL2_E2TRE_SHIFT                           1
#define TRFCR_EL2_E2TRE_WIDTH                           1

#define TRFCR_EL2_E0HTRE                                GENMASK(0, 0)
#define TRFCR_EL2_E0HTRE_MASK                           GENMASK(0, 0)
#define TRFCR_EL2_E0HTRE_SHIFT                          0
#define TRFCR_EL2_E0HTRE_WIDTH                          1

#define TRFCR_EL2_RES0                                  (UL(0) | GENMASK_ULL(63, 7) | GENMASK_ULL(4, 4) | GENMASK_ULL(2, 2))
#define TRFCR_EL2_RES1                                  (UL(0))
#define TRFCR_EL2_UNKN                                  (UL(0))

#define REG_HDFGRTR2_EL2                                S3_4_C3_C1_0
#define SYS_HDFGRTR2_EL2                                sys_reg(3, 4, 3, 1, 0)
#define SYS_HDFGRTR2_EL2_Op0                            3
#define SYS_HDFGRTR2_EL2_Op1                            4
#define SYS_HDFGRTR2_EL2_CRn                            3
#define SYS_HDFGRTR2_EL2_CRm                            1
#define SYS_HDFGRTR2_EL2_Op2                            0

#define HDFGRTR2_EL2_nPMBMAR_EL1                        GENMASK(24, 24)
#define HDFGRTR2_EL2_nPMBMAR_EL1_MASK                   GENMASK(24, 24)
#define HDFGRTR2_EL2_nPMBMAR_EL1_SHIFT                  24
#define HDFGRTR2_EL2_nPMBMAR_EL1_WIDTH                  1

#define HDFGRTR2_EL2_nMDSTEPOP_EL1                      GENMASK(23, 23)
#define HDFGRTR2_EL2_nMDSTEPOP_EL1_MASK                 GENMASK(23, 23)
#define HDFGRTR2_EL2_nMDSTEPOP_EL1_SHIFT                23
#define HDFGRTR2_EL2_nMDSTEPOP_EL1_WIDTH                1

#define HDFGRTR2_EL2_nTRBMPAM_EL1                       GENMASK(22, 22)
#define HDFGRTR2_EL2_nTRBMPAM_EL1_MASK                  GENMASK(22, 22)
#define HDFGRTR2_EL2_nTRBMPAM_EL1_SHIFT                 22
#define HDFGRTR2_EL2_nTRBMPAM_EL1_WIDTH                 1

#define HDFGRTR2_EL2_nTRCITECR_EL1                      GENMASK(20, 20)
#define HDFGRTR2_EL2_nTRCITECR_EL1_MASK                 GENMASK(20, 20)
#define HDFGRTR2_EL2_nTRCITECR_EL1_SHIFT                20
#define HDFGRTR2_EL2_nTRCITECR_EL1_WIDTH                1

#define HDFGRTR2_EL2_nPMSDSFR_EL1                       GENMASK(19, 19)
#define HDFGRTR2_EL2_nPMSDSFR_EL1_MASK                  GENMASK(19, 19)
#define HDFGRTR2_EL2_nPMSDSFR_EL1_SHIFT                 19
#define HDFGRTR2_EL2_nPMSDSFR_EL1_WIDTH                 1

#define HDFGRTR2_EL2_nSPMDEVAFF_EL1                     GENMASK(18, 18)
#define HDFGRTR2_EL2_nSPMDEVAFF_EL1_MASK                GENMASK(18, 18)
#define HDFGRTR2_EL2_nSPMDEVAFF_EL1_SHIFT               18
#define HDFGRTR2_EL2_nSPMDEVAFF_EL1_WIDTH               1

#define HDFGRTR2_EL2_nSPMID                             GENMASK(17, 17)
#define HDFGRTR2_EL2_nSPMID_MASK                        GENMASK(17, 17)
#define HDFGRTR2_EL2_nSPMID_SHIFT                       17
#define HDFGRTR2_EL2_nSPMID_WIDTH                       1

#define HDFGRTR2_EL2_nSPMSCR_EL1                        GENMASK(16, 16)
#define HDFGRTR2_EL2_nSPMSCR_EL1_MASK                   GENMASK(16, 16)
#define HDFGRTR2_EL2_nSPMSCR_EL1_SHIFT                  16
#define HDFGRTR2_EL2_nSPMSCR_EL1_WIDTH                  1

#define HDFGRTR2_EL2_nSPMACCESSR_EL1                    GENMASK(15, 15)
#define HDFGRTR2_EL2_nSPMACCESSR_EL1_MASK               GENMASK(15, 15)
#define HDFGRTR2_EL2_nSPMACCESSR_EL1_SHIFT              15
#define HDFGRTR2_EL2_nSPMACCESSR_EL1_WIDTH              1

#define HDFGRTR2_EL2_nSPMCR_EL0                         GENMASK(14, 14)
#define HDFGRTR2_EL2_nSPMCR_EL0_MASK                    GENMASK(14, 14)
#define HDFGRTR2_EL2_nSPMCR_EL0_SHIFT                   14
#define HDFGRTR2_EL2_nSPMCR_EL0_WIDTH                   1

#define HDFGRTR2_EL2_nSPMOVS                            GENMASK(13, 13)
#define HDFGRTR2_EL2_nSPMOVS_MASK                       GENMASK(13, 13)
#define HDFGRTR2_EL2_nSPMOVS_SHIFT                      13
#define HDFGRTR2_EL2_nSPMOVS_WIDTH                      1

#define HDFGRTR2_EL2_nSPMINTEN                          GENMASK(12, 12)
#define HDFGRTR2_EL2_nSPMINTEN_MASK                     GENMASK(12, 12)
#define HDFGRTR2_EL2_nSPMINTEN_SHIFT                    12
#define HDFGRTR2_EL2_nSPMINTEN_WIDTH                    1

#define HDFGRTR2_EL2_nSPMCNTEN                          GENMASK(11, 11)
#define HDFGRTR2_EL2_nSPMCNTEN_MASK                     GENMASK(11, 11)
#define HDFGRTR2_EL2_nSPMCNTEN_SHIFT                    11
#define HDFGRTR2_EL2_nSPMCNTEN_WIDTH                    1

#define HDFGRTR2_EL2_nSPMSELR_EL0                       GENMASK(10, 10)
#define HDFGRTR2_EL2_nSPMSELR_EL0_MASK                  GENMASK(10, 10)
#define HDFGRTR2_EL2_nSPMSELR_EL0_SHIFT                 10
#define HDFGRTR2_EL2_nSPMSELR_EL0_WIDTH                 1

#define HDFGRTR2_EL2_nSPMEVTYPERn_EL0                   GENMASK(9, 9)
#define HDFGRTR2_EL2_nSPMEVTYPERn_EL0_MASK              GENMASK(9, 9)
#define HDFGRTR2_EL2_nSPMEVTYPERn_EL0_SHIFT             9
#define HDFGRTR2_EL2_nSPMEVTYPERn_EL0_WIDTH             1

#define HDFGRTR2_EL2_nSPMEVCNTRn_EL0                    GENMASK(8, 8)
#define HDFGRTR2_EL2_nSPMEVCNTRn_EL0_MASK               GENMASK(8, 8)
#define HDFGRTR2_EL2_nSPMEVCNTRn_EL0_SHIFT              8
#define HDFGRTR2_EL2_nSPMEVCNTRn_EL0_WIDTH              1

#define HDFGRTR2_EL2_nPMSSCR_EL1                        GENMASK(7, 7)
#define HDFGRTR2_EL2_nPMSSCR_EL1_MASK                   GENMASK(7, 7)
#define HDFGRTR2_EL2_nPMSSCR_EL1_SHIFT                  7
#define HDFGRTR2_EL2_nPMSSCR_EL1_WIDTH                  1

#define HDFGRTR2_EL2_nPMSSDATA                          GENMASK(6, 6)
#define HDFGRTR2_EL2_nPMSSDATA_MASK                     GENMASK(6, 6)
#define HDFGRTR2_EL2_nPMSSDATA_SHIFT                    6
#define HDFGRTR2_EL2_nPMSSDATA_WIDTH                    1

#define HDFGRTR2_EL2_nMDSELR_EL1                        GENMASK(5, 5)
#define HDFGRTR2_EL2_nMDSELR_EL1_MASK                   GENMASK(5, 5)
#define HDFGRTR2_EL2_nMDSELR_EL1_SHIFT                  5
#define HDFGRTR2_EL2_nMDSELR_EL1_WIDTH                  1

#define HDFGRTR2_EL2_nPMUACR_EL1                        GENMASK(4, 4)
#define HDFGRTR2_EL2_nPMUACR_EL1_MASK                   GENMASK(4, 4)
#define HDFGRTR2_EL2_nPMUACR_EL1_SHIFT                  4
#define HDFGRTR2_EL2_nPMUACR_EL1_WIDTH                  1

#define HDFGRTR2_EL2_nPMICFILTR_EL0                     GENMASK(3, 3)
#define HDFGRTR2_EL2_nPMICFILTR_EL0_MASK                GENMASK(3, 3)
#define HDFGRTR2_EL2_nPMICFILTR_EL0_SHIFT               3
#define HDFGRTR2_EL2_nPMICFILTR_EL0_WIDTH               1

#define HDFGRTR2_EL2_nPMICNTR_EL0                       GENMASK(2, 2)
#define HDFGRTR2_EL2_nPMICNTR_EL0_MASK                  GENMASK(2, 2)
#define HDFGRTR2_EL2_nPMICNTR_EL0_SHIFT                 2
#define HDFGRTR2_EL2_nPMICNTR_EL0_WIDTH                 1

#define HDFGRTR2_EL2_nPMIAR_EL1                         GENMASK(1, 1)
#define HDFGRTR2_EL2_nPMIAR_EL1_MASK                    GENMASK(1, 1)
#define HDFGRTR2_EL2_nPMIAR_EL1_SHIFT                   1
#define HDFGRTR2_EL2_nPMIAR_EL1_WIDTH                   1

#define HDFGRTR2_EL2_nPMECR_EL1                         GENMASK(0, 0)
#define HDFGRTR2_EL2_nPMECR_EL1_MASK                    GENMASK(0, 0)
#define HDFGRTR2_EL2_nPMECR_EL1_SHIFT                   0
#define HDFGRTR2_EL2_nPMECR_EL1_WIDTH                   1

#define HDFGRTR2_EL2_RES0                               (UL(0) | GENMASK_ULL(63, 25) | GENMASK_ULL(21, 21))
#define HDFGRTR2_EL2_RES1                               (UL(0))
#define HDFGRTR2_EL2_UNKN                               (UL(0))

#define REG_HDFGWTR2_EL2                                S3_4_C3_C1_1
#define SYS_HDFGWTR2_EL2                                sys_reg(3, 4, 3, 1, 1)
#define SYS_HDFGWTR2_EL2_Op0                            3
#define SYS_HDFGWTR2_EL2_Op1                            4
#define SYS_HDFGWTR2_EL2_CRn                            3
#define SYS_HDFGWTR2_EL2_CRm                            1
#define SYS_HDFGWTR2_EL2_Op2                            1

#define HDFGWTR2_EL2_nPMBMAR_EL1                        GENMASK(24, 24)
#define HDFGWTR2_EL2_nPMBMAR_EL1_MASK                   GENMASK(24, 24)
#define HDFGWTR2_EL2_nPMBMAR_EL1_SHIFT                  24
#define HDFGWTR2_EL2_nPMBMAR_EL1_WIDTH                  1

#define HDFGWTR2_EL2_nMDSTEPOP_EL1                      GENMASK(23, 23)
#define HDFGWTR2_EL2_nMDSTEPOP_EL1_MASK                 GENMASK(23, 23)
#define HDFGWTR2_EL2_nMDSTEPOP_EL1_SHIFT                23
#define HDFGWTR2_EL2_nMDSTEPOP_EL1_WIDTH                1

#define HDFGWTR2_EL2_nTRBMPAM_EL1                       GENMASK(22, 22)
#define HDFGWTR2_EL2_nTRBMPAM_EL1_MASK                  GENMASK(22, 22)
#define HDFGWTR2_EL2_nTRBMPAM_EL1_SHIFT                 22
#define HDFGWTR2_EL2_nTRBMPAM_EL1_WIDTH                 1

#define HDFGWTR2_EL2_nPMZR_EL0                          GENMASK(21, 21)
#define HDFGWTR2_EL2_nPMZR_EL0_MASK                     GENMASK(21, 21)
#define HDFGWTR2_EL2_nPMZR_EL0_SHIFT                    21
#define HDFGWTR2_EL2_nPMZR_EL0_WIDTH                    1

#define HDFGWTR2_EL2_nTRCITECR_EL1                      GENMASK(20, 20)
#define HDFGWTR2_EL2_nTRCITECR_EL1_MASK                 GENMASK(20, 20)
#define HDFGWTR2_EL2_nTRCITECR_EL1_SHIFT                20
#define HDFGWTR2_EL2_nTRCITECR_EL1_WIDTH                1

#define HDFGWTR2_EL2_nPMSDSFR_EL1                       GENMASK(19, 19)
#define HDFGWTR2_EL2_nPMSDSFR_EL1_MASK                  GENMASK(19, 19)
#define HDFGWTR2_EL2_nPMSDSFR_EL1_SHIFT                 19
#define HDFGWTR2_EL2_nPMSDSFR_EL1_WIDTH                 1

#define HDFGWTR2_EL2_nSPMSCR_EL1                        GENMASK(16, 16)
#define HDFGWTR2_EL2_nSPMSCR_EL1_MASK                   GENMASK(16, 16)
#define HDFGWTR2_EL2_nSPMSCR_EL1_SHIFT                  16
#define HDFGWTR2_EL2_nSPMSCR_EL1_WIDTH                  1

#define HDFGWTR2_EL2_nSPMACCESSR_EL1                    GENMASK(15, 15)
#define HDFGWTR2_EL2_nSPMACCESSR_EL1_MASK               GENMASK(15, 15)
#define HDFGWTR2_EL2_nSPMACCESSR_EL1_SHIFT              15
#define HDFGWTR2_EL2_nSPMACCESSR_EL1_WIDTH              1

#define HDFGWTR2_EL2_nSPMCR_EL0                         GENMASK(14, 14)
#define HDFGWTR2_EL2_nSPMCR_EL0_MASK                    GENMASK(14, 14)
#define HDFGWTR2_EL2_nSPMCR_EL0_SHIFT                   14
#define HDFGWTR2_EL2_nSPMCR_EL0_WIDTH                   1

#define HDFGWTR2_EL2_nSPMOVS                            GENMASK(13, 13)
#define HDFGWTR2_EL2_nSPMOVS_MASK                       GENMASK(13, 13)
#define HDFGWTR2_EL2_nSPMOVS_SHIFT                      13
#define HDFGWTR2_EL2_nSPMOVS_WIDTH                      1

#define HDFGWTR2_EL2_nSPMINTEN                          GENMASK(12, 12)
#define HDFGWTR2_EL2_nSPMINTEN_MASK                     GENMASK(12, 12)
#define HDFGWTR2_EL2_nSPMINTEN_SHIFT                    12
#define HDFGWTR2_EL2_nSPMINTEN_WIDTH                    1

#define HDFGWTR2_EL2_nSPMCNTEN                          GENMASK(11, 11)
#define HDFGWTR2_EL2_nSPMCNTEN_MASK                     GENMASK(11, 11)
#define HDFGWTR2_EL2_nSPMCNTEN_SHIFT                    11
#define HDFGWTR2_EL2_nSPMCNTEN_WIDTH                    1

#define HDFGWTR2_EL2_nSPMSELR_EL0                       GENMASK(10, 10)
#define HDFGWTR2_EL2_nSPMSELR_EL0_MASK                  GENMASK(10, 10)
#define HDFGWTR2_EL2_nSPMSELR_EL0_SHIFT                 10
#define HDFGWTR2_EL2_nSPMSELR_EL0_WIDTH                 1

#define HDFGWTR2_EL2_nSPMEVTYPERn_EL0                   GENMASK(9, 9)
#define HDFGWTR2_EL2_nSPMEVTYPERn_EL0_MASK              GENMASK(9, 9)
#define HDFGWTR2_EL2_nSPMEVTYPERn_EL0_SHIFT             9
#define HDFGWTR2_EL2_nSPMEVTYPERn_EL0_WIDTH             1

#define HDFGWTR2_EL2_nSPMEVCNTRn_EL0                    GENMASK(8, 8)
#define HDFGWTR2_EL2_nSPMEVCNTRn_EL0_MASK               GENMASK(8, 8)
#define HDFGWTR2_EL2_nSPMEVCNTRn_EL0_SHIFT              8
#define HDFGWTR2_EL2_nSPMEVCNTRn_EL0_WIDTH              1

#define HDFGWTR2_EL2_nPMSSCR_EL1                        GENMASK(7, 7)
#define HDFGWTR2_EL2_nPMSSCR_EL1_MASK                   GENMASK(7, 7)
#define HDFGWTR2_EL2_nPMSSCR_EL1_SHIFT                  7
#define HDFGWTR2_EL2_nPMSSCR_EL1_WIDTH                  1

#define HDFGWTR2_EL2_nMDSELR_EL1                        GENMASK(5, 5)
#define HDFGWTR2_EL2_nMDSELR_EL1_MASK                   GENMASK(5, 5)
#define HDFGWTR2_EL2_nMDSELR_EL1_SHIFT                  5
#define HDFGWTR2_EL2_nMDSELR_EL1_WIDTH                  1

#define HDFGWTR2_EL2_nPMUACR_EL1                        GENMASK(4, 4)
#define HDFGWTR2_EL2_nPMUACR_EL1_MASK                   GENMASK(4, 4)
#define HDFGWTR2_EL2_nPMUACR_EL1_SHIFT                  4
#define HDFGWTR2_EL2_nPMUACR_EL1_WIDTH                  1

#define HDFGWTR2_EL2_nPMICFILTR_EL0                     GENMASK(3, 3)
#define HDFGWTR2_EL2_nPMICFILTR_EL0_MASK                GENMASK(3, 3)
#define HDFGWTR2_EL2_nPMICFILTR_EL0_SHIFT               3
#define HDFGWTR2_EL2_nPMICFILTR_EL0_WIDTH               1

#define HDFGWTR2_EL2_nPMICNTR_EL0                       GENMASK(2, 2)
#define HDFGWTR2_EL2_nPMICNTR_EL0_MASK                  GENMASK(2, 2)
#define HDFGWTR2_EL2_nPMICNTR_EL0_SHIFT                 2
#define HDFGWTR2_EL2_nPMICNTR_EL0_WIDTH                 1

#define HDFGWTR2_EL2_nPMIAR_EL1                         GENMASK(1, 1)
#define HDFGWTR2_EL2_nPMIAR_EL1_MASK                    GENMASK(1, 1)
#define HDFGWTR2_EL2_nPMIAR_EL1_SHIFT                   1
#define HDFGWTR2_EL2_nPMIAR_EL1_WIDTH                   1

#define HDFGWTR2_EL2_nPMECR_EL1                         GENMASK(0, 0)
#define HDFGWTR2_EL2_nPMECR_EL1_MASK                    GENMASK(0, 0)
#define HDFGWTR2_EL2_nPMECR_EL1_SHIFT                   0
#define HDFGWTR2_EL2_nPMECR_EL1_WIDTH                   1

#define HDFGWTR2_EL2_RES0                               (UL(0) | GENMASK_ULL(63, 25) | GENMASK_ULL(18, 17) | GENMASK_ULL(6, 6))
#define HDFGWTR2_EL2_RES1                               (UL(0))
#define HDFGWTR2_EL2_UNKN                               (UL(0))

#define REG_HFGRTR2_EL2                                 S3_4_C3_C1_2
#define SYS_HFGRTR2_EL2                                 sys_reg(3, 4, 3, 1, 2)
#define SYS_HFGRTR2_EL2_Op0                             3
#define SYS_HFGRTR2_EL2_Op1                             4
#define SYS_HFGRTR2_EL2_CRn                             3
#define SYS_HFGRTR2_EL2_CRm                             1
#define SYS_HFGRTR2_EL2_Op2                             2

#define HFGRTR2_EL2_nACTLRALIAS_EL1                     GENMASK(14, 14)
#define HFGRTR2_EL2_nACTLRALIAS_EL1_MASK                GENMASK(14, 14)
#define HFGRTR2_EL2_nACTLRALIAS_EL1_SHIFT               14
#define HFGRTR2_EL2_nACTLRALIAS_EL1_WIDTH               1

#define HFGRTR2_EL2_nACTLRMASK_EL1                      GENMASK(13, 13)
#define HFGRTR2_EL2_nACTLRMASK_EL1_MASK                 GENMASK(13, 13)
#define HFGRTR2_EL2_nACTLRMASK_EL1_SHIFT                13
#define HFGRTR2_EL2_nACTLRMASK_EL1_WIDTH                1

#define HFGRTR2_EL2_nTCR2ALIAS_EL1                      GENMASK(12, 12)
#define HFGRTR2_EL2_nTCR2ALIAS_EL1_MASK                 GENMASK(12, 12)
#define HFGRTR2_EL2_nTCR2ALIAS_EL1_SHIFT                12
#define HFGRTR2_EL2_nTCR2ALIAS_EL1_WIDTH                1

#define HFGRTR2_EL2_nTCRALIAS_EL1                       GENMASK(11, 11)
#define HFGRTR2_EL2_nTCRALIAS_EL1_MASK                  GENMASK(11, 11)
#define HFGRTR2_EL2_nTCRALIAS_EL1_SHIFT                 11
#define HFGRTR2_EL2_nTCRALIAS_EL1_WIDTH                 1

#define HFGRTR2_EL2_nSCTLRALIAS2_EL1                    GENMASK(10, 10)
#define HFGRTR2_EL2_nSCTLRALIAS2_EL1_MASK               GENMASK(10, 10)
#define HFGRTR2_EL2_nSCTLRALIAS2_EL1_SHIFT              10
#define HFGRTR2_EL2_nSCTLRALIAS2_EL1_WIDTH              1

#define HFGRTR2_EL2_nSCTLRALIAS_EL1                     GENMASK(9, 9)
#define HFGRTR2_EL2_nSCTLRALIAS_EL1_MASK                GENMASK(9, 9)
#define HFGRTR2_EL2_nSCTLRALIAS_EL1_SHIFT               9
#define HFGRTR2_EL2_nSCTLRALIAS_EL1_WIDTH               1

#define HFGRTR2_EL2_nCPACRALIAS_EL1                     GENMASK(8, 8)
#define HFGRTR2_EL2_nCPACRALIAS_EL1_MASK                GENMASK(8, 8)
#define HFGRTR2_EL2_nCPACRALIAS_EL1_SHIFT               8
#define HFGRTR2_EL2_nCPACRALIAS_EL1_WIDTH               1

#define HFGRTR2_EL2_nTCR2MASK_EL1                       GENMASK(7, 7)
#define HFGRTR2_EL2_nTCR2MASK_EL1_MASK                  GENMASK(7, 7)
#define HFGRTR2_EL2_nTCR2MASK_EL1_SHIFT                 7
#define HFGRTR2_EL2_nTCR2MASK_EL1_WIDTH                 1

#define HFGRTR2_EL2_nTCRMASK_EL1                        GENMASK(6, 6)
#define HFGRTR2_EL2_nTCRMASK_EL1_MASK                   GENMASK(6, 6)
#define HFGRTR2_EL2_nTCRMASK_EL1_SHIFT                  6
#define HFGRTR2_EL2_nTCRMASK_EL1_WIDTH                  1

#define HFGRTR2_EL2_nSCTLR2MASK_EL1                     GENMASK(5, 5)
#define HFGRTR2_EL2_nSCTLR2MASK_EL1_MASK                GENMASK(5, 5)
#define HFGRTR2_EL2_nSCTLR2MASK_EL1_SHIFT               5
#define HFGRTR2_EL2_nSCTLR2MASK_EL1_WIDTH               1

#define HFGRTR2_EL2_nSCTLRMASK_EL1                      GENMASK(4, 4)
#define HFGRTR2_EL2_nSCTLRMASK_EL1_MASK                 GENMASK(4, 4)
#define HFGRTR2_EL2_nSCTLRMASK_EL1_SHIFT                4
#define HFGRTR2_EL2_nSCTLRMASK_EL1_WIDTH                1

#define HFGRTR2_EL2_nCPACRMASK_EL1                      GENMASK(3, 3)
#define HFGRTR2_EL2_nCPACRMASK_EL1_MASK                 GENMASK(3, 3)
#define HFGRTR2_EL2_nCPACRMASK_EL1_SHIFT                3
#define HFGRTR2_EL2_nCPACRMASK_EL1_WIDTH                1

#define HFGRTR2_EL2_nRCWSMASK_EL1                       GENMASK(2, 2)
#define HFGRTR2_EL2_nRCWSMASK_EL1_MASK                  GENMASK(2, 2)
#define HFGRTR2_EL2_nRCWSMASK_EL1_SHIFT                 2
#define HFGRTR2_EL2_nRCWSMASK_EL1_WIDTH                 1

#define HFGRTR2_EL2_nERXGSR_EL1                         GENMASK(1, 1)
#define HFGRTR2_EL2_nERXGSR_EL1_MASK                    GENMASK(1, 1)
#define HFGRTR2_EL2_nERXGSR_EL1_SHIFT                   1
#define HFGRTR2_EL2_nERXGSR_EL1_WIDTH                   1

#define HFGRTR2_EL2_nPFAR_EL1                           GENMASK(0, 0)
#define HFGRTR2_EL2_nPFAR_EL1_MASK                      GENMASK(0, 0)
#define HFGRTR2_EL2_nPFAR_EL1_SHIFT                     0
#define HFGRTR2_EL2_nPFAR_EL1_WIDTH                     1

#define HFGRTR2_EL2_RES0                                (UL(0) | GENMASK_ULL(63, 15))
#define HFGRTR2_EL2_RES1                                (UL(0))
#define HFGRTR2_EL2_UNKN                                (UL(0))

#define REG_HFGWTR2_EL2                                 S3_4_C3_C1_3
#define SYS_HFGWTR2_EL2                                 sys_reg(3, 4, 3, 1, 3)
#define SYS_HFGWTR2_EL2_Op0                             3
#define SYS_HFGWTR2_EL2_Op1                             4
#define SYS_HFGWTR2_EL2_CRn                             3
#define SYS_HFGWTR2_EL2_CRm                             1
#define SYS_HFGWTR2_EL2_Op2                             3

#define HFGWTR2_EL2_nACTLRALIAS_EL1                     GENMASK(14, 14)
#define HFGWTR2_EL2_nACTLRALIAS_EL1_MASK                GENMASK(14, 14)
#define HFGWTR2_EL2_nACTLRALIAS_EL1_SHIFT               14
#define HFGWTR2_EL2_nACTLRALIAS_EL1_WIDTH               1

#define HFGWTR2_EL2_nACTLRMASK_EL1                      GENMASK(13, 13)
#define HFGWTR2_EL2_nACTLRMASK_EL1_MASK                 GENMASK(13, 13)
#define HFGWTR2_EL2_nACTLRMASK_EL1_SHIFT                13
#define HFGWTR2_EL2_nACTLRMASK_EL1_WIDTH                1

#define HFGWTR2_EL2_nTCR2ALIAS_EL1                      GENMASK(12, 12)
#define HFGWTR2_EL2_nTCR2ALIAS_EL1_MASK                 GENMASK(12, 12)
#define HFGWTR2_EL2_nTCR2ALIAS_EL1_SHIFT                12
#define HFGWTR2_EL2_nTCR2ALIAS_EL1_WIDTH                1

#define HFGWTR2_EL2_nTCRALIAS_EL1                       GENMASK(11, 11)
#define HFGWTR2_EL2_nTCRALIAS_EL1_MASK                  GENMASK(11, 11)
#define HFGWTR2_EL2_nTCRALIAS_EL1_SHIFT                 11
#define HFGWTR2_EL2_nTCRALIAS_EL1_WIDTH                 1

#define HFGWTR2_EL2_nSCTLRALIAS2_EL1                    GENMASK(10, 10)
#define HFGWTR2_EL2_nSCTLRALIAS2_EL1_MASK               GENMASK(10, 10)
#define HFGWTR2_EL2_nSCTLRALIAS2_EL1_SHIFT              10
#define HFGWTR2_EL2_nSCTLRALIAS2_EL1_WIDTH              1

#define HFGWTR2_EL2_nSCTLRALIAS_EL1                     GENMASK(9, 9)
#define HFGWTR2_EL2_nSCTLRALIAS_EL1_MASK                GENMASK(9, 9)
#define HFGWTR2_EL2_nSCTLRALIAS_EL1_SHIFT               9
#define HFGWTR2_EL2_nSCTLRALIAS_EL1_WIDTH               1

#define HFGWTR2_EL2_nCPACRALIAS_EL1                     GENMASK(8, 8)
#define HFGWTR2_EL2_nCPACRALIAS_EL1_MASK                GENMASK(8, 8)
#define HFGWTR2_EL2_nCPACRALIAS_EL1_SHIFT               8
#define HFGWTR2_EL2_nCPACRALIAS_EL1_WIDTH               1

#define HFGWTR2_EL2_nTCR2MASK_EL1                       GENMASK(7, 7)
#define HFGWTR2_EL2_nTCR2MASK_EL1_MASK                  GENMASK(7, 7)
#define HFGWTR2_EL2_nTCR2MASK_EL1_SHIFT                 7
#define HFGWTR2_EL2_nTCR2MASK_EL1_WIDTH                 1

#define HFGWTR2_EL2_nTCRMASK_EL1                        GENMASK(6, 6)
#define HFGWTR2_EL2_nTCRMASK_EL1_MASK                   GENMASK(6, 6)
#define HFGWTR2_EL2_nTCRMASK_EL1_SHIFT                  6
#define HFGWTR2_EL2_nTCRMASK_EL1_WIDTH                  1

#define HFGWTR2_EL2_nSCTLR2MASK_EL1                     GENMASK(5, 5)
#define HFGWTR2_EL2_nSCTLR2MASK_EL1_MASK                GENMASK(5, 5)
#define HFGWTR2_EL2_nSCTLR2MASK_EL1_SHIFT               5
#define HFGWTR2_EL2_nSCTLR2MASK_EL1_WIDTH               1

#define HFGWTR2_EL2_nSCTLRMASK_EL1                      GENMASK(4, 4)
#define HFGWTR2_EL2_nSCTLRMASK_EL1_MASK                 GENMASK(4, 4)
#define HFGWTR2_EL2_nSCTLRMASK_EL1_SHIFT                4
#define HFGWTR2_EL2_nSCTLRMASK_EL1_WIDTH                1

#define HFGWTR2_EL2_nCPACRMASK_EL1                      GENMASK(3, 3)
#define HFGWTR2_EL2_nCPACRMASK_EL1_MASK                 GENMASK(3, 3)
#define HFGWTR2_EL2_nCPACRMASK_EL1_SHIFT                3
#define HFGWTR2_EL2_nCPACRMASK_EL1_WIDTH                1

#define HFGWTR2_EL2_nRCWSMASK_EL1                       GENMASK(2, 2)
#define HFGWTR2_EL2_nRCWSMASK_EL1_MASK                  GENMASK(2, 2)
#define HFGWTR2_EL2_nRCWSMASK_EL1_SHIFT                 2
#define HFGWTR2_EL2_nRCWSMASK_EL1_WIDTH                 1

#define HFGWTR2_EL2_nPFAR_EL1                           GENMASK(0, 0)
#define HFGWTR2_EL2_nPFAR_EL1_MASK                      GENMASK(0, 0)
#define HFGWTR2_EL2_nPFAR_EL1_SHIFT                     0
#define HFGWTR2_EL2_nPFAR_EL1_WIDTH                     1

#define HFGWTR2_EL2_RES0                                (UL(0) | GENMASK_ULL(63, 15) | GENMASK_ULL(1, 1))
#define HFGWTR2_EL2_RES1                                (UL(0))
#define HFGWTR2_EL2_UNKN                                (UL(0))

#define REG_HDFGRTR_EL2                                 S3_4_C3_C1_4
#define SYS_HDFGRTR_EL2                                 sys_reg(3, 4, 3, 1, 4)
#define SYS_HDFGRTR_EL2_Op0                             3
#define SYS_HDFGRTR_EL2_Op1                             4
#define SYS_HDFGRTR_EL2_CRn                             3
#define SYS_HDFGRTR_EL2_CRm                             1
#define SYS_HDFGRTR_EL2_Op2                             4

#define HDFGRTR_EL2_PMBIDR_EL1                          GENMASK(63, 63)
#define HDFGRTR_EL2_PMBIDR_EL1_MASK                     GENMASK(63, 63)
#define HDFGRTR_EL2_PMBIDR_EL1_SHIFT                    63
#define HDFGRTR_EL2_PMBIDR_EL1_WIDTH                    1

#define HDFGRTR_EL2_nPMSNEVFR_EL1                       GENMASK(62, 62)
#define HDFGRTR_EL2_nPMSNEVFR_EL1_MASK                  GENMASK(62, 62)
#define HDFGRTR_EL2_nPMSNEVFR_EL1_SHIFT                 62
#define HDFGRTR_EL2_nPMSNEVFR_EL1_WIDTH                 1

#define HDFGRTR_EL2_nBRBDATA                            GENMASK(61, 61)
#define HDFGRTR_EL2_nBRBDATA_MASK                       GENMASK(61, 61)
#define HDFGRTR_EL2_nBRBDATA_SHIFT                      61
#define HDFGRTR_EL2_nBRBDATA_WIDTH                      1

#define HDFGRTR_EL2_nBRBCTL                             GENMASK(60, 60)
#define HDFGRTR_EL2_nBRBCTL_MASK                        GENMASK(60, 60)
#define HDFGRTR_EL2_nBRBCTL_SHIFT                       60
#define HDFGRTR_EL2_nBRBCTL_WIDTH                       1

#define HDFGRTR_EL2_nBRBIDR                             GENMASK(59, 59)
#define HDFGRTR_EL2_nBRBIDR_MASK                        GENMASK(59, 59)
#define HDFGRTR_EL2_nBRBIDR_SHIFT                       59
#define HDFGRTR_EL2_nBRBIDR_WIDTH                       1

#define HDFGRTR_EL2_PMCEIDn_EL0                         GENMASK(58, 58)
#define HDFGRTR_EL2_PMCEIDn_EL0_MASK                    GENMASK(58, 58)
#define HDFGRTR_EL2_PMCEIDn_EL0_SHIFT                   58
#define HDFGRTR_EL2_PMCEIDn_EL0_WIDTH                   1

#define HDFGRTR_EL2_PMUSERENR_EL0                       GENMASK(57, 57)
#define HDFGRTR_EL2_PMUSERENR_EL0_MASK                  GENMASK(57, 57)
#define HDFGRTR_EL2_PMUSERENR_EL0_SHIFT                 57
#define HDFGRTR_EL2_PMUSERENR_EL0_WIDTH                 1

#define HDFGRTR_EL2_TRBTRG_EL1                          GENMASK(56, 56)
#define HDFGRTR_EL2_TRBTRG_EL1_MASK                     GENMASK(56, 56)
#define HDFGRTR_EL2_TRBTRG_EL1_SHIFT                    56
#define HDFGRTR_EL2_TRBTRG_EL1_WIDTH                    1

#define HDFGRTR_EL2_TRBSR_EL1                           GENMASK(55, 55)
#define HDFGRTR_EL2_TRBSR_EL1_MASK                      GENMASK(55, 55)
#define HDFGRTR_EL2_TRBSR_EL1_SHIFT                     55
#define HDFGRTR_EL2_TRBSR_EL1_WIDTH                     1

#define HDFGRTR_EL2_TRBPTR_EL1                          GENMASK(54, 54)
#define HDFGRTR_EL2_TRBPTR_EL1_MASK                     GENMASK(54, 54)
#define HDFGRTR_EL2_TRBPTR_EL1_SHIFT                    54
#define HDFGRTR_EL2_TRBPTR_EL1_WIDTH                    1

#define HDFGRTR_EL2_TRBMAR_EL1                          GENMASK(53, 53)
#define HDFGRTR_EL2_TRBMAR_EL1_MASK                     GENMASK(53, 53)
#define HDFGRTR_EL2_TRBMAR_EL1_SHIFT                    53
#define HDFGRTR_EL2_TRBMAR_EL1_WIDTH                    1

#define HDFGRTR_EL2_TRBLIMITR_EL1                       GENMASK(52, 52)
#define HDFGRTR_EL2_TRBLIMITR_EL1_MASK                  GENMASK(52, 52)
#define HDFGRTR_EL2_TRBLIMITR_EL1_SHIFT                 52
#define HDFGRTR_EL2_TRBLIMITR_EL1_WIDTH                 1

#define HDFGRTR_EL2_TRBIDR_EL1                          GENMASK(51, 51)
#define HDFGRTR_EL2_TRBIDR_EL1_MASK                     GENMASK(51, 51)
#define HDFGRTR_EL2_TRBIDR_EL1_SHIFT                    51
#define HDFGRTR_EL2_TRBIDR_EL1_WIDTH                    1

#define HDFGRTR_EL2_TRBBASER_EL1                        GENMASK(50, 50)
#define HDFGRTR_EL2_TRBBASER_EL1_MASK                   GENMASK(50, 50)
#define HDFGRTR_EL2_TRBBASER_EL1_SHIFT                  50
#define HDFGRTR_EL2_TRBBASER_EL1_WIDTH                  1

#define HDFGRTR_EL2_TRCVICTLR                           GENMASK(48, 48)
#define HDFGRTR_EL2_TRCVICTLR_MASK                      GENMASK(48, 48)
#define HDFGRTR_EL2_TRCVICTLR_SHIFT                     48
#define HDFGRTR_EL2_TRCVICTLR_WIDTH                     1

#define HDFGRTR_EL2_TRCSTATR                            GENMASK(47, 47)
#define HDFGRTR_EL2_TRCSTATR_MASK                       GENMASK(47, 47)
#define HDFGRTR_EL2_TRCSTATR_SHIFT                      47
#define HDFGRTR_EL2_TRCSTATR_WIDTH                      1

#define HDFGRTR_EL2_TRCSSCSRn                           GENMASK(46, 46)
#define HDFGRTR_EL2_TRCSSCSRn_MASK                      GENMASK(46, 46)
#define HDFGRTR_EL2_TRCSSCSRn_SHIFT                     46
#define HDFGRTR_EL2_TRCSSCSRn_WIDTH                     1

#define HDFGRTR_EL2_TRCSEQSTR                           GENMASK(45, 45)
#define HDFGRTR_EL2_TRCSEQSTR_MASK                      GENMASK(45, 45)
#define HDFGRTR_EL2_TRCSEQSTR_SHIFT                     45
#define HDFGRTR_EL2_TRCSEQSTR_WIDTH                     1

#define HDFGRTR_EL2_TRCPRGCTLR                          GENMASK(44, 44)
#define HDFGRTR_EL2_TRCPRGCTLR_MASK                     GENMASK(44, 44)
#define HDFGRTR_EL2_TRCPRGCTLR_SHIFT                    44
#define HDFGRTR_EL2_TRCPRGCTLR_WIDTH                    1

#define HDFGRTR_EL2_TRCOSLSR                            GENMASK(43, 43)
#define HDFGRTR_EL2_TRCOSLSR_MASK                       GENMASK(43, 43)
#define HDFGRTR_EL2_TRCOSLSR_SHIFT                      43
#define HDFGRTR_EL2_TRCOSLSR_WIDTH                      1

#define HDFGRTR_EL2_TRCIMSPECn                          GENMASK(41, 41)
#define HDFGRTR_EL2_TRCIMSPECn_MASK                     GENMASK(41, 41)
#define HDFGRTR_EL2_TRCIMSPECn_SHIFT                    41
#define HDFGRTR_EL2_TRCIMSPECn_WIDTH                    1

#define HDFGRTR_EL2_TRCID                               GENMASK(40, 40)
#define HDFGRTR_EL2_TRCID_MASK                          GENMASK(40, 40)
#define HDFGRTR_EL2_TRCID_SHIFT                         40
#define HDFGRTR_EL2_TRCID_WIDTH                         1

#define HDFGRTR_EL2_TRCCNTVRn                           GENMASK(37, 37)
#define HDFGRTR_EL2_TRCCNTVRn_MASK                      GENMASK(37, 37)
#define HDFGRTR_EL2_TRCCNTVRn_SHIFT                     37
#define HDFGRTR_EL2_TRCCNTVRn_WIDTH                     1

#define HDFGRTR_EL2_TRCCLAIM                            GENMASK(36, 36)
#define HDFGRTR_EL2_TRCCLAIM_MASK                       GENMASK(36, 36)
#define HDFGRTR_EL2_TRCCLAIM_SHIFT                      36
#define HDFGRTR_EL2_TRCCLAIM_WIDTH                      1

#define HDFGRTR_EL2_TRCAUXCTLR                          GENMASK(35, 35)
#define HDFGRTR_EL2_TRCAUXCTLR_MASK                     GENMASK(35, 35)
#define HDFGRTR_EL2_TRCAUXCTLR_SHIFT                    35
#define HDFGRTR_EL2_TRCAUXCTLR_WIDTH                    1

#define HDFGRTR_EL2_TRCAUTHSTATUS                       GENMASK(34, 34)
#define HDFGRTR_EL2_TRCAUTHSTATUS_MASK                  GENMASK(34, 34)
#define HDFGRTR_EL2_TRCAUTHSTATUS_SHIFT                 34
#define HDFGRTR_EL2_TRCAUTHSTATUS_WIDTH                 1

#define HDFGRTR_EL2_TRC                                 GENMASK(33, 33)
#define HDFGRTR_EL2_TRC_MASK                            GENMASK(33, 33)
#define HDFGRTR_EL2_TRC_SHIFT                           33
#define HDFGRTR_EL2_TRC_WIDTH                           1

#define HDFGRTR_EL2_PMSLATFR_EL1                        GENMASK(32, 32)
#define HDFGRTR_EL2_PMSLATFR_EL1_MASK                   GENMASK(32, 32)
#define HDFGRTR_EL2_PMSLATFR_EL1_SHIFT                  32
#define HDFGRTR_EL2_PMSLATFR_EL1_WIDTH                  1

#define HDFGRTR_EL2_PMSIRR_EL1                          GENMASK(31, 31)
#define HDFGRTR_EL2_PMSIRR_EL1_MASK                     GENMASK(31, 31)
#define HDFGRTR_EL2_PMSIRR_EL1_SHIFT                    31
#define HDFGRTR_EL2_PMSIRR_EL1_WIDTH                    1

#define HDFGRTR_EL2_PMSIDR_EL1                          GENMASK(30, 30)
#define HDFGRTR_EL2_PMSIDR_EL1_MASK                     GENMASK(30, 30)
#define HDFGRTR_EL2_PMSIDR_EL1_SHIFT                    30
#define HDFGRTR_EL2_PMSIDR_EL1_WIDTH                    1

#define HDFGRTR_EL2_PMSICR_EL1                          GENMASK(29, 29)
#define HDFGRTR_EL2_PMSICR_EL1_MASK                     GENMASK(29, 29)
#define HDFGRTR_EL2_PMSICR_EL1_SHIFT                    29
#define HDFGRTR_EL2_PMSICR_EL1_WIDTH                    1

#define HDFGRTR_EL2_PMSFCR_EL1                          GENMASK(28, 28)
#define HDFGRTR_EL2_PMSFCR_EL1_MASK                     GENMASK(28, 28)
#define HDFGRTR_EL2_PMSFCR_EL1_SHIFT                    28
#define HDFGRTR_EL2_PMSFCR_EL1_WIDTH                    1

#define HDFGRTR_EL2_PMSEVFR_EL1                         GENMASK(27, 27)
#define HDFGRTR_EL2_PMSEVFR_EL1_MASK                    GENMASK(27, 27)
#define HDFGRTR_EL2_PMSEVFR_EL1_SHIFT                   27
#define HDFGRTR_EL2_PMSEVFR_EL1_WIDTH                   1

#define HDFGRTR_EL2_PMSCR_EL1                           GENMASK(26, 26)
#define HDFGRTR_EL2_PMSCR_EL1_MASK                      GENMASK(26, 26)
#define HDFGRTR_EL2_PMSCR_EL1_SHIFT                     26
#define HDFGRTR_EL2_PMSCR_EL1_WIDTH                     1

#define HDFGRTR_EL2_PMBSR_EL1                           GENMASK(25, 25)
#define HDFGRTR_EL2_PMBSR_EL1_MASK                      GENMASK(25, 25)
#define HDFGRTR_EL2_PMBSR_EL1_SHIFT                     25
#define HDFGRTR_EL2_PMBSR_EL1_WIDTH                     1

#define HDFGRTR_EL2_PMBPTR_EL1                          GENMASK(24, 24)
#define HDFGRTR_EL2_PMBPTR_EL1_MASK                     GENMASK(24, 24)
#define HDFGRTR_EL2_PMBPTR_EL1_SHIFT                    24
#define HDFGRTR_EL2_PMBPTR_EL1_WIDTH                    1

#define HDFGRTR_EL2_PMBLIMITR_EL1                       GENMASK(23, 23)
#define HDFGRTR_EL2_PMBLIMITR_EL1_MASK                  GENMASK(23, 23)
#define HDFGRTR_EL2_PMBLIMITR_EL1_SHIFT                 23
#define HDFGRTR_EL2_PMBLIMITR_EL1_WIDTH                 1

#define HDFGRTR_EL2_PMMIR_EL1                           GENMASK(22, 22)
#define HDFGRTR_EL2_PMMIR_EL1_MASK                      GENMASK(22, 22)
#define HDFGRTR_EL2_PMMIR_EL1_SHIFT                     22
#define HDFGRTR_EL2_PMMIR_EL1_WIDTH                     1

#define HDFGRTR_EL2_PMSELR_EL0                          GENMASK(19, 19)
#define HDFGRTR_EL2_PMSELR_EL0_MASK                     GENMASK(19, 19)
#define HDFGRTR_EL2_PMSELR_EL0_SHIFT                    19
#define HDFGRTR_EL2_PMSELR_EL0_WIDTH                    1

#define HDFGRTR_EL2_PMOVS                               GENMASK(18, 18)
#define HDFGRTR_EL2_PMOVS_MASK                          GENMASK(18, 18)
#define HDFGRTR_EL2_PMOVS_SHIFT                         18
#define HDFGRTR_EL2_PMOVS_WIDTH                         1

#define HDFGRTR_EL2_PMINTEN                             GENMASK(17, 17)
#define HDFGRTR_EL2_PMINTEN_MASK                        GENMASK(17, 17)
#define HDFGRTR_EL2_PMINTEN_SHIFT                       17
#define HDFGRTR_EL2_PMINTEN_WIDTH                       1

#define HDFGRTR_EL2_PMCNTEN                             GENMASK(16, 16)
#define HDFGRTR_EL2_PMCNTEN_MASK                        GENMASK(16, 16)
#define HDFGRTR_EL2_PMCNTEN_SHIFT                       16
#define HDFGRTR_EL2_PMCNTEN_WIDTH                       1

#define HDFGRTR_EL2_PMCCNTR_EL0                         GENMASK(15, 15)
#define HDFGRTR_EL2_PMCCNTR_EL0_MASK                    GENMASK(15, 15)
#define HDFGRTR_EL2_PMCCNTR_EL0_SHIFT                   15
#define HDFGRTR_EL2_PMCCNTR_EL0_WIDTH                   1

#define HDFGRTR_EL2_PMCCFILTR_EL0                       GENMASK(14, 14)
#define HDFGRTR_EL2_PMCCFILTR_EL0_MASK                  GENMASK(14, 14)
#define HDFGRTR_EL2_PMCCFILTR_EL0_SHIFT                 14
#define HDFGRTR_EL2_PMCCFILTR_EL0_WIDTH                 1

#define HDFGRTR_EL2_PMEVTYPERn_EL0                      GENMASK(13, 13)
#define HDFGRTR_EL2_PMEVTYPERn_EL0_MASK                 GENMASK(13, 13)
#define HDFGRTR_EL2_PMEVTYPERn_EL0_SHIFT                13
#define HDFGRTR_EL2_PMEVTYPERn_EL0_WIDTH                1

#define HDFGRTR_EL2_PMEVCNTRn_EL0                       GENMASK(12, 12)
#define HDFGRTR_EL2_PMEVCNTRn_EL0_MASK                  GENMASK(12, 12)
#define HDFGRTR_EL2_PMEVCNTRn_EL0_SHIFT                 12
#define HDFGRTR_EL2_PMEVCNTRn_EL0_WIDTH                 1

#define HDFGRTR_EL2_OSDLR_EL1                           GENMASK(11, 11)
#define HDFGRTR_EL2_OSDLR_EL1_MASK                      GENMASK(11, 11)
#define HDFGRTR_EL2_OSDLR_EL1_SHIFT                     11
#define HDFGRTR_EL2_OSDLR_EL1_WIDTH                     1

#define HDFGRTR_EL2_OSECCR_EL1                          GENMASK(10, 10)
#define HDFGRTR_EL2_OSECCR_EL1_MASK                     GENMASK(10, 10)
#define HDFGRTR_EL2_OSECCR_EL1_SHIFT                    10
#define HDFGRTR_EL2_OSECCR_EL1_WIDTH                    1

#define HDFGRTR_EL2_OSLSR_EL1                           GENMASK(9, 9)
#define HDFGRTR_EL2_OSLSR_EL1_MASK                      GENMASK(9, 9)
#define HDFGRTR_EL2_OSLSR_EL1_SHIFT                     9
#define HDFGRTR_EL2_OSLSR_EL1_WIDTH                     1

#define HDFGRTR_EL2_DBGPRCR_EL1                         GENMASK(7, 7)
#define HDFGRTR_EL2_DBGPRCR_EL1_MASK                    GENMASK(7, 7)
#define HDFGRTR_EL2_DBGPRCR_EL1_SHIFT                   7
#define HDFGRTR_EL2_DBGPRCR_EL1_WIDTH                   1

#define HDFGRTR_EL2_DBGAUTHSTATUS_EL1                   GENMASK(6, 6)
#define HDFGRTR_EL2_DBGAUTHSTATUS_EL1_MASK              GENMASK(6, 6)
#define HDFGRTR_EL2_DBGAUTHSTATUS_EL1_SHIFT             6
#define HDFGRTR_EL2_DBGAUTHSTATUS_EL1_WIDTH             1

#define HDFGRTR_EL2_DBGCLAIM                            GENMASK(5, 5)
#define HDFGRTR_EL2_DBGCLAIM_MASK                       GENMASK(5, 5)
#define HDFGRTR_EL2_DBGCLAIM_SHIFT                      5
#define HDFGRTR_EL2_DBGCLAIM_WIDTH                      1

#define HDFGRTR_EL2_MDSCR_EL1                           GENMASK(4, 4)
#define HDFGRTR_EL2_MDSCR_EL1_MASK                      GENMASK(4, 4)
#define HDFGRTR_EL2_MDSCR_EL1_SHIFT                     4
#define HDFGRTR_EL2_MDSCR_EL1_WIDTH                     1

#define HDFGRTR_EL2_DBGWVRn_EL1                         GENMASK(3, 3)
#define HDFGRTR_EL2_DBGWVRn_EL1_MASK                    GENMASK(3, 3)
#define HDFGRTR_EL2_DBGWVRn_EL1_SHIFT                   3
#define HDFGRTR_EL2_DBGWVRn_EL1_WIDTH                   1

#define HDFGRTR_EL2_DBGWCRn_EL1                         GENMASK(2, 2)
#define HDFGRTR_EL2_DBGWCRn_EL1_MASK                    GENMASK(2, 2)
#define HDFGRTR_EL2_DBGWCRn_EL1_SHIFT                   2
#define HDFGRTR_EL2_DBGWCRn_EL1_WIDTH                   1

#define HDFGRTR_EL2_DBGBVRn_EL1                         GENMASK(1, 1)
#define HDFGRTR_EL2_DBGBVRn_EL1_MASK                    GENMASK(1, 1)
#define HDFGRTR_EL2_DBGBVRn_EL1_SHIFT                   1
#define HDFGRTR_EL2_DBGBVRn_EL1_WIDTH                   1

#define HDFGRTR_EL2_DBGBCRn_EL1                         GENMASK(0, 0)
#define HDFGRTR_EL2_DBGBCRn_EL1_MASK                    GENMASK(0, 0)
#define HDFGRTR_EL2_DBGBCRn_EL1_SHIFT                   0
#define HDFGRTR_EL2_DBGBCRn_EL1_WIDTH                   1

#define HDFGRTR_EL2_RES0                                (UL(0) | GENMASK_ULL(49, 49) | GENMASK_ULL(42, 42) | GENMASK_ULL(39, 38) | GENMASK_ULL(21, 20) | GENMASK_ULL(8, 8))
#define HDFGRTR_EL2_RES1                                (UL(0))
#define HDFGRTR_EL2_UNKN                                (UL(0))

#define REG_HDFGWTR_EL2                                 S3_4_C3_C1_5
#define SYS_HDFGWTR_EL2                                 sys_reg(3, 4, 3, 1, 5)
#define SYS_HDFGWTR_EL2_Op0                             3
#define SYS_HDFGWTR_EL2_Op1                             4
#define SYS_HDFGWTR_EL2_CRn                             3
#define SYS_HDFGWTR_EL2_CRm                             1
#define SYS_HDFGWTR_EL2_Op2                             5

#define HDFGWTR_EL2_nPMSNEVFR_EL1                       GENMASK(62, 62)
#define HDFGWTR_EL2_nPMSNEVFR_EL1_MASK                  GENMASK(62, 62)
#define HDFGWTR_EL2_nPMSNEVFR_EL1_SHIFT                 62
#define HDFGWTR_EL2_nPMSNEVFR_EL1_WIDTH                 1

#define HDFGWTR_EL2_nBRBDATA                            GENMASK(61, 61)
#define HDFGWTR_EL2_nBRBDATA_MASK                       GENMASK(61, 61)
#define HDFGWTR_EL2_nBRBDATA_SHIFT                      61
#define HDFGWTR_EL2_nBRBDATA_WIDTH                      1

#define HDFGWTR_EL2_nBRBCTL                             GENMASK(60, 60)
#define HDFGWTR_EL2_nBRBCTL_MASK                        GENMASK(60, 60)
#define HDFGWTR_EL2_nBRBCTL_SHIFT                       60
#define HDFGWTR_EL2_nBRBCTL_WIDTH                       1

#define HDFGWTR_EL2_PMUSERENR_EL0                       GENMASK(57, 57)
#define HDFGWTR_EL2_PMUSERENR_EL0_MASK                  GENMASK(57, 57)
#define HDFGWTR_EL2_PMUSERENR_EL0_SHIFT                 57
#define HDFGWTR_EL2_PMUSERENR_EL0_WIDTH                 1

#define HDFGWTR_EL2_TRBTRG_EL1                          GENMASK(56, 56)
#define HDFGWTR_EL2_TRBTRG_EL1_MASK                     GENMASK(56, 56)
#define HDFGWTR_EL2_TRBTRG_EL1_SHIFT                    56
#define HDFGWTR_EL2_TRBTRG_EL1_WIDTH                    1

#define HDFGWTR_EL2_TRBSR_EL1                           GENMASK(55, 55)
#define HDFGWTR_EL2_TRBSR_EL1_MASK                      GENMASK(55, 55)
#define HDFGWTR_EL2_TRBSR_EL1_SHIFT                     55
#define HDFGWTR_EL2_TRBSR_EL1_WIDTH                     1

#define HDFGWTR_EL2_TRBPTR_EL1                          GENMASK(54, 54)
#define HDFGWTR_EL2_TRBPTR_EL1_MASK                     GENMASK(54, 54)
#define HDFGWTR_EL2_TRBPTR_EL1_SHIFT                    54
#define HDFGWTR_EL2_TRBPTR_EL1_WIDTH                    1

#define HDFGWTR_EL2_TRBMAR_EL1                          GENMASK(53, 53)
#define HDFGWTR_EL2_TRBMAR_EL1_MASK                     GENMASK(53, 53)
#define HDFGWTR_EL2_TRBMAR_EL1_SHIFT                    53
#define HDFGWTR_EL2_TRBMAR_EL1_WIDTH                    1

#define HDFGWTR_EL2_TRBLIMITR_EL1                       GENMASK(52, 52)
#define HDFGWTR_EL2_TRBLIMITR_EL1_MASK                  GENMASK(52, 52)
#define HDFGWTR_EL2_TRBLIMITR_EL1_SHIFT                 52
#define HDFGWTR_EL2_TRBLIMITR_EL1_WIDTH                 1

#define HDFGWTR_EL2_TRBBASER_EL1                        GENMASK(50, 50)
#define HDFGWTR_EL2_TRBBASER_EL1_MASK                   GENMASK(50, 50)
#define HDFGWTR_EL2_TRBBASER_EL1_SHIFT                  50
#define HDFGWTR_EL2_TRBBASER_EL1_WIDTH                  1

#define HDFGWTR_EL2_TRFCR_EL1                           GENMASK(49, 49)
#define HDFGWTR_EL2_TRFCR_EL1_MASK                      GENMASK(49, 49)
#define HDFGWTR_EL2_TRFCR_EL1_SHIFT                     49
#define HDFGWTR_EL2_TRFCR_EL1_WIDTH                     1

#define HDFGWTR_EL2_TRCVICTLR                           GENMASK(48, 48)
#define HDFGWTR_EL2_TRCVICTLR_MASK                      GENMASK(48, 48)
#define HDFGWTR_EL2_TRCVICTLR_SHIFT                     48
#define HDFGWTR_EL2_TRCVICTLR_WIDTH                     1

#define HDFGWTR_EL2_TRCSSCSRn                           GENMASK(46, 46)
#define HDFGWTR_EL2_TRCSSCSRn_MASK                      GENMASK(46, 46)
#define HDFGWTR_EL2_TRCSSCSRn_SHIFT                     46
#define HDFGWTR_EL2_TRCSSCSRn_WIDTH                     1

#define HDFGWTR_EL2_TRCSEQSTR                           GENMASK(45, 45)
#define HDFGWTR_EL2_TRCSEQSTR_MASK                      GENMASK(45, 45)
#define HDFGWTR_EL2_TRCSEQSTR_SHIFT                     45
#define HDFGWTR_EL2_TRCSEQSTR_WIDTH                     1

#define HDFGWTR_EL2_TRCPRGCTLR                          GENMASK(44, 44)
#define HDFGWTR_EL2_TRCPRGCTLR_MASK                     GENMASK(44, 44)
#define HDFGWTR_EL2_TRCPRGCTLR_SHIFT                    44
#define HDFGWTR_EL2_TRCPRGCTLR_WIDTH                    1

#define HDFGWTR_EL2_TRCOSLAR                            GENMASK(42, 42)
#define HDFGWTR_EL2_TRCOSLAR_MASK                       GENMASK(42, 42)
#define HDFGWTR_EL2_TRCOSLAR_SHIFT                      42
#define HDFGWTR_EL2_TRCOSLAR_WIDTH                      1

#define HDFGWTR_EL2_TRCIMSPECn                          GENMASK(41, 41)
#define HDFGWTR_EL2_TRCIMSPECn_MASK                     GENMASK(41, 41)
#define HDFGWTR_EL2_TRCIMSPECn_SHIFT                    41
#define HDFGWTR_EL2_TRCIMSPECn_WIDTH                    1

#define HDFGWTR_EL2_TRCCNTVRn                           GENMASK(37, 37)
#define HDFGWTR_EL2_TRCCNTVRn_MASK                      GENMASK(37, 37)
#define HDFGWTR_EL2_TRCCNTVRn_SHIFT                     37
#define HDFGWTR_EL2_TRCCNTVRn_WIDTH                     1

#define HDFGWTR_EL2_TRCCLAIM                            GENMASK(36, 36)
#define HDFGWTR_EL2_TRCCLAIM_MASK                       GENMASK(36, 36)
#define HDFGWTR_EL2_TRCCLAIM_SHIFT                      36
#define HDFGWTR_EL2_TRCCLAIM_WIDTH                      1

#define HDFGWTR_EL2_TRCAUXCTLR                          GENMASK(35, 35)
#define HDFGWTR_EL2_TRCAUXCTLR_MASK                     GENMASK(35, 35)
#define HDFGWTR_EL2_TRCAUXCTLR_SHIFT                    35
#define HDFGWTR_EL2_TRCAUXCTLR_WIDTH                    1

#define HDFGWTR_EL2_TRC                                 GENMASK(33, 33)
#define HDFGWTR_EL2_TRC_MASK                            GENMASK(33, 33)
#define HDFGWTR_EL2_TRC_SHIFT                           33
#define HDFGWTR_EL2_TRC_WIDTH                           1

#define HDFGWTR_EL2_PMSLATFR_EL1                        GENMASK(32, 32)
#define HDFGWTR_EL2_PMSLATFR_EL1_MASK                   GENMASK(32, 32)
#define HDFGWTR_EL2_PMSLATFR_EL1_SHIFT                  32
#define HDFGWTR_EL2_PMSLATFR_EL1_WIDTH                  1

#define HDFGWTR_EL2_PMSIRR_EL1                          GENMASK(31, 31)
#define HDFGWTR_EL2_PMSIRR_EL1_MASK                     GENMASK(31, 31)
#define HDFGWTR_EL2_PMSIRR_EL1_SHIFT                    31
#define HDFGWTR_EL2_PMSIRR_EL1_WIDTH                    1

#define HDFGWTR_EL2_PMSICR_EL1                          GENMASK(29, 29)
#define HDFGWTR_EL2_PMSICR_EL1_MASK                     GENMASK(29, 29)
#define HDFGWTR_EL2_PMSICR_EL1_SHIFT                    29
#define HDFGWTR_EL2_PMSICR_EL1_WIDTH                    1

#define HDFGWTR_EL2_PMSFCR_EL1                          GENMASK(28, 28)
#define HDFGWTR_EL2_PMSFCR_EL1_MASK                     GENMASK(28, 28)
#define HDFGWTR_EL2_PMSFCR_EL1_SHIFT                    28
#define HDFGWTR_EL2_PMSFCR_EL1_WIDTH                    1

#define HDFGWTR_EL2_PMSEVFR_EL1                         GENMASK(27, 27)
#define HDFGWTR_EL2_PMSEVFR_EL1_MASK                    GENMASK(27, 27)
#define HDFGWTR_EL2_PMSEVFR_EL1_SHIFT                   27
#define HDFGWTR_EL2_PMSEVFR_EL1_WIDTH                   1

#define HDFGWTR_EL2_PMSCR_EL1                           GENMASK(26, 26)
#define HDFGWTR_EL2_PMSCR_EL1_MASK                      GENMASK(26, 26)
#define HDFGWTR_EL2_PMSCR_EL1_SHIFT                     26
#define HDFGWTR_EL2_PMSCR_EL1_WIDTH                     1

#define HDFGWTR_EL2_PMBSR_EL1                           GENMASK(25, 25)
#define HDFGWTR_EL2_PMBSR_EL1_MASK                      GENMASK(25, 25)
#define HDFGWTR_EL2_PMBSR_EL1_SHIFT                     25
#define HDFGWTR_EL2_PMBSR_EL1_WIDTH                     1

#define HDFGWTR_EL2_PMBPTR_EL1                          GENMASK(24, 24)
#define HDFGWTR_EL2_PMBPTR_EL1_MASK                     GENMASK(24, 24)
#define HDFGWTR_EL2_PMBPTR_EL1_SHIFT                    24
#define HDFGWTR_EL2_PMBPTR_EL1_WIDTH                    1

#define HDFGWTR_EL2_PMBLIMITR_EL1                       GENMASK(23, 23)
#define HDFGWTR_EL2_PMBLIMITR_EL1_MASK                  GENMASK(23, 23)
#define HDFGWTR_EL2_PMBLIMITR_EL1_SHIFT                 23
#define HDFGWTR_EL2_PMBLIMITR_EL1_WIDTH                 1

#define HDFGWTR_EL2_PMCR_EL0                            GENMASK(21, 21)
#define HDFGWTR_EL2_PMCR_EL0_MASK                       GENMASK(21, 21)
#define HDFGWTR_EL2_PMCR_EL0_SHIFT                      21
#define HDFGWTR_EL2_PMCR_EL0_WIDTH                      1

#define HDFGWTR_EL2_PMSWINC_EL0                         GENMASK(20, 20)
#define HDFGWTR_EL2_PMSWINC_EL0_MASK                    GENMASK(20, 20)
#define HDFGWTR_EL2_PMSWINC_EL0_SHIFT                   20
#define HDFGWTR_EL2_PMSWINC_EL0_WIDTH                   1

#define HDFGWTR_EL2_PMSELR_EL0                          GENMASK(19, 19)
#define HDFGWTR_EL2_PMSELR_EL0_MASK                     GENMASK(19, 19)
#define HDFGWTR_EL2_PMSELR_EL0_SHIFT                    19
#define HDFGWTR_EL2_PMSELR_EL0_WIDTH                    1

#define HDFGWTR_EL2_PMOVS                               GENMASK(18, 18)
#define HDFGWTR_EL2_PMOVS_MASK                          GENMASK(18, 18)
#define HDFGWTR_EL2_PMOVS_SHIFT                         18
#define HDFGWTR_EL2_PMOVS_WIDTH                         1

#define HDFGWTR_EL2_PMINTEN                             GENMASK(17, 17)
#define HDFGWTR_EL2_PMINTEN_MASK                        GENMASK(17, 17)
#define HDFGWTR_EL2_PMINTEN_SHIFT                       17
#define HDFGWTR_EL2_PMINTEN_WIDTH                       1

#define HDFGWTR_EL2_PMCNTEN                             GENMASK(16, 16)
#define HDFGWTR_EL2_PMCNTEN_MASK                        GENMASK(16, 16)
#define HDFGWTR_EL2_PMCNTEN_SHIFT                       16
#define HDFGWTR_EL2_PMCNTEN_WIDTH                       1

#define HDFGWTR_EL2_PMCCNTR_EL0                         GENMASK(15, 15)
#define HDFGWTR_EL2_PMCCNTR_EL0_MASK                    GENMASK(15, 15)
#define HDFGWTR_EL2_PMCCNTR_EL0_SHIFT                   15
#define HDFGWTR_EL2_PMCCNTR_EL0_WIDTH                   1

#define HDFGWTR_EL2_PMCCFILTR_EL0                       GENMASK(14, 14)
#define HDFGWTR_EL2_PMCCFILTR_EL0_MASK                  GENMASK(14, 14)
#define HDFGWTR_EL2_PMCCFILTR_EL0_SHIFT                 14
#define HDFGWTR_EL2_PMCCFILTR_EL0_WIDTH                 1

#define HDFGWTR_EL2_PMEVTYPERn_EL0                      GENMASK(13, 13)
#define HDFGWTR_EL2_PMEVTYPERn_EL0_MASK                 GENMASK(13, 13)
#define HDFGWTR_EL2_PMEVTYPERn_EL0_SHIFT                13
#define HDFGWTR_EL2_PMEVTYPERn_EL0_WIDTH                1

#define HDFGWTR_EL2_PMEVCNTRn_EL0                       GENMASK(12, 12)
#define HDFGWTR_EL2_PMEVCNTRn_EL0_MASK                  GENMASK(12, 12)
#define HDFGWTR_EL2_PMEVCNTRn_EL0_SHIFT                 12
#define HDFGWTR_EL2_PMEVCNTRn_EL0_WIDTH                 1

#define HDFGWTR_EL2_OSDLR_EL1                           GENMASK(11, 11)
#define HDFGWTR_EL2_OSDLR_EL1_MASK                      GENMASK(11, 11)
#define HDFGWTR_EL2_OSDLR_EL1_SHIFT                     11
#define HDFGWTR_EL2_OSDLR_EL1_WIDTH                     1

#define HDFGWTR_EL2_OSECCR_EL1                          GENMASK(10, 10)
#define HDFGWTR_EL2_OSECCR_EL1_MASK                     GENMASK(10, 10)
#define HDFGWTR_EL2_OSECCR_EL1_SHIFT                    10
#define HDFGWTR_EL2_OSECCR_EL1_WIDTH                    1

#define HDFGWTR_EL2_OSLAR_EL1                           GENMASK(8, 8)
#define HDFGWTR_EL2_OSLAR_EL1_MASK                      GENMASK(8, 8)
#define HDFGWTR_EL2_OSLAR_EL1_SHIFT                     8
#define HDFGWTR_EL2_OSLAR_EL1_WIDTH                     1

#define HDFGWTR_EL2_DBGPRCR_EL1                         GENMASK(7, 7)
#define HDFGWTR_EL2_DBGPRCR_EL1_MASK                    GENMASK(7, 7)
#define HDFGWTR_EL2_DBGPRCR_EL1_SHIFT                   7
#define HDFGWTR_EL2_DBGPRCR_EL1_WIDTH                   1

#define HDFGWTR_EL2_DBGCLAIM                            GENMASK(5, 5)
#define HDFGWTR_EL2_DBGCLAIM_MASK                       GENMASK(5, 5)
#define HDFGWTR_EL2_DBGCLAIM_SHIFT                      5
#define HDFGWTR_EL2_DBGCLAIM_WIDTH                      1

#define HDFGWTR_EL2_MDSCR_EL1                           GENMASK(4, 4)
#define HDFGWTR_EL2_MDSCR_EL1_MASK                      GENMASK(4, 4)
#define HDFGWTR_EL2_MDSCR_EL1_SHIFT                     4
#define HDFGWTR_EL2_MDSCR_EL1_WIDTH                     1

#define HDFGWTR_EL2_DBGWVRn_EL1                         GENMASK(3, 3)
#define HDFGWTR_EL2_DBGWVRn_EL1_MASK                    GENMASK(3, 3)
#define HDFGWTR_EL2_DBGWVRn_EL1_SHIFT                   3
#define HDFGWTR_EL2_DBGWVRn_EL1_WIDTH                   1

#define HDFGWTR_EL2_DBGWCRn_EL1                         GENMASK(2, 2)
#define HDFGWTR_EL2_DBGWCRn_EL1_MASK                    GENMASK(2, 2)
#define HDFGWTR_EL2_DBGWCRn_EL1_SHIFT                   2
#define HDFGWTR_EL2_DBGWCRn_EL1_WIDTH                   1

#define HDFGWTR_EL2_DBGBVRn_EL1                         GENMASK(1, 1)
#define HDFGWTR_EL2_DBGBVRn_EL1_MASK                    GENMASK(1, 1)
#define HDFGWTR_EL2_DBGBVRn_EL1_SHIFT                   1
#define HDFGWTR_EL2_DBGBVRn_EL1_WIDTH                   1

#define HDFGWTR_EL2_DBGBCRn_EL1                         GENMASK(0, 0)
#define HDFGWTR_EL2_DBGBCRn_EL1_MASK                    GENMASK(0, 0)
#define HDFGWTR_EL2_DBGBCRn_EL1_SHIFT                   0
#define HDFGWTR_EL2_DBGBCRn_EL1_WIDTH                   1

#define HDFGWTR_EL2_RES0                                (UL(0) | GENMASK_ULL(63, 63) | GENMASK_ULL(59, 58) | GENMASK_ULL(51, 51) | GENMASK_ULL(47, 47) | GENMASK_ULL(43, 43) | GENMASK_ULL(40, 38) | GENMASK_ULL(34, 34) | GENMASK_ULL(30, 30) | GENMASK_ULL(22, 22) | GENMASK_ULL(9, 9) | GENMASK_ULL(6, 6))
#define HDFGWTR_EL2_RES1                                (UL(0))
#define HDFGWTR_EL2_UNKN                                (UL(0))

#define REG_HAFGRTR_EL2                                 S3_4_C3_C1_6
#define SYS_HAFGRTR_EL2                                 sys_reg(3, 4, 3, 1, 6)
#define SYS_HAFGRTR_EL2_Op0                             3
#define SYS_HAFGRTR_EL2_Op1                             4
#define SYS_HAFGRTR_EL2_CRn                             3
#define SYS_HAFGRTR_EL2_CRm                             1
#define SYS_HAFGRTR_EL2_Op2                             6

#define HAFGRTR_EL2_AMEVTYPER115_EL0                    GENMASK(49, 49)
#define HAFGRTR_EL2_AMEVTYPER115_EL0_MASK               GENMASK(49, 49)
#define HAFGRTR_EL2_AMEVTYPER115_EL0_SHIFT              49
#define HAFGRTR_EL2_AMEVTYPER115_EL0_WIDTH              1

#define HAFGRTR_EL2_AMEVCNTR115_EL0                     GENMASK(48, 48)
#define HAFGRTR_EL2_AMEVCNTR115_EL0_MASK                GENMASK(48, 48)
#define HAFGRTR_EL2_AMEVCNTR115_EL0_SHIFT               48
#define HAFGRTR_EL2_AMEVCNTR115_EL0_WIDTH               1

#define HAFGRTR_EL2_AMEVTYPER114_EL0                    GENMASK(47, 47)
#define HAFGRTR_EL2_AMEVTYPER114_EL0_MASK               GENMASK(47, 47)
#define HAFGRTR_EL2_AMEVTYPER114_EL0_SHIFT              47
#define HAFGRTR_EL2_AMEVTYPER114_EL0_WIDTH              1

#define HAFGRTR_EL2_AMEVCNTR114_EL0                     GENMASK(46, 46)
#define HAFGRTR_EL2_AMEVCNTR114_EL0_MASK                GENMASK(46, 46)
#define HAFGRTR_EL2_AMEVCNTR114_EL0_SHIFT               46
#define HAFGRTR_EL2_AMEVCNTR114_EL0_WIDTH               1

#define HAFGRTR_EL2_AMEVTYPER113_EL0                    GENMASK(45, 45)
#define HAFGRTR_EL2_AMEVTYPER113_EL0_MASK               GENMASK(45, 45)
#define HAFGRTR_EL2_AMEVTYPER113_EL0_SHIFT              45
#define HAFGRTR_EL2_AMEVTYPER113_EL0_WIDTH              1

#define HAFGRTR_EL2_AMEVCNTR113_EL0                     GENMASK(44, 44)
#define HAFGRTR_EL2_AMEVCNTR113_EL0_MASK                GENMASK(44, 44)
#define HAFGRTR_EL2_AMEVCNTR113_EL0_SHIFT               44
#define HAFGRTR_EL2_AMEVCNTR113_EL0_WIDTH               1

#define HAFGRTR_EL2_AMEVTYPER112_EL0                    GENMASK(43, 43)
#define HAFGRTR_EL2_AMEVTYPER112_EL0_MASK               GENMASK(43, 43)
#define HAFGRTR_EL2_AMEVTYPER112_EL0_SHIFT              43
#define HAFGRTR_EL2_AMEVTYPER112_EL0_WIDTH              1

#define HAFGRTR_EL2_AMEVCNTR112_EL0                     GENMASK(42, 42)
#define HAFGRTR_EL2_AMEVCNTR112_EL0_MASK                GENMASK(42, 42)
#define HAFGRTR_EL2_AMEVCNTR112_EL0_SHIFT               42
#define HAFGRTR_EL2_AMEVCNTR112_EL0_WIDTH               1

#define HAFGRTR_EL2_AMEVTYPER111_EL0                    GENMASK(41, 41)
#define HAFGRTR_EL2_AMEVTYPER111_EL0_MASK               GENMASK(41, 41)
#define HAFGRTR_EL2_AMEVTYPER111_EL0_SHIFT              41
#define HAFGRTR_EL2_AMEVTYPER111_EL0_WIDTH              1

#define HAFGRTR_EL2_AMEVCNTR111_EL0                     GENMASK(40, 40)
#define HAFGRTR_EL2_AMEVCNTR111_EL0_MASK                GENMASK(40, 40)
#define HAFGRTR_EL2_AMEVCNTR111_EL0_SHIFT               40
#define HAFGRTR_EL2_AMEVCNTR111_EL0_WIDTH               1

#define HAFGRTR_EL2_AMEVTYPER110_EL0                    GENMASK(39, 39)
#define HAFGRTR_EL2_AMEVTYPER110_EL0_MASK               GENMASK(39, 39)
#define HAFGRTR_EL2_AMEVTYPER110_EL0_SHIFT              39
#define HAFGRTR_EL2_AMEVTYPER110_EL0_WIDTH              1

#define HAFGRTR_EL2_AMEVCNTR110_EL0                     GENMASK(38, 38)
#define HAFGRTR_EL2_AMEVCNTR110_EL0_MASK                GENMASK(38, 38)
#define HAFGRTR_EL2_AMEVCNTR110_EL0_SHIFT               38
#define HAFGRTR_EL2_AMEVCNTR110_EL0_WIDTH               1

#define HAFGRTR_EL2_AMEVTYPER19_EL0                     GENMASK(37, 37)
#define HAFGRTR_EL2_AMEVTYPER19_EL0_MASK                GENMASK(37, 37)
#define HAFGRTR_EL2_AMEVTYPER19_EL0_SHIFT               37
#define HAFGRTR_EL2_AMEVTYPER19_EL0_WIDTH               1

#define HAFGRTR_EL2_AMEVCNTR19_EL0                      GENMASK(36, 36)
#define HAFGRTR_EL2_AMEVCNTR19_EL0_MASK                 GENMASK(36, 36)
#define HAFGRTR_EL2_AMEVCNTR19_EL0_SHIFT                36
#define HAFGRTR_EL2_AMEVCNTR19_EL0_WIDTH                1

#define HAFGRTR_EL2_AMEVTYPER18_EL0                     GENMASK(35, 35)
#define HAFGRTR_EL2_AMEVTYPER18_EL0_MASK                GENMASK(35, 35)
#define HAFGRTR_EL2_AMEVTYPER18_EL0_SHIFT               35
#define HAFGRTR_EL2_AMEVTYPER18_EL0_WIDTH               1

#define HAFGRTR_EL2_AMEVCNTR18_EL0                      GENMASK(34, 34)
#define HAFGRTR_EL2_AMEVCNTR18_EL0_MASK                 GENMASK(34, 34)
#define HAFGRTR_EL2_AMEVCNTR18_EL0_SHIFT                34
#define HAFGRTR_EL2_AMEVCNTR18_EL0_WIDTH                1

#define HAFGRTR_EL2_AMEVTYPER17_EL0                     GENMASK(33, 33)
#define HAFGRTR_EL2_AMEVTYPER17_EL0_MASK                GENMASK(33, 33)
#define HAFGRTR_EL2_AMEVTYPER17_EL0_SHIFT               33
#define HAFGRTR_EL2_AMEVTYPER17_EL0_WIDTH               1

#define HAFGRTR_EL2_AMEVCNTR17_EL0                      GENMASK(32, 32)
#define HAFGRTR_EL2_AMEVCNTR17_EL0_MASK                 GENMASK(32, 32)
#define HAFGRTR_EL2_AMEVCNTR17_EL0_SHIFT                32
#define HAFGRTR_EL2_AMEVCNTR17_EL0_WIDTH                1

#define HAFGRTR_EL2_AMEVTYPER16_EL0                     GENMASK(31, 31)
#define HAFGRTR_EL2_AMEVTYPER16_EL0_MASK                GENMASK(31, 31)
#define HAFGRTR_EL2_AMEVTYPER16_EL0_SHIFT               31
#define HAFGRTR_EL2_AMEVTYPER16_EL0_WIDTH               1

#define HAFGRTR_EL2_AMEVCNTR16_EL0                      GENMASK(30, 30)
#define HAFGRTR_EL2_AMEVCNTR16_EL0_MASK                 GENMASK(30, 30)
#define HAFGRTR_EL2_AMEVCNTR16_EL0_SHIFT                30
#define HAFGRTR_EL2_AMEVCNTR16_EL0_WIDTH                1

#define HAFGRTR_EL2_AMEVTYPER15_EL0                     GENMASK(29, 29)
#define HAFGRTR_EL2_AMEVTYPER15_EL0_MASK                GENMASK(29, 29)
#define HAFGRTR_EL2_AMEVTYPER15_EL0_SHIFT               29
#define HAFGRTR_EL2_AMEVTYPER15_EL0_WIDTH               1

#define HAFGRTR_EL2_AMEVCNTR15_EL0                      GENMASK(28, 28)
#define HAFGRTR_EL2_AMEVCNTR15_EL0_MASK                 GENMASK(28, 28)
#define HAFGRTR_EL2_AMEVCNTR15_EL0_SHIFT                28
#define HAFGRTR_EL2_AMEVCNTR15_EL0_WIDTH                1

#define HAFGRTR_EL2_AMEVTYPER14_EL0                     GENMASK(27, 27)
#define HAFGRTR_EL2_AMEVTYPER14_EL0_MASK                GENMASK(27, 27)
#define HAFGRTR_EL2_AMEVTYPER14_EL0_SHIFT               27
#define HAFGRTR_EL2_AMEVTYPER14_EL0_WIDTH               1

#define HAFGRTR_EL2_AMEVCNTR14_EL0                      GENMASK(26, 26)
#define HAFGRTR_EL2_AMEVCNTR14_EL0_MASK                 GENMASK(26, 26)
#define HAFGRTR_EL2_AMEVCNTR14_EL0_SHIFT                26
#define HAFGRTR_EL2_AMEVCNTR14_EL0_WIDTH                1

#define HAFGRTR_EL2_AMEVTYPER13_EL0                     GENMASK(25, 25)
#define HAFGRTR_EL2_AMEVTYPER13_EL0_MASK                GENMASK(25, 25)
#define HAFGRTR_EL2_AMEVTYPER13_EL0_SHIFT               25
#define HAFGRTR_EL2_AMEVTYPER13_EL0_WIDTH               1

#define HAFGRTR_EL2_AMEVCNTR13_EL0                      GENMASK(24, 24)
#define HAFGRTR_EL2_AMEVCNTR13_EL0_MASK                 GENMASK(24, 24)
#define HAFGRTR_EL2_AMEVCNTR13_EL0_SHIFT                24
#define HAFGRTR_EL2_AMEVCNTR13_EL0_WIDTH                1

#define HAFGRTR_EL2_AMEVTYPER12_EL0                     GENMASK(23, 23)
#define HAFGRTR_EL2_AMEVTYPER12_EL0_MASK                GENMASK(23, 23)
#define HAFGRTR_EL2_AMEVTYPER12_EL0_SHIFT               23
#define HAFGRTR_EL2_AMEVTYPER12_EL0_WIDTH               1

#define HAFGRTR_EL2_AMEVCNTR12_EL0                      GENMASK(22, 22)
#define HAFGRTR_EL2_AMEVCNTR12_EL0_MASK                 GENMASK(22, 22)
#define HAFGRTR_EL2_AMEVCNTR12_EL0_SHIFT                22
#define HAFGRTR_EL2_AMEVCNTR12_EL0_WIDTH                1

#define HAFGRTR_EL2_AMEVTYPER11_EL0                     GENMASK(21, 21)
#define HAFGRTR_EL2_AMEVTYPER11_EL0_MASK                GENMASK(21, 21)
#define HAFGRTR_EL2_AMEVTYPER11_EL0_SHIFT               21
#define HAFGRTR_EL2_AMEVTYPER11_EL0_WIDTH               1

#define HAFGRTR_EL2_AMEVCNTR11_EL0                      GENMASK(20, 20)
#define HAFGRTR_EL2_AMEVCNTR11_EL0_MASK                 GENMASK(20, 20)
#define HAFGRTR_EL2_AMEVCNTR11_EL0_SHIFT                20
#define HAFGRTR_EL2_AMEVCNTR11_EL0_WIDTH                1

#define HAFGRTR_EL2_AMEVTYPER10_EL0                     GENMASK(19, 19)
#define HAFGRTR_EL2_AMEVTYPER10_EL0_MASK                GENMASK(19, 19)
#define HAFGRTR_EL2_AMEVTYPER10_EL0_SHIFT               19
#define HAFGRTR_EL2_AMEVTYPER10_EL0_WIDTH               1

#define HAFGRTR_EL2_AMEVCNTR10_EL0                      GENMASK(18, 18)
#define HAFGRTR_EL2_AMEVCNTR10_EL0_MASK                 GENMASK(18, 18)
#define HAFGRTR_EL2_AMEVCNTR10_EL0_SHIFT                18
#define HAFGRTR_EL2_AMEVCNTR10_EL0_WIDTH                1

#define HAFGRTR_EL2_AMCNTEN1                            GENMASK(17, 17)
#define HAFGRTR_EL2_AMCNTEN1_MASK                       GENMASK(17, 17)
#define HAFGRTR_EL2_AMCNTEN1_SHIFT                      17
#define HAFGRTR_EL2_AMCNTEN1_WIDTH                      1

#define HAFGRTR_EL2_AMEVCNTR03_EL0                      GENMASK(4, 4)
#define HAFGRTR_EL2_AMEVCNTR03_EL0_MASK                 GENMASK(4, 4)
#define HAFGRTR_EL2_AMEVCNTR03_EL0_SHIFT                4
#define HAFGRTR_EL2_AMEVCNTR03_EL0_WIDTH                1

#define HAFGRTR_EL2_AMEVCNTR02_EL0                      GENMASK(3, 3)
#define HAFGRTR_EL2_AMEVCNTR02_EL0_MASK                 GENMASK(3, 3)
#define HAFGRTR_EL2_AMEVCNTR02_EL0_SHIFT                3
#define HAFGRTR_EL2_AMEVCNTR02_EL0_WIDTH                1

#define HAFGRTR_EL2_AMEVCNTR01_EL0                      GENMASK(2, 2)
#define HAFGRTR_EL2_AMEVCNTR01_EL0_MASK                 GENMASK(2, 2)
#define HAFGRTR_EL2_AMEVCNTR01_EL0_SHIFT                2
#define HAFGRTR_EL2_AMEVCNTR01_EL0_WIDTH                1

#define HAFGRTR_EL2_AMEVCNTR00_EL0                      GENMASK(1, 1)
#define HAFGRTR_EL2_AMEVCNTR00_EL0_MASK                 GENMASK(1, 1)
#define HAFGRTR_EL2_AMEVCNTR00_EL0_SHIFT                1
#define HAFGRTR_EL2_AMEVCNTR00_EL0_WIDTH                1

#define HAFGRTR_EL2_AMCNTEN0                            GENMASK(0, 0)
#define HAFGRTR_EL2_AMCNTEN0_MASK                       GENMASK(0, 0)
#define HAFGRTR_EL2_AMCNTEN0_SHIFT                      0
#define HAFGRTR_EL2_AMCNTEN0_WIDTH                      1

#define HAFGRTR_EL2_RES0                                (UL(0) | GENMASK_ULL(63, 50) | GENMASK_ULL(16, 5))
#define HAFGRTR_EL2_RES1                                (UL(0))
#define HAFGRTR_EL2_UNKN                                (UL(0))

#define REG_HFGITR2_EL2                                 S3_4_C3_C1_7
#define SYS_HFGITR2_EL2                                 sys_reg(3, 4, 3, 1, 7)
#define SYS_HFGITR2_EL2_Op0                             3
#define SYS_HFGITR2_EL2_Op1                             4
#define SYS_HFGITR2_EL2_CRn                             3
#define SYS_HFGITR2_EL2_CRm                             1
#define SYS_HFGITR2_EL2_Op2                             7

#define HFGITR2_EL2_nDCCIVAPS                           GENMASK(1, 1)
#define HFGITR2_EL2_nDCCIVAPS_MASK                      GENMASK(1, 1)
#define HFGITR2_EL2_nDCCIVAPS_SHIFT                     1
#define HFGITR2_EL2_nDCCIVAPS_WIDTH                     1

#define HFGITR2_EL2_TSBCSYNC                            GENMASK(0, 0)
#define HFGITR2_EL2_TSBCSYNC_MASK                       GENMASK(0, 0)
#define HFGITR2_EL2_TSBCSYNC_SHIFT                      0
#define HFGITR2_EL2_TSBCSYNC_WIDTH                      1

#define HFGITR2_EL2_RES0                                (UL(0) | GENMASK_ULL(63, 2))
#define HFGITR2_EL2_RES1                                (UL(0))
#define HFGITR2_EL2_UNKN                                (UL(0))

#define REG_ZCR_EL2                                     S3_4_C1_C2_0
#define SYS_ZCR_EL2                                     sys_reg(3, 4, 1, 2, 0)
#define SYS_ZCR_EL2_Op0                                 3
#define SYS_ZCR_EL2_Op1                                 4
#define SYS_ZCR_EL2_CRn                                 1
#define SYS_ZCR_EL2_CRm                                 2
#define SYS_ZCR_EL2_Op2                                 0

/* For ZCR_EL2 fields see ZCR_ELx */

#define REG_HCRX_EL2                                    S3_4_C1_C2_2
#define SYS_HCRX_EL2                                    sys_reg(3, 4, 1, 2, 2)
#define SYS_HCRX_EL2_Op0                                3
#define SYS_HCRX_EL2_Op1                                4
#define SYS_HCRX_EL2_CRn                                1
#define SYS_HCRX_EL2_CRm                                2
#define SYS_HCRX_EL2_Op2                                2

#define HCRX_EL2_PACMEn                                 GENMASK(24, 24)
#define HCRX_EL2_PACMEn_MASK                            GENMASK(24, 24)
#define HCRX_EL2_PACMEn_SHIFT                           24
#define HCRX_EL2_PACMEn_WIDTH                           1

#define HCRX_EL2_EnFPM                                  GENMASK(23, 23)
#define HCRX_EL2_EnFPM_MASK                             GENMASK(23, 23)
#define HCRX_EL2_EnFPM_SHIFT                            23
#define HCRX_EL2_EnFPM_WIDTH                            1

#define HCRX_EL2_GCSEn                                  GENMASK(22, 22)
#define HCRX_EL2_GCSEn_MASK                             GENMASK(22, 22)
#define HCRX_EL2_GCSEn_SHIFT                            22
#define HCRX_EL2_GCSEn_WIDTH                            1

#define HCRX_EL2_EnIDCP128                              GENMASK(21, 21)
#define HCRX_EL2_EnIDCP128_MASK                         GENMASK(21, 21)
#define HCRX_EL2_EnIDCP128_SHIFT                        21
#define HCRX_EL2_EnIDCP128_WIDTH                        1

#define HCRX_EL2_EnSDERR                                GENMASK(20, 20)
#define HCRX_EL2_EnSDERR_MASK                           GENMASK(20, 20)
#define HCRX_EL2_EnSDERR_SHIFT                          20
#define HCRX_EL2_EnSDERR_WIDTH                          1

#define HCRX_EL2_TMEA                                   GENMASK(19, 19)
#define HCRX_EL2_TMEA_MASK                              GENMASK(19, 19)
#define HCRX_EL2_TMEA_SHIFT                             19
#define HCRX_EL2_TMEA_WIDTH                             1

#define HCRX_EL2_EnSNERR                                GENMASK(18, 18)
#define HCRX_EL2_EnSNERR_MASK                           GENMASK(18, 18)
#define HCRX_EL2_EnSNERR_SHIFT                          18
#define HCRX_EL2_EnSNERR_WIDTH                          1

#define HCRX_EL2_D128En                                 GENMASK(17, 17)
#define HCRX_EL2_D128En_MASK                            GENMASK(17, 17)
#define HCRX_EL2_D128En_SHIFT                           17
#define HCRX_EL2_D128En_WIDTH                           1

#define HCRX_EL2_PTTWI                                  GENMASK(16, 16)
#define HCRX_EL2_PTTWI_MASK                             GENMASK(16, 16)
#define HCRX_EL2_PTTWI_SHIFT                            16
#define HCRX_EL2_PTTWI_WIDTH                            1

#define HCRX_EL2_SCTLR2En                               GENMASK(15, 15)
#define HCRX_EL2_SCTLR2En_MASK                          GENMASK(15, 15)
#define HCRX_EL2_SCTLR2En_SHIFT                         15
#define HCRX_EL2_SCTLR2En_WIDTH                         1

#define HCRX_EL2_TCR2En                                 GENMASK(14, 14)
#define HCRX_EL2_TCR2En_MASK                            GENMASK(14, 14)
#define HCRX_EL2_TCR2En_SHIFT                           14
#define HCRX_EL2_TCR2En_WIDTH                           1

#define HCRX_EL2_MSCEn                                  GENMASK(11, 11)
#define HCRX_EL2_MSCEn_MASK                             GENMASK(11, 11)
#define HCRX_EL2_MSCEn_SHIFT                            11
#define HCRX_EL2_MSCEn_WIDTH                            1

#define HCRX_EL2_MCE2                                   GENMASK(10, 10)
#define HCRX_EL2_MCE2_MASK                              GENMASK(10, 10)
#define HCRX_EL2_MCE2_SHIFT                             10
#define HCRX_EL2_MCE2_WIDTH                             1

#define HCRX_EL2_CMOW                                   GENMASK(9, 9)
#define HCRX_EL2_CMOW_MASK                              GENMASK(9, 9)
#define HCRX_EL2_CMOW_SHIFT                             9
#define HCRX_EL2_CMOW_WIDTH                             1

#define HCRX_EL2_VFNMI                                  GENMASK(8, 8)
#define HCRX_EL2_VFNMI_MASK                             GENMASK(8, 8)
#define HCRX_EL2_VFNMI_SHIFT                            8
#define HCRX_EL2_VFNMI_WIDTH                            1

#define HCRX_EL2_VINMI                                  GENMASK(7, 7)
#define HCRX_EL2_VINMI_MASK                             GENMASK(7, 7)
#define HCRX_EL2_VINMI_SHIFT                            7
#define HCRX_EL2_VINMI_WIDTH                            1

#define HCRX_EL2_TALLINT                                GENMASK(6, 6)
#define HCRX_EL2_TALLINT_MASK                           GENMASK(6, 6)
#define HCRX_EL2_TALLINT_SHIFT                          6
#define HCRX_EL2_TALLINT_WIDTH                          1

#define HCRX_EL2_SMPME                                  GENMASK(5, 5)
#define HCRX_EL2_SMPME_MASK                             GENMASK(5, 5)
#define HCRX_EL2_SMPME_SHIFT                            5
#define HCRX_EL2_SMPME_WIDTH                            1

#define HCRX_EL2_FGTnXS                                 GENMASK(4, 4)
#define HCRX_EL2_FGTnXS_MASK                            GENMASK(4, 4)
#define HCRX_EL2_FGTnXS_SHIFT                           4
#define HCRX_EL2_FGTnXS_WIDTH                           1

#define HCRX_EL2_FnXS                                   GENMASK(3, 3)
#define HCRX_EL2_FnXS_MASK                              GENMASK(3, 3)
#define HCRX_EL2_FnXS_SHIFT                             3
#define HCRX_EL2_FnXS_WIDTH                             1

#define HCRX_EL2_EnASR                                  GENMASK(2, 2)
#define HCRX_EL2_EnASR_MASK                             GENMASK(2, 2)
#define HCRX_EL2_EnASR_SHIFT                            2
#define HCRX_EL2_EnASR_WIDTH                            1

#define HCRX_EL2_EnALS                                  GENMASK(1, 1)
#define HCRX_EL2_EnALS_MASK                             GENMASK(1, 1)
#define HCRX_EL2_EnALS_SHIFT                            1
#define HCRX_EL2_EnALS_WIDTH                            1

#define HCRX_EL2_EnAS0                                  GENMASK(0, 0)
#define HCRX_EL2_EnAS0_MASK                             GENMASK(0, 0)
#define HCRX_EL2_EnAS0_SHIFT                            0
#define HCRX_EL2_EnAS0_WIDTH                            1

#define HCRX_EL2_RES0                                   (UL(0) | GENMASK_ULL(63, 25) | GENMASK_ULL(13, 12))
#define HCRX_EL2_RES1                                   (UL(0))
#define HCRX_EL2_UNKN                                   (UL(0))

#define REG_SMPRIMAP_EL2                                S3_4_C1_C2_5
#define SYS_SMPRIMAP_EL2                                sys_reg(3, 4, 1, 2, 5)
#define SYS_SMPRIMAP_EL2_Op0                            3
#define SYS_SMPRIMAP_EL2_Op1                            4
#define SYS_SMPRIMAP_EL2_CRn                            1
#define SYS_SMPRIMAP_EL2_CRm                            2
#define SYS_SMPRIMAP_EL2_Op2                            5

#define SMPRIMAP_EL2_P15                                GENMASK(63, 60)
#define SMPRIMAP_EL2_P15_MASK                           GENMASK(63, 60)
#define SMPRIMAP_EL2_P15_SHIFT                          60
#define SMPRIMAP_EL2_P15_WIDTH                          4

#define SMPRIMAP_EL2_P14                                GENMASK(59, 56)
#define SMPRIMAP_EL2_P14_MASK                           GENMASK(59, 56)
#define SMPRIMAP_EL2_P14_SHIFT                          56
#define SMPRIMAP_EL2_P14_WIDTH                          4

#define SMPRIMAP_EL2_P13                                GENMASK(55, 52)
#define SMPRIMAP_EL2_P13_MASK                           GENMASK(55, 52)
#define SMPRIMAP_EL2_P13_SHIFT                          52
#define SMPRIMAP_EL2_P13_WIDTH                          4

#define SMPRIMAP_EL2_P12                                GENMASK(51, 48)
#define SMPRIMAP_EL2_P12_MASK                           GENMASK(51, 48)
#define SMPRIMAP_EL2_P12_SHIFT                          48
#define SMPRIMAP_EL2_P12_WIDTH                          4

#define SMPRIMAP_EL2_P11                                GENMASK(47, 44)
#define SMPRIMAP_EL2_P11_MASK                           GENMASK(47, 44)
#define SMPRIMAP_EL2_P11_SHIFT                          44
#define SMPRIMAP_EL2_P11_WIDTH                          4

#define SMPRIMAP_EL2_P10                                GENMASK(43, 40)
#define SMPRIMAP_EL2_P10_MASK                           GENMASK(43, 40)
#define SMPRIMAP_EL2_P10_SHIFT                          40
#define SMPRIMAP_EL2_P10_WIDTH                          4

#define SMPRIMAP_EL2_F9                                 GENMASK(39, 36)
#define SMPRIMAP_EL2_F9_MASK                            GENMASK(39, 36)
#define SMPRIMAP_EL2_F9_SHIFT                           36
#define SMPRIMAP_EL2_F9_WIDTH                           4

#define SMPRIMAP_EL2_P8                                 GENMASK(35, 32)
#define SMPRIMAP_EL2_P8_MASK                            GENMASK(35, 32)
#define SMPRIMAP_EL2_P8_SHIFT                           32
#define SMPRIMAP_EL2_P8_WIDTH                           4

#define SMPRIMAP_EL2_P7                                 GENMASK(31, 28)
#define SMPRIMAP_EL2_P7_MASK                            GENMASK(31, 28)
#define SMPRIMAP_EL2_P7_SHIFT                           28
#define SMPRIMAP_EL2_P7_WIDTH                           4

#define SMPRIMAP_EL2_P6                                 GENMASK(27, 24)
#define SMPRIMAP_EL2_P6_MASK                            GENMASK(27, 24)
#define SMPRIMAP_EL2_P6_SHIFT                           24
#define SMPRIMAP_EL2_P6_WIDTH                           4

#define SMPRIMAP_EL2_P5                                 GENMASK(23, 20)
#define SMPRIMAP_EL2_P5_MASK                            GENMASK(23, 20)
#define SMPRIMAP_EL2_P5_SHIFT                           20
#define SMPRIMAP_EL2_P5_WIDTH                           4

#define SMPRIMAP_EL2_P4                                 GENMASK(19, 16)
#define SMPRIMAP_EL2_P4_MASK                            GENMASK(19, 16)
#define SMPRIMAP_EL2_P4_SHIFT                           16
#define SMPRIMAP_EL2_P4_WIDTH                           4

#define SMPRIMAP_EL2_P3                                 GENMASK(15, 12)
#define SMPRIMAP_EL2_P3_MASK                            GENMASK(15, 12)
#define SMPRIMAP_EL2_P3_SHIFT                           12
#define SMPRIMAP_EL2_P3_WIDTH                           4

#define SMPRIMAP_EL2_P2                                 GENMASK(11, 8)
#define SMPRIMAP_EL2_P2_MASK                            GENMASK(11, 8)
#define SMPRIMAP_EL2_P2_SHIFT                           8
#define SMPRIMAP_EL2_P2_WIDTH                           4

#define SMPRIMAP_EL2_P1                                 GENMASK(7, 4)
#define SMPRIMAP_EL2_P1_MASK                            GENMASK(7, 4)
#define SMPRIMAP_EL2_P1_SHIFT                           4
#define SMPRIMAP_EL2_P1_WIDTH                           4

#define SMPRIMAP_EL2_P0                                 GENMASK(3, 0)
#define SMPRIMAP_EL2_P0_MASK                            GENMASK(3, 0)
#define SMPRIMAP_EL2_P0_SHIFT                           0
#define SMPRIMAP_EL2_P0_WIDTH                           4

#define SMPRIMAP_EL2_RES0                               (UL(0))
#define SMPRIMAP_EL2_RES1                               (UL(0))
#define SMPRIMAP_EL2_UNKN                               (UL(0))

#define REG_SMCR_EL2                                    S3_4_C1_C2_6
#define SYS_SMCR_EL2                                    sys_reg(3, 4, 1, 2, 6)
#define SYS_SMCR_EL2_Op0                                3
#define SYS_SMCR_EL2_Op1                                4
#define SYS_SMCR_EL2_CRn                                1
#define SYS_SMCR_EL2_CRm                                2
#define SYS_SMCR_EL2_Op2                                6

/* For SMCR_EL2 fields see SMCR_ELx */

#define REG_VNCR_EL2                                    S3_4_C2_C2_0
#define SYS_VNCR_EL2                                    sys_reg(3, 4, 2, 2, 0)
#define SYS_VNCR_EL2_Op0                                3
#define SYS_VNCR_EL2_Op1                                4
#define SYS_VNCR_EL2_CRn                                2
#define SYS_VNCR_EL2_CRm                                2
#define SYS_VNCR_EL2_Op2                                0

#define VNCR_EL2_RESS                                   GENMASK(63, 57)
#define VNCR_EL2_RESS_MASK                              GENMASK(63, 57)
#define VNCR_EL2_RESS_SHIFT                             57
#define VNCR_EL2_RESS_WIDTH                             7

#define VNCR_EL2_BADDR                                  GENMASK(56, 12)
#define VNCR_EL2_BADDR_MASK                             GENMASK(56, 12)
#define VNCR_EL2_BADDR_SHIFT                            12
#define VNCR_EL2_BADDR_WIDTH                            45

#define VNCR_EL2_RES0                                   (UL(0) | GENMASK_ULL(11, 0))
#define VNCR_EL2_RES1                                   (UL(0))
#define VNCR_EL2_UNKN                                   (UL(0))

#define REG_GCSCR_EL2                                   S3_4_C2_C5_0
#define SYS_GCSCR_EL2                                   sys_reg(3, 4, 2, 5, 0)
#define SYS_GCSCR_EL2_Op0                               3
#define SYS_GCSCR_EL2_Op1                               4
#define SYS_GCSCR_EL2_CRn                               2
#define SYS_GCSCR_EL2_CRm                               5
#define SYS_GCSCR_EL2_Op2                               0

/* For GCSCR_EL2 fields see GCSCR_ELx */

#define REG_GCSPR_EL2                                   S3_4_C2_C5_1
#define SYS_GCSPR_EL2                                   sys_reg(3, 4, 2, 5, 1)
#define SYS_GCSPR_EL2_Op0                               3
#define SYS_GCSPR_EL2_Op1                               4
#define SYS_GCSPR_EL2_CRn                               2
#define SYS_GCSPR_EL2_CRm                               5
#define SYS_GCSPR_EL2_Op2                               1

/* For GCSPR_EL2 fields see GCSPR_ELx */

#define REG_DACR32_EL2                                  S3_4_C3_C0_0
#define SYS_DACR32_EL2                                  sys_reg(3, 4, 3, 0, 0)
#define SYS_DACR32_EL2_Op0                              3
#define SYS_DACR32_EL2_Op1                              4
#define SYS_DACR32_EL2_CRn                              3
#define SYS_DACR32_EL2_CRm                              0
#define SYS_DACR32_EL2_Op2                              0

#define DACR32_EL2_D15                                  GENMASK(31, 30)
#define DACR32_EL2_D15_MASK                             GENMASK(31, 30)
#define DACR32_EL2_D15_SHIFT                            30
#define DACR32_EL2_D15_WIDTH                            2

#define DACR32_EL2_D14                                  GENMASK(29, 28)
#define DACR32_EL2_D14_MASK                             GENMASK(29, 28)
#define DACR32_EL2_D14_SHIFT                            28
#define DACR32_EL2_D14_WIDTH                            2

#define DACR32_EL2_D13                                  GENMASK(27, 26)
#define DACR32_EL2_D13_MASK                             GENMASK(27, 26)
#define DACR32_EL2_D13_SHIFT                            26
#define DACR32_EL2_D13_WIDTH                            2

#define DACR32_EL2_D12                                  GENMASK(25, 24)
#define DACR32_EL2_D12_MASK                             GENMASK(25, 24)
#define DACR32_EL2_D12_SHIFT                            24
#define DACR32_EL2_D12_WIDTH                            2

#define DACR32_EL2_D11                                  GENMASK(23, 22)
#define DACR32_EL2_D11_MASK                             GENMASK(23, 22)
#define DACR32_EL2_D11_SHIFT                            22
#define DACR32_EL2_D11_WIDTH                            2

#define DACR32_EL2_D10                                  GENMASK(21, 20)
#define DACR32_EL2_D10_MASK                             GENMASK(21, 20)
#define DACR32_EL2_D10_SHIFT                            20
#define DACR32_EL2_D10_WIDTH                            2

#define DACR32_EL2_D9                                   GENMASK(19, 18)
#define DACR32_EL2_D9_MASK                              GENMASK(19, 18)
#define DACR32_EL2_D9_SHIFT                             18
#define DACR32_EL2_D9_WIDTH                             2

#define DACR32_EL2_D8                                   GENMASK(17, 16)
#define DACR32_EL2_D8_MASK                              GENMASK(17, 16)
#define DACR32_EL2_D8_SHIFT                             16
#define DACR32_EL2_D8_WIDTH                             2

#define DACR32_EL2_D7                                   GENMASK(15, 14)
#define DACR32_EL2_D7_MASK                              GENMASK(15, 14)
#define DACR32_EL2_D7_SHIFT                             14
#define DACR32_EL2_D7_WIDTH                             2

#define DACR32_EL2_D6                                   GENMASK(13, 12)
#define DACR32_EL2_D6_MASK                              GENMASK(13, 12)
#define DACR32_EL2_D6_SHIFT                             12
#define DACR32_EL2_D6_WIDTH                             2

#define DACR32_EL2_D5                                   GENMASK(11, 10)
#define DACR32_EL2_D5_MASK                              GENMASK(11, 10)
#define DACR32_EL2_D5_SHIFT                             10
#define DACR32_EL2_D5_WIDTH                             2

#define DACR32_EL2_D4                                   GENMASK(9, 8)
#define DACR32_EL2_D4_MASK                              GENMASK(9, 8)
#define DACR32_EL2_D4_SHIFT                             8
#define DACR32_EL2_D4_WIDTH                             2

#define DACR32_EL2_D3                                   GENMASK(7, 6)
#define DACR32_EL2_D3_MASK                              GENMASK(7, 6)
#define DACR32_EL2_D3_SHIFT                             6
#define DACR32_EL2_D3_WIDTH                             2

#define DACR32_EL2_D2                                   GENMASK(5, 4)
#define DACR32_EL2_D2_MASK                              GENMASK(5, 4)
#define DACR32_EL2_D2_SHIFT                             4
#define DACR32_EL2_D2_WIDTH                             2

#define DACR32_EL2_D1                                   GENMASK(3, 2)
#define DACR32_EL2_D1_MASK                              GENMASK(3, 2)
#define DACR32_EL2_D1_SHIFT                             2
#define DACR32_EL2_D1_WIDTH                             2

#define DACR32_EL2_D0                                   GENMASK(1, 0)
#define DACR32_EL2_D0_MASK                              GENMASK(1, 0)
#define DACR32_EL2_D0_SHIFT                             0
#define DACR32_EL2_D0_WIDTH                             2

#define DACR32_EL2_RES0                                 (UL(0) | GENMASK_ULL(63, 32))
#define DACR32_EL2_RES1                                 (UL(0))
#define DACR32_EL2_UNKN                                 (UL(0))

#define REG_FAR_EL2                                     S3_4_C6_C0_0
#define SYS_FAR_EL2                                     sys_reg(3, 4, 6, 0, 0)
#define SYS_FAR_EL2_Op0                                 3
#define SYS_FAR_EL2_Op1                                 4
#define SYS_FAR_EL2_CRn                                 6
#define SYS_FAR_EL2_CRm                                 0
#define SYS_FAR_EL2_Op2                                 0

#define FAR_EL2_ADDR                                    GENMASK(63, 0)
#define FAR_EL2_ADDR_MASK                               GENMASK(63, 0)
#define FAR_EL2_ADDR_SHIFT                              0
#define FAR_EL2_ADDR_WIDTH                              64

#define FAR_EL2_RES0                                    (UL(0))
#define FAR_EL2_RES1                                    (UL(0))
#define FAR_EL2_UNKN                                    (UL(0))

#define REG_PMSCR_EL2                                   S3_4_C9_C9_0
#define SYS_PMSCR_EL2                                   sys_reg(3, 4, 9, 9, 0)
#define SYS_PMSCR_EL2_Op0                               3
#define SYS_PMSCR_EL2_Op1                               4
#define SYS_PMSCR_EL2_CRn                               9
#define SYS_PMSCR_EL2_CRm                               9
#define SYS_PMSCR_EL2_Op2                               0

#define PMSCR_EL2_PCT                                   GENMASK(7, 6)
#define PMSCR_EL2_PCT_MASK                              GENMASK(7, 6)
#define PMSCR_EL2_PCT_SHIFT                             6
#define PMSCR_EL2_PCT_WIDTH                             2
#define PMSCR_EL2_PCT_VIRT                              UL(0b00)
#define PMSCR_EL2_PCT_PHYS                              UL(0b01)
#define PMSCR_EL2_PCT_GUEST                             UL(0b11)

#define PMSCR_EL2_TS                                    GENMASK(5, 5)
#define PMSCR_EL2_TS_MASK                               GENMASK(5, 5)
#define PMSCR_EL2_TS_SHIFT                              5
#define PMSCR_EL2_TS_WIDTH                              1

#define PMSCR_EL2_PA                                    GENMASK(4, 4)
#define PMSCR_EL2_PA_MASK                               GENMASK(4, 4)
#define PMSCR_EL2_PA_SHIFT                              4
#define PMSCR_EL2_PA_WIDTH                              1

#define PMSCR_EL2_CX                                    GENMASK(3, 3)
#define PMSCR_EL2_CX_MASK                               GENMASK(3, 3)
#define PMSCR_EL2_CX_SHIFT                              3
#define PMSCR_EL2_CX_WIDTH                              1

#define PMSCR_EL2_E2SPE                                 GENMASK(1, 1)
#define PMSCR_EL2_E2SPE_MASK                            GENMASK(1, 1)
#define PMSCR_EL2_E2SPE_SHIFT                           1
#define PMSCR_EL2_E2SPE_WIDTH                           1

#define PMSCR_EL2_E0HSPE                                GENMASK(0, 0)
#define PMSCR_EL2_E0HSPE_MASK                           GENMASK(0, 0)
#define PMSCR_EL2_E0HSPE_SHIFT                          0
#define PMSCR_EL2_E0HSPE_WIDTH                          1

#define PMSCR_EL2_RES0                                  (UL(0) | GENMASK_ULL(63, 8) | GENMASK_ULL(2, 2))
#define PMSCR_EL2_RES1                                  (UL(0))
#define PMSCR_EL2_UNKN                                  (UL(0))

#define REG_MPAMHCR_EL2                                 S3_4_C10_C4_0
#define SYS_MPAMHCR_EL2                                 sys_reg(3, 4, 10, 4, 0)
#define SYS_MPAMHCR_EL2_Op0                             3
#define SYS_MPAMHCR_EL2_Op1                             4
#define SYS_MPAMHCR_EL2_CRn                             10
#define SYS_MPAMHCR_EL2_CRm                             4
#define SYS_MPAMHCR_EL2_Op2                             0

#define MPAMHCR_EL2_TRAP_MPAMIDR_EL1                    GENMASK(31, 31)
#define MPAMHCR_EL2_TRAP_MPAMIDR_EL1_MASK               GENMASK(31, 31)
#define MPAMHCR_EL2_TRAP_MPAMIDR_EL1_SHIFT              31
#define MPAMHCR_EL2_TRAP_MPAMIDR_EL1_WIDTH              1

#define MPAMHCR_EL2_GSTAPP_PLK                          GENMASK(8, 8)
#define MPAMHCR_EL2_GSTAPP_PLK_MASK                     GENMASK(8, 8)
#define MPAMHCR_EL2_GSTAPP_PLK_SHIFT                    8
#define MPAMHCR_EL2_GSTAPP_PLK_WIDTH                    1

#define MPAMHCR_EL2_EL1_VPMEN                           GENMASK(1, 1)
#define MPAMHCR_EL2_EL1_VPMEN_MASK                      GENMASK(1, 1)
#define MPAMHCR_EL2_EL1_VPMEN_SHIFT                     1
#define MPAMHCR_EL2_EL1_VPMEN_WIDTH                     1

#define MPAMHCR_EL2_EL0_VPMEN                           GENMASK(0, 0)
#define MPAMHCR_EL2_EL0_VPMEN_MASK                      GENMASK(0, 0)
#define MPAMHCR_EL2_EL0_VPMEN_SHIFT                     0
#define MPAMHCR_EL2_EL0_VPMEN_WIDTH                     1

#define MPAMHCR_EL2_RES0                                (UL(0) | GENMASK_ULL(63, 32) | GENMASK_ULL(30, 9) | GENMASK_ULL(7, 2))
#define MPAMHCR_EL2_RES1                                (UL(0))
#define MPAMHCR_EL2_UNKN                                (UL(0))

#define REG_MPAMVPMV_EL2                                S3_4_C10_C4_1
#define SYS_MPAMVPMV_EL2                                sys_reg(3, 4, 10, 4, 1)
#define SYS_MPAMVPMV_EL2_Op0                            3
#define SYS_MPAMVPMV_EL2_Op1                            4
#define SYS_MPAMVPMV_EL2_CRn                            10
#define SYS_MPAMVPMV_EL2_CRm                            4
#define SYS_MPAMVPMV_EL2_Op2                            1

#define MPAMVPMV_EL2_VPM_V31                            GENMASK(31, 31)
#define MPAMVPMV_EL2_VPM_V31_MASK                       GENMASK(31, 31)
#define MPAMVPMV_EL2_VPM_V31_SHIFT                      31
#define MPAMVPMV_EL2_VPM_V31_WIDTH                      1

#define MPAMVPMV_EL2_VPM_V30                            GENMASK(30, 30)
#define MPAMVPMV_EL2_VPM_V30_MASK                       GENMASK(30, 30)
#define MPAMVPMV_EL2_VPM_V30_SHIFT                      30
#define MPAMVPMV_EL2_VPM_V30_WIDTH                      1

#define MPAMVPMV_EL2_VPM_V29                            GENMASK(29, 29)
#define MPAMVPMV_EL2_VPM_V29_MASK                       GENMASK(29, 29)
#define MPAMVPMV_EL2_VPM_V29_SHIFT                      29
#define MPAMVPMV_EL2_VPM_V29_WIDTH                      1

#define MPAMVPMV_EL2_VPM_V28                            GENMASK(28, 28)
#define MPAMVPMV_EL2_VPM_V28_MASK                       GENMASK(28, 28)
#define MPAMVPMV_EL2_VPM_V28_SHIFT                      28
#define MPAMVPMV_EL2_VPM_V28_WIDTH                      1

#define MPAMVPMV_EL2_VPM_V27                            GENMASK(27, 27)
#define MPAMVPMV_EL2_VPM_V27_MASK                       GENMASK(27, 27)
#define MPAMVPMV_EL2_VPM_V27_SHIFT                      27
#define MPAMVPMV_EL2_VPM_V27_WIDTH                      1

#define MPAMVPMV_EL2_VPM_V26                            GENMASK(26, 26)
#define MPAMVPMV_EL2_VPM_V26_MASK                       GENMASK(26, 26)
#define MPAMVPMV_EL2_VPM_V26_SHIFT                      26
#define MPAMVPMV_EL2_VPM_V26_WIDTH                      1

#define MPAMVPMV_EL2_VPM_V25                            GENMASK(25, 25)
#define MPAMVPMV_EL2_VPM_V25_MASK                       GENMASK(25, 25)
#define MPAMVPMV_EL2_VPM_V25_SHIFT                      25
#define MPAMVPMV_EL2_VPM_V25_WIDTH                      1

#define MPAMVPMV_EL2_VPM_V24                            GENMASK(24, 24)
#define MPAMVPMV_EL2_VPM_V24_MASK                       GENMASK(24, 24)
#define MPAMVPMV_EL2_VPM_V24_SHIFT                      24
#define MPAMVPMV_EL2_VPM_V24_WIDTH                      1

#define MPAMVPMV_EL2_VPM_V23                            GENMASK(23, 23)
#define MPAMVPMV_EL2_VPM_V23_MASK                       GENMASK(23, 23)
#define MPAMVPMV_EL2_VPM_V23_SHIFT                      23
#define MPAMVPMV_EL2_VPM_V23_WIDTH                      1

#define MPAMVPMV_EL2_VPM_V22                            GENMASK(22, 22)
#define MPAMVPMV_EL2_VPM_V22_MASK                       GENMASK(22, 22)
#define MPAMVPMV_EL2_VPM_V22_SHIFT                      22
#define MPAMVPMV_EL2_VPM_V22_WIDTH                      1

#define MPAMVPMV_EL2_VPM_V21                            GENMASK(21, 21)
#define MPAMVPMV_EL2_VPM_V21_MASK                       GENMASK(21, 21)
#define MPAMVPMV_EL2_VPM_V21_SHIFT                      21
#define MPAMVPMV_EL2_VPM_V21_WIDTH                      1

#define MPAMVPMV_EL2_VPM_V20                            GENMASK(20, 20)
#define MPAMVPMV_EL2_VPM_V20_MASK                       GENMASK(20, 20)
#define MPAMVPMV_EL2_VPM_V20_SHIFT                      20
#define MPAMVPMV_EL2_VPM_V20_WIDTH                      1

#define MPAMVPMV_EL2_VPM_V19                            GENMASK(19, 19)
#define MPAMVPMV_EL2_VPM_V19_MASK                       GENMASK(19, 19)
#define MPAMVPMV_EL2_VPM_V19_SHIFT                      19
#define MPAMVPMV_EL2_VPM_V19_WIDTH                      1

#define MPAMVPMV_EL2_VPM_V18                            GENMASK(18, 18)
#define MPAMVPMV_EL2_VPM_V18_MASK                       GENMASK(18, 18)
#define MPAMVPMV_EL2_VPM_V18_SHIFT                      18
#define MPAMVPMV_EL2_VPM_V18_WIDTH                      1

#define MPAMVPMV_EL2_VPM_V17                            GENMASK(17, 17)
#define MPAMVPMV_EL2_VPM_V17_MASK                       GENMASK(17, 17)
#define MPAMVPMV_EL2_VPM_V17_SHIFT                      17
#define MPAMVPMV_EL2_VPM_V17_WIDTH                      1

#define MPAMVPMV_EL2_VPM_V16                            GENMASK(16, 16)
#define MPAMVPMV_EL2_VPM_V16_MASK                       GENMASK(16, 16)
#define MPAMVPMV_EL2_VPM_V16_SHIFT                      16
#define MPAMVPMV_EL2_VPM_V16_WIDTH                      1

#define MPAMVPMV_EL2_VPM_V15                            GENMASK(15, 15)
#define MPAMVPMV_EL2_VPM_V15_MASK                       GENMASK(15, 15)
#define MPAMVPMV_EL2_VPM_V15_SHIFT                      15
#define MPAMVPMV_EL2_VPM_V15_WIDTH                      1

#define MPAMVPMV_EL2_VPM_V14                            GENMASK(14, 14)
#define MPAMVPMV_EL2_VPM_V14_MASK                       GENMASK(14, 14)
#define MPAMVPMV_EL2_VPM_V14_SHIFT                      14
#define MPAMVPMV_EL2_VPM_V14_WIDTH                      1

#define MPAMVPMV_EL2_VPM_V13                            GENMASK(13, 13)
#define MPAMVPMV_EL2_VPM_V13_MASK                       GENMASK(13, 13)
#define MPAMVPMV_EL2_VPM_V13_SHIFT                      13
#define MPAMVPMV_EL2_VPM_V13_WIDTH                      1

#define MPAMVPMV_EL2_VPM_V12                            GENMASK(12, 12)
#define MPAMVPMV_EL2_VPM_V12_MASK                       GENMASK(12, 12)
#define MPAMVPMV_EL2_VPM_V12_SHIFT                      12
#define MPAMVPMV_EL2_VPM_V12_WIDTH                      1

#define MPAMVPMV_EL2_VPM_V11                            GENMASK(11, 11)
#define MPAMVPMV_EL2_VPM_V11_MASK                       GENMASK(11, 11)
#define MPAMVPMV_EL2_VPM_V11_SHIFT                      11
#define MPAMVPMV_EL2_VPM_V11_WIDTH                      1

#define MPAMVPMV_EL2_VPM_V10                            GENMASK(10, 10)
#define MPAMVPMV_EL2_VPM_V10_MASK                       GENMASK(10, 10)
#define MPAMVPMV_EL2_VPM_V10_SHIFT                      10
#define MPAMVPMV_EL2_VPM_V10_WIDTH                      1

#define MPAMVPMV_EL2_VPM_V9                             GENMASK(9, 9)
#define MPAMVPMV_EL2_VPM_V9_MASK                        GENMASK(9, 9)
#define MPAMVPMV_EL2_VPM_V9_SHIFT                       9
#define MPAMVPMV_EL2_VPM_V9_WIDTH                       1

#define MPAMVPMV_EL2_VPM_V8                             GENMASK(8, 8)
#define MPAMVPMV_EL2_VPM_V8_MASK                        GENMASK(8, 8)
#define MPAMVPMV_EL2_VPM_V8_SHIFT                       8
#define MPAMVPMV_EL2_VPM_V8_WIDTH                       1

#define MPAMVPMV_EL2_VPM_V7                             GENMASK(7, 7)
#define MPAMVPMV_EL2_VPM_V7_MASK                        GENMASK(7, 7)
#define MPAMVPMV_EL2_VPM_V7_SHIFT                       7
#define MPAMVPMV_EL2_VPM_V7_WIDTH                       1

#define MPAMVPMV_EL2_VPM_V6                             GENMASK(6, 6)
#define MPAMVPMV_EL2_VPM_V6_MASK                        GENMASK(6, 6)
#define MPAMVPMV_EL2_VPM_V6_SHIFT                       6
#define MPAMVPMV_EL2_VPM_V6_WIDTH                       1

#define MPAMVPMV_EL2_VPM_V5                             GENMASK(5, 5)
#define MPAMVPMV_EL2_VPM_V5_MASK                        GENMASK(5, 5)
#define MPAMVPMV_EL2_VPM_V5_SHIFT                       5
#define MPAMVPMV_EL2_VPM_V5_WIDTH                       1

#define MPAMVPMV_EL2_VPM_V4                             GENMASK(4, 4)
#define MPAMVPMV_EL2_VPM_V4_MASK                        GENMASK(4, 4)
#define MPAMVPMV_EL2_VPM_V4_SHIFT                       4
#define MPAMVPMV_EL2_VPM_V4_WIDTH                       1

#define MPAMVPMV_EL2_VPM_V3                             GENMASK(3, 3)
#define MPAMVPMV_EL2_VPM_V3_MASK                        GENMASK(3, 3)
#define MPAMVPMV_EL2_VPM_V3_SHIFT                       3
#define MPAMVPMV_EL2_VPM_V3_WIDTH                       1

#define MPAMVPMV_EL2_VPM_V2                             GENMASK(2, 2)
#define MPAMVPMV_EL2_VPM_V2_MASK                        GENMASK(2, 2)
#define MPAMVPMV_EL2_VPM_V2_SHIFT                       2
#define MPAMVPMV_EL2_VPM_V2_WIDTH                       1

#define MPAMVPMV_EL2_VPM_V1                             GENMASK(1, 1)
#define MPAMVPMV_EL2_VPM_V1_MASK                        GENMASK(1, 1)
#define MPAMVPMV_EL2_VPM_V1_SHIFT                       1
#define MPAMVPMV_EL2_VPM_V1_WIDTH                       1

#define MPAMVPMV_EL2_VPM_V0                             GENMASK(0, 0)
#define MPAMVPMV_EL2_VPM_V0_MASK                        GENMASK(0, 0)
#define MPAMVPMV_EL2_VPM_V0_SHIFT                       0
#define MPAMVPMV_EL2_VPM_V0_WIDTH                       1

#define MPAMVPMV_EL2_RES0                               (UL(0) | GENMASK_ULL(63, 32))
#define MPAMVPMV_EL2_RES1                               (UL(0))
#define MPAMVPMV_EL2_UNKN                               (UL(0))

#define REG_MPAM2_EL2                                   S3_4_C10_C5_0
#define SYS_MPAM2_EL2                                   sys_reg(3, 4, 10, 5, 0)
#define SYS_MPAM2_EL2_Op0                               3
#define SYS_MPAM2_EL2_Op1                               4
#define SYS_MPAM2_EL2_CRn                               10
#define SYS_MPAM2_EL2_CRm                               5
#define SYS_MPAM2_EL2_Op2                               0

#define MPAM2_EL2_MPAMEN                                GENMASK(63, 63)
#define MPAM2_EL2_MPAMEN_MASK                           GENMASK(63, 63)
#define MPAM2_EL2_MPAMEN_SHIFT                          63
#define MPAM2_EL2_MPAMEN_WIDTH                          1

#define MPAM2_EL2_TIDR                                  GENMASK(58, 58)
#define MPAM2_EL2_TIDR_MASK                             GENMASK(58, 58)
#define MPAM2_EL2_TIDR_SHIFT                            58
#define MPAM2_EL2_TIDR_WIDTH                            1

#define MPAM2_EL2_ALTSP_HFC                             GENMASK(56, 56)
#define MPAM2_EL2_ALTSP_HFC_MASK                        GENMASK(56, 56)
#define MPAM2_EL2_ALTSP_HFC_SHIFT                       56
#define MPAM2_EL2_ALTSP_HFC_WIDTH                       1

#define MPAM2_EL2_ALTSP_EL2                             GENMASK(55, 55)
#define MPAM2_EL2_ALTSP_EL2_MASK                        GENMASK(55, 55)
#define MPAM2_EL2_ALTSP_EL2_SHIFT                       55
#define MPAM2_EL2_ALTSP_EL2_WIDTH                       1

#define MPAM2_EL2_ALTSP_FRCD                            GENMASK(54, 54)
#define MPAM2_EL2_ALTSP_FRCD_MASK                       GENMASK(54, 54)
#define MPAM2_EL2_ALTSP_FRCD_SHIFT                      54
#define MPAM2_EL2_ALTSP_FRCD_WIDTH                      1

#define MPAM2_EL2_EnMPAMSM                              GENMASK(50, 50)
#define MPAM2_EL2_EnMPAMSM_MASK                         GENMASK(50, 50)
#define MPAM2_EL2_EnMPAMSM_SHIFT                        50
#define MPAM2_EL2_EnMPAMSM_WIDTH                        1

#define MPAM2_EL2_TRAPMPAM0EL1                          GENMASK(49, 49)
#define MPAM2_EL2_TRAPMPAM0EL1_MASK                     GENMASK(49, 49)
#define MPAM2_EL2_TRAPMPAM0EL1_SHIFT                    49
#define MPAM2_EL2_TRAPMPAM0EL1_WIDTH                    1

#define MPAM2_EL2_TRAPMPAM1EL1                          GENMASK(48, 48)
#define MPAM2_EL2_TRAPMPAM1EL1_MASK                     GENMASK(48, 48)
#define MPAM2_EL2_TRAPMPAM1EL1_SHIFT                    48
#define MPAM2_EL2_TRAPMPAM1EL1_WIDTH                    1

#define MPAM2_EL2_PMG_D                                 GENMASK(47, 40)
#define MPAM2_EL2_PMG_D_MASK                            GENMASK(47, 40)
#define MPAM2_EL2_PMG_D_SHIFT                           40
#define MPAM2_EL2_PMG_D_WIDTH                           8

#define MPAM2_EL2_PMG_I                                 GENMASK(39, 32)
#define MPAM2_EL2_PMG_I_MASK                            GENMASK(39, 32)
#define MPAM2_EL2_PMG_I_SHIFT                           32
#define MPAM2_EL2_PMG_I_WIDTH                           8

#define MPAM2_EL2_PARTID_D                              GENMASK(31, 16)
#define MPAM2_EL2_PARTID_D_MASK                         GENMASK(31, 16)
#define MPAM2_EL2_PARTID_D_SHIFT                        16
#define MPAM2_EL2_PARTID_D_WIDTH                        16

#define MPAM2_EL2_PARTID_I                              GENMASK(15, 0)
#define MPAM2_EL2_PARTID_I_MASK                         GENMASK(15, 0)
#define MPAM2_EL2_PARTID_I_SHIFT                        0
#define MPAM2_EL2_PARTID_I_WIDTH                        16

#define MPAM2_EL2_RES0                                  (UL(0) | GENMASK_ULL(62, 59) | GENMASK_ULL(57, 57) | GENMASK_ULL(53, 51))
#define MPAM2_EL2_RES1                                  (UL(0))
#define MPAM2_EL2_UNKN                                  (UL(0))

#define REG_MPAMVPM0_EL2                                S3_4_C10_C6_0
#define SYS_MPAMVPM0_EL2                                sys_reg(3, 4, 10, 6, 0)
#define SYS_MPAMVPM0_EL2_Op0                            3
#define SYS_MPAMVPM0_EL2_Op1                            4
#define SYS_MPAMVPM0_EL2_CRn                            10
#define SYS_MPAMVPM0_EL2_CRm                            6
#define SYS_MPAMVPM0_EL2_Op2                            0

#define MPAMVPM0_EL2_PhyPARTID3                         GENMASK(63, 48)
#define MPAMVPM0_EL2_PhyPARTID3_MASK                    GENMASK(63, 48)
#define MPAMVPM0_EL2_PhyPARTID3_SHIFT                   48
#define MPAMVPM0_EL2_PhyPARTID3_WIDTH                   16

#define MPAMVPM0_EL2_PhyPARTID2                         GENMASK(47, 32)
#define MPAMVPM0_EL2_PhyPARTID2_MASK                    GENMASK(47, 32)
#define MPAMVPM0_EL2_PhyPARTID2_SHIFT                   32
#define MPAMVPM0_EL2_PhyPARTID2_WIDTH                   16

#define MPAMVPM0_EL2_PhyPARTID1                         GENMASK(31, 16)
#define MPAMVPM0_EL2_PhyPARTID1_MASK                    GENMASK(31, 16)
#define MPAMVPM0_EL2_PhyPARTID1_SHIFT                   16
#define MPAMVPM0_EL2_PhyPARTID1_WIDTH                   16

#define MPAMVPM0_EL2_PhyPARTID0                         GENMASK(15, 0)
#define MPAMVPM0_EL2_PhyPARTID0_MASK                    GENMASK(15, 0)
#define MPAMVPM0_EL2_PhyPARTID0_SHIFT                   0
#define MPAMVPM0_EL2_PhyPARTID0_WIDTH                   16

#define MPAMVPM0_EL2_RES0                               (UL(0))
#define MPAMVPM0_EL2_RES1                               (UL(0))
#define MPAMVPM0_EL2_UNKN                               (UL(0))

#define REG_MPAMVPM1_EL2                                S3_4_C10_C6_1
#define SYS_MPAMVPM1_EL2                                sys_reg(3, 4, 10, 6, 1)
#define SYS_MPAMVPM1_EL2_Op0                            3
#define SYS_MPAMVPM1_EL2_Op1                            4
#define SYS_MPAMVPM1_EL2_CRn                            10
#define SYS_MPAMVPM1_EL2_CRm                            6
#define SYS_MPAMVPM1_EL2_Op2                            1

#define MPAMVPM1_EL2_PhyPARTID7                         GENMASK(63, 48)
#define MPAMVPM1_EL2_PhyPARTID7_MASK                    GENMASK(63, 48)
#define MPAMVPM1_EL2_PhyPARTID7_SHIFT                   48
#define MPAMVPM1_EL2_PhyPARTID7_WIDTH                   16

#define MPAMVPM1_EL2_PhyPARTID6                         GENMASK(47, 32)
#define MPAMVPM1_EL2_PhyPARTID6_MASK                    GENMASK(47, 32)
#define MPAMVPM1_EL2_PhyPARTID6_SHIFT                   32
#define MPAMVPM1_EL2_PhyPARTID6_WIDTH                   16

#define MPAMVPM1_EL2_PhyPARTID5                         GENMASK(31, 16)
#define MPAMVPM1_EL2_PhyPARTID5_MASK                    GENMASK(31, 16)
#define MPAMVPM1_EL2_PhyPARTID5_SHIFT                   16
#define MPAMVPM1_EL2_PhyPARTID5_WIDTH                   16

#define MPAMVPM1_EL2_PhyPARTID4                         GENMASK(15, 0)
#define MPAMVPM1_EL2_PhyPARTID4_MASK                    GENMASK(15, 0)
#define MPAMVPM1_EL2_PhyPARTID4_SHIFT                   0
#define MPAMVPM1_EL2_PhyPARTID4_WIDTH                   16

#define MPAMVPM1_EL2_RES0                               (UL(0))
#define MPAMVPM1_EL2_RES1                               (UL(0))
#define MPAMVPM1_EL2_UNKN                               (UL(0))

#define REG_MPAMVPM2_EL2                                S3_4_C10_C6_2
#define SYS_MPAMVPM2_EL2                                sys_reg(3, 4, 10, 6, 2)
#define SYS_MPAMVPM2_EL2_Op0                            3
#define SYS_MPAMVPM2_EL2_Op1                            4
#define SYS_MPAMVPM2_EL2_CRn                            10
#define SYS_MPAMVPM2_EL2_CRm                            6
#define SYS_MPAMVPM2_EL2_Op2                            2

#define MPAMVPM2_EL2_PhyPARTID11                        GENMASK(63, 48)
#define MPAMVPM2_EL2_PhyPARTID11_MASK                   GENMASK(63, 48)
#define MPAMVPM2_EL2_PhyPARTID11_SHIFT                  48
#define MPAMVPM2_EL2_PhyPARTID11_WIDTH                  16

#define MPAMVPM2_EL2_PhyPARTID10                        GENMASK(47, 32)
#define MPAMVPM2_EL2_PhyPARTID10_MASK                   GENMASK(47, 32)
#define MPAMVPM2_EL2_PhyPARTID10_SHIFT                  32
#define MPAMVPM2_EL2_PhyPARTID10_WIDTH                  16

#define MPAMVPM2_EL2_PhyPARTID9                         GENMASK(31, 16)
#define MPAMVPM2_EL2_PhyPARTID9_MASK                    GENMASK(31, 16)
#define MPAMVPM2_EL2_PhyPARTID9_SHIFT                   16
#define MPAMVPM2_EL2_PhyPARTID9_WIDTH                   16

#define MPAMVPM2_EL2_PhyPARTID8                         GENMASK(15, 0)
#define MPAMVPM2_EL2_PhyPARTID8_MASK                    GENMASK(15, 0)
#define MPAMVPM2_EL2_PhyPARTID8_SHIFT                   0
#define MPAMVPM2_EL2_PhyPARTID8_WIDTH                   16

#define MPAMVPM2_EL2_RES0                               (UL(0))
#define MPAMVPM2_EL2_RES1                               (UL(0))
#define MPAMVPM2_EL2_UNKN                               (UL(0))

#define REG_MPAMVPM3_EL2                                S3_4_C10_C6_3
#define SYS_MPAMVPM3_EL2                                sys_reg(3, 4, 10, 6, 3)
#define SYS_MPAMVPM3_EL2_Op0                            3
#define SYS_MPAMVPM3_EL2_Op1                            4
#define SYS_MPAMVPM3_EL2_CRn                            10
#define SYS_MPAMVPM3_EL2_CRm                            6
#define SYS_MPAMVPM3_EL2_Op2                            3

#define MPAMVPM3_EL2_PhyPARTID15                        GENMASK(63, 48)
#define MPAMVPM3_EL2_PhyPARTID15_MASK                   GENMASK(63, 48)
#define MPAMVPM3_EL2_PhyPARTID15_SHIFT                  48
#define MPAMVPM3_EL2_PhyPARTID15_WIDTH                  16

#define MPAMVPM3_EL2_PhyPARTID14                        GENMASK(47, 32)
#define MPAMVPM3_EL2_PhyPARTID14_MASK                   GENMASK(47, 32)
#define MPAMVPM3_EL2_PhyPARTID14_SHIFT                  32
#define MPAMVPM3_EL2_PhyPARTID14_WIDTH                  16

#define MPAMVPM3_EL2_PhyPARTID13                        GENMASK(31, 16)
#define MPAMVPM3_EL2_PhyPARTID13_MASK                   GENMASK(31, 16)
#define MPAMVPM3_EL2_PhyPARTID13_SHIFT                  16
#define MPAMVPM3_EL2_PhyPARTID13_WIDTH                  16

#define MPAMVPM3_EL2_PhyPARTID12                        GENMASK(15, 0)
#define MPAMVPM3_EL2_PhyPARTID12_MASK                   GENMASK(15, 0)
#define MPAMVPM3_EL2_PhyPARTID12_SHIFT                  0
#define MPAMVPM3_EL2_PhyPARTID12_WIDTH                  16

#define MPAMVPM3_EL2_RES0                               (UL(0))
#define MPAMVPM3_EL2_RES1                               (UL(0))
#define MPAMVPM3_EL2_UNKN                               (UL(0))

#define REG_MPAMVPM4_EL2                                S3_4_C10_C6_4
#define SYS_MPAMVPM4_EL2                                sys_reg(3, 4, 10, 6, 4)
#define SYS_MPAMVPM4_EL2_Op0                            3
#define SYS_MPAMVPM4_EL2_Op1                            4
#define SYS_MPAMVPM4_EL2_CRn                            10
#define SYS_MPAMVPM4_EL2_CRm                            6
#define SYS_MPAMVPM4_EL2_Op2                            4

#define MPAMVPM4_EL2_PhyPARTID19                        GENMASK(63, 48)
#define MPAMVPM4_EL2_PhyPARTID19_MASK                   GENMASK(63, 48)
#define MPAMVPM4_EL2_PhyPARTID19_SHIFT                  48
#define MPAMVPM4_EL2_PhyPARTID19_WIDTH                  16

#define MPAMVPM4_EL2_PhyPARTID18                        GENMASK(47, 32)
#define MPAMVPM4_EL2_PhyPARTID18_MASK                   GENMASK(47, 32)
#define MPAMVPM4_EL2_PhyPARTID18_SHIFT                  32
#define MPAMVPM4_EL2_PhyPARTID18_WIDTH                  16

#define MPAMVPM4_EL2_PhyPARTID17                        GENMASK(31, 16)
#define MPAMVPM4_EL2_PhyPARTID17_MASK                   GENMASK(31, 16)
#define MPAMVPM4_EL2_PhyPARTID17_SHIFT                  16
#define MPAMVPM4_EL2_PhyPARTID17_WIDTH                  16

#define MPAMVPM4_EL2_PhyPARTID16                        GENMASK(15, 0)
#define MPAMVPM4_EL2_PhyPARTID16_MASK                   GENMASK(15, 0)
#define MPAMVPM4_EL2_PhyPARTID16_SHIFT                  0
#define MPAMVPM4_EL2_PhyPARTID16_WIDTH                  16

#define MPAMVPM4_EL2_RES0                               (UL(0))
#define MPAMVPM4_EL2_RES1                               (UL(0))
#define MPAMVPM4_EL2_UNKN                               (UL(0))

#define REG_MPAMVPM5_EL2                                S3_4_C10_C6_5
#define SYS_MPAMVPM5_EL2                                sys_reg(3, 4, 10, 6, 5)
#define SYS_MPAMVPM5_EL2_Op0                            3
#define SYS_MPAMVPM5_EL2_Op1                            4
#define SYS_MPAMVPM5_EL2_CRn                            10
#define SYS_MPAMVPM5_EL2_CRm                            6
#define SYS_MPAMVPM5_EL2_Op2                            5

#define MPAMVPM5_EL2_PhyPARTID23                        GENMASK(63, 48)
#define MPAMVPM5_EL2_PhyPARTID23_MASK                   GENMASK(63, 48)
#define MPAMVPM5_EL2_PhyPARTID23_SHIFT                  48
#define MPAMVPM5_EL2_PhyPARTID23_WIDTH                  16

#define MPAMVPM5_EL2_PhyPARTID22                        GENMASK(47, 32)
#define MPAMVPM5_EL2_PhyPARTID22_MASK                   GENMASK(47, 32)
#define MPAMVPM5_EL2_PhyPARTID22_SHIFT                  32
#define MPAMVPM5_EL2_PhyPARTID22_WIDTH                  16

#define MPAMVPM5_EL2_PhyPARTID21                        GENMASK(31, 16)
#define MPAMVPM5_EL2_PhyPARTID21_MASK                   GENMASK(31, 16)
#define MPAMVPM5_EL2_PhyPARTID21_SHIFT                  16
#define MPAMVPM5_EL2_PhyPARTID21_WIDTH                  16

#define MPAMVPM5_EL2_PhyPARTID20                        GENMASK(15, 0)
#define MPAMVPM5_EL2_PhyPARTID20_MASK                   GENMASK(15, 0)
#define MPAMVPM5_EL2_PhyPARTID20_SHIFT                  0
#define MPAMVPM5_EL2_PhyPARTID20_WIDTH                  16

#define MPAMVPM5_EL2_RES0                               (UL(0))
#define MPAMVPM5_EL2_RES1                               (UL(0))
#define MPAMVPM5_EL2_UNKN                               (UL(0))

#define REG_MPAMVPM6_EL2                                S3_4_C10_C6_6
#define SYS_MPAMVPM6_EL2                                sys_reg(3, 4, 10, 6, 6)
#define SYS_MPAMVPM6_EL2_Op0                            3
#define SYS_MPAMVPM6_EL2_Op1                            4
#define SYS_MPAMVPM6_EL2_CRn                            10
#define SYS_MPAMVPM6_EL2_CRm                            6
#define SYS_MPAMVPM6_EL2_Op2                            6

#define MPAMVPM6_EL2_PhyPARTID27                        GENMASK(63, 48)
#define MPAMVPM6_EL2_PhyPARTID27_MASK                   GENMASK(63, 48)
#define MPAMVPM6_EL2_PhyPARTID27_SHIFT                  48
#define MPAMVPM6_EL2_PhyPARTID27_WIDTH                  16

#define MPAMVPM6_EL2_PhyPARTID26                        GENMASK(47, 32)
#define MPAMVPM6_EL2_PhyPARTID26_MASK                   GENMASK(47, 32)
#define MPAMVPM6_EL2_PhyPARTID26_SHIFT                  32
#define MPAMVPM6_EL2_PhyPARTID26_WIDTH                  16

#define MPAMVPM6_EL2_PhyPARTID25                        GENMASK(31, 16)
#define MPAMVPM6_EL2_PhyPARTID25_MASK                   GENMASK(31, 16)
#define MPAMVPM6_EL2_PhyPARTID25_SHIFT                  16
#define MPAMVPM6_EL2_PhyPARTID25_WIDTH                  16

#define MPAMVPM6_EL2_PhyPARTID24                        GENMASK(15, 0)
#define MPAMVPM6_EL2_PhyPARTID24_MASK                   GENMASK(15, 0)
#define MPAMVPM6_EL2_PhyPARTID24_SHIFT                  0
#define MPAMVPM6_EL2_PhyPARTID24_WIDTH                  16

#define MPAMVPM6_EL2_RES0                               (UL(0))
#define MPAMVPM6_EL2_RES1                               (UL(0))
#define MPAMVPM6_EL2_UNKN                               (UL(0))

#define REG_MPAMVPM7_EL2                                S3_4_C10_C6_7
#define SYS_MPAMVPM7_EL2                                sys_reg(3, 4, 10, 6, 7)
#define SYS_MPAMVPM7_EL2_Op0                            3
#define SYS_MPAMVPM7_EL2_Op1                            4
#define SYS_MPAMVPM7_EL2_CRn                            10
#define SYS_MPAMVPM7_EL2_CRm                            6
#define SYS_MPAMVPM7_EL2_Op2                            7

#define MPAMVPM7_EL2_PhyPARTID31                        GENMASK(63, 48)
#define MPAMVPM7_EL2_PhyPARTID31_MASK                   GENMASK(63, 48)
#define MPAMVPM7_EL2_PhyPARTID31_SHIFT                  48
#define MPAMVPM7_EL2_PhyPARTID31_WIDTH                  16

#define MPAMVPM7_EL2_PhyPARTID30                        GENMASK(47, 32)
#define MPAMVPM7_EL2_PhyPARTID30_MASK                   GENMASK(47, 32)
#define MPAMVPM7_EL2_PhyPARTID30_SHIFT                  32
#define MPAMVPM7_EL2_PhyPARTID30_WIDTH                  16

#define MPAMVPM7_EL2_PhyPARTID29                        GENMASK(31, 16)
#define MPAMVPM7_EL2_PhyPARTID29_MASK                   GENMASK(31, 16)
#define MPAMVPM7_EL2_PhyPARTID29_SHIFT                  16
#define MPAMVPM7_EL2_PhyPARTID29_WIDTH                  16

#define MPAMVPM7_EL2_PhyPARTID28                        GENMASK(15, 0)
#define MPAMVPM7_EL2_PhyPARTID28_MASK                   GENMASK(15, 0)
#define MPAMVPM7_EL2_PhyPARTID28_SHIFT                  0
#define MPAMVPM7_EL2_PhyPARTID28_WIDTH                  16

#define MPAMVPM7_EL2_RES0                               (UL(0))
#define MPAMVPM7_EL2_RES1                               (UL(0))
#define MPAMVPM7_EL2_UNKN                               (UL(0))

#define REG_ICH_HFGRTR_EL2                              S3_4_C12_C9_4
#define SYS_ICH_HFGRTR_EL2                              sys_reg(3, 4, 12, 9, 4)
#define SYS_ICH_HFGRTR_EL2_Op0                          3
#define SYS_ICH_HFGRTR_EL2_Op1                          4
#define SYS_ICH_HFGRTR_EL2_CRn                          12
#define SYS_ICH_HFGRTR_EL2_CRm                          9
#define SYS_ICH_HFGRTR_EL2_Op2                          4

#define ICH_HFGRTR_EL2_ICC_PPI_ACTIVERn_EL1             GENMASK(20, 20)
#define ICH_HFGRTR_EL2_ICC_PPI_ACTIVERn_EL1_MASK        GENMASK(20, 20)
#define ICH_HFGRTR_EL2_ICC_PPI_ACTIVERn_EL1_SHIFT       20
#define ICH_HFGRTR_EL2_ICC_PPI_ACTIVERn_EL1_WIDTH       1

#define ICH_HFGRTR_EL2_ICC_PPI_PRIORITYRn_EL1           GENMASK(19, 19)
#define ICH_HFGRTR_EL2_ICC_PPI_PRIORITYRn_EL1_MASK      GENMASK(19, 19)
#define ICH_HFGRTR_EL2_ICC_PPI_PRIORITYRn_EL1_SHIFT     19
#define ICH_HFGRTR_EL2_ICC_PPI_PRIORITYRn_EL1_WIDTH     1

#define ICH_HFGRTR_EL2_ICC_PPI_PENDRn_EL1               GENMASK(18, 18)
#define ICH_HFGRTR_EL2_ICC_PPI_PENDRn_EL1_MASK          GENMASK(18, 18)
#define ICH_HFGRTR_EL2_ICC_PPI_PENDRn_EL1_SHIFT         18
#define ICH_HFGRTR_EL2_ICC_PPI_PENDRn_EL1_WIDTH         1

#define ICH_HFGRTR_EL2_ICC_PPI_ENABLERn_EL1             GENMASK(17, 17)
#define ICH_HFGRTR_EL2_ICC_PPI_ENABLERn_EL1_MASK        GENMASK(17, 17)
#define ICH_HFGRTR_EL2_ICC_PPI_ENABLERn_EL1_SHIFT       17
#define ICH_HFGRTR_EL2_ICC_PPI_ENABLERn_EL1_WIDTH       1

#define ICH_HFGRTR_EL2_ICC_PPI_HMRn_EL1                 GENMASK(16, 16)
#define ICH_HFGRTR_EL2_ICC_PPI_HMRn_EL1_MASK            GENMASK(16, 16)
#define ICH_HFGRTR_EL2_ICC_PPI_HMRn_EL1_SHIFT           16
#define ICH_HFGRTR_EL2_ICC_PPI_HMRn_EL1_WIDTH           1

#define ICH_HFGRTR_EL2_ICC_IAFFIDR_EL1                  GENMASK(7, 7)
#define ICH_HFGRTR_EL2_ICC_IAFFIDR_EL1_MASK             GENMASK(7, 7)
#define ICH_HFGRTR_EL2_ICC_IAFFIDR_EL1_SHIFT            7
#define ICH_HFGRTR_EL2_ICC_IAFFIDR_EL1_WIDTH            1

#define ICH_HFGRTR_EL2_ICC_ICSR_EL1                     GENMASK(6, 6)
#define ICH_HFGRTR_EL2_ICC_ICSR_EL1_MASK                GENMASK(6, 6)
#define ICH_HFGRTR_EL2_ICC_ICSR_EL1_SHIFT               6
#define ICH_HFGRTR_EL2_ICC_ICSR_EL1_WIDTH               1

#define ICH_HFGRTR_EL2_ICC_PCR_EL1                      GENMASK(5, 5)
#define ICH_HFGRTR_EL2_ICC_PCR_EL1_MASK                 GENMASK(5, 5)
#define ICH_HFGRTR_EL2_ICC_PCR_EL1_SHIFT                5
#define ICH_HFGRTR_EL2_ICC_PCR_EL1_WIDTH                1

#define ICH_HFGRTR_EL2_ICC_HPPIR_EL1                    GENMASK(4, 4)
#define ICH_HFGRTR_EL2_ICC_HPPIR_EL1_MASK               GENMASK(4, 4)
#define ICH_HFGRTR_EL2_ICC_HPPIR_EL1_SHIFT              4
#define ICH_HFGRTR_EL2_ICC_HPPIR_EL1_WIDTH              1

#define ICH_HFGRTR_EL2_ICC_HAPR_EL1                     GENMASK(3, 3)
#define ICH_HFGRTR_EL2_ICC_HAPR_EL1_MASK                GENMASK(3, 3)
#define ICH_HFGRTR_EL2_ICC_HAPR_EL1_SHIFT               3
#define ICH_HFGRTR_EL2_ICC_HAPR_EL1_WIDTH               1

#define ICH_HFGRTR_EL2_ICC_CR0_EL1                      GENMASK(2, 2)
#define ICH_HFGRTR_EL2_ICC_CR0_EL1_MASK                 GENMASK(2, 2)
#define ICH_HFGRTR_EL2_ICC_CR0_EL1_SHIFT                2
#define ICH_HFGRTR_EL2_ICC_CR0_EL1_WIDTH                1

#define ICH_HFGRTR_EL2_ICC_IDRn_EL1                     GENMASK(1, 1)
#define ICH_HFGRTR_EL2_ICC_IDRn_EL1_MASK                GENMASK(1, 1)
#define ICH_HFGRTR_EL2_ICC_IDRn_EL1_SHIFT               1
#define ICH_HFGRTR_EL2_ICC_IDRn_EL1_WIDTH               1

#define ICH_HFGRTR_EL2_ICC_APR_EL1                      GENMASK(0, 0)
#define ICH_HFGRTR_EL2_ICC_APR_EL1_MASK                 GENMASK(0, 0)
#define ICH_HFGRTR_EL2_ICC_APR_EL1_SHIFT                0
#define ICH_HFGRTR_EL2_ICC_APR_EL1_WIDTH                1

#define ICH_HFGRTR_EL2_RES0                             (UL(0) | GENMASK_ULL(63, 21) | GENMASK_ULL(15, 8))
#define ICH_HFGRTR_EL2_RES1                             (UL(0))
#define ICH_HFGRTR_EL2_UNKN                             (UL(0))

#define REG_ICH_HFGWTR_EL2                              S3_4_C12_C9_6
#define SYS_ICH_HFGWTR_EL2                              sys_reg(3, 4, 12, 9, 6)
#define SYS_ICH_HFGWTR_EL2_Op0                          3
#define SYS_ICH_HFGWTR_EL2_Op1                          4
#define SYS_ICH_HFGWTR_EL2_CRn                          12
#define SYS_ICH_HFGWTR_EL2_CRm                          9
#define SYS_ICH_HFGWTR_EL2_Op2                          6

#define ICH_HFGWTR_EL2_ICC_PPI_ACTIVERn_EL1             GENMASK(20, 20)
#define ICH_HFGWTR_EL2_ICC_PPI_ACTIVERn_EL1_MASK        GENMASK(20, 20)
#define ICH_HFGWTR_EL2_ICC_PPI_ACTIVERn_EL1_SHIFT       20
#define ICH_HFGWTR_EL2_ICC_PPI_ACTIVERn_EL1_WIDTH       1

#define ICH_HFGWTR_EL2_ICC_PPI_PRIORITYRn_EL1           GENMASK(19, 19)
#define ICH_HFGWTR_EL2_ICC_PPI_PRIORITYRn_EL1_MASK      GENMASK(19, 19)
#define ICH_HFGWTR_EL2_ICC_PPI_PRIORITYRn_EL1_SHIFT     19
#define ICH_HFGWTR_EL2_ICC_PPI_PRIORITYRn_EL1_WIDTH     1

#define ICH_HFGWTR_EL2_ICC_PPI_PENDRn_EL1               GENMASK(18, 18)
#define ICH_HFGWTR_EL2_ICC_PPI_PENDRn_EL1_MASK          GENMASK(18, 18)
#define ICH_HFGWTR_EL2_ICC_PPI_PENDRn_EL1_SHIFT         18
#define ICH_HFGWTR_EL2_ICC_PPI_PENDRn_EL1_WIDTH         1

#define ICH_HFGWTR_EL2_ICC_PPI_ENABLERn_EL1             GENMASK(17, 17)
#define ICH_HFGWTR_EL2_ICC_PPI_ENABLERn_EL1_MASK        GENMASK(17, 17)
#define ICH_HFGWTR_EL2_ICC_PPI_ENABLERn_EL1_SHIFT       17
#define ICH_HFGWTR_EL2_ICC_PPI_ENABLERn_EL1_WIDTH       1

#define ICH_HFGWTR_EL2_ICC_ICSR_EL1                     GENMASK(6, 6)
#define ICH_HFGWTR_EL2_ICC_ICSR_EL1_MASK                GENMASK(6, 6)
#define ICH_HFGWTR_EL2_ICC_ICSR_EL1_SHIFT               6
#define ICH_HFGWTR_EL2_ICC_ICSR_EL1_WIDTH               1

#define ICH_HFGWTR_EL2_ICC_PCR_EL1                      GENMASK(5, 5)
#define ICH_HFGWTR_EL2_ICC_PCR_EL1_MASK                 GENMASK(5, 5)
#define ICH_HFGWTR_EL2_ICC_PCR_EL1_SHIFT                5
#define ICH_HFGWTR_EL2_ICC_PCR_EL1_WIDTH                1

#define ICH_HFGWTR_EL2_ICC_CR0_EL1                      GENMASK(2, 2)
#define ICH_HFGWTR_EL2_ICC_CR0_EL1_MASK                 GENMASK(2, 2)
#define ICH_HFGWTR_EL2_ICC_CR0_EL1_SHIFT                2
#define ICH_HFGWTR_EL2_ICC_CR0_EL1_WIDTH                1

#define ICH_HFGWTR_EL2_ICC_APR_EL1                      GENMASK(0, 0)
#define ICH_HFGWTR_EL2_ICC_APR_EL1_MASK                 GENMASK(0, 0)
#define ICH_HFGWTR_EL2_ICC_APR_EL1_SHIFT                0
#define ICH_HFGWTR_EL2_ICC_APR_EL1_WIDTH                1

#define ICH_HFGWTR_EL2_RES0                             (UL(0) | GENMASK_ULL(63, 21) | GENMASK_ULL(16, 7) | GENMASK_ULL(4, 3) | GENMASK_ULL(1, 1))
#define ICH_HFGWTR_EL2_RES1                             (UL(0))
#define ICH_HFGWTR_EL2_UNKN                             (UL(0))

#define REG_ICH_HFGITR_EL2                              S3_4_C12_C9_7
#define SYS_ICH_HFGITR_EL2                              sys_reg(3, 4, 12, 9, 7)
#define SYS_ICH_HFGITR_EL2_Op0                          3
#define SYS_ICH_HFGITR_EL2_Op1                          4
#define SYS_ICH_HFGITR_EL2_CRn                          12
#define SYS_ICH_HFGITR_EL2_CRm                          9
#define SYS_ICH_HFGITR_EL2_Op2                          7

#define ICH_HFGITR_EL2_GICRCDNMIA                       GENMASK(10, 10)
#define ICH_HFGITR_EL2_GICRCDNMIA_MASK                  GENMASK(10, 10)
#define ICH_HFGITR_EL2_GICRCDNMIA_SHIFT                 10
#define ICH_HFGITR_EL2_GICRCDNMIA_WIDTH                 1

#define ICH_HFGITR_EL2_GICRCDIA                         GENMASK(9, 9)
#define ICH_HFGITR_EL2_GICRCDIA_MASK                    GENMASK(9, 9)
#define ICH_HFGITR_EL2_GICRCDIA_SHIFT                   9
#define ICH_HFGITR_EL2_GICRCDIA_WIDTH                   1

#define ICH_HFGITR_EL2_GICCDDI                          GENMASK(8, 8)
#define ICH_HFGITR_EL2_GICCDDI_MASK                     GENMASK(8, 8)
#define ICH_HFGITR_EL2_GICCDDI_SHIFT                    8
#define ICH_HFGITR_EL2_GICCDDI_WIDTH                    1

#define ICH_HFGITR_EL2_GICCDEOI                         GENMASK(7, 7)
#define ICH_HFGITR_EL2_GICCDEOI_MASK                    GENMASK(7, 7)
#define ICH_HFGITR_EL2_GICCDEOI_SHIFT                   7
#define ICH_HFGITR_EL2_GICCDEOI_WIDTH                   1

#define ICH_HFGITR_EL2_GICCDHM                          GENMASK(6, 6)
#define ICH_HFGITR_EL2_GICCDHM_MASK                     GENMASK(6, 6)
#define ICH_HFGITR_EL2_GICCDHM_SHIFT                    6
#define ICH_HFGITR_EL2_GICCDHM_WIDTH                    1

#define ICH_HFGITR_EL2_GICCDRCFG                        GENMASK(5, 5)
#define ICH_HFGITR_EL2_GICCDRCFG_MASK                   GENMASK(5, 5)
#define ICH_HFGITR_EL2_GICCDRCFG_SHIFT                  5
#define ICH_HFGITR_EL2_GICCDRCFG_WIDTH                  1

#define ICH_HFGITR_EL2_GICCDPEND                        GENMASK(4, 4)
#define ICH_HFGITR_EL2_GICCDPEND_MASK                   GENMASK(4, 4)
#define ICH_HFGITR_EL2_GICCDPEND_SHIFT                  4
#define ICH_HFGITR_EL2_GICCDPEND_WIDTH                  1

#define ICH_HFGITR_EL2_GICCDAFF                         GENMASK(3, 3)
#define ICH_HFGITR_EL2_GICCDAFF_MASK                    GENMASK(3, 3)
#define ICH_HFGITR_EL2_GICCDAFF_SHIFT                   3
#define ICH_HFGITR_EL2_GICCDAFF_WIDTH                   1

#define ICH_HFGITR_EL2_GICCDPRI                         GENMASK(2, 2)
#define ICH_HFGITR_EL2_GICCDPRI_MASK                    GENMASK(2, 2)
#define ICH_HFGITR_EL2_GICCDPRI_SHIFT                   2
#define ICH_HFGITR_EL2_GICCDPRI_WIDTH                   1

#define ICH_HFGITR_EL2_GICCDDIS                         GENMASK(1, 1)
#define ICH_HFGITR_EL2_GICCDDIS_MASK                    GENMASK(1, 1)
#define ICH_HFGITR_EL2_GICCDDIS_SHIFT                   1
#define ICH_HFGITR_EL2_GICCDDIS_WIDTH                   1

#define ICH_HFGITR_EL2_GICCDEN                          GENMASK(0, 0)
#define ICH_HFGITR_EL2_GICCDEN_MASK                     GENMASK(0, 0)
#define ICH_HFGITR_EL2_GICCDEN_SHIFT                    0
#define ICH_HFGITR_EL2_GICCDEN_WIDTH                    1

#define ICH_HFGITR_EL2_RES0                             (UL(0) | GENMASK_ULL(63, 11))
#define ICH_HFGITR_EL2_RES1                             (UL(0))
#define ICH_HFGITR_EL2_UNKN                             (UL(0))

#define REG_ICH_HCR_EL2                                 S3_4_C12_C11_0
#define SYS_ICH_HCR_EL2                                 sys_reg(3, 4, 12, 11, 0)
#define SYS_ICH_HCR_EL2_Op0                             3
#define SYS_ICH_HCR_EL2_Op1                             4
#define SYS_ICH_HCR_EL2_CRn                             12
#define SYS_ICH_HCR_EL2_CRm                             11
#define SYS_ICH_HCR_EL2_Op2                             0

#define ICH_HCR_EL2_EOIcount                            GENMASK(31, 27)
#define ICH_HCR_EL2_EOIcount_MASK                       GENMASK(31, 27)
#define ICH_HCR_EL2_EOIcount_SHIFT                      27
#define ICH_HCR_EL2_EOIcount_WIDTH                      5

#define ICH_HCR_EL2_DVIM                                GENMASK(15, 15)
#define ICH_HCR_EL2_DVIM_MASK                           GENMASK(15, 15)
#define ICH_HCR_EL2_DVIM_SHIFT                          15
#define ICH_HCR_EL2_DVIM_WIDTH                          1

#define ICH_HCR_EL2_TDIR                                GENMASK(14, 14)
#define ICH_HCR_EL2_TDIR_MASK                           GENMASK(14, 14)
#define ICH_HCR_EL2_TDIR_SHIFT                          14
#define ICH_HCR_EL2_TDIR_WIDTH                          1

#define ICH_HCR_EL2_TSEI                                GENMASK(13, 13)
#define ICH_HCR_EL2_TSEI_MASK                           GENMASK(13, 13)
#define ICH_HCR_EL2_TSEI_SHIFT                          13
#define ICH_HCR_EL2_TSEI_WIDTH                          1

#define ICH_HCR_EL2_TALL1                               GENMASK(12, 12)
#define ICH_HCR_EL2_TALL1_MASK                          GENMASK(12, 12)
#define ICH_HCR_EL2_TALL1_SHIFT                         12
#define ICH_HCR_EL2_TALL1_WIDTH                         1

#define ICH_HCR_EL2_TALL0                               GENMASK(11, 11)
#define ICH_HCR_EL2_TALL0_MASK                          GENMASK(11, 11)
#define ICH_HCR_EL2_TALL0_SHIFT                         11
#define ICH_HCR_EL2_TALL0_WIDTH                         1

#define ICH_HCR_EL2_TC                                  GENMASK(10, 10)
#define ICH_HCR_EL2_TC_MASK                             GENMASK(10, 10)
#define ICH_HCR_EL2_TC_SHIFT                            10
#define ICH_HCR_EL2_TC_WIDTH                            1

#define ICH_HCR_EL2_vSGIEOICount                        GENMASK(8, 8)
#define ICH_HCR_EL2_vSGIEOICount_MASK                   GENMASK(8, 8)
#define ICH_HCR_EL2_vSGIEOICount_SHIFT                  8
#define ICH_HCR_EL2_vSGIEOICount_WIDTH                  1

#define ICH_HCR_EL2_VGrp1DIE                            GENMASK(7, 7)
#define ICH_HCR_EL2_VGrp1DIE_MASK                       GENMASK(7, 7)
#define ICH_HCR_EL2_VGrp1DIE_SHIFT                      7
#define ICH_HCR_EL2_VGrp1DIE_WIDTH                      1

#define ICH_HCR_EL2_VGrp1EIE                            GENMASK(6, 6)
#define ICH_HCR_EL2_VGrp1EIE_MASK                       GENMASK(6, 6)
#define ICH_HCR_EL2_VGrp1EIE_SHIFT                      6
#define ICH_HCR_EL2_VGrp1EIE_WIDTH                      1

#define ICH_HCR_EL2_VGrp0DIE                            GENMASK(5, 5)
#define ICH_HCR_EL2_VGrp0DIE_MASK                       GENMASK(5, 5)
#define ICH_HCR_EL2_VGrp0DIE_SHIFT                      5
#define ICH_HCR_EL2_VGrp0DIE_WIDTH                      1

#define ICH_HCR_EL2_VGrp0EIE                            GENMASK(4, 4)
#define ICH_HCR_EL2_VGrp0EIE_MASK                       GENMASK(4, 4)
#define ICH_HCR_EL2_VGrp0EIE_SHIFT                      4
#define ICH_HCR_EL2_VGrp0EIE_WIDTH                      1

#define ICH_HCR_EL2_NPIE                                GENMASK(3, 3)
#define ICH_HCR_EL2_NPIE_MASK                           GENMASK(3, 3)
#define ICH_HCR_EL2_NPIE_SHIFT                          3
#define ICH_HCR_EL2_NPIE_WIDTH                          1

#define ICH_HCR_EL2_LRENPIE                             GENMASK(2, 2)
#define ICH_HCR_EL2_LRENPIE_MASK                        GENMASK(2, 2)
#define ICH_HCR_EL2_LRENPIE_SHIFT                       2
#define ICH_HCR_EL2_LRENPIE_WIDTH                       1

#define ICH_HCR_EL2_UIE                                 GENMASK(1, 1)
#define ICH_HCR_EL2_UIE_MASK                            GENMASK(1, 1)
#define ICH_HCR_EL2_UIE_SHIFT                           1
#define ICH_HCR_EL2_UIE_WIDTH                           1

#define ICH_HCR_EL2_En                                  GENMASK(0, 0)
#define ICH_HCR_EL2_En_MASK                             GENMASK(0, 0)
#define ICH_HCR_EL2_En_SHIFT                            0
#define ICH_HCR_EL2_En_WIDTH                            1

#define ICH_HCR_EL2_RES0                                (UL(0) | GENMASK_ULL(63, 32) | GENMASK_ULL(26, 16) | GENMASK_ULL(9, 9))
#define ICH_HCR_EL2_RES1                                (UL(0))
#define ICH_HCR_EL2_UNKN                                (UL(0))

#define REG_ICH_VTR_EL2                                 S3_4_C12_C11_1
#define SYS_ICH_VTR_EL2                                 sys_reg(3, 4, 12, 11, 1)
#define SYS_ICH_VTR_EL2_Op0                             3
#define SYS_ICH_VTR_EL2_Op1                             4
#define SYS_ICH_VTR_EL2_CRn                             12
#define SYS_ICH_VTR_EL2_CRm                             11
#define SYS_ICH_VTR_EL2_Op2                             1

#define ICH_VTR_EL2_PRIbits                             GENMASK(31, 29)
#define ICH_VTR_EL2_PRIbits_MASK                        GENMASK(31, 29)
#define ICH_VTR_EL2_PRIbits_SHIFT                       29
#define ICH_VTR_EL2_PRIbits_WIDTH                       3

#define ICH_VTR_EL2_PREbits                             GENMASK(28, 26)
#define ICH_VTR_EL2_PREbits_MASK                        GENMASK(28, 26)
#define ICH_VTR_EL2_PREbits_SHIFT                       26
#define ICH_VTR_EL2_PREbits_WIDTH                       3

#define ICH_VTR_EL2_IDbits                              GENMASK(25, 23)
#define ICH_VTR_EL2_IDbits_MASK                         GENMASK(25, 23)
#define ICH_VTR_EL2_IDbits_SHIFT                        23
#define ICH_VTR_EL2_IDbits_WIDTH                        3

#define ICH_VTR_EL2_SEIS                                GENMASK(22, 22)
#define ICH_VTR_EL2_SEIS_MASK                           GENMASK(22, 22)
#define ICH_VTR_EL2_SEIS_SHIFT                          22
#define ICH_VTR_EL2_SEIS_WIDTH                          1

#define ICH_VTR_EL2_A3V                                 GENMASK(21, 21)
#define ICH_VTR_EL2_A3V_MASK                            GENMASK(21, 21)
#define ICH_VTR_EL2_A3V_SHIFT                           21
#define ICH_VTR_EL2_A3V_WIDTH                           1

#define ICH_VTR_EL2_nV4                                 GENMASK(20, 20)
#define ICH_VTR_EL2_nV4_MASK                            GENMASK(20, 20)
#define ICH_VTR_EL2_nV4_SHIFT                           20
#define ICH_VTR_EL2_nV4_WIDTH                           1

#define ICH_VTR_EL2_TDS                                 GENMASK(19, 19)
#define ICH_VTR_EL2_TDS_MASK                            GENMASK(19, 19)
#define ICH_VTR_EL2_TDS_SHIFT                           19
#define ICH_VTR_EL2_TDS_WIDTH                           1

#define ICH_VTR_EL2_DVIM                                GENMASK(18, 18)
#define ICH_VTR_EL2_DVIM_MASK                           GENMASK(18, 18)
#define ICH_VTR_EL2_DVIM_SHIFT                          18
#define ICH_VTR_EL2_DVIM_WIDTH                          1

#define ICH_VTR_EL2_ListRegs                            GENMASK(4, 0)
#define ICH_VTR_EL2_ListRegs_MASK                       GENMASK(4, 0)
#define ICH_VTR_EL2_ListRegs_SHIFT                      0
#define ICH_VTR_EL2_ListRegs_WIDTH                      5

#define ICH_VTR_EL2_RES0                                (UL(0) | GENMASK_ULL(63, 32) | GENMASK_ULL(17, 5))
#define ICH_VTR_EL2_RES1                                (UL(0))
#define ICH_VTR_EL2_UNKN                                (UL(0))

#define REG_ICH_MISR_EL2                                S3_4_C12_C11_2
#define SYS_ICH_MISR_EL2                                sys_reg(3, 4, 12, 11, 2)
#define SYS_ICH_MISR_EL2_Op0                            3
#define SYS_ICH_MISR_EL2_Op1                            4
#define SYS_ICH_MISR_EL2_CRn                            12
#define SYS_ICH_MISR_EL2_CRm                            11
#define SYS_ICH_MISR_EL2_Op2                            2

#define ICH_MISR_EL2_VGrp1D                             GENMASK(7, 7)
#define ICH_MISR_EL2_VGrp1D_MASK                        GENMASK(7, 7)
#define ICH_MISR_EL2_VGrp1D_SHIFT                       7
#define ICH_MISR_EL2_VGrp1D_WIDTH                       1

#define ICH_MISR_EL2_VGrp1E                             GENMASK(6, 6)
#define ICH_MISR_EL2_VGrp1E_MASK                        GENMASK(6, 6)
#define ICH_MISR_EL2_VGrp1E_SHIFT                       6
#define ICH_MISR_EL2_VGrp1E_WIDTH                       1

#define ICH_MISR_EL2_VGrp0D                             GENMASK(5, 5)
#define ICH_MISR_EL2_VGrp0D_MASK                        GENMASK(5, 5)
#define ICH_MISR_EL2_VGrp0D_SHIFT                       5
#define ICH_MISR_EL2_VGrp0D_WIDTH                       1

#define ICH_MISR_EL2_VGrp0E                             GENMASK(4, 4)
#define ICH_MISR_EL2_VGrp0E_MASK                        GENMASK(4, 4)
#define ICH_MISR_EL2_VGrp0E_SHIFT                       4
#define ICH_MISR_EL2_VGrp0E_WIDTH                       1

#define ICH_MISR_EL2_NP                                 GENMASK(3, 3)
#define ICH_MISR_EL2_NP_MASK                            GENMASK(3, 3)
#define ICH_MISR_EL2_NP_SHIFT                           3
#define ICH_MISR_EL2_NP_WIDTH                           1

#define ICH_MISR_EL2_LRENP                              GENMASK(2, 2)
#define ICH_MISR_EL2_LRENP_MASK                         GENMASK(2, 2)
#define ICH_MISR_EL2_LRENP_SHIFT                        2
#define ICH_MISR_EL2_LRENP_WIDTH                        1

#define ICH_MISR_EL2_U                                  GENMASK(1, 1)
#define ICH_MISR_EL2_U_MASK                             GENMASK(1, 1)
#define ICH_MISR_EL2_U_SHIFT                            1
#define ICH_MISR_EL2_U_WIDTH                            1

#define ICH_MISR_EL2_EOI                                GENMASK(0, 0)
#define ICH_MISR_EL2_EOI_MASK                           GENMASK(0, 0)
#define ICH_MISR_EL2_EOI_SHIFT                          0
#define ICH_MISR_EL2_EOI_WIDTH                          1

#define ICH_MISR_EL2_RES0                               (UL(0) | GENMASK_ULL(63, 8))
#define ICH_MISR_EL2_RES1                               (UL(0))
#define ICH_MISR_EL2_UNKN                               (UL(0))

#define REG_ICH_VCTLR_EL2                               S3_4_C12_C11_4
#define SYS_ICH_VCTLR_EL2                               sys_reg(3, 4, 12, 11, 4)
#define SYS_ICH_VCTLR_EL2_Op0                           3
#define SYS_ICH_VCTLR_EL2_Op1                           4
#define SYS_ICH_VCTLR_EL2_CRn                           12
#define SYS_ICH_VCTLR_EL2_CRm                           11
#define SYS_ICH_VCTLR_EL2_Op2                           4

#define ICH_VCTLR_EL2_V3                                GENMASK(1, 1)
#define ICH_VCTLR_EL2_V3_MASK                           GENMASK(1, 1)
#define ICH_VCTLR_EL2_V3_SHIFT                          1
#define ICH_VCTLR_EL2_V3_WIDTH                          1

#define ICH_VCTLR_EL2_En                                GENMASK(0, 0)
#define ICH_VCTLR_EL2_En_MASK                           GENMASK(0, 0)
#define ICH_VCTLR_EL2_En_SHIFT                          0
#define ICH_VCTLR_EL2_En_WIDTH                          1

#define ICH_VCTLR_EL2_RES0                              (UL(0) | GENMASK_ULL(63, 2))
#define ICH_VCTLR_EL2_RES1                              (UL(0))
#define ICH_VCTLR_EL2_UNKN                              (UL(0))

#define REG_CONTEXTIDR_EL2                              S3_4_C13_C0_1
#define SYS_CONTEXTIDR_EL2                              sys_reg(3, 4, 13, 0, 1)
#define SYS_CONTEXTIDR_EL2_Op0                          3
#define SYS_CONTEXTIDR_EL2_Op1                          4
#define SYS_CONTEXTIDR_EL2_CRn                          13
#define SYS_CONTEXTIDR_EL2_CRm                          0
#define SYS_CONTEXTIDR_EL2_Op2                          1

/* For CONTEXTIDR_EL2 fields see CONTEXTIDR_ELx */

#define REG_CNTPOFF_EL2                                 S3_4_C14_C0_6
#define SYS_CNTPOFF_EL2                                 sys_reg(3, 4, 14, 0, 6)
#define SYS_CNTPOFF_EL2_Op0                             3
#define SYS_CNTPOFF_EL2_Op1                             4
#define SYS_CNTPOFF_EL2_CRn                             14
#define SYS_CNTPOFF_EL2_CRm                             0
#define SYS_CNTPOFF_EL2_Op2                             6

#define CNTPOFF_EL2_PhysicalOffset                      GENMASK(63, 0)
#define CNTPOFF_EL2_PhysicalOffset_MASK                 GENMASK(63, 0)
#define CNTPOFF_EL2_PhysicalOffset_SHIFT                0
#define CNTPOFF_EL2_PhysicalOffset_WIDTH                64

#define CNTPOFF_EL2_RES0                                (UL(0))
#define CNTPOFF_EL2_RES1                                (UL(0))
#define CNTPOFF_EL2_UNKN                                (UL(0))

#define REG_CPACR_EL12                                  S3_5_C1_C0_2
#define SYS_CPACR_EL12                                  sys_reg(3, 5, 1, 0, 2)
#define SYS_CPACR_EL12_Op0                              3
#define SYS_CPACR_EL12_Op1                              5
#define SYS_CPACR_EL12_CRn                              1
#define SYS_CPACR_EL12_CRm                              0
#define SYS_CPACR_EL12_Op2                              2

/* For CPACR_EL12 fields see CPACR_EL1 */

#define REG_ZCR_EL12                                    S3_5_C1_C2_0
#define SYS_ZCR_EL12                                    sys_reg(3, 5, 1, 2, 0)
#define SYS_ZCR_EL12_Op0                                3
#define SYS_ZCR_EL12_Op1                                5
#define SYS_ZCR_EL12_CRn                                1
#define SYS_ZCR_EL12_CRm                                2
#define SYS_ZCR_EL12_Op2                                0

/* For ZCR_EL12 fields see ZCR_EL1 */

#define REG_TRFCR_EL12                                  S3_5_C1_C2_1
#define SYS_TRFCR_EL12                                  sys_reg(3, 5, 1, 2, 1)
#define SYS_TRFCR_EL12_Op0                              3
#define SYS_TRFCR_EL12_Op1                              5
#define SYS_TRFCR_EL12_CRn                              1
#define SYS_TRFCR_EL12_CRm                              2
#define SYS_TRFCR_EL12_Op2                              1

/* For TRFCR_EL12 fields see TRFCR_EL1 */

#define REG_SMCR_EL12                                   S3_5_C1_C2_6
#define SYS_SMCR_EL12                                   sys_reg(3, 5, 1, 2, 6)
#define SYS_SMCR_EL12_Op0                               3
#define SYS_SMCR_EL12_Op1                               5
#define SYS_SMCR_EL12_CRn                               1
#define SYS_SMCR_EL12_CRm                               2
#define SYS_SMCR_EL12_Op2                               6

/* For SMCR_EL12 fields see SMCR_EL1 */

#define REG_GCSCR_EL12                                  S3_5_C2_C5_0
#define SYS_GCSCR_EL12                                  sys_reg(3, 5, 2, 5, 0)
#define SYS_GCSCR_EL12_Op0                              3
#define SYS_GCSCR_EL12_Op1                              5
#define SYS_GCSCR_EL12_CRn                              2
#define SYS_GCSCR_EL12_CRm                              5
#define SYS_GCSCR_EL12_Op2                              0

/* For GCSCR_EL12 fields see GCSCR_EL1 */

#define REG_GCSPR_EL12                                  S3_5_C2_C5_1
#define SYS_GCSPR_EL12                                  sys_reg(3, 5, 2, 5, 1)
#define SYS_GCSPR_EL12_Op0                              3
#define SYS_GCSPR_EL12_Op1                              5
#define SYS_GCSPR_EL12_CRn                              2
#define SYS_GCSPR_EL12_CRm                              5
#define SYS_GCSPR_EL12_Op2                              1

/* For GCSPR_EL12 fields see GCSPR_EL1 */

#define REG_FAR_EL12                                    S3_5_C6_C0_0
#define SYS_FAR_EL12                                    sys_reg(3, 5, 6, 0, 0)
#define SYS_FAR_EL12_Op0                                3
#define SYS_FAR_EL12_Op1                                5
#define SYS_FAR_EL12_CRn                                6
#define SYS_FAR_EL12_CRm                                0
#define SYS_FAR_EL12_Op2                                0

#define FAR_EL12_ADDR                                   GENMASK(63, 0)
#define FAR_EL12_ADDR_MASK                              GENMASK(63, 0)
#define FAR_EL12_ADDR_SHIFT                             0
#define FAR_EL12_ADDR_WIDTH                             64

#define FAR_EL12_RES0                                   (UL(0))
#define FAR_EL12_RES1                                   (UL(0))
#define FAR_EL12_UNKN                                   (UL(0))

#define REG_MPAM1_EL12                                  S3_5_C10_C5_0
#define SYS_MPAM1_EL12                                  sys_reg(3, 5, 10, 5, 0)
#define SYS_MPAM1_EL12_Op0                              3
#define SYS_MPAM1_EL12_Op1                              5
#define SYS_MPAM1_EL12_CRn                              10
#define SYS_MPAM1_EL12_CRm                              5
#define SYS_MPAM1_EL12_Op2                              0

/* For MPAM1_EL12 fields see MPAM1_ELx */

#define REG_CONTEXTIDR_EL12                             S3_5_C13_C0_1
#define SYS_CONTEXTIDR_EL12                             sys_reg(3, 5, 13, 0, 1)
#define SYS_CONTEXTIDR_EL12_Op0                         3
#define SYS_CONTEXTIDR_EL12_Op1                         5
#define SYS_CONTEXTIDR_EL12_CRn                         13
#define SYS_CONTEXTIDR_EL12_CRm                         0
#define SYS_CONTEXTIDR_EL12_Op2                         1

/* For CONTEXTIDR_EL12 fields see CONTEXTIDR_EL1 */

#define TTBRx_EL1_ASID                                  GENMASK(63, 48)
#define TTBRx_EL1_ASID_MASK                             GENMASK(63, 48)
#define TTBRx_EL1_ASID_SHIFT                            48
#define TTBRx_EL1_ASID_WIDTH                            16

#define TTBRx_EL1_BADDR                                 GENMASK(47, 1)
#define TTBRx_EL1_BADDR_MASK                            GENMASK(47, 1)
#define TTBRx_EL1_BADDR_SHIFT                           1
#define TTBRx_EL1_BADDR_WIDTH                           47

#define TTBRx_EL1_CnP                                   GENMASK(0, 0)
#define TTBRx_EL1_CnP_MASK                              GENMASK(0, 0)
#define TTBRx_EL1_CnP_SHIFT                             0
#define TTBRx_EL1_CnP_WIDTH                             1

#define TTBRx_EL1_RES0                                  (UL(0))
#define TTBRx_EL1_RES1                                  (UL(0))
#define TTBRx_EL1_UNKN                                  (UL(0))

#define REG_TTBR0_EL1                                   S3_0_C2_C0_0
#define SYS_TTBR0_EL1                                   sys_reg(3, 0, 2, 0, 0)
#define SYS_TTBR0_EL1_Op0                               3
#define SYS_TTBR0_EL1_Op1                               0
#define SYS_TTBR0_EL1_CRn                               2
#define SYS_TTBR0_EL1_CRm                               0
#define SYS_TTBR0_EL1_Op2                               0

/* For TTBR0_EL1 fields see TTBRx_EL1 */

#define REG_TTBR1_EL1                                   S3_0_C2_C0_1
#define SYS_TTBR1_EL1                                   sys_reg(3, 0, 2, 0, 1)
#define SYS_TTBR1_EL1_Op0                               3
#define SYS_TTBR1_EL1_Op1                               0
#define SYS_TTBR1_EL1_CRn                               2
#define SYS_TTBR1_EL1_CRm                               0
#define SYS_TTBR1_EL1_Op2                               1

/* For TTBR1_EL1 fields see TTBRx_EL1 */

#define REG_TCR_EL1                                     S3_0_C2_C0_2
#define SYS_TCR_EL1                                     sys_reg(3, 0, 2, 0, 2)
#define SYS_TCR_EL1_Op0                                 3
#define SYS_TCR_EL1_Op1                                 0
#define SYS_TCR_EL1_CRn                                 2
#define SYS_TCR_EL1_CRm                                 0
#define SYS_TCR_EL1_Op2                                 2

#define TCR_EL1_MTX1                                    GENMASK(61, 61)
#define TCR_EL1_MTX1_MASK                               GENMASK(61, 61)
#define TCR_EL1_MTX1_SHIFT                              61
#define TCR_EL1_MTX1_WIDTH                              1

#define TCR_EL1_MTX0                                    GENMASK(60, 60)
#define TCR_EL1_MTX0_MASK                               GENMASK(60, 60)
#define TCR_EL1_MTX0_SHIFT                              60
#define TCR_EL1_MTX0_WIDTH                              1

#define TCR_EL1_DS                                      GENMASK(59, 59)
#define TCR_EL1_DS_MASK                                 GENMASK(59, 59)
#define TCR_EL1_DS_SHIFT                                59
#define TCR_EL1_DS_WIDTH                                1

#define TCR_EL1_TCMA1                                   GENMASK(58, 58)
#define TCR_EL1_TCMA1_MASK                              GENMASK(58, 58)
#define TCR_EL1_TCMA1_SHIFT                             58
#define TCR_EL1_TCMA1_WIDTH                             1

#define TCR_EL1_TCMA0                                   GENMASK(57, 57)
#define TCR_EL1_TCMA0_MASK                              GENMASK(57, 57)
#define TCR_EL1_TCMA0_SHIFT                             57
#define TCR_EL1_TCMA0_WIDTH                             1

#define TCR_EL1_E0PD1                                   GENMASK(56, 56)
#define TCR_EL1_E0PD1_MASK                              GENMASK(56, 56)
#define TCR_EL1_E0PD1_SHIFT                             56
#define TCR_EL1_E0PD1_WIDTH                             1

#define TCR_EL1_E0PD0                                   GENMASK(55, 55)
#define TCR_EL1_E0PD0_MASK                              GENMASK(55, 55)
#define TCR_EL1_E0PD0_SHIFT                             55
#define TCR_EL1_E0PD0_WIDTH                             1

#define TCR_EL1_NFD1                                    GENMASK(54, 54)
#define TCR_EL1_NFD1_MASK                               GENMASK(54, 54)
#define TCR_EL1_NFD1_SHIFT                              54
#define TCR_EL1_NFD1_WIDTH                              1

#define TCR_EL1_NFD0                                    GENMASK(53, 53)
#define TCR_EL1_NFD0_MASK                               GENMASK(53, 53)
#define TCR_EL1_NFD0_SHIFT                              53
#define TCR_EL1_NFD0_WIDTH                              1

#define TCR_EL1_TBID1                                   GENMASK(52, 52)
#define TCR_EL1_TBID1_MASK                              GENMASK(52, 52)
#define TCR_EL1_TBID1_SHIFT                             52
#define TCR_EL1_TBID1_WIDTH                             1

#define TCR_EL1_TBID0                                   GENMASK(51, 51)
#define TCR_EL1_TBID0_MASK                              GENMASK(51, 51)
#define TCR_EL1_TBID0_SHIFT                             51
#define TCR_EL1_TBID0_WIDTH                             1

#define TCR_EL1_HWU162                                  GENMASK(50, 50)
#define TCR_EL1_HWU162_MASK                             GENMASK(50, 50)
#define TCR_EL1_HWU162_SHIFT                            50
#define TCR_EL1_HWU162_WIDTH                            1

#define TCR_EL1_HWU161                                  GENMASK(49, 49)
#define TCR_EL1_HWU161_MASK                             GENMASK(49, 49)
#define TCR_EL1_HWU161_SHIFT                            49
#define TCR_EL1_HWU161_WIDTH                            1

#define TCR_EL1_HWU160                                  GENMASK(48, 48)
#define TCR_EL1_HWU160_MASK                             GENMASK(48, 48)
#define TCR_EL1_HWU160_SHIFT                            48
#define TCR_EL1_HWU160_WIDTH                            1

#define TCR_EL1_HWU159                                  GENMASK(47, 47)
#define TCR_EL1_HWU159_MASK                             GENMASK(47, 47)
#define TCR_EL1_HWU159_SHIFT                            47
#define TCR_EL1_HWU159_WIDTH                            1

#define TCR_EL1_HWU062                                  GENMASK(46, 46)
#define TCR_EL1_HWU062_MASK                             GENMASK(46, 46)
#define TCR_EL1_HWU062_SHIFT                            46
#define TCR_EL1_HWU062_WIDTH                            1

#define TCR_EL1_HWU061                                  GENMASK(45, 45)
#define TCR_EL1_HWU061_MASK                             GENMASK(45, 45)
#define TCR_EL1_HWU061_SHIFT                            45
#define TCR_EL1_HWU061_WIDTH                            1

#define TCR_EL1_HWU060                                  GENMASK(44, 44)
#define TCR_EL1_HWU060_MASK                             GENMASK(44, 44)
#define TCR_EL1_HWU060_SHIFT                            44
#define TCR_EL1_HWU060_WIDTH                            1

#define TCR_EL1_HWU059                                  GENMASK(43, 43)
#define TCR_EL1_HWU059_MASK                             GENMASK(43, 43)
#define TCR_EL1_HWU059_SHIFT                            43
#define TCR_EL1_HWU059_WIDTH                            1

#define TCR_EL1_HPD1                                    GENMASK(42, 42)
#define TCR_EL1_HPD1_MASK                               GENMASK(42, 42)
#define TCR_EL1_HPD1_SHIFT                              42
#define TCR_EL1_HPD1_WIDTH                              1

#define TCR_EL1_HPD0                                    GENMASK(41, 41)
#define TCR_EL1_HPD0_MASK                               GENMASK(41, 41)
#define TCR_EL1_HPD0_SHIFT                              41
#define TCR_EL1_HPD0_WIDTH                              1

#define TCR_EL1_HD                                      GENMASK(40, 40)
#define TCR_EL1_HD_MASK                                 GENMASK(40, 40)
#define TCR_EL1_HD_SHIFT                                40
#define TCR_EL1_HD_WIDTH                                1

#define TCR_EL1_HA                                      GENMASK(39, 39)
#define TCR_EL1_HA_MASK                                 GENMASK(39, 39)
#define TCR_EL1_HA_SHIFT                                39
#define TCR_EL1_HA_WIDTH                                1

#define TCR_EL1_TBI1                                    GENMASK(38, 38)
#define TCR_EL1_TBI1_MASK                               GENMASK(38, 38)
#define TCR_EL1_TBI1_SHIFT                              38
#define TCR_EL1_TBI1_WIDTH                              1

#define TCR_EL1_TBI0                                    GENMASK(37, 37)
#define TCR_EL1_TBI0_MASK                               GENMASK(37, 37)
#define TCR_EL1_TBI0_SHIFT                              37
#define TCR_EL1_TBI0_WIDTH                              1

#define TCR_EL1_AS                                      GENMASK(36, 36)
#define TCR_EL1_AS_MASK                                 GENMASK(36, 36)
#define TCR_EL1_AS_SHIFT                                36
#define TCR_EL1_AS_WIDTH                                1

#define TCR_EL1_IPS                                     GENMASK(34, 32)
#define TCR_EL1_IPS_MASK                                GENMASK(34, 32)
#define TCR_EL1_IPS_SHIFT                               32
#define TCR_EL1_IPS_WIDTH                               3

#define TCR_EL1_TG1                                     GENMASK(31, 30)
#define TCR_EL1_TG1_MASK                                GENMASK(31, 30)
#define TCR_EL1_TG1_SHIFT                               30
#define TCR_EL1_TG1_WIDTH                               2
#define TCR_EL1_TG1_16K                                 UL(0b01)
#define TCR_EL1_TG1_4K                                  UL(0b10)
#define TCR_EL1_TG1_64K                                 UL(0b11)

#define TCR_EL1_SH1                                     GENMASK(29, 28)
#define TCR_EL1_SH1_MASK                                GENMASK(29, 28)
#define TCR_EL1_SH1_SHIFT                               28
#define TCR_EL1_SH1_WIDTH                               2
#define TCR_EL1_SH1_NONE                                UL(0b00)
#define TCR_EL1_SH1_OUTER                               UL(0b10)
#define TCR_EL1_SH1_INNER                               UL(0b11)

#define TCR_EL1_ORGN1                                   GENMASK(27, 26)
#define TCR_EL1_ORGN1_MASK                              GENMASK(27, 26)
#define TCR_EL1_ORGN1_SHIFT                             26
#define TCR_EL1_ORGN1_WIDTH                             2
#define TCR_EL1_ORGN1_NC                                UL(0b00)
#define TCR_EL1_ORGN1_WBWA                              UL(0b01)
#define TCR_EL1_ORGN1_WT                                UL(0b10)
#define TCR_EL1_ORGN1_WBnWA                             UL(0b11)

#define TCR_EL1_IRGN1                                   GENMASK(25, 24)
#define TCR_EL1_IRGN1_MASK                              GENMASK(25, 24)
#define TCR_EL1_IRGN1_SHIFT                             24
#define TCR_EL1_IRGN1_WIDTH                             2
#define TCR_EL1_IRGN1_NC                                UL(0b00)
#define TCR_EL1_IRGN1_WBWA                              UL(0b01)
#define TCR_EL1_IRGN1_WT                                UL(0b10)
#define TCR_EL1_IRGN1_WBnWA                             UL(0b11)

#define TCR_EL1_EPD1                                    GENMASK(23, 23)
#define TCR_EL1_EPD1_MASK                               GENMASK(23, 23)
#define TCR_EL1_EPD1_SHIFT                              23
#define TCR_EL1_EPD1_WIDTH                              1

#define TCR_EL1_A1                                      GENMASK(22, 22)
#define TCR_EL1_A1_MASK                                 GENMASK(22, 22)
#define TCR_EL1_A1_SHIFT                                22
#define TCR_EL1_A1_WIDTH                                1

#define TCR_EL1_T1SZ                                    GENMASK(21, 16)
#define TCR_EL1_T1SZ_MASK                               GENMASK(21, 16)
#define TCR_EL1_T1SZ_SHIFT                              16
#define TCR_EL1_T1SZ_WIDTH                              6

#define TCR_EL1_TG0                                     GENMASK(15, 14)
#define TCR_EL1_TG0_MASK                                GENMASK(15, 14)
#define TCR_EL1_TG0_SHIFT                               14
#define TCR_EL1_TG0_WIDTH                               2
#define TCR_EL1_TG0_4K                                  UL(0b00)
#define TCR_EL1_TG0_64K                                 UL(0b01)
#define TCR_EL1_TG0_16K                                 UL(0b10)

#define TCR_EL1_SH0                                     GENMASK(13, 12)
#define TCR_EL1_SH0_MASK                                GENMASK(13, 12)
#define TCR_EL1_SH0_SHIFT                               12
#define TCR_EL1_SH0_WIDTH                               2
#define TCR_EL1_SH0_NONE                                UL(0b00)
#define TCR_EL1_SH0_OUTER                               UL(0b10)
#define TCR_EL1_SH0_INNER                               UL(0b11)

#define TCR_EL1_ORGN0                                   GENMASK(11, 10)
#define TCR_EL1_ORGN0_MASK                              GENMASK(11, 10)
#define TCR_EL1_ORGN0_SHIFT                             10
#define TCR_EL1_ORGN0_WIDTH                             2
#define TCR_EL1_ORGN0_NC                                UL(0b00)
#define TCR_EL1_ORGN0_WBWA                              UL(0b01)
#define TCR_EL1_ORGN0_WT                                UL(0b10)
#define TCR_EL1_ORGN0_WBnWA                             UL(0b11)

#define TCR_EL1_IRGN0                                   GENMASK(9, 8)
#define TCR_EL1_IRGN0_MASK                              GENMASK(9, 8)
#define TCR_EL1_IRGN0_SHIFT                             8
#define TCR_EL1_IRGN0_WIDTH                             2
#define TCR_EL1_IRGN0_NC                                UL(0b00)
#define TCR_EL1_IRGN0_WBWA                              UL(0b01)
#define TCR_EL1_IRGN0_WT                                UL(0b10)
#define TCR_EL1_IRGN0_WBnWA                             UL(0b11)

#define TCR_EL1_EPD0                                    GENMASK(7, 7)
#define TCR_EL1_EPD0_MASK                               GENMASK(7, 7)
#define TCR_EL1_EPD0_SHIFT                              7
#define TCR_EL1_EPD0_WIDTH                              1

#define TCR_EL1_T0SZ                                    GENMASK(5, 0)
#define TCR_EL1_T0SZ_MASK                               GENMASK(5, 0)
#define TCR_EL1_T0SZ_SHIFT                              0
#define TCR_EL1_T0SZ_WIDTH                              6

#define TCR_EL1_RES0                                    (UL(0) | GENMASK_ULL(63, 62) | GENMASK_ULL(35, 35) | GENMASK_ULL(6, 6))
#define TCR_EL1_RES1                                    (UL(0))
#define TCR_EL1_UNKN                                    (UL(0))

#define REG_TCR_EL12                                    S3_5_C2_C0_2
#define SYS_TCR_EL12                                    sys_reg(3, 5, 2, 0, 2)
#define SYS_TCR_EL12_Op0                                3
#define SYS_TCR_EL12_Op1                                5
#define SYS_TCR_EL12_CRn                                2
#define SYS_TCR_EL12_CRm                                0
#define SYS_TCR_EL12_Op2                                2

/* For TCR_EL12 fields see TCR_EL1 */

#define REG_TCRALIAS_EL1                                S3_0_C2_C7_6
#define SYS_TCRALIAS_EL1                                sys_reg(3, 0, 2, 7, 6)
#define SYS_TCRALIAS_EL1_Op0                            3
#define SYS_TCRALIAS_EL1_Op1                            0
#define SYS_TCRALIAS_EL1_CRn                            2
#define SYS_TCRALIAS_EL1_CRm                            7
#define SYS_TCRALIAS_EL1_Op2                            6

/* For TCRALIAS_EL1 fields see TCR_EL1 */

#define REG_TCR2_EL1                                    S3_0_C2_C0_3
#define SYS_TCR2_EL1                                    sys_reg(3, 0, 2, 0, 3)
#define SYS_TCR2_EL1_Op0                                3
#define SYS_TCR2_EL1_Op1                                0
#define SYS_TCR2_EL1_CRn                                2
#define SYS_TCR2_EL1_CRm                                0
#define SYS_TCR2_EL1_Op2                                3

#define TCR2_EL1_FNGNA1                                 GENMASK(21, 21)
#define TCR2_EL1_FNGNA1_MASK                            GENMASK(21, 21)
#define TCR2_EL1_FNGNA1_SHIFT                           21
#define TCR2_EL1_FNGNA1_WIDTH                           1

#define TCR2_EL1_FNGNA0                                 GENMASK(20, 20)
#define TCR2_EL1_FNGNA0_MASK                            GENMASK(20, 20)
#define TCR2_EL1_FNGNA0_SHIFT                           20
#define TCR2_EL1_FNGNA0_WIDTH                           1

#define TCR2_EL1_FNG1                                   GENMASK(18, 18)
#define TCR2_EL1_FNG1_MASK                              GENMASK(18, 18)
#define TCR2_EL1_FNG1_SHIFT                             18
#define TCR2_EL1_FNG1_WIDTH                             1

#define TCR2_EL1_FNG0                                   GENMASK(17, 17)
#define TCR2_EL1_FNG0_MASK                              GENMASK(17, 17)
#define TCR2_EL1_FNG0_SHIFT                             17
#define TCR2_EL1_FNG0_WIDTH                             1

#define TCR2_EL1_A2                                     GENMASK(16, 16)
#define TCR2_EL1_A2_MASK                                GENMASK(16, 16)
#define TCR2_EL1_A2_SHIFT                               16
#define TCR2_EL1_A2_WIDTH                               1

#define TCR2_EL1_DisCH1                                 GENMASK(15, 15)
#define TCR2_EL1_DisCH1_MASK                            GENMASK(15, 15)
#define TCR2_EL1_DisCH1_SHIFT                           15
#define TCR2_EL1_DisCH1_WIDTH                           1

#define TCR2_EL1_DisCH0                                 GENMASK(14, 14)
#define TCR2_EL1_DisCH0_MASK                            GENMASK(14, 14)
#define TCR2_EL1_DisCH0_SHIFT                           14
#define TCR2_EL1_DisCH0_WIDTH                           1

#define TCR2_EL1_HAFT                                   GENMASK(11, 11)
#define TCR2_EL1_HAFT_MASK                              GENMASK(11, 11)
#define TCR2_EL1_HAFT_SHIFT                             11
#define TCR2_EL1_HAFT_WIDTH                             1

#define TCR2_EL1_PTTWI                                  GENMASK(10, 10)
#define TCR2_EL1_PTTWI_MASK                             GENMASK(10, 10)
#define TCR2_EL1_PTTWI_SHIFT                            10
#define TCR2_EL1_PTTWI_WIDTH                            1

#define TCR2_EL1_D128                                   GENMASK(5, 5)
#define TCR2_EL1_D128_MASK                              GENMASK(5, 5)
#define TCR2_EL1_D128_SHIFT                             5
#define TCR2_EL1_D128_WIDTH                             1

#define TCR2_EL1_AIE                                    GENMASK(4, 4)
#define TCR2_EL1_AIE_MASK                               GENMASK(4, 4)
#define TCR2_EL1_AIE_SHIFT                              4
#define TCR2_EL1_AIE_WIDTH                              1

#define TCR2_EL1_POE                                    GENMASK(3, 3)
#define TCR2_EL1_POE_MASK                               GENMASK(3, 3)
#define TCR2_EL1_POE_SHIFT                              3
#define TCR2_EL1_POE_WIDTH                              1

#define TCR2_EL1_E0POE                                  GENMASK(2, 2)
#define TCR2_EL1_E0POE_MASK                             GENMASK(2, 2)
#define TCR2_EL1_E0POE_SHIFT                            2
#define TCR2_EL1_E0POE_WIDTH                            1

#define TCR2_EL1_PIE                                    GENMASK(1, 1)
#define TCR2_EL1_PIE_MASK                               GENMASK(1, 1)
#define TCR2_EL1_PIE_SHIFT                              1
#define TCR2_EL1_PIE_WIDTH                              1

#define TCR2_EL1_PnCH                                   GENMASK(0, 0)
#define TCR2_EL1_PnCH_MASK                              GENMASK(0, 0)
#define TCR2_EL1_PnCH_SHIFT                             0
#define TCR2_EL1_PnCH_WIDTH                             1

#define TCR2_EL1_RES0                                   (UL(0) | GENMASK_ULL(63, 22) | GENMASK_ULL(19, 19) | GENMASK_ULL(13, 12) | GENMASK_ULL(9, 6))
#define TCR2_EL1_RES1                                   (UL(0))
#define TCR2_EL1_UNKN                                   (UL(0))

#define REG_TCR2_EL12                                   S3_5_C2_C0_3
#define SYS_TCR2_EL12                                   sys_reg(3, 5, 2, 0, 3)
#define SYS_TCR2_EL12_Op0                               3
#define SYS_TCR2_EL12_Op1                               5
#define SYS_TCR2_EL12_CRn                               2
#define SYS_TCR2_EL12_CRm                               0
#define SYS_TCR2_EL12_Op2                               3

/* For TCR2_EL12 fields see TCR2_EL1 */

#define REG_TCR2ALIAS_EL1                               S3_0_C2_C7_7
#define SYS_TCR2ALIAS_EL1                               sys_reg(3, 0, 2, 7, 7)
#define SYS_TCR2ALIAS_EL1_Op0                           3
#define SYS_TCR2ALIAS_EL1_Op1                           0
#define SYS_TCR2ALIAS_EL1_CRn                           2
#define SYS_TCR2ALIAS_EL1_CRm                           7
#define SYS_TCR2ALIAS_EL1_Op2                           7

/* For TCR2ALIAS_EL1 fields see TCR2_EL1 */

#define REG_TCR2_EL2                                    S3_4_C2_C0_3
#define SYS_TCR2_EL2                                    sys_reg(3, 4, 2, 0, 3)
#define SYS_TCR2_EL2_Op0                                3
#define SYS_TCR2_EL2_Op1                                4
#define SYS_TCR2_EL2_CRn                                2
#define SYS_TCR2_EL2_CRm                                0
#define SYS_TCR2_EL2_Op2                                3

#define TCR2_EL2_FNG1                                   GENMASK(18, 18)
#define TCR2_EL2_FNG1_MASK                              GENMASK(18, 18)
#define TCR2_EL2_FNG1_SHIFT                             18
#define TCR2_EL2_FNG1_WIDTH                             1

#define TCR2_EL2_FNG0                                   GENMASK(17, 17)
#define TCR2_EL2_FNG0_MASK                              GENMASK(17, 17)
#define TCR2_EL2_FNG0_SHIFT                             17
#define TCR2_EL2_FNG0_WIDTH                             1

#define TCR2_EL2_A2                                     GENMASK(16, 16)
#define TCR2_EL2_A2_MASK                                GENMASK(16, 16)
#define TCR2_EL2_A2_SHIFT                               16
#define TCR2_EL2_A2_WIDTH                               1

#define TCR2_EL2_DisCH1                                 GENMASK(15, 15)
#define TCR2_EL2_DisCH1_MASK                            GENMASK(15, 15)
#define TCR2_EL2_DisCH1_SHIFT                           15
#define TCR2_EL2_DisCH1_WIDTH                           1

#define TCR2_EL2_DisCH0                                 GENMASK(14, 14)
#define TCR2_EL2_DisCH0_MASK                            GENMASK(14, 14)
#define TCR2_EL2_DisCH0_SHIFT                           14
#define TCR2_EL2_DisCH0_WIDTH                           1

#define TCR2_EL2_AMEC1                                  GENMASK(13, 13)
#define TCR2_EL2_AMEC1_MASK                             GENMASK(13, 13)
#define TCR2_EL2_AMEC1_SHIFT                            13
#define TCR2_EL2_AMEC1_WIDTH                            1

#define TCR2_EL2_AMEC0                                  GENMASK(12, 12)
#define TCR2_EL2_AMEC0_MASK                             GENMASK(12, 12)
#define TCR2_EL2_AMEC0_SHIFT                            12
#define TCR2_EL2_AMEC0_WIDTH                            1

#define TCR2_EL2_HAFT                                   GENMASK(11, 11)
#define TCR2_EL2_HAFT_MASK                              GENMASK(11, 11)
#define TCR2_EL2_HAFT_SHIFT                             11
#define TCR2_EL2_HAFT_WIDTH                             1

#define TCR2_EL2_PTTWI                                  GENMASK(10, 10)
#define TCR2_EL2_PTTWI_MASK                             GENMASK(10, 10)
#define TCR2_EL2_PTTWI_SHIFT                            10
#define TCR2_EL2_PTTWI_WIDTH                            1

#define TCR2_EL2_D128                                   GENMASK(5, 5)
#define TCR2_EL2_D128_MASK                              GENMASK(5, 5)
#define TCR2_EL2_D128_SHIFT                             5
#define TCR2_EL2_D128_WIDTH                             1

#define TCR2_EL2_AIE                                    GENMASK(4, 4)
#define TCR2_EL2_AIE_MASK                               GENMASK(4, 4)
#define TCR2_EL2_AIE_SHIFT                              4
#define TCR2_EL2_AIE_WIDTH                              1

#define TCR2_EL2_POE                                    GENMASK(3, 3)
#define TCR2_EL2_POE_MASK                               GENMASK(3, 3)
#define TCR2_EL2_POE_SHIFT                              3
#define TCR2_EL2_POE_WIDTH                              1

#define TCR2_EL2_E0POE                                  GENMASK(2, 2)
#define TCR2_EL2_E0POE_MASK                             GENMASK(2, 2)
#define TCR2_EL2_E0POE_SHIFT                            2
#define TCR2_EL2_E0POE_WIDTH                            1

#define TCR2_EL2_PIE                                    GENMASK(1, 1)
#define TCR2_EL2_PIE_MASK                               GENMASK(1, 1)
#define TCR2_EL2_PIE_SHIFT                              1
#define TCR2_EL2_PIE_WIDTH                              1

#define TCR2_EL2_PnCH                                   GENMASK(0, 0)
#define TCR2_EL2_PnCH_MASK                              GENMASK(0, 0)
#define TCR2_EL2_PnCH_SHIFT                             0
#define TCR2_EL2_PnCH_WIDTH                             1

#define TCR2_EL2_RES0                                   (UL(0) | GENMASK_ULL(63, 19) | GENMASK_ULL(9, 6))
#define TCR2_EL2_RES1                                   (UL(0))
#define TCR2_EL2_UNKN                                   (UL(0))

#define MAIR2_ELx_Attr7                                 GENMASK(63, 56)
#define MAIR2_ELx_Attr7_MASK                            GENMASK(63, 56)
#define MAIR2_ELx_Attr7_SHIFT                           56
#define MAIR2_ELx_Attr7_WIDTH                           8

#define MAIR2_ELx_Attr6                                 GENMASK(55, 48)
#define MAIR2_ELx_Attr6_MASK                            GENMASK(55, 48)
#define MAIR2_ELx_Attr6_SHIFT                           48
#define MAIR2_ELx_Attr6_WIDTH                           8

#define MAIR2_ELx_Attr5                                 GENMASK(47, 40)
#define MAIR2_ELx_Attr5_MASK                            GENMASK(47, 40)
#define MAIR2_ELx_Attr5_SHIFT                           40
#define MAIR2_ELx_Attr5_WIDTH                           8

#define MAIR2_ELx_Attr4                                 GENMASK(39, 32)
#define MAIR2_ELx_Attr4_MASK                            GENMASK(39, 32)
#define MAIR2_ELx_Attr4_SHIFT                           32
#define MAIR2_ELx_Attr4_WIDTH                           8

#define MAIR2_ELx_Attr3                                 GENMASK(31, 24)
#define MAIR2_ELx_Attr3_MASK                            GENMASK(31, 24)
#define MAIR2_ELx_Attr3_SHIFT                           24
#define MAIR2_ELx_Attr3_WIDTH                           8

#define MAIR2_ELx_Attr2                                 GENMASK(23, 16)
#define MAIR2_ELx_Attr2_MASK                            GENMASK(23, 16)
#define MAIR2_ELx_Attr2_SHIFT                           16
#define MAIR2_ELx_Attr2_WIDTH                           8

#define MAIR2_ELx_Attr1                                 GENMASK(15, 8)
#define MAIR2_ELx_Attr1_MASK                            GENMASK(15, 8)
#define MAIR2_ELx_Attr1_SHIFT                           8
#define MAIR2_ELx_Attr1_WIDTH                           8

#define MAIR2_ELx_Attr0                                 GENMASK(7, 0)
#define MAIR2_ELx_Attr0_MASK                            GENMASK(7, 0)
#define MAIR2_ELx_Attr0_SHIFT                           0
#define MAIR2_ELx_Attr0_WIDTH                           8

#define MAIR2_ELx_RES0                                  (UL(0))
#define MAIR2_ELx_RES1                                  (UL(0))
#define MAIR2_ELx_UNKN                                  (UL(0))

#define REG_MAIR2_EL1                                   S3_0_C10_C2_1
#define SYS_MAIR2_EL1                                   sys_reg(3, 0, 10, 2, 1)
#define SYS_MAIR2_EL1_Op0                               3
#define SYS_MAIR2_EL1_Op1                               0
#define SYS_MAIR2_EL1_CRn                               10
#define SYS_MAIR2_EL1_CRm                               2
#define SYS_MAIR2_EL1_Op2                               1

/* For MAIR2_EL1 fields see MAIR2_ELx */

#define REG_MAIR2_EL2                                   S3_4_C10_C1_1
#define SYS_MAIR2_EL2                                   sys_reg(3, 4, 10, 1, 1)
#define SYS_MAIR2_EL2_Op0                               3
#define SYS_MAIR2_EL2_Op1                               4
#define SYS_MAIR2_EL2_CRn                               10
#define SYS_MAIR2_EL2_CRm                               1
#define SYS_MAIR2_EL2_Op2                               1

/* For MAIR2_EL2 fields see MAIR2_ELx */

#define REG_AMAIR2_EL1                                  S3_0_C10_C3_1
#define SYS_AMAIR2_EL1                                  sys_reg(3, 0, 10, 3, 1)
#define SYS_AMAIR2_EL1_Op0                              3
#define SYS_AMAIR2_EL1_Op1                              0
#define SYS_AMAIR2_EL1_CRn                              10
#define SYS_AMAIR2_EL1_CRm                              3
#define SYS_AMAIR2_EL1_Op2                              1

#define AMAIR2_EL1_ImpDef                               GENMASK(63, 0)
#define AMAIR2_EL1_ImpDef_MASK                          GENMASK(63, 0)
#define AMAIR2_EL1_ImpDef_SHIFT                         0
#define AMAIR2_EL1_ImpDef_WIDTH                         64

#define AMAIR2_EL1_RES0                                 (UL(0))
#define AMAIR2_EL1_RES1                                 (UL(0))
#define AMAIR2_EL1_UNKN                                 (UL(0))

#define REG_AMAIR2_EL2                                  S3_4_C10_C3_1
#define SYS_AMAIR2_EL2                                  sys_reg(3, 4, 10, 3, 1)
#define SYS_AMAIR2_EL2_Op0                              3
#define SYS_AMAIR2_EL2_Op1                              4
#define SYS_AMAIR2_EL2_CRn                              10
#define SYS_AMAIR2_EL2_CRm                              3
#define SYS_AMAIR2_EL2_Op2                              1

#define AMAIR2_EL2_ImpDef                               GENMASK(63, 0)
#define AMAIR2_EL2_ImpDef_MASK                          GENMASK(63, 0)
#define AMAIR2_EL2_ImpDef_SHIFT                         0
#define AMAIR2_EL2_ImpDef_WIDTH                         64

#define AMAIR2_EL2_RES0                                 (UL(0))
#define AMAIR2_EL2_RES1                                 (UL(0))
#define AMAIR2_EL2_UNKN                                 (UL(0))

#define PIRx_ELx_Perm15                                 GENMASK(63, 60)
#define PIRx_ELx_Perm15_MASK                            GENMASK(63, 60)
#define PIRx_ELx_Perm15_SHIFT                           60
#define PIRx_ELx_Perm15_WIDTH                           4

#define PIRx_ELx_Perm14                                 GENMASK(59, 56)
#define PIRx_ELx_Perm14_MASK                            GENMASK(59, 56)
#define PIRx_ELx_Perm14_SHIFT                           56
#define PIRx_ELx_Perm14_WIDTH                           4

#define PIRx_ELx_Perm13                                 GENMASK(55, 52)
#define PIRx_ELx_Perm13_MASK                            GENMASK(55, 52)
#define PIRx_ELx_Perm13_SHIFT                           52
#define PIRx_ELx_Perm13_WIDTH                           4

#define PIRx_ELx_Perm12                                 GENMASK(51, 48)
#define PIRx_ELx_Perm12_MASK                            GENMASK(51, 48)
#define PIRx_ELx_Perm12_SHIFT                           48
#define PIRx_ELx_Perm12_WIDTH                           4

#define PIRx_ELx_Perm11                                 GENMASK(47, 44)
#define PIRx_ELx_Perm11_MASK                            GENMASK(47, 44)
#define PIRx_ELx_Perm11_SHIFT                           44
#define PIRx_ELx_Perm11_WIDTH                           4

#define PIRx_ELx_Perm10                                 GENMASK(43, 40)
#define PIRx_ELx_Perm10_MASK                            GENMASK(43, 40)
#define PIRx_ELx_Perm10_SHIFT                           40
#define PIRx_ELx_Perm10_WIDTH                           4

#define PIRx_ELx_Perm9                                  GENMASK(39, 36)
#define PIRx_ELx_Perm9_MASK                             GENMASK(39, 36)
#define PIRx_ELx_Perm9_SHIFT                            36
#define PIRx_ELx_Perm9_WIDTH                            4

#define PIRx_ELx_Perm8                                  GENMASK(35, 32)
#define PIRx_ELx_Perm8_MASK                             GENMASK(35, 32)
#define PIRx_ELx_Perm8_SHIFT                            32
#define PIRx_ELx_Perm8_WIDTH                            4

#define PIRx_ELx_Perm7                                  GENMASK(31, 28)
#define PIRx_ELx_Perm7_MASK                             GENMASK(31, 28)
#define PIRx_ELx_Perm7_SHIFT                            28
#define PIRx_ELx_Perm7_WIDTH                            4

#define PIRx_ELx_Perm6                                  GENMASK(27, 24)
#define PIRx_ELx_Perm6_MASK                             GENMASK(27, 24)
#define PIRx_ELx_Perm6_SHIFT                            24
#define PIRx_ELx_Perm6_WIDTH                            4

#define PIRx_ELx_Perm5                                  GENMASK(23, 20)
#define PIRx_ELx_Perm5_MASK                             GENMASK(23, 20)
#define PIRx_ELx_Perm5_SHIFT                            20
#define PIRx_ELx_Perm5_WIDTH                            4

#define PIRx_ELx_Perm4                                  GENMASK(19, 16)
#define PIRx_ELx_Perm4_MASK                             GENMASK(19, 16)
#define PIRx_ELx_Perm4_SHIFT                            16
#define PIRx_ELx_Perm4_WIDTH                            4

#define PIRx_ELx_Perm3                                  GENMASK(15, 12)
#define PIRx_ELx_Perm3_MASK                             GENMASK(15, 12)
#define PIRx_ELx_Perm3_SHIFT                            12
#define PIRx_ELx_Perm3_WIDTH                            4

#define PIRx_ELx_Perm2                                  GENMASK(11, 8)
#define PIRx_ELx_Perm2_MASK                             GENMASK(11, 8)
#define PIRx_ELx_Perm2_SHIFT                            8
#define PIRx_ELx_Perm2_WIDTH                            4

#define PIRx_ELx_Perm1                                  GENMASK(7, 4)
#define PIRx_ELx_Perm1_MASK                             GENMASK(7, 4)
#define PIRx_ELx_Perm1_SHIFT                            4
#define PIRx_ELx_Perm1_WIDTH                            4

#define PIRx_ELx_Perm0                                  GENMASK(3, 0)
#define PIRx_ELx_Perm0_MASK                             GENMASK(3, 0)
#define PIRx_ELx_Perm0_SHIFT                            0
#define PIRx_ELx_Perm0_WIDTH                            4

#define PIRx_ELx_RES0                                   (UL(0))
#define PIRx_ELx_RES1                                   (UL(0))
#define PIRx_ELx_UNKN                                   (UL(0))

#define REG_PIRE0_EL1                                   S3_0_C10_C2_2
#define SYS_PIRE0_EL1                                   sys_reg(3, 0, 10, 2, 2)
#define SYS_PIRE0_EL1_Op0                               3
#define SYS_PIRE0_EL1_Op1                               0
#define SYS_PIRE0_EL1_CRn                               10
#define SYS_PIRE0_EL1_CRm                               2
#define SYS_PIRE0_EL1_Op2                               2

/* For PIRE0_EL1 fields see PIRx_ELx */

#define REG_PIRE0_EL12                                  S3_5_C10_C2_2
#define SYS_PIRE0_EL12                                  sys_reg(3, 5, 10, 2, 2)
#define SYS_PIRE0_EL12_Op0                              3
#define SYS_PIRE0_EL12_Op1                              5
#define SYS_PIRE0_EL12_CRn                              10
#define SYS_PIRE0_EL12_CRm                              2
#define SYS_PIRE0_EL12_Op2                              2

/* For PIRE0_EL12 fields see PIRE0_EL1 */

#define REG_PIRE0_EL2                                   S3_4_C10_C2_2
#define SYS_PIRE0_EL2                                   sys_reg(3, 4, 10, 2, 2)
#define SYS_PIRE0_EL2_Op0                               3
#define SYS_PIRE0_EL2_Op1                               4
#define SYS_PIRE0_EL2_CRn                               10
#define SYS_PIRE0_EL2_CRm                               2
#define SYS_PIRE0_EL2_Op2                               2

/* For PIRE0_EL2 fields see PIRx_ELx */

#define REG_PIR_EL1                                     S3_0_C10_C2_3
#define SYS_PIR_EL1                                     sys_reg(3, 0, 10, 2, 3)
#define SYS_PIR_EL1_Op0                                 3
#define SYS_PIR_EL1_Op1                                 0
#define SYS_PIR_EL1_CRn                                 10
#define SYS_PIR_EL1_CRm                                 2
#define SYS_PIR_EL1_Op2                                 3

/* For PIR_EL1 fields see PIRx_ELx */

#define REG_PIR_EL12                                    S3_5_C10_C2_3
#define SYS_PIR_EL12                                    sys_reg(3, 5, 10, 2, 3)
#define SYS_PIR_EL12_Op0                                3
#define SYS_PIR_EL12_Op1                                5
#define SYS_PIR_EL12_CRn                                10
#define SYS_PIR_EL12_CRm                                2
#define SYS_PIR_EL12_Op2                                3

/* For PIR_EL12 fields see PIR_EL1 */

#define REG_PIR_EL2                                     S3_4_C10_C2_3
#define SYS_PIR_EL2                                     sys_reg(3, 4, 10, 2, 3)
#define SYS_PIR_EL2_Op0                                 3
#define SYS_PIR_EL2_Op1                                 4
#define SYS_PIR_EL2_CRn                                 10
#define SYS_PIR_EL2_CRm                                 2
#define SYS_PIR_EL2_Op2                                 3

/* For PIR_EL2 fields see PIRx_ELx */

#define REG_POR_EL0                                     S3_3_C10_C2_4
#define SYS_POR_EL0                                     sys_reg(3, 3, 10, 2, 4)
#define SYS_POR_EL0_Op0                                 3
#define SYS_POR_EL0_Op1                                 3
#define SYS_POR_EL0_CRn                                 10
#define SYS_POR_EL0_CRm                                 2
#define SYS_POR_EL0_Op2                                 4

/* For POR_EL0 fields see PIRx_ELx */

#define REG_POR_EL1                                     S3_0_C10_C2_4
#define SYS_POR_EL1                                     sys_reg(3, 0, 10, 2, 4)
#define SYS_POR_EL1_Op0                                 3
#define SYS_POR_EL1_Op1                                 0
#define SYS_POR_EL1_CRn                                 10
#define SYS_POR_EL1_CRm                                 2
#define SYS_POR_EL1_Op2                                 4

/* For POR_EL1 fields see PIRx_ELx */

#define REG_POR_EL2                                     S3_4_C10_C2_4
#define SYS_POR_EL2                                     sys_reg(3, 4, 10, 2, 4)
#define SYS_POR_EL2_Op0                                 3
#define SYS_POR_EL2_Op1                                 4
#define SYS_POR_EL2_CRn                                 10
#define SYS_POR_EL2_CRm                                 2
#define SYS_POR_EL2_Op2                                 4

/* For POR_EL2 fields see PIRx_ELx */

#define REG_POR_EL12                                    S3_5_C10_C2_4
#define SYS_POR_EL12                                    sys_reg(3, 5, 10, 2, 4)
#define SYS_POR_EL12_Op0                                3
#define SYS_POR_EL12_Op1                                5
#define SYS_POR_EL12_CRn                                10
#define SYS_POR_EL12_CRm                                2
#define SYS_POR_EL12_Op2                                4

/* For POR_EL12 fields see POR_EL1 */

#define REG_S2POR_EL1                                   S3_0_C10_C2_5
#define SYS_S2POR_EL1                                   sys_reg(3, 0, 10, 2, 5)
#define SYS_S2POR_EL1_Op0                               3
#define SYS_S2POR_EL1_Op1                               0
#define SYS_S2POR_EL1_CRn                               10
#define SYS_S2POR_EL1_CRm                               2
#define SYS_S2POR_EL1_Op2                               5

/* For S2POR_EL1 fields see PIRx_ELx */

#define REG_S2PIR_EL2                                   S3_4_C10_C2_5
#define SYS_S2PIR_EL2                                   sys_reg(3, 4, 10, 2, 5)
#define SYS_S2PIR_EL2_Op0                               3
#define SYS_S2PIR_EL2_Op1                               4
#define SYS_S2PIR_EL2_CRn                               10
#define SYS_S2PIR_EL2_CRm                               2
#define SYS_S2PIR_EL2_Op2                               5

/* For S2PIR_EL2 fields see PIRx_ELx */

#define REG_LORSA_EL1                                   S3_0_C10_C4_0
#define SYS_LORSA_EL1                                   sys_reg(3, 0, 10, 4, 0)
#define SYS_LORSA_EL1_Op0                               3
#define SYS_LORSA_EL1_Op1                               0
#define SYS_LORSA_EL1_CRn                               10
#define SYS_LORSA_EL1_CRm                               4
#define SYS_LORSA_EL1_Op2                               0

#define LORSA_EL1_SA                                    GENMASK(51, 16)
#define LORSA_EL1_SA_MASK                               GENMASK(51, 16)
#define LORSA_EL1_SA_SHIFT                              16
#define LORSA_EL1_SA_WIDTH                              36

#define LORSA_EL1_Valid                                 GENMASK(0, 0)
#define LORSA_EL1_Valid_MASK                            GENMASK(0, 0)
#define LORSA_EL1_Valid_SHIFT                           0
#define LORSA_EL1_Valid_WIDTH                           1

#define LORSA_EL1_RES0                                  (UL(0) | GENMASK_ULL(63, 52) | GENMASK_ULL(15, 1))
#define LORSA_EL1_RES1                                  (UL(0))
#define LORSA_EL1_UNKN                                  (UL(0))

#define REG_LOREA_EL1                                   S3_0_C10_C4_1
#define SYS_LOREA_EL1                                   sys_reg(3, 0, 10, 4, 1)
#define SYS_LOREA_EL1_Op0                               3
#define SYS_LOREA_EL1_Op1                               0
#define SYS_LOREA_EL1_CRn                               10
#define SYS_LOREA_EL1_CRm                               4
#define SYS_LOREA_EL1_Op2                               1

#define LOREA_EL1_EA_51_48                              GENMASK(51, 48)
#define LOREA_EL1_EA_51_48_MASK                         GENMASK(51, 48)
#define LOREA_EL1_EA_51_48_SHIFT                        48
#define LOREA_EL1_EA_51_48_WIDTH                        4

#define LOREA_EL1_EA_47_16                              GENMASK(47, 16)
#define LOREA_EL1_EA_47_16_MASK                         GENMASK(47, 16)
#define LOREA_EL1_EA_47_16_SHIFT                        16
#define LOREA_EL1_EA_47_16_WIDTH                        32

#define LOREA_EL1_RES0                                  (UL(0) | GENMASK_ULL(63, 52) | GENMASK_ULL(15, 0))
#define LOREA_EL1_RES1                                  (UL(0))
#define LOREA_EL1_UNKN                                  (UL(0))

#define REG_LORN_EL1                                    S3_0_C10_C4_2
#define SYS_LORN_EL1                                    sys_reg(3, 0, 10, 4, 2)
#define SYS_LORN_EL1_Op0                                3
#define SYS_LORN_EL1_Op1                                0
#define SYS_LORN_EL1_CRn                                10
#define SYS_LORN_EL1_CRm                                4
#define SYS_LORN_EL1_Op2                                2

#define LORN_EL1_Num                                    GENMASK(7, 0)
#define LORN_EL1_Num_MASK                               GENMASK(7, 0)
#define LORN_EL1_Num_SHIFT                              0
#define LORN_EL1_Num_WIDTH                              8

#define LORN_EL1_RES0                                   (UL(0) | GENMASK_ULL(63, 8))
#define LORN_EL1_RES1                                   (UL(0))
#define LORN_EL1_UNKN                                   (UL(0))

#define REG_LORC_EL1                                    S3_0_C10_C4_3
#define SYS_LORC_EL1                                    sys_reg(3, 0, 10, 4, 3)
#define SYS_LORC_EL1_Op0                                3
#define SYS_LORC_EL1_Op1                                0
#define SYS_LORC_EL1_CRn                                10
#define SYS_LORC_EL1_CRm                                4
#define SYS_LORC_EL1_Op2                                3

#define LORC_EL1_DS                                     GENMASK(9, 2)
#define LORC_EL1_DS_MASK                                GENMASK(9, 2)
#define LORC_EL1_DS_SHIFT                               2
#define LORC_EL1_DS_WIDTH                               8

#define LORC_EL1_EN                                     GENMASK(0, 0)
#define LORC_EL1_EN_MASK                                GENMASK(0, 0)
#define LORC_EL1_EN_SHIFT                               0
#define LORC_EL1_EN_WIDTH                               1

#define LORC_EL1_RES0                                   (UL(0) | GENMASK_ULL(63, 10) | GENMASK_ULL(1, 1))
#define LORC_EL1_RES1                                   (UL(0))
#define LORC_EL1_UNKN                                   (UL(0))

#define REG_MPAMIDR_EL1                                 S3_0_C10_C4_4
#define SYS_MPAMIDR_EL1                                 sys_reg(3, 0, 10, 4, 4)
#define SYS_MPAMIDR_EL1_Op0                             3
#define SYS_MPAMIDR_EL1_Op1                             0
#define SYS_MPAMIDR_EL1_CRn                             10
#define SYS_MPAMIDR_EL1_CRm                             4
#define SYS_MPAMIDR_EL1_Op2                             4

#define MPAMIDR_EL1_HAS_SDEFLT                          GENMASK(61, 61)
#define MPAMIDR_EL1_HAS_SDEFLT_MASK                     GENMASK(61, 61)
#define MPAMIDR_EL1_HAS_SDEFLT_SHIFT                    61
#define MPAMIDR_EL1_HAS_SDEFLT_WIDTH                    1

#define MPAMIDR_EL1_HAS_FORCE_NS                        GENMASK(60, 60)
#define MPAMIDR_EL1_HAS_FORCE_NS_MASK                   GENMASK(60, 60)
#define MPAMIDR_EL1_HAS_FORCE_NS_SHIFT                  60
#define MPAMIDR_EL1_HAS_FORCE_NS_WIDTH                  1

#define MPAMIDR_EL1_SP4                                 GENMASK(59, 59)
#define MPAMIDR_EL1_SP4_MASK                            GENMASK(59, 59)
#define MPAMIDR_EL1_SP4_SHIFT                           59
#define MPAMIDR_EL1_SP4_WIDTH                           1

#define MPAMIDR_EL1_HAS_TIDR                            GENMASK(58, 58)
#define MPAMIDR_EL1_HAS_TIDR_MASK                       GENMASK(58, 58)
#define MPAMIDR_EL1_HAS_TIDR_SHIFT                      58
#define MPAMIDR_EL1_HAS_TIDR_WIDTH                      1

#define MPAMIDR_EL1_HAS_ALTSP                           GENMASK(57, 57)
#define MPAMIDR_EL1_HAS_ALTSP_MASK                      GENMASK(57, 57)
#define MPAMIDR_EL1_HAS_ALTSP_SHIFT                     57
#define MPAMIDR_EL1_HAS_ALTSP_WIDTH                     1

#define MPAMIDR_EL1_PMG_MAX                             GENMASK(39, 32)
#define MPAMIDR_EL1_PMG_MAX_MASK                        GENMASK(39, 32)
#define MPAMIDR_EL1_PMG_MAX_SHIFT                       32
#define MPAMIDR_EL1_PMG_MAX_WIDTH                       8

#define MPAMIDR_EL1_VPMR_MAX                            GENMASK(20, 18)
#define MPAMIDR_EL1_VPMR_MAX_MASK                       GENMASK(20, 18)
#define MPAMIDR_EL1_VPMR_MAX_SHIFT                      18
#define MPAMIDR_EL1_VPMR_MAX_WIDTH                      3

#define MPAMIDR_EL1_HAS_HCR                             GENMASK(17, 17)
#define MPAMIDR_EL1_HAS_HCR_MASK                        GENMASK(17, 17)
#define MPAMIDR_EL1_HAS_HCR_SHIFT                       17
#define MPAMIDR_EL1_HAS_HCR_WIDTH                       1

#define MPAMIDR_EL1_PARTID_MAX                          GENMASK(15, 0)
#define MPAMIDR_EL1_PARTID_MAX_MASK                     GENMASK(15, 0)
#define MPAMIDR_EL1_PARTID_MAX_SHIFT                    0
#define MPAMIDR_EL1_PARTID_MAX_WIDTH                    16

#define MPAMIDR_EL1_RES0                                (UL(0) | GENMASK_ULL(63, 62) | GENMASK_ULL(56, 40) | GENMASK_ULL(31, 21) | GENMASK_ULL(16, 16))
#define MPAMIDR_EL1_RES1                                (UL(0))
#define MPAMIDR_EL1_UNKN                                (UL(0))

#define REG_LORID_EL1                                   S3_0_C10_C4_7
#define SYS_LORID_EL1                                   sys_reg(3, 0, 10, 4, 7)
#define SYS_LORID_EL1_Op0                               3
#define SYS_LORID_EL1_Op1                               0
#define SYS_LORID_EL1_CRn                               10
#define SYS_LORID_EL1_CRm                               4
#define SYS_LORID_EL1_Op2                               7

#define LORID_EL1_LD                                    GENMASK(23, 16)
#define LORID_EL1_LD_MASK                               GENMASK(23, 16)
#define LORID_EL1_LD_SHIFT                              16
#define LORID_EL1_LD_WIDTH                              8

#define LORID_EL1_LR                                    GENMASK(7, 0)
#define LORID_EL1_LR_MASK                               GENMASK(7, 0)
#define LORID_EL1_LR_SHIFT                              0
#define LORID_EL1_LR_WIDTH                              8

#define LORID_EL1_RES0                                  (UL(0) | GENMASK_ULL(63, 24) | GENMASK_ULL(15, 8))
#define LORID_EL1_RES1                                  (UL(0))
#define LORID_EL1_UNKN                                  (UL(0))

#define REG_MPAM1_EL1                                   S3_0_C10_C5_0
#define SYS_MPAM1_EL1                                   sys_reg(3, 0, 10, 5, 0)
#define SYS_MPAM1_EL1_Op0                               3
#define SYS_MPAM1_EL1_Op1                               0
#define SYS_MPAM1_EL1_CRn                               10
#define SYS_MPAM1_EL1_CRm                               5
#define SYS_MPAM1_EL1_Op2                               0

#define MPAM1_EL1_MPAMEN                                GENMASK(63, 63)
#define MPAM1_EL1_MPAMEN_MASK                           GENMASK(63, 63)
#define MPAM1_EL1_MPAMEN_SHIFT                          63
#define MPAM1_EL1_MPAMEN_WIDTH                          1

#define MPAM1_EL1_FORCED_NS                             GENMASK(60, 60)
#define MPAM1_EL1_FORCED_NS_MASK                        GENMASK(60, 60)
#define MPAM1_EL1_FORCED_NS_SHIFT                       60
#define MPAM1_EL1_FORCED_NS_WIDTH                       1

#define MPAM1_EL1_ALTSP_FRCD                            GENMASK(54, 54)
#define MPAM1_EL1_ALTSP_FRCD_MASK                       GENMASK(54, 54)
#define MPAM1_EL1_ALTSP_FRCD_SHIFT                      54
#define MPAM1_EL1_ALTSP_FRCD_WIDTH                      1

#define MPAM1_EL1_PMG_D                                 GENMASK(47, 40)
#define MPAM1_EL1_PMG_D_MASK                            GENMASK(47, 40)
#define MPAM1_EL1_PMG_D_SHIFT                           40
#define MPAM1_EL1_PMG_D_WIDTH                           8

#define MPAM1_EL1_PMG_I                                 GENMASK(39, 32)
#define MPAM1_EL1_PMG_I_MASK                            GENMASK(39, 32)
#define MPAM1_EL1_PMG_I_SHIFT                           32
#define MPAM1_EL1_PMG_I_WIDTH                           8

#define MPAM1_EL1_PARTID_D                              GENMASK(31, 16)
#define MPAM1_EL1_PARTID_D_MASK                         GENMASK(31, 16)
#define MPAM1_EL1_PARTID_D_SHIFT                        16
#define MPAM1_EL1_PARTID_D_WIDTH                        16

#define MPAM1_EL1_PARTID_I                              GENMASK(15, 0)
#define MPAM1_EL1_PARTID_I_MASK                         GENMASK(15, 0)
#define MPAM1_EL1_PARTID_I_SHIFT                        0
#define MPAM1_EL1_PARTID_I_WIDTH                        16

#define MPAM1_EL1_RES0                                  (UL(0) | GENMASK_ULL(62, 61) | GENMASK_ULL(59, 55) | GENMASK_ULL(53, 48))
#define MPAM1_EL1_RES1                                  (UL(0))
#define MPAM1_EL1_UNKN                                  (UL(0))

#define REG_MPAM0_EL1                                   S3_0_C10_C5_1
#define SYS_MPAM0_EL1                                   sys_reg(3, 0, 10, 5, 1)
#define SYS_MPAM0_EL1_Op0                               3
#define SYS_MPAM0_EL1_Op1                               0
#define SYS_MPAM0_EL1_CRn                               10
#define SYS_MPAM0_EL1_CRm                               5
#define SYS_MPAM0_EL1_Op2                               1

#define MPAM0_EL1_PMG_D                                 GENMASK(47, 40)
#define MPAM0_EL1_PMG_D_MASK                            GENMASK(47, 40)
#define MPAM0_EL1_PMG_D_SHIFT                           40
#define MPAM0_EL1_PMG_D_WIDTH                           8

#define MPAM0_EL1_PMG_I                                 GENMASK(39, 32)
#define MPAM0_EL1_PMG_I_MASK                            GENMASK(39, 32)
#define MPAM0_EL1_PMG_I_SHIFT                           32
#define MPAM0_EL1_PMG_I_WIDTH                           8

#define MPAM0_EL1_PARTID_D                              GENMASK(31, 16)
#define MPAM0_EL1_PARTID_D_MASK                         GENMASK(31, 16)
#define MPAM0_EL1_PARTID_D_SHIFT                        16
#define MPAM0_EL1_PARTID_D_WIDTH                        16

#define MPAM0_EL1_PARTID_I                              GENMASK(15, 0)
#define MPAM0_EL1_PARTID_I_MASK                         GENMASK(15, 0)
#define MPAM0_EL1_PARTID_I_SHIFT                        0
#define MPAM0_EL1_PARTID_I_WIDTH                        16

#define MPAM0_EL1_RES0                                  (UL(0) | GENMASK_ULL(63, 48))
#define MPAM0_EL1_RES1                                  (UL(0))
#define MPAM0_EL1_UNKN                                  (UL(0))

#define REG_ISR_EL1                                     S3_0_C12_C1_0
#define SYS_ISR_EL1                                     sys_reg(3, 0, 12, 1, 0)
#define SYS_ISR_EL1_Op0                                 3
#define SYS_ISR_EL1_Op1                                 0
#define SYS_ISR_EL1_CRn                                 12
#define SYS_ISR_EL1_CRm                                 1
#define SYS_ISR_EL1_Op2                                 0

#define ISR_EL1_IS                                      GENMASK(10, 10)
#define ISR_EL1_IS_MASK                                 GENMASK(10, 10)
#define ISR_EL1_IS_SHIFT                                10
#define ISR_EL1_IS_WIDTH                                1

#define ISR_EL1_FS                                      GENMASK(9, 9)
#define ISR_EL1_FS_MASK                                 GENMASK(9, 9)
#define ISR_EL1_FS_SHIFT                                9
#define ISR_EL1_FS_WIDTH                                1

#define ISR_EL1_A                                       GENMASK(8, 8)
#define ISR_EL1_A_MASK                                  GENMASK(8, 8)
#define ISR_EL1_A_SHIFT                                 8
#define ISR_EL1_A_WIDTH                                 1

#define ISR_EL1_I                                       GENMASK(7, 7)
#define ISR_EL1_I_MASK                                  GENMASK(7, 7)
#define ISR_EL1_I_SHIFT                                 7
#define ISR_EL1_I_WIDTH                                 1

#define ISR_EL1_F                                       GENMASK(6, 6)
#define ISR_EL1_F_MASK                                  GENMASK(6, 6)
#define ISR_EL1_F_SHIFT                                 6
#define ISR_EL1_F_WIDTH                                 1

#define ISR_EL1_RES0                                    (UL(0) | GENMASK_ULL(63, 11) | GENMASK_ULL(5, 0))
#define ISR_EL1_RES1                                    (UL(0))
#define ISR_EL1_UNKN                                    (UL(0))

#define REG_ICC_NMIAR1_EL1                              S3_0_C12_C9_5
#define SYS_ICC_NMIAR1_EL1                              sys_reg(3, 0, 12, 9, 5)
#define SYS_ICC_NMIAR1_EL1_Op0                          3
#define SYS_ICC_NMIAR1_EL1_Op1                          0
#define SYS_ICC_NMIAR1_EL1_CRn                          12
#define SYS_ICC_NMIAR1_EL1_CRm                          9
#define SYS_ICC_NMIAR1_EL1_Op2                          5

#define ICC_NMIAR1_EL1_INTID                            GENMASK(23, 0)
#define ICC_NMIAR1_EL1_INTID_MASK                       GENMASK(23, 0)
#define ICC_NMIAR1_EL1_INTID_SHIFT                      0
#define ICC_NMIAR1_EL1_INTID_WIDTH                      24

#define ICC_NMIAR1_EL1_RES0                             (UL(0) | GENMASK_ULL(63, 24))
#define ICC_NMIAR1_EL1_RES1                             (UL(0))
#define ICC_NMIAR1_EL1_UNKN                             (UL(0))

#define REG_TRBLIMITR_EL1                               S3_0_C9_C11_0
#define SYS_TRBLIMITR_EL1                               sys_reg(3, 0, 9, 11, 0)
#define SYS_TRBLIMITR_EL1_Op0                           3
#define SYS_TRBLIMITR_EL1_Op1                           0
#define SYS_TRBLIMITR_EL1_CRn                           9
#define SYS_TRBLIMITR_EL1_CRm                           11
#define SYS_TRBLIMITR_EL1_Op2                           0

#define TRBLIMITR_EL1_LIMIT                             GENMASK(63, 12)
#define TRBLIMITR_EL1_LIMIT_MASK                        GENMASK(63, 12)
#define TRBLIMITR_EL1_LIMIT_SHIFT                       12
#define TRBLIMITR_EL1_LIMIT_WIDTH                       52

#define TRBLIMITR_EL1_XE                                GENMASK(6, 6)
#define TRBLIMITR_EL1_XE_MASK                           GENMASK(6, 6)
#define TRBLIMITR_EL1_XE_SHIFT                          6
#define TRBLIMITR_EL1_XE_WIDTH                          1

#define TRBLIMITR_EL1_nVM                               GENMASK(5, 5)
#define TRBLIMITR_EL1_nVM_MASK                          GENMASK(5, 5)
#define TRBLIMITR_EL1_nVM_SHIFT                         5
#define TRBLIMITR_EL1_nVM_WIDTH                         1

#define TRBLIMITR_EL1_TM                                GENMASK(4, 3)
#define TRBLIMITR_EL1_TM_MASK                           GENMASK(4, 3)
#define TRBLIMITR_EL1_TM_SHIFT                          3
#define TRBLIMITR_EL1_TM_WIDTH                          2
#define TRBLIMITR_EL1_TM_STOP                           UL(0b00)
#define TRBLIMITR_EL1_TM_IRQ                            UL(0b01)
#define TRBLIMITR_EL1_TM_IGNR                           UL(0b11)

#define TRBLIMITR_EL1_FM                                GENMASK(2, 1)
#define TRBLIMITR_EL1_FM_MASK                           GENMASK(2, 1)
#define TRBLIMITR_EL1_FM_SHIFT                          1
#define TRBLIMITR_EL1_FM_WIDTH                          2
#define TRBLIMITR_EL1_FM_FILL                           UL(0b00)
#define TRBLIMITR_EL1_FM_WRAP                           UL(0b01)
#define TRBLIMITR_EL1_FM_CBUF                           UL(0b11)

#define TRBLIMITR_EL1_E                                 GENMASK(0, 0)
#define TRBLIMITR_EL1_E_MASK                            GENMASK(0, 0)
#define TRBLIMITR_EL1_E_SHIFT                           0
#define TRBLIMITR_EL1_E_WIDTH                           1

#define TRBLIMITR_EL1_RES0                              (UL(0) | GENMASK_ULL(11, 7))
#define TRBLIMITR_EL1_RES1                              (UL(0))
#define TRBLIMITR_EL1_UNKN                              (UL(0))

#define REG_TRBPTR_EL1                                  S3_0_C9_C11_1
#define SYS_TRBPTR_EL1                                  sys_reg(3, 0, 9, 11, 1)
#define SYS_TRBPTR_EL1_Op0                              3
#define SYS_TRBPTR_EL1_Op1                              0
#define SYS_TRBPTR_EL1_CRn                              9
#define SYS_TRBPTR_EL1_CRm                              11
#define SYS_TRBPTR_EL1_Op2                              1

#define TRBPTR_EL1_PTR                                  GENMASK(63, 0)
#define TRBPTR_EL1_PTR_MASK                             GENMASK(63, 0)
#define TRBPTR_EL1_PTR_SHIFT                            0
#define TRBPTR_EL1_PTR_WIDTH                            64

#define TRBPTR_EL1_RES0                                 (UL(0))
#define TRBPTR_EL1_RES1                                 (UL(0))
#define TRBPTR_EL1_UNKN                                 (UL(0))

#define REG_TRBBASER_EL1                                S3_0_C9_C11_2
#define SYS_TRBBASER_EL1                                sys_reg(3, 0, 9, 11, 2)
#define SYS_TRBBASER_EL1_Op0                            3
#define SYS_TRBBASER_EL1_Op1                            0
#define SYS_TRBBASER_EL1_CRn                            9
#define SYS_TRBBASER_EL1_CRm                            11
#define SYS_TRBBASER_EL1_Op2                            2

#define TRBBASER_EL1_BASE                               GENMASK(63, 12)
#define TRBBASER_EL1_BASE_MASK                          GENMASK(63, 12)
#define TRBBASER_EL1_BASE_SHIFT                         12
#define TRBBASER_EL1_BASE_WIDTH                         52

#define TRBBASER_EL1_RES0                               (UL(0) | GENMASK_ULL(11, 0))
#define TRBBASER_EL1_RES1                               (UL(0))
#define TRBBASER_EL1_UNKN                               (UL(0))

#define REG_TRBSR_EL1                                   S3_0_C9_C11_3
#define SYS_TRBSR_EL1                                   sys_reg(3, 0, 9, 11, 3)
#define SYS_TRBSR_EL1_Op0                               3
#define SYS_TRBSR_EL1_Op1                               0
#define SYS_TRBSR_EL1_CRn                               9
#define SYS_TRBSR_EL1_CRm                               11
#define SYS_TRBSR_EL1_Op2                               3

#define TRBSR_EL1_MSS2                                  GENMASK(55, 32)
#define TRBSR_EL1_MSS2_MASK                             GENMASK(55, 32)
#define TRBSR_EL1_MSS2_SHIFT                            32
#define TRBSR_EL1_MSS2_WIDTH                            24

#define TRBSR_EL1_EC                                    GENMASK(31, 26)
#define TRBSR_EL1_EC_MASK                               GENMASK(31, 26)
#define TRBSR_EL1_EC_SHIFT                              26
#define TRBSR_EL1_EC_WIDTH                              6

#define TRBSR_EL1_DAT                                   GENMASK(23, 23)
#define TRBSR_EL1_DAT_MASK                              GENMASK(23, 23)
#define TRBSR_EL1_DAT_SHIFT                             23
#define TRBSR_EL1_DAT_WIDTH                             1

#define TRBSR_EL1_IRQ                                   GENMASK(22, 22)
#define TRBSR_EL1_IRQ_MASK                              GENMASK(22, 22)
#define TRBSR_EL1_IRQ_SHIFT                             22
#define TRBSR_EL1_IRQ_WIDTH                             1

#define TRBSR_EL1_TRG                                   GENMASK(21, 21)
#define TRBSR_EL1_TRG_MASK                              GENMASK(21, 21)
#define TRBSR_EL1_TRG_SHIFT                             21
#define TRBSR_EL1_TRG_WIDTH                             1

#define TRBSR_EL1_WRAP                                  GENMASK(20, 20)
#define TRBSR_EL1_WRAP_MASK                             GENMASK(20, 20)
#define TRBSR_EL1_WRAP_SHIFT                            20
#define TRBSR_EL1_WRAP_WIDTH                            1

#define TRBSR_EL1_EA                                    GENMASK(18, 18)
#define TRBSR_EL1_EA_MASK                               GENMASK(18, 18)
#define TRBSR_EL1_EA_SHIFT                              18
#define TRBSR_EL1_EA_WIDTH                              1

#define TRBSR_EL1_S                                     GENMASK(17, 17)
#define TRBSR_EL1_S_MASK                                GENMASK(17, 17)
#define TRBSR_EL1_S_SHIFT                               17
#define TRBSR_EL1_S_WIDTH                               1

#define TRBSR_EL1_MSS                                   GENMASK(15, 0)
#define TRBSR_EL1_MSS_MASK                              GENMASK(15, 0)
#define TRBSR_EL1_MSS_SHIFT                             0
#define TRBSR_EL1_MSS_WIDTH                             16

#define TRBSR_EL1_RES0                                  (UL(0) | GENMASK_ULL(63, 56) | GENMASK_ULL(25, 24) | GENMASK_ULL(19, 19) | GENMASK_ULL(16, 16))
#define TRBSR_EL1_RES1                                  (UL(0))
#define TRBSR_EL1_UNKN                                  (UL(0))

#define REG_TRBMAR_EL1                                  S3_0_C9_C11_4
#define SYS_TRBMAR_EL1                                  sys_reg(3, 0, 9, 11, 4)
#define SYS_TRBMAR_EL1_Op0                              3
#define SYS_TRBMAR_EL1_Op1                              0
#define SYS_TRBMAR_EL1_CRn                              9
#define SYS_TRBMAR_EL1_CRm                              11
#define SYS_TRBMAR_EL1_Op2                              4

#define TRBMAR_EL1_PAS                                  GENMASK(11, 10)
#define TRBMAR_EL1_PAS_MASK                             GENMASK(11, 10)
#define TRBMAR_EL1_PAS_SHIFT                            10
#define TRBMAR_EL1_PAS_WIDTH                            2
#define TRBMAR_EL1_PAS_SECURE                           UL(0b00)
#define TRBMAR_EL1_PAS_NON_SECURE                       UL(0b01)
#define TRBMAR_EL1_PAS_ROOT                             UL(0b10)
#define TRBMAR_EL1_PAS_REALM                            UL(0b11)

#define TRBMAR_EL1_SH                                   GENMASK(9, 8)
#define TRBMAR_EL1_SH_MASK                              GENMASK(9, 8)
#define TRBMAR_EL1_SH_SHIFT                             8
#define TRBMAR_EL1_SH_WIDTH                             2
#define TRBMAR_EL1_SH_NON_SHAREABLE                     UL(0b00)
#define TRBMAR_EL1_SH_OUTER_SHAREABLE                   UL(0b10)
#define TRBMAR_EL1_SH_INNER_SHAREABLE                   UL(0b11)

#define TRBMAR_EL1_Attr                                 GENMASK(7, 0)
#define TRBMAR_EL1_Attr_MASK                            GENMASK(7, 0)
#define TRBMAR_EL1_Attr_SHIFT                           0
#define TRBMAR_EL1_Attr_WIDTH                           8

#define TRBMAR_EL1_RES0                                 (UL(0) | GENMASK_ULL(63, 12))
#define TRBMAR_EL1_RES1                                 (UL(0))
#define TRBMAR_EL1_UNKN                                 (UL(0))

#define REG_TRBTRG_EL1                                  S3_0_C9_C11_6
#define SYS_TRBTRG_EL1                                  sys_reg(3, 0, 9, 11, 6)
#define SYS_TRBTRG_EL1_Op0                              3
#define SYS_TRBTRG_EL1_Op1                              0
#define SYS_TRBTRG_EL1_CRn                              9
#define SYS_TRBTRG_EL1_CRm                              11
#define SYS_TRBTRG_EL1_Op2                              6

#define TRBTRG_EL1_TRG                                  GENMASK(31, 0)
#define TRBTRG_EL1_TRG_MASK                             GENMASK(31, 0)
#define TRBTRG_EL1_TRG_SHIFT                            0
#define TRBTRG_EL1_TRG_WIDTH                            32

#define TRBTRG_EL1_RES0                                 (UL(0) | GENMASK_ULL(63, 32))
#define TRBTRG_EL1_RES1                                 (UL(0))
#define TRBTRG_EL1_UNKN                                 (UL(0))

#define REG_TRBIDR_EL1                                  S3_0_C9_C11_7
#define SYS_TRBIDR_EL1                                  sys_reg(3, 0, 9, 11, 7)
#define SYS_TRBIDR_EL1_Op0                              3
#define SYS_TRBIDR_EL1_Op1                              0
#define SYS_TRBIDR_EL1_CRn                              9
#define SYS_TRBIDR_EL1_CRm                              11
#define SYS_TRBIDR_EL1_Op2                              7

#define TRBIDR_EL1_MPAM                                 GENMASK(15, 12)
#define TRBIDR_EL1_MPAM_MASK                            GENMASK(15, 12)
#define TRBIDR_EL1_MPAM_SHIFT                           12
#define TRBIDR_EL1_MPAM_WIDTH                           4
#define TRBIDR_EL1_MPAM_SIGNED                          false
#define TRBIDR_EL1_MPAM_NI                              UL(0b0000)
#define TRBIDR_EL1_MPAM_DEFAULT                         UL(0b0001)
#define TRBIDR_EL1_MPAM_IMP                             UL(0b0010)

#define TRBIDR_EL1_EA                                   GENMASK(11, 8)
#define TRBIDR_EL1_EA_MASK                              GENMASK(11, 8)
#define TRBIDR_EL1_EA_SHIFT                             8
#define TRBIDR_EL1_EA_WIDTH                             4
#define TRBIDR_EL1_EA_NON_DESC                          UL(0b0000)
#define TRBIDR_EL1_EA_IGNORE                            UL(0b0001)
#define TRBIDR_EL1_EA_SERROR                            UL(0b0010)

#define TRBIDR_EL1_F                                    GENMASK(5, 5)
#define TRBIDR_EL1_F_MASK                               GENMASK(5, 5)
#define TRBIDR_EL1_F_SHIFT                              5
#define TRBIDR_EL1_F_WIDTH                              1

#define TRBIDR_EL1_P                                    GENMASK(4, 4)
#define TRBIDR_EL1_P_MASK                               GENMASK(4, 4)
#define TRBIDR_EL1_P_SHIFT                              4
#define TRBIDR_EL1_P_WIDTH                              1

#define TRBIDR_EL1_Align                                GENMASK(3, 0)
#define TRBIDR_EL1_Align_MASK                           GENMASK(3, 0)
#define TRBIDR_EL1_Align_SHIFT                          0
#define TRBIDR_EL1_Align_WIDTH                          4

#define TRBIDR_EL1_RES0                                 (UL(0) | GENMASK_ULL(63, 16) | GENMASK_ULL(7, 6))
#define TRBIDR_EL1_RES1                                 (UL(0))
#define TRBIDR_EL1_UNKN                                 (UL(0))

#define REG_HPFAR_EL2                                   S3_4_C6_C0_4
#define SYS_HPFAR_EL2                                   sys_reg(3, 4, 6, 0, 4)
#define SYS_HPFAR_EL2_Op0                               3
#define SYS_HPFAR_EL2_Op1                               4
#define SYS_HPFAR_EL2_CRn                               6
#define SYS_HPFAR_EL2_CRm                               0
#define SYS_HPFAR_EL2_Op2                               4

#define HPFAR_EL2_NS                                    GENMASK(63, 63)
#define HPFAR_EL2_NS_MASK                               GENMASK(63, 63)
#define HPFAR_EL2_NS_SHIFT                              63
#define HPFAR_EL2_NS_WIDTH                              1

#define HPFAR_EL2_FIPA                                  GENMASK(47, 4)
#define HPFAR_EL2_FIPA_MASK                             GENMASK(47, 4)
#define HPFAR_EL2_FIPA_SHIFT                            4
#define HPFAR_EL2_FIPA_WIDTH                            44

#define HPFAR_EL2_RES0                                  (UL(0) | GENMASK_ULL(62, 48) | GENMASK_ULL(3, 0))
#define HPFAR_EL2_RES1                                  (UL(0))
#define HPFAR_EL2_UNKN                                  (UL(0))

#endif /* __ASM_SYSREG_DEFS_H */
