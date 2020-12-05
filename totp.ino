#include <Time.h>
#include <TimeLib.h>
#include <LiquidCrystal.h>
#include "base32.h"
#include "sha1.h"
//#include "hmac.h"
#include "buildTime.h"
#include "MemoryFree.h"


LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const auto timeZone = 5;
long int currentTime; 
long int lastTime = 0;
String secret = "HGIJX2YH5IMJNRHP";
String code = "";  
auto bRightPin = 44;
auto bLeftPin = 46;
auto bChScrPin = 48;
auto soundPin = A0;
auto bChScrState = 0;

void setup() {
  Serial.begin(9600);  
  pinMode(bChScrPin,INPUT);  
  pinMode(bLeftPin,INPUT);  
  pinMode(bRightPin,INPUT); 
  pinMode(soundPin,OUTPUT); 
  lcd.begin(16, 2);
  setTime(BUILD_HOUR,BUILD_MIN,BUILD_SEC,BUILD_DAY,BUILD_MONTH,BUILD_YEAR);  
  setTime(now()-timeZone*3600+7);
  currentTime = now();
}
int freeRam() {
    extern int __heap_start, * __brkval;
    int v;
    return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}
String HMAC(String& msg, String K, String(*H)(String text), const int b) {

    
    
  
    if (K.length() > b)              //Если ключ больше размера блока, укорачиваем по хешу
        K = H(K);
    while (K.length() < b)           //Если меньше, дополняем нулями
        K += '\0';
    String k;
    auto tmp = K.c_str();                     //Копия ключа для будущих вычислений
    for (auto i = 0; i < b; i++)
        k+= char(tmp[i] ^ 0x36);
  
    Serial.println("\nk: ");
    for (auto i = 0; i < b; i++){
        Serial.print(int(k[i])); Serial.print(' ');
}Serial.println("\nmsg is:");
    for (auto i = 0; i < msg.length(); i++) {
        Serial.print(int(msg[i])); Serial.print(' ');
        k+=msg[i];
    }
  
 Serial.println("\nkmsg is:");
 for (auto i : k) {
     Serial.print(int(i)); Serial.print(' ');
 }
    String buf = H(k);
    Serial.println("buf is: "+ buf);

    for (auto i = 0; i < b; i++)
        K[i] ^= 0x5c;

    //Конв. строку hex символов в строку байт
    for (auto i = 0; i < buf.length(); i++) {
        buf[i] = (uint8_t)strtol(buf.substring(i, 2).c_str(), 0, 16);
        buf.remove(i + 1, 1);
    }

    return H(K + buf);
}

String Cast2Digs(int number){
  return number>9?String(number):'0'+String(number);
}

void CodeScreen(){
  if(currentTime != lastTime) {
    lastTime = currentTime;
    tone(soundPin,700,100); 
    String time_value = "00000000";
    for (int i = 0; i < 8; i++) {      
      time_value[7 - i] = char(currentTime);
      Serial.print(int(time_value[7-i])); Serial.print(' ');
      currentTime /= 256;      
    }Serial.println();
    code = HMAC(time_value, base32decode(secret), sha1, 64);
  }
  lcd.setCursor(0, 0);
  lcd.print("Code:     ");//+code);
  lcd.setCursor(0, 1);
  lcd.print("Left:     ");
  long int leftSec = 30-now()%30;
  lcd.print(Cast2Digs(leftSec) + " sec");
}
  
void TimeScreen(){
  auto l = digitalRead(bLeftPin)==HIGH;
  auto r = digitalRead(bRightPin)==HIGH;
  if(l) {
    adjustTime(-1);
    tone(soundPin,400,100);
  }
  else if(r) {    
    adjustTime(1);
    tone(soundPin,400,100);    
  }         
  adjustTime(timeZone*3600);
  lcd.setCursor(0, 0);
  lcd.print("Date: "+String(year())+'.'+Cast2Digs(month())+'.'+Cast2Digs(day()));
  lcd.setCursor(0, 1);
  lcd.print("Time:   "+Cast2Digs(hour())+':'+Cast2Digs(minute())+':'+Cast2Digs(second()));
  adjustTime(-timeZone*3600);
  }

void loop() { 
  currentTime=now()/30;
  if(digitalRead(bChScrPin)==HIGH) {  //Если переключили экран
    tone(soundPin,256,256);
    bChScrState++;                    //Изменяем состояние экрана (всего 3)
    if(bChScrState==3)                //Если сдвинули экран к крайнему, возвращаемся
      bChScrState=0;
    lcd.clear();
    delay(250);
  }
  if(millis()%100==0){                //Каждые 100мс обновляем экраны
    lcd.clear();
    if(bChScrState == 0)         
        CodeScreen();                
    else if(bChScrState==1)
      TimeScreen();    
    else
      bChScrState=0;
  }
}
