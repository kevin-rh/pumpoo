#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <L298N.h>

#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif

const int MAX_INPUT_COUNT = 5;
const int MAX_TIME_SEC_INPUT = 99999;
const int MAX_VOL_SEC_INPUT = 99999;
const char BLANK_SPACE[6] = "_____";
const char INPUT_CURSOR = MAX_INPUT_COUNT;
/// Keypad Listener Class
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {5, 4, 3, 2}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

/// LCD Control Class
int cursor = 0;
String num_input = "";
LiquidCrystal_I2C lcd(0x27,20,4);

/// Time Variable Class
double dec_time_sec = 0; 
unsigned long start_time = 0;
long elapsed_time;

/// Pump Control Class
const double speed9V = .681886713; // TODO: Measure Pump Speed in mL
bool done_dispensing = true;
bool dispense_water = false;

const int enA = 13;
const int in1 = 12;
const int in2 = 11;

void turnOnPump(){
  if (dispense_water == true) return;

  Serial.print("Turn on Pump ");
  analogWrite(enA, 255);//PWM maximum(255) values(0-255)
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  dispense_water = true;
  start_time = millis();
}
void turnOffPump(){
  if (dispense_water == false) return;

  Serial.print("Turn off Pump ");
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  dispense_water = false;
  elapsed_time += (long)(millis() - start_time);
}

/// Preset Class
int preset[10] = {0,0,0,0,0,0,0,0,0,0};

void savePreset(char key, int value){
  // Save value at the array of preset (0-9 index)
  preset[key-'0'] = value;
}
int getPreset(char key){
  return preset[key-'0'];
}

// State Manager Class
int select_state = 0;//initial selecting state (input volume)
void (*cur_page)(KeypadEvent) = HomePageEvent;

void changeStateTo(void (*event)(KeypadEvent)){
  // Change the event listener to the desired event 
  cur_page = (*event);
  lcd.clear();
  keypad.addEventListener((*event)); 
  done_dispensing = false;
  elapsed_time = 0;
  num_input = "";
}


///// Main Program /////

/*  Set Up Part
  Declare pin input and output.
  Initialize pin output value.
*/

uint8_t big_heart[8] = {
  0b00000,
  0b01010,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000
  };
uint8_t small_heart[8] = {
  0b00000,
  0b00000,
  0b01010,
  0b01110,
  0b00100,
  0b00000,
  0b00000
  };
uint8_t rain0[8] = {
  0b10000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  };
uint8_t rain1[8] = {
  0b10000,
  0b00000,
  0b01000,
  0b00000,
  0b00000,
  0b00000,
  0b11111
  };
uint8_t rain2[8] = {
  0b00010,
  0b10000,
  0b10000,
  0b00000,
  0b00000,
  0b00000,
  0b11111
  };
uint8_t rain3[8] = {
  0b01001,
  0b00010,
  0b00010,
  0b00000,
  0b10000,
  0b10000,
  0b11111
  };
uint8_t rain4[8] = {
  0b10100,
  0b00000,
  0b01001,
  0b00000,
  0b00010,
  0b10010,
  0b11111
  };
uint8_t rain5[8] = {
  0b01000,
  0b00000,
  0b10100,
  0b00000,
  0b01001,
  0b01011,
  0b11111
  };
uint8_t rain6[8] = {
  0b01000,
  0b00000,
  0b10100,
  0b00000,
  0b01001,
  0b11111,
  0b11111
  };
uint8_t rain7[8] = {
  0b00001,
  0b00000,
  0b10100,
  0b00000,
  0b11111,
  0b11111,
  0b11111
  };
uint8_t rain8[8] = {
  0b00010,
  0b00000,
  0b10101,
  0b11111,
  0b11111,
  0b11111,
  0b11111
  };
uint8_t rain9[8] = {
  0b00100,
  0b00010,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
  };
uint8_t rain10[8] = {
  0b00100,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
  };
uint8_t rain11[8] = {
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
  };
void setup(){
  Serial.begin(9600);
  keypad.addEventListener(HomePageEvent); // Add an event listener for this keypad
  //lcd setup
  lcd.init();
  lcd.backlight();
  //pump setup
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  // Turn off motors - Initial state
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);

  lcd.createChar(0, big_heart);
  lcd.createChar(1, small_heart);
  lcd.createChar(2, rain0);
  lcd.createChar(3, rain1);
  lcd.createChar(4, rain2);
  lcd.createChar(5, rain3);
  lcd.createChar(6, rain4);
  lcd.createChar(7, rain5);
  lcd.createChar(8, rain6);
  lcd.createChar(9, rain7);
  lcd.createChar(10, rain8);
  lcd.createChar(11, rain9);
  lcd.createChar(12, rain10);
  lcd.createChar(13, rain11);


  lcd.home();
  start_time = millis();
}

