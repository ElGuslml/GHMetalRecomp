// ghmetallica - the shell build's stand-in for generated/default/ghmetallica_init.h (GHMETALLICA_SHELL in CMake).
//
// The shell is the executable without the recompiled game: it builds without default.xex (no codegen), starts, reads
// its settings and checks for the game files, so that packaging and startup can be tried on a machine without the
// game. It cannot run the game: with the files present it stops at the image check below.

#pragma once

#include <rex/image_info.h>
#include <rex/ppc/func.h>

extern const rex::PPCImageInfo PPCImageConfig;
