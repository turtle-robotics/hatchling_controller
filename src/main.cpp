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
const float deadzone = 0.17;

const int LOGO_START_X = 96;
const int LOGO_START_Y = 32;

// various constant data the controller uses
const uint8_t broadcast_address[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

const int address_count = 21;

const char* team_names[address_count] PROGMEM = {
  "the grippaz",  // 1
  "team 27#",  // 2
  "Team Amazing",  // 3
  "Mort",  // 4
  "boooo", // 5
  "bunta", // 6
  "kirchoff", // 7
  "Yggdrasil", // 8
  "ethan h", // 9
  "Geodude", // 10
  "BUG", // 11
  "The Goblins", // 12
  "TURTLE Quad Squad", // 13
  "K-emi", // 14
  "Oddish", // 15
  "o4-mini", // 16
  "Nice Rack and Pinion", // 17
  "top dawg", // 18
  "hjg poggiers", // 19
  "Logan", // 20
  "BROADCAST" // 21
};

const uint8_t address_list[address_count][6] PROGMEM = {
  {0x08, 0xb6, 0x1f, 0xb8, 0x62, 0xc8}, // 1
  {0xc8, 0x2e, 0x18, 0xf0, 0x12, 0x74}, // 2
  {0x08, 0xb6, 0x1f, 0xb8, 0x04, 0x7c}, // 3
  {0x08, 0xb6, 0x1f, 0xb6, 0x07, 0x34}, // 4
  {0xc8, 0x2e, 0x18, 0xf2, 0x34, 0x68}, // 5
  {0x24, 0xdc, 0xc3, 0x44, 0xff, 0xd8}, // 6 
  {0xc8, 0x2e, 0x18, 0xef, 0xe5, 0x54}, // 7 
  {0x08, 0xb6, 0x1f, 0xb8, 0xc8, 0xcc}, // 8 
  {0x08, 0xb6, 0x1f, 0xb9, 0x07, 0xac}, // 9 
  {0x08, 0xb6, 0x1f, 0xb9, 0x06, 0x44}, // 10
  {0xc8, 0x2e, 0x18, 0xef, 0xe5, 0x54}, // 11
  {0xc8, 0x2e, 0x18, 0xf1, 0x8d, 0x04}, // 12
  {0x24, 0xdc, 0xc3, 0x44, 0xf6, 0x30}, // 13
  {0x08, 0xb6, 0x1f, 0xb7, 0xfe, 0x74}, // 14
  {0xc8, 0x2e, 0x18, 0xf2, 0x32, 0xb0},	// 15
  {0xc8, 0x2e, 0x18, 0xf0, 0x72, 0x6c}, // 16
  {0x08, 0xb6, 0x1f, 0xb8, 0x2b, 0xe8}, // 17
  {0xc8, 0x2e, 0x18, 0xf2, 0x34, 0xb4}, // 18
  {0xc8, 0x2e, 0x18, 0xef, 0xfd, 0xd0}, // 19
  {0x08, 0xb6, 0x1f, 0xb8, 0x61, 0xbc}, // 20
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}  // 21  
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

// enum to track state declaration and increment operator
#pragma region state_enum_stuff
// Controller State enum
enum State{
  SEND_MODE,
  DEBUG_MODE
};
State& operator++(State& currentState){
  switch(currentState) {
    case SEND_MODE : return currentState = DEBUG_MODE;
    case DEBUG_MODE : return currentState = SEND_MODE;
    default : return currentState;
  }
  assert(false); // this should be unreachable, but if by some "miracle" the code makes it here it'll throw an error
  return currentState; // this is just to prevent the compiler for yelling at you

}

State operator++(State& currentState, int)
{
  State tmp(currentState);
  ++currentState;
  return tmp;
}
#pragma endregion state_enum_stuff

// ESP-now stuff
esp_now_peer_info_t peerInfo[address_count];
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) { // callback function when data sent
  /*Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  */ // uncomment this if want debug message spam
}

// Create a struct_message called controllerData
struct_message controllerData = {0, 0, 0, 0, 0, 0, 0, 0, 0};

// create an object to store last cycle's controllerData so we can see button pulses
struct_message lastControllerData = {0, 0, 0, 0, 0, 0, 0, 0, 0};;

// Create objects for controller and display
Controller controller;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_LENGTH, &Wire, OLED_RESET);

#pragma region function_declarations
// function declarations
void readMacAddress();
void updateData();

void drawDefaults(); // just has the following functions in 1
void drawTeamName();
void drawTurtleLogo();
void drawControllerBorders();

void drawControllerState(u_int startX = 30, u_int startY = 32);
void drawJoystickState(u_int startX, u_int startY);
void drawButtonState(u_int startX, u_int startY);

void sendingModeOperations();
void debugModeOperations();
// function to only return true on rising edge
bool getButtonRisingEdge(bool currentVal, bool oldVal);
#pragma endregion function_declarations

