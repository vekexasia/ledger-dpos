#include <stdbool.h>
#include <inttypes.h>
#ifndef IO_PROTOCOL
  #define IO_PROTOCOL
  /**
   * Adds a new buffer to the respose
   * @param what what to add
   * @param length
   */
  void addToResponse(void *what, uint16_t length);

  /**
   * Initializes the response object
   */
  void initResponse();

  /**
   * Writes to output and returns number of bytes written
   * @param out the pointer to the output buffer
   * @return the num of bytes written
   */
  unsigned int flushResponseToIO(void * out);

  typedef struct commContext_t {
      bool started;
      uint8_t command;
      uint32_t read;
      uint32_t totalAmount;
      short crc16;
  } commContext_t;

  typedef struct commPacket_t {
      uint8_t data[256];
      uint8_t length;
      bool first;
  } commPacket_t;

  extern commContext_t commContext;
  extern commPacket_t commPacket;

  uint64_t readUint64LE(uint8_t * data);
  uint8_t encodeVarInt(uint64_t value, uint8_t * whereTo);
#endif