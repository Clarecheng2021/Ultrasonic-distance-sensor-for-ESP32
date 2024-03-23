#include <IRremote.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED屏幕宽度，单位为像素
#define SCREEN_HEIGHT 64 // OLED屏幕高度，单位为像素
#define OLED_ADDRESS 0x3C // OLED屏幕I2C地址

#define IR_RECEIVE_PIN 14 // 红外接收模块的接收引脚
#define TRIGGER_PIN 13 // 超声波模块的Trig引脚
#define ECHO_PIN 12 // 超声波模块的Echo引脚

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
IRrecv irrecv(IR_RECEIVE_PIN);
decode_results results;

bool screenOn = true; // 屏幕状态标志

void setup() {
  Serial.begin(9600);
  irrecv.enableIRIn(); // 启用红外接收
  Wire.begin(); // 初始化I2C总线
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS); // 初始化OLED屏幕
  display.clearDisplay(); // 清空屏幕内容
  pinMode(TRIGGER_PIN, OUTPUT); // 设置超声波Trig引脚为输出模式
  pinMode(ECHO_PIN, INPUT); // 设置超声波Echo引脚为输入模式
}

void loop() {
  if (irrecv.decode(&results)) {
    if (!screenOn && results.value != 0xFFA25D) {
      irrecv.resume(); // 如果屏幕关闭，且不是按下1键，直接忽略该红外信号
      return;
    }
    
    switch (results.value) {
      case 0xFFA25D: // 按键码1
        screenOn = !screenOn; // 切换屏幕状态
        if (!screenOn) {
          turnOffScreen(); // 熄灭屏幕
        } else {
          turnOnScreen(); // 打开屏幕
          readSensor(); // 读取传感器数据
        }
        break;
      case 0xFF629D: // 按键码2
        screenOn = true; // 打开屏幕
        readSensor(); // 读取传感器数据
        break;
      case 0xFFE21D: // 按键码3
        screenOn = true; // 打开屏幕
        monitorSensor(); // 实时监控传感器数据
        break;
    }
    irrecv.resume(); // 继续接收下一个红外信号
  }
}

void turnOffScreen() {
  display.ssd1306_command(SSD1306_DISPLAYOFF); // 关闭屏幕
  display.display(); // 更新屏幕状态
}

void turnOnScreen() {
  display.ssd1306_command(SSD1306_DISPLAYON); // 打开屏幕
  display.display(); // 更新屏幕状态
}

void readSensor() {
  if (!screenOn) {
    turnOnScreen(); // 打开屏幕
  }

  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance_cm = duration * 0.034 / 2;

  display.clearDisplay(); // 清空屏幕内容
  display.setTextSize(2); // 将字体放大为2倍
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Distance: ");
  display.print(distance_cm);
  display.println(" cm");
  display.display(); // 显示屏幕内容
}

void monitorSensor() {
  if (!screenOn) {
    turnOnScreen(); // 打开屏幕
  }

  while (true) { // 无限循环监控传感器数据
    if (irrecv.decode(&results)) { // 如果接收到红外信号
      if (results.value == 0xFFA25D) { // 如果按下按键码为1
        screenOn = false; // 关闭屏幕
        turnOffScreen();
        return; // 退出监控模式
      }
      irrecv.resume(); // 继续接收下一个红外信号
    }
    
    digitalWrite(TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);
    
    long duration = pulseIn(ECHO_PIN, HIGH);
    float distance_cm = duration * 0.034 / 2;
  
    display.clearDisplay(); // 清空屏幕内容
    display.setTextSize(2); // 将字体放大为2倍
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Distance: ");
    display.print(distance_cm);
    display.println(" cm");
    display.display(); // 显示屏幕内容
    
    delay(1000); // 每秒更新一次数据
  }
}
