def decode_byte_array(input_bytes):
    """
    Decodes a 64B/66B encoded byte array into the original 64B data.

    Args:
        input_bytes (list[int]): Encoded byte array.

    Returns:
        list[int]: Decoded byte array (original 64B data).
    """
    if len(input_bytes) % 9 != 0:
        raise ValueError("Input length must be a multiple of 9 bytes for 64B/66B decoding.")

    decoded_data = []
    for i in range(0, len(input_bytes), 9):
        # Process each 9-byte block
        block = input_bytes[i:i + 9]

        # Convert to binary string
        bit_stream = ''.join(f'{byte:08b}' for byte in block)

        # Sync header validation
        sync_header = bit_stream[:2]
        if sync_header != "10":
            raise ValueError(f"Invalid sync header {sync_header} at block starting index {i}")

        # Extract 64 bits of payload
        payload = bit_stream[2:66]

        # Convert the 64 bits into 8 bytes
        decoded_chunk = [int(payload[j:j + 8], 2) for j in range(0, 64, 8)]
        decoded_data.extend(decoded_chunk)

    return decoded_data


def main():
    # Input data
    input_bytes = [
        0x80, 0x2F, 0xF9, 0xA1, 0x05, 0x38, 0x4A, 0x10,
        0x8B, 0x80, 0xDE, 0xA2, 0x9E, 0xDC, 0x3D, 0x06,
        0x20, 0xD3, 0x80, 0xCB, 0x8A, 0xBA, 0xD3, 0xF7,
        0x42, 0x70, 0x89, 0x80, 0x4F, 0x1B, 0x17, 0x17,
        0x5B, 0xC9, 0x29, 0x70, 0x80, 0xC8, 0x77, 0x90,
        0x00, 0xC2, 0x8F, 0x8B, 0x56, 0x80, 0x22, 0x13,
        0x7C, 0x5F, 0x1A, 0x43, 0x72, 0x08, 0x80, 0xE6,
        0x47, 0xB9, 0xDD, 0x88, 0x77, 0xAD, 0x0D, 0x80,
        0x63, 0x69, 0xEE, 0xBF, 0x37, 0xBB, 0xB5, 0x4A,
    ]
    # Decode the input
    decoded_output = decode_byte_array(input_bytes)
    # Print the decoded data
    print("Decoded Data:", decoded_output)


if __name__ == "__main__":
    main()
