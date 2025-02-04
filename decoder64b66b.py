def decode_64b66b(encoded):

    header = (encoded[0] >> 6) & 0b11
    if header != 0b01:
        raise ValueError("Invalid sync header")

    scrambled_data = 0
    scrambled_data |= (encoded[0] & 0x3F) << 58
    scrambled_data |= encoded[1] << 50
    scrambled_data |= encoded[2] << 42
    scrambled_data |= encoded[3] << 34
    scrambled_data |= encoded[4] << 26
    scrambled_data |= encoded[5] << 18
    scrambled_data |= encoded[6] << 10
    scrambled_data |= encoded[7] << 2
    scrambled_data |= (encoded[8] >> 6) & 0b11

    descrambler_state = (1 << 58) - 1
    original_data = 0

    for i in range(63, -1, -1):
        scrambled_bit = (scrambled_data >> i) & 1

        feedback = ((descrambler_state >> 38) & 1) ^ ((descrambler_state >> 57) & 1)

        original_bit = scrambled_bit ^ feedback

        descrambler_state = ((descrambler_state << 1) | scrambled_bit) & ((1 << 58) - 1)

        original_data = (original_data << 1) | original_bit

    return original_data


encoded_example = [0x48, 0xD1, 0x59, 0xE2, 0x48, 0xBF, 0x04, 0xA3, 0x80]
decoded = decode_64b66b(encoded_example)
print(f"Decoded data: {decoded:016X}")
