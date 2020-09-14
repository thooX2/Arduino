//**********************************************************************************************************************//
//                                                                                                                      //
//***************************************************Code Start Line****************************************************//
//                                                                                                                      //
//**********************************************************************************************************************//



//**********************************************************************************************************************//
//                                                                                                                      //
//*****************************************************Header Files*****************************************************//
//                                                                                                                      //
//**********************************************************************************************************************//

//I2C LCD 관련 헤더파일
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//DHT11 온습도 센서 관련 헤더파일
#include <DHT11.h>

//MQ7 일산화탄소 센서 관련 헤더파일
#include <MQ7.h>

//내장 기억장치 관련 헤더파일
#include <EEPROM.h>

//**********************************************************************************************************************//
//                                                                                                                      //
//******************************************************Pin Number******************************************************//
//                                                                                                                      //
//**********************************************************************************************************************//

#define DHT11_PIN 2 //온습도 센서 핀 정의

#define RED_PIN 9 //RED LED 핀 정의
#define GREEN_PIN 10 //GREEN LED 핀 정의
#define BLUE_PIN 11 //BLUE LED 핀 정의

#define BUZZER_PIN 13 //피에조 버저 핀 정의

#define MQ7_PIN A1 //일산화탄소 센서 핀 정의

#define UP_BTN_PIN 3 //버튼 핀 정의
#define DOWN_BTN_PIN 4
#define LEFT_BTN_PIN 5
#define RIGHT_BTN_PIN 6
#define SET_BTN_PIN 7
//#define POWER_BTN_PIN 8

//**********************************************************************************************************************//
//                                                                                                                      //
//************************************************Functions Declaration*************************************************//
//                                                                                                                      //
//**********************************************************************************************************************//

void LED_Color(int,int,int); //LED 색상 변환 함수

void Alarm(float); //일산화탄소 농도 경보 함수

void Auto_Mode(void); //LED Mode 변환 함수
void White_Mode(void);
//void Rainbow_Mode(void); // Kill
void Red_Mode(void);
void Orange_Mode(void);
void Yellow_Mode(void);
void Green_Mode(void);
void Sky_blue_Mode(void);
void Blue_Mode(void);
void Magenta_Mode(void);
void Off_Mode(void);

void Execute_LED_Mode(void); //LED Mode 실행 함수

void Setting_LED_Mode(void); //LED Mode 설정 함수

//void Power_Control_Mode(void);

//**********************************************************************************************************************//
//                                                                                                                      //
//************************************************Variables Declaration*************************************************//
//                                                                                                                      //
//**********************************************************************************************************************//

LiquidCrystal_I2C lcd(0x27,16,2);  //LCD 주소 할당 

DHT11 dht11(DHT11_PIN); //온습도 클래스 객체 생성

MQ7 mq7(MQ7_PIN, 5.0); //일산화탄소 클래스 객체 생성

byte Celsius[8] = //섭씨 특수문자 변수
{
  B10000,
  B00110,
  B01001,
  B01000,
  B01000,
  B01001,
  B00110,
  B00000
};

float Temporature, Humidity; //온도, 습도 변수
float ppm; //일산화탄소 농도 변수

unsigned long Previous_Measure_Time = 0; //과거 측정 시간 기록 변수
unsigned long Current_Measure_Time; //현재 측정 시간 기록 변수
unsigned long Previous_Main_BTN_Time = 0; //과거 Loop 함수 내 버튼 클릭 측정 시간 기록 변수
unsigned long Current_Main_BTN_Time; //현재 Loop 함수 내 버튼 클릭 측정 시간 기록 변수 
unsigned long Previous_Set_BTN_Time = 0; //과거 Setting_LED_Mode 함수 내 버튼 클릭 측정 시간 기록 변수
unsigned long Current_Set_BTN_Time; //현재 Setting_LED_Mode 함수 내 버튼 클릭 측정 시간 기록 변수

char LED_Mode_Flag = 'A'; //LED Mode 플래그
int Setting_LED_Mode_Control_Flag = 0; //Setting_LED_Mode 함수 내 통제 플래그
bool Setting_LED_Mode_Finish_Flag = false; //Setting_LED_Mode 함수 내 종료 플래그

