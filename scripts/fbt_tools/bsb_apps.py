from SCons.Action import Action
from SCons.Builder import Builder
from SCons.Errors import StopError
from SCons.Node.FS import Dir, File
from shutil import copytree, rmtree


def _js_app_action(target, source, env):
    source_dir = source[0]
    target_dir = target[0].Dir("..")

    source_path = source_dir.abspath
    target_path = target_dir.abspath

    rmtree(target_path)
    copytree(source_path, target_path)


def _js_app_emitter(target, source, env):
    assert len(target) == len(source)

    if not isinstance(source[0], Dir):
        raise StopError("Application source path must be a directory")

    if not isinstance(target[0], Dir):
        raise StopError("Application target path must be a directory")

    source_dir = source[0]
    target_dir = target[0]

    # TODO: Fix wrong directory expansion
    source = [env.Dir("${PROJECT_ROOT}").Dir(source_dir.relpath)]
    source += env.GlobRecursive("*", source_dir)

    target = [target_dir.File("appmeta/manifest.json")]

    return (target, source)


def generate(env):
    if not env["VERBOSE"]:
        env.SetDefault(
            JSAPPCOMSTR="\tJSAPP\t${TARGET}",
        )

    env.Append(
        BUILDERS={
            "JsAppBuilder": Builder(
                action=Action(
                    _js_app_action,
                    "${JSAPPCOMSTR}",
                ),
                emitter=_js_app_emitter,
            ),
        }
    )


def exists(env):
    return True