/*  Main Loop
  Responsible of LCD print
  Take action if needed on each state
*/

void loop(){
  char customKey = keypad.getKey();

  if((*cur_page) == HomePageEvent)
  {
    lcd.setCursor(0, 0);
    lcd.print("Pumpoo         "); 
    lcd.setCursor(15,0);

    long now_time = millis();
    long delta_time = now_time - start_time;
    if (delta_time < 200){
      lcd.printByte(2);
    }else if(delta_time < 400){
      lcd.printByte(3);
    }else if(delta_time < 600){
      lcd.printByte(4);
    }else if(delta_time < 800){
      lcd.printByte(5);
    }else if(delta_time < 1000){
      lcd.printByte(6);
    }else if(delta_time < 1200){
      lcd.printByte(7);
    }else if(delta_time < 1400){
      lcd.printByte(8);
    }else if(delta_time < 1600){
      lcd.printByte(9);
    }else if(delta_time < 1800){
      lcd.printByte(10);
    }else if(delta_time < 2000){
      lcd.printByte(11);
    }else if(delta_time < 2200){
      lcd.printByte(12);
    }else if(delta_time < 2400){
      lcd.printByte(13);
    }
    if (delta_time>2400) start_time = now_time;


    lcd.setCursor(0, 1);
    char buffer[17];
    char to_print[17];
    dtostrf(speed9V, 10, 9, to_print);

    sprintf(buffer, "%s mL/s", to_print);
    lcd.print(buffer);
  }

  if((*cur_page) == ManualFillWaterEvent)
  {
    long pass_time = dispense_water ? (long)(millis() - start_time) : 0;
    long total_elapsed_time = elapsed_time + pass_time;

    // Prepare data to be printed
    char buffer[16];
    char doublebuf[8];    
    double time_to_print = ((double)total_elapsed_time) / 1000;
    double volume_to_print = ((double)total_elapsed_time) / 1000 * speed9V;

    // Print
    dtostrf(time_to_print, 6, 2, doublebuf);
    sprintf(buffer,"Time: %s sec", doublebuf);
    lcd.setCursor(0,0);
    lcd.print(buffer);

    dtostrf(volume_to_print, 7, 3, doublebuf);
    sprintf(buffer,"Vol.: %s mL", doublebuf);
    lcd.setCursor(0,1);   
    lcd.print(buffer);
    
  }
  if((*cur_page) == SelectingModeEvent)
  {
    lcd.setCursor(5,0);
    lcd.print("Select");
    switch(select_state){
    case 0:
      lcd.setCursor(2,1);
      lcd.print("Volume Mode");
      break;
    case 1:
      lcd.setCursor(4,1);
      lcd.print("Time Mode");
      break;
    case 2:
      lcd.setCursor(3,1);
      lcd.print("Speed Mode");
      break;
    }
  }
  if((*cur_page) == VolumeMode)
  { 
    lcd.setCursor(0,0);
    lcd.print("<Volume>");

    lcd.setCursor(16-6,0);
    lcd.print(" OK[#]");
    lcd.setCursor(16-6,1);
    lcd.print("DEL[*]");
  }
  else if((*cur_page) == TimeMode)
  {
    lcd.setCursor(0,0);
    lcd.print("<Time>");

    lcd.setCursor(16-6,0);
    lcd.print(" OK[#]");
    lcd.setCursor(16-6,1);
    lcd.print("DEL[*]");
  }
  else if((*cur_page) == SpeedMode)
  {
    lcd.setCursor(0,0);
    lcd.print("<Speed>"); 

    lcd.setCursor(16-6,0);
    lcd.print(" OK[#]");
    lcd.setCursor(16-6,1);
    lcd.print("DEL[*]");
  }
  if((*cur_page) == DispensingEvent)
  {
    long pass_time = (long)(millis() - start_time);
    long total_elapsed_time = elapsed_time + pass_time;
    long delta_time = ((long)dec_time_sec * 1000) - total_elapsed_time;

    if (delta_time>0)
    {
      // Prepare data to be printed
      char buffer[16];
      char doublebuf[8];
      double time_to_print = ((double)delta_time) / 1000;
      double volume_to_print = ((double)total_elapsed_time) / 1000 * speed9V;

      // Print
      dtostrf(time_to_print, 9, 6, doublebuf);
      sprintf(buffer,"T-%s sec", doublebuf);
      lcd.setCursor(0,0);
      lcd.print(buffer);

      dtostrf(volume_to_print, 7, 3, doublebuf);
      sprintf(buffer,"Vol.: %s mL", doublebuf);
      lcd.setCursor(0,1);   
      lcd.print(buffer);
    }
    else
    {
      // Action
      turnOffPump();
      done_dispensing = true;

      // Prepare data to be printed
      char buffer[16];
      char doublebuf[8];
      double time_to_print = (double) dec_time_sec;
      double volume_to_print = ((double)dec_time_sec) * speed9V;
      
      // Print
      dtostrf(time_to_print, 5, 2, doublebuf);
      sprintf(buffer,"[DONE] %s sec", doublebuf);
      lcd.setCursor(0,0);
      lcd.print(buffer);

      dtostrf(volume_to_print, 7, 3, doublebuf);
      sprintf(buffer,"Vol.: %s mL", doublebuf);
      lcd.setCursor(0,1);   
      lcd.print(buffer);
    }
  }  
}