bool UP_BTN_Flag = false; //버튼 클릭 플래그
bool DOWN_BTN_Flag = false;
bool LEFT_BTN_Flag = false;
bool RIGHT_BTN_Flag = false;
bool SET_BTN_Flag = false;
//bool POWER_BTN_Flag = false;

//**********************************************************************************************************************//
//                                                                                                                      //
//********************************************************Setup*********************************************************//
//                                                                                                                      //
//**********************************************************************************************************************//

void setup()
{
  Serial.begin(9600); //시리얼 통신 시작

  pinMode(RED_PIN, OUTPUT);  //RED LED 출력
  pinMode(GREEN_PIN, OUTPUT);   //GREEN LED 출력
  pinMode(BLUE_PIN, OUTPUT);   //BLUE LED 출력

  pinMode(MQ7_PIN, INPUT); //일산화탄소 센서 입력

  pinMode(BUZZER_PIN, OUTPUT); //피에조 버저 입력

  pinMode(UP_BTN_PIN, INPUT_PULLUP); //버튼 입력
  pinMode(DOWN_BTN_PIN, INPUT_PULLUP);
  pinMode(LEFT_BTN_PIN, INPUT_PULLUP);
  pinMode(RIGHT_BTN_PIN, INPUT_PULLUP);
  pinMode(SET_BTN_PIN, INPUT_PULLUP);
  //pinMode(POWER_BTN_PIN, INPUT_PULLUP);

  digitalWrite(RED_PIN, HIGH);   //RED PIN OFF (RGB LED가 Anode Type이므로 값이 HIGH인 경우 꺼지고 LOW인 경우 켜진다.)
  digitalWrite(GREEN_PIN, HIGH);  //GREEN PIN OFF
  digitalWrite(BLUE_PIN, HIGH); //BLUE PIN OFF

  lcd.begin(); //LCD 시작
  lcd.backlight(); //LCD backlight ON
  lcd.createChar(0,Celsius); //LCD에 섭씨 특수문자 생성
  lcd.clear(); //LCD 출력 문자 제거
  lcd.print("  CO LED Alarm"); //LCD 문자 출력
  lcd.setCursor(0,1); //LCD 커서 아래로 이동
  lcd.print("   Loading..."); //LCD 문자 출력
  delay(3000); //3초 대기

  dht11.read(Humidity,Temporature);  //온도, 습도 읽기
  ppm=mq7.getPPM(); //일산화탄소 농도 읽기
  lcd.clear(); //LCD 출력 문자 제거
  lcd.print(Temporature); //LCD 온도 출력
  lcd.write(byte(0)); //LCD 섭씨 특수문자 출력
  lcd.print(" / ");
  lcd.print(Humidity); //LCD 습도 출력
  lcd.print("%");
  lcd.setCursor(0,1); //LCD 커서 아래로 이동
  lcd.print("CO : "); //LCD 일산화탄소 농도 출력
  lcd.print(ppm);
  lcd.print("ppm");
  LED_Mode_Flag = (char)(EEPROM.read(0)); //과거 LED_Mode_Flag를 읽어 과거 LED Mode 유지
}

//**********************************************************************************************************************//
//                                                                                                                      //
//*********************************************************Loop*********************************************************//
//                                                                                                                      //
//**********************************************************************************************************************//

