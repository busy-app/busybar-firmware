#!/usr/bin/env python3
import os, sys, time
import subprocess, argparse
import shutil, platform

from flipper.storage_socket import FlipperStorage

# Device
DEVICE_IP = "10.0.4.20"
DEVICE_IP_REF = "10.0.5.20"
DEVICE_PORT = 23

# Firmware U5:
U5_TARGET_HW = 20

# Firmware SI917:
SI_TARGET_HW = 64
SI_RADIO_FW_PATH = "./lib/wiseconnect/connectivity_firmware/standard/SiWG917-B.2.14.5.0.0.10.rps"    # TODO: auto discover?

# Script settings:
RUN_ASSETS_DIR = ".run_assets"    # All build outputs will be placed here
UPDATE_BUNDLE_DIR = os.path.join(RUN_ASSETS_DIR, "upd_bundle")              # Vanilla, for update via storage.py and CLI
UPDATE_BUNDLE_TAR = os.path.join(RUN_ASSETS_DIR, "upd_bundle.tar")          # .tar, for update via HTTP API
UPDATE_BUNDLE_PROD_DIR = os.path.join(RUN_ASSETS_DIR, "upd_bundle_prod")    # For production line

# End of script settings

def subprocess_exec(cmd, verbose=False):
    if verbose:
        print("Run:", ' '.join(cmd) if isinstance(cmd, list) else cmd)
    result = subprocess.run(
        cmd,
        stdin=sys.stdin,
        stdout=sys.stdout,
        stderr=sys.stderr,
    )
    return result.returncode

def wait_for_device(device_ip, verbose=False):
    ts = time.time()

    ping_cmd = ['ping', '-c', '1', '-W', '1']  # Unix: -c count, -W timeout (sec)
    if platform.system() == 'Windows':
        ping_cmd = ['ping', '-n', '1', '-w', '1000']  # Windows: -n count, -w timeout (ms)

    ping_cmd.append(device_ip)

    while True:
        try:
            result = subprocess.run(
                ping_cmd,
                capture_output=True,
                text=True,
            )
            if result.returncode == 0:
                break
            elif verbose:
                print(f"Ping {device_ip} failed, ret: {result.returncode}")
        except Exception as e:
            print(f"Ping error: {device_ip}: {e}")
            time.sleep(1)
        time.sleep(0.1)

    if verbose:
        print(f"Device found in {time.time() - ts:.3f} seconds.")

def telnet_config_ensure():
    telnet_cfg_path = os.path.expanduser("~/.telnetrc")
    if platform.system() == "Windows":
        # Have not tested this on Windows, but it *may* work
        config_dir = os.environ.get("APPDATA") or os.path.expanduser("~")
        telnet_cfg_path = os.path.join(config_dir, "telnetrc")
    
    required_lines = [
        "DEFAULT",
        "  mode character",
        "  set binary"
    ]

    if not os.path.exists(telnet_cfg_path):
        with open(telnet_cfg_path, "w") as f:
            f.write("\n".join(required_lines) + "\n")
        print(f"\t!!! Created {telnet_cfg_path} with required configuration.")
        return

    with open(telnet_cfg_path, "r") as f:
        lines = f.read().splitlines()

    content_ok = all(line in lines for line in required_lines)

    if not content_ok:
        shutil.copy2(telnet_cfg_path, telnet_cfg_path + ".bak")
        with open(telnet_cfg_path, "w") as f:
            f.write("\n".join(required_lines) + "\n")
        print(f"\t!!! Updated {telnet_cfg_path} with required configuration, backup created as {telnet_cfg_path}.bak")

def telnet_launch(host: str, port: int):
    try:
        subprocess.run(["telnet", host, str(port)])
    except FileNotFoundError:
        print("FileNotFoundError: Telnet is not installed or not found in PATH.")
        return 1
    return 0

