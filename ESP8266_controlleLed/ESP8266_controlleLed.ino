

#define BLYNK_PRINT Serial

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6RDtwgMBK"
#define BLYNK_TEMPLATE_NAME "TEST"
#define BLYNK_AUTH_TOKEN "wo-F6FyU_cajnKQ6-B6BWDfiPRHeWbBj"


#include <Blynk.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <FirebaseESP8266.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <Wire.h>
#include <time.h>
#include <vector>
char ssid[] = "PeiD";
char pass[] = "12345678";
#define WIFI_SSID "PeiD"
#define WIFI_PASSWORD "12345678"

#define Led D6
#define Button D7
#define Led2 D2
#define Button2 D3
bool previousButtonState = LOW; 
bool previousButtonState2 = HIGH; // Trạng thái trước đó của nút bấm
int timezone = 7*3600;
int dat = 0;
int solanbat = 0;
int solantat = 0;
FirebaseData firebaseData;
String path = "/";
String dataPath = "wo-F6FyU_cajnKQ6-B6BWDfiPRHeWbBj/Led/1-11-2023";
FirebaseJson json;
FirebaseAuth auth;
FirebaseConfig config;
String getCurrentDate() {
  // Lấy thời gian hiện tại
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  String ngay  = String(p_tm -> tm_mday);
  String thang = String(p_tm -> tm_mon + 1) ;
  String nam = String(p_tm -> tm_year + 1900);
  
  // Trả về chuỗi ngày giờ
  String date = ngay + "-" + thang +"-" + nam ;
  return date;
}
String getSoLanTat(String datapath){
  if (Firebase.get(firebaseData, datapath)) {
      String x = firebaseData.stringData();
      Serial.println(x);
      char kyTu = ',';
      int soLan = 0;
      for(int i = 0; i < x.length(); i ++){
        if(x[i] == kyTu){
          soLan++;
        }
      }
      return String(soLan + 1);
  }
  return "0";
}
String getSoLanBat(String datapath){
  if (Firebase.get(firebaseData, datapath)) {
      String x = firebaseData.stringData();
      Serial.println(x);
      char kyTu = ',';
      int soLan = 0;
      for(int i = 0; i < x.length(); i ++){
        if(x[i] == kyTu){
          soLan++;
        }
      }
      return String(soLan + 1);
  }
  return "0";
}
String getCurrentTime() {
  // Lấy thời gian hiện tại
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  String gio = String(p_tm -> tm_hour);
  String phut = String(p_tm -> tm_min);
  String giay = String(p_tm -> tm_sec);
  // Trả về chuỗi ngày giờ
  String Time = gio + ":" + phut + ":" + giay;
  return Time;
}
void getAllDataFromFirebase(String datapath) {
  if (Firebase.get(firebaseData, datapath)) {
    if (firebaseData.dataType() == "json") {
      // Dữ liệu là dạng JSON, bạn có thể xử lý dữ liệu ở đây
      Serial.println("Data from Firebase: " + firebaseData.jsonString());
    } else {
      Serial.println("Data is not in JSON format");
    }
  } else {
    Serial.println("Failed to get data from Firebase");
  }

}
// 
int deleteDataFromFirebase(String datapath) {
  if (Firebase.get(firebaseData, datapath)) {
    if (firebaseData.dataType() == "json") {
      // Dữ liệu là dạng JSON, bạn có thể xử lý dữ liệu ở đây
      Firebase.set(firebaseData, datapath, NULL);
      Serial.println("Xoa thanh cong");
      return 1; // co the xoa
    } else {
      Serial.println("Da xoa roi");
      return 0; // da xoa 1 lan r
      
    }
    
  } else {
    Serial.println("Empty");
    return 2; // ko ton tai
  }

}
void setup() {
  Serial.begin(9600);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  pinMode(Led, OUTPUT);
  pinMode(Button, INPUT_PULLUP);
  pinMode(Led2, OUTPUT);
  pinMode(Button2, INPUT_PULLUP);  // Thiết lập chân nút bấm với chế độ INPUT_PULLUP
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: " + WiFi.localIP().toString());

  configTime(timezone,dat,"pool.ntp.org","time.nist.gov");
  Serial.println("connecting...");
  while(!time(nullptr)){
    Serial.println(".");
    delay(500);
  }
  Serial.println("time response ok...");

  config.database_url = "https://iot-nhom-7-d70d7-default-rtdb.firebaseio.com/";
  config.signer.tokens.legacy_token = "QNZWTt0SoQivgDeFSL5imCXQxGr3gwMEl9Cq49O2";
  
  Firebase.begin(&config, &auth);
  getAllDataFromFirebase(dataPath);
  deleteDataFromFirebase(dataPath);
}

