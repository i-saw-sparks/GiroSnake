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
int lastMov = 4;
int puntos = 9;
bool fin = false, inicio = true;

class SQsnake{
  public: 
  int posX, posY;
  
  SQsnake(int posX, int posY){
    this->posX = posX;
    this->posY = posY;
  }

  void SetPos(int posX, int posY){
    this->posX = posX;
    this->posY = posY;
  }
  void Draw(){
    tft.drawRect(posX, posY, 2, 2, ST77XX_RED);
  }

  void Destroy(int posX, int posY){
    tft.drawRect(posX, posY, 2, 2, ST77XX_WHITE);
  }
};

class Fruta: public SQsnake{
  public:
  
  Fruta(int posX, int posY):SQsnake(posX, posY){}

  void Fruta::Draw(){
    tft.drawRect(posX, posY, 2, 2, ST77XX_BLACK);
  } 
  
};

List<int> listaX;
List<int> listaY;
List<int> frutaX;
List<int> frutaY;

Fruta fruta(30, 30);
void setup() {
  gyro.begin();
  tft.initR(INITR_BLACKTAB);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.fillScreen(ST77XX_WHITE);
  tft.setCursor(0, 140);
  tft.print(" puntos: ");
  tft.print(puntos);
  Serial.begin(9600);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  if(!fin){

  if(inicio){    
    frutaX.Add(30);
    frutaY.Add(30);
    fruta.Draw();
    inicio = false;
  }
  
  if(gyro.readSensor() > 0){
      x = gyro.getAccelX_mss();
      y = gyro.getAccelY_mss();
  }

  Move(x, y);

  for(int i=0; i < listaX.Count(); i++){
    if(pixX == listaX[i] && pixY==listaY[i]){
      fin = true;
    }    
  }

  for(int i = 0; i < frutaX.Count(); i++){
    if(pixX == frutaX[i] && pixY == frutaY[i] || pixX == frutaX[i]+1 && pixY == frutaY[i]-1){

        puntos += 3;
        frutaX.RemoveFirst();
        frutaY.RemoveFirst();
        tft.drawRect(fruta.posX, fruta.posY, 2, 2, ST77XX_WHITE);
        int tempx, tempy;
        tempx= random(0, 126);
        tempy = random(0, 130);        
        fruta.SetPos(tempx, tempy);
        Serial.println(tempx + " " + tempy);
        fruta.Draw();
        frutaX.Add(tempx);
        frutaY.Add(tempy);
        tft.fillRect(100, 140, 30, 30, ST77XX_WHITE);
        tft.setCursor(100, 140);
        tft.print(puntos);
    }    
  }
  
  SQsnake snake((int)pixX, (int)pixY);
  
  listaX.Add((int)pixX);
  listaY.Add((int)pixY);
  if(listaX.Count()>puntos){
    snake.Destroy(listaX[0], listaY[0]); 
    listaX.RemoveFirst();
    listaY.RemoveFirst();
  }
  snake.Draw();
  delay(100);
  }
}

void Move(float x, float y){
  if(x > 2.5 && lastMov != 2){
    pixX-=1;
    lastMov = 1;   
  }else if(x < -2.5 && lastMov != 1){
     pixX+=1;
     lastMov=2;
  }else if(y>1.5 && lastMov != 4){    
    pixY-=1;
    lastMov=3;
  }else if(y<-1.5 && lastMov != 3){
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
  
  if(pixY>130){
    pixY=0;
  }else if(pixY<0){
    pixY=130;
  }
}
