# Project Prompt — ESP-IDF `ota_upload`

## 1. Project goal

Create a production-quality, reusable ESP-IDF component named `ota_upload`.

The component must allow a developer to upload newly built ESP-IDF application firmware directly from the development computer to an ESP32 over the local network (Wi-Fi), without requiring an external web server, cloud service, AWS server, or firmware-hosting service.

The intended developer experience is comparable to ArduinoOTA / PlatformIO OTA:

1. The first firmware installation is performed over USB.
2. The firmware contains `ota_upload`.
3. Subsequent application firmware updates can be sent directly from the developer's computer to the ESP32 over the LAN.
4. The ESP32 writes the received application image to the inactive OTA partition.
5. After successful validation, the new OTA partition becomes the boot partition.
6. The ESP32 reboots into the new firmware.

The project must be suitable for publication in the official **ESP Component Registry** and must include an example, documentation, configuration, host-side tooling, and tests where practical.

---

## 2. Important naming convention

This project is an explicit exception to the developer's normal `lowerCamelCase` convention.

**Use `snake_case` throughout this project.**

This applies to:

- C functions
- C variables
- C structs
- typedefs where appropriate
- filenames
- directories
- CMake variables owned by this project where appropriate
- Python functions and variables
- configuration names
- internal identifiers

Examples:

```c
ota_upload_start();
ota_upload_stop();
ota_upload_config_t;
ota_upload_get_status();
```

Do **not** use:

```c
otaManagerStart();
otaManagerStop();
otaManagerConfig;
```

ESP-IDF/ESP-IDF-style conventions should be followed where they make sense.

Constants and Kconfig symbols may use the normal uppercase convention:

```c
CONFIG_OTA_UPLOAD_PORT
OTA_UPLOAD_DEFAULT_PORT
```

---

## 3. General implementation requirements

Use native ESP-IDF functionality wherever possible.

Do not depend on the Arduino framework.

The embedded implementation should use the official ESP-IDF OTA APIs, including the appropriate APIs from:

```c
esp_ota_ops.h
```

The implementation will normally use functionality equivalent to:

```c
esp_ota_get_next_update_partition()
esp_ota_begin()
esp_ota_write()
esp_ota_end()
esp_ota_set_boot_partition()
esp_restart()
```

Use the actual APIs correctly according to the supported ESP-IDF version.

Use `ESP_LOGE`, `ESP_LOGW`, `ESP_LOGI`, `ESP_LOGD`, etc. extensively and appropriately.

All errors must be handled. Do not silently ignore ESP-IDF return values.

---

## 4. OTA architecture

The OTA mechanism is a **push model**:

```text
Development computer
        |
        | local network / Wi-Fi
        | direct firmware upload
        v
      ESP32
        |
        +--> inactive OTA partition
        |
        +--> validate image
        |
        +--> select new boot partition
        |
        +--> reboot
```

This is deliberately different from `esp_https_ota`, where the ESP32 normally downloads firmware from an HTTP/HTTPS server.

For this project:

**The developer's computer initiates the transfer and sends the firmware directly to the ESP32.**

No special web server is required.

---

## 5. Partition requirements

OTA requires a suitable partition table.

The documentation and example must explain this clearly.

At minimum, the design must account for:

- `otadata`
- `ota_0`
- `ota_1`
- normal supporting data partitions such as NVS as required

Provide a valid example `partitions.csv`.

Do not hard-code offsets unnecessarily. Prefer a conventional ESP-IDF partition layout.

The component must detect/report useful errors where possible when the running firmware does not have a usable OTA partition available.

---

## 6. Network OTA receiver

Implement a network service on the ESP32 that accepts an application firmware image from the host-side uploader.

The initial implementation should preferably use a simple TCP-based protocol.

The protocol must be explicitly documented.

At minimum it must provide enough information to safely transfer:

- protocol/version identification
- firmware size
- firmware data
- transfer progress
- success/failure result
- validation result

Design the protocol so it can be extended later without unnecessarily breaking compatibility.

Do not invent unnecessary complexity for v1.0.0.

---

## 7. Firmware safety

OTA must never overwrite the currently running application partition.

The component must:

1. determine the currently running partition;
2. determine the correct inactive OTA update partition;
3. start the OTA operation;
4. receive and write the firmware;
5. abort cleanly if the network transfer fails;
6. call the appropriate ESP-IDF image/OTA validation functions;
7. only select the new boot partition after a successful complete transfer;
8. reboot only after successful completion.

A failed or interrupted upload must leave the existing firmware bootable.

Use ESP-IDF's native image validation mechanisms rather than implementing a redundant custom firmware parser.

Additional checksum/hash support may be added to the transfer protocol where useful, but should complement rather than replace ESP-IDF image validation.

---

## 8. mDNS discovery

Support mDNS so devices can be addressed by a hostname such as:

```text
thisProject.local
```

The component should allow the application to configure a hostname.

For example:

```c
ota_upload_config_t config = {
    .hostname = "thisProject",
    .port = 3232,
};

ESP_ERROR_CHECK(ota_upload_start(&config));
```

Use designated initializers and provide sensible defaults.

Also investigate advertising a dedicated OTA mDNS service, for example a service type such as:

```text
_esp-ota._tcp
```

The exact service name must be documented and consistently used by both the embedded component and the host-side discovery tool.

The component advertises the `_esp-ota._tcp` service so standard mDNS tools can
list OTA-capable devices. The current host tool does not perform discovery or
interactive selection; the user supplies the selected hostname or IP address.

---

## 9. Host-side uploader

Provide a host-side Python implementation that can upload the application `.bin` file directly to the ESP32.

Internally this may consist of a module/tool such as:

```text
tools/
    ota_upload.py
```

The uploader must support at least:

```text
host/address
firmware filename
port
progress reporting
connection timeout
transfer errors
device-side errors
```

Example low-level invocation:

```bash
python ota_upload.py thisProject.local build/thisProject.bin
```

However, this is **not intended to be the normal end-user interface**.

The preferred user experience is integration with `idf.py`.

---

## 10. `idf.py` integration — major requirement

The implemented developer workflow is:

```bash
idf.py build
idf.py ota --host thisProject.local
```

The current implementation does not support:

```bash
idf.py ota
```

The current `ota` command:

1. locate the built application firmware from `build/project_description.json`;
2. require the target hostname or IP address through `--host`;
3. upload the application image;
4. show upload progress;
5. report device-side validation and success/failure.

Host-side mDNS discovery and interactive device selection are not implemented in
the current version. Use `dns-sd -B _esp-ota._tcp local` to inspect advertised
devices, then pass the selected hostname or IP address with `--host`.

Example:

```text
$ idf.py ota --host thisProject.local

Firmware:
  build/thisProject.bin
  Size: 1,247,632 bytes

Uploading: [==============================] 100%

OTA successful.
```

---

## 11. IMPORTANT: investigate ESP-IDF trust restrictions for `idf.py` extensions

Do **not** simply assume that placing `idf_ext.py` inside a third-party Registry component will make `idf.py ota` available.

Current ESP-IDF documentation states that `idf.py` can discover an `idf_ext.py` from components participating in the build, but component extensions are subject to a **trust policy**.

In particular, ESP-IDF currently documents that Registry component extensions are automatically trusted for the `espressif/` namespace, while arbitrary third-party Registry namespaces are not automatically trusted. Untrusted component extensions may be skipped unless:

```bash
IDF_EXTENSION_ALLOW_UNTRUSTED=1
```

is enabled.

Therefore a component published under a namespace such as:

```text
mrwheel/ota_upload
```

must **not** rely on an automatically loaded `idf_ext.py` unless this behavior has been verified for the targeted ESP-IDF versions.

Research the current supported mechanism and implement the cleanest safe solution.

ESP-IDF also supports `idf.py` extensions supplied by installed Python packages using Python entry points in the:

```text
idf_extension
```

entry-point group.

This may be the preferable architecture for the host-side `idf.py ota` integration.

Possible architecture:

```text
ESP Component Registry
    mrwheel/ota_upload
        |
        +--> embedded ESP-IDF component

Python package / host tooling
        |
        +--> OTA uploader
        +--> mDNS discovery
        +--> idf.py extension
        +--> `idf.py ota`
```

The implementation must clearly document how the host-side integration is installed.