void loop()
{
  Execute_LED_Mode(); //LED Mode 실행
  if(digitalRead(SET_BTN_PIN) == LOW) //Set 버튼이 눌렸다면 Set 버튼 플래그 값 변환
  {
    SET_BTN_Flag = true;
  }
  /*if(digitalRead(POWER_BTN_PIN) == LOW)
  {
    POWER_BTN_Flag = true;
  }*/

  Current_Measure_Time = millis(); //현재 측정 시간 기록
  if(Current_Measure_Time - Previous_Measure_Time > 5000) //과거 측정 시간과 현재 측정 시간의 차가 5초 이상인 경우
  {
    dht11.read(Humidity,Temporature);  //온도, 습도 읽기
    ppm=mq7.getPPM(); //일산화탄소 농도 읽기
    lcd.clear(); //LCD 출력 문자 제거
    lcd.print(Temporature); //LCD 온도 출력
    lcd.write(byte(0)); //LCD 섭씨 특수문자 출력
    lcd.print(" / ");
    lcd.print(Humidity); //LCD 습도 출력
    lcd.print("%");
    lcd.setCursor(0,1); //LCD 커서 아래로 이동
    lcd.print("CO : "); //LCD 일산화탄소 농도 출력
    lcd.print(ppm);
    lcd.print("ppm");
    Previous_Measure_Time = Current_Measure_Time; //과거 측정 시간을 현재 측정 시간으로 기록
  }
  
  Current_Main_BTN_Time = millis(); //현재 Loop 함수 내 버튼 클릭 시간 기록 
  if(Current_Main_BTN_Time - Previous_Main_BTN_Time > 1000) //과거 Loop 함수 내 버튼 클릭 시간과 현재 Loop 함수 내 버튼 클릭 시간의 차가 1초 이상인 경우
  {
    if(SET_BTN_Flag == true) //Set 버튼 플래그가 true인 경우
    {
      SET_BTN_Flag = false; //Set 버튼 플래그를 원 상태인 false로 변환
      Setting_LED_Mode_Finish_Flag = false; //Setting_LED_Mode 함수 내 종료 플래그를 false로 변환
      Setting_LED_Mode(); //Setting_LED_Mode 함수 실행
    }
    /*else if(POWER_BTN_Flag == true)
    {
      POWER_BTN_Flag = false;
      Power_Control_Mode();
    }*/
  }
}

//**********************************************************************************************************************//
//                                                                                                                      //
//*************************************************Functions Definition*************************************************//
//                                                                                                                      //
//**********************************************************************************************************************//



//**********************************************************************************************************************//
//                                                                                                                      //
//********************************************LED_Color Function Definition*********************************************//
//                                                                                                                      //
//**********************************************************************************************************************//

void LED_Color(int RED, int GREEN, int BLUE) // RGB LED 색상 및 밝기 세팅
{
  analogWrite(RED_PIN, 255 - RED); //RGB LED 내의 RED 색상의 밝기를 변환
  analogWrite(GREEN_PIN, 255 - GREEN); //RGB LED 내의 GREEN 색상의 밝기를 변환
  analogWrite(BLUE_PIN, 255 - BLUE);  //RGB LED 내의 BLUE 색상의 밝기를 변환
}

//**********************************************************************************************************************//
//                                                                                                                      //
//**********************************************Alarm Function Definition***********************************************//
//                                                                                                                      //
//**********************************************************************************************************************//

