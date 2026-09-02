#pragma once

#include "anim_file_i.h"

#include "components/anim_file_img.h"
#include "components/anim_file_load.h"
#include "components/anim_file_seq.h"
#include "components/anim_file_start.h"
#include "components/anim_file_mask.h"

struct AnimFile {
    File* file;
    AnimFileOption options;
    AnimFileMeta meta;

    // Components should not touch other components' state directly.
    AnimFileImg img;
    AnimFileSeq seq;
    AnimFileStart start;
    AnimFileMask mask;

#ifdef ANIM_FILE_PROFILE_PERFORMANCE
    Profiler* profiler;
#endif
};
