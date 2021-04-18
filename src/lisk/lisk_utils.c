#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "ed25519.h"
#include "lisk_utils.h"
#include "cx.h"
#include "os.h"


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

void satoshiToString(uint64_t amount, char *out) {

  uint64_t partInt = amount / 100000000;
  uint64_t partDecimal = amount - (partInt*100000000l) ;

  uint8_t i = 0;

  // TODO: Calc the # of digits for partInt
  while(partInt > 0) {
    out[i++] = (uint8_t) (partInt % 10 + '0');
    partInt /=10;
  }

  // Swap elements
  uint8_t j = 0;
  uint8_t tmp;
  for (; j<i/2; j++) {
    tmp = out[j];
    out[j] = out[i-1-j];
    out[i-1-j] = tmp;
  }

  if (partDecimal > 0) {
    out[i++] = '.';
    uint32_t satoshi = 10000000;
    while (satoshi > 0 && partDecimal > 0) {
      out[i++] = (uint8_t) (partDecimal / satoshi + '0');
      partDecimal -= (partDecimal/satoshi) * satoshi;
      satoshi /= 10;
    }
  }
}

uint32_t extractAccountInfo(uint8_t *data, local_address_t *account) {
  uint32_t readCounter = 0;

  //PathLength
  account->pathLength = data[0];
  readCounter++;

  if(account->pathLength == 0) {
    return readCounter;
  }

  if(account->pathLength > MAX_BIP32_PATH) {
    THROW(INVALID_PARAMETER);
  }

  //Path
  bip32_buffer_to_array(data + 1, account->pathLength, account->path);
  readCounter += account->pathLength * 4;

  derivePrivatePublic(account, &private_key, &public_key);

  return readCounter;
}

/**
 * Reads the packet, sets the signContext and patches the packet data values by skipping the header.
 * @param dataBuffer the  buffer to read from.
 * @return the amount of bytesRead
 */
uint32_t setSignContext(commPacket_t *packet) {
  // reset current result
  uint8_t tmp[256];
  os_memset(signContext.digest, 0, 32);

  uint32_t bytesRead = extractAccountInfo(packet->data, &reqContext.account);
  derivePrivatePublic(&reqContext.account, &private_key, &public_key);
  signContext.signableContentLength = (*(packet->data + bytesRead)) << 8;
  signContext.signableContentLength += (*(packet->data + bytesRead + 1));
  if (signContext.signableContentLength >= commContext.totalAmount) {
    THROW(0x6700); // INCORRECT_LENGTH
  }
  bytesRead += 2;
  // Old requestPublicKey
  // signContext.reserved = *(packet->data + bytesRead);
  // bytesRead++;
  signContext.reserved = false;

  // clean up packet->data by removing the consumed content (sign context)
  os_memmove(tmp, packet->data + bytesRead, packet->length - bytesRead);
  os_memmove(packet->data, tmp, packet->length - bytesRead);
  packet->length -= bytesRead;

  return bytesRead;
}

uint32_t setReqContextForGetPubKey(commPacket_t *packet) {
  reqContext.showConfirmation = packet->data[0];
  return extractAccountInfo(packet->data + 1, &reqContext.account);
}