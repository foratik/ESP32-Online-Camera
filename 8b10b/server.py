from datetime import datetime
import io
from flask import Flask, request, send_file
from PIL import Image
from decoder8b10b import EncDec_8B10B  # Ensure this module correctly decodes 10b -> 8b

app = Flask(__name__)

def bytes_to_bitstream(raw_data):
    """ Convert raw byte data into a bitstream string. """
    return ''.join(f"{byte:08b}" for byte in raw_data)

@app.route('/send_photo', methods=['POST'])
def send_photo():
    # Check if we have raw binary data in the request body
    raw_data = request.form
    if not raw_data:
        return "No data found in the request", 400

    timestamp = datetime.now().strftime('%Y-%m-%d_%H-%M-%S')

    decoded_bytes = bytearray()
    # Convert to bitstream and decode
    for d in raw_data:
        split = d.split(' ')
        for s in split[:-1]:
            ctrl, decoded_byte = EncDec_8B10B.dec_8b10b(int(s, 16))
            decoded_bytes.append(decoded_byte)

    # Attempt to reconstruct the image
    try:
        image = Image.open(io.BytesIO(decoded_bytes))
    except Exception as e:
        return f"Image reconstruction failed: {str(e)}", 400

    # Save the image for verification
    image_path = f"received_image_{timestamp}.png"
    image.save(image_path)

    # Return the reconstructed image
    return send_file(image_path, mimetype='image/png')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
