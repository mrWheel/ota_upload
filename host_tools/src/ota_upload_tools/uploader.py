#-- SPDX-License-Identifier: GPL-3.0-or-later
#-- Copyright (C) 2026 Willem Aandewiel

import argparse
import socket
import struct
import sys
from pathlib import Path

MAGIC = b"OTAMGR01"
DEFAULT_PORT = 3232
CHUNK_SIZE = 64 * 1024


def upload_firmware(host: str, firmware: Path, port: int = DEFAULT_PORT,
                    timeout: float = 10.0) -> None:
    firmware = Path(firmware)
    image_size = firmware.stat().st_size

    print(f"Firmware: {firmware}")
    print(f"Size:     {image_size:,} bytes")
    print(f"Target:   {host}:{port}")

    with socket.create_connection((host, port), timeout=timeout) as sock:
        sock.settimeout(timeout)
        sock.sendall(MAGIC + struct.pack(">I", image_size))

        sent = 0
        with firmware.open("rb") as image:
            while True:
                chunk = image.read(CHUNK_SIZE)
                if not chunk:
                    break
                sock.sendall(chunk)
                sent += len(chunk)
                percent = (sent * 100) // image_size
                print(f"\rUploading: {percent:3d}% ({sent:,}/{image_size:,})",
                      end="", flush=True)

        print()
        response = b""
        while not response.endswith(b"\n"):
            part = sock.recv(256)
            if not part:
                break
            response += part

    status = response.decode("utf-8", errors="replace").strip()
    if not status.startswith("OK"):
        raise RuntimeError(status or "Device closed connection without status")

    print(status)
    print("OTA upload completed successfully.")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Upload an ESP-IDF application image to ota_upload")
    parser.add_argument("firmware", type=Path)
    parser.add_argument("--host", required=True)
    parser.add_argument("--port", type=int, default=DEFAULT_PORT)
    parser.add_argument("--timeout", type=float, default=10.0)
    args = parser.parse_args()

    try:
        upload_firmware(args.host, args.firmware, args.port, args.timeout)
    except Exception as exc:
        print(f"OTA failed: {exc}", file=sys.stderr)
        raise SystemExit(1)


if __name__ == "__main__":
    main()
