from SCons.Action import Action
from SCons.Builder import Builder


def _rps_sign_action(target, source, env):
    """Build action: sign an RPS file using commander-cli."""
    import subprocess
    import sys

    args = [
        sys.executable,
        env.subst("${SIGN_SILABS_SCRIPT}"),
        "--keystore",
        env.subst("${SI917_SIGN_KEYSTORE}"),
        "--input",
        source[0].abspath,
        "--output",
        target[0].abspath,
    ]

    commander = env.subst("${COMMANDER_CLI}")
    if commander:
        args.extend(["--commander", commander])

    return subprocess.run(args).returncode


def generate(env):
    env.SetDefault(
        SIGN_SILABS_SCRIPT=env.Real("${FBT_SCRIPT_DIR}/sign_silabs_rps.py"),
    )

    if not env["VERBOSE"]:
        env.SetDefault(
            RPSSIGNCOMSTR="\tRPSSIGN\t${TARGET}",
        )

    env.Append(
        BUILDERS={
            "RPSSign": Builder(
                action=Action(
                    _rps_sign_action,
                    "${RPSSIGNCOMSTR}",
                ),
                suffix=".rps",
                src_suffix=".rps",
            ),
        }
    )


def exists(env):
    return True
