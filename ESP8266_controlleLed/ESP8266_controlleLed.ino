#define BLYNK_PRINT Serial

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6RDtwgMBK"
#define BLYNK_TEMPLATE_NAME "TEST"
#define BLYNK_AUTH_TOKEN "wo-F6FyU_cajnKQ6-B6BWDfiPRHeWbBj"

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Blynk.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <ESP8266Firebase.h>
#include <TimeLib.h>
#include <unordered_map>
#include <time.h>
#include <FS.h>

#define Led D6
#define Led2 D2
#define Button D7
#define Button2 D3
#define ButtonReset D4
#define REFERENCE_URL "https://iot-nhom-7-d70d7-default-rtdb.firebaseio.com/"  // Your Firebase project reference url

bool previousButtonState = HIGH, previousButtonState2 = HIGH, previousButtonState3 = HIGH; 
int dat = 0, timezone = 7*3600;
String current_date = "";
Firebase firebase(REFERENCE_URL);
WiFiManager wifiManager;
std::unordered_map<int, int> number;
unsigned long previousMillis = 0;
const long interval = 1000;  
const String days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char *fileName = "data.txt";

const String getDayOfWeekString(String date) {
  int day, month, year;
  sscanf(date.c_str(), "%d-%d-%d", &day, &month, &year);

  // Adjust month and year for the algorithm
  if (month < 3) {
    month += 12;
    year--;
  }

  int k = year % 100;
  int j = year / 100;

  // Zeller's Congruence algorithm
  int dayOfWeek = (day + 13 * (month + 1) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;

  // Convert to 1-based index (Sunday is 0 in the algorithm)
  if (dayOfWeek == 0) {
    dayOfWeek = 7;
  }

  // Trả về tên của thứ
  return days[dayOfWeek - 1];
}

String readDataFromFile(int ledpin) {
  // Mở file để đọc
  File file = SPIFFS.open(String(ledpin) + fileName, "r");
  if (!file) {
    Serial.println("Không thể mở file để đọc");
    return "null";
  }
  // Đọc dữ liệu từ file và in ra Serial Monitor
  String data = file.readString();
  Serial.println("Dữ liệu đọc từ file: " + data);
  // Đóng file
  file.close();
  return data;
}

void writeDataToFile(String data, int ledpin) {
  // Mở hoặc tạo file để ghi
  File file = SPIFFS.open(String(ledpin) + fileName, "w");
  if (!file) {
    Serial.println("Không thể mở hoặc tạo file để ghi");
    return;
  }
  // Ghi dữ liệu vào file
  file.print(data);
  // Đóng file
  file.close();
  Serial.println("Dữ liệu đã được ghi :" + data);
}

String getCurrentDate() {
  // Lấy thời gian hiện tại
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  String ngay  = String(p_tm -> tm_mday);
  String thang = String(p_tm -> tm_mon + 1) ;
  String nam = String(p_tm -> tm_year + 1900);
  
  // Trả về chuỗi ngày giờ
  String date = ngay + "-" + thang +"-" + nam ;
  if (date == "1-1-1970"){
    delay(66);
    return getCurrentDate();
  }
  if (current_date != date){
    current_date = date;
    number[Led] = 0;
    number[Led2] = 0;
  }
  return getDayOfWeekString(date) + "/" + date;
}

void getSoLanBatAsync(int ledPin) {
  String currentDate = getCurrentDate();
  String dataPath = String(BLYNK_AUTH_TOKEN) + (ledPin == Led ? "/D6/" : "/D2/") + currentDate + "/ON";
  number[ledPin] = 0;
  String x = firebase.getString(dataPath);
  if (x != "ul") {
    Serial.println(x);
    char kyTu = ',';
    int soLan = 0;
    for (int i = 0; i < x.length(); i++) {
      if (x[i] == kyTu) {
        soLan++;
      }
    }
    number[ledPin] = soLan + 1;
    setDataTatAsync(ledPin, currentDate);
  }
}

void setDataTatAsync(int ledPin, String currentDate) {
  Serial.println("backup data!");
  String currentTime =  readDataFromFile(ledPin);
  firebase.setString(String(BLYNK_AUTH_TOKEN) + (ledPin == D6 ? "/D6/" : "/D2/") + currentDate + "/" + "OFF" + "/" + String(number[ledPin] - 1), currentTime);
  Blynk.virtualWrite(ledPin == Led ? V2 : V7, false);
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

String getCurrentMinute() {
  // Lấy thời gian hiện tại
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  String gio = String(p_tm -> tm_hour);
  String phut = String(p_tm -> tm_min);
  // Trả về chuỗi ngày giờ
  String Time = gio + ":" + phut;
  return Time;
}

void SetUpWifi(){
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  Serial.begin(9600);
  wifiManager.setConfigPortalTimeout(180);
  if (!wifiManager.autoConnect("NHÀ THÔNG MINH"))
  {
    Serial.println("KẾT NỐI THẤT BẠI ĐANG RESET ESP CHỜ.....");
    ESP.reset();//Nếu kết nối thất bại, thử kết nối lại bằng cách reset thiết bị
    delay(800);
  }
  else{
    configTime(timezone,dat,"pool.ntp.org","time.nist.gov");
    Serial.println("time :" + getCurrentDate());
    SetupBlynk();
    SetupFirebase();
  }
}

void SetupBlynk(){
  String ssid = wifiManager.getWiFiSSID();
  String password = wifiManager.getWiFiPass();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid.c_str(), password.c_str());
  Serial.println("Connected to WiFi:");
  pinMode(Led, OUTPUT);
  pinMode(Button, INPUT_PULLUP);
  pinMode(Led2, OUTPUT);
  pinMode(Button2, INPUT_PULLUP);  // Thiết lập chân nút bấm với chế độ INPUT_PULLUP
}

void SetupFirebase(){
  // Lấy thời gian hiện tại
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  String ngay  = String(p_tm -> tm_mday);
  String thang = String(p_tm -> tm_mon + 1) ;
  String nam = String(p_tm -> tm_year + 1900);
  // Trả về chuỗi ngày giờ
  current_date = ngay + "-" + thang +"-" + nam ;
  getSoLanBatAsync(Led);
  getSoLanBatAsync(Led2);
  Serial.println("Get result : " + String(number[Led2]) + " " + String(number[Led]));
}

void Blynk_write(int led, int status){
  digitalWrite(led, status ? HIGH : LOW);
  writeDataToFirebase(led, status);
}

BLYNK_WRITE(V2) {
  Blynk_write(Led, param.asInt());
}

BLYNK_WRITE(V7) {
  Blynk_write(Led2, param.asInt());
}

void writeDataToFirebase(int ledPin, int ledState){
  String currentDate = getCurrentDate(); // Thời gian hiện tại
  String currentTime = getCurrentTime();
  firebase.setString(String(BLYNK_AUTH_TOKEN) + (ledPin == D6 ? "/D6/" : "/D2/") + currentDate + "/" + (ledState ? "ON" : "OFF") + "/" + String(number[ledPin]), currentTime);
  number[ledPin] += ledState ? 0 : 1;
}

void SaveData(int ledPin){
  String currentTime = getCurrentTime();
  writeDataToFile(currentTime, ledPin);
}

void activeLed(int ledPin, bool status){
  digitalWrite(ledPin, status);
  int ledState = digitalRead(ledPin);
  Blynk.virtualWrite(ledPin == Led ? V2 : V7, ledState);
  writeDataToFirebase(ledPin, ledState);
}

void buttonPressed(int buttonPin, int ledPin, bool &state) {
  bool currentButtonState = digitalRead(buttonPin);
  if (currentButtonState != state) {
    bool finalButtonState = digitalRead(buttonPin);
    if (finalButtonState == LOW && finalButtonState != state) {
      activeLed(ledPin, !digitalRead(ledPin));
    } 
    state = finalButtonState;
  }
}

void buttonPressed(int buttonPin, bool &state) {
  bool currentButtonState = digitalRead(buttonPin);
  if (currentButtonState != state) {
    bool finalButtonState = digitalRead(buttonPin);
    if (finalButtonState == LOW && finalButtonState != state) {
      wifiManager.resetSettings();        
      SetUpWifi();
    } 
    state = finalButtonState;
  }
}

void setup() {
  // Khởi tạo SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("Không thể khởi tạo SPIFFS");
    return;
  }
  SetUpWifi();
}

void backupData(){
  // Lấy thời gian hiện tại
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Cập nhật thời điểm lần gọi cuối cùng
    // Kiểm tra và gọi SaveData cho Led1 và Led2
    if (digitalRead(Led)) {
      SaveData(Led);
    }
    if (digitalRead(Led2)) {
      SaveData(Led2);
    }
  }
}

void loop() {
  Blynk.run();
  buttonPressed(Button, Led, previousButtonState);
  buttonPressed(Button2, Led2, previousButtonState2);
  buttonPressed(ButtonReset, previousButtonState3);
  backupData();
}
