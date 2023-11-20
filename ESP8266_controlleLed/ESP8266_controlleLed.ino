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
#include <FirebaseESP8266.h>
#include <TimeLib.h>
#include <unordered_map>
#include <time.h>
#include <FS.h>
#include <Ticker.h>

#define Led D6
#define Led2 D2
#define Button D7
#define Button2 D3
#define ButtonReset D4

bool previousButtonState = HIGH, previousButtonState2 = HIGH, previousButtonState3 = HIGH, planClick = HIGH; 
int dat = 0, timezone = 7*3600;
String path = "/", current_date = "", ngay, thang, nam, date, planOn[4], planOff[4];
FirebaseJson json;
FirebaseData firebaseData;
FirebaseJsonData firebaseJsonData;
FirebaseAuth auth;
FirebaseConfig config;
WiFiManager wifiManager;
std::unordered_map<int, int> number;
const String days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char *fileName = "data.txt";
Ticker reconnectTimer;


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
  Serial.println("Dữ liệu đã được ghi vào file " + String(ledpin) + ":" + data);
}

String getCurrentDate() {
  // Lấy thời gian hiện tại
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  ngay  = String(p_tm -> tm_mday);
  thang = String(p_tm -> tm_mon + 1) ;
  nam = String(p_tm -> tm_year + 1900);
  
  // Trả về chuỗi ngày giờ
  date = ngay + "-" + thang +"-" + nam ;
  if (date == "1-1-1970"){
    delay(66);
    return getCurrentDate();
  }
  if (current_date != date){
    resetDay(date);
  }
  return date;
}

String connectCurrentDate() {
  // Lấy thời gian hiện tại
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  ngay  = String(p_tm -> tm_mday);
  thang = String(p_tm -> tm_mon + 1) ;
  nam = String(p_tm -> tm_year + 1900);
  
  // Trả về chuỗi ngày giờ
  date = ngay + "-" + thang +"-" + nam ;
  if (date == "1-1-1970"){
    delay(66);
    return getCurrentDate();
  }
  if (current_date != date){
    current_date = date;
  }
  return date;
}

void resetDay(String date){
  Firebase.setString(firebaseData, path + String(BLYNK_AUTH_TOKEN) + "/D6/" + getDayOfWeekString(current_date) + path + "onetime", NULL);
  Firebase.setString(firebaseData, path + String(BLYNK_AUTH_TOKEN) + "/D2/" + getDayOfWeekString(current_date) + path + "onetime", NULL);
  current_date = date;
  number.clear();
  UpdatePlan();
}

void getSoLanBatAsync(int ledPin) {
  String currentDate = getCurrentDate();
  String datapath = path + String(BLYNK_AUTH_TOKEN) + (ledPin == Led ? "/D6/" : "/D2/") + getDayOfWeekString(currentDate) + path + currentDate;
  int result = 0;
  Serial.println(datapath);
  bool status = Firebase.get(firebaseData, datapath);
  if (status) {
    String x = firebaseData.stringData();
    char kyTu = '}';
    for (int i = 0; i < x.length(); i++) {
      if (x[i] == kyTu) {
        result++;
      }
    }
  }
  number[ledPin] = result;
  if (status){
    setDataTatAsync(ledPin, currentDate);
  }
}

void setDataTatAsync(int ledPin, String currentDate) {
  Serial.println("backup data!");
  String currentTime =  readDataFromFile(ledPin);
  Firebase.setString(firebaseData, path + String(BLYNK_AUTH_TOKEN) + (ledPin == D6 ? "/D6/" : "/D2/") + getDayOfWeekString(currentDate) + path + currentDate + path + String(number[ledPin] - 1) + path + "OFF", currentTime);
  Blynk.virtualWrite(ledPin == Led ? V2 : V7, false);
}

String getCurrentTime() {
  // Lấy thời gian hiện tại
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  String gio = String(p_tm -> tm_hour);
  String phut = String(p_tm -> tm_min);
  String giay = String(p_tm -> tm_sec);
  // Đảm bảo rằng chuỗi phút và giây có hai chữ số
  if (phut.length() == 1) {
    phut = "0" + phut;
  }
  if (giay.length() == 1) {
    giay = "0" + giay;
  }
  // Trả về chuỗi ngày giờ
  String Time = gio + ":" + phut + ":" + giay;
  return Time;
}

