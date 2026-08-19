# PKI compatibility notes

## Background

ReplyBot originally answered every valid command by DM, including commands sent on the Primary channel.

That stopped being reliable after Meshtastic tightened direct-message handling. Normal text DMs now expect the destination public key to be available, and the router can reject the old channel-encrypted DM fallback when it is not.

The result was a common failure mode: ReplyBot received `/ping`, built the response, then failed when it tried to DM a sender whose key was not in the node database.

## Current behavior

| Incoming command | Sender key known | Reply |
| --- | --- | --- |
| DM | Yes/No | Direct reply attempt |
| Primary channel | Yes | Direct reply |
| Primary channel | No | Tagged Primary-channel reply |
| Secondary channel | Any | Ignored |

A DM is never converted to a public response. If a private request cannot be answered privately, ReplyBot stays silent.

For Primary-channel commands, the request is already public, so falling back to the Primary channel does not expose a private conversation.

## Key lookup

In normal unlicensed operation ReplyBot uses `NodeDB::copyPublicKey()` before choosing a private reply. This checks the same key storage used by current firmware instead of only looking at the currently hot node record.

ReplyBot does not perform encryption itself. The normal Meshtastic router still handles PKI, packet encoding, and transmission.

Licensed/ham mode does not require the same PKI path, so it is allowed to send the normal unicast reply without this key check.

## Why not start a key exchange automatically?

It is possible to queue the command, request NodeInfo or key verification, and send the reply after a key is learned. That adds state, timeouts, retries, and extra traffic for what is meant to be a small diagnostics module.

For now the Primary-channel fallback is simpler and deterministic.

## Parser cleanup

The README has always described commands as case-insensitive, but the old code used case-sensitive `strncmp()` calls. The parser now matches the documented behavior.

Accepted examples:

- `/ping`
- `/PING`
- `  /Hello`
- `/test extra text`

Rejected examples:

- `ping`
- `/pingpong`
- `/hello-world`
- `/status`

## Test checklist

Host tests cover the parser and the direct-vs-channel routing decision. Before upstreaming the firmware change, the useful hardware checks are:

1. DM `/ping` -> private reply.
2. Primary `/ping` from a known-key node -> private reply.
3. Primary `/ping` from a fresh/unknown-key node -> tagged Primary reply.
4. Secondary-channel `/ping` -> no reply.
5. DM cooldown is enforced at 15 seconds.
6. Primary cooldown is enforced at 60 seconds.
7. Mixed-case commands work.
8. Normal text is ignored.

A deferred key-exchange path may be worth revisiting later if the extra complexity is justified.