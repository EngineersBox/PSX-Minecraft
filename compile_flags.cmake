# Version string rendered at the top left
target_compile_definitions(PSXMC PUBLIC -DPSXMC_VERSION_STRING="PSXMC Infdev")

# ===============
# = DEBUG FLAGS =
# ===============

# For all boolean flags: (1 = enabled, 0 = disabled)

# Global toggle of debug flags
target_compile_definitions(PSXMC PUBLIC -DPSXMC_DEBUG=1)

# ==== In-game UI overlays ====

# FPS
target_compile_definitions(PSXMC PUBLIC -DPSXMC_DEBUG_OVERLAY_FPS=1)
# Position
target_compile_definitions(PSXMC PUBLIC -DPSXMC_DEBUG_OVERLAY_POS=1)
# Direction
target_compile_definitions(PSXMC PUBLIC -DPSXMC_DEBUG_OVERLAY_DIR=1)
# Facing direction
target_compile_definitions(PSXMC PUBLIC -DPSXMC_DEBUG_OVERLAY_FACING=1)
# Memory usage
target_compile_definitions(PSXMC PUBLIC -DPSXMC_DEBUG_OVERLAY_WORLD=0)
# World data like day count, time of day and weather 
target_compile_definitions(PSXMC PUBLIC -DPSXMC_DEBUG_OVERLAY_MEM=0)
# Critical path duration tree
target_compile_definitions(PSXMC PUBLIC -DPSXMC_DEBUG_OVERLAY_DURATION_TREE=1)

# ==== Player ====

# No-clip
target_compile_definitions(PSXMC PUBLIC -DPSXMC_DEBUG_PLAYER_NOCLIP=1)

# ==== PCSX-Redux MIPS API ====

# Address sanitiser
target_compile_definitions(PSXMC PUBLIC -DPSXMC_DEBUG_PCSX_ASAN=1)
# Kernel checker
target_compile_definitions(PSXMC PUBLIC -DPSXMC_DEBUG_PCSX_KERNEL_CHECKER=0)
# Debugging utils (breakpoint, lua exec, exit, dialog box, putc)
target_compile_definitions(PSXMC PUBLIC -DPSXMC_DEBUG_PCSX_UTILS=1)