void Alarm(float ppm) // 일정한 수치의 일산화탄소 농도가 넘으면 RGB LED(시각)와 피에조 버저(청각)를 이용하여 경보
{
  if(200>ppm) //일산화탄소 농도가 50ppm 이상 200ppm 미만일 경우 점멸 효과
  {
    LED_Color(255,0,0); //RGB LED를 적색으로 변환
    tone(13,62); //피에조 버저의 음높이를 62(1옥타브 시)로 설정하여 소리 발생
    delay(1200); //1.2초간 지연 시간 발생
    LED_Color(0,0,0); //RGB LED를 끄기
    noTone(13); //피에조 버저 소리 끄기
    delay(1200); //1.2초간 지연 시간 발생
  }
  else if(200<=ppm && 400>ppm) //일산화탄소 농도가 200ppm 이상 400ppm 미만일 경우 점멸 효과
  {
    for(int i=0; i<2; i++) //타 점멸 효과와 점멸이 발생하는 시간을 동기화하기 위하여 반복 (2.4초 = 1.2초 * 2번)
    {
      LED_Color(255,0,0); //RGB LED를 적색으로 변환
      tone(13,123); //피에조 버저의 음높이를 123(2옥타브 시)로 설정하여 소리 발생
      delay(600); //0.6초간 지연 시간 발생
      LED_Color(0,0,0); //RGB LED를 끄기
      noTone(13); //피에조 버저 소리 끄기
      delay(600); //0.6초간 지연 시간 발생
    }
  }
  else if(400<=ppm && 800>ppm) //일산화탄소 농도가 400ppm 이상 800ppm 미만일 경우 점멸 효과
  {
    for(int i=0; i<3; i++) //타 점멸 효과와 점멸이 발생하는 시간을 동기화하기 위하여 반복 (2.4초 = 0.8초 * 3번)
    {
      LED_Color(255,0,0); //RGB LED를 적색으로 변환
      tone(13,247); //피에조 버저의 음높이를 247(3옥타브 시)로 설정하여 소리 발생
      delay(400); //0.4초간 지연 시간 발생
      LED_Color(0,0,0); //RGB LED를 끄기
      noTone(13); //피에조 버저 소리 끄기
      delay(400); //0.4초간 지연 시간 발생
    }    
  }
  else if(800<=ppm && 1600>ppm) //일산화탄소 농도가 800ppm 이상 1600ppm 미만일 경우 점멸 효과
  {
    for(int i=0; i<4; i++) //타 점멸 효과와 점멸이 발생하는 시간을 동기화하기 위하여 반복 (2.4초 = 0.6초 * 4번)
    {
      LED_Color(255,0,0); //RGB LED를 적색으로 변환
      tone(13,494); //피에조 버저의 음높이를 494(4옥타브 시)로 설정하여 소리 발생
      delay(300); //0.3초간 지연 시간 발생
      LED_Color(0,0,0); //RGB LED를 끄기
      noTone(13); //피에조 버저 소리 끄기
      delay(300); //0.3초간 지연 시간 발생
    }    
  }
  else if(1600<=ppm && 3200>ppm) //일산화탄소 농도가 1600ppm 이상 3200ppm 미만일 경우 점멸 효과
  {
    for(int i=0; i<5; i++) //타 점멸 효과와 점멸이 발생하는 시간을 동기화하기 위하여 반복 (2.4초 = 0.48초 * 5번)
    {
      LED_Color(255,0,0); //RGB LED를 적색으로 변환
      tone(13,988); //피에조 버저의 음높이를 988(5옥타브 시)로 설정하여 소리 발생
      delay(240); //0.24초간 지연 시간 발생
      LED_Color(0,0,0); //RGB LED를 끄기
      noTone(13); //피에조 버저 소리 끄기
      delay(240); //0.24초간 지연 시간 발생
    }
  }
  else if(3200<=ppm) //일산화탄소 농도가 3200ppm 이상일 경우 점멸 효과
  {
    for(int i=0; i<6; i++) //타 점멸 효과와 점멸이 발생하는 시간을 동기화하기 위하여 반복 (2.4초 = 0.4초 * 6번)
    {
      LED_Color(255,0,0); //RGB LED를 적색으로 변환
      tone(13,1976); //피에조 버저의 음높이를 1976(6옥타브 시)로 설정하여 소리 발생
      delay(200); //0.2초간 지연 시간 발생
      LED_Color(0,0,0); //RGB LED를 끄기
      noTone(13); //피에조 버저 소리 끄기
      delay(200); //0.2초간 지연 시간 발생
    }
  }
}

//**********************************************************************************************************************//
//                                                                                                                      //
//********************************************Auto_Mode Function Definition*********************************************//
//                                                                                                                      //
//**********************************************************************************************************************//

void Auto_Mode()
{
  LED_Mode_Flag = 'A'; //LED_Mode 플래그를 A로 변환
  if(50<=ppm) //일산화탄소 농도가 50ppm 이상인 경우
  {
    Alarm(ppm); //일산화탄소 경보 함수 실행
  }
  else if(ppm<30) //일산화탄소 농도가 30ppm 미만인 경우
  {
    LED_Color(0,255,255); //RGB LED를 하늘색으로 변환
  }
  else if(30<=ppm && 40>ppm) //일산화탄소 농도가 30ppm 이상 40ppm 미만인 경우
  {
    LED_Color(0,255,0); //RGB LED를 초록색으로 변환
  }
  else if(40<=ppm && 50>ppm) //일산화탄소 농도가 40ppm 이상 50ppm 미만인 경우
  {
    LED_Color(255,169,0); //RGB LED를 주황색으로 변환
  }
}

//**********************************************************************************************************************//
//                                                                                                                      //
//********************************************White_Mode Function Definition********************************************//
//                                                                                                                      //
//**********************************************************************************************************************//

