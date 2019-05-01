#include "ListLib.h"
#include <MPU9250.h>
#include <SPI.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>


#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 8

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

MPU9250 gyro(Wire, 0x68);
float x, y;
float pixX =20, pixY=20;
int ch = 0;
int lastMov = 0;
int puntos = 90;


class SQsnake{
  private: 
  int posX, posY;

  public:
  SQsnake(int posX, int posY){
    this->posX = posX;
    this->posY = posY;
  }
  
  void setSP(int posX, int posY){    
    this->posX = posX;
    this->posY = posY;
  }
  
  int getSX(){
    return posX;
  }

  int getSY(){
    return posY;
  }

  void drawSnake(){
    tft.drawRect(posX, posY, 2, 2, ST77XX_RED);
  }

  void destroySnake(int posX, int posY){
    tft.drawRect(posX, posY, 2, 2, ST77XX_WHITE);
  }
};

List<int> listaX;
List<int> listaY;

void setup() {
  gyro.begin();
  tft.initR(INITR_BLACKTAB);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.fillScreen(ST77XX_WHITE);
  tft.setCursor(20, 140);
  tft.print("puntos: " + puntos);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(gyro.readSensor() > 0){
      x = gyro.getAccelX_mss();
      y = gyro.getAccelY_mss();
  }

  Move(x, y);

  
  SQsnake snake((int)pixX, (int)pixY);
  
  listaX.Add((int)pixX);
  listaY.Add((int)pixY);
  Serial.println(listaX.Count());
  if(listaX.Count()>puntos){
    snake.destroySnake(listaX[0], listaY[0]);
    listaX.RemoveFirst();
    listaY.RemoveFirst();
  }
  snake.drawSnake();
  delay(200);
}

void Move(float x, float y){
  if(x > 2.5){
    pixX-=1;
    lastMov = 1;   
  }else if(x < -2.5){
     pixX+=1;
     lastMov=2;
  }else if(y>1.5){    
    pixY-=1;
    lastMov=3;
  }else if(y<-1.5){
    pixY+=1;
    lastMov=4;
  }else{
    switch (lastMov){
      case 1:
       pixX-=1;
      break;
        
      case 2:
      pixX+=1;
      break;
      
      case 3:
      pixY-=1;
      break;
     
      case 4:
      pixY+=1;
      break;
    }
  }

  if(pixX > 128){
    pixX=0;
  }else if(pixX<0){
    pixX = 128;
  }
  
  if(pixY>160){
    pixY=0;
  }else if(pixY<0){
    pixY=160;
  }
}
