from SCons.Action import Action
from SCons.Builder import Builder


def _rps_sign_action(target, source, env):
    """Build action: sign an RPS file using commander-cli."""
    import subprocess
    import sys

    service_url = env.subst("${SI917_SIGN_SERVICE_URL}")
    service_token = env.subst("${SI917_SIGN_SERVICE_TOKEN}")
    service_profile = env.subst("${SI917_SIGN_SERVICE_PROFILE}")

    args = [
        sys.executable,
        env.subst("${SIGN_SILABS_SCRIPT}"),
        "--input",
        source[0].abspath,
        "--output",
        target[0].abspath,
    ]

    if service_url and service_token and service_profile:
        args.extend(
            [
                "--service-url",
                service_url,
                "--token",
                service_token,
                "--profile",
                service_profile,
            ]
        )
    else:
        args.extend(["--keystore", env.subst("${SI917_SIGN_KEYSTORE}")])

    commander = env.subst("${COMMANDER_CLI}")
    if commander and not (service_url and service_token and service_profile):
        args.extend(["--commander", commander])

    return subprocess.run(args).returncode


def generate(env):
    env.SetDefault(
        SIGN_SILABS_SCRIPT=env.Real("${FBT_SCRIPT_DIR}/sign_silabs_rps.py"),
        SI917_SIGN_SERVICE_URL="",
        SI917_SIGN_SERVICE_TOKEN="",
        SI917_SIGN_SERVICE_PROFILE="",
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