void White_Mode()
{
  LED_Mode_Flag = 'W'; //LED_Mode 플래그를 W로 변환
  if(50<=ppm) //일산화탄소 농도가 50ppm 이상인 경우
  {
    Alarm(ppm); //일산화탄소 경보 함수 실행
  }
  else if(50>ppm) //일산화탄소 농도가 50ppm 미만인 경우
  {
    LED_Color(255,255,255); //RGB LED를 흰색으로 변환
  }
}

//**********************************************************************************************************************//
//                                                                                                                      //
//*******************************************Rainbow_Mode Function Definition*******************************************//
//                                                                                                                      //
//**********************************************************************************************************************//

/*void Rainbow_Mode()
{
  LED_Mode_Flag = 'R';
  if(50<=ppm)
  {
    Alarm(ppm);
  }
  else if(50>ppm)
  {  
    for(int i=0; i<256; i++)
    {
      LED_Color(255,i,0);
      delay(7.8125);
    }
    for(int i=255; i>=0; i--)
    {
      LED_Color(i,255,0);
      delay(7.8125);
    }
    for(int i=0; i<256; i++)
    {
      LED_Color(0,255,i);
      delay(7.8125);
    }
    for(int i=255; i>=0; i--)
    {
      LED_Color(0,i,255);
      delay(7.8125);
    }
    for(int i=0; i<256; i++)
    {
      LED_Color(i,0,255);
      delay(7.8125);
    }
    for(int i=255; i>=0; i--)
    {
      LED_Color(255,0,i);
      delay(7.8125);
    }
  }
}*/

//**********************************************************************************************************************//
//                                                                                                                      //
//*********************************************Red_Mode Function Definition*********************************************//
//                                                                                                                      //
//**********************************************************************************************************************//

void Red_Mode()
{
  LED_Mode_Flag = 'R'; //LED_Mode 플래그를 R로 변환
  if(50<=ppm)
  {
    Alarm(ppm);
  }
  else if(50>ppm)
  {
    LED_Color(255,0,0); //RGB LED를 적색으로 변환
  }
}

//**********************************************************************************************************************//
//                                                                                                                      //
//*******************************************Orange_Mode Function Definition********************************************//
//                                                                                                                      //
//**********************************************************************************************************************//

void Orange_Mode()
{
  LED_Mode_Flag = 'E'; //LED_Mode 플래그를 E로 변환
  if(50<=ppm)
  {
    Alarm(ppm);
  }
  else if(50>ppm)
  {
    LED_Color(255,128,0); //RGB LED를 주황색으로 변환
  }
}

//**********************************************************************************************************************//
//                                                                                                                      //
//*******************************************Yellow_Mode Function Definition********************************************//
//                                                                                                                      //
//**********************************************************************************************************************//

void Yellow_Mode()
{
  LED_Mode_Flag = 'Y'; //LED_Mode 플래그 Y로 변환
  if(50<=ppm)
  {
    Alarm(ppm);
  }
  else if(50>ppm)
  {
    LED_Color(255,255,0); //RGB LED를 노란색으로 변환
  }
}

//**********************************************************************************************************************//
//                                                                                                                      //
//********************************************Green_Mode Function Definition********************************************//
//                                                                                                                      //
//**********************************************************************************************************************//

void Green_Mode()
{
  LED_Mode_Flag = 'G'; //LED_Mode 플래그를 G로 변환
  if(50<=ppm)
  {
    Alarm(ppm);
  }
  else if(50>ppm)
  {
    LED_Color(0,255,0); //RGB LED를 녹색으로 변환
  }
}

//**********************************************************************************************************************//
//                                                                                                                      //
//*******************************************Sky_blue_Mode Function Definition******************************************//
//                                                                                                                      //
//**********************************************************************************************************************//

void Sky_blue_Mode()
{
  LED_Mode_Flag = 'S'; //LED_Mode 플래그를 S로 변환
  if(50<=ppm)
  {
    Alarm(ppm);
  }
  else if(50>ppm)
  {
    LED_Color(0,255,255); //RGB LED를 하늘색으로 변환
  }
}

//**********************************************************************************************************************//
//                                                                                                                      //
//********************************************Blue_Mode Function Definition*********************************************//
//                                                                                                                      //
//**********************************************************************************************************************//