The goal remains that a developer should **not** have to manually locate and invoke:

```text
managed_components/.../tools/ota_upload.py
```

for normal use.

If a companion Python package is necessary, provide a clean installation method and document it.

Do not require users to globally enable loading of arbitrary untrusted ESP-IDF component extensions merely to make this component convenient.

---

## 12. Build artifact discovery

The `idf.py ota` extension should determine the correct application binary automatically from the ESP-IDF project/build information.

The current implementation obtains the application binary from
`build/project_description.json`, so the application/project name does not need
to be hardcoded in the host tool.

The extension uses ESP-IDF's configured project and build directories when they
are provided by `idf.py`.

If the firmware has not yet been built, produce a clear message.

The extension does not build the firmware automatically. Run `idf.py build`
before `idf.py ota`.

---

## 13. Kconfig / menuconfig

Provide `Kconfig` options so the component integrates naturally with:

```bash
idf.py menuconfig
```

Suggested structure:

```text
Component config
  -> OTA Upload
       [*] Enable OTA Upload
       Default OTA port
       [*] Enable mDNS
       Default hostname / hostname behavior
       [*] Reboot after successful update
       Logging options where useful
```

Do not put configuration into Kconfig when it is clearly better supplied at runtime by `ota_upload_config_t`.

Establish a clean distinction between:

- compile-time defaults;
- application/runtime configuration.

---

## 14. Public API

Keep the public API small.

A possible initial API is:

```c
#include "ota_upload.h"

ota_upload_config_t config = OTA_UPLOAD_CONFIG_DEFAULT();

config.hostname = "thisProject";

ESP_ERROR_CHECK(ota_upload_start(&config));
```

Potential API:

```c
esp_err_t ota_upload_start(const ota_upload_config_t *config);
esp_err_t ota_upload_stop(void);
bool ota_upload_is_running(void);
```

Only add public functions that have a real use case.

Do not expose internal sockets, tasks, buffers, or ESP-IDF implementation details unnecessarily.

---

## 15. FreeRTOS behavior

The network receiver will probably require its own task.

Document:

- task purpose;
- task lifetime;
- stack size;
- priority;
- shutdown behavior;
- network timeout behavior.

Avoid busy waiting.

Use appropriate FreeRTOS synchronization primitives where required.

The component must not monopolize a CPU core.

Do not pin tasks to a core unless there is a demonstrated reason.

---

## 16. Memory usage

ESP32 projects may have limited RAM.

Do not load the entire firmware image into RAM.

Receive the image in reasonably sized chunks and immediately pass them to the ESP-IDF OTA write mechanism.

Make buffer size configurable only if there is a meaningful reason.

Document approximate RAM overhead.

---

## 17. Security

Version `1.0.0` is primarily intended for development on a trusted local network.

Nevertheless, do not design the protocol so badly that security cannot be added later.

Document clearly that an unauthenticated OTA listener must **not** be considered secure for deployment on an untrusted network.

Design the protocol/API so later versions can add:

- authentication;
- challenge/response;
- TLS;
- signed firmware;
- secure boot compatibility;
- flash encryption compatibility.

Do not falsely claim that v1.0.0 is production-secure if authentication/encryption is not implemented.

---

## 18. v1.0.0 scope

Target the following for v1.0.0:

| Feature | v1.0.0 |
|---|---|
| Direct development computer -> ESP32 OTA | Yes |
| External web server required | No |
| Native ESP-IDF OTA APIs | Yes |
| `ota_0` / `ota_1` support | Yes |
| TCP firmware upload | Yes |
| mDNS hostname | Yes |
| mDNS OTA service advertising | Yes |
| Upload progress | Yes |
| ESP-IDF image validation | Yes |
| Automatic reboot | Yes |
| `Kconfig` | Yes |
| `ESP_LOGx` logging | Yes |
| Python host uploader | Yes |
| `idf.py ota --host <hostname-or-ip>` integration | Yes, through the installed host package |
| Basic example | Yes |
| README | Yes |
| API documentation | Yes |
| Protocol documentation | Yes |
| ESP Component Registry compatible | Yes |
| Authentication | Later |
| TLS | Later |
| Advanced rollback/self-test | Later |
| Firmware signing integration documentation | Later |

