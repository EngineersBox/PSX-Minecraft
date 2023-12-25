#include "input.h"

#include <psxapi.h>

void initInput(Input *input) {
    // Init BIOS pad driver and set pad buffers (buffers are updated
    // automatically on every V-Blank)
    InitPAD(
        (uint8_t *) &input->pad_buffer[0][0],
        PAD_SECTION_SIZE,
        (uint8_t *) &input->pad_buffer[1][0],
        PAD_SECTION_SIZE
    );
    // Start pad
    StartPAD();
    // Don't make pad driver acknowledge V-Blank IRQ (recommended)
    ChangeClearPAD(0);
    input->pad = (PADTYPE *) &input->pad_buffer[0][0];
}
