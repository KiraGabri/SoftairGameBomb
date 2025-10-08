#include <LiquidCrystal.h>
#include <Keypad.h>

#define BAUD_RATE 19200
#define ERROR_VALUE -1
// BUZZER Macros
#define BUZZER_PIN A4
#define BUZZER_KEYPAD_PIN A5

// LCD Macros
#define LCD_COLUMNS 16
#define LCD_ROWS 2
#define LCD_BLANK "                "
#define LCD_SPACE ' '
#define LCD_POS_MIDDLE 5
#define LCD_POS_START 0
#define LCD_POS_END 15

// Keypad Macros
#define KEYPAD_ENTER_KEY '#'
#define KEYPAD_DELETE_KEY '*'
#define KEYPAD_A_KEY 'A'
#define KEYPAD_B_KEY 'B'
#define KEYPAD_C_KEY 'C'
#define KEYPAD_D_KEY 'D'
#define KEYPAD_ROWS 4
#define KEYPAD_COLUMNS 4

// Admin macros
#define ADMIN_INPUTS_DEFAULT 10
#define ADMIN_DEFAULT_MIN_MINUTES 0
#define ADMIN_MAX_DIGITS 3
#define ADMIN_MAX_INPUTS 255

// Password check macros
#define PSW_CHAR_ADDED 2
#define PSW_CHAR_NOT_ADDED 3 
#define PSW_CHAR_DELETED 4
#define PSW_CHAR_NOT_DELETED 5
#define PSW_WRONG -1
#define PSW_CORRECT 1
#define PSW_EQUALS 0
#define PSW_OFFSET 1

// Time input macros
#define TMI_FIGURES 4
#define TMI_MIDDLE 2
#define TMI_OFFSET 1
#define TMI_CHAR_ZERO '0'
#define TMI_CHAR_NINE '9'

// TONE Macros
#define TONE_DEFAULT 1500
#define TONE_DEFAULT_TIMER 2000
#define TONE_TIMER_60 2300
#define TONE_TIMER_30 2500
#define TONE_TIMER_15 2700
#define TONE_TIMER_5 3000 
#define TONE_ERROR 1000

// TONE duration Macros
#define TONE_DURATION_KEYPAD 50
#define TONE_DURATION_DEFAULT_TIMER 100
#define TONE_DURATION_BOMB_LAST 6000
#define TONE_ITER_BOMB_FAILED 6
#define DELAY_AFTER_EXPLOSION 2000

// Timer Macros
#define TIMER_PRINT_ZERO "00"
#define TIMER_PRINT_UNIT '0'
#define TIMER_PRINT_SEPARATOR ':'
#define TIMER_ZERO 0
#define TIMER_TEN 10

// SERIAL macros
#define DEFUSE_STARTED 'S'
#define DEFUSE_FAILED 'F'
#define DEFUSE_SUCCESS 'D'
#define DEFUSE_WAITING 'W'

/* *********************************************************************************************************** */

// START Funzioni inizio programma
void getNewPassword();
void requestTime();
void printRequestTime(char *temp_time);

/* *********************************************************************************************************** */

// TIMER Funzioni Timer 
void bombTimer();
unsigned long refreshTimer(unsigned long previousMillis);
void timer_printCurrentTime();

/* *********************************************************************************************************** */

// PASSWORD Funzioni password quando la bomba e` attiva 
byte manageKeyPress(char currentKey,char *stringPass);
byte addChar(char currentKey,char *stringPass);
byte removeChar(char *stringPass);
byte comparePassword(char *stringPass); 

/* *********************************************************************************************************** */

// BUZZER Funzioni per funzionamento del suono  
void playTimerTone();
void playBombExplodeTone();
void playBombDefusedTone();

/* *********************************************************************************************************** */

// SERIAL_COMS Funzioni comunicazione con la seriale 
char serial_check_defuser_state(int oldState);
bool isBombDefused(char state);
void serial_serial_sendTime(int def_time);
void serial_sendResponse(char k);

/* *********************************************************************************************************** */

// ADMIN Funzioni per settaggio di parametri minimi della bomba
void setAdminParameters();
void getNewMinTime();
void getNewAdminInputs();

