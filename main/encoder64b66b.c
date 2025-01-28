#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

void encode64b66b(uint8_t *input, size_t input_len, uint8_t *output, size_t *output_len) {
    size_t input_blocks = input_len / 8;  // Number of 64-bit blocks
    *output_len = input_blocks * 9;      // Each block expands to 9 bytes (66 bits)

    uint64_t scrambler_state = 0xFFFF; // Initial scrambler state
    for (size_t i = 0; i < input_blocks; i++) {
        // Read 64 bits from input
        uint64_t block = 0;
        for (int j = 0; j < 8; j++) {
            block = (block << 8) | input[i * 8 + j];
        }

        // Scramble the block
        uint64_t scrambled = block ^ scrambler_state;

        // Update scrambler state
        scrambler_state = ((scrambler_state << 1) ^ ((scrambled >> 63) & 1)) & 0xFFFFFFFFFFFFFFFF;

        // Write the sync header separately
        size_t output_offset = i * 9;
        output[output_offset] = 0b10000000; // Sync header: '10' in the top two bits

        // Write the scrambled data
        for (int j = 0; j < 8; j++) {
            output[output_offset + 1 + j] = (scrambled >> (56 - 8 * j)) & 0xFF;
        }
    }
}


int main() {
    const size_t SIZE = 64;

    // Declare the byte array
    uint8_t fbbuf[SIZE];

    // Seed the random number generator
    srand((unsigned int)time(NULL));

    // Fill the array with random values
    for (size_t i = 0; i < SIZE; i++) {
        fbbuf[i] = rand() % 256; // Random byte (0-255)
    }

    // Print the array
    printf("fbbuf contents:\n");
    for (size_t i = 0; i < SIZE; i++) {
        printf("%02X ", fbbuf[i]);
        if ((i + 1) % 8 == 0) printf("\n");
    }

    size_t encoded_len = 0;
    size_t output_buffer_len = SIZE * 9 / 8 + 9;  // Maximum possible size
    uint8_t *encoded_data = (uint8_t *)malloc(output_buffer_len);
    if (!encoded_data) {
        printf("Memory allocation failed\n");
        return 1;
    }

    encode64b66b(fbbuf, SIZE, encoded_data, &encoded_len);

    printf("\nEncoded data:\n");
    for (size_t i = 0; i < encoded_len; i++) {
        printf("%02X ", encoded_data[i]);
        if ((i + 1) % 8 == 0) printf("\n");
    }
    printf("\n");

    free(encoded_data);
    return 0;
}
