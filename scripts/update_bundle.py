#!/usr/bin/env python3

import os
import json
import shutil
import zlib
from flipper.app import App

# Bundler for updater package
# Usage example:
#   python3 update_bundle.py --stage updater_stage.bin --resources resources.zip --sil-fw sil_fw.bin --sil-radio-fw sil_radio_fw.bin --output ./update_folder


class Main(App):
    def init(self):
        self.parser.add_argument("--stage", required=False, help="Updater stage file")
        self.parser.add_argument(
            "--resources", required=False, help="Updater resources file"
        )
        self.parser.add_argument(
            "--sil-fw", required=False, help="Updater SIL firmware file"
        )
        self.parser.add_argument(
            "--sil-radio-fw", required=False, help="Updater SIL radio firmware file"
        )
        self.parser.add_argument("--dfu", required=False, help="Updater DFU file")
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

        # Copy files into output directory if provided
        if args.stage:
            stage_dst = os.path.join(args.output, os.path.basename(args.stage))
            shutil.copy2(args.stage, stage_dst)
            stage_crc32 = self.compute_crc32(stage_dst)
            manifest["updater_stage_crc32"] = stage_crc32
            manifest["updater_stage"] = os.path.basename(stage_dst)
        if args.resources:
            resources_dst = os.path.join(args.output, os.path.basename(args.resources))
            shutil.copy2(args.resources, resources_dst)
            manifest["updater_resources"] = os.path.basename(resources_dst)
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


if __name__ == "__main__":
    Main()()
