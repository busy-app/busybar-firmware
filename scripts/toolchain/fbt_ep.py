#!/bin/env python3

import json
import logging
import multiprocessing
import os
import pathlib
import subprocess
import sys
from abc import ABC, abstractmethod
from dataclasses import dataclass
from enum import Enum
from typing import ClassVar

logging.basicConfig(level=logging.INFO)
logging.getLogger().handlers[0].setFormatter(
    logging.Formatter("%(levelname)s: %(message)s")
)


@dataclass
class PathConfig:
    work_dir: pathlib.Path
    repo_dir: pathlib.Path


class LayerDeployment(ABC):
    def __init__(self, layer: "FbtRepoLayer", path_config: PathConfig):
        self.layer = layer
        self.path_config = path_config
        self.path = self._build_path()

    @abstractmethod
    def ensure(self) -> None:
        pass

    @abstractmethod
    def _build_path(self) -> pathlib.Path:
        pass


@dataclass
class FbtRepoLayer:
    class SourceType(Enum):
        # Managed using external git repo. Requires "url" and "ref"
        GIT = "git"
        # Managed using git submodule in the main repo. Requires "path"
        SUBMODULE = "submodule"
        # Managed using a catalog. Requires "name" and "version"
        CATALOG = "catalog"

    name: str
    source: dict

    @staticmethod
    def from_dict(d) -> "FbtRepoLayer":
        return FbtRepoLayer(
            name=d["name"],
            source=d["source"],
        )

    def get_layer_deployment(self, path_config: PathConfig) -> LayerDeployment:
        def validate_keys(d, keys):
            for key in keys:
                if key not in d:
                    raise Exception(f"fbtng: missing '{key}' in {d}")

        if self.source["type"] == FbtRepoLayer.SourceType.GIT.value:
            validate_keys(self.source, ["url", "ref"])
            return GitLayerDeployment(self, path_config)
        if self.source["type"] == FbtRepoLayer.SourceType.SUBMODULE.value:
            validate_keys(self.source, ["path"])
            return SubmoduleLayerDeployment(self, path_config)
        if self.source["type"] == FbtRepoLayer.SourceType.CATALOG.value:
            validate_keys(self.source, ["name", "version"])
            return CatalogLayerDeployment(self, path_config)
        raise Exception(f"fbtng: unsupported source type {self.source['type']}")


class GitLayerDeployment(LayerDeployment):
    def ensure(self):
        # Check if the layer is already cloned and its version is correct
        layer_path = self.path  # TODO: remove
        if not layer_path.exists():
            logging.info(f"fbtng: fetching {self.layer.name}...")
            subprocess.run(
                ["git", "clone", self.layer.source["url"], layer_path]
            ).check_returncode()

        # Check if there are local changes - emit a warning if there are
        if subprocess.run(["git", "diff", "--quiet"], cwd=layer_path).returncode != 0:
            logging.warning(
                f"fbtng: {self.layer.name} has local changes, not updating, using as-is"
            )
            return

        # Check if the layer is at the correct version
        try:
            subprocess.run(
                ["git", "checkout", self.layer.source["ref"]], cwd=layer_path
            ).check_returncode()
        except subprocess.CalledProcessError:
            # Fetch tags and try again
            subprocess.run(
                ["git", "fetch", "--tags", "--prune"], cwd=layer_path
            ).check_returncode()
            subprocess.run(
                ["git", "checkout", self.layer.source["ref"]], cwd=layer_path
            ).check_returncode()

        # Update submodules
        subprocess.run(
            [
                "git",
                "submodule",
                "update",
                "--init",
                "--recursive",
                "--jobs",
                str(multiprocessing.cpu_count()),
            ],
            cwd=layer_path,
        ).check_returncode()

    def _build_path(self) -> pathlib.Path:
        return self.path_config.work_dir / self.layer.name


class SubmoduleLayerDeployment(LayerDeployment):
    def ensure(self):
        # Check if the layer is already cloned
        layer_path = self.path
        if not layer_path.exists():
            raise Exception(f"fbtng: expecting {self.layer.name} to be a submodule")

        # Check if there are local changes - emit a warning if there are
        if subprocess.run(["git", "diff", "--quiet"], cwd=layer_path).returncode != 0:
            logging.warning(f"fbtng: {self.layer.name} has local changes")
        # No need to update the submodule, it's managed by the main repo

    def _build_path(self) -> pathlib.Path:
        return self.path_config.repo_dir / self.layer.source["path"]


class CatalogLayerDeployment(LayerDeployment):
    def ensure(self):
        raise NotImplementedError("CatalogLayerDeployment is not implemented yet")

    def _build_path(self) -> pathlib.Path:
        return self.path_config.work_dir / self.layer.name


