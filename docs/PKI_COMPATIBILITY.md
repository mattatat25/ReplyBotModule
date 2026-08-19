# ReplyBot and modern Meshtastic direct messages

## Why ReplyBot needed a compatibility rewrite

ReplyBot was originally designed around a simple rule: accept a slash command either by direct message or on the primary channel, then always send the diagnostic back as a direct message.

That behavior was convenient when a unicast text packet could fall back to ordinary channel encryption. Modern Meshtastic intentionally tightened direct-message handling. Normal text DMs now use the destination node's public key and the router refuses the old insecure legacy-DM fallback when that key is unavailable.

The result was subtle: ReplyBot could hear a perfectly valid `/ping` on the primary channel, build a response, and then fail when it tried to privately unicast the answer to a sender whose key was not available to the ReplyBot node.

The parser and radio diagnostics were not the real problem. The response transport assumption was.

## ReplyBot 2.x response policy

The rewritten module makes the transport choice explicit:

| Incoming command | Sender public key known | Reply |
| --- | --- | --- |
| Direct message | Yes | Direct message |
| Direct message | No | Direct-message attempt; never public fallback |
| Primary-channel broadcast | Yes | Direct message using normal Meshtastic PKI routing |
| Primary-channel broadcast | No | Primary-channel response tagged with the requesting node ID |
| Secondary-channel broadcast | Any | Ignored |

This preserves the original "private when possible" experience without asking the router to perform a legacy DM that current firmware is designed to reject.

## Why an incoming DM never falls back to the channel

Privacy is a one-way boundary here. A user who sends a command publicly has already chosen a public transport, so a public diagnostic fallback does not reveal a private conversation. A user who sends `/ping` privately has not made that choice.

ReplyBot therefore never converts an incoming DM into a public channel response. If a direct response cannot be sent, the module remains silent rather than exposing the interaction.

## How the key check works

For normal unlicensed operation, ReplyBot uses `NodeDB::copyPublicKey()` rather than reading only the currently hot node record. That mirrors the key-resolution path used by current Meshtastic routing and allows the firmware's warm key store to satisfy the check when available.

Licensed/ham mode is treated separately because current Meshtastic deliberately does not use PKI for those unicasts; a public key is therefore not required before choosing a direct reply.

The module does not implement its own encryption. It only decides whether a private response is currently viable. The normal Meshtastic router remains responsible for PKI selection, encryption, packet routing, and failure handling.

## Why ReplyBot does not automatically start a key exchange

A more complex design could queue a pending `/ping`, initiate a NodeInfo or key-verification exchange, wait for the sender key to become authoritative, and then release a delayed private reply.

That is intentionally not part of this compatibility rewrite. It would add state, timeouts, cross-module coupling, retransmission decisions, and extra mesh traffic to a module whose purpose is to be a tiny link-health check.

The deterministic channel fallback is easier to reason about and keeps ReplyBot useful even on a freshly discovered node.

## Parser fixes included in the rewrite

The old documentation described commands as case-insensitive, while the implementation used case-sensitive `strncmp()` calls. The parser is now actually case-insensitive and continues to allow leading whitespace and additional text after a valid command token.

Examples accepted by the new parser:

- `/ping`
- `/PING`
- `  /Hello`
- `/test anything after this is ignored`

Examples rejected:

- `ping`
- `/pingpong`
- `/hello-world`
- `/status`

## Test strategy

The repository now contains a small firmware-independent logic layer in `ReplyBotLogic.h`. `tests/replybot_logic_test.cpp` validates the command parser and the privacy/routing decision matrix using ordinary C++11.

GitHub Actions compiles the test with `-Wall -Wextra -Werror` and runs it on every push and pull request.

This host test intentionally does not pretend to replace real radio testing. Before proposing the same source change upstream to `meshtastic/firmware`, the recommended device matrix is:

1. PKI-capable node sends `/ping` by DM -> private response.
2. Known-key node sends `/ping` on Primary -> private response.
3. Unknown-key node sends `/ping` on Primary -> tagged Primary response.
4. `/ping` on a secondary channel -> no response.
5. Repeated DM command inside 15 seconds -> suppressed.
6. Repeated Primary command inside 60 seconds -> suppressed.
7. Uppercase/mixed-case command -> accepted.
8. Ordinary non-command text -> ignored.

## Future work

Possible future additions, in order of usefulness:

- Optional deferred private reply after key acquisition.
- Protobuf configuration instead of compile-time-only enablement.
- Optional command set beyond link diagnostics.
- Firmware-native unit/integration coverage in the Meshtastic repository.
- Exposed counters for received commands, rate-limited commands, PKI replies, and channel fallbacks.

The compatibility rewrite deliberately keeps those out of the critical path so ReplyBot can remain small, predictable, and inexpensive on the mesh.