void HomePageEvent(KeypadEvent key){
  switch (keypad.getState()){
  case PRESSED:
    if (key == 'D') {
        Serial.println("Go to Selecting Mode");
        changeStateTo(SelectingModeEvent);
    }
    if (key == 'A'){
        Serial.println("Go to ManualFillWater");
        turnOnPump();
        changeStateTo(ManualFillWaterEvent); 
    }
    break;

  case RELEASED:
      // if (key == '*') {
      // }
      break;

  case HOLD:
      getPreset(key);
      break;
  }
}

void ManualFillWaterEvent(KeypadEvent key){
  switch (keypad.getState()){
  case PRESSED:
    if (key == 'D') { // Go back to Home Page
        Serial.println("Go to HomePage"); 
        select_state = 0; // input volume state
        changeStateTo(HomePageEvent); 
    }
    if (key == 'A'){
        Serial.print("Turn on water ");
        turnOnPump();
    }
    break;
  case RELEASED:
    if (key == 'A'){
        Serial.print("Turn off water ");
        turnOffPump();
    }
    break;
  case HOLD:
    greenBoxListener(key);
    break;
  }
}
bool debounce_press = false;
void SelectingModeEvent(KeypadEvent key){
  switch (keypad.getState()){
  case PRESSED:
    if (key == 'D') { // Go back to Home Page
        Serial.println("Go to HomePage"); 
        select_state = 0; // input volume state
        changeStateTo(HomePageEvent); 
    }
    if (key == 'B' or key == 'C') { // scroll up or down
      if(debounce_press == false) debounce_press = true;
    }
    break;
  case RELEASED:
    if (key == 'B') { // scroll up
      if(select_state == 0 && debounce_press == true)
      {
        Serial.println("Input Speed Mode-2-C");
      }else if(select_state == 2 && debounce_press == true)
      {
        Serial.println("Input Time Mode-1-C");
      }else if(select_state == 1 && debounce_press == true)
      {
        Serial.println("Input Volume Mode-0-C");
      }
      // TODO: above this to be removed

      if(debounce_press == true){
        lcd.clear();
        select_state++;
        if (select_state > 2) select_state = 0;
      }
    }
    if (key == 'C'){ // scroll dwon
      
      if(select_state == 0 && debounce_press == true)
      {
        Serial.println("Input Speed Mode-2-C");
      }else if(select_state == 2 && debounce_press == true)
      {
        Serial.println("Input Time Mode-1-C");
      }else if(select_state == 1 && debounce_press == true)
      {
        Serial.println("Input Volume Mode-0-C");
      }
      // TODO: above this to be removed
      if(debounce_press == true){
        lcd.clear();
        debounce_press = false;
        select_state--;
        if (select_state < 0) select_state = 2;
      }
    }
    if(key == '#')
    {
      switch(select_state){
      case 0:
        Serial.println("Decided to Input Volume Mode-0");
        changeStateTo(VolumeMode); 
        lcd.setCursor(INPUT_CURSOR - MAX_INPUT_COUNT, 1);
        lcd.print(BLANK_SPACE);
        lcd.setCursor(INPUT_CURSOR + 1, 1);
        lcd.print("mL");
        break;
      case 1:
        Serial.println("Decided to Input Time Mode-1");
        changeStateTo(TimeMode);
        lcd.setCursor(INPUT_CURSOR - MAX_INPUT_COUNT, 1);
        lcd.print(BLANK_SPACE);
        lcd.setCursor(INPUT_CURSOR + 1, 1);
        lcd.print("sec");
        break;
      case 2:
        Serial.println("Decided to Input Speed Mode-2");
        changeStateTo(SpeedMode);
        break; 
      }
    }
    break;
  case HOLD:
    greenBoxListener(key);
    break;
  }
}


