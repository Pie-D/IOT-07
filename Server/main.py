from datetime import datetime
from flask import Flask, request
import firebase_admin
from firebase_admin import credentials, firestore
cred = credentials.Certificate(
    'D:/IOT-07/Server/credential/iot-nhom-7-d70d7-firebase-adminsdk-ojzsi-0e7aa020a5.json')

firebase_admin.initialize_app(cred)

db = firestore.client()

DayOfWeek = {
    2: 'Thứ Hai',
    3: 'Thứ Ba',
    4: 'Thứ Tư',
    5: 'Thứ Năm',
    6: 'Thứ Sáu',
    7: 'Thứ Bảy',
    8: 'Chủ Nhật'
}


def current_day_of_week():
    current_day_of_week = datetime.now().weekday()
    return DayOfWeek.get(current_day_of_week + 2)


def current_time():
    current_time = datetime.now().strftime("%H-%M")
    return current_time


def read_time_led_on_day(day):
    collections = db.collection("Plan").document(
        "wo-F6FyU_cajnKQ6-B6BWDfiPRHeWbBj").collections()
    for collection in collections:
        # print(collection.id + day)
        for doc in collection.stream():
            # print(f"{doc.id} => {doc.to_dict()}")
            time_values = doc.to_dict().get('Time', [])

        # Lặp qua danh sách thời gian trong từng tài liệu
            for time_value in time_values:
                time_on = time_value.get('ON', '')
                for x in collection.id:
                    print("-" + x + "-")
                for x in day:
                    print("-" + x + "-")
                print(len(day))
                if collection.id == day:
                    return time_on
    return "no"


if __name__ == '__main__':
    # collections = db.collection("Plan").document(
    #     "wo-F6FyU_cajnKQ6-B6BWDfiPRHeWbBj").collections()
    # for collection in collections:
    #     print(collection.id)
    #     for doc in collection.stream():
    #         print(f"{doc.id} => {doc.to_dict()}")
    #         time_values = doc.to_dict().get('Time', [])

    #     # Lặp qua danh sách thời gian trong từng tài liệu
    #         for time_value in time_values:
    #             time_on = time_value.get('ON', '')
    #             time_off = time_value.get('OFF', '')

    #             # In thông tin về thời gian bật/tắt
    #             print(f"Time ON: {time_on}, Time OFF: {time_off}")
    if ("Thứ Hai" == "Thứ Hai"):
        print("true")
    print(read_time_led_on_day("Thứ Hai"))
    # y = current_time()
    # print(y)
    # x = current_day_of_week()
    # print(x)
