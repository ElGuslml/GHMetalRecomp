// ghmetallica - the shell build's image: no recompiled code (src/shell_init.h).

#include "shell_init.h"

PPCFuncMapping PPCFuncMappings[] = {{0, nullptr}};

const rex::PPCImageInfo PPCImageConfig = {
    .code_base = 0,
    .code_size = 0,
    .image_base = 0,
    .image_size = 0,
    .func_mappings = PPCFuncMappings,
};
