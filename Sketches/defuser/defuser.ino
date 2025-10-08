#include <LiquidCrystal.h>

#define BAUD_RATE 19200
#define LED_PIN 8

// seconds
#define DEFUSE_DEFAULT_TIME 90
#define DEFUSE_MIN_TIME 1

// SERIAL macros
#define DEFUSE_STARTED 'S'
#define DEFUSE_FAILED 'F'
#define DEFUSE_SUCCESS 'D'
#define DEFUSE_WAITING 'W'

//LCD
#define LCD_COLUMNS 16
#define LCD_ROWS 2
#define LCD_BOX 255
#define LCD_BLANK "                "

// DEFUSE_TIMES
#define DEFUSE_TIME_90 90
#define DEFUSE_TIME_30 60
#define DEFUSE_TIME_15 30
#define DEFUSE_TIME_1 15

#define MINS_HIGH 99
#define MINS_MID 30
#define MINS_LOW 15
#define MINS_LAST 1

#define DELAY_DEFAULT_TIME 5000

/* *********************************************************************************************************** */

// Time calc
int defuseTime_calculate(int mins);
/* *********************************************************************************************************** */

// SERIAL 
char defuseStatusRefresh(int defuse_temp_time);
int serialComms();
void disconnection_restart();
/* *********************************************************************************************************** */

/* *********************************************************************************************************** */

// Timer
unsigned long refreshTimer(unsigned long previousMillis);

/* *********************************************************************************************************** */

/* *********************************************************************************************************** */

// LCD
void lcdRefresh();

/* *********************************************************************************************************** */


LiquidCrystal lcd(7,6,5,4,3,2);

char response = DEFUSE_STARTED;
char oldResponse = -1;
char defuseStatus = DEFUSE_WAITING;
char oldDefuseStatus = -1;

// Timer variables
unsigned long timer_defuser;
unsigned long timer_serial;
unsigned long interval_defuser = 1000UL;
unsigned long interval_serial = 2000UL;

// flags
bool serial_waiting = false;

// Defuse time is in seconds
int defuseTime = DEFUSE_DEFAULT_TIME;
int defuse_time_remaining = DEFUSE_DEFAULT_TIME;

void setup() 
{
  Serial.begin(BAUD_RATE);
  pinMode(LED_PIN,OUTPUT);
  lcd.begin(16,2);
}

void loop() 
{
  defuseStatus = defuseStatusRefresh( serialComms() ); 
  if(defuseStatus != DEFUSE_SUCCESS)
  {    
    if(defuseStatus == DEFUSE_STARTED)
    {
      if(defuseStatus != oldDefuseStatus)
      {
        lcd.setCursor(0,0);
        lcd.print(LCD_BLANK);
        lcd.setCursor(0,0);
        lcd.print("   DEFUSING...");
        //lcd.print(defuseTime);
      }
      timer_defuser = refreshTimer(timer_defuser);
    }
    else
    {
      if(defuseStatus != oldDefuseStatus)
      {
        lcd.setCursor(0,0);
        lcd.print(LCD_BLANK);
        lcd.setCursor(0,0);
        lcd.print("CONNECT DEFUSER");   
      }  
    }
    oldDefuseStatus = defuseStatus;
  }
  else
  {
    lcd.setCursor(0,0);
    lcd.print(LCD_BLANK);
    lcd.setCursor(0,0);
    lcd.print("  BOMB DEFUSED");
    lcd.setCursor(0,1);
    lcd.print(LCD_BLANK);
    Serial.println(DEFUSE_SUCCESS);
    
    defuseStatus = DEFUSE_WAITING;
    oldDefuseStatus = DEFUSE_FAILED;
    defuseTime = DEFUSE_DEFAULT_TIME;
    defuse_time_remaining = DEFUSE_DEFAULT_TIME;
    Serial.end();
    Serial.begin(BAUD_RATE);
    Serial.flush();
    delay(DELAY_DEFAULT_TIME);
    lcd.clear();
    
  }
  
}