BLYNK_WRITE(V2) {
  int button = param.asInt();
  if (button == 1) {
    digitalWrite(Led, HIGH);
  } else {
    digitalWrite(Led, LOW);
  }
}
BLYNK_WRITE(V7) {
  int button = param.asInt();
  if (button == 1) {
    digitalWrite(Led2, HIGH);
  } else {
    digitalWrite(Led2, LOW);
  }
}

void buttonPressed(int buttonPin, int ledPin, bool &state) {
  bool currentButtonState = digitalRead(buttonPin);

  if (currentButtonState != state) {
    delay(50);

    bool finalButtonState = digitalRead(buttonPin);

    if (finalButtonState == LOW && finalButtonState != state) {
      String currentDate = getCurrentDate(); // Thời gian hiện tại
      String currentTime = getCurrentTime();
      int firstDashIndex = currentDate.indexOf('-');
      int secondDashIndex = currentDate.indexOf('-', currentDate.indexOf('-') + 1);
      String dayStr = "";  // Tìm vị trí của dấu gạch đầu tiên
      String monthStr = "";
      String yearStr = "";
      int dayInt;
      int monthInt;
      int yearInt;
      dayStr = currentDate.substring(0, firstDashIndex);
      monthStr = currentDate.substring(firstDashIndex + 1, secondDashIndex);
      yearStr = currentDate.substring(secondDashIndex + 1);  // Lấy chuỗi con từ sau dấu gạch thứ hai đến hết chuỗi
      dayInt = dayStr.toInt();
      monthInt = monthStr.toInt();
      yearInt = yearStr.toInt();

      if(dayInt == 2){
        if(monthInt != 1){
          monthInt = monthInt - 1;
        } else {
          monthInt = 12;
          yearInt = yearInt - 1;
        }
        int count = 0;
        for(int i = 31 ; i > 0 ; i --){
          String x = String(BLYNK_AUTH_TOKEN) + "/Led/" + String(i) + "-" + String(monthInt) + "-" + String(yearInt);
          if(deleteDataFromFirebase(x) % 2 == 0){
            count ++;
          }
          if(count == 4 ) break;
        }
      }


      String x = String(BLYNK_AUTH_TOKEN) + "/Led/" + currentDate ;
      String soLanBat = getSoLanBat(x + "/ON");
      String soLanTat = getSoLanTat(x + "/OFF");
      Serial.println(x); 
      Firebase.setString(firebaseData, path + BLYNK_AUTH_TOKEN + "/" + "Led" +"/" + currentDate + "/" + (digitalRead(ledPin) ? "OFF" : "ON") + "/" + (digitalRead(ledPin) ? soLanTat : soLanBat) , currentTime);
      Serial.println((digitalRead(ledPin) ? "OFF" : "ON"));
      digitalWrite(ledPin, !digitalRead(ledPin));
      int ledState = digitalRead(ledPin);

      if (ledPin == Led) {
        Blynk.virtualWrite(V2, ledState);
      } else if (ledPin == Led2) {
        Blynk.virtualWrite(V7, ledState);
        
      }
    } 

    state = finalButtonState;
  }
}

// void buttonPressed(int buttonPin, int ledPin, bool &state) {
//   bool currentButtonState = digitalRead(buttonPin);

//   if (currentButtonState != state) {
//     delay(50);

//     bool finalButtonState = digitalRead(buttonPin);

//     if (finalButtonState == LOW && finalButtonState != state) {
//       digitalWrite(ledPin, !digitalRead(ledPin));
//       int ledState = digitalRead(ledPin);

//       // Gửi dữ liệu lên Firebase Realtime Database
//       String ledName = (ledPin == Led) ? "led1" : "led2"; // Tên đèn tương ứng
//       // String currentTime = String(millis()); // Thời gian hiện tại
//       // String data = currentTime + "_" + ledName + "_" + (ledState ? "ON" : "OFF");
//       // Firebase.setString(firebaseData, path + "/data", data);

//       if (ledPin == Led) {
//         Blynk.virtualWrite(V2, ledState);
//       } else if (ledPin == Led2) {
//         Blynk.virtualWrite(V7, ledState);
//       }
//     }

//     state = finalButtonState;
//   }
// }

void loop() {
  Blynk.run();
  buttonPressed(Button, Led, previousButtonState);
}
