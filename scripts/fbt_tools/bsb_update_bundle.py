from SCons.Action import Action
from SCons.Builder import Builder


def _update_bundle_action(target, source, env):
    """Build action: invoke update_bundle.py to create an update bundle archive."""
    import subprocess
    import sys

    args = [
        sys.executable,
        env.subst("${UPDATE_BUNDLE_SCRIPT}"),
        "--target",
        env.subst("${BUNDLE_TARGET_HW}"),
        "--output-tar",
        target[0].abspath,
    ]

    for env_key, cli_arg in [
        ("BUNDLE_STAGE", "--stage"),
        ("BUNDLE_DFU", "--dfu"),
        ("BUNDLE_SIL_FW", "--sil-fw"),
        ("BUNDLE_SIL_RADIO_FW", "--sil-radio-fw"),
        ("BUNDLE_RESOURCES", "--resources"),
        ("BUNDLE_BKP_RESOURCES", "--bkp-resources"),
        ("BUNDLE_SECURITY_FLAGS", "--security-flags"),
    ]:
        val = env.get(env_key)
        if val:
            if hasattr(val, "abspath"):
                args.extend([cli_arg, val.abspath])
            else:
                resolved = env.subst(f"${{{env_key}}}")
                if resolved:
                    args.extend([cli_arg, resolved])

    return subprocess.run(args).returncode


def CompressBundle(env, target, source):
    """Create a .tgz by gzipping a .tar bundle."""
    from SCons.Script import Move

    return env.Command(
        target,
        source,
        action=[
            ["${PYTHON3}", "-m", "gzip", "--best", "${SOURCE}"],
            Move("${TARGET}", "${SOURCE}.gz"),
        ],
    )


def FlashOverHttp(env, name, source):
    """Create a flash-over-HTTP phony target."""
    return env.PhonyTarget(
        name,
        [
            [
                "${PYTHON3}",
                "${UPDATE_OVER_HTTP_SCRIPT}",
                "--file",
                "${SOURCE}",
            ]
        ],
        source=source,
    )


def generate(env):
    env.SetDefault(
        UPDATE_BUNDLE_SCRIPT=env.Real("${FBT_SCRIPT_DIR}/update_bundle.py"),
        UPDATE_OVER_HTTP_SCRIPT=env.Real("${FBT_SCRIPT_DIR}/update_over_http.py"),
    )

    if not env["VERBOSE"]:
        env.SetDefault(
            UPDATEBUNDLECOMSTR="\tBUNDLE\t${TARGET}",
        )

    env.Append(
        BUILDERS={
            "UpdateBundle": Builder(
                action=Action(
                    _update_bundle_action,
                    "${UPDATEBUNDLECOMSTR}",
                ),
                suffix=".tar",
            ),
        }
    )

    env.AddMethod(CompressBundle)
    env.AddMethod(FlashOverHttp)


def exists(env):
    return True