---

## 19. Target ESP chips

Design the component around ESP-IDF APIs rather than chip-specific implementation.

Initially investigate/support Wi-Fi capable targets such as:

```text
ESP32
ESP32-S2
ESP32-S3
ESP32-C3
ESP32-C6
```

Do not claim support for a target until it has either been tested or the documentation clearly labels it as expected/untested.

Use the `targets` metadata in `idf_component.yml` when appropriate.

---

## 20. ESP Component Registry structure

The repository/component should be structured approximately as follows, adjusting where necessary after implementing the host-side Python packaging correctly:

```text
ota_upload/
├── CMakeLists.txt
├── idf_component.yml
├── Kconfig
├── LICENSE
├── README.md
├── API.md
├── CHANGELOG.md
│
├── include/
│   └── ota_upload.h
│
├── ota_upload.c
├── ota_upload_network.c
├── ota_upload_mdns.c
│
├── private_include/
│   └── ...
│
├── examples/
│   └── basic/
│       ├── CMakeLists.txt
│       ├── README.md
│       ├── partitions.csv
│       ├── sdkconfig.defaults
│       └── main/
│           ├── CMakeLists.txt
│           ├── idf_component.yml
│           └── main.c
│
├── tools/
│   └── ...
│
└── docs/
    ├── protocol.md
    ├── security.md
    └── idf_py_integration.md
```

If the Python `idf.py` extension is best distributed as a separate installable Python package, organize the repository cleanly, for example:

```text
host_tools/
├── pyproject.toml
└── src/
    └── ...
```

Do not distort the ESP-IDF component layout merely to accommodate Python packaging.

---

## 21. `idf_component.yml`

Create a proper Registry manifest.

It should include at least appropriate values for:

```yaml
version: "1.0.0"
description: "..."
license: "MIT"

dependencies:
  idf:
    version: "..."
```

Also include appropriate metadata such as:

```yaml
url:
repository:
documentation:
issues:
maintainers:
tags:
```

Use the actual repository URL once known.

Use valid SPDX license identifiers.

The manifest must be valid according to the current IDF Component Manager specification.

---

## 22. Examples

Provide at least:

```text
examples/basic
```

The example must be a complete, self-contained ESP-IDF project.

The ESP Component Registry automatically discovers examples under `examples/`, so structure it according to the current Registry requirements.

The example must demonstrate:

1. Wi-Fi initialization;
2. network connection;
3. starting `ota_upload`;
4. mDNS/OTA availability;
5. first USB flash;
6. subsequent OTA upload.

Keep credentials out of source control.

Use an appropriate example mechanism for Wi-Fi credentials, preferably menuconfig/Kconfig or another standard ESP-IDF example pattern.

The example README must provide exact commands.

---

## 23. Example developer workflow

The documentation should lead toward a workflow similar to:

### Add the component

```bash
idf.py add-dependency "mrwheel/ota_upload^1.0.0"
```

The exact namespace must be replaced by the actual Registry namespace when known.

### Configure

```bash
idf.py menuconfig
```

### First installation

```bash
idf.py build
idf.py flash monitor
```

### Subsequent development

```bash
idf.py build
idf.py ota --host thisProject.local
```

The final documentation must distinguish clearly between functionality available immediately after adding the Registry component and any additional one-time host-tool installation needed for `idf.py ota`.

Do not pretend that a third-party Registry component automatically receives trusted `idf.py` extension privileges if ESP-IDF does not permit that.

---

## 24. Documentation

Create high-quality English documentation.

At minimum:

### `README.md`

Include:

- purpose;
- major features;
- supported ESP-IDF versions;
- supported/tested chips;
- installation;
- minimal API example;
- required partition table;
- first USB flash;
- OTA workflow;
  - `idf.py ota --host <hostname-or-ip>`;
  - mDNS hostname and service advertising;
- troubleshooting;
- security warning;
- links to detailed docs.

### `API.md`

Document the public C API.

### `docs/protocol.md`

Document the OTA wire protocol sufficiently that another developer could implement an uploader independently.

### `docs/security.md`

Explain the v1.0.0 threat model and limitations.