// runtime variables
int currentAddressIndex = 0;
bool lastSwitchButtonState = false;
enum State currentState = SEND_MODE;
bool flipX = true;
bool flipY = true;

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
  for(int i = 0; i < 20; ++i){
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
  display.setTextSize(2);      // 2x size of normal font
  display.setTextColor(WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.setTextWrap(true);
  
  // initial print to display
  drawDefaults();
  drawControllerState();
  display.display();

}



bool switchButtonPressed = false; // just so that we dont have to do controller.getS() a million times since that might reread the voltage every time, don't know honestly

void loop() {
  // loop code for stuff like button debouncing
  controller.controllerUpdate();
  // get all the updated controller data loaded
  updateData();
  switchButtonPressed = controller.getS();
  
  // switch device state if switch button gets pressed
  if(getButtonRisingEdge(switchButtonPressed, lastSwitchButtonState)){ // this is so we only switch once per button press
    ++currentState; // swap states
  }
  
  switch (currentState) // choose what to do based on current state
  {
  case SEND_MODE: sendingModeOperations(); break;
  case DEBUG_MODE: debugModeOperations(); break;
  default: break; // do nothing, this should be impossible to ever happen tho
  }
  

  

  // update last button states
  lastControllerData = controllerData;
  lastSwitchButtonState = switchButtonPressed;

  // delay according to polling rate
  delay((1.0f/poll_rate) * 1000);

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
  controllerData.j1x = controller.getJoy1X(deadzone) * ((flipX) ? -1 : 1); // flip x axis if flipX is true
  controllerData.j1y = controller.getJoy1Y(deadzone) * ((flipY) ? -1 : 1); // flip y axis if flipY is true
  controllerData.j1z = controller.getJoy1Z();

  controllerData.butA = controller.getA();
  controllerData.butB = controller.getB();
  controllerData.butX = controller.getX();
  controllerData.butY = controller.getY();
  controllerData.butR = controller.getR();
  controllerData.butL = controller.getL();
  Serial.printf("joy1: %.2f %.2f %d  buttons: %d %d %d %d %d %d\n", controllerData.j1x, controllerData.j1y,
    controllerData.j1z, controllerData.butA, controllerData.butB,
    controllerData.butX, controllerData.butY, controllerData.butR, controllerData.butL); // uncomment if want debug messages

}

#pragma region drawing_functions

void drawDefaults(){
  drawTeamName();
  drawTurtleLogo();
  drawControllerBorders();
}

void drawTeamName(){
  display.setCursor(0,0);
  display.printf("#%d:%s", currentAddressIndex+1, team_names[currentAddressIndex]);
}

void drawTurtleLogo(){
  display.drawBitmap(LOGO_START_X, LOGO_START_Y, turtle_logo, LOGO_WIDTH, LOGO_HEIGHT, 1); // draw sick af turtle logo
}

void drawControllerState(u_int startX, u_int startY){
  drawJoystickState(startX, startY);
  drawButtonState(startX + 31, startY);
  drawControllerBorders();
}

void drawJoystickState(u_int startX, u_int startY){
  // draw joystick state

  // some settings
  const u_int regionSize = 31;
  const u_int circleSize = 2;
  // used later
  const u_int maxDist = ((regionSize - 4)/2) - circleSize; // the minus 4 is for a nice 2 pixel border around the region just for spacing

  // calculate coordinates
  u_int CENTER_X = startX + regionSize/2;
  u_int CENTER_Y = startY + regionSize/2;

  u_int xCoord = CENTER_X + controllerData.j1x * maxDist;
  u_int yCoord = CENTER_Y + controllerData.j1y * maxDist;

  // draw
  display.fillCircle(CENTER_X + lastControllerData.j1x * maxDist, CENTER_Y + lastControllerData.j1y * maxDist, circleSize, BLACK); // wipe last
  if(controllerData.j1z)
    display.fillCircle(xCoord, yCoord, 2, WHITE);
  else
    display.drawCircle(xCoord, yCoord, 2, WHITE);

}

void drawButtonState(u_int startX, u_int startY){
  // some settings
  const u_int regionXSize = 35;
  const u_int regionYSize = 32;
  
  const u_int buttonRadius = 2;

  // calculate coordinates
  u_int centerX = startX + regionXSize/2;
  u_int centerY = startY + regionYSize/2 - regionYSize/13;

    // L button
    u_int LButtonStartX = centerX - regionXSize/5 - 3;
    u_int LButtonStartY = centerY - regionYSize/5 - 2;

    // R button
    u_int RButtonStartX = centerX + regionXSize/5 - 3;
    u_int RButtonStartY = centerY - regionYSize/5 - 2;

    // A button
    u_int AButtonStartX = centerX;
    u_int AButtonStartY = centerY + regionXSize/17 + regionYSize/3;

    // B button
    u_int BButtonStartX = centerX + regionXSize/4;
    u_int BButtonStartY = centerY + (regionXSize/17 + regionYSize/3)/2 + 1;

    // X button
    u_int XButtonStartX = centerX - regionXSize/4;
    u_int XButtonStartY = centerY + (regionXSize/17 + regionYSize/3)/2 + 1;

    // Y button
    u_int YButtonStartX = centerX;
    u_int YButtonStartY = centerY + regionXSize/17;
  
  // draw L button
  if(controllerData.butL){
    display.fillRoundRect(LButtonStartX, LButtonStartY, 6, 4, 2, WHITE);
  }
  else{
    display.fillRoundRect(LButtonStartX, LButtonStartY, 6, 4, 2, BLACK);
    display.drawRoundRect(LButtonStartX, LButtonStartY, 6, 4, 2, WHITE);
  }

  // draw R button
  if(controllerData.butR){
    display.fillRoundRect(RButtonStartX, RButtonStartY, 6, 4, 2, WHITE);
  }
  else{
    display.fillRoundRect(RButtonStartX, RButtonStartY, 6, 4, 2, BLACK);
    display.drawRoundRect(RButtonStartX, RButtonStartY, 6, 4, 2, WHITE);
  }

  // draw A button
  if(controllerData.butA){
    display.fillCircle(AButtonStartX, AButtonStartY, buttonRadius, WHITE);
  }
  else{
    display.fillCircle(AButtonStartX, AButtonStartY, buttonRadius, BLACK);
    display.drawCircle(AButtonStartX, AButtonStartY, buttonRadius, WHITE);
  }

  // draw B button
  if(controllerData.butB){
    display.fillCircle(BButtonStartX, BButtonStartY, buttonRadius, WHITE);
  }
  else{
    display.fillCircle(BButtonStartX, BButtonStartY, buttonRadius, BLACK);
    display.drawCircle(BButtonStartX, BButtonStartY, buttonRadius, WHITE);
  }

  // draw X button
  if(controllerData.butX){
    display.fillCircle(XButtonStartX, XButtonStartY, buttonRadius, WHITE);
  }
  else{
    display.fillCircle(XButtonStartX, XButtonStartY, buttonRadius, BLACK);
    display.drawCircle(XButtonStartX, XButtonStartY, buttonRadius, WHITE);
  }

  // draw Y button
  if(controllerData.butY){
    display.fillCircle(YButtonStartX, YButtonStartY, buttonRadius, WHITE);
  }
  else{
    display.fillCircle(YButtonStartX, YButtonStartY, buttonRadius, BLACK);
    display.drawCircle(YButtonStartX, YButtonStartY, buttonRadius, WHITE);
  }

}

void drawControllerBorders(){
  display.drawRoundRect(32, 34, 27, 27, 2, WHITE); // draw border around joystick
}
#pragma endregion drawing_functions

void sendingModeOperations(){

  if(getButtonRisingEdge(switchButtonPressed, lastSwitchButtonState)){ // if just swapped to sending mode
    display.clearDisplay();  // display new team info
    drawDefaults();
    drawControllerState();
    display.display();
  }
  
  drawControllerState();
  display.display();

  // fire in the hole
  esp_err_t result = esp_now_send(address_list[currentAddressIndex], (uint8_t *) &controllerData, sizeof(controllerData));


  /*if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }*/ // uncomment if want debug messages
}

void debugModeOperations(){
  if(getButtonRisingEdge(switchButtonPressed, lastSwitchButtonState)){
    display.clearDisplay();
    drawDefaults();
    drawControllerState();
    //print debug indicator
    display.setTextSize(1);
    display.setCursor(0, 56);
    display.printf("DEBUG");
    display.setTextSize(2);
  }
  bool update = false; // track if update happened
  drawControllerState();
  

   if(getButtonRisingEdge(controllerData.butA, lastControllerData.butA)){ // Go back a team when you press A
    currentAddressIndex = (currentAddressIndex + 1) % address_count;
    update = true;
  }

  if(getButtonRisingEdge(controllerData.butB, lastControllerData.butB)){ // Go back a team when you press B
    if(currentAddressIndex == 0){
      currentAddressIndex = address_count - 1;
    }
    else{
      currentAddressIndex = (currentAddressIndex - 1) % address_count;
    }
    update = true;
  }
  
  if(getButtonRisingEdge(controllerData.butX, lastControllerData.butX)){ // flip joystick x axis when press X
    flipX = !flipX;
  }

  if(getButtonRisingEdge(controllerData.butY, lastControllerData.butY)){ // flip joystick y axis when press Y
    flipY = !flipY;
  }

  if(update){
    display.clearDisplay();
    drawDefaults();
    drawControllerState();
    // print debug indicator
    display.setTextSize(1);
    display.setCursor(0, 56);
    display.printf("DEBUG");
    display.setTextSize(2);
  }

  display.display();
}

bool getButtonRisingEdge(bool currentVal, bool oldVal){
  return currentVal && !oldVal; // return true only on rising edge of signal
}