def run_cli(args):
    if args.device_ip == "ref" or args.device_ip == "r":
        args.device_ip = DEVICE_IP_REF

    if args.verbose:
        print(f"Connecting to {args.device_ip}:{args.device_port}...")
    
    wait_for_device(args.device_ip, verbose=args.verbose)

    telnet_config_ensure()
    telnet_launch(args.device_ip, args.device_port)

def run_build_all(args):
    ret = run_build_u5(args)
    if ret != 0:
        return ret
    
    ret = run_build_si(args)
    if ret != 0:
        return ret
    
    ret = run_build_update_bundles(args)
    return ret


def run_build_u5(args):
    cmd = ["./fbt", "TARGET_HW=" + str(U5_TARGET_HW), "updater_bin", "firmware_dfu", "resources"]
    return subprocess_exec(cmd, verbose=args.verbose)

def run_build_si(args):
    cmd = ["./fbt", "TARGET_HW=" + str(SI_TARGET_HW)]
    return subprocess_exec(cmd, verbose=args.verbose)

def ensure_run_assets_dir():
    if not os.path.exists(RUN_ASSETS_DIR):
        os.makedirs(RUN_ASSETS_DIR)
    else:
        if not os.path.isdir(RUN_ASSETS_DIR):
            print(f"Error: {RUN_ASSETS_DIR} exists but is not a directory.")
            sys.exit(1)

def ensure_update_tar(upd_bundle_tar):
    if not os.path.exists(upd_bundle_tar):
        print(f"Update bundle tar file '{upd_bundle_tar}' does not exist. Please run './run build-bundles' first.")
        return False
    return True

def run_build_update_bundles(args):
    upd_bundle_dir = UPDATE_BUNDLE_DIR
    upd_bundle_prod_dir = UPDATE_BUNDLE_PROD_DIR
    upd_bundle_tar = UPDATE_BUNDLE_TAR

    ensure_run_assets_dir()
    # TODO: check if the firmware builded successfully before running this command?

    if os.path.exists(upd_bundle_tar):
        os.remove(upd_bundle_tar)
    
    if os.path.exists(upd_bundle_dir):
        shutil.rmtree(upd_bundle_dir)

    bundles_cmds = []

    # Update bundle default
    bundles_cmds.append([
        "./scripts/update_bundle.py",
        "--target", f"{U5_TARGET_HW}",
        "--output", upd_bundle_dir,
        "--stage", f"fbt_layers/fbtng/build/f{U5_TARGET_HW}-updater-D/updater.bin",
        "--dfu", f"fbt_layers/fbtng/build/f{U5_TARGET_HW}-firmware-D/firmware.dfu",
        "--sil-fw", f"fbt_layers/fbtng/build/f{SI_TARGET_HW}-firmware-D/firmware.rps",
        "--resources", f"fbt_layers/fbtng/build/f{U5_TARGET_HW}-firmware-D/resources",
        "--sil-radio-fw", f"{SI_RADIO_FW_PATH}"
    ])

    # Update bundle.tar
    bundles_cmds.append([
        "./scripts/update_bundle.py",
        "--target", f"{U5_TARGET_HW}",
        "--output-tar", f"{upd_bundle_tar}",
        "--stage", f"fbt_layers/fbtng/build/f{U5_TARGET_HW}-updater-D/updater.bin",
        "--dfu", f"fbt_layers/fbtng/build/f{U5_TARGET_HW}-firmware-D/firmware.dfu",
        "--sil-fw", f"fbt_layers/fbtng/build/f{SI_TARGET_HW}-firmware-D/firmware.rps",
        "--resources", f"fbt_layers/fbtng/build/f{U5_TARGET_HW}-firmware-D/resources",
        "--sil-radio-fw", f"{SI_RADIO_FW_PATH}"
    ])

    # Update bundle for production line
    bundles_cmds.append([
        "./scripts/update_bundle.py",
        "--target", f"{U5_TARGET_HW}",
        "--output", upd_bundle_prod_dir,
        "--stage", f"fbt_layers/fbtng/build/f{U5_TARGET_HW}-updater-D/updater.bin",
        "--dfu", f"fbt_layers/fbtng/build/f{U5_TARGET_HW}-firmware-D/firmware.dfu",
        "--sil-fw", f"fbt_layers/fbtng/build/f{SI_TARGET_HW}-firmware-D/firmware.rps",
        "--resources", f"fbt_layers/fbtng/build/f{U5_TARGET_HW}-firmware-D/resources",
        "--sil-radio-fw", f"{SI_RADIO_FW_PATH}"
    ])
    bundles_cmds.append([
        "cp", "-v",
        f"fbt_layers/fbtng/build/f{U5_TARGET_HW}-firmware-D/firmware.elf",
        upd_bundle_prod_dir
    ])

    for cmd in bundles_cmds:
        ret = subprocess_exec(cmd, verbose=args.verbose)
        if ret != 0:
            print(f"Cmd {cmd} failed with return code:", ret)
            return ret
    return ret

