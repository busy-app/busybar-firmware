from SCons.Builder import Builder
from SCons.Defaults import Touch


def AddFwFlashTarget(env, targetenv, **kw):
    fwflash_target = env.FwFlash(
        targetenv.File("${BUILD_DIR}/../${TARGET_HW}_${FIRMWARE_BUILD_CFG}_flash.flag"),
        targetenv["FW_RPS"],
        **kw,
    )
    env.Alias(targetenv.subst("${FIRMWARE_BUILD_CFG}_flash"), fwflash_target)
    if env["FORCE"]:
        env.AlwaysBuild(fwflash_target)
    return fwflash_target


def generate(env):
    env.SetDefault(
        SILABS_FLASH_SCRIPT=env.File("${FBT_SCRIPT_DIR}/flashrps.py").rfile(),
        DEBUG_INTERFACE_SERIAL="auto",
        FW_FLASH_EXTRA_COMMANDS="",
    )

    env.AddMethod(AddFwFlashTarget)

    env.Append(
        BUILDERS={
            "FwFlash": Builder(
                action=[
                    [
                        "${PYTHON3}",
                        "${SILABS_FLASH_SCRIPT}",
                        "-d" if env["VERBOSE"] else "",
                        "-p",
                        "${SI917_PORT}",
                        "${SOURCE}",
                        "${ARGS}",
                    ],
                    Touch("${TARGET}"),
                ]
            ),
        }
    )


def exists(env):
    return True