### `docs/idf_py_integration.md`

Explain exactly how the `idf.py ota` integration works, how it is installed, and why the chosen integration mechanism is used.

### `CHANGELOG.md`

Start with `1.0.0`.

---

## 25. ESP-IDF version compatibility

Choose a sensible minimum supported ESP-IDF 5.x version after checking the APIs actually used.

Do not automatically select the newest ESP-IDF version as the minimum unless required.

Avoid deprecated APIs.

Where ESP-IDF versions differ, use clean compile-time compatibility handling only when justified.

Document the supported version range.

---

## 26. Code quality

Requirements:

- `snake_case`, as specified above;
- clear module boundaries;
- minimal public API;
- no Arduino dependencies;
- no unnecessary global mutable state;
- proper `esp_err_t` error propagation;
- extensive but sensible `ESP_LOGx`;
- no ignored compiler warnings;
- no magic constants when named constants are appropriate;
- no entire-image RAM buffering;
- clean shutdown/error paths;
- comments explain **why**, not obvious syntax.

Use normal ESP-IDF C style.

Prefer C unless there is a strong technical reason to require C++.

---

## 27. Testing

Where practical, add tests for:

- protocol parsing;
- invalid headers;
- firmware size validation;
- interrupted transfer;
- socket timeout;
- invalid image;
- OTA abort behavior;
- mDNS discovery logic on the host;
- build artifact discovery;
- host command-line parsing.

At minimum, document a manual test matrix covering:

1. successful OTA;
2. interrupted OTA;
3. wrong/non-ESP firmware;
4. reboot after successful OTA;
5. power-cycle after failed OTA;
6. device not found;
7. multiple devices found;
8. explicit `--host`;
9. incorrect hostname;
10. project with custom application name.

---

## 28. Publication

The finished component must be suitable for publication in the ESP Component Registry.

Follow the current official Component Manager/Registry packaging requirements.

The Registry documentation currently recommends/uses files such as:

```text
CMakeLists.txt
idf_component.yml
README.md
LICENSE
```

and automatically discovers examples under:

```text
examples/
```

The project should also be suitable for automated publication from GitHub Actions in a later step.

Do not publish anything automatically as part of development unless explicitly requested.

---

## 29. Design principle

The most important usability goal is:

> After the first USB installation, OTA flashing should feel like a normal ESP-IDF development operation rather than a separate firmware-distribution system.

The implemented end result is:

```bash
idf.py build
idf.py ota --host <hostname-or-ip>
```

not:

```bash
python managed_components/mrwheel__ota_upload/tools/ota_upload.py \
    thisProject.local \
    build/thisProject.bin
```

The Python uploader is also available as a direct command, but the documented
developer interface is the installed `idf.py ota` extension.

The developer should normally interact with `idf.py`.

---

## 30. Do not hide architectural limitations

Before implementing `idf.py ota`, verify the current ESP-IDF extension mechanism.

Current ESP-IDF documentation specifically describes:

- project/component `idf_ext.py` extensions;
- trusted-source restrictions for component extensions;
- `espressif/` Registry namespace trust behavior;
- `IDF_EXTENSION_ALLOW_UNTRUSTED`;
- Python package `idf_extension` entry points.

The solution must work within these rules rather than relying on undocumented behavior.

If a separate Python package is required to achieve a safe, convenient `idf.py ota` command for a third-party Registry component, implement/document that architecture clearly.

---

## 31. Deliverables

Produce a complete repository ready for development and eventual publication, containing:

1. `ota_upload` ESP-IDF component;
2. public header/API;
3. OTA receiver;
4. mDNS support;
5. OTA partition handling;
6. `Kconfig`;
7. `idf_component.yml`;
8. basic example project;
9. example OTA partition table;
10. Python host uploader;
11. safe `idf.py ota` integration;
  12. mDNS service advertising;
13. progress/error reporting;
14. README;
15. API documentation;
16. protocol documentation;
17. security documentation;
18. changelog;
19. tests/test plan;
20. publication-ready Registry metadata.

Implement this incrementally and keep the project buildable at meaningful milestones.

Do not sacrifice correctness or ESP-IDF conventions merely to make the first prototype shorter.
