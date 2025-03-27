#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <TurtleController.h>

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
const int pollRate = 50;

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  float j1x;
  float j1y;
  bool j1z;
  
  float j2x;
  float j2y;
  bool j2z;

  bool butA;
  bool butB;
  bool butX;
  bool butY;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) { // callback function when data sent
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}


const int j1x = 1;
const int j1y = 2;
const int j1z = 3;

const int j2x = 4;
const int j2y = 5;
const int j2z = 6;

const int butAPin = 7;
const int butBPin = 8;
const int butXPin = 9;
const int butYPin = 10;

Controller controller;

void readMacAddress();

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(); // might not work, was originally WiFi.STA.begin()

  // to print mac address (allegedly)
  /*Serial.print("[DEFAULT] ESP32 Board MAC Address: ");
  readMacAddress();*/

  // esp now setup
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6); // copy broadcastaddress to peerInfo address
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }


  // controller setup
  delay(1000);
  controller = Controller(j1x, j1y, j1z, j2x, j2y, j2z, butAPin, butBPin, butXPin, butYPin);

}

void loop() {
  // put your main code here, to run repeatedly:
  controller.controllerUpdate();

  // get all the controller data loaded
  myData.j1x = controller.getJoy1X();
  myData.j1y = controller.getJoy1Y();
  myData.j1z = controller.getJoy1Z();

  myData.j2x = controller.getJoy2X();
  myData.j2y = controller.getJoy2Y();
  myData.j2z = controller.getJoy2Z();

  myData.butA = controller.getA();
  myData.butB = controller.getB();
  myData.butX = controller.getX();
  myData.butY = controller.getY();

  // fire in the hole
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));


  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }

  delay((1.0f/pollRate) * 1000);
}

void readMacAddress(){
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("Failed to read MAC address");
  }
}