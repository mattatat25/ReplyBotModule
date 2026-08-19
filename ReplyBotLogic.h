#pragma once

#include <cctype>
#include <cstdint>
#include <cstring>

namespace replybot
{
enum class ReplyRoute : uint8_t { DIRECT, CHANNEL_FALLBACK };

inline bool tokenEqualsIgnoreCase(const char *input, const char *token)
{
    if (!input || !token)
        return false;

    while (*token) {
        if (!*input)
            return false;
        const unsigned char lhs = static_cast<unsigned char>(*input);
        const unsigned char rhs = static_cast<unsigned char>(*token);
        if (std::tolower(lhs) != std::tolower(rhs))
            return false;
        ++input;
        ++token;
    }

    return *input == '\0' || std::isspace(static_cast<unsigned char>(*input));
}

inline bool isSupportedCommand(const char *message)
{
    if (!message)
        return false;

    while (*message && std::isspace(static_cast<unsigned char>(*message)))
        ++message;

    return tokenEqualsIgnoreCase(message, "/ping") || tokenEqualsIgnoreCase(message, "/hello") ||
           tokenEqualsIgnoreCase(message, "/test");
}

inline ReplyRoute chooseReplyRoute(bool incomingDirectMessage, bool senderPublicKeyKnown)
{
    // Never turn an incoming private conversation into a public response. A direct
    // message always gets a direct reply attempt; current Meshtastic routing will
    // use PKI when required. Channel commands may safely fall back to the channel
    // when a private reply cannot yet be constructed.
    return (incomingDirectMessage || senderPublicKeyKnown) ? ReplyRoute::DIRECT : ReplyRoute::CHANNEL_FALLBACK;
}
} // namespace replybot
