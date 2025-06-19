#!/usr/bin/env python3

import os
import json
import shutil
import zlib
import tarfile
import tempfile
from flipper.app import App

# Bundler for updater package
# Usage example:
#   python3 update_bundle.py --stage updater_stage.bin --resources /path/to/resources --sil-fw sil_fw.bin --sil-radio-fw sil_radio_fw.bin --output ./update_folder


class Main(App):
    def init(self):
        self.parser.add_argument("--stage", required=False, help="Updater stage file")
        self.parser.add_argument(
            "--resources",
            help="Path to the folder containing resource files to be included in resources.tar",
            type=str,
            default=None,
        )
        self.parser.add_argument(
            "--sil-fw", required=False, help="Updater SIL firmware file"
        )
        self.parser.add_argument(
            "--sil-radio-fw", required=False, help="Updater SIL radio firmware file"
        )
        self.parser.add_argument("--dfu", required=False, help="Updater DFU file")
        self.parser.add_argument(
            "--update-name",
            help="Optional short description of the update",
            type=str,
            default=None,
        )

        # Mutually exclusive group for output options
        output_group = self.parser.add_mutually_exclusive_group(required=True)
        output_group.add_argument(
            "--output",
            help="Output directory for update bundle (creates a folder)",
            type=str,
            default=None,
        )
        output_group.add_argument(
            "--output-tar",
            help="Output TAR archive for update bundle (creates a single .tar file)",
            type=str,
            default=None,
        )

        self.parser.add_argument(
            "--target",
            required=True,
            type=int,
            help="Hardware target (uint8) for this update bundle",
        )
        self.parser.set_defaults(func=self.main)

    def main(self):
        args = self.args
        is_tar_output = args.output_tar is not None
        temp_dir_path = None

        if is_tar_output:
            # Create a temporary directory to stage files
            temp_dir_path = tempfile.mkdtemp(prefix="update_bundle_")
            self.logger.info(
                f"Staging update bundle in temporary directory: {temp_dir_path}"
            )
            actual_output_path = temp_dir_path
        else:
            # args.output is guaranteed to be set if not args.output_tar
            actual_output_path = args.output
            os.makedirs(actual_output_path, exist_ok=True)

        try:
            manifest = {"target": args.target, "version": 1}
            if args.update_name:
                manifest["update_name"] = args.update_name

            # Copy files into actual_output_path directory if provided
            if args.stage:
                stage_basename = os.path.basename(args.stage)
                stage_dst = os.path.join(actual_output_path, stage_basename)
                shutil.copy2(args.stage, stage_dst)
                stage_crc32 = self.compute_crc32(stage_dst)
                manifest["updater_stage_crc32"] = stage_crc32
                manifest["updater_stage"] = stage_basename

            # Handle resources
            if args.resources:  # args.resources is the input folder for resources
                generated_resources_tar_filename = "resources.tar"
                # This is the path *within* the (possibly temporary) output bundle structure
                resources_bundle_tar_path = os.path.join(
                    actual_output_path, generated_resources_tar_filename
                )

                self.logger.info(
                    f"Creating TAR archive for resources from folder: {args.resources} into {resources_bundle_tar_path}"
                )
                try:
                    self.create_tar_from_folder(
                        resources_bundle_tar_path, args.resources
                    )
                    manifest["updater_resources"] = generated_resources_tar_filename
                    self.logger.info(
                        f"Successfully created {resources_bundle_tar_path}"
                    )
                except Exception as e:
                    self.logger.error(
                        f"Failed to create TAR from {args.resources}: {e}"
                    )
                    return 1

            if args.sil_fw:
                sil_fw_basename = os.path.basename(args.sil_fw)
                sil_fw_dst = os.path.join(actual_output_path, sil_fw_basename)
                shutil.copy2(args.sil_fw, sil_fw_dst)
                manifest["updater_sil_fw"] = sil_fw_basename

            if args.sil_radio_fw:
                sil_radio_fw_basename = os.path.basename(args.sil_radio_fw)
                sil_radio_fw_dst = os.path.join(
                    actual_output_path, sil_radio_fw_basename
                )
                shutil.copy2(args.sil_radio_fw, sil_radio_fw_dst)
                manifest["updater_sil_radio_fw"] = sil_radio_fw_basename

            if args.dfu:
                dfu_basename = os.path.basename(args.dfu)
                dfu_dst = os.path.join(actual_output_path, dfu_basename)
                shutil.copy2(args.dfu, dfu_dst)
                manifest["updater_dfu"] = dfu_basename

            # Write update.json
            manifest_path = os.path.join(actual_output_path, "update.json")
            with open(manifest_path, "w") as f:
                json.dump(manifest, f, indent=4)

            if is_tar_output:
                self.logger.info(
                    f"Creating final update bundle TAR archive at: {args.output_tar}"
                )
                self.create_tar_from_folder(
                    args.output_tar, actual_output_path
                )  # actual_output_path is the temp_dir
                self.logger.info(
                    f"Update bundle TAR archive created at {args.output_tar}"
                )
            else:
                self.logger.info(f"Update bundle folder created at {args.output}")

            return 0

        except Exception as e:
            self.logger.error(f"An error occurred during update bundle creation: {e}")
            return 1
        finally:
            if temp_dir_path:
                self.logger.info(f"Cleaning up temporary directory: {temp_dir_path}")
                shutil.rmtree(temp_dir_path)

    @staticmethod
    def compute_crc32(filepath):
        buf_size = 0xFFFF  # 64 KiB
        crc = 0
        with open(filepath, "rb") as f:
            while True:
                data = f.read(buf_size)
                if not data:
                    break
                crc = zlib.crc32(data, crc)
        return crc & 0xFFFFFFFF

    def create_tar_from_folder(self, output_filename, source_dir):
        """
        Creates a TAR archive from the source_dir with normalized metadata.
        """

        def reset_tarinfo(tarinfo):
            tarinfo.uid = 0
            tarinfo.gid = 0
            tarinfo.uname = "root"
            tarinfo.gname = "root"
            tarinfo.mtime = 0  # For deterministic output
            return tarinfo

        if not os.path.isdir(source_dir):
            raise FileNotFoundError(f"Source directory not found: {source_dir}")

        with tarfile.open(output_filename, "w") as tar:
            self.logger.debug(
                f"Archiving contents of {source_dir} into {output_filename}"
            )
            for item in os.listdir(source_dir):  # Iterate over top-level items first
                item_path = os.path.join(source_dir, item)
                # arcname is the path as it will appear in the archive's root
                self.logger.debug(f"Adding to TAR: {item_path} as {item}")
                tar.add(item_path, arcname=item, filter=reset_tarinfo)


if __name__ == "__main__":
    Main()()
