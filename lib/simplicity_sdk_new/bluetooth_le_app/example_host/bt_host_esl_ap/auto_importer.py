"""
Automatic dependency importer
"""

# Copyright 2026 Silicon Laboratories Inc. www.silabs.com
#
# SPDX-License-Identifier: Zlib
#
# The licensor of this software is Silicon Laboratories Inc.
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
import importlib
import subprocess
import sys
import os
from pathlib import Path
from ap_logger import getLogger
try:
    from packaging.requirements import Requirement
    def normalize_name(req): return Requirement(req).name
except ImportError:
    def normalize_name(req): return req.split("==")[0].split(">=")[0].split("<=")[0]

class AutoImporter:
    """
    AutoImporter provides automatic module import and on-demand installation
    of missing dependencies into a local virtual environment.
    """

    def __init__(self, venv_path=None):
        if venv_path is None:
            venv_path = self.get_default_venv_path()
        self.venv = Path(venv_path)

    @property
    def log(self):
        return getLogger("IMP")

    def _prepare_venv(self):
        """
        Ensure that the virtual environment exists and that its site-packages
        directory is added to sys.path. Returns the path to the venv Python.
        """
        if not self.venv.exists():
            self.log.warning("Virtual environment not found, creating it...")
            try:
                subprocess.run(
                    [sys.executable, "-m", "venv", str(self.venv)],
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.PIPE,
                    check=True,
                )
            except subprocess.CalledProcessError as e:
                err = e.stderr.decode().rstrip() if e.stderr else "Unknown error"
                raise RuntimeError(f"Failed to create virtual environment: {err}.") from e


        # Determine python executable path depending on host OS
        if sys.platform == "win32":
            python = self.venv / "Scripts" / "python.exe"
            site = self.venv / "Lib" / "site-packages"
        else:
            python = self.venv / "bin" / "python"
            site = next((self.venv / "lib").glob("python*/site-packages"))

        # Ensure site-packages is on sys.path
        site = str(site)
        if site not in sys.path:
            sys.path.insert(0, site)

        return str(python)

    def _try_import(self, module, attr=None):
        """
        Attempt to import a module and optionally an attribute.
        """
        mod = importlib.import_module(normalize_name(module))  # Strip version specifiers, if any
        return getattr(mod, attr) if attr else mod

    def get_default_venv_path(self):
        """
        Return a reliable, platform‑independent location for storing the tool's
        virtual environment. The path is chosen to avoid shared or network
        filesystems (e.g., VirtualBox sf_ mounts) where venv creation may fail
        due to missing symlink support. Uses the user's local cache directory on
        Linux/macOS and LOCALAPPDATA on Windows to ensure consistent behavior
        across hosts and guests.
        """
        if sys.platform == "win32":
            base = Path(os.getenv("LOCALAPPDATA", Path.home() / "AppData" / "Local"))
        else:
            base = Path(os.getenv("XDG_CACHE_HOME", Path.home() / ".cache"))

        return base / "autoimporter" / "venv"

    def ensure_import(self, module, attr=None):
        """
        Import a module or attribute, installing it into a local virtual environment if necessary.
        Accepts pip-style requirement specifiers.
        Returns the imported module or attribute.
        Raises ImportError or RuntimeError on failure.
        """

        # First: always try system import
        try:
            return self._try_import(module, attr)
        except ImportError:
            self.log.warning("System import of '%s' failed. Falling back to virtual environment handling.", module)

        # Prepare venv and install after unsuccessful attempt
        python = self._prepare_venv()

        try:
            subprocess.run(
                [python, "-m", "pip", "install", module],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.PIPE,
                check=True,
            )
        except subprocess.CalledProcessError as e:
            err = e.stderr.decode().rstrip() if e.stderr else "Unknown error"
            raise RuntimeError(f"Failed to install '{module}' into the virtual environment: {err}.") from e

        # Retry import - now using the virtual environment
        try:
            return self._try_import(module, attr)
        except ImportError as e:
            raise ImportError(
                f"Module '{module}' could not be imported from the virtual environment."
            ) from e
        except AttributeError as e:
            raise AttributeError(
                f"Module '{module}' has no attribute '{attr}'"
            ) from e
