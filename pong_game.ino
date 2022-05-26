
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <U8glib.h>


#define OLED_RESET 4
#define NO_CHANGE 0
#define STICK_UP 1
#define STICK_DOWN 2
int buzzerPin = 8;

// Use the emum type to create unique values for our constants
enum {
    POSITION = 0,
    SOUND,
    PAD_SPEED,
    BALL_SPEED,
    SKEW,
    SIZE,
    MULTIPLAYER,
    LAST
};

// Define and initialize our data structure for the parameters.
// The paddle position is changed simply by rotating the RE shaft when in normal game mode.
// There are 4 adjustable (i.e. customizable) parameters: sound, ball speed, skew and paddle size.
// To enter the adjustment/customization mode: press the RE switch. The parameters will cycle sequentially
// from sound->speed->skew->size and then the normal game play will be resumed with the adjusted values.
struct {
    const char *name;
    byte        minValue;
    byte        maxValue;
    byte        incValue;
    byte        activeValue;
} params[] = {
    {"Position",     0,  0,  2,  0},   // paddle position
    {"Sound",        0,  1,  1,  1},   // game sound
    {"Pad-Speed",    1, 19, 1, 1},
    {"Ball-Speed",   1, 19,  1,  4},   // ball speed
    {"Skew",         1, 15,  1,  1},   // ball skew
    {"Size",        10, 45,  5, 25},    // paddle size
    {"Multiplayer",  0, 1, 1, 0},
};


volatile byte joystickStatus = NO_CHANGE;
volatile unsigned long last_press = 0;

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_FAST); // for 1306 I2C (128x64)

// Arduino PIN numbers
const int SW_pin = 2; // digital pin connected to switch output
const int X_pin = 0; // analog pin connected to X output
const int Y_pin = 1; // analog pin connected to Y output
const int oled_CLK = 5; // analog pin connected to CLK output
const int oled_SDA = 4; // analog pin connected

int screenHeight;
int screenWidth;
float activeHeight;
const int pWidth = 6;
int pHeight;
int px, py;
int bx, by, br; // ball's center coordinate (x,y) and its radius respectively
int xinc; // ball increment in x-direction
int yinc; // ball increment in y-direction
byte skew; // current skew value
int ball_start_pos = 4;
unsigned int gameLoseScore = 0;   // tracks the score
unsigned int matchScore = 0;
boolean inAdjustMode = false; // normal game mode by default
char strBuf[16];              // used for string formatting
int switchPressCount;
unsigned long lastTime = 0;
int px2, py2;
int mpUserScore = 0;
int mpAIScore = 0;
int AIdir = 1;
unsigned long currentMillisScore;
unsigned long previousMillisScore = 0;
int justScored = 0;

ISR(readJoystickStatus)
{

  unsigned long press_time = millis();
  if (press_time - last_press > 500) {
    last_press = press_time;
    switchPressCount++;
  }

    
}

void setup() {
  // put your setup code here, to run once:
  pinMode(SW_pin, INPUT_PULLUP);
  pinMode (buzzerPin, OUTPUT);
  digitalWrite(SW_pin, HIGH); // activate internal PULLUP because pinmode is INPUT
  Serial.begin(9600);

//  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
//  display.clearDisplay();

   screenHeight = u8g.getHeight();
   screenWidth =  u8g.getWidth();
   activeHeight = 16;
   pHeight = params[SIZE].activeValue;

  Serial.print("screenHeight: ");
  Serial.print(screenHeight);
  Serial.print(" screenWidth: ");
  Serial.print(screenWidth);

  switchPressCount = 0;
  // Clear the screen
  u8g.firstPage(); 
  do {} 
     while(u8g.nextPage());

  xinc = ball_start_pos;
  skew = 1;

  /*      _ _
      /     \
     |   *   |  [bx,by] is the coordinate of the ball's center
      \ _ _ /
   
         |<->|   br is the ball's radius
*/

  // Configure starting parameters
  randomSeed(analogRead(1));
  br = 2;
  bx = br;
  by = random(br, screenHeight - br);
  yinc = random(-skew, skew);

  px = 0.7 * screenWidth;
  py = activeHeight;
  px2 = 0.2 * screenWidth;
  py2 = 5;

  Serial.println();
  Serial.println(params[MULTIPLAYER].activeValue);
  // Set up the gfx font
  u8g.setFont(u8g_font_profont10r);
  attachInterrupt(digitalPinToInterrupt(SW_pin), readJoystickStatus, FALLING);
}




