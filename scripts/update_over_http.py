from pathlib import Path

import requests
from flipper.app import App


class _ProgressStream:
    def __init__(
        self,
        file_obj,
        total_size,
        logger,
        report_step=1,
        chunk_size=256 * 1024,
    ):
        self._file = file_obj
        self._total_size = total_size if total_size and total_size > 0 else None
        self._logger = logger
        self._bytes_read = 0
        self._report_step = max(1, report_step)
        self._chunk_size = max(1024, chunk_size)
        self._last_reported = -self._report_step
        self._last_reported_bytes = -1
        self._report_progress(force=True)

    def __iter__(self):
        while True:
            chunk = self._file.read(self._chunk_size)
            if not chunk:
                self._report_progress(force=True)
                break
            self._bytes_read += len(chunk)
            self._report_progress()
            yield chunk

    def __len__(self):
        return self._total_size or 0

    def _report_progress(self, force=False):
        if self._total_size is None:
            if force and self._bytes_read == self._last_reported_bytes:
                return
            if force or self._bytes_read - self._last_reported_bytes >= 1024 * 1024:
                self._last_reported_bytes = self._bytes_read
                self._logger.info("Upload progress: %d bytes sent", self._bytes_read)
            return

        percent = int((self._bytes_read * 100) / self._total_size)
        if percent > 100:
            percent = 100
        if force and percent == self._last_reported:
            return
        if (
            force
            or percent - self._last_reported >= self._report_step
            or percent == 100
        ):
            self._last_reported = percent
            print(
                f"\rUpload progress: {percent}% ({self._bytes_read}/{self._total_size} bytes)",
                end="",
                flush=True,
            )

    def finish(self):
        self._report_progress(force=True)


class Main(App):
    def init(self):
        self.parser.add_argument(
            "--file",
            required=True,
            help="Path to the update bundle file (TAR)",
        )
        self.parser.add_argument(
            "--url",
            required=False,
            help="URL to send the update to (default: http://busybar.local/api/update)",
            default="http://busybar.local/api/update",
        )
        self.parser.set_defaults(func=self.main)

    def main(self):
        args = self.args

        file_path = Path(args.file)
        try:
            total_size = file_path.stat().st_size
        except OSError as exc:
            self.logger.error("Unable to access update bundle %s: %s", file_path, exc)
            raise

        self.logger.info(
            "Uploading %s (%d bytes) to %s",
            file_path,
            total_size,
            args.url,
        )

        headers = {
            "Content-Type": "application/x-www-form-urlencoded",
            "Expect": "100-continue",
            "Content-Length": str(total_size),
        }

        with file_path.open("rb") as file_obj:
            progress_stream = _ProgressStream(file_obj, total_size, self.logger)
            try:
                response = requests.post(
                    args.url,
                    data=progress_stream,
                    headers=headers,
                )
            finally:
                progress_stream.finish()

        print()  # New line after progress

        self.logger.info(
            f"Response: {response.status_code}, content: {response.content.decode()}"
        )
        return 0 if response.status_code == 200 else 1


if __name__ == "__main__":
    Main()()
