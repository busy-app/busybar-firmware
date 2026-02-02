from SCons.Action import Action
from SCons.Builder import Builder
from SCons.Util import splitext


def create_header_file_action(target, source, env):
    with open(target[0].abspath, "wt") as fout:
        fout.write("#include <lvgl.h>\r\n\r\n")

        for f in source:
            fout.write(f"extern const lv_image_dsc_t {splitext(f.name)[0]};\r\n")


def generate(env):
    env.SetDefault(
        AUDIO_CONVERTER=env.Real("${FBT_SCRIPT_DIR}/audio.py"),
        ANIM_CONVERTER=env.Real("${FBT_SCRIPT_DIR}/seq2anim.py"),
        IMAGE_CONVERTER=env.Real("${FBT_SCRIPT_DIR}/image.py"),
        SWAGGER_GENERATOR=env.Real("${FBT_SCRIPT_DIR}/swagger.py"),
        SWAGGER_DIST_DIR=env.Dir("swagger-dist"),
    )

    if not env["VERBOSE"]:
        env.SetDefault(
            AUDIOCOMSTR="\tAUDIO\t${TARGET}",
            ANIMCOMSTR="\tANIM\t${TARGET}",
            IMAGECONVCOMSTR="\tIMGCONV\t${TARGET}",
            IMAGEHEADERCOMSTR="\tIMGHDR\t${TARGET}",
            SWAGGERCOMSTR="\tSWAG\t${TARGET}",
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
            ),
            "ImageConverter": Builder(
                action=Action(
                    [
                        [
                            "${PYTHON3}",
                            "${IMAGE_CONVERTER}",
                            "--cf",
                            "AUTO",
                            "--ofmt",
                            "${IMAGE_FILE_FORMAT}",
                            "--name",
                            "${IMAGE_INTERNAL_NAME}",
                            "${SOURCE}",
                            "-o",
                            "${TARGET.dir}",
                        ],
                    ],
                    "${IMAGECONVCOMSTR}",
                ),
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
            ),
        }
    )


def exists(env):
    return True
