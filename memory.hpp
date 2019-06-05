#pragma once

#include <cstdint>

enum
{
    MR_KBSR = 0xFE00,  /* keyboard status */
    MR_KBDR = 0xFE02,  /* keyboard data */
    MR_DSR  = 0xFE04,  /* display status */
    MR_DDR  = 0xFE06,  /* display data */
    MR_MCR  = 0xFFFE,  /* machine control register */
};

/* 65536 locations */
extern uint16_t memory[UINT16_MAX];

/* Memory Map:
 *
 *   0x0000 +----------------------+
 *          |  Trap Vector Table   |
 *   0x00FF +----------------------+
 *   0x0100 +----------------------+
 *          |Interrupt Vector Table|
 *   0x01FF +----------------------+
 *   0x0200 +----------------------+
 *          |                      |
 *          |                      |
 *          | Operating system and |
 *          =                      =
 *          |   Supervisor Stack   |
 *          |                      |
 *          |                      |
 *   0x2FFF +----------------------+
 *   0x3000 +----------------------+
 *          |                      |
 *          |                      |
 *          |                      |
 *          |    Available for     |
 *          =                      =
 *          |    user programs     |
 *          |                      |
 *          |                      |
 *          |                      |
 *   0xFDFF +----------------------+
 *   0xFE00 +----------------------+
 *          |   Device register    |
 *          |      addresses       |
 *   0xFFFF +----------------------+
 * 
 */

inline void mem_write(uint16_t address, uint16_t val) { memory[address] = val; }
inline void mem_write88(uint16_t address, uint16_t val_h, uint16_t val_l) { memory[address] = val_h << 8 | (val_l & 0xFF); }
uint16_t mem_read(uint16_t address);