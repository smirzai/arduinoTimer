#include <LiquidCrystal.h>
#include <DFR_Key.h>


#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5


#define pageNORMAL   0
#define pageMENU1    1
#define pageSETTIME  2
#define pageSETDATE  3
#define pageSETALARMCOUNT 4
#define pageSETTIMERS 5
#define pageTIMER  6

//Pin assignments for SainSmart LCD Keypad Shield
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); 

int curr_page = 0;
int curr_menu = 0;
boolean refresh = 0;
char main_menus[5][11]= { "Set Time", "Set Date", "Set Alarms", "Set timers" };
char main_menus_n = 4;

#define N_TIMERS 5
#define TIMER_STATUS_IDLE  0
#define TIMER_STATUS_RUNNING  1
#define TIMER_STATUS_PAUSED  2

char timer_names[N_TIMERS][10] = {"shiva", "shayan", "timer 1", "timer 2", "timer 3"};
char timers_status[N_TIMERS] = {TIMER_STATUS_IDLE, TIMER_STATUS_IDLE, TIMER_STATUS_IDLE, TIMER_STATUS_IDLE, TIMER_STATUS_IDLE};
unsigned long timers_offset[N_TIMERS] = {0, 0, 0, 0, 0};
unsigned long timers_pause_offset[N_TIMERS] = {0, 0, 0, 0, 0};
char curr_timer = 0;

// read the buttons
int read_LCD_buttons()
{
   int adc_key_in  = 0;

   adc_key_in = analogRead(0);      // read the value from the sensor 
   // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
   // we add approx 50 to those values and check to see if we are close
   if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
   if (adc_key_in < 50)   return btnRIGHT;  
   if (adc_key_in < 195)  return btnUP; 
   if (adc_key_in < 380)  return btnDOWN; 
   if (adc_key_in < 555)  return btnLEFT; 
   if (adc_key_in < 790)  return btnSELECT;   
   return btnNONE;  // when all others fail, return this...
}



int readThermo() {
   long int sensorValue = analogRead(A0);
   return (int) (sensorValue * 5000 / 1023);
}
  

int m, h, s;
int om, oh, os;   // timer offset



void setup() 
{   
  lcd.clear();
  Serial.begin(9600);
  
  m = h = s = 1;
  os = 10;
  om = 1;
  oh = 1;
  
}


void printTime() {
  lcd.setCursor(6, 2);
  char tm[9];
  char *p = tm;
  
  sprintf(tm, "%02d:%02d:%02d", h, m, s);
  
  lcd.print(tm);  
} 

void print_timer() {
  if ((timers_status[curr_timer] == TIMER_STATUS_PAUSED) && !refresh) 
     return;
  refresh = 0;

  lcd.setCursor(2, 2);
  char tm[9];
  char *p = tm;
  
  lcd.print(timer_names[curr_timer]);
  unsigned long tmr = millis() / 1000  - timers_offset[curr_timer];
  char h, m, s;
  s = tmr % 60;
  tmr /= 60;
  m = tmr % 60;
  tmr /= 60;
  h = tmr;
  
  sprintf(tm, "%02d:%02d:%02d", h, m, s);
  lcd.setCursor(8, 2);
  lcd.print(tm);
  
  
  
}

int set_time_digit = 0;
unsigned long set_time_timer = 0;
int set_time_toggle = 0;

void print_set_time() {
  lcd.setCursor(6, 2);
  char tm[9];
  char *p = tm;
  
  sprintf(tm, "%02d:%02d:%02d", h, m, s);
  if (millis() >  set_time_timer + 500) {
    set_time_timer = millis();
    set_time_toggle = !set_time_toggle;
  }
  int digit = set_time_digit;
  if (set_time_digit > 3)
    digit += 2;
  else if (set_time_digit > 1)
    digit ++;
    
  if (set_time_toggle) {
    tm[digit]  = ' ';
  }
  
  
  lcd.print(tm);  
} 


  
void    print_set_timers() {
    if (refresh) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(timer_names[curr_timer]);
      refresh = 0;
  }
}

  
int cnt = 0;
void setTime() {
  unsigned long ms = millis() / 1000;
  
  if (cnt > 100) {
  Serial.println("start");
  Serial.println(ms);
  Serial.println("oh");
  Serial.println(oh);
  

  
  }
  ms += os + om * 60 + ((unsigned long) oh) * 3600;
  if (cnt > 100) {
     Serial.println(ms);
     Serial.println("---------");

  cnt = 0;
  }
  cnt++;
  
  s = ms % 60;
  m = (ms / 60) % 60;
  h = ms / 3600;
}


