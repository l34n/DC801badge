//DONE: Scan from frame buffer
//DONE: Interupt based timing
//DONE: Replace Arduino digitalWrites with direct writes to ports (for speed!)
  //digitalWrite code: about 10ms. Direct writes < 3ms!

// Add button functionality (top one only for now)
#include "ClickButton.h"
int function = 0;
const int buttonPin1 = 7;
ClickButton button1(buttonPin1, LOW, CLICKBTN_PULLUP);

//#define BUZZER A4            //PF1
#define COL_DATA 10          //PB6
#define COL_OUTPUT_ENABLE A1 //PF6
#define COL_STROBE A0        //PF7 Strobe basically STCP 
#define COL_CLOCK 13         //PC7 Clock input SHCP
#define COL_MEM_RESET 5      //PC6
#define ROW_DATA 9           //PB5
#define ROW_OUTPUT_ENABLE 8  //PB4
#define ROW_STROBE 6         //PD7
#define ROW_CLOCK 12         //PD6
#define ROW_MEM_RESET 4      //PD4
#define JOY_X A2             //PF5
#define JOY_Y A3             //PF4 maybe swapped with above, test it!
#define BATTERY_VOLTS A5     //PF0
#define BUTTON_TOP 7         //PE6
//#define BUTTON_BOTTOM      //PD5 No Arduino Pin

/* Released to public domain */  
const int selectPin = 2;  
const int joystick_xPin = A2;  
const int joystick_yPin = A3;  
int oldX = 0;  
int oldY = 0;  
int oldSelect = 0;

//Calibration values
int highX = 0;
int highY = 0;
int lowX = 1000;
int lowY = 1000;

static int currentCol = 0;
//int colBit = 0;
//int rowBit = 0;

