import shutil
import requests
import json
import hashlib
import tarfile
import os
import sys
from io import BytesIO
from pathlib import Path
from ansi.color import fg
from SCons.Action import Action
from SCons.Builder import Builder
from SCons.Errors import StopError
from SCons.Node.FS import Dir

GITHUB_API_VERSION = "2026-03-10"

def github_api_send_request(url):
    try:
        headers = {
            "Accept": "application/vnd.github+json",
            "User-Agent": "fbt_js_downloader",
            "X-GitHub-Api-Version": GITHUB_API_VERSION
        }

        if "GH_TOKEN" in os.environ:
            print("Using GitHub token for authentication", file=sys.stderr)
            headers["Authorization"] = f"Bearer {os.environ['GH_TOKEN']}"
        
        data = requests.get(url, headers=headers, timeout=10)
        data.raise_for_status()
        return data
    except requests.RequestException as e:
        print(fg.brightyellow(f"WARNING: Unable to load data from {url} Error: {e}"))
        return e.response

def _js_app_generate_github_api_url(url:str,version:str):
    base_url = "https://github.com"

    if base_url not in url:
        print(fg.brightyellow(f"WARNING: {url} doesn't contain {base_url}"))
        return None

    release_path = "/releases/"

    if version == 'latest':
        release_path += version
    else: 
        release_path += "tags/"+version

    api_url = "https://api.github.com/repos" + url.removeprefix(base_url) + release_path
    return api_url

def _js_app_get_release_data(url:str, version:str):
    api_url = _js_app_generate_github_api_url(url, version)
    if(api_url is None): return None

    return github_api_send_request(api_url)

def _js_app_get_release_data_by_version_or_latest(url, version):
    release_data = _js_app_get_release_data(url, version)
    if release_data is not None:
        if release_data.status_code == 404:
            print(fg.brightyellow("WARNING: Trying latest version as a fallback")) 
            release_data = _js_app_get_release_data(url, 'latest')

    if (release_data is None) or (release_data.status_code != 200):
        raise StopError(f"Failed to get release data from {url}") 

    return release_data.json()

def _js_app_json_validate(keys:list, expected_keys:list):
    missing_keys = set(expected_keys) - set(keys)
    if missing_keys:
        raise StopError(f"Fields {list(missing_keys)} are missing in app.json")

def _js_app_json_read(source_path:str):
    app_json_path = Path(source_path) / "app.json"

    url = None
    version = None
    exists = app_json_path.is_file()
    if exists:
        with open(app_json_path, "r", encoding="utf-8") as f:
            data = json.load(f)

            _js_app_json_validate(data.keys(),["url", "version"])
            url = data["url"]
            version = data["version"]

    return exists, url, version        

def is_safe_path(base_dir, target_path):
    abs_base = os.path.realpath(base_dir)
    abs_target = os.path.realpath(os.path.join(abs_base, target_path)) 
    return os.path.commonpath([abs_base, abs_target]) == abs_base

def _js_app_tar_extract(target_dir, tar_content):
    target_path = os.path.realpath(target_dir.Dir("..").abspath)
    with tarfile.open(name=None, fileobj=BytesIO(tar_content)) as tar:
        secure_members = []
        for member in tar.getmembers():
            if member.issym() or member.islnk():
                raise StopError("Symlinks and hard links are not allowed")
            if not is_safe_path(target_path, member.name):
                raise StopError("Failed to extract archive")
            secure_members.append(member)

        tar.extractall(target_path, members=secure_members)

def _js_app_action(target, source, env):
    verbose = env["VERBOSE"]
    source_dir = source[0]
    target_dir = target[0].Dir("..")
    
    source_path = source_dir.abspath
    target_path = target_dir.abspath
    exists, url, version = _js_app_json_read(source_path)
    if(exists):
        release_data = _js_app_get_release_data_by_version_or_latest(url, version)
                    
        name = release_data['name']
        assets = release_data['assets'][0]  
        expected_hash = assets['digest'].split(':')[1]
        artifact_url = assets['browser_download_url']
        if verbose: print(f"Loading version: {name} Url: {artifact_url}")

        artifact = github_api_send_request(artifact_url)
        if(artifact is None) or (artifact.status_code != 200):
            raise StopError(f"Failed to download {name} from {url}") 
        
        loaded_sha256 = hashlib.sha256(artifact.content).hexdigest()        
        if verbose:
            print(f"Expect: {expected_hash}\n\rLoaded: {loaded_sha256}")

        if expected_hash == loaded_sha256:
            shutil.rmtree(target_path, ignore_errors=True)
            _js_app_tar_extract(target_dir, artifact.content)
        else:
           raise StopError(f"Release hash mismatch!\r\nExpect: {expected_hash}\n\rLoaded: {loaded_sha256}") 
    else:
        if target_dir.is_under(env.Dir("${BUILD_DIR}")):
            shutil.rmtree(target_path, ignore_errors=True)

        shutil.copytree(source_path, target_path)

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

    json_src_path = source[0].abspath
    exists, url, version = _js_app_json_read(json_src_path)
    if(exists):
        release_data = _js_app_get_release_data_by_version_or_latest(url, version) 
        version_node = env.Value(release_data['name'])
        source.append(version_node)

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
