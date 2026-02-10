# ReplyBotModule
MESHTASTIC REPLYBOT MODULE

The ReplyBot module adds a friendly, lightweight auto-responder to your Meshtastic node. When enabled it listens for simple slash commands and replies with useful diagnostics to help you understand the health of your off‑grid mesh network. 
This document explains what the module does, how to use it, and offers some context about why it might be helpful.

WHY A REPLYBOT?

In a mesh network it’s sometimes hard to tell whether your messages are reaching other nodes or how healthy the link is. ReplyBot lets you send a quick command from any Meshtastic device and get an immediate response. 
Each reply reports three useful diagnostics: the hop distance (how many relays were used to deliver your message), the received signal strength indicator (RSSI, a measure of signal quality in dBm, normalized if necessary) and the signal‑to‑noise ratio (SNR), 
which shows how much the signal stands above the radio noise floor. It’s like a radio “mic check” for your mesh: you can quickly verify that your device is connected and see the health of the connection.

SUPPORTED COMMANDS

ReplyBot reacts to three simple slash commands. Commands are case‑insensitive and must be preceded by a slash (/). 
Any additional text after the command is ignored. Supported commands are:

/ping	Ask the bot to confirm it’s alive and get a diagnostic reply.
/hello	An alias for /ping; returns the same diagnostic reply.
/test	Another alias for /ping.

Commands can be sent either as a direct message to the ReplyBot’s node or as a broadcast on the primary channel. When you send a DM with a supported command, the bot replies directly to you. 
If you post the command on the primary channel, the bot still sees it (because it runs in “promiscuous” mode) and returns the diagnostics via DM so you don’t spam the rest of the network. 
Only the primary channel is monitored for broadcasts, messages on secondary channels are ignored.

RATE LIMITING

To keep the network responsive for everyone, ReplyBot enforces a per‑sender cooldown. When you send a command, the bot checks when it last responded to your node and ignores requests that arrive too frequently. 
Direct messages have a 15‑second cooldown, while broadcast commands on the primary channel have a 60‑second cooldown. Simply wait a bit between commands if you find yourself rate‑limited.

HOW IT WORKS 

The ReplyBot module is written in C++ as part of the Meshtastic firmware. When compiled into the firmware, it registers itself as a text message handler and runs in promiscuous mode so it can see broadcasts as well as direct messages. 
Internally, it listens for incoming packets on the text message port, filters out any traffic that isn’t addressed to it or broadcast on the primary channel, 
parses the payload for supported slash commands, applies a per‑sender cooldown, computes the hop distance/RSSI/SNR diagnostics and finally sends a direct message back to the sender. 
The reply uses a friendly format such as “🎙️ Mic Check : 1 Hops away | RSSI –75 | SNR 9.4”. You can customize the wording or remove the emoji by editing the source code before building your firmware.

INSTALLATION AND COMPILATION

ReplyBot is not enabled by default in the Meshtastic firmware. To enable it, open the Variant.h file in the firmware source tree and add the line:

#undef MESHTASTIC_EXCLUDE_REPLYBOT


After you add this line, compile and flash the firmware for your board following the standard Meshtastic build instructions. 
If you wish to disable the module, remove the #undef directive.

CUSTOMIZATION

Developers can tweak the bot to fit their needs. The per‑sender cooldown periods are controlled by the constants REPLYBOT_DM_COOLDOWN_MS and REPLYBOT_LF_COOLDOWN_MS. 
You can adjust the reply text by editing the format string in the source code to include other statistics or to remove the emoji. 
The number of senders tracked for cooldown can also be changed by modifying REPLYBOT_COOLDOWN_SLOTS.

After making changes, rebuild and re‑flash your firmware.

TROUBLESHOOTING

If you’re not receiving replies, make sure the module is actually compiled in and enabled. Commands broadcast on secondary channels are ignored, so use the primary channel when broadcasting. 
Respect the cooldown periods between commands. Finally, check your mesh connectivity.

CONTRIBUTING

The Meshtastic project is community‑driven and welcomes contributions of all kinds. If you’d like to improve the ReplyBot or other modules, visit the Meshtastic firmware repository
to open issues or submit pull requests. Contributions to documentation, testing and user support are just as valuable as code.

License

Meshtastic firmware, including this module, is released under the GNU General Public License (GPL) version 3. See the LICENSE file in the firmware repository for full details.