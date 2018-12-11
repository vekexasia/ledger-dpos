#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include "os.h"
#include "cx.h"
#include "dposutils.h"
#include "../../io.h"
#include "ed25519.h"

signContext_t signContext;

/**
 * Gets a bigendian representation of the usable publicKey
 * @param publicKey the raw public key containing both coordinated for the elliptic curve
 * @param encoded result holder
 */
void getEncodedPublicKey(cx_ecfp_public_key_t *publicKey, uint8_t *encoded) {
  uint8_t i;
  for (i = 0; i < 32; i++) {
    encoded[i] = publicKey->W[64 - i];
  }
  if ((publicKey->W[32] & 1) != 0) {
    encoded[31] |= 0x80;
  }
}


/**
 * Derive address uint64_t value from a byte array.
 * @param source source byte array where the next 8 bytes represent the address
 * @param rev whether or not to reverse the info. (publickey address derivation => true)
 * @return the encoded address in uint64_t data.
 */
uint64_t deriveAddressFromUintArray(uint8_t *source, bool rev) {

  uint8_t address[8];
  uint8_t i;
  for (i = 0; i < 8; i++) {
    address[i] = source[rev == true ? 7 - i : i];
  }

  uint64_t encodedAddress = 0;
  for (i = 0; i < 8; i++) {
    encodedAddress = encodedAddress << 8;
    encodedAddress += address[i];
  }

  return encodedAddress;
}


/**
 * Returns a string representation of the encoded Address
 * @param encodedAddress address to represent
 * @param output the output where the string representation will be returned
 * @return the length of the string representation.
 */
uint8_t deriveAddressStringRepresentation(uint64_t encodedAddress, char *output) {

  char brocca[22];
  uint8_t i = 0;
  while (encodedAddress > 0) {
    uint64_t remainder = encodedAddress % 10;
    encodedAddress = encodedAddress / 10;
    brocca[i++] = (char) (remainder + '0');
  }

  uint8_t total = i;
  for (i = 0; i < total; i++) {
    output[total - 1 - i] = brocca[i];
  }

  os_memmove(&output[total], ADDRESS_SUFFIX, ADDRESS_SUFFIX_LENGTH);
  output[total + ADDRESS_SUFFIX_LENGTH] = '\0'; // for strlen
  return (uint8_t) (total + ADDRESS_SUFFIX_LENGTH /*suffix*/);
}

/**
 * Derive address associated to the specific publicKey.
 * @param publicKey original publicKey
 * @return the encoded address.
 */
uint64_t deriveAddressFromPublic(cx_ecfp_public_key_t *publicKey) {
  uint8_t encodedPkey[32];

  getEncodedPublicKey(publicKey, encodedPkey);

  unsigned char hashedPkey[32];
  cx_hash_sha256(encodedPkey, 32, hashedPkey, 32);

  return deriveAddressFromUintArray(
    hashedPkey,
    true
  );
}


/**
 * Reads the packet, sets the signContext and patches the packet data values by skipping the header.
 * @param dataBuffer the  buffer to read from.
 * @return the amount of bytesRead
 */
uint32_t setSignContext(commPacket_t *packet) {
  // reset current result
  uint8_t tmp[256];
  os_memset(&signContext.digest, 0, 32);
  uint32_t bytesRead = derivePrivatePublic(packet->data, &signContext.privateKey, &signContext.publicKey);
  signContext.signableContentLength = (*(packet->data + bytesRead)) << 8;
  signContext.signableContentLength += (*(packet->data + bytesRead + 1));
  if (signContext.signableContentLength >= commContext.totalAmount) {
    THROW(0x6700); // INCORRECT_LENGTH
  }
  bytesRead += 2;
  signContext.reserved = *(packet->data + bytesRead);
  bytesRead++;

  // clean up packet->data by removing the consumed content (sign context)
  os_memmove(tmp, packet->data + bytesRead, packet->length - bytesRead);
  os_memmove(packet->data, tmp, packet->length - bytesRead);
  packet->length -= bytesRead;
  PRINTF("REmoved data %d %d %d", bytesRead, commContext.totalAmount, commContext.totalAmount-bytesRead);

  return bytesRead;
}