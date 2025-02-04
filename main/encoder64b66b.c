#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint64_t scrambler_state = (1ULL << 58) - 1;

void encode_64b66b(uint64_t data, uint8_t encoded[9]) {
    uint64_t scrambled_data = 0;
    
    for (int i = 63; i >= 0; i--) {
        uint8_t input_bit = (data >> i) & 1;
        
        uint8_t feedback = ((scrambler_state >> 38) & 1) ^ ((scrambler_state >> 57) & 1);
        
        uint8_t scrambled_bit = input_bit ^ feedback;
        
        scrambler_state = (scrambler_state << 1) | scrambled_bit;
        scrambler_state &= (1ULL << 58) - 1;
        
        scrambled_data = (scrambled_data << 1) | scrambled_bit;
    }

    memset(encoded, 0, 9);
    
    encoded[0] = 0x40 | ((scrambled_data >> 58) & 0x3F);
    
    encoded[1] = (scrambled_data >> 50) & 0xFF;
    encoded[2] = (scrambled_data >> 42) & 0xFF;
    encoded[3] = (scrambled_data >> 34) & 0xFF;
    encoded[4] = (scrambled_data >> 26) & 0xFF;
    encoded[5] = (scrambled_data >> 18) & 0xFF;
    encoded[6] = (scrambled_data >> 10) & 0xFF;
    encoded[7] = (scrambled_data >> 2)  & 0xFF;
    encoded[8] = (scrambled_data & 0x03) << 6;

    printf("Encoded 66-bit block: ");
    for (int i = 0; i < 9; i++) {
        printf("0x%02X, ", encoded[i]);
    }
    printf("\n");
}

int main() {
    uint64_t test_data = 0x2345678923456789;
    uint8_t encoded_block[9];
    
    encode_64b66b(test_data, encoded_block);
    
    return 0;
}
