#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <TurtleController.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// constants definitions
const int SCREEN_WIDTH = 128;
const int SCREEN_LENGTH = 64;
const int OLED_RESET = -1; // change this if the oled has a reset pin

const int joy_x_pin = 34;
const int joy_y_pin = 32;
const int joy_z_pin = 33;

const int but_a_pin = 13;
const int but_b_pin = 12;
const int but_x_pin = 14;
const int but_y_pin = 27;
const int but_r_pin = 19;
const int but_l_pin = 18;
const int but_s_pin = 5;

const int poll_rate = 64;
const float deadzone = 0.3;

const int LOGO_START_X = 96;
const int LOGO_START_Y = 32;

// various constant data the controller uses
const uint8_t broadcast_address[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

const int address_count = 15;

const char* team_names[address_count] PROGMEM = {
  "the grippaz",  // 1
  "team 27#",  // 2
  "Team 3",  // 3
  "Team 4",  // 4
  "Team 5",  // 5
  "Team 6",  // 6
  "Team 7",  // 7
  "Team 8",  // 8
  "Team 9",  // 9
  "Team 10", // 10
  "Team 11", // 11
  "Team 12", // 12
  "Team 13", // 13
  "Team 14", // 14
  "Team 15"  // 15
};

const uint8_t address_list[address_count][6] PROGMEM = {
  {0x08, 0xb6, 0x1f, 0xb8, 0x62, 0xc8}, // 1
  {0xc8, 0x2e, 0x18, 0xf2, 0x33, 0xec}, // 2
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // 3
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // 4 
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // 5 
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // 6
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // 7
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // 8
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // 9
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // 10
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // 11
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // 12
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // 13
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // 14
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // 15
};

const int LOGO_WIDTH = 32;
const int LOGO_HEIGHT = 32;
const unsigned char turtle_logo [] PROGMEM = { // sick af turtle logo
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xf0, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x38, 0xc1, 0xe0, 0x00, 0x09, 0x4b, 0xc0, 
	0x00, 0x01, 0x13, 0xc0, 0x00, 0x21, 0x97, 0x80, 0x00, 0x1f, 0xf0, 0x00, 0x00, 0x0f, 0xf9, 0x00, 
	0x00, 0x1f, 0xfe, 0x00, 0x00, 0x9f, 0xfc, 0xc0, 0x00, 0x7f, 0xfc, 0xc0, 0x00, 0x1f, 0xfc, 0x20, 
	0x07, 0x9f, 0xff, 0x80, 0x0f, 0x1f, 0xfc, 0x20, 0x0e, 0x7f, 0xf8, 0xe0, 0x06, 0x0f, 0xf8, 0x60, 
	0x00, 0x07, 0xe6, 0x60, 0x00, 0x04, 0x42, 0x60, 0x00, 0x09, 0x40, 0x20, 0x00, 0x03, 0x40, 0x20, 
	0x00, 0x07, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};



// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  float j1x;
  float j1y;
  bool j1z;

  bool butA;
  bool butB;
  bool butX;
  bool butY;
  bool butR;
  bool butL;
} struct_message;

// ESP-now stuff
esp_now_peer_info_t peerInfo[address_count];
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) { // callback function when data sent
  /*Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  */ // uncomment this if want debug message spam
}

// Create a struct_message called myData
struct_message myData;

// Create objects for controller and display
Controller controller;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_LENGTH, &Wire, OLED_RESET);

// function declarations
void readMacAddress();
void updateData();

int address_index = 0;
void setup() {
  Serial.begin(115200);
 

  WiFi.mode(WIFI_STA);
  WiFi.begin();

  // esp now setup
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    while(true){
      Serial.println("im killing myself");
    }
  }
 
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peers
  for(int i = 0; i < address_count; ++i){
    memcpy(peerInfo[i].peer_addr, address_list[i], 6); // copy address from list to peerInfo address
    peerInfo[i].channel = 0;  
    peerInfo[i].encrypt = false;
    
    // Add peer        
    if (esp_now_add_peer(&(peerInfo[i])) != ESP_OK){
      Serial.printf("Failed to add peer %d\n", i);
    }
    
  }
  

  // controller setup
  delay(1000);
  
  controller = Controller(joy_x_pin, joy_y_pin, joy_z_pin, but_a_pin, but_b_pin, but_x_pin, but_y_pin, but_r_pin, but_l_pin, but_s_pin);
 
  // OLED setup

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println("oled don't work, killing self"); // inform that suicide is imminent
    while(true) {
      Serial.println("im killing myself");
    } // kill self
  }
  Serial.println("oled setup successful");

  // use oled time
  display.clearDisplay();
  // display configs
  display.setTextSize(2);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.setTextWrap(true);
  // initial print to display
  display.setCursor(0, 0);     // Start at top-left corner
  display.printf("Team %d", address_index+1);
  display.println();
  display.printf(team_names[address_index]);
  display.drawBitmap(LOGO_START_X, LOGO_START_Y, turtle_logo, LOGO_WIDTH, LOGO_HEIGHT, 1); // draw sick af turtle logo
  display.display();

}


bool lastSwitchButtonState = false;

void loop() {
  // put your main code here, to run repeatedly:
  
  controller.controllerUpdate();

  // get all the controller data loaded
  updateData();
  
  // switch target devices if button gets pressed
  if(lastSwitchButtonState == false && controller.getS() == true){
    address_index = (address_index + 1) % address_count;
    
    /*Serial.printf("Team %d", address_index+1);
    Serial.println();
    Serial.printf(team_names[address_index]);*/ // uncomment if want debug message

    display.clearDisplay();  // display new team info
    display.setCursor(0,0);
    display.printf("Team %d", address_index+1  );
    display.println();
    display.printf(team_names[address_index]);
    display.drawBitmap(LOGO_START_X, LOGO_START_Y, turtle_logo, LOGO_WIDTH, LOGO_HEIGHT, 1); // draw sick af turtle logo
    display.display();
  }

  

  // fire in the hole
  esp_err_t result = esp_now_send(address_list[address_index], (uint8_t *) &myData, sizeof(myData));
  /*if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }*/ // uncomment if want debug messages

  // update last switch button state
  if(controller.getS() == true){
    lastSwitchButtonState = true;
  }
  else{
    lastSwitchButtonState = false;
  }
  delay((1.0f/poll_rate) * 1000); // delay according to polling rate

  
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

void updateData(){
  myData.j1x = controller.getJoy1X(deadzone);
  myData.j1y = controller.getJoy1Y(deadzone);
  myData.j1z = controller.getJoy1Z();

  myData.butA = controller.getA();
  myData.butB = controller.getB();
  myData.butX = controller.getX();
  myData.butY = controller.getY();
  myData.butR = controller.getR();
  myData.butL = controller.getL();
  Serial.printf("joy1: %.2f %.2f %d  buttons: %d %d %d %d %d %d\n", myData.j1x, myData.j1y,
    myData.j1z, myData.butA, myData.butB,
    myData.butX, myData.butY, myData.butR, myData.butL); // uncomment if want debug messages

}