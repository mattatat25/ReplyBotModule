#include "../ReplyBotLogic.h"

#include <cassert>
#include <iostream>

int main()
{
    // Command parsing: documented commands, case-insensitivity and whitespace.
    assert(replybot::isSupportedCommand("/ping"));
    assert(replybot::isSupportedCommand("/PING"));
    assert(replybot::isSupportedCommand("  /Hello"));
    assert(replybot::isSupportedCommand("\t/test extra text"));
    assert(replybot::isSupportedCommand("/PiNg   extra"));

    // Reject partial command names and unrelated text.
    assert(!replybot::isSupportedCommand(nullptr));
    assert(!replybot::isSupportedCommand(""));
    assert(!replybot::isSupportedCommand("ping"));
    assert(!replybot::isSupportedCommand("/pingpong"));
    assert(!replybot::isSupportedCommand("/hello-world"));
    assert(!replybot::isSupportedCommand("/status"));

    // Privacy/routing matrix.
    assert(replybot::chooseReplyRoute(true, true) == replybot::ReplyRoute::DIRECT);
    assert(replybot::chooseReplyRoute(true, false) == replybot::ReplyRoute::DIRECT);
    assert(replybot::chooseReplyRoute(false, true) == replybot::ReplyRoute::DIRECT);
    assert(replybot::chooseReplyRoute(false, false) == replybot::ReplyRoute::CHANNEL_FALLBACK);

    std::cout << "ReplyBot logic tests passed\n";
    return 0;
}
