# ReplyBotModule

<p align="center">
  <img src="https://img.shields.io/badge/license-GPL--3.0-blue.svg" />
  <img src="https://img.shields.io/badge/Meshtastic-Firmware-success" />
  <img src="https://img.shields.io/badge/language-C%2B%2B-informational" />
  <img src="https://img.shields.io/badge/status-maintained-brightgreen" />
</p>

ReplyBot is a small Meshtastic firmware module for quick link checks. Send it a slash command and it replies with hop count, RSSI, and SNR.

ReplyBot was originally added to Meshtastic in the 2.7.x line. This repo keeps the standalone module and compatibility fixes as Meshtastic changes.

<p align="center">
  <img src="replybot_banner.jpeg" />
</p>

## Commands

| Command | Description |
| --- | --- |
| `/ping` | Return link diagnostics |
| `/hello` | Alias for `/ping` |
| `/test` | Alias for `/ping` |

Commands are case-insensitive. Leading whitespace and extra text after the command are allowed.

Example:

```text
🎙️ Mic Check: 1 Hops | RSSI -75 | SNR 9.4
```

## Reply behavior

Current Meshtastic DMs use PKI and may reject a unicast when the destination public key is not available. ReplyBot handles that case instead of silently losing the response.

| Command source | Sender key known | Reply |
| --- | --- | --- |
| Direct message | Any | Direct reply attempt |
| Primary channel | Yes | Direct reply |
| Primary channel | No | Tagged Primary-channel reply |
| Secondary channel | Any | Ignored |

Unknown-key fallback example:

```text
🎙️ For !1234abcd: 2 Hops | RSSI -91 | SNR 7.5
```

A command received by DM is never moved to a public channel. Licensed/ham mode keeps the firmware's normal non-PKI unicast behavior.

More detail is in [`docs/PKI_COMPATIBILITY.md`](docs/PKI_COMPATIBILITY.md).

## Rate limiting

| Message type | Per-sender cooldown |
| --- | --- |
| Direct message | 15 seconds |
| Primary-channel command | 60 seconds |

ReplyBot uses a small fixed-size cooldown table and does not dynamically allocate tracking entries.

## Enabling the module

ReplyBot is excluded from normal builds by default. In a Meshtastic source tree containing the module, enable it for your variant with:

```cpp
#undef MESHTASTIC_EXCLUDE_REPLYBOT
```

Then build and flash the firmware normally for your board.

## Tests

The command parser and reply-routing decision are kept in `ReplyBotLogic.h` so they can be tested without building the full firmware tree.

Run the host test with:

```bash
g++ -std=c++11 -Wall -Wextra -Werror tests/replybot_logic_test.cpp -o /tmp/replybot_logic_test
/tmp/replybot_logic_test
```

The test covers command parsing, case handling, invalid commands, and the direct-vs-channel fallback decision. GitHub Actions runs the same test on pushes and pull requests.

Real-device testing is still recommended before sending changes upstream to `meshtastic/firmware`.

## Files

- `ReplyBotModule.cpp` - Meshtastic packet handling and replies
- `ReplyBotModule.h` - module interface
- `ReplyBotLogic.h` - parser and reply-routing logic
- `tests/replybot_logic_test.cpp` - host test
- `docs/PKI_COMPATIBILITY.md` - notes on the current DM behavior

## Contributing

Issues, fixes, and radio test reports are welcome. For upstream changes, test against the current Meshtastic development branch and include the firmware version and hardware used.

## Shout-out

Huge thanks to [lzmesh.com](http://lzmesh.com) for helping with Meshtastic experimentation and learning.

<a href="https://discord.gg/FuK8fFjwjq">
  <img src="https://img.shields.io/badge/Discord-%235865F2.svg?logo=discord&logoColor=white" />
</a>

## License

Meshtastic firmware and this module are licensed under the **GNU General Public License v3.0**. See [`LICENSE`](LICENSE).