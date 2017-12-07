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