/* *********************************************************************************************************** */

// controlli vari
bool key_check_isNAN(char key);

/* *********************************************************************************************************** */

// Admin inputs
byte inputs_to_admin = ADMIN_INPUTS_DEFAULT; 

// LCD
LiquidCrystal lcd(7,6,5,4,3,2);
//Led7 led7(DATA_PIN,LATCH_PIN,CLOCK_PIN);

// Keypad
const char keys[KEYPAD_ROWS][KEYPAD_COLUMNS] = 
{
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
const byte rowPins[KEYPAD_ROWS] = {8,9,10,11};
const byte columnPins[KEYPAD_COLUMNS] = {12,13,A0,A1};
Keypad keypad = Keypad(makeKeymap(keys),rowPins,columnPins,KEYPAD_ROWS,KEYPAD_COLUMNS); // Ci penserà la funzione a fare pinMode ecc.

// input Chars
short int inputedChars = 0;
// password
char* password;

// Timer
unsigned long time_start= 0,time_passed = 0;
unsigned long interval = 1000UL;   
// Time
short int MIN_minutes = ADMIN_DEFAULT_MIN_MINUTES;
short int minutes = 0;
short int seconds = 0;

// flags
bool time_started = false;

// tone
short int tone_modifier = 1;

void setup()
{
  Serial.begin(BAUD_RATE);
//Entropy.initialize();
  pinMode(BUZZER_PIN,OUTPUT);
  lcd.begin(16,2);
}

void loop() 
{
  password = (char *) malloc(sizeof(char)* ( LCD_COLUMNS) );
  password[0] = '\0';
  if(!time_started)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Insert Password:");
    getNewPassword();
    requestTime();
    time_started = true;
    inputedChars = ERROR_VALUE ;
  }
  if( time_started )
  {
    bombTimer();
    time_started = false ;
  }
  free(password);
  delay(DELAY_AFTER_EXPLOSION);
  
}

/* *********************************************************************************************************** */
// START Funzioni inizio programma
void getNewPassword()
{
  char newChar;
  short int oldInputedChars = ERROR_VALUE;
  byte deletePressed = 0 ;
  inputedChars = ERROR_VALUE;
  newChar = NULL;
  while(newChar != KEYPAD_ENTER_KEY )
  {
    newChar = keypad.getKey();
    oldInputedChars = inputedChars;
    if(deletePressed == inputs_to_admin )
    {
      setAdminParameters();
      newChar == NO_KEY;
      deletePressed = 0;
    }
    if(newChar)
    {
      
      if(newChar == KEYPAD_ENTER_KEY)
      {
        lcd.clear();
        tone(BUZZER_KEYPAD_PIN,TONE_DEFAULT,TONE_DURATION_KEYPAD);
        deletePressed = 0;
      }
      else if( newChar == KEYPAD_DELETE_KEY )
      {
        removeChar(password);
        tone(BUZZER_KEYPAD_PIN,TONE_DEFAULT,TONE_DURATION_KEYPAD);
        if(inputedChars == ERROR_VALUE )
        {
          deletePressed++;
        }
      }
      else if( inputedChars < LCD_COLUMNS - PSW_OFFSET )
      {
        addChar(newChar,password);
        tone(BUZZER_KEYPAD_PIN,TONE_DEFAULT,TONE_DURATION_KEYPAD);
        deletePressed = 0;
      }
      else
      {
        lcd.setCursor(LCD_POS_START,LCD_POS_START);
        lcd.println("Max 16 letters  ");
        tone(BUZZER_KEYPAD_PIN,TONE_ERROR,TONE_DURATION_KEYPAD);
      }
     
    }
    
    if( oldInputedChars != inputedChars )
    {
      lcd.clear();
      lcd.setCursor(LCD_POS_START,LCD_POS_START);
      lcd.print("Insert Password:");
      lcd.setCursor(LCD_POS_START,1);
      lcd.print(password);
      lcd.setCursor(LCD_POS_START,inputedChars);
    }
    
  }
}

