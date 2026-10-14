#ifndef PE1_AKAO_FILE_HEADER_H
#define PE1_AKAO_FILE_HEADER_H

#include <stddef.h>

/* Shared prefix of the sample and command buffers accepted by the validator.
 * Each buffer kind defines the remaining header and payload independently. */
typedef struct AkaoFilePrefix {
    unsigned int magic;
    unsigned char contents[];
} AkaoFilePrefix;

typedef char akao_file_prefix_size[(sizeof(AkaoFilePrefix) == 4) ? 1 : -1];
typedef char akao_file_prefix_contents[
    (offsetof(AkaoFilePrefix, contents) == 4) ? 1 : -1];

int Spu_ValidateSampleHeader(void *header);

#endif
