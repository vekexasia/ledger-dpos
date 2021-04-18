#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include "io.h"
#include "os.h"

/**
 * Holds the responses
 */
struct response {
    uint8_t n;
    uint8_t *what[8];
    uint16_t whatLength[8];
} response;

void addToResponse(void *what, uint16_t length) {
  if (response.n == 8) {
    // OVERFLOW;
    THROW(0x9850); // Max Value Reached.
    return;
  }
  response.what[response.n] = what;
  response.whatLength[response.n] = length;
  response.n = response.n + 1;
}

void initResponse() {
  response.n = 0;
}

/**
 * Writes to output and returns number of bytes written
 * @param out the pointer to the output buffer
 * @return the num of bytes written
 */
unsigned int flushResponseToIO(void *out) {
  // Write how many infos toWrite
  os_memmove(out, &(response.n), 1);
  unsigned int total = 1;
  uint8_t i = 0;
  for (i = 0; i < response.n; i++) {
    // Write length.
    os_memmove(out + total, &response.whatLength[i], 2);
    total += 2;
    // Write data
    os_memmove(out + total, response.what[i], response.whatLength[i]);
    total += response.whatLength[i];
  }
  // Reset.
  initResponse();

  return total;
}

commContext_t commContext;
commPacket_t commPacket;

uint64_t readUint64LE(uint8_t * data) {
  uint32_t toRet = 0;
  uint8_t i;
  for (i = 0; i < 8; i++) {
    toRet |= ((uint64_t ) data[i]) << (8*i);
  }
  return toRet;
}

uint8_t encodeVarInt(uint64_t value, uint8_t * whereTo) {
  uint8_t tmp;
  if (value <= 0xfc) {
    os_memmove(whereTo, &value, 1);
    return 1;
  } else if (value <= 0xffff) {
    tmp = 0xfd;
    os_memmove(whereTo, &tmp, 1);
    os_memmove(whereTo+1, &value, 2);
    return 3;
  } else if (value <= 0xffffffff) {
    tmp = 0xfe;
    os_memmove(whereTo, &tmp, 1);
    os_memmove(whereTo+1, &value, 4);
    return 5;
  } else if (value <= 0xffffffffffffffff) {
    tmp = 0xff;
    os_memmove(whereTo, &tmp, 1);
    os_memmove(whereTo+1, &value, 8);
    return 9;
  }
  // Error
  return -1;
}