void Blue_Mode()
{
  LED_Mode_Flag = 'B'; //LED_Mode 플래그를 B로 변환
  if(50<=ppm)
  {
    Alarm(ppm);
  }
  else if(50>ppm)
  {
    LED_Color(0,0,255); //RGB LED를 파란색으로 변환
  }
}

//**********************************************************************************************************************//
//                                                                                                                      //
//*******************************************Magenta_Mode Function Definition*******************************************//
//                                                                                                                      //
//**********************************************************************************************************************//

void Magenta_Mode()
{
  LED_Mode_Flag = 'M'; //LED_Mode 플래그를 M으로 변환
  if(50<=ppm)
  {
    Alarm(ppm);
  }
  else if(50>ppm)
  {
    LED_Color(255,0,255); //RGB LED를 자주색으로 변환
  }
}

//**********************************************************************************************************************//
//                                                                                                                      //
//*********************************************Off_Mode Function Definition*********************************************//
//                                                                                                                      //
//**********************************************************************************************************************//

void Off_Mode()
{
  LED_Mode_Flag = 'O'; //LED_Mode 플래그를 O로 변환
  if(50<=ppm)
  {
    Alarm(ppm);
  }
  else if(50>ppm)
  {
    LED_Color(0,0,0); //RGB LED를 끄기
  }
}

//**********************************************************************************************************************//
//                                                                                                                      //
//*****************************************Execute_LED_Mode Function Definition*****************************************//
//                                                                                                                      //
//**********************************************************************************************************************//

void Execute_LED_Mode()
{
  switch(LED_Mode_Flag)
  {
    case 'A': //LED_Mode 플래그가 A인 경우
      EEPROM.write(0,'A'); //내장 메모리의 0번 주소에 A라는 값을 삽입 (내장 메모리는 숫자 밖에 기억을 못하므로 A의 아스키 코드 값인 65로 저장됨)
      Auto_Mode(); //Auto_Mode 실행
      break;
    case 'W': //LED_Mode 플래그가 W인 경우
      EEPROM.write(0,'W'); //내장 메모리의 0번 주소에 W라는 값을 삽입
      White_Mode(); //White_Mode 실행
      break;
    /*case 'R':
      Rainbow_Mode();
      break;*/
    case 'R': //LED_Mode 플래그가 R인 경우
      EEPROM.write(0,'R'); //내장 메모리의 0번 주소에 R라는 값을 삽입
      Red_Mode(); //Red_Mode 실행
      break;
    case 'E': //LED_Mode 플래그가 E인 경우
      EEPROM.write(0,'E'); //내장 메모리의 0번 주소에 E라는 값을 삽입
      Orange_Mode(); //Orange_Mode 실행
      break;
    case 'Y': //LED_Mode 플래그가 Y인 경우
      EEPROM.write(0,'Y'); //내장 메모리의 0번 주소에 Y라는 값을 삽입
      Yellow_Mode(); //Yellow_Mode 실행
      break;
    case 'G': //LED_Mode 플래그가 G인 경우
      EEPROM.write(0,'G'); //내장 메모리의 0번 주소에 G라는 값을 삽입
      Green_Mode(); //Green_Mode 실행
      break;
    case 'S': //LED_Mode 플래그가 S인 경우
      EEPROM.write(0,'S'); //내장 메모리의 0번 주소에 S라는 값을 삽입
      Sky_blue_Mode(); //Sky_blue_Mode 실행
      break;
    case 'B': //LED_Mode 플래그가 B인 경우
      EEPROM.write(0,'B'); //내장 메모리의 0번 주소에 B라는 값을 삽입
      Blue_Mode(); //Blue_Mode 실행
      break;
    case 'M': //LED_Mode 플래그가 M인 경우
      EEPROM.write(0,'M'); //내장 메모리의 0번 주소에 M이라는 값을 삽입
      Magenta_Mode(); //Magenta_Mode 실행
      break;
    case 'O': //LED_Mode 플래그가 O인 경우
      EEPROM.write(0,'O'); //내장 메모리의 0번 주소에 O라는 값을 삽입
      Off_Mode(); //Off_Mode 실행
      break;
    default:
      break;
  }
}

