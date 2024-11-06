#include "console.h"

#include <string.h>
#include <stdio.h>

#include "../logging/logging.h"

ConsoleCommand* binarySearchCommand(char* command_str) {
    int lower = 0;
	int mid;
	int upper = console_commands_size - 1;
	int ret;
	while (lower <= upper) {
		mid = (lower + upper) / 2;
		ConsoleCommand* command = &console_commands[mid];
		ret = strcmp(command->name, command_str);
		if (ret == 0) {
			return command;
		} else if (ret > 0) {
			upper = mid - 1;
		} else {
			lower = mid + 1;
		}
	}
	return NULL;
}

int consoleExecv(const Console* console) {
    char* buf = (char*) console->command_buffer;
    if (buf[0] != '/') {
        // Doesn't begin with a slash => not a command
        return -1;
    }
    char* tokens[CONSOLE_MAX_TOKENS] = {0};
    size_t token_count = 0;
    for (char* token = strtok(buf, " ");
         token != NULL && token_count < CONSOLE_MAX_TOKENS - 1;
         token = strtok(NULL, " ")) {
        tokens[token_count++] = token;
    }
    if (token_count < 1) {
        return -1;
    }
    const ConsoleCommand* cmd = binarySearchCommand(tokens[0] + 1);
    DEBUG_LOG("[CONSOLE] Found command: %s @ %p\n", cmd->name, cmd);
    if (cmd != NULL) {
        printf("[CONSOLE] Unknown command: %s\n", tokens[0] + 1);
        return cmd->handler(console, tokens);
    }
    return -1;
}
