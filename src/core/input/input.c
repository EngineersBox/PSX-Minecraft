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
    if (input->in_focus == NULL) {
        InputHandlerVTable* handler = NULL;
        cvector_for_each_in(handler, input->handlers) {
            if (handler->input_handler(input, handler->ctx)) {
                // If the input handler acquires focus (returning true)
                // we set it as the focused handler and return since all
                // subsequent handlers should not attempt anything
                input->in_focus = handler;
                return;
            }
        }
        return;
    }
    if (!input->in_focus->input_handler(input, input->in_focus->ctx)) {
        // If the focused handler relinquishes focus (returning false)
        // we reset the focused handler to avoid re-attempting an invocation
        // so we can try all other handlers.
        input->in_focus = NULL;
        // FIXME: Once we close an inventory and release the input handler,
        //        it is possible to open the player inventory immediately
        //        after if we don't press fast enough. As such we should
        //        debounce when we release a handler before allowing a new
        //        handler to be aquired.
    }
}
