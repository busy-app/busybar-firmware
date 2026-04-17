import os
from conan import ConanFile
from conan.tools.scm import Git
from conan.tools.files import copy, update_conandata


class ZwaveCcConfiguratorRecipe(ConanFile):
    python_requires = "silabs_package_assistant/[~1]@silabs"
    # Set the name and version of the package
    name = "zwave_cc_configurator"
    user = "silabs"
    target_version = "1.0.0"

    # Optional metadata
    license = "www.silabs.com/about-us/legal/master-software-license-agreement"
    author = "Silicon Laboratories Inc."
    description = "Provides Z-Wave cc_configurator apack"
    revision_mode = "scm_folder"

    # Dictionary to declare properties
    options = {
      "compatibleVersion": ["ANY"],
      "subPackage": [True, False],
      "releaseNotesUrl": ["ANY"],
      "packageType": ["ANY"],
      "sdkLtsTag": ["ANY"]
    }

    # Dictionary to define properties values.
    # Alternative is to set values in def configure(self) of recipe
    default_options = {
      "compatibleVersion": "ANY",
      "subPackage": False,
      "releaseNotesUrl": "",
      "packageType": "apack",
      "sdkLtsTag": ""
    }

    def set_version(self):
        silabs_package_assistant = self.python_requires[
            "silabs_package_assistant"
        ].module
        self.version = f"{self.target_version}{silabs_package_assistant.get_prerelease_version()}"
        self.output.info(f"Resolved context: {self.name}, {self.version}, {self.channel}, {self.user}")

    def requirements(self):
        pass

    def layout(self):
        # Set the source and build folders to the current directory
        self.folders.source = "."
        self.folders.build = "."

    def deploy(self):
        # Copy all files from the package folder to the deploy folder
        copy(self, "*", src=self.package_folder, dst=self.deploy_folder)

    def export(self):
        # Add commit info to the package for revision / git commit correspondence
        git = Git(self)
        scm_commit = git.get_commit(repository=False)
        self.output.info(f"Obtained commit {scm_commit}")
        update_conandata(self, {"sources": {"commit": scm_commit, "url": "ssh://git@stash.silabs.com/z-wave/zw-protocol.git"}})

    def package_id(self):
        # Completely clear all the info, resulting ``package_id`` will be the same
        self.info.clear()

    def package(self):
        # Define the files to be included in the package
        files_to_package = {
            "apack.info",
            "README.md",
            "*.jinja",
            "*.py",
            "z-wave_cc_configurator.slsdk"
        }

        # Copy the files to the package folder
        for file in files_to_package:
            copy(self, pattern=file, src=self.source_folder, dst=os.path.join(self.package_folder, "."))

    def package_info(self):
        self.runenv_info.prepend_path("PATH", self.package_folder)