//**********************************************************************************************************************//
//                                                                                                                      //
//*****************************************Setting_LED_Mode Function Definition*****************************************//
//                                                                                                                      //
//**********************************************************************************************************************//

void Setting_LED_Mode()
{
  Setting_LED_Mode_Control_Flag = 0; //Setting_LED_Mode 통제 플래그를 0으로 변환
  while(Setting_LED_Mode_Finish_Flag == false) //Setting_LED_Mode 종료 플래그가 false인 경우
  {
    Previous_Set_BTN_Time = millis(); //과거 Setting_LED_Mode 함수 내 버튼 클릭 시간 기록
    switch(Setting_LED_Mode_Control_Flag)
    {
      case 0: //Setting_LED_Mode 통제 플래그가 0인 경우
        lcd.clear(); //LCD 출력 문자 제거
        lcd.print("Setting LED Mode"); //LCD 문자 출력
        lcd.setCursor(0,1); //LCD 커서 아래로 이동
        lcd.print(">Auto Mode"); //LCD 문자 출력
        break;
      case 1:
        lcd.clear();
        lcd.print(" Auto Mode");
        lcd.setCursor(0,1);
        lcd.print(">White Mode");
        break;
      /*case 2:
        lcd.clear();
        lcd.print(" Light Mode");
        lcd.setCursor(0,1);
        lcd.print(">Rainbow Mode");
        break;*/
      case 2:
        lcd.clear();
        lcd.print(" White Mode");
        lcd.setCursor(0,1);
        lcd.print(">Red Mode");
        break;
      case 3:
        lcd.clear();
        lcd.print(" Red Mode");
        lcd.setCursor(0,1);
        lcd.print(">Orange Mode");
        break;
      case 4:
        lcd.clear();
        lcd.print(" Orange Mode");
        lcd.setCursor(0,1);
        lcd.print(">Yellow Mode");
        break;
      case 5:
        lcd.clear();
        lcd.print(" Yellow Mode");
        lcd.setCursor(0,1);
        lcd.print(">Green Mode");
        break;
      case 6:
        lcd.clear();
        lcd.print(" Green Mode");
        lcd.setCursor(0,1);
        lcd.print(">Sky blue Mode");
        break;
      case 7:
        lcd.clear();
        lcd.print(" Sky blue Mode");
        lcd.setCursor(0,1);
        lcd.print(">Blue Mode");
        break;
      case 8:
        lcd.clear();
        lcd.print(" Blue Mode");
        lcd.setCursor(0,1);
        lcd.print(">Magenta Mode");
        break;
      case 9:
        lcd.clear();
        lcd.print(" Magenta Mode");
        lcd.setCursor(0,1);
        lcd.print(">Off Mode");
        break;      
      default:
        break;
    }
    delay(500); //0.5초 대기
    
    if(digitalRead(UP_BTN_PIN) == LOW) //UP 버튼이 눌렸다면 UP 버튼 플래그 값 변환
    {
      UP_BTN_Flag = true;
    }
    if(digitalRead(DOWN_BTN_PIN) == LOW) //DOWN 버튼이 눌렸다면 DOWN 버튼 플래그 값 변환
    {
      DOWN_BTN_Flag = true;
    }
    if(digitalRead(LEFT_BTN_PIN) == LOW) //LEFT 버튼이 눌렸다면 LEFT 버튼 플래그 값 변환
    {
      LEFT_BTN_Flag = true;
    }
    if(digitalRead(RIGHT_BTN_PIN) == LOW) //RIGHT 버튼이 눌렸다면 RIGHT 버튼 플래그 값 변환
    {
      RIGHT_BTN_Flag = true;
    }
    
    Current_Set_BTN_Time = millis(); //현재 Setting_LED_Mode 함수 내 버튼 클릭 시간 기록
    if(Current_Set_BTN_Time - Previous_Set_BTN_Time > 500) //과거 Setting_LED_Mode 함수 내 버튼 클릭 시간과 현재 Setting_LED_Mode 함수 내 버튼 클릭 시간의 차가 0.5초 이상인 경우
    {
      if(UP_BTN_Flag == true) //UP 버튼 플래그가 true인 경우
      {        
        UP_BTN_Flag = false; //UP 버튼 플래그를 false로 변환
        if(Setting_LED_Mode_Control_Flag <= 0) //Setting_LED_Mode 통제 플래그가 0과 같거나 작은 경우
        {
          Setting_LED_Mode_Control_Flag = 10; //Setting_LED_Mode 통제 플래그를 10으로 변환
        }
        Setting_LED_Mode_Control_Flag--; //Setting_LED_Mode 통제 플래그를 1 감소
        Previous_Set_BTN_Time = Current_Set_BTN_Time; //과거 Setting_LED_Mode 함수 내 버튼 클릭 시간을 현재 Setting_LED_Mode 함수 내 버튼 클릭 시간으로 기록
      }
      else if(DOWN_BTN_Flag == true) //DOWN 버튼 플래그가 true인 경우
      {
        DOWN_BTN_Flag = false; //DOWN 버튼 플래그를 false로 변환
        if(Setting_LED_Mode_Control_Flag >= 9) //Setting_LED_Mode 통제 플래그가 9와 같거나 큰 경우
        {
          Setting_LED_Mode_Control_Flag = -1; //Setting_LED_Mode 통제 플래그를 -1로 변환
        }
        Setting_LED_Mode_Control_Flag++; //Setting_LED_Mode 통제 플래그를 1 증가
        Previous_Set_BTN_Time = Current_Set_BTN_Time; //과거 Setting_LED_Mode 함수 내 버튼 클릭 시간을 현재 Setting_LED_Mode 함수 내 버튼 클릭 시간으로 기록
      }
      else if(LEFT_BTN_Flag == true) //LEFT 버튼 플래그가 true인 경우
      {
        LEFT_BTN_Flag = false; //LEFT 버튼 플래그를 false로 변환
        Setting_LED_Mode_Finish_Flag = true; //Setting_LED_Mode 종료 플래그를 true로 변환
        Previous_Set_BTN_Time = Current_Set_BTN_Time; //과거 Setting_LED_Mode 함수 내 버튼 클릭 시간을 현재 Setting_LED_Mode 함수 내 버튼 클릭 시간으로 기록
        return;      
      }
      else if(RIGHT_BTN_Flag == true) //RIGHT 버튼 플래그가 true인 경우
      {
        RIGHT_BTN_Flag = false; //RIGHT 버튼 플래그를 false로 변환
        switch(Setting_LED_Mode_Control_Flag)
        {
          case 0: //Setting_LED_Mode 통제 플래그가 0인 경우
            LED_Mode_Flag = 'A'; //LED_Mode 플래그를 A로 변환
            break;
          case 1:
            LED_Mode_Flag = 'W';
            break;
          /*case 2:
            LED_Mode_Flag = 'R';
            break;*/
          case 2:
            LED_Mode_Flag = 'R';
            break;
          case 3:
            LED_Mode_Flag = 'E';
            break;
          case 4:
            LED_Mode_Flag = 'Y';
            break;
          case 5:
            LED_Mode_Flag = 'G';
            break;
          case 6:
            LED_Mode_Flag = 'S';
            break;
          case 7:
            LED_Mode_Flag = 'B';
            break;
          case 8:
            LED_Mode_Flag = 'M';
            break;
          case 9:
            LED_Mode_Flag = 'O';
            break;
          default:
            break;
        }
        Setting_LED_Mode_Finish_Flag = true; //Setting_LED_Mode 종료 플래그를 true로 변환
        Previous_Set_BTN_Time = Current_Set_BTN_Time; //과거 Setting_LED_Mode 함수 내 버튼 클릭 시간을 현재 Setting_LED_Mode 함수 내 버튼 클릭 시간으로 기록
        return;
      }
    }
  }
}

//**********************************************************************************************************************//
//                                                                                                                      //
//****************************************Power_Control_Mode Function Definition****************************************//
//                                                                                                                      //
//**********************************************************************************************************************//

/*void Power_Control_Mode()
{

}*/



//**********************************************************************************************************************//
//                                                                                                                      //
//***************************************************Code Finish Line***************************************************//
//                                                                                                                      //
//**********************************************************************************************************************//
