from flask import Flask
import requests


def get_time_schedule(led_id, day, action, time):
    url = f'https://iot-nhom-7-d70d7-default-rtdb.firebaseio.com/Plan/wo-F6FyU_cajnKQ6-B6BWDfiPRHeWbBj/{day}/{led_id}/Time.json'
    response = requests.get(url)
    if response.status_code == 200:
        result = response.json()
    else:
        print('Failed to retrieve data. Status code:', response.status_code)
    for _, times in result.items():
        action_time = times.get(action, '')
        if time == action_time:
            if action == 'ON':
                status = '1'
            else:
                status = '0'
            blynk_url_update = f'https://sgp1.blynk.cloud/external/api/update?token=wo-F6FyU_cajnKQ6-B6BWDfiPRHeWbBj&{led_id}={status}'
            response = requests.get(blynk_url_update)
            # blynk_url_get = f'https://sgp1.blynk.cloud/external/api/isHardwareConnected?token=wo-F6FyU_cajnKQ6-B6BWDfiPRHeWbBj&{led_id}'
            # response = requests.get(blynk_url_get)
            # print(response.json())
        if response.status_code == 200:
            print('API đã được gửi thành công đến Blynk.')
            return True
        else:
            print('Failed to send API to Blynk. Status code:',
                  response.status_code)
            return False


# Sử dụng hàm
led_id = 'V2'
day = 'Monday'
action = 'ON'

result = get_time_schedule(led_id, day, action, '9-00')
print(result)
