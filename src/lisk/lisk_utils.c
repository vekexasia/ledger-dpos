#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "ed25519.h"
#include "lisk_utils.h"
#include "cx.h"
#include "os.h"


signContext_t signContext;

void getEncodedPublicKey(cx_ecfp_public_key_t *publicKey, uint8_t *encoded) {
  uint8_t i;
  for (i = 0; i < 32; i++) {
    encoded[i] = publicKey->W[64 - i];
  }
  if ((publicKey->W[32] & 1) != 0) {
    encoded[31] |= 0x80;
  }
}

void getPubKeyHash160(uint8_t *encodedPublicKey, uint8_t *encoded) {
  unsigned char hashedPkey[32];
  cx_hash_sha256(encodedPublicKey, 32, hashedPkey, 32);
  memmove(encoded, hashedPkey, ADDRESS_HASH_LENGTH);
}

void uint64_to_string(uint64_t input, char *out)
{
  uint8_t i = 0;
  uint64_t n = input;
  do
    i++;
  while ( n /= 10 );

  out[i] = '\0';
  n = input;
  do
    out[--i] = ( n % 10 ) + '0';
  while ( n /= 10 );
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

  // Derive Public Key
  derivePrivatePublic(account, &private_key, &public_key);
  // Encode public key
  getEncodedPublicKey(&public_key, account->encodedPublicKey);
  // Derive 20 bytes address from SHA256(pubkey)
  getPubKeyHash160(account->encodedPublicKey, account->addressHash);
  // Encode address into lisk32 format
  lisk_addr_encode(account->addressLisk32, LISK32_ADDRESS_PREFIX, account->addressHash, ADDRESS_HASH_LENGTH);

  return readCounter;
}

/**
 * DEPRECATED
 * Reads the packet, sets the signContext and patches the packet data values by skipping the header.
 * @param dataBuffer the  buffer to read from.
 * @return the amount of bytesRead
 */
uint32_t setSignContext(commPacket_t *packet) {
  // reset current result
  uint8_t tmp[256];
  memset(signContext.digest, 0, 32);

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
  memmove(tmp, packet->data + bytesRead, packet->length - bytesRead);
  memmove(packet->data, tmp, packet->length - bytesRead);
  packet->length -= bytesRead;

  return bytesRead;
}

uint32_t setReqContextForSign(commPacket_t *packet) {
  reqContext.signableContentLength = 0;
  uint32_t headerBytesRead = 0;

  // Extract Account
  headerBytesRead += extractAccountInfo(packet->data, &(reqContext.account));

  // Data Length
  reqContext.signableContentLength = lisk_read_u16(packet->data + headerBytesRead, 1, 0);
  headerBytesRead += 2;
  // Check signable content length if is correct
  if (reqContext.signableContentLength >= commContext.totalAmount) {
    THROW(0x6700); // INCORRECT_LENGTH
  }

  return headerBytesRead;
}

uint32_t setReqContextForGetPubKey(commPacket_t *packet) {
  reqContext.showConfirmation = packet->data[0];
  return extractAccountInfo(packet->data + 1, &reqContext.account);
}

void reset_contexts() {
  // Kill private key - shouldn't be necessary but just in case.
  memset(&private_key, 0, sizeof(private_key));

  memset(&reqContext, 0, sizeof(reqContext));
  memset(&txContext, 0, sizeof(txContext));
  memset(&commContext, 0, sizeof(commContext));
  memset(&commPacket, 0, sizeof(commPacket));

  // Allow restart of operation
  commContext.started = false;
  commContext.read = 0;
}