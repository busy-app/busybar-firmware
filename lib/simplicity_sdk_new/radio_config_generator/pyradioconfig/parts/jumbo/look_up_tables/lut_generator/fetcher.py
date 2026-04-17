import subprocess
from pathlib import Path

class Fetcher:
    def __init__(self, path: Path):
        self.path = path

    def _run_cmd(self, cmd):
        """Helper to run a shell command and print output."""
        result = subprocess.run(cmd, cwd=self.path, text=True, capture_output=True)
        if result.returncode != 0:
            raise RuntimeError(f"Command failed: {' '.join(cmd)}\n{result.stderr}")
        return result.stdout.strip()

    def _get_current_tag(self) -> str:
        """
        Get the current git tag pointing at HEAD.
        Returns an empty string if no tag is found.
        """
        try:
            tag = subprocess.check_output(
                ["git", "describe", "--tags", "--exact-match"],
                cwd=self.path,
                stderr=subprocess.STDOUT
            ).decode().strip()
            return tag
        except subprocess.CalledProcessError:
            return ""

    def on_tag(self, target_tag: str) -> bool:
        """
        Check if HEAD is on the given tag.
        """
        return self._get_current_tag() == target_tag

    def update_submodule(self, tag:str=None):
        """
        Fetch a submodule and checkout a tag.

        :param tag: The tag to check out.
        """

        if self.on_tag(tag):
            print(f"Submodule {self.path} is already at tag {tag}")
            return

        # Fetch all tags inside the submodule
        self._run_cmd(["git", "fetch", "--tags"])

        # Checkout the specified tag
        if tag:
            self._run_cmd(["git", "checkout", f"{tag}"])

        print(f"Submodule {self.path} is now at tag {tag}")