void checkCurrentPlanTime() {
  String date = getCurrentDate();
  if (date != current_date){
    resetDay(date);
    return;
  }
  // Lấy thời gian hiện tại
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  String gio = String(p_tm -> tm_hour);
  String phut = String(p_tm -> tm_min);
  if (phut.length() == 1) {
    phut = "0" + phut;
  }
  String Time = gio + ":" + phut;
  for (int i = 0; i < 4; i++) {
    int index = planOn[i].indexOf(Time);
    int ledPin = (i == 0 || i == 2) ? Led : Led2;
    if (index != -1) {
      planOn[i].replace(Time, "");
      if (!digitalRead(ledPin)){
        activeLed(ledPin, true);
      }
    }
    index = planOff[i].indexOf(Time);
    if (index != -1) {
      planOff[i].replace(Time, "");
      if (digitalRead(ledPin)){
        activeLed(ledPin, false);
      }
    }
  }
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
    Serial.println("time :" + connectCurrentDate());
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
  config.database_url = //"https://iot-unity-5a64f-default-rtdb.asia-southeast1.firebasedatabase.app/";
  "https://iot-nhom-7-d70d7-default-rtdb.firebaseio.com/";
  config.signer.tokens.legacy_token = //"XWYmXKU9f5egoHYHVhtqGn6H4wZJEirWKVtTX1Yv";
   "QNZWTt0SoQivgDeFSL5imCXQxGr3gwMEl9Cq49O2";
  Firebase.begin(&config, &auth);
  getSoLanBatAsync(Led);
  getSoLanBatAsync(Led2);
  Serial.println("Get result : " + String(number[Led2]) + " " + String(number[Led]));
  UpdatePlan();
  for (int i = 0; i < 4; i++) {
    Serial.println(String(i) + " ON : " + planOn[i] + " OFF: " + planOff[i]);
  }
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

BLYNK_WRITE(V1) {
  if (param.asInt()){
    UpdatePlan();
  }
}

void writeDataToFirebase(int ledPin, bool status){
  String currentDate = getCurrentDate(); // Thời gian hiện tại
  String currentTime = getCurrentTime();
  Firebase.setString(firebaseData, path + String(BLYNK_AUTH_TOKEN) + (ledPin == D6 ? "/D6/" : "/D2/") + getDayOfWeekString(currentDate) + path + currentDate + path + String(number[ledPin]) + path + (status ? "ON" : "OFF"), currentTime);
  number[ledPin] += status ? 0 : 1;
}

void SaveData(int ledPin){
  String currentTime = getCurrentTime();
  writeDataToFile(currentTime, ledPin);
}

void activeLed(int ledPin, bool status){
  digitalWrite(ledPin, status);
  Blynk.virtualWrite(ledPin == Led ? V2 : V7, status);
  writeDataToFirebase(ledPin, status);
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

void GetDataPlan(int ledPin, String type){
  String currentDay = getDayOfWeekString(getCurrentDate());
  String datapath = path + String(BLYNK_AUTH_TOKEN) + (ledPin == Led ? "/D6/" : "/D2/") + currentDay + path + type + path;
  int result = 0;

  // Determine the index based on the ledPin and type
  int index = (ledPin == Led ? 0 : 1) + (type == "onetime" ? 2 : 0);

  if (Firebase.get(firebaseData, datapath + "ON")) {
    // Convert const String to C-style string and create a new String
    planOn[index] = firebaseData.stringData();
  }
  if (Firebase.get(firebaseData, datapath + "OFF")) {
    // Convert const String to C-style string and create a new String
    planOff[index] = firebaseData.stringData();
  }
}

void UpdatePlan(){
  planOn[4] = {""};
  planOff[4] = {""};
  GetDataPlan(Led, "alltime");
  GetDataPlan(Led, "onetime");
  GetDataPlan(Led2, "alltime");
  GetDataPlan(Led2, "onetime");
  Serial.println("Update plan successfully!");
}

void updateTime(){
  if (digitalRead(Led)) {
      SaveData(Led);
    }
  if (digitalRead(Led2)) {
    SaveData(Led2);
  }
}

void setup() {
  // Khởi tạo SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("Không thể khởi tạo SPIFFS");
    return;
  }
  SetUpWifi();
  reconnectTimer.attach(1, updateTime);
}

void loop() {
  Blynk.run();
  buttonPressed(Button, Led, previousButtonState);
  buttonPressed(Button2, Led2, previousButtonState2);
  buttonPressed(ButtonReset, previousButtonState3);
  checkCurrentPlanTime();
}