void printRequestTime(char *temp_time)
{
   lcd.setCursor(LCD_POS_MIDDLE,1);
   for(int i = 0 ; i < TMI_FIGURES ; i++)
    {
      if( i == TMI_MIDDLE )
      {
        lcd.print(TIMER_PRINT_SEPARATOR);
      }
      if(temp_time[i] >= TMI_CHAR_ZERO && temp_time[i] <= TMI_CHAR_NINE)
      {
        lcd.print(temp_time[i]);
      }
      else
      {
        lcd.print(LCD_SPACE);
      }
    }
}

  // Funzione richiesta del tempo alla detonazione
void requestTime()
{
  String temp_convert;
  String min_convert = "  ";
  char * temp_time = (char*) malloc(sizeof(char)*TMI_FIGURES);
  strcpy(temp_time,"    ");
  char currentKey = NULL;
  lcd.setCursor(LCD_POS_START,LCD_POS_START);
  lcd.print("Insert time :");
  inputedChars = ERROR_VALUE;
  do
  {
    currentKey = keypad.getKey();

    if(currentKey && !key_check_isNAN(currentKey) && currentKey != KEYPAD_ENTER_KEY)
    {
      tone(BUZZER_KEYPAD_PIN,TONE_DEFAULT,TONE_DURATION_KEYPAD);
      if(currentKey!= KEYPAD_DELETE_KEY)
      {
        if( ( inputedChars + TMI_OFFSET ) < TMI_FIGURES )
        {
          addChar(currentKey,temp_time);
        }
        else
        {
          tone(BUZZER_KEYPAD_PIN,TONE_ERROR,TONE_DURATION_KEYPAD);
        }
      }
      else
      {
        removeChar(temp_time);
        lcd.setCursor(LCD_POS_START,LCD_POS_START);
        lcd.print(LCD_BLANK);
        lcd.setCursor(LCD_POS_START,LCD_POS_START);
        lcd.print("Insert time :");
      }
     
    }
    min_convert[0] = temp_time[0];
    min_convert[1] = temp_time[1];
    minutes = min_convert.toInt();
    min_convert[0] = '\0';
    min_convert[1] = '\0';
    if(minutes < MIN_minutes && inputedChars + TMI_OFFSET == TMI_MIDDLE)
    {
      lcd.setCursor(LCD_POS_START,LCD_POS_START);
      lcd.print("Minimum mins:");
      lcd.print(MIN_minutes);
    }
    printRequestTime(temp_time);
  }
  while(currentKey != KEYPAD_ENTER_KEY || ( inputedChars + TMI_OFFSET ) < TMI_MIDDLE || minutes < MIN_minutes);
  temp_convert.concat(temp_time[0]);
  temp_convert.concat(temp_time[1]);
  minutes = temp_convert.toInt();
  temp_convert[0] = temp_time[2];
  temp_convert[1] = temp_time[3];
  seconds = temp_convert.toInt();
  free(temp_time);
  lcd.clear();
  seconds++;
}



/* ******************************************************************************************************************************************************************************** */
// TIMER Funzioni Timer
void bombTimer()
{
  lcd.clear();
  char* attemptPassword = (char *) malloc(sizeof(char)*( LCD_COLUMNS)); ;
  unsigned long previousMillis;
  byte result = NULL; // occhio
  char key = NULL;
  char state = ERROR_VALUE;
  inputedChars = ERROR_VALUE;
  while( seconds > 0 || minutes > 0 )
  {
    state =  serial_check_defuser_state(state);
    key = keypad.getKey();
    if(key)
    {
      result = manageKeyPress(key,attemptPassword);
      lcd.setCursor(LCD_POS_START,1);
      lcd.print(LCD_BLANK);
    }
    lcd.setCursor(0,1);
    lcd.print(attemptPassword);
    previousMillis = refreshTimer(previousMillis);
    if( result == PSW_CORRECT || isBombDefused(state) ) 
    {
      seconds = 0; minutes = 0;
    }
  }
  
  if(result != PSW_CORRECT && !isBombDefused(state) )
  {
    playBombExplodeTone();
    delay(DELAY_AFTER_EXPLOSION);
  }
  else
  {
    playBombDefusedTone();
    delay(DELAY_AFTER_EXPLOSION);
  }
  lcd.clear();
  minutes = 0;
  seconds = 0;
  time_started = false; 
  Serial.end();
  Serial.begin(BAUD_RATE);
  free(attemptPassword);
}

