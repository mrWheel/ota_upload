# Security

Version 1.0.0 is intended for development on a trusted local network.

The initial TCP protocol has no authentication, encryption, or authorization.
Anyone who can reach the OTA port may attempt to upload firmware.

Do not:

- expose the port to the public Internet;
- port-forward it from a router;
- use this version as a production OTA security solution.

ESP-IDF still validates the received application image through its native OTA
image handling. That protects against malformed images but is not authentication.

Future work may add:

- authentication/challenge-response;
- TLS;
- signed-image requirements;
- Secure Boot documentation;
- Flash Encryption documentation;
- rollback/self-test workflows.
