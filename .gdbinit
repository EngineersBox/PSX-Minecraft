define target remote
target extended-remote $arg0
symbol-file ./build/PSXMC.elf
monitor reset shellhalt
load ./build/PSXMC.elf
end