void printMenu() {
  if (refresh) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(main_menus[curr_menu]);
    refresh = 0;
  }
}

unsigned long key_debounce = 0;


void process_key_normal(int key) {  
    if (key == btnSELECT) {
      curr_page = 1;
      curr_menu=0;
      refresh = 1;
    }
}

void process_key_menu(int key) { 
  if (!key)
    return;
    
  switch (key) {
    case btnUP:
      curr_menu++;
      refresh = 1;
      break;
      
    case btnDOWN:
      curr_menu--;
      refresh = 1;
      break;
      
    case btnSELECT:
      switch (curr_menu)  {
        case 0:
          curr_page = pageSETTIME;
          curr_menu=0;
          refresh = true;
          lcd.clear();
        break;
        
        case 3:
          curr_page = pageSETTIMERS;
          curr_menu=0;
          refresh = true;
          lcd.clear();
         break;
          
     }
     
    
    break;
  }

  if (curr_menu > main_menus_n -1)
     curr_menu = 0;
     
  if (curr_menu < 0)
    curr_menu = main_menus_n - 1;
  
}


void  process_key_settimers(int key) {  
  switch (key) {
    case btnUP:
      curr_timer++;
      refresh = true;
    break;
    case btnDOWN:
      curr_timer--;
      refresh = true;
    break;
    case btnSELECT:
      refresh = true;
      curr_page = pageTIMER;
      if (timers_status[curr_timer] == TIMER_STATUS_IDLE) {
         timers_offset[curr_timer] = millis() / 1000;
         timers_pause_offset[curr_timer] = 0;
      } else if (timers_status[curr_timer] == TIMER_STATUS_PAUSED) {
         timers_offset[curr_timer] += millis() / 1000 - timers_pause_offset[curr_timer];
      }
         
      timers_status[curr_timer] = TIMER_STATUS_RUNNING;
      lcd.clear();
      refresh = 1;
    break;
  }
  if (curr_timer > 4)
    curr_timer = 0;
  if (curr_timer < 0)
    curr_timer = 4;
  
}
void  process_key_timer(int key) {
  switch (key) {
    case btnRIGHT:      
      if (timers_status[curr_timer] == TIMER_STATUS_PAUSED) {        
         timers_offset[curr_timer] += millis() / 1000 - timers_pause_offset[curr_timer]; 
         timers_status[curr_timer]  = TIMER_STATUS_RUNNING;
      } else {
        timers_status[curr_timer]  = TIMER_STATUS_PAUSED;
        timers_pause_offset[curr_timer] = millis() / 1000;
      }
      
    break;
    case btnDOWN:
    break;
    case btnSELECT:
      curr_page = pageNORMAL;
      lcd.clear();
      refresh = 1;
      
    break;
  }
  
}


void process_key_settime(int key) {  
  switch (key) {
    case btnRIGHT:
      set_time_digit++;
    break;
    case btnLEFT:
      set_time_digit--;
    break;
    case btnSELECT:
      curr_page = pageNORMAL;
      lcd.clear();
      refresh = 1;
    break;

    case btnUP:
    case btnDOWN:
    int v;
    v = key == btnUP ? 1 : -1;
    switch (set_time_digit) {
      case 0:
        oh += v * 10;
        break;
      case 1:
        oh += v;
        break;
      case 2:
        om += v * 10;
        break;
      case 3:
        om += v;
        break;
      case 4:
        os += v * 10;
        break;
      case 5:
        os += v;
        break;
    }
    
    break;
  }
  if (set_time_digit > 5)
    set_time_digit = 0;
  if (set_time_digit < 0)
    set_time_digit = 5;
}

int read_key() {
  if (millis() < key_debounce + 150)
     return btnNONE;
     
  key_debounce = millis();
  
  return read_LCD_buttons() ;
}



void loop() 
{  
  setTime();
  int key = read_key();
  switch (curr_page) {    
    case pageNORMAL:
      process_key_normal(key);
      break;
    case pageMENU1:
      process_key_menu(key);  
      break;
    case pageSETTIME:
      process_key_settime(key);
      break;
    case pageSETTIMERS:
      process_key_settimers(key);
      break;
    case pageTIMER:
      process_key_timer(key);
      break;
      
  }

  switch (curr_page) {    
    case pageNORMAL:
      printTime();
      break;
    case pageMENU1:
      printMenu();
      break;
    case pageSETTIME:
      print_set_time();    
      break;
    case pageSETTIMERS:
      print_set_timers();
      break;
    case pageTIMER:
      print_timer();
      break;
  }
   
}

