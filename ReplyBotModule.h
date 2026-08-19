#pragma once
#include "configuration.h"
#if !MESHTASTIC_EXCLUDE_REPLYBOT
#include "SinglePortModule.h"
#include "mesh/generated/meshtastic/mesh.pb.h"

class ReplyBotModule : public SinglePortModule
{
  public:
    ReplyBotModule();
    void setup() override;
    bool wantPacket(const meshtastic_MeshPacket *p) override;
    ProcessMessage handleReceived(const meshtastic_MeshPacket &mp) override;

  protected:
    bool isCommand(const char *msg) const;
    bool senderHasPublicKey(uint32_t sender) const;
    void sendDirectReply(const meshtastic_MeshPacket &rx, const char *text);
    void sendChannelReply(const meshtastic_MeshPacket &rx, const char *text);
    void sendText(uint32_t destination, uint32_t channel, const char *text);
};
#endif // MESHTASTIC_EXCLUDE_REPLYBOT