/* *********************************************************************************************************** */

// Time calc
int defuseTime_calculate(int mins)
{
  if( mins > MINS_MID )
  {
    return DEFUSE_TIME_90;
  }
  else if( mins >= MINS_LOW )
  {
    return DEFUSE_TIME_30;
  }
  else if ( mins >= MINS_LAST )
  {
    return DEFUSE_TIME_15;
  }
  else
  {
    return DEFUSE_TIME_1;
  }
}

/* *********************************************************************************************************** */

// SERIAL 

char defuseStatusRefresh(int defuse_temp_time)
{
  
  if(defuseStatus == DEFUSE_WAITING)
  {
    if(defuse_temp_time <= DEFUSE_DEFAULT_TIME && defuse_temp_time >= DEFUSE_MIN_TIME )
    {
      defuseTime = defuse_temp_time ;
      defuse_time_remaining = defuse_temp_time;
      digitalWrite(LED_PIN,HIGH);
      return DEFUSE_STARTED;
    }
    return DEFUSE_WAITING;
  }
  else if(defuseStatus == DEFUSE_STARTED)
  {
    if(defuse_time_remaining <= 0 && response != DEFUSE_FAILED)
    {
      digitalWrite(LED_PIN,LOW);
      return DEFUSE_SUCCESS;
    }
  }
  
  
}

void disconnection_restart()
{
  defuseStatus = DEFUSE_WAITING;
  oldDefuseStatus = -1;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("DISCONNECTED!");
  defuseTime = DEFUSE_DEFAULT_TIME;
  defuse_time_remaining = DEFUSE_DEFAULT_TIME;
  Serial.flush();
  Serial.end();
  Serial.begin(BAUD_RATE);
  Serial.flush();
  digitalWrite(LED_PIN,LOW);
  delay(DELAY_DEFAULT_TIME);
  lcd.clear();
}

int serialComms()
{
  int defuse_temp_time = -1;
  char tmp_response;
  unsigned long currentMillis = millis();
  if(currentMillis - timer_serial > interval_serial)
  {
    Serial.println(defuseStatus);
    timer_serial = currentMillis;
    if(defuseStatus == DEFUSE_STARTED)
    {
      if(serial_waiting && response != DEFUSE_STARTED)
      {
        disconnection_restart();
      }
      
      serial_waiting = !serial_waiting;
      response = DEFUSE_FAILED;
      lcd.setCursor(0,0);
      lcd.print(response);
    }
     
   }
   else
   {
    if(Serial.available())
    {
        if( (defuseStatus == DEFUSE_WAITING) )
        {
          defuse_temp_time = Serial.parseInt();
          defuse_temp_time = defuseTime_calculate(defuse_temp_time);
        }
        else
        {
          tmp_response = Serial.read();
          if(tmp_response == DEFUSE_SUCCESS || tmp_response == DEFUSE_STARTED)
          {
            response = tmp_response;
            lcd.setCursor(0,0);
            lcd.print(response);
          }
        }
    }
   }
  return defuse_temp_time;      
}

/* *********************************************************************************************************** */

// Timer
unsigned long refreshTimer(unsigned long previousMillis)
{
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > interval_defuser)
  {
    defuse_time_remaining--;
    lcdRefresh();
    return currentMillis;
  }
  return previousMillis;
}

/* *********************************************************************************************************** */

// LCD
void lcdRefresh()
{
  int lcd_boxes_num = LCD_COLUMNS * ( (float) ( (float) defuse_time_remaining /  (float) defuseTime ) ) ;
  for(int i = 0; i < LCD_COLUMNS - lcd_boxes_num ; i++)
  {
    lcd.setCursor(i,1);
    lcd.write(LCD_BOX);
  }
  lcd.setCursor(0,0);
}

/* *********************************************************************************************************** */