@dataclass
class RepoConfig:
    MIN_SUPPORTED_VERSION: ClassVar[int] = 1
    MAX_SUPPORTED_VERSION: ClassVar[int] = 1

    version: int
    components: list[FbtRepoLayer]

    @staticmethod
    def from_dict(d) -> "RepoConfig":
        config = RepoConfig(
            version=d["version"],
            components=[
                FbtRepoLayer.from_dict(component) for component in d["components"]
            ],
        )
        if (
            config.version < RepoConfig.MIN_SUPPORTED_VERSION
            or config.version > RepoConfig.MAX_SUPPORTED_VERSION
        ):
            raise Exception(
                f"fbtng: unsupported fbtng.json version {config.version}, "
                f"supported versions are {RepoConfig.MIN_SUPPORTED_VERSION} to "
                f"{RepoConfig.MAX_SUPPORTED_VERSION}"
            )
        return config


@dataclass
class FbtEnvConfig:
    no_sync: bool = False
    toolchain_path: str = ""
    verbose: bool = False
    shallow_submodules: bool = False

    @staticmethod
    def from_env(env) -> "FbtEnvConfig":
        true_values = {"1", "true", "True", "yes", "Yes"}
        return FbtEnvConfig(
            no_sync=env.get("FBT_NO_SYNC") in true_values,
            toolchain_path=env.get("FBT_TOOLCHAIN_PATH"),
            verbose=env.get("FBT_VERBOSE") in true_values,
            shallow_submodules=env.get("FBT_GIT_SUBMODULE_SHALLOW") in true_values,
        )


class FbtNG:
    DEFAULT_SCONS_ARGS = ["--warn=target-not-built"]
    WORK_DIR_NAME = ".fbt"
    CONFIG_FILE_NAME = "fbt-project.json"
    FBTNG_COMPONENT_NAME = "fbtng"

    def __init__(self, repo_dir: str, env: dict):
        self.path_config = PathConfig(
            work_dir=pathlib.Path(repo_dir) / self.WORK_DIR_NAME,
            repo_dir=pathlib.Path(repo_dir),
        )
        self.env_config = FbtEnvConfig.from_env(env)
        self._load_config()

    def _load_config(self):
        with open(self.path_config.repo_dir / self.CONFIG_FILE_NAME) as f:
            self.config = RepoConfig.from_dict(json.load(f))

        if self.env_config.verbose:
            logging.getLogger().setLevel(logging.DEBUG)
            logging.debug(f"fbtng: {self.config}")

        fbtng_component = next(
            (c for c in self.config.components if c.name == self.FBTNG_COMPONENT_NAME),
            None,
        )
        if not (fbtng_component):
            raise Exception("fbtng component not found in fbtng.json")
        self.fbtng = fbtng_component.get_layer_deployment(self.path_config)

        self.layers = [
            c.get_layer_deployment(self.path_config)
            for c in self.config.components
            if c.name != self.FBTNG_COMPONENT_NAME
        ]

    def _update_git_submodules(self, git_repo: pathlib.Path):
        if not os.path.exists(git_repo / ".git"):
            logging.error(f"fbtng: {git_repo} is not a git repository")

        git_cmd = ["git", "submodule", "update", "--init", "--recursive"]
        if self.env_config.verbose:
            git_cmd.extend(["--verbose"])
        if self.env_config.shallow_submodules:
            git_cmd.extend(["--depth", "1"])
        git_cmd.extend(["--jobs", str(multiprocessing.cpu_count())])

        subprocess.run(git_cmd, cwd=git_repo).check_returncode()

    def _ensure_repo_layers(self):
        self._update_git_submodules(self.path_config.repo_dir)
        for component in [self.fbtng, *self.layers]:
            component.ensure()

    def run(self, args: list[str]):
        self._ensure_repo_layers()

        if sys.platform == "win32":
            python = "python"
        else:
            python = "python3"

        cmdline = [python, "-m", "SCons", *self.DEFAULT_SCONS_ARGS]
        if not self.env_config.verbose:
            cmdline.append("-Q")
        cmdline.extend(["-C", self.fbtng.path])
        for layer in self.layers:
            cmdline.extend(["-Y", layer.path])
        cmdline.extend(["-Y", self.path_config.repo_dir])
        cmdline.extend(args)

        logging.debug(cmdline)
        return subprocess.run(cmdline).returncode


def main():
    # Check that git is installed
    if subprocess.run(["git", "--version"], stdout=subprocess.DEVNULL).returncode != 0:
        logging.error("git is not installed")
        return 1

    fbtng = FbtNG(os.getcwd(), os.environ)
    sys.exit(fbtng.run(sys.argv[1:]))


if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        logging.warning("Interrupted by user")
        sys.exit(1)
    except subprocess.CalledProcessError as e:
        logging.error(f"Command failed with exit code {e.returncode}")
        sys.exit(e.returncode)
    except Exception as e:
        logging.error(e)
        sys.exit(1)
