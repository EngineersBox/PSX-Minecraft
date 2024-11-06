#pragma once

#ifndef _PSXMC__CORE__CONSOLE_H_
#define _PSXMC__CORE__CONSOLE_H_

#include <stdlib.h>

#define CONSOLE_BUFFER_SIZE 128
#define CONSOLE_MAX_TOKENS (CONSOLE_BUFFER_SIZE >> 1)
#define CONSOLE_HISTORY_LINES 10

typedef struct Console {
    char command_buffer[CONSOLE_BUFFER_SIZE];
    // TODO: Make history a circular buffer
    /*char* history[CONSOLE_HISTORY_LINES][CONSOLE_BUFFER_SIZE];*/
} Console;

typedef int (*ConsoleCommandHandler)(const Console* console, char* tokens[CONSOLE_MAX_TOKENS]);

typedef struct ConsoleCommand {
    const char* name;
    ConsoleCommandHandler handler;
} ConsoleCommand;

extern int console_commands_size;
extern ConsoleCommand console_commands[];

// -1: No matching command, 0: Success, 1: Failure
int consoleExecv(const Console* console);

#endif // _PSXMC__CORE__CONSOLE_H_