unsigned long refreshTimer(unsigned long previousMillis)
{
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > interval)
  {
    playTimerTone();
    if(seconds != 0 )
    {
      seconds--;
    }
    else
    {
      if( minutes > 0 )
      {
        minutes--;
        seconds = 59;
      }
    }
    timer_printCurrentTime();
    return currentMillis;
  }
  return previousMillis;
}

void timer_printCurrentTime()
{
    lcd.setCursor(LCD_POS_MIDDLE,LCD_POS_START);
    if( minutes < TIMER_ZERO )
    {
      lcd.print(TIMER_PRINT_ZERO);
    }
    else if(minutes <  TIMER_TEN )
    {
      lcd.print(TIMER_PRINT_UNIT);
      lcd.print(minutes);
    }
    else
    {
      lcd.print(minutes);
    }
    lcd.print(TIMER_PRINT_SEPARATOR);
    if(seconds < TIMER_TEN)
    {
      lcd.print(TIMER_PRINT_UNIT);
    }
    lcd.print(seconds);
}

/* ******************************************************************************************************************************************************************************** */
// PASSWORD Funzioni password quando la bomba e` attiva

byte manageKeyPress(char currentKey,char *stringPass)
{
  byte result;
  if(currentKey == KEYPAD_ENTER_KEY)
  {
    result = comparePassword(stringPass);
    if(result == PSW_CORRECT )
    {
      tone(BUZZER_KEYPAD_PIN,TONE_DEFAULT,TONE_DURATION_KEYPAD);
    }
    else
    {
      tone(BUZZER_KEYPAD_PIN,TONE_ERROR,TONE_DURATION_KEYPAD);
    }
    return result;
  }
  else if( currentKey == KEYPAD_DELETE_KEY )
  {
    tone(BUZZER_KEYPAD_PIN,TONE_DEFAULT,TONE_DURATION_KEYPAD);
    return removeChar(stringPass);
  }
  else if( inputedChars < LCD_COLUMNS - PSW_OFFSET )
  {
    tone(BUZZER_KEYPAD_PIN,TONE_DEFAULT,TONE_DURATION_KEYPAD);
    return addChar(currentKey,stringPass);   
  }
  else
  {
    tone(BUZZER_KEYPAD_PIN,TONE_ERROR,TONE_DURATION_KEYPAD);
  }
  return PSW_CHAR_NOT_ADDED;
}

byte addChar(char currentKey,char *stringPass)
{
  stringPass[inputedChars + PSW_OFFSET] = currentKey;
  inputedChars++;
  stringPass[inputedChars + PSW_OFFSET] = '\0';
  return PSW_CHAR_ADDED;
}

byte removeChar(char *stringPass)
{
  if(inputedChars > ERROR_VALUE)
  {
    stringPass[inputedChars] = '\0';
    inputedChars--;
    return PSW_CHAR_DELETED;
  }
  return PSW_CHAR_NOT_DELETED;
}


byte comparePassword(char* stringPass)
{
  if( strcmp(password,stringPass) == PSW_EQUALS )
  {
    return PSW_CORRECT;
  }
  return PSW_WRONG;
}

/* ******************************************************************************************************************************************************************************** */



// BUZZER Funzioni per funzionamento del suono 
void playTimerTone()
{
  if( minutes < 1 )
  {
    tone_modifier += 2;
    if (seconds <= 5 )
    {
      tone(BUZZER_PIN,TONE_TIMER_5 + tone_modifier,TONE_DURATION_DEFAULT_TIMER);
    }
    else if( seconds <= 15 )
    {
      tone(BUZZER_PIN,TONE_TIMER_15 + tone_modifier,TONE_DURATION_DEFAULT_TIMER);
    }
    else if( seconds <= 30 )
    {
      tone(BUZZER_PIN,TONE_TIMER_30 + tone_modifier,TONE_DURATION_DEFAULT_TIMER);
    }
    else if ( seconds <= 60 )
    {
      tone(BUZZER_PIN,TONE_TIMER_60 + tone_modifier,TONE_DURATION_DEFAULT_TIMER);
    }
    else
    {
       tone(BUZZER_PIN,TONE_DEFAULT_TIMER,TONE_DURATION_DEFAULT_TIMER);
    }
  }
  else
  {
    tone(BUZZER_PIN,TONE_DEFAULT_TIMER,TONE_DURATION_DEFAULT_TIMER);
  }
  
}

