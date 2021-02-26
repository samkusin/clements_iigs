#ifndef CLEM_EMULATOR_H
#define CLEM_EMULATOR_H

#include "clem_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int clemens_init(
    ClemensMachine* machine,
    uint32_t speed_factor,
    uint32_t clocks_step,
    void* rom,
    size_t romSize,
    void* e0bank,
    void* e1bank,
    void* fpiRAM,
    unsigned int fpiRAMBankCount
);

void clemens_emulate(ClemensMachine* clem);

/**
 * @brief Verify the machine is initialized/ready for emulation
 *
 * The emulator does not ever allocate memory.  This method checks if the
 * machine was initialized via clemens_init with valid parameters.
 *
 * @param clem The machine
 * @return true The machine has ROM/RAM and clock settings
 * @return false
 */
bool clemens_is_initialized(ClemensMachine* clem);

/**
 * @brief Trivial validation function (one conditional)
 *
 * Better used for polling every frame vs a more comprehensive check via
 * clemens_is_initialized
 *
 * @param clem The machine
 * @return true Passes a trivial check
 * @return false Fails a trivial check
 */
bool clemens_is_initialized_simple(ClemensMachine* clem);

#ifdef __cplusplus
}
#endif

#endif