def run_wait_for_device(args):
    if args.device_ip == "ref" or args.device_ip == "r":
        args.device_ip = DEVICE_IP_REF

    wait_for_device(args.device_ip, verbose=args.verbose)
    print(f"Device {args.device_ip} is reachable.")
    return 0

def run_update_via_http(args):
    # https://flipperzero.atlassian.net/wiki/spaces/BL/pages/29543628801/Self-update
    if args.device_ip == "ref" or args.device_ip == "r":
        args.device_ip = DEVICE_IP_REF

    upd_bundle_tar = UPDATE_BUNDLE_TAR
    assert ensure_update_tar(upd_bundle_tar) == True
    
    cmd = ["curl", "-vvv", f"http://{args.device_ip}/api/v0/update",
           "--data-binary", f"@{upd_bundle_tar}"]

    wait_for_device(args.device_ip, verbose=args.verbose)

    ret = subprocess_exec(cmd, verbose=args.verbose)
    if ret != 0:
        print(f"Update via HTTP failed with return code: {ret}")
    return ret

def run_update_via_storage(args):
    # https://flipperzero.atlassian.net/wiki/spaces/BL/pages/29543628801/Self-update
    if args.device_ip == "ref" or args.device_ip == "r":
        args.device_ip = DEVICE_IP_REF

    upd_bundle = UPDATE_BUNDLE_DIR
    bsb_update_dst = "/ext/tmp/upd_bundle"
    bsb_update_json = bsb_update_dst + "/update.json"
    # TODO: ensure that the update bundle exists

    cmd = ["python3", "./scripts/storage.py", "-p", args.device_ip, "send", upd_bundle, bsb_update_dst]

    wait_for_device(args.device_ip, verbose=args.verbose)

    ret = subprocess_exec(cmd, verbose=args.verbose)
    if ret != 0:
        print(f"Uploading bundle via storage.py failed with return code: {ret}")
        return ret

    print(f"Sending boot command to the device {args.device_ip}:{args.device_port}...")
    cmd_cli = f"update install {bsb_update_json}"
    bsb = FlipperStorage((args.device_ip, args.device_port))
    bsb.start()
    bsb.send_and_wait_eol(f"{cmd_cli}\r\n")

    return None

def run_flash_u5_dfu(args):
    if args.device_ip == "ref" or args.device_ip == "r":
        args.device_ip = DEVICE_IP_REF

    # wait_for_device(args.device_ip, verbose=args.verbose)

    dfu_file = os.path.join(UPDATE_BUNDLE_DIR, "firmware.dfu")  # TODO: auto discover
    cmd = ["./scripts/update.py",
            "-p", args.device_ip,
            "u5",
            dfu_file,
            "--to-dfu"
        ]

    ret = subprocess_exec(cmd, verbose=args.verbose)
    if ret != 0:
        print("\tPlease ensure that the device is in DFU mode and try again.")
        print("\tYou can do it by pressing and holding START/STOP and BACK button for 2 sec,")
        print("\tand then releasing BACK button, and then releasing START/STOP button after 1 sec.")
        print("\tMost possibly after the falsh via DFU you will have to reboot it manually.")
        print(f"Flashing U5 DFU failed with return code: {ret}")
    return ret