//frameBuffer[y][x]
int frameBuffer[16][16] = {{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                           {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                           {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                           {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                           {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                           {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                           {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                           {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                           {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                           {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                           {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                           {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                           {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                           {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                           {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                           {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}};
                         
int frameStore[16][5] = {{1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1}};

void setup()
{
 
  //pinMode(BUZZER, OUTPUT);
  pinMode(COL_DATA, OUTPUT);
  pinMode(COL_OUTPUT_ENABLE, OUTPUT);
  pinMode(COL_STROBE, OUTPUT);
  pinMode(COL_CLOCK, OUTPUT);
  pinMode(COL_MEM_RESET, OUTPUT);
  pinMode(ROW_DATA, OUTPUT);
  pinMode(ROW_OUTPUT_ENABLE, OUTPUT);
  pinMode(ROW_STROBE, OUTPUT);
  pinMode(ROW_CLOCK, OUTPUT);
  pinMode(ROW_MEM_RESET, OUTPUT);

//Joystick
  pinMode(selectPin, INPUT);  
  digitalWrite(selectPin, HIGH);  
  Serial.begin(9600); 

  //digitalWrite(BUZZER, HIGH);
  
  digitalWrite(COL_OUTPUT_ENABLE, LOW);
  digitalWrite(ROW_OUTPUT_ENABLE, LOW);
  digitalWrite(COL_MEM_RESET, HIGH);
  digitalWrite(ROW_MEM_RESET, HIGH);
  
  digitalWrite(COL_DATA, LOW);
  digitalWrite(COL_CLOCK, LOW);
  digitalWrite(ROW_DATA, LOW);
  digitalWrite(ROW_DATA, LOW);
  
  setupTimer();
}

void setupTimer()
{
//http://letsmakerobots.com/node/28278  
  
// initialize timer5 
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;        //
  TCCR1B = 0;        //
  TCNT1  = 0;        //

  //OCR1A = 31250;            // compare match register 16MHz/256/2Hz //or 8Mhz/256/1hz or 8Mhz/256/1hz
  OCR1A = 20;            // compare match register  1ms
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  interrupts();             // enable all interrupts
}

ISR(TIMER1_COMPA_vect)
{
  //pinMode(1, OUTPUT); //For Frame Rate measurement. Currently about 400FPS!
  //digitalWrite(1, HIGH);
  //for(int i = 0; i < 16; i++)
  //{
    currentCol++;
    if(currentCol > 15) currentCol = 0;
    
     writeCol((1 << currentCol) ^ 0b1111111111111111);
     writeRowBuffer(currentCol);//(frameBuffer[0]);
     strobe();
  //}
  //digitalWrite(1, LOW);
  //delayMicroseconds(10);
}

void writeCol(int data)
{
  for(int i = 0; i < 16; i++)
  {
    //COL_DATA is PB6
    PORTB = (PORTB & 0b10111111) | (((data >> i) & 1) << 6);
    //digitalWrite(COL_DATA, (data >> i) & 1);
    
    //COL_CLOCK is PC7
    PORTC = PORTC | 0b10000000;
    PORTC = PORTC & 0b01111111;
    //digitalWrite(COL_CLOCK, HIGH);
    //digitalWrite(COL_CLOCK, LOW);  
  } 
}

void writeRow(int data)
{
  for(int i = 0; i < 16; i++)
  {
    //ROW_DATA is PB5
    PORTB = (PORTB & 0b11011111) | (((data >> i) & 1) << 5);
    //digitalWrite(ROW_DATA, (data >> i) & 1);
    
    //ROW_CLOCK is PD6
    PORTD = PORTD | 0b01000000; 
    PORTD = PORTD & 0b10111111;    
    //digitalWrite(ROW_CLOCK, HIGH); 
    //digitalWrite(ROW_CLOCK, LOW);  
  } 
}

void writeRowBuffer(int col)
{
  for(int i = 0; i < 16; i++)
  {
    //ROW_DATA is PB5
    PORTB = (PORTB & 0b11011111) | (((frameBuffer[15-i][col]) & 1) << 5);
    //digitalWrite(ROW_DATA, (data >> i) & 1);
    
    //ROW_CLOCK is PD6
    PORTD = PORTD | 0b01000000; 
    PORTD = PORTD & 0b10111111;    
    //digitalWrite(ROW_CLOCK, HIGH); 
    //digitalWrite(ROW_CLOCK, LOW);  
  } 
}

void strobe()
{
  //COL_STROBE is PF7
  //ROW_STROBE is PD7
  PORTD = PORTD | 0b10000000; 
  PORTF = PORTF | 0b10000000; 
  PORTD = PORTD & 0b01111111;
  PORTF = PORTF & 0b01111111;
  
  //digitalWrite(ROW_STROBE, HIGH);
  //digitalWrite(COL_STROBE, HIGH);
  //digitalWrite(COL_STROBE, LOW);
  //digitalWrite(ROW_STROBE, LOW);
 }

void invert()
{
  for(int i = 0; i < 16; i++)
  {
    for(int j = 0; j < 16; j++)
    {
      frameBuffer[i][j] ^= 1;  
    }
  }  
    //Gotta invert the StoreBuffer too
  for(int i = 0; i < 16; i++)
  {
    for(int j = 0; j < 5; j++)
    {
      frameStore[i][j] ^= 1;  
    }
  }
}
void reset(int a) //reset all LEDs to 0 or 1
{
  for(int i = 0; i < 10; i++)
  {
     for(int j = 0; j < 10; j++)
     {
       frameBuffer[i][j] = a;
     }
   } 
}

void calibrate()
{
  
}  

void shift()
{
 for(int j = 0; j < 16; j++)
 {
   for(int i = 0; i < 16; i++)
   {
     if( j == 0 ){
       frameStore[i][0] = frameStore[i][1];
       frameStore[i][1] = frameStore[i][2];
       frameStore[i][2] = frameStore[i][3];
       frameStore[i][3] = frameStore[i][4];
       frameStore[i][4] = frameBuffer[i][j];
       
     }
     if( j == 15 ){
       frameBuffer[i][j] = frameStore[i][0];
     }
     else
       {
       frameBuffer[i][j] = frameBuffer[i][j+1];
     }
   }
 }
}

void loop()
{
  static long lastTime = millis();
//  //if(frameBuffer[0][0] == 1){ invert(); }
//  
//     button1.Update();
//
//  if (button1.click != 0) function = button1.click;
//  
//  // Toggle LED on single clicks
//  if(button1.click == 1) invert();
  
   {  
      int joystick_x;  
      int joystick_y;  
      int select;  
      
      //once you have your true high and low values from below, modify the 0 & 1023 respectively 
      
      //joystick_x = map(analogRead(joystick_xPin), 0, 1023, 15, 0);  
      //joystick_y = map(analogRead(joystick_yPin), 0, 1023, 0, 16);  
      joystick_x = analogRead(joystick_xPin);
      joystick_y = analogRead(joystick_yPin);
   
     select = !digitalRead(selectPin);  
      if((oldX != joystick_x) ||  
          (oldY != joystick_y) ||  
         (oldSelect != select)){ 
            if (joystick_x > highX && joystick_x < 1000)
            {
              highX = joystick_x;
            }
            if (joystick_y > highY && joystick_y < 1000)
            {
              highY = joystick_y;
            }
            if (joystick_x < lowX)
            {
              lowX = joystick_x;
            }
            if (joystick_y < lowY)
            {
              lowY = joystick_y;
            }
            Serial.print("High X: ");
            Serial.print(highX);
            Serial.print(" High Y: ");
            Serial.print(highY);
            
            Serial.print("\nLow X: ");
            Serial.print(lowX);
            Serial.print(" Low Y: ");
            Serial.print(lowY);
         

       
       
//           Serial.print("joystick X: ");  
//           Serial.print(joystick_x);  
//           Serial.print(" joystick Y: ");  
//           Serial.print(joystick_y);
//           if(joystick_x < 16 && joystick_y < 16)
//           {  
//             reset(1);
//             frameBuffer[joystick_y][joystick_x] = 0;
//           }
             
           if(select){  
              Serial.print(" select");
          
           }  
           Serial.println("");  
           oldX = joystick_x;  
           oldY = joystick_y;  
           oldSelect = select;  
      }  
     delay(10);  
    }
}
//  if(millis() > lastTime + 1000)
//  {
//    lastTime = millis();
//    int a = frameBuffer[0][0];
//    a ^=1;
//    reset(a);
//    Serial.print(a);
//    //shift();
//    //invert();
//  }


