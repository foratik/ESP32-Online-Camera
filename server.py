from flask import Flask, request
import requests
from datetime import datetime

app = Flask(__name__)

TELEGRAM_TOKEN = "735345723:fhyuafskoifjwef"
CHAT_ID = "99999999"

@app.route('/send_photo', methods=['POST'])
def send_photo():
    if 'photo' not in request.files:
        return "No photo found", 400

    photo = request.files['photo']
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

    url = f"https://api.telegram.org/bot{TELEGRAM_TOKEN}/sendPhoto"
    files = {'photo': (photo.filename, photo.stream, 'image/jpeg')}
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