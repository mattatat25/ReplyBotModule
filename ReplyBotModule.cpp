#include "configuration.h"
#if !MESHTASTIC_EXCLUDE_REPLYBOT
/*
 * ReplyBotModule.cpp
 *
 * ReplyBot listens for /ping, /hello and /test on direct messages or the
 * primary channel and returns lightweight link diagnostics.
 *
 * Modern Meshtastic text DMs use PKI and intentionally refuse insecure legacy
 * DM fallback when the destination public key is unknown. ReplyBot therefore
 * checks whether a channel sender's public key is available before choosing a
 * private response. If the key is known, the normal Meshtastic router sends the
 * reply as a PKI DM. If it is not known, ReplyBot answers on the same primary
 * channel and tags the requesting node number so the diagnostic is still useful.
 *
 * Incoming DMs are never converted into public replies: they always receive a
 * direct reply attempt, preserving the privacy boundary of the original packet.
 */

#include "Channels.h"
#include "MeshService.h"
#include "NodeDB.h"
#include "ReplyBotLogic.h"
#include "ReplyBotModule.h"
#include "mesh/MeshTypes.h"
#include "main.h"

#include <Arduino.h>
#include <cstring>

namespace
{
struct ReplyBotCooldownEntry {
    uint32_t from = 0;
    uint32_t lastMs = 0;
};

static constexpr uint8_t REPLYBOT_COOLDOWN_SLOTS = 8;
static constexpr uint32_t REPLYBOT_DM_COOLDOWN_MS = 15 * 1000;
static constexpr uint32_t REPLYBOT_CHANNEL_COOLDOWN_MS = 60 * 1000;

ReplyBotCooldownEntry cooldowns[REPLYBOT_COOLDOWN_SLOTS];
uint8_t cooldownIndex = 0;

bool isRateLimited(uint32_t from, uint32_t cooldownMs)
{
    const uint32_t now = millis();
    for (auto &entry : cooldowns) {
        if (entry.from != from)
            continue;

        if ((uint32_t)(now - entry.lastMs) < cooldownMs)
            return true;

        entry.lastMs = now;
        return false;
    }

    cooldowns[cooldownIndex].from = from;
    cooldowns[cooldownIndex].lastMs = now;
    cooldownIndex = (cooldownIndex + 1) % REPLYBOT_COOLDOWN_SLOTS;
    return false;
}
} // namespace

ReplyBotModule::ReplyBotModule() : SinglePortModule("replybot", meshtastic_PortNum_TEXT_MESSAGE_APP)
{
    // Required so the module can observe slash commands sent to the primary
    // channel in addition to packets addressed directly to this node.
    isPromiscuous = true;
}

void ReplyBotModule::setup()
{
    // No protobuf configuration yet. If the module is compiled in, it is active.
}

bool ReplyBotModule::wantPacket(const meshtastic_MeshPacket *p)
{
    return p && p->decoded.portnum == ourPortNum;
}

ProcessMessage ReplyBotModule::handleReceived(const meshtastic_MeshPacket &mp)
{
    const uint32_t ourNode = nodeDB->getNodeNum();
    const bool isDM = (mp.to == ourNode);
    const bool isPrimaryBroadcast = (mp.channel == channels.getPrimaryIndex()) && isBroadcast(mp.to);

    if (!isDM && !isPrimaryBroadcast)
        return ProcessMessage::CONTINUE;

    if (mp.decoded.payload.size == 0)
        return ProcessMessage::CONTINUE;

    char command[260] = {};
    size_t commandLength = mp.decoded.payload.size;
    if (commandLength > sizeof(command) - 1)
        commandLength = sizeof(command) - 1;
    memcpy(command, mp.decoded.payload.bytes, commandLength);

    if (!isCommand(command))
        return ProcessMessage::CONTINUE;

    const uint32_t cooldownMs = isDM ? REPLYBOT_DM_COOLDOWN_MS : REPLYBOT_CHANNEL_COOLDOWN_MS;
    if (isRateLimited(mp.from, cooldownMs))
        return ProcessMessage::CONTINUE;

    const int hopsAway = getHopsAway(mp);
    int rssi = mp.rx_rssi;
    if (rssi > 0)
        rssi -= 200;
    const float snr = mp.rx_snr;

    char reply[112];
    snprintf(reply, sizeof(reply), "🎙️ Mic Check: %d Hops | RSSI %d | SNR %.1f", hopsAway, rssi, snr);

    const bool hasSenderKey = senderHasPublicKey(mp.from);
    const replybot::ReplyRoute route = replybot::chooseReplyRoute(isDM, hasSenderKey);

    if (route == replybot::ReplyRoute::DIRECT) {
        sendDirectReply(mp, reply);
    } else {
        // Public fallback is only used for a command that was already public.
        // Tag the requester because every node on the primary channel will see it.
        char channelReply[144];
        snprintf(channelReply, sizeof(channelReply),
                 "🎙️ For !%08x: %d Hops | RSSI %d | SNR %.1f", mp.from, hopsAway, rssi, snr);
        LOG_DEBUG("ReplyBot: no public key for 0x%08x; replying on primary channel", mp.from);
        sendChannelReply(mp, channelReply);
    }

    return ProcessMessage::CONTINUE;
}

bool ReplyBotModule::isCommand(const char *msg) const
{
    return replybot::isSupportedCommand(msg);
}

bool ReplyBotModule::senderHasPublicKey(uint32_t sender) const
{
#if !(MESHTASTIC_EXCLUDE_PKI)
    // Licensed (ham) mode deliberately does not use PKI for unicast text.
    if (owner.is_licensed)
        return true;

    if (!nodeDB)
        return false;

    meshtastic_NodeInfoLite_public_key_t key = {0, {0}};
    return nodeDB->copyPublicKey(sender, key) && key.size == 32;
#else
    (void)sender;
    // Builds without PKI do not need a public key to send a unicast reply.
    return true;
#endif
}

void ReplyBotModule::sendDirectReply(const meshtastic_MeshPacket &rx, const char *text)
{
    sendText(rx.from, rx.channel, text);
}

void ReplyBotModule::sendChannelReply(const meshtastic_MeshPacket &rx, const char *text)
{
    sendText(NODENUM_BROADCAST, rx.channel, text);
}

void ReplyBotModule::sendText(uint32_t destination, uint8_t channel, const char *text)
{
    if (!text)
        return;

    meshtastic_MeshPacket *packet = allocDataPacket();
    if (!packet) {
        LOG_WARN("ReplyBot: packet allocation failed");
        return;
    }

    packet->to = destination;
    packet->channel = channel;
    packet->want_ack = false;
    packet->decoded.want_response = false;

    size_t length = strlen(text);
    if (length > sizeof(packet->decoded.payload.bytes))
        length = sizeof(packet->decoded.payload.bytes);
    packet->decoded.payload.size = length;
    memcpy(packet->decoded.payload.bytes, text, length);

    service->sendToMesh(packet);
}
#endif // MESHTASTIC_EXCLUDE_REPLYBOT