void CheckAdjustMode()
{
  if (switchPressCount >= SOUND && switchPressCount <= MULTIPLAYER) {
    sprintf(strBuf, "%s:%d", params[switchPressCount].name, params[switchPressCount].activeValue);
    u8g.drawStr(5, 15, strBuf);

    unsigned long presentTime = millis();
    if( presentTime - lastTime > 100) {
      lastTime = presentTime;
      int VRx = analogRead(X_pin);
      if (VRx < 430) {
        if(params[switchPressCount].activeValue + params[switchPressCount].incValue >= params[switchPressCount].maxValue)
          params[switchPressCount].activeValue = params[switchPressCount].maxValue;
        else
          params[switchPressCount].activeValue +=  params[switchPressCount].incValue;
      } else if (VRx > 570) {
        if (params[switchPressCount].activeValue - params[switchPressCount].incValue <= params[switchPressCount].minValue)
          params[switchPressCount].activeValue = params[switchPressCount].minValue;
        else
          params[switchPressCount].activeValue -=  params[switchPressCount].incValue;
      }
    }
    if (xinc < 0)
      xinc = -1 * params[BALL_SPEED].activeValue;
    else
      xinc = params[BALL_SPEED].activeValue;
    skew = params[SKEW].activeValue;
    pHeight = params[SIZE].activeValue;

    if(params[MULTIPLAYER].activeValue){
      px = 0.8 * screenWidth;
      ball_start_pos = screenWidth/2;
    }
    else{
      ball_start_pos = 4;
      px = 0.7 * screenWidth;
      mpUserScore = 0;
      mpAIScore = 0;
    }
      
  }

  
    
  if(switchPressCount == LAST || switchPressCount == 0) {
    switchPressCount = 0;
    inAdjustMode = false;
  }
  else
    inAdjustMode = true;
  
}

void printJoystick(){
  Serial.print("SW: ");
  Serial.print(digitalRead(SW_pin));
  Serial.print(" VRx: ");
  Serial.print(analogRead(X_pin));
  Serial.print(" VRy: ");
  Serial.print(analogRead(Y_pin));
  Serial.print(" activeHeight: ");
  Serial.print(activeHeight);
  Serial.print("\n");
}

void printPaddle(){
  Serial.print("px-py: ");
  Serial.print(px);
  Serial.print("-");
  Serial.print(py);
  Serial.print("\n");
}

void printBall()
{
  Serial.print("xinc - yinc: ");
  Serial.print(xinc);
  Serial.print("-");
  Serial.print(yinc);
  Serial.print("\n");
  
  Serial.print("bx - by: ");
  Serial.print(bx);
  Serial.print("-");
  Serial.print(by);
  Serial.print("\n");
}

void CheckJoystick(){
  // If joystick was moved, print out the current status
  if (joystickStatus != NO_CHANGE) {
    printJoystick();
  }

}

void wallSound(){
  tone(buzzerPin, 1000, 100);
}

void scoreSound(){
  tone(buzzerPin, 800, 50);


}

void loop() {


  //printJoystick();
  //printPaddle();
  //printBall();
  
  CheckJoystick();

    // If the ball reached the left wall, reverse its direction
  if (xinc <= 0 && bx <= br)
  {
    if(params[MULTIPLAYER].activeValue){
      xinc = params[BALL_SPEED].activeValue;
      yinc = random(-skew, skew);
      
      bx = ball_start_pos;
      by = random(br, screenHeight-br);

      if(!inAdjustMode)
        mpUserScore++;
    }
      
    else
      xinc *= -1;
    if(params[SOUND].activeValue)
      wallSound();
  }



  // See if the ball hit the paddle...
  if((xinc > 0) && (bx+br >= px) && (by+br >= py) && (by <= py+pHeight+br) && (bx-br <= px+pWidth))
  {
    // Reverse its x-direction
    xinc *= -1;
    if(!inAdjustMode)
      matchScore++;
    
    // Reverse its y-direction, depending on where the ball touched the paddle.
    // The top 3/7 of the paddle bounces the ball in one direction and the bottom 3/7 of the paddle 
    // bounces it in the opposite direction (with some randomness thrown in, as a function of skew). 
    if (by <= py + (3 * pHeight) / 7)
      yinc = -random(1, skew + 1); // Add some randomness to the ball motion, based on the current skew setting
    if (by >= py + (4 * pHeight) / 7)
      yinc = random(1, skew + 1);  // Add some randomness to the ball motion, based on the current skew setting
    if(params[SOUND].activeValue)
      wallSound();
  }

  // AI paddle hit
  if(params[MULTIPLAYER].activeValue){
    if((xinc < 0) && (bx-br <= px2 + pWidth) && (by+br >= py2) && (by <= py2+pHeight+br) && (bx-br >= px2))
    {
      // Reverse its x-direction
      xinc *= -1;
      if(!inAdjustMode)
        matchScore++;
      
      // Reverse its y-direction, depending on where the ball touched the paddle.
      // The top 3/7 of the paddle bounces the ball in one direction and the bottom 3/7 of the paddle 
      // bounces it in the opposite direction (with some randomness thrown in, as a function of skew). 
      if (by <= py2 + (3 * pHeight) / 7)
        yinc = -random(1, skew + 1); // Add some randomness to the ball motion, based on the current skew setting
      if (by >= py2 + (4 * pHeight) / 7)
        yinc = random(1, skew + 1);  // Add some randomness to the ball motion, based on the current skew setting

      if(params[SOUND].activeValue)
        wallSound();
    }

    
  }


    // Check if the ball is clear of the top and bottom walls
  if (by+yinc >= (screenHeight-br) ||   // is the ball above the bottom wall, or
     (by+yinc <= br))                   // is the ball below the top wall
  {
    // ...bounce it off the wall
    yinc *= -1;
    if(params[SOUND].activeValue)
      wallSound();
  }

// See if the ball missed the paddle and reached the right side.
  // If so, update score and relaunch it from the left side.
  if (bx >= screenWidth)
  {
    if(params[MULTIPLAYER].activeValue){
      bx = ball_start_pos;
    }
    else
      bx = px - ((px / xinc) *  xinc);
      
    xinc = params[BALL_SPEED].activeValue;
    yinc = random(-skew, skew);
    //Serial.println(yinc);
    by = random(br, screenHeight-br);

    
    if (!inAdjustMode){
      if(params[MULTIPLAYER].activeValue){
        mpAIScore++;
      }
      else{
        gameLoseScore++;
        matchScore = 0;
        Serial.println("Lost point - incrementing gameLoseScore");  
      }
      
    }
   if(params[SOUND].activeValue)
    scoreSound();

   justScored = 1;
   previousMillisScore = millis();
  }
  else
  {
    // Advance the ball in the horizontal and vertical directions
    bx += xinc;
    by += yinc;
  }

  currentMillisScore = millis();

  if (currentMillisScore - previousMillisScore >= 100 && justScored == 1) {
    // save the last time you blinked the LED
    previousMillisScore = currentMillisScore;
    justScored = 0;
    Serial.println("secound sound!");
    scoreSound(); 
  }

//  unsigned long presTimeAI = millis();
//  if(presTimeAI - lastTimeAI > 100){
//    lastTimeAI = presTimeAI;
//    if(AIdir > 0 )
//      py2 += params[PAD_SPEED].activeValue;
//  }

  if(AIdir > 0){
    if(py2 + params[PAD_SPEED].activeValue + params[SIZE].activeValue >= screenHeight){
      AIdir = - AIdir;
      py2 = screenHeight - params[SIZE].activeValue;
    } else
      py2 += params[PAD_SPEED].activeValue;
  }
  else{
    if(py2 - params[PAD_SPEED].activeValue <= 0){
      AIdir = - AIdir;
      py2 = 0;
    } else
      py2 -= params[PAD_SPEED].activeValue;
    
  }
    



  int VRy = analogRead(Y_pin);
  if (VRy > 570 && activeHeight > 0)
    activeHeight -= params[PAD_SPEED].activeValue;
  
  else if (VRy < 430 && activeHeight + pHeight < screenHeight){
    activeHeight += params[PAD_SPEED].activeValue;
    
  }
  py = (int)activeHeight;

  
  u8g.firstPage();
    // Draw the borders
    do{
      u8g.drawLine(0, 0, screenWidth-1, 0);
      if(!params[MULTIPLAYER].activeValue)
        u8g.drawLine(0, 0, 0, screenHeight-1);
      u8g.drawLine(0, screenHeight-1, screenWidth-1, screenHeight-1);

      CheckAdjustMode();


      if(params[MULTIPLAYER].activeValue){


          if (inAdjustMode == true)
        {
          u8g.drawFrame(px2, py2, pWidth, pHeight);
          u8g.drawCircle(bx, by, br, U8G_DRAW_ALL); // Draw the ball as a circle
          u8g.drawFrame(px, py, pWidth, pHeight);   // Draw the paddle as a frame
        }
        else{
          u8g.drawRBox(px2, py2, pWidth, pHeight, 2); // AI paddle
          u8g.drawDisc(bx, by, br, U8G_DRAW_ALL);   // Draw the ball as a disc (i.e. solid circle)
          u8g.drawRBox(px, py, pWidth, pHeight, 2); // Draw paddle as solid box; use rounded corners with a radius of 2  
        }
        
      
        sprintf(strBuf, "%d", mpUserScore);
        u8g.drawStr((px+pWidth+screenWidth-u8g.getStrPixelWidth(strBuf))/2, screenHeight/2, strBuf);
        sprintf(strBuf, "%d", mpAIScore);
        u8g.drawStr(px2/2, screenHeight/2, strBuf);
      } else {
        if (inAdjustMode == true)
        {
          u8g.drawCircle(bx, by, br, U8G_DRAW_ALL); // Draw the ball as a circle
          u8g.drawFrame(px, py, pWidth, pHeight);   // Draw the paddle as a frame
        }
        else{
          u8g.drawDisc(bx, by, br, U8G_DRAW_ALL);   // Draw the ball as a disc (i.e. solid circle)
          u8g.drawRBox(px, py, pWidth, pHeight, 2); // Draw paddle as solid box; use rounded corners with a radius of 2  
        }
        
  
        
        sprintf(strBuf, "%d", gameLoseScore);
        u8g.drawStr((px+pWidth+screenWidth-u8g.getStrPixelWidth(strBuf))/2, screenHeight/4, strBuf);
        sprintf(strBuf, "%d", matchScore);
        u8g.drawStr((px+pWidth+screenWidth-u8g.getStrPixelWidth(strBuf))/2, screenHeight/4 * 3, strBuf);
      }
      
      
    }while(u8g.nextPage());
      


}
