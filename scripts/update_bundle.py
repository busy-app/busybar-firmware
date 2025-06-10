#!/usr/bin/env python3

import os
import json
import shutil
import zlib
import tarfile
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
        self.parser.add_argument(
            "--output", required=True, help="Output directory for update bundle"
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
        os.makedirs(args.output, exist_ok=True)

        manifest = {"target": args.target, "version": 1}
        if args.update_name:
            manifest["update_name"] = args.update_name

        # Copy files into output directory if provided
        if args.stage:
            stage_dst = os.path.join(args.output, os.path.basename(args.stage))
            shutil.copy2(args.stage, stage_dst)
            stage_crc32 = self.compute_crc32(stage_dst)
            manifest["updater_stage_crc32"] = stage_crc32
            manifest["updater_stage"] = os.path.basename(stage_dst)

        # Handle resources
        if args.resources:
            generated_resources_tar_filename = "resources.tar"
            resources_dst_path = os.path.join(
                args.output, generated_resources_tar_filename
            )

            self.logger.info(
                f"Creating TAR archive for resources from folder: {args.resources}"
            )
            try:
                self.create_tar_from_folder(resources_dst_path, args.resources)
                manifest["updater_resources"] = generated_resources_tar_filename
                self.logger.info(f"Successfully created {resources_dst_path}")
            except Exception as e:
                self.logger.error(f"Failed to create TAR from {args.resources}: {e}")
                return 1  # Indicate an error

        if args.sil_fw:
            sil_fw_dst = os.path.join(args.output, os.path.basename(args.sil_fw))
            shutil.copy2(args.sil_fw, sil_fw_dst)
            manifest["updater_sil_fw"] = os.path.basename(sil_fw_dst)

        if args.sil_radio_fw:
            sil_radio_fw_dst = os.path.join(
                args.output, os.path.basename(args.sil_radio_fw)
            )
            shutil.copy2(args.sil_radio_fw, sil_radio_fw_dst)
            manifest["updater_sil_radio_fw"] = os.path.basename(sil_radio_fw_dst)

        if args.dfu:
            dfu_dst = os.path.join(args.output, os.path.basename(args.dfu))
            shutil.copy2(args.dfu, dfu_dst)
            manifest["updater_dfu"] = os.path.basename(dfu_dst)

        # Write update.json
        manifest_path = os.path.join(args.output, "update.json")
        with open(manifest_path, "w") as f:
            json.dump(manifest, f, indent=4)
        self.logger.info(f"Update bundle created at {args.output}")
        return 0

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
            self.logger.error(f"Resources folder not found: {source_dir}")
            raise FileNotFoundError(f"Resources folder not found: {source_dir}")

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
