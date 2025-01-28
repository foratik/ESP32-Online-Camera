from datetime import datetime

import requests
from flask import Flask, request

from decoder8b10b import EncDec_8B10B

app = Flask(__name__)

TELEGRAM_TOKEN = "735345723:fhyuafskoifjwef"
CHAT_ID = "99999999"


def decode_64b66b(encoded_data):
    """
    Decodes 64B/66B encoded data.
    Args:
        encoded_data (bytes): The 64B/66B encoded binary data.
    Returns:
        bytes: The decoded 64B data.
    """
    decoded_data = bytearray()
    bit_stream = ''.join(f'{byte:08b}' for byte in encoded_data)  # Convert to binary string

    i = 0
    while i < len(bit_stream) - 65:  # Process in 66-bit chunks
        sync_header = bit_stream[i:i + 2]
        block = bit_stream[i + 2:i + 66]

        if sync_header == "10":  # Data block
            decoded_data.extend(int(block[j:j + 8], 2) for j in range(0, 64, 8))
        elif sync_header == "01":  # Control block (ignore or handle as needed)
            pass
        else:
            raise ValueError("Invalid sync header in 64B/66B stream")

        i += 66

    return bytes(decoded_data)


@app.route('/send_photo', methods=['POST'])
def send_photo():
    if 'photo' not in request.files:
        return "No photo found", 400
    photo = request.files['photo']
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    raw_data = photo.read()
    try:
        decoded_data = EncDec_8B10B.dec_8b10b(raw_data)
    except ValueError as e:
        return f"Decoding failed: {str(e)}", 400

    # Send the decoded photo to Telegram
    url = f"https://api.telegram.org/bot{TELEGRAM_TOKEN}/sendPhoto"
    files = {'photo': ('decoded_photo.jpg', decoded_data, 'image/jpeg')}
    data = {
        'chat_id': CHAT_ID,
        'caption': f"Image captured at {timestamp} (GMT+3:30)"
    }

    response = requests.post(url, data=data, files=files)
    if response.status_code == 200:
        return "Photo sent", 200
    else:
        return f"Failed to send photo: {response.text}", 500


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
