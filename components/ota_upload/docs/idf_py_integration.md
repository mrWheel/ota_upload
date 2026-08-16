# `idf.py ota` integration

ESP-IDF supports extensions from component `idf_ext.py` files and from installed
Python packages using the `idf_extension` entry-point group.

Third-party components downloaded from the ESP Component Registry are not
automatically trusted for `idf_ext.py`; the automatically trusted Registry
namespace is `espressif/`.

For that reason this project does not require users to enable:

```bash
IDF_EXTENSION_ALLOW_UNTRUSTED=1
```

Instead, the host-side command is supplied as a companion Python package in
`host_tools/`.

Install it into the same Python environment used by `idf.py`:

```bash
python -m pip install -e ./host_tools
```

The package declares:

```toml
[project.entry-points.idf_extension]
ota_manager_ext = "ota_manager_tools.idf_ext:action_extensions"
```

After installation:

```bash
idf.py --help
idf.py ota --host <>.local
```

The extension reads ESP-IDF's `build/project_description.json` to find the
application binary instead of assuming a project filename.
