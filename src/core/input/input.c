#include "input.h"

#include <psxapi.h>

#include "../../util/inttypes.h"
#include "../../structure/cvector_utils.h"
#include "../../logging/logging.h"

Input input = (Input) {0};

void inputInit(Input *input) {
    // Init BIOS pad driver and set pad buffers (buffers are updated
    // automatically on every V-Blank)
    InitPAD(
        (u8*) &input->pad_buffer[0][0],
        PAD_SECTION_SIZE,
        (u8*) &input->pad_buffer[1][0],
        PAD_SECTION_SIZE
    );
    // Start pad
    StartPAD();
    // Don't make pad driver acknowledge V-Blank IRQ (recommended)
    ChangeClearPAD(0);
    input->pad = (PADTYPE*) &input->pad_buffer[0][0];
    input->handlers = NULL;
    cvector_init(input->handlers, 0, NULL);
    input->in_focus = NULL;
}

void inputUpdate(Input* input) {
    if (input->in_focus == NULL
        && debounceIsExpired(input->aquire_debounce, INPUT_AQUIRE_DEBOUNCE_MS)) {
        /*&& debounce(&input->aquire_debounce, INPUT_AQUIRE_DEBOUNCE_MS)) {*/
        InputHandlerVTable* handler = NULL;
        cvector_for_each_in(handler, input->handlers) {
            if (handler->input_handler(input, handler->ctx) == INPUT_HANDLER_RETAIN) {
                // If the input handler acquires focus (returning true)
                // we set it as the focused handler and return since all
                // subsequent handlers should not attempt anything
                input->in_focus = handler;
                return;
            }
        }
        return;
    }
    InputHandlerState state;
    if (input->in_focus != NULL &&
        (state = input->in_focus->input_handler(input, input->in_focus->ctx)) != INPUT_HANDLER_RETAIN) {
        // If the focused handler relinquishes focus (returning false)
        // we reset the focused handler to avoid re-attempting an invocation
        // so we can try all other handlers.
        input->in_focus = NULL;
        if (state == INPUT_HANDLER_RELINQUISH) {
            debounceResetDelay(input->aquire_debounce);
        } else {
            debounceResetNoDelay(input->aquire_debounce);
        }
    }
}
