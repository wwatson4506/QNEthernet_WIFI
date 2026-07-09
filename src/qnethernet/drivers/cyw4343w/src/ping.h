// ping.h

#ifndef PING_H
#define PING_H

#include "event.h"

constexpr size_t pingDataSize   = 64; //32
constexpr uint8_t pingTTL       = 64;
constexpr uint16_t pingId       = 0x514E;

typedef struct {
    IPADDR dip;
    uint8_t   ttl = QNETHERNET_DEFAULT_PING_TTL;  /* Time to live */
    uint16_t   ident = QNETHERNET_DEFAULT_PING_ID; /* Identifier */
    uint16_t   seq = 0;                            /* Sequence number */
	const uint8_t* data = nullptr;
    size_t dataSize     = 0;
} PING_DATA;

class Ping {
public:
  // Function type for receiving ping replies. data may be NULL and dataSize
  // will be zero if there's no echo reply data.
  using replyf = std::function<void(const PING_DATA& reply)>;

  // Creates a new Ping object with no reply callback.
  Ping() = default;

  // Creates a new Ping object with the given reply callback.
  Ping(replyf f)
      : replyf_(f) {}

  // Sets the callback to the given function.
  void setCallback(replyf f) {
    replyf_ = f;
  }
  replyf replyf_;
private:

};
#endif // PING_H