void playBombExplodeTone()
{
  delay(1000);
  tone(BUZZER_KEYPAD_PIN,TONE_TIMER_5/*TONE_ERROR*/,TONE_DURATION_BOMB_LAST);
  delay(TONE_DURATION_BOMB_LAST);
}

void playBombDefusedTone()
{
  for( int i = 0 ; i < TONE_ITER_BOMB_FAILED ; i++ )
  {
    tone(BUZZER_PIN,TONE_TIMER_5,TONE_DURATION_DEFAULT_TIMER);
    delay(200);
    tone(BUZZER_PIN,TONE_TIMER_15,TONE_DURATION_DEFAULT_TIMER);
    delay(200);
    tone(BUZZER_PIN,TONE_TIMER_30,TONE_DURATION_DEFAULT_TIMER);
    delay(200);
    tone(BUZZER_PIN,TONE_TIMER_60,TONE_DURATION_DEFAULT_TIMER);
    delay(200);
    tone(BUZZER_PIN,TONE_DEFAULT_TIMER,TONE_DURATION_DEFAULT_TIMER);
  }
}

/* ******************************************************************************************************************************************************************************** */

// SERIAL_COMS Funzioni comunicazione con la seriale
char serial_check_defuser_state(char oldState)
{
  char defuse_state = oldState;
  if(Serial.available())
  {
    defuse_state = Serial.read();
  }
  return defuse_state;
}

void serial_sendTime(int def_time)
{
  Serial.println(def_time);
}

void serial_sendResponse(char k)
{
  Serial.println(k);
  Serial.flush();
}

bool isBombDefused(char state)
{
  bool result;
  switch(state)
  {
    case DEFUSE_WAITING: serial_sendTime(minutes); result = false; break;
    case DEFUSE_STARTED: serial_sendResponse(DEFUSE_STARTED); 
                         lcd.setCursor(LCD_POS_START,LCD_POS_START); 
                         lcd.print(KEYPAD_DELETE_KEY);
                         result = false;
                         break;
    case DEFUSE_FAILED : 
                         lcd.setCursor(LCD_POS_START,LCD_POS_START); 
                         lcd.print(LCD_SPACE);
                         result = false; 
                         break;
    case DEFUSE_SUCCESS: result = true; break;
    default: result = false; break;
  }
  return result;
}


/* ******************************************************************************************************************************************************************************** */

// ADMIN Funzioni per settaggio di parametri minimi della bomba

void setAdminParameters()
{
  getNewAdminInputs();
  getNewMinTime();
  lcd.clear();
  lcd.setCursor(LCD_POS_START,LCD_POS_START);
  lcd.print("Saving...");
  delay(2500);
  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print("Insert Password:");
  lcd.setCursor(0,1);
  lcd.print(password);
  lcd.setCursor(0,inputedChars);
}

