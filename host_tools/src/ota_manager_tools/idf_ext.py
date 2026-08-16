#-- SPDX-License-Identifier: GPL-3.0-or-later
#-- Copyright (C) 2026 Willem Aandewiel

import json
from pathlib import Path

from .uploader import DEFAULT_PORT, upload_firmware


def _find_app_binary(project_dir: Path, build_dir: Path) -> Path:
    description_file = build_dir / "project_description.json"
    if not description_file.exists():
        raise RuntimeError(
            f"{description_file} does not exist. Run 'idf.py build' first.")

    data = json.loads(description_file.read_text(encoding="utf-8"))

    app_bin = data.get("app_bin")
    if app_bin:
        candidate = build_dir / app_bin
        if candidate.exists():
            return candidate

    project_name = data.get("project_name")
    if project_name:
        candidate = build_dir / f"{project_name}.bin"
        if candidate.exists():
            return candidate

    bins = sorted(build_dir.glob("*.bin"))
    bins = [
        p for p in bins
        if p.name not in {
            "bootloader.bin",
            "partition-table.bin",
            "ota_data_initial.bin",
            "ota_data_custom.bin"
        }
    ]
    if len(bins) == 1:
        return bins[0]
    if len(bins) > 1:
        return bins[0]  # Return first remaining candidate

    raise RuntimeError(
        "Unable to determine application binary from project_description.json")


def ota_action(target_name, ctx, global_args, host=None, port=None, timeout=None, **action_args):
    if port is None:
        port = DEFAULT_PORT
    if timeout is None:
        timeout = 10.0

    project_dir = Path(global_args.project_dir or ".").resolve()
    build_dir_value = global_args.build_dir or "build"
    build_dir = Path(build_dir_value)
    if not build_dir.is_absolute():
        build_dir = project_dir / build_dir

    firmware = _find_app_binary(project_dir, build_dir)
    upload_firmware(host, firmware, port=port, timeout=timeout)


def action_extensions(base_actions, project_path):
    return {
        "version": "1.0.0",
        "actions": {
            "ota": {
                "callback": ota_action,
                "help": "Upload the built application directly to ota_manager",
                "options": [
                    {
                        "names": ["--host"],
                        "help": "OTA target hostname or IP address",
                        "required": True,
                    },
                    {
                        "names": ["--port"],
                        "help": "OTA TCP port",
                        "type": int,
                        "default": DEFAULT_PORT,
                    },
                    {
                        "names": ["--timeout"],
                        "help": "Socket timeout in seconds",
                        "type": float,
                        "default": 10.0,
                    },
                ],
            }
        }
    }
