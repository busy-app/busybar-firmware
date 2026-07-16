"""Storage subcommands: read-only queries and reversible mutations on /ext.

The `storage_dir` fixture (conftest.py) hands out an empty /ext/cli_test and wipes
it afterwards, even on failure. Coverage matrix: scratchpad/cli_coverage_matrix.md.
"""

import hashlib
import re

import allure
import pytest

from utils.cli_helpers import BLOB_PAYLOAD, TEXT_PAYLOAD, read_chunks

pytestmark = pytest.mark.cli


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("Commands Check")
class TestCLIStorageReadOnly:
    """Read-only storage subcommands on /ext (no filesystem mutations)."""

    @allure.id("2044")
    @allure.title("CLI. Command Storage.")
    @pytest.mark.story_commands_check
    def test_cli_command_storage(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("storage")
        assert response.strip(), "Storage command should return storage information"
        assert len(response.strip()) > 10, "Storage output should be substantial"

    @allure.title("CLI. storage. Command info.")
    def test_storage_info(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("storage info /ext")
        for field in ("Type:", "Total:", "Free:", "Used:"):
            assert field in response, f"missing field {field}: {response!r}"

    @allure.title("CLI. storage. Command list.")
    def test_storage_list(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("storage list /ext")
        # /ext always has at least one directory ([D]) or file ([F])
        assert "[D]" in response or "[F]" in response, response

    @allure.title("CLI. storage. Command stat.")
    def test_storage_stat(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("storage stat /ext")
        assert response.strip(), "stat should return directory info"
        assert "Usage" not in response and "Error" not in response, response

    @allure.title("CLI. storage. Command tree.")
    def test_storage_tree(self, persistent_cli_connection):
        # recursive listing of a small standard dir; 'Empty' is a valid result
        response = persistent_cli_connection.execute_command("storage tree /ext/update")
        assert response.strip(), "tree should return output"
        assert "Storage error" not in response and "Usage" not in response, response

    @allure.title("CLI. storage. Command read.")
    def test_storage_read(self, persistent_cli_connection):
        # small text file that always exists on /ext
        response = persistent_cli_connection.execute_command(
            "storage read /ext/.sys_update.txt"
        )
        assert "Size:" in response, f"expected a size header: {response!r}"
        assert "Storage error" not in response, response

    @allure.title("CLI. storage. Command timestamp.")
    def test_storage_timestamp(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command(
            "storage timestamp /ext/.sys_update.txt"
        )
        assert re.search(r"Timestamp\s+\d+", response), (
            f"expected 'Timestamp <number>': {response!r}"
        )


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("Commands Check")
class TestCLIStorageMutating:
    """Reversible mutating operations on /ext (everything is cleaned up)."""

    @allure.title("CLI. storage. File round-trip (copy/md5/stat/rename/remove).")
    def test_storage_file_roundtrip(self, persistent_cli_connection, storage_dir):
        cli, d = persistent_cli_connection, storage_dir
        # self-contained source file: archive a standard directory
        tar = cli.execute_command(f"tar c {d}/seed.tar /ext/user_assets", timeout=10)
        assert "RET: 0" in tar, tar
        cp = cli.execute_command(f"storage copy {d}/seed.tar {d}/copy.bin")
        assert "error" not in cp.lower(), cp
        md5 = cli.execute_command(f"storage md5 {d}/copy.bin")
        assert re.search(r"\b[0-9a-f]{32}\b", md5), f"no md5 hash: {md5!r}"
        stat = cli.execute_command(f"storage stat {d}/copy.bin")
        assert "size" in stat.lower(), stat
        cli.execute_command(f"storage rename {d}/copy.bin {d}/renamed.bin")
        lst = cli.execute_command(f"storage list {d}")
        assert "renamed.bin" in lst, lst
        rm = cli.execute_command(f"storage remove {d}/renamed.bin")
        assert "error" not in rm.lower(), rm

    @allure.title("CLI. storage. Command extract (tar archive).")
    def test_storage_extract(self, persistent_cli_connection, storage_dir):
        cli, d = persistent_cli_connection, storage_dir
        c = cli.execute_command(f"tar c {d}/seed.tar /ext/user_assets", timeout=10)
        assert "RET: 0" in c, c
        ex = cli.execute_command(f"storage extract {d}/seed.tar {d}/out", timeout=10)
        assert "success" in ex.lower(), ex
        assert "Storage error" not in ex, ex

    @allure.title("CLI. Command tar (compress + extract).")
    def test_tar_compress_extract(self, persistent_cli_connection, storage_dir):
        cli, d = persistent_cli_connection, storage_dir
        c = cli.execute_command(f"tar c {d}/seed.tar /ext/user_assets", timeout=10)
        assert "RET: 0" in c and "success" in c.lower(), c
        x = cli.execute_command(f"tar x {d}/seed.tar {d}/out", timeout=10)
        assert "RET: 0" in x and "success" in x.lower(), x

    @allure.title("CLI. storage. Command write (interactive text).")
    def test_storage_write(self, persistent_cli_connection, storage_dir):
        cli = persistent_cli_connection
        path = f"{storage_dir}/written.txt"
        # CR only: `storage write` starts reading raw keys right after the command,
        # so the LF of a CRLF would land in the file as its first byte
        cli.tn.write(f"storage write {path}\r".encode("utf-8"))
        cli.tn.read_until(b"exit by Ctrl+C.", timeout=5)
        cli.tn.write(TEXT_PAYLOAD + b"\x03")  # ETX flushes the buffer and exits
        cli.tn.read_until(b">: ", timeout=10)

        stat = cli.execute_command(f"storage stat {path}")
        assert f"size: {len(TEXT_PAYLOAD)}b" in stat, stat
        md5 = cli.execute_command(f"storage md5 {path}")
        assert hashlib.md5(TEXT_PAYLOAD).hexdigest() in md5, f"content differs: {md5!r}"

    @allure.title("CLI. storage. Commands write_chunk + read_chunks (binary round-trip).")
    def test_storage_chunks_roundtrip(self, persistent_cli_connection, storage_dir):
        cli = persistent_cli_connection
        path = f"{storage_dir}/chunked.bin"

        # write_chunk <path> <size>: 'Ready' and then exactly <size> raw bytes
        cli.tn.write(f"storage write_chunk {path} {len(BLOB_PAYLOAD)}\r".encode("utf-8"))
        cli.tn.read_until(b"Ready\r\n", timeout=5)
        cli.tn.write(BLOB_PAYLOAD)
        cli.tn.read_until(b">: ", timeout=10)

        md5 = cli.execute_command(f"storage md5 {path}")
        assert hashlib.md5(BLOB_PAYLOAD).hexdigest() in md5, f"content differs: {md5!r}"

        data = read_chunks(cli, path, chunk_size=128)
        assert data == BLOB_PAYLOAD, "read_chunks returned different bytes than were written"

    @allure.title("CLI. Command log_dump.")
    def test_log_dump(self, persistent_cli_connection):
        cli = persistent_cli_connection
        # LOG_STORAGE_DUMP_DEFAULT_FILE_PATH in applications/services/log_storage/log_storage.h
        path = "/ext/log.txt"
        try:
            response = cli.execute_command("log_dump", timeout=15, slow_command=True)
            assert f"Log successfully saved to {path}" in response, response
            stat = cli.execute_command(f"storage stat {path}")
            assert re.search(r"size:\s*[1-9]\d*b", stat), f"empty log dump: {stat!r}"
        finally:
            cli.execute_command(f"storage remove {path}")

    @allure.title("CLI. Command storage_benchmark.")
    @pytest.mark.regression  # heavy, noisy SD benchmark (write+read every block size)
    def test_storage_benchmark(self, persistent_cli_connection):
        cli = persistent_cli_connection
        try:
            response = cli.execute_command("storage_benchmark", timeout=60, slow_command=True)
            assert "SD card is alive" in response, response
            assert re.search(r"Total files:\s+\d+", response), response
            # write+read pass for every block size, 512B .. 128KiB
            assert re.search(r"Write 512 bytes took .* speed .* KiB/s", response), response
            assert re.search(r"Read 131072 bytes took .* speed .* KiB/s", response), response
            assert "Failed to" not in response and "mismatch" not in response, response
        finally:
            # the benchmark always leaves its scratch file behind
            cli.execute_command("storage remove /ext/benchmark.bin")

    @allure.title("CLI. Command crypto_backup (create/verify cycle).")
    @pytest.mark.regression  # 25s timeouts, writes /bkp/crypto_backup.bin
    def test_crypto_backup_cycle(self, persistent_cli_connection):
        cli = persistent_cli_connection
        # `create` writes /bkp/crypto_backup.bin with O_CREATE_NEW, so a leftover
        # file from a prior run makes it fail (and permanently skip). Start clean.
        cli.execute_command("crypto_backup remove", timeout=10)
        try:
            created = cli.execute_command("crypto_backup create", timeout=25, slow_command=True)
            if "RET: 1" in created or "Error" in created:
                pytest.skip(
                    "crypto_backup unavailable on an unprovisioned device "
                    f"(enclave/OTP invalid): {created.strip()[:120]!r}"
                )
            assert "RET: 0" in created or "Progress: 100%" in created, created
            verified = cli.execute_command("crypto_backup verify", timeout=25, slow_command=True)
            assert "RET: 0" in verified, verified
        finally:
            cli.execute_command("crypto_backup remove", timeout=10)