//Selecting Mode - Input Volume
void VolumeMode(KeypadEvent key)
{
  switch (keypad.getState()){
  case PRESSED:
    switch(key){
    // Go back to selecting mode
    case 'B':
    case 'C': 
      Serial.println("Back to Selecting Mode");
      changeStateTo(SelectingModeEvent);
      break;

    // Confirm Value
    case '#':
      dec_time_sec = num_input.toDouble() / speed9V;
      Serial.print("decided Volume: ");
      Serial.println(num_input.toDouble());
      turnOnPump();
      changeStateTo(DispensingEvent);
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':  
      char to_print[6];
      if (num_input.length() == MAX_INPUT_COUNT) num_input.remove(num_input.length()-1, 1);
      num_input += key; 
      num_input.toCharArray(to_print, 6);
      
      Serial.println(num_input);
      
      lcd.setCursor(INPUT_CURSOR - MAX_INPUT_COUNT, 1);
      lcd.print(BLANK_SPACE);

      lcd.setCursor(INPUT_CURSOR - num_input.length(), 1);
      lcd.print(to_print);
      break;
    case '*':
      if (num_input.length()>0) {
        char to_print[6];
        num_input[num_input.length()-1] = 0;
        num_input.remove(num_input.length()-1, 1);
        num_input.toCharArray(to_print, 6);
        
        Serial.println("del");
        Serial.println(to_print);

        lcd.setCursor(INPUT_CURSOR - MAX_INPUT_COUNT, 1);
        lcd.print(BLANK_SPACE);

        lcd.setCursor(INPUT_CURSOR - num_input.length(), 1);
        lcd.print(to_print);
      }
      break;
    }
    break;
  }
}
//Selecting Mode - Input Time
void TimeMode(KeypadEvent key)
{
  switch (keypad.getState()){
  case PRESSED:
    switch(key){
    // Go back to selecting mode
    case 'B':
    case 'C': 
      Serial.println("Back to Selecting Mode");
      changeStateTo(SelectingModeEvent);
      break;

    // Confirm Value
    case '#':
      dec_time_sec = num_input.toDouble();
      Serial.print("decided Time: ");
      Serial.println(dec_time_sec);
      turnOnPump();
      changeStateTo(DispensingEvent);
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':  
      char to_print[6];
      if (num_input.length() == MAX_INPUT_COUNT) num_input.remove(num_input.length()-1, 1);
      num_input += key; 
      num_input.toCharArray(to_print, 6);
      
      Serial.println(num_input);
      
      lcd.setCursor(INPUT_CURSOR - MAX_INPUT_COUNT, 1);
      lcd.print(BLANK_SPACE);

      lcd.setCursor(INPUT_CURSOR - num_input.length(), 1);
      lcd.print(to_print);
      break;
    case '*':
      if (num_input.length()>0) {
        char to_print[6];
        num_input[num_input.length()-1] = 0;
        num_input.remove(num_input.length()-1, 1);
        num_input.toCharArray(to_print, 6);
        
        Serial.println("del");
        Serial.println(to_print);

        lcd.setCursor(INPUT_CURSOR - MAX_INPUT_COUNT, 1);
        lcd.print(BLANK_SPACE);

        lcd.setCursor(INPUT_CURSOR - num_input.length(), 1);
        lcd.print(to_print);
      }
      break;
    }
    break;
  }
}
void DispensingEvent(KeypadEvent key)
{
  switch (keypad.getState()){
    case PRESSED:
      // Check if dispensing is finished.
      if (done_dispensing == false) break; // TODO: doneDispensing

      switch(key){
      // Go back to the Selecting Mode page.
      case 'B':
      case 'C': 
        // Reset Value
        Serial.println("Back to Selecting Mode");
        changeStateTo(SelectingModeEvent);
        break;
      // Go back to Home page.
      case 'D':
        // Reset Value
        changeStateTo(HomePageEvent);
      }
    break;
    case HOLD:
      if (key == 'D'){
        // TODO: Abort STOP
        // Reset Value
        num_input = "";
        turnOffPump();
        changeStateTo(HomePageEvent);
      }
    break;
  }
}
//Selecting Mode - Input Speed
void SpeedMode(KeypadEvent key)
{
  //TODO
  if (key == 'B'||key =='C' ) //go back to selecting mode
  { 
      Serial.println("Back to Selecting Mode");
      changeStateTo(SelectingModeEvent);
  }
}

void greenBoxListener(char key){
  switch(key){
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    // TODO: implement water counter
    Serial.print("save key ");
    break;
  case 'B':
    Serial.print("Reset counter");
    break;
  }
}
