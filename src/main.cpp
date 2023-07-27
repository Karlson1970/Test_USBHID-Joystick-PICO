#include <Arduino.h>
#include <U8g2lib.h>
#include "PicoGamepad.h"

PicoGamepad gamepad;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
int x = 0;
int direction = -1;  // Направление движения

int Axe_x = 0;
int Axe_y = 0;
int val = 0;
// кнопки
const uint8_t ButtonA = 4;
const uint8_t ButtonB = 5;
const uint8_t ButtonC = 6; 
// Пины энкодера
const uint8_t encoderPinA = 15;
const uint8_t encoderPinB = 14;
int16_t encoderPos = 0;

void encoderA() {
  // Считываем значения с обоих пинов энкодера
  int a = digitalRead(encoderPinA);
  int b = digitalRead(encoderPinB);

// По изменению состояния A или B пина
// Определяем направление вращения энкодера
  if (a == HIGH && b == LOW && encoderPos !=256) { 
    encoderPos++;  
  }
}

void encoderB() {
// Считываем значения с обоих пинов энкодера
  int a = digitalRead(encoderPinA);
  int b = digitalRead(encoderPinB);

// По изменению состояния A или B пина
// Определяем направление вращения энкодера
  if (a == LOW && b == HIGH && encoderPos != -256){
    encoderPos--;
  }
}

void setup() {
// аппаратные прерывания от энкодера канал A и канал B
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  pinMode(ButtonA, INPUT_PULLUP);
  pinMode(ButtonB, INPUT_PULLUP);
  pinMode(ButtonC, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderPinA), encoderA, RISING);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), encoderB, RISING);
//включаем внутренний датчик температуры
  adc_init();
  adc_set_temp_sensor_enabled(1);

  u8g2.begin();
}

void loop() {
  char buf[200];

  u8g2.setColorIndex(1);
  u8g2.setFont(u8g2_font_8x13B_tf);
  u8g2.firstPage();  
  do {
// выводим на дисплей текст
    strcpy(buf, "Joystick Raspberry PICO USB HID (SSD1306 SDA GPIO2; SCL GPIO3)(Encoder(Axe Z) CHA GPIO15; CHB GPIO14 (Axe X GPIO26; Axe Y GPIO27))(BUTTON GPIO4 GPIO5 GPIO6)");
    u8g2.drawStr(x, 10, buf);
// бегущая строка вправо влево
    int string_width = u8g2.getUTF8Width(buf); 
    if ((x > u8g2.getDisplayWidth() && direction == 1) || (x + string_width <= 0 && direction == -1)) {
      direction *= -1;
    }  
    x += direction;
// получение данных от встроенного сенсора температуры
    adc_select_input(4); 
	  uint16_t adcReading = adc_read();
    analogReadResolution(10);
    float celsius = adcReading * 33.0f / 1024.0f;
    sprintf(buf, "%.1f\xb0\x43", celsius);
    u8g2.drawUTF8(75, 40, buf);
    adc_select_input(0);
    Axe_x = adc_read();
    adc_select_input(1);
    Axe_y = adc_read();
// выводим значение осей
    sprintf(buf, "x - %d",Axe_x);
    u8g2.drawStr(0, 30, buf);
    sprintf(buf, "y - %d",Axe_y);
    u8g2.drawStr(0, 45, buf);
    sprintf(buf, "z - %d",encoderPos);
    u8g2.drawStr(0, 60, buf);
  } while (u8g2.nextPage());
  
  val = map(Axe_x, 0, 1023, -32767, 32767);
  gamepad.SetX(val);
  val = map(Axe_y, 0, 1023, -32767, 32767);
  gamepad.SetY(val);
  val = map(encoderPos, -256, 256, -32767, 32767);
  gamepad.SetZ(val);
  gamepad.SetButton(0, !digitalRead(ButtonA));
  gamepad.SetButton(1, !digitalRead(ButtonB));
  gamepad.SetButton(2, !digitalRead(ButtonC));
  gamepad.send_update();
}
