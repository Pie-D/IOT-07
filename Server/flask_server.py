from flask import Flask
from flask import request, jsonify
from flask_cors import CORS, cross_origin
from datetime import datetime
import firebase_admin
import requests
from firebase_admin import credentials, firestore
app = Flask(__name__)
CORS(app)
app.config['CORS_HEADERS'] = 'Content-Type'

DayOfWeek = {
    2: 'Monday',
    3: 'Tuesday',
    4: 'Wednesday',
    5: 'Thursday',
    6: 'Friday',
    7: 'Saturday',
    8: 'Sunday',
}


def current_day_of_week():
    current_day_of_week = datetime.now().weekday()
    return DayOfWeek.get(current_day_of_week)


def current_time():
    current_time = datetime.now().strftime("%H-%M")
    return current_time


def get_time_schedule(led_id, status):
    day = current_day_of_week()  # lấy xem hôm nay là thứ mấy
    url = f'https://iot-nhom-7-d70d7-default-rtdb.firebaseio.com/Plan/wo-F6FyU_cajnKQ6-B6BWDfiPRHeWbBj/{day}/{led_id}/Time.json'
    response = requests.get(url)
    if response.status_code == 200:
        result = response.json()
    else:
        print('Failed to retrieve data. Status code:', response.status_code)
    for _, times in result.items():
        status_time = times.get(status, '')  # lay thoi gian bat tat trong ngay
        time = current_time()  # lay thoi gian hien tai
        if time == status_time:
            if status == 'ON':
                state = '1'
            else:
                state = '0'
            blynk_url_update = f'https://sgp1.blynk.cloud/external/api/update?token=wo-F6FyU_cajnKQ6-B6BWDfiPRHeWbBj&{led_id}={state}'
            response = requests.get(blynk_url_update)
        if response.status_code == 200:
            print('API đã được gửi thành công đến Blynk.')
            return True
        else:
            print('Failed to send API to Blynk. Status code:',
                  response.status_code)
            return False


@app.route('/update/<string:led_id>/<string:status>', methods=['POST', 'GET'])
@cross_origin(origin='*')
def update_led(led_id, status):
    print(led_id, status)
    if (get_time_schedule(led_id, status)):
        return {f'den {led_id}': f'cap nhat thanh cong {status}'}


if __name__ == '__main__':
    app.run(host='0.0.0.0', port='8888')
