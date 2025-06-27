#!/usr/bin/env python3

import os, sys, time
import subprocess, argparse
import shutil, platform

DEVICE_IP = "10.0.4.20"
DEVICE_IP_REF = "10.0.5.20"
DEVICE_PORT = 23

U5_TARGET_HW = 20
SI_TARGET_HW = 64

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
    return ret

def run_build_u5(args):
    cmd = f"./fbt TARGET_HW={U5_TARGET_HW}  updater_bin firmware_dfu resources"
    if args.verbose:
        print("Running:", cmd)
    ret = os.system(cmd)
    if ret != 0:
        print("Build failed for U5 target with return code:", ret)
    return ret

def run_build_si(args):
    cmd = f"./fbt TARGET_HW={SI_TARGET_HW}"
    if args.verbose:
        print("Running:", cmd)
    ret = os.system(cmd)
    if ret != 0:
        print("Build failed for SI917 target with return code:", ret)
    return ret

def run_build_update_bundle(args):
    upd_bundle_dir = "upd_bundle"
    upd_bundle_tar = "upd_bundle.tar"
    cmd_bundle = f"./scripts/update_bundle.py --target 20 --output {upd_bundle_dir} --stage fbt_layers/fbtng/build/f20-updater-D/updater.bin --dfu fbt_layers/fbtng/build/f20-firmware-D/firmware.dfu --sil-fw  fbt_layers/fbtng/build/f64-firmware-D/firmware.rps --resources fbt_layers/fbtng/build/f20-firmware-D/resources --sil-radio-fw ./lib/wiseconnect/connectivity_firmware/standard/SiWG917-B.2.13.4.1.0.4.rps"
    cmd_bundle_tar = f"./scripts/update_bundle.py --target 20 --output-tar {upd_bundle_tar} --stage fbt_layers/fbtng/build/f20-updater-D/updater.bin --dfu fbt_layers/fbtng/build/f20-firmware-D/firmware.dfu --sil-fw  fbt_layers/fbtng/build/f64-firmware-D/firmware.rps --resources fbt_layers/fbtng/build/f20-firmware-D/resources --sil-radio-fw ./lib/wiseconnect/connectivity_firmware/standard/SiWG917-B.2.13.4.1.0.4.rps"

    if args.verbose:
        print("Running:", cmd_bundle)
    ret = os.system(cmd_bundle_tar)
    if ret != 0:
        print("Update bundle build failed with return code:", ret)
    return ret

def run_update_via_tar_and_curl(args):
    if args.device_ip == "ref" or args.device_ip == "r":
        args.device_ip = DEVICE_IP_REF
    
    # curl -vvv "http://${DEVICE_IP}/api/v0/update" --data-binary '@upd_bundle.tar'
    upd_bundle_tar = "upd_bundle.tar"
    cmd = f"curl -vvv \"http://{args.device_ip}/api/v0/update\" --data-binary '@{upd_bundle_tar}'"
    if args.verbose:
        print("Running:", cmd)

    wait_for_device(args.device_ip, verbose=args.verbose)

    ret = os.system(cmd)
    if ret != 0:
        print("Update via tar and curl failed with return code:", ret)
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
        "build", help="Build all firmwares"
    )
    p_build_all.set_defaults(func=run_build_all)

    p_build_u5 = subparsers.add_parser(
        "build-u5", help="Build U5 firmware"
    )
    p_build_u5.set_defaults(func=run_build_u5)

    p_build_si = subparsers.add_parser(
        "build-si", help="Build SI917 firmware"
    )
    p_build_si.set_defaults(func=run_build_si)

    p_build_update_bundle = subparsers.add_parser(
        "build-update-bundle", help="Build update bundle"
    )
    p_build_update_bundle.set_defaults(func=run_build_update_bundle)

    p_update_via_tar_and_curl = subparsers.add_parser(
        "update-curl", help="Update device via tar and curl"
    )
    p_update_via_tar_and_curl.add_argument("-d", "--device_ip", help="Device IP", type=str, default=DEVICE_IP)
    p_update_via_tar_and_curl.set_defaults(func=run_update_via_tar_and_curl)

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

# https://flipperzero.atlassian.net/wiki/spaces/BL/pages/29465640962/Firmware+update
