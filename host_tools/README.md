# ota-upload-tools

Host-side companion package for the ESP-IDF `ota_upload` component.

Install into the same Python environment used by ESP-IDF:

```bash
python -m pip install -e .
```

Commands:

```bash
ota-upload --host thisProject.local build/thisProject.bin
idf.py ota --host thisProject.local
```

The `idf.py ota` command obtains the application binary from
`build/project_description.json`.