def main():
    # print("cwd:", os.getcwd())
    parser = argparse.ArgumentParser(description="Runner")
    # parser.add_argument("-v", "--verbose", help="Verbose", action="store_true")
    parser.parse_known_args()

    subparsers = parser.add_subparsers(
        dest="command", help="Commands to run", required=False
    )

    # CLI tool
    p_run_cli = subparsers.add_parser(
        "cli", help="CLI terminal via Telnet"
    )
    p_run_cli.add_argument("-d", "--device_ip", help="Device IP", type=str, default=DEVICE_IP)
    p_run_cli.add_argument("-p", "--device_port", help="Device Port", type=int, default=DEVICE_PORT)
    p_run_cli.set_defaults(func=run_cli)


    p_build_all = subparsers.add_parser(
        "build", help="Build all firmwares and bundles"
    )
    p_build_all.set_defaults(func=run_build_all)

    p_build_u5 = subparsers.add_parser(
        "build-u5", help="Build U5 firmware only"
    )
    p_build_u5.set_defaults(func=run_build_u5)

    p_build_si = subparsers.add_parser(
        "build-si", help="Build SI917 firmware only"
    )
    p_build_si.set_defaults(func=run_build_si)

    p_build_update_bundle = subparsers.add_parser(
        "build-bundles", help="Build all bundles: update, production, etc."
    )
    p_build_update_bundle.set_defaults(func=run_build_update_bundles)

    p_flash_u5_dfu = subparsers.add_parser(
        "flash-u5-dfu", help="Flash U5 firmware via DFU"
    )
    p_flash_u5_dfu.add_argument("-d", "--device_ip", help="Device IP", type=str, default=DEVICE_IP)
    p_flash_u5_dfu.add_argument("-p", "--device_port", help="Device Port", type=int, default=DEVICE_PORT)
    p_flash_u5_dfu.set_defaults(func=run_flash_u5_dfu)

    p_update_via_http = subparsers.add_parser(
        "update-http", help="Update device via HTTP API using curl (upd_bundle.tar)"
    )
    p_update_via_http.add_argument("-d", "--device_ip", help="Device IP", type=str, default=DEVICE_IP)
    p_update_via_http.add_argument("-p", "--device_port", help="Device Port", type=int, default=DEVICE_PORT)
    p_update_via_http.set_defaults(func=run_update_via_http)

    p_update_via_storage = subparsers.add_parser(
        "update-storage", help="Update device via storage.py (update bundle)"
    )
    p_update_via_storage.add_argument("-d", "--device_ip", help="Device IP", type=str, default=DEVICE_IP)
    p_update_via_storage.add_argument("-p", "--device_port", help="Device Port", type=int, default=DEVICE_PORT)
    p_update_via_storage.set_defaults(func=run_update_via_storage)

    p_wait_for_device = subparsers.add_parser(
        "wait", help="Just wait for device to be reachable via ping, nothing else"
    )
    p_wait_for_device.add_argument("-d", "--device_ip", help="Device IP", type=str, default=DEVICE_IP)
    p_wait_for_device.set_defaults(func=run_wait_for_device)
    

    args = parser.parse_args()

    args.verbose = True

    if args.command is not None:
        return args.func(args)
    else:
        parser.print_help()

if __name__ == "__main__":
    try:
        ret = main()
        print("RET: ", ret)
        if ret and ret != 0:
            print("Run: Exiting with error code", ret, file=sys.stderr)
            sys.exit(1)
    except KeyboardInterrupt:
        print("Run: Exited", file=sys.stderr)
        sys.exit(2)
    # except subprocess.CalledProcessError as e:
    #     sys.exit(e.returncode)
    except Exception as e:
        print(f"Run: Error: {e}", file=sys.stderr)
        sys.exit(3)

# Info:
# - https://flipperzero.atlassian.net/wiki/spaces/BL/pages/29465640962/Firmware+update

# TODO:
# - success build flags for bundles
# - check if bundle exists before running updates
# - readme
