#include <Wire.h> /Некрасов Сергей
#include <Servo.h>
Servo servox;
Servo servoy;
Servo servoz;
const int MPU = 0x68; 
float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float accAngleX, accAngleY, gyroAngleX, gyroAngleY, gyroAngleZ;
float roll, pitch, yaw;
float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
float elapsedTime, currentTime, previousTime;
int c = 0;
float K = 0.04;
float sx,sy,sz;
void setup() {
  servoz.attach(3);
  servox.attach(5);
  servoy.attach(6);
  Serial.begin(9600);
  Wire.begin();                      
  Wire.beginTransmission(MPU); //MPU = 0x68 подключение к датчику 
  Wire.write(0x6B);           // обращаемся к регистру 0х6В
  Wire.write(0x00);           // отправляем регистру 0 для сброса       
  Wire.endTransmission(true);   // заканчиваем передачу
  servox.write(90);
  servoy.write(90);
  servoz.write(90);     
  delay(1000);
  calculate_error(); // Калибровка датчика (расчет накапливаемой ошибки) 
}
void loop() {
  Wire.beginTransmission(MPU); //подключение к датчику
  Wire.write(0x3B);  // обращаемся к первому регистру аксселерометра
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true); // считывание данных акселерометра
  AccX = (Wire.read() << 8 | Wire.read()) / 16384.0; // данные оси X
  AccY = (Wire.read() << 8 | Wire.read()) / 16384.0; // данные оси Y
  AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0; // данные оси Z
  accAngleX = (atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / PI) - AccErrorX;
  accAngleY = (atan(-1 * AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2))) * 180 / PI) - AccErrorY;
  previousTime = currentTime; //предыдущие время чтения
  currentTime = millis();     //фактическое время чтения
  elapsedTime = (currentTime - previousTime) / 1000; //время между итерациями
  Wire.beginTransmission(MPU); //подключение к датчику
  Wire.write(0x43); // обращаемся к первому регистру гироскопа
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true); // считваем данные с гироскопа
  GyroX = (Wire.read() << 8 | Wire.read()) / 131.0; // ускорение по оси X
  GyroY = (Wire.read() << 8 | Wire.read()) / 131.0; // ускорение по оси Y
  GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0; // ускорение по оси Z
  GyroX = GyroX - GyroErrorX; //Корректировка данных на велечину ошибки
  GyroY = GyroY - GyroErrorY;
  GyroZ = GyroZ - GyroErrorZ;
  gyroAngleX = gyroAngleX + GyroX * elapsedTime; 
  gyroAngleY = gyroAngleY + GyroY * elapsedTime;
  yaw =  yaw + GyroZ * elapsedTime; // угол по оси Z
  yaw = constrain(yaw,-90,90);
  roll = (1-K) * gyroAngleX + K * accAngleX; // угол по оси X
  pitch = (1-K) * gyroAngleY + K * accAngleY; // угол по оси Y
  if (roll < 0) sx = 90 + roll; //преобразования углов в градусы для серво
  else sx = roll + 90;
  if (pitch < 0) sy = 90 + pitch;
  else sy = pitch + 90;
  if (yaw < 0) sz = 90 + yaw;
  else sz = yaw + 90;
  servox.write(sx); // вывод значений углов на серво
  servoy.write(sy);
  servoz.write(sz);
}

void calculate_error() {
    while (c < 200) {
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    AccX = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    AccY = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    // Sum all readings
    AccErrorX = AccErrorX + ((atan((AccY) / sqrt(pow((AccX), 2) + pow((AccZ), 2))) * 180 / PI));
    AccErrorY = AccErrorY + ((atan(-1 * (AccX) / sqrt(pow((AccY), 2) + pow((AccZ), 2))) * 180 / PI));
    c++;
  }
  AccErrorX = AccErrorX / 200;
  AccErrorY = AccErrorY / 200;
  c = 0;
  while (c < 200) {
    Wire.beginTransmission(MPU);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    GyroX = Wire.read() << 8 | Wire.read();
    GyroY = Wire.read() << 8 | Wire.read();
    GyroZ = Wire.read() << 8 | Wire.read();
    // Sum all readings
    GyroErrorX = GyroErrorX + (GyroX / 131.0);
    GyroErrorY = GyroErrorY + (GyroY / 131.0);
    GyroErrorZ = GyroErrorZ + (GyroZ / 131.0);
    c++;
  }
  GyroErrorX = GyroErrorX / 200;
  GyroErrorY = GyroErrorY / 200;
  GyroErrorZ = GyroErrorZ / 200;
}
