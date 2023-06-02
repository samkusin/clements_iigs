#ifndef CLEM_SCC_H
#define CLEM_SCC_H

#include "clem_mmio_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CLEM_SCC_WR2_INT_VECTOR      0x2
#define CLEM_SCC_WR3_RECV_CONTROL    0x3
#define CLEM_SCC_WR4_CLOCK_DATA_RATE 0x4
#define CLEM_SCC_WR9_MASTER_INT      0x9
#define CLEM_SCC_WR11_CLOCK_MODE     0xB
#define CLEM_SCC_WR12_TIME_CONST_LO  0xC
#define CLEM_SCC_WR13_TIME_CONST_HI  0xD
#define CLEM_SCC_WR14_MISC_CONTROL   0xE
#define CLEM_SCC_WR15_INT_ENABLE     0xF

#define CLEM_SCC_RR2_INT_VECTOR     0x2
#define CLEM_SCC_RR12_TIME_CONST_LO 0xC
#define CLEM_SCC_RR13_TIME_CONST_HI 0xD
#define CLEM_SCC_RR15_INT_ENABLE    0xF

//  Receiver options WR3
#define CLEM_SCC_RX_ENABLE         0x01
#define CLEM_SCC_TX_RX_AUTO_ENABLE 0x20

//  Misc Data Format/Rate WR4
#define CLEM_SCC_PARITY_ENABLED 0x01
#define CLEM_SCC_PARITY_EVEN    0x02
#define CLEM_SCC_STOP_SYNC_MODE 0x00
#define CLEM_SCC_STOP_BIT_1     0x04
#define CLEM_SCC_STOP_BIT_1_5   0x08
#define CLEM_SCC_STOP_BIT_2     0x0c
#define CLEM_SCC_MONO_SYNC_MODE 0x00
#define CLEM_SCC_BI_SYNC_MODE   0x10
// #define CLEM_SCC_SDLC_SYNC_MODE      0x20
#define CLEM_SCC_EXT_SYNC_MODE 0x30
#define CLEM_SCC_CLOCK_X1      0x00
#define CLEM_SCC_CLOCK_X16     0x40
#define CLEM_SCC_CLOCK_X32     0x80
#define CLEM_SCC_CLOCK_X64     0xc0

//  Transmit protocol WR5
#define CLEM_SCC_TX_RTS            0x02
#define CLEM_SCC_TX_ENABLE         0x08
#define CLEM_SCC_TX_SEND_BREAK     0x10
#define CLEM_SCC_TX_BITS_5_OR_LESS 0x00
#define CLEM_SCC_TX_BITS_7         0x20
#define CLEM_SCC_TX_BITS_6         0x40
#define CLEM_SCC_TX_BITS_8         0x60
#define CLEM_SCC_TX_DTR            0x80

//  WR10
#define CLEM_SCC_SYNC_SIZE_8BIT 0x00
#define CLEM_SCC_SYNC_SIZE_6BIT 0x01

//  Clock Mode (WR11)
#define CLEM_SCC_CLK_TRxC_OUT_XTAL  0x00
#define CLEM_SCC_CLK_TRxC_OUT_XMIT  0x01
#define CLEM_SCC_CLK_TRxC_OUT_BRG   0x02
#define CLEM_SCC_CLK_TRxC_OUT_DPLL  0x03
#define CLEM_SCC_TRxC_OUT_ENABLE    0x04
#define CLEM_SCC_CLK_TX_SOURCE_RTxC 0x00
#define CLEM_SCC_CLK_TX_SOURCE_TRxC 0x08
#define CLEM_SCC_CLK_TX_SOURCE_BRG  0x10
#define CLEM_SCC_CLK_TX_SOURCE_DPLL 0x18
#define CLEM_SCC_CLK_RX_SOURCE_RTxC 0x00
#define CLEM_SCC_CLK_RX_SOURCE_TRxC 0x20
#define CLEM_SCC_CLK_RX_SOURCE_BRG  0x40
#define CLEM_SCC_CLK_RX_SOURCE_DPLL 0x60
#define CLEM_SCC_CLK_XTAL_ON        0x80

// WR14
#define CLEM_SCC_CLK_BRG_ON     0x01
#define CLEM_SCC_CLK_BRG_PCLK   0x02
#define CLEM_SCC_DTR_FUNCTION   0x04
#define CLEM_SCC_AUTO_ENABLE    0x08
#define CLEM_SCC_LOCAL_LOOPBACK 0x10

//  RR0
#define CLEM_SCC_IN_CTS 0x20

//  Port settings
void clem_scc_channel_port_lo(struct ClemensDeviceSCCChannel *channel, uint8_t flags);
void clem_scc_channel_port_hi(struct ClemensDeviceSCCChannel *channel, uint8_t flags);

/**
 * @brief
 *
 * @param scc
 */
void clem_scc_reset(struct ClemensDeviceSCC *scc);

/**
 * @brief
 *
 * @param scc
 * @param clock
 */
void clem_scc_glu_sync(struct ClemensDeviceSCC *scc, struct ClemensClock *clock);

/**
 * @brief
 *
 * @param scc
 * @param ioreg
 * @param value
 */
void clem_scc_write_switch(struct ClemensDeviceSCC *scc, struct ClemensTimeSpec *tspec,
                           uint8_t ioreg, uint8_t value);

/**
 * @brief
 *
 * @param scc
 * @param ioreg
 * @param flags
 * @return uint8_t
 */
uint8_t clem_scc_read_switch(struct ClemensDeviceSCC *scc, uint8_t ioreg, uint8_t flags);

#ifdef __cplusplus
}
#endif

#endif
