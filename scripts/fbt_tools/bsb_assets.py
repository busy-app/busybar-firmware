from SCons.Action import Action
from SCons.Builder import Builder
from SCons.Util import splitext


def create_header_file_action(target, source, env):
    with open(target[0].abspath, "wt") as fout:
        fout.write("#include <lvgl.h>\r\n\r\n")

        for f in source:
            fout.write(f"extern const lv_image_dsc_t {splitext(f.name)[0]};\r\n")

def gzip_action(target, source, env):
    import gzip
    import shutil
    src = source[0].abspath
    dst = target[0].abspath

    with open(src, "rb") as fin:
        with gzip.open(dst, "wb", compresslevel=9) as fout:
            shutil.copyfileobj(fin, fout)

def copy_action(target, source, env):
    import shutil
    src = source[0].abspath
    dst = target[0].abspath

    shutil.copyfile(src, dst)


def _script_dep_emitter(script_var):
    def emitter(target, source, env):
        env.Depends(target, env.File(env[script_var]))
        return (target, source)

    return emitter


def generate(env):
    env.SetDefault(
        AUDIO_CONVERTER=env.Real("${FBT_SCRIPT_DIR}/audio.py"),
        ANIM_CONVERTER=env.Real("${FBT_SCRIPT_DIR}/seq2anim.py"),
        FONT_CONVERTER=env.Real("${FBT_SCRIPT_DIR}/ttf2font.py"),
        IMAGE_CONVERTER=env.Real("${FBT_SCRIPT_DIR}/image.py"),
        SWAGGER_GENERATOR=env.Real("${FBT_SCRIPT_DIR}/swagger.py"),
        OPENAPI_GENERATOR=env.Real("${FBT_SCRIPT_DIR}/openapi_merge.py"),
        SWAGGER_DIST_DIR=env.Dir("swagger-dist"),
    )

    if not env["VERBOSE"]:
        env.SetDefault(
            AUDIOCOMSTR="\tAUDIO\t${TARGET}",
            ANIMCOMSTR="\tANIM\t${TARGET}",
            FONTCOMSTR="\tFONT\t${TARGET}",
            IMAGECONVCOMSTR="\tIMGCONV\t${TARGET}",
            IMAGEHEADERCOMSTR="\tIMGHDR\t${TARGET}",
            SWAGGERCOMSTR="\tSWAG\t${TARGET}",
            GZIPCOMSTR="\tGZIP\t${TARGET}",
            COPYCOMSTR="\tCOPY\t${TARGET}",
            OPENAPICOMSTR="\tOPENAPI\t${TARGET}",
        )

    env.Append(
        BUILDERS={
            "AudioConverter": Builder(
                action=Action(
                    [
                        [
                            "${PYTHON3}",
                            "${AUDIO_CONVERTER}",
                            "${SOURCE}",
                            "${TARGET}",
                        ],
                    ],
                    "${AUDIOCOMSTR}",
                ),
                emitter=_script_dep_emitter("AUDIO_CONVERTER"),
            ),
            "AnimationConverter": Builder(
                action=Action(
                    [
                        [
                            "${PYTHON3}",
                            "${ANIM_CONVERTER}",
                            "-i",
                            "${SOURCE}",
                            "-o",
                            "${TARGET}",
                            # TODO: Read fps from meta.txt
                            "-f",
                            "60",
                        ],
                    ],
                    "${ANIMCOMSTR}",
                ),
                emitter=_script_dep_emitter("ANIM_CONVERTER"),
            ),
            "FontConverter": Builder(
                action=Action(
                    [
                        [
                            "${PYTHON3}",
                            "${FONT_CONVERTER}",
                            "${SOURCE}",
                            "${TARGET}",
                        ],
                    ],
                    "${FONTCOMSTR}",
                ),
                emitter=_script_dep_emitter("FONT_CONVERTER"),
            ),
            "ImageConverter": Builder(
                action=Action(
                    [
                        [
                            "${PYTHON3}",
                            "${IMAGE_CONVERTER}",
                            "-i",
                            "${SOURCE}",
                            "-o",
                            "${TARGET}",
                            "-f",
                            "${FORMAT}",
                        ],
                    ],
                    "${IMAGECONVCOMSTR}",
                ),
                emitter=_script_dep_emitter("IMAGE_CONVERTER"),
            ),
            "ImageHeaderGenerator": Builder(
                action=Action(
                    create_header_file_action,
                    "${IMAGEHEADERCOMSTR}",
                ),
            ),
            "SwaggerGenerator": Builder(
                action=Action(
                    [
                        [
                            "${PYTHON3}",
                            "${SWAGGER_GENERATOR}",
                            "${SOURCE}",
                            "-o",
                            "${TARGET.dir}",
                            "--dist-dir",
                            "${SWAGGER_DIST_DIR}",
                            "-q",
                        ],
                    ],
                    "${SWAGGERCOMSTR}",
                ),
                emitter=_script_dep_emitter("SWAGGER_GENERATOR"),
            ),
            "Gzip": Builder(
                action=Action(
                    gzip_action,
                    "${GZIPCOMSTR}",
                ),
            ),
            "Copy": Builder(
                action=Action(
                    copy_action,
                    "${COPYCOMSTR}",
                ),
            ),
            "OpenapiMerge": Builder(
                action=Action(
                    [
                        [
                            "${PYTHON3}",
                            "${OPENAPI_GENERATOR}",
                            "merge",
                            "${TARGET}",
                            "${SOURCES}",
                        ],
                    ],
                    "${OPENAPICOMSTR}",
                ),
                emitter=_script_dep_emitter("OPENAPI_GENERATOR"),
            ),
        }
    )


def exists(env):
    return True
