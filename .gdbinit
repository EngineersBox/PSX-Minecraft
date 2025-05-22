define target remote
target extended-remote host.docker.internal:3333
symbol-file ./build/PSXMC.elf
monitor reset shellhalt
load ./build/PSXMC.elf
end
