from SCons.Builder import Builder
from SCons.Errors import StopError

from ansi.color import fg

import requests
import hashlib
import os

def sha256sum(filename):
    try:
        with open(filename, "rb") as f:
            return hashlib.file_digest(f, "sha256").hexdigest()
    except OSError:
        return None


def download_data(url):
    try:
        data = requests.get(url, timeout=10)
        data.raise_for_status()
        return data
    except requests.RequestException:
        print(fg.brightyellow(f"WARNING: Unable to load data from {url}"))
        return None

def download_cacert_sha(url):
    expected_sha_raw = download_data(url)
    if expected_sha_raw != None:
        return expected_sha_raw.text.split(" ")[0]
    return None

def download_cacert(url, target_path, expected_sha):
    payload = download_data(url)
    if payload == None:
        return False
        
    new_sha = hashlib.sha256(payload.content).hexdigest()        
    if(new_sha != expected_sha):
        print(fg.brightyellow("Downloaded payload hash differs from expected"))                 
        return False
            
    with open(target_path, "wb") as f:
        f.write(payload.content)
    return True    

def update_cacert(target, source, env):
    """Update current cacert file from remote url."""

    SRC_URL = "https://curl.se/ca/cacert.pem"
    SRC_URL_SHA256 = "https://curl.se/ca/cacert.pem.sha256"

    target_path = target[0].abspath
   
    if os.path.exists(target_path):
        current_sha = sha256sum(target_path)
        expected_sha = download_cacert_sha(SRC_URL_SHA256)
        print(f"Cert file exists compare hash\ncurrent_sha: {current_sha}\nexpected_sha: {expected_sha}")

        if(expected_sha == None):
            print(fg.brightyellow(f"Failed to load new hash from {SRC_URL_SHA256}, use current {target_path} file"))    
        elif (expected_sha == current_sha):
            print(fg.green(f"File {target_path} is up to date"))
        else:
            print(f"Cacert hash different, downloading new")
            if download_cacert(SRC_URL, target_path, expected_sha):
                print(fg.green(f"ATTENTION: {target_path} has been updated from {SRC_URL}. Commit the change."))
            else:
                print(fg.brightyellow(f"Failed to download, use current {target_path} file"))    
   
    else:
        print(f"Cacert file is missing, downloading new")
        expected_sha = download_cacert_sha(SRC_URL_SHA256)
        if (expected_sha!=None) and (download_cacert(SRC_URL, target_path, expected_sha)):
            print(fg.green(f"ATTENTION: {target_path} has been updated from {SRC_URL}. Commit the change."))
        else:
            raise StopError(f"Failed to download cert and no local copy found")


def strip_pem_comments(target, source, env):
    """Remove the curl bundle header and per-cert name labels; keep only PEM blocks."""
    in_cert = False
    out_lines = []
    with open(source[0].abspath) as f:
        for line in f:
            line = line.rstrip("\r\n")
            if line == "-----BEGIN CERTIFICATE-----":
                in_cert = True
                out_lines.append(line)
            elif line == "-----END CERTIFICATE-----":
                in_cert = False
                out_lines.append(line)
                out_lines.append("")  # blank line between certs
            elif in_cert:
                out_lines.append(line)
    while out_lines and out_lines[-1] == "":
        out_lines.pop()
    with open(target[0].abspath, "w", newline='\n') as f:
        f.write("\n".join(out_lines) + "\n")


def generate(env):
    env.Append(
        BUILDERS={
            "UpdateCerts": Builder(action=update_cacert),
            "CertStripCopy": Builder(action=strip_pem_comments)
        }
    )

def exists(env):
    return True