void getNewMinTime()
{
  // Set min_time
  char key;
  char *min_Minutes = (char*) malloc(sizeof(char)*TMI_MIDDLE);
  short int inp_min = 0;
  String temp_MIN_convert;
  lcd.clear();
  lcd.setCursor(LCD_POS_START,LCD_POS_START);
  lcd.print("Insert MIN mins:");  
  do
  {
    key = keypad.getKey();
    
    if(key && !key_check_isNAN(key) && key != KEYPAD_ENTER_KEY)
    {
      tone(BUZZER_KEYPAD_PIN,TONE_DEFAULT,TONE_DURATION_KEYPAD);
      if(key!= KEYPAD_DELETE_KEY)
      {
        if( inp_min < TMI_MIDDLE )
        {
          min_Minutes[inp_min] = key;
          inp_min++;
        }
        else
        {
          tone(BUZZER_KEYPAD_PIN,TONE_ERROR,TONE_DURATION_KEYPAD);
        }
      }
      else
      {
        if(inp_min != 0)
        {
          min_Minutes[inp_min - TMI_OFFSET] = '\0';
          inp_min--;
        }
        else
        {
          tone(BUZZER_KEYPAD_PIN,TONE_ERROR,TONE_DURATION_KEYPAD);
        }
      }
     
    }
     lcd.setCursor(LCD_POS_START,1);
     for(int i = 0 ; i < TMI_MIDDLE ; i++)
     {
       if(min_Minutes[i] >= TMI_CHAR_ZERO && min_Minutes[i] <= TMI_CHAR_NINE)
       {
         lcd.print(min_Minutes[i]);
       }
       else
       {
         lcd.print(LCD_SPACE);
       }
     }  
  }while( key != KEYPAD_ENTER_KEY || inp_min < TMI_MIDDLE );
  temp_MIN_convert.concat(min_Minutes[0]);
  temp_MIN_convert.concat(min_Minutes[1]);
  MIN_minutes = temp_MIN_convert.toInt();
  free(min_Minutes);
  
}

void getNewAdminInputs()
{
  char key;
  short int tmp_inps_to_admin;
  inputedChars = ERROR_VALUE;
  String temp_inputs;
  char *adm_inps = ( char * ) malloc(sizeof(char)*LCD_COLUMNS);
  lcd.clear();
  lcd.setCursor(LCD_POS_START,LCD_POS_START);
  lcd.println("Insert adm input");
  do
  {
    key = keypad.getKey();
    // FUNZIONE DI CONTROLLO ALFANUMERICI
    if(key && !key_check_isNAN(key) && key != KEYPAD_ENTER_KEY)
    {
      
      if( key == KEYPAD_DELETE_KEY )
      {
        lcd.clear();
        removeChar(adm_inps);
        tone(BUZZER_KEYPAD_PIN,TONE_DEFAULT,TONE_DURATION_KEYPAD);
      }
      else if( inputedChars < ADMIN_MAX_DIGITS - 1  )
      {
        lcd.clear();
        addChar(key,adm_inps);
        tone(BUZZER_KEYPAD_PIN,TONE_DEFAULT,TONE_DURATION_KEYPAD);
      }
      temp_inputs = adm_inps;
      tmp_inps_to_admin = temp_inputs.toInt();
      if( inputedChars >= ADMIN_MAX_DIGITS  && tmp_inps_to_admin > ADMIN_MAX_INPUTS)
      {
        lcd.clear();
        lcd.setCursor(LCD_POS_START,LCD_POS_START);
        lcd.println("Max num 255     ");
        tone(BUZZER_KEYPAD_PIN,TONE_ERROR,TONE_DURATION_KEYPAD);
      }
      else
      {
        lcd.clear();
        lcd.setCursor(LCD_POS_START,LCD_POS_START);
        lcd.println("Insert adm input");
      }
      
    }
    if(strlen(adm_inps) > 0 && inputedChars != ERROR_VALUE){
    lcd.setCursor(LCD_POS_START,1);
    lcd.print(adm_inps);
    }
  }while( key != KEYPAD_ENTER_KEY || inputedChars < LCD_POS_START || tmp_inps_to_admin > ADMIN_MAX_INPUTS || tmp_inps_to_admin <= LCD_POS_START);
  temp_inputs = adm_inps;
  inputs_to_admin = temp_inputs.toInt();
  inputedChars = ERROR_VALUE;
  free(adm_inps);
}

/* ******************************************************************************************************************************************************************************** */

// controlli key
bool key_check_isNAN(char key)
{
  switch (key)
  {
    case KEYPAD_A_KEY: 
    case KEYPAD_B_KEY: 
    case KEYPAD_C_KEY: 
    case KEYPAD_D_KEY: return true;
    default: return false;
  }
}

/* ******************************************************************************************************************************************************************************** */
