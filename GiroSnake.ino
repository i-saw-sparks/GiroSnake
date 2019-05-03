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
bool fin = false, inicio = true, once = true, punt = true;
 

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
    tft.fillRect(posX, posY, 4, 4, ST77XX_BLACK);
  } 

  void Fruta::Destroy(int posX, int posY){
    tft.fillRect(posX, posY, 4, 4, ST77XX_WHITE);
  }
  
};

List<int> listaX;
List<int> listaY;
List<int> frutaX;
List<int> frutaY;
byte b = 120;
int addr = 0;


Fruta fruta(30, 30);
void setup() {
  gyro.begin();
  Wire.begin();
  tft.initR(INITR_BLACKTAB);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.fillScreen(ST77XX_WHITE);
  tft.setCursor(0, 140);
  tft.print(" puntos: ");
  tft.print(puntos/3);
  Serial.begin(9600);
  tft.fillRect(1,1,4,4,ST77XX_BLACK);
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
    if(pixX >= frutaX[0]-1 && pixX <= frutaX[0]+3 && pixY >= frutaY[0]-1 && pixY <= frutaY[0]+ 3){
        puntos += 3;
        frutaX.RemoveFirst();
        frutaY.RemoveFirst();
        fruta.Destroy(fruta.posX, fruta.posY);
        int tempx, tempy;
        tempx= random(0, 126);
        tempy = random(0, 130);        
        fruta.SetPos(tempx, tempy);
        fruta.Draw();
        frutaX.Add(tempx);
        frutaY.Add(tempy);
        tft.fillRect(100, 140, 30, 30, ST77XX_WHITE);
        tft.setCursor(100, 140);
        tft.print(puntos/3);
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
  delay(50);
  }
  if(fin){    
      if(once){
        tft.fillScreen(ST77XX_WHITE);
        tft.setTextSize(1);
        tft.setCursor(0,0);
        once = false;
      }
      if(punt){
        b = i2c_eeprom_read_byte(0x50, addr); //access an address from the memory
        if(b != 255){
          tft.println(b);
        }else{
          i2c_eeprom_write_byte(0x50, addr, (byte)(puntos/3));
          delay(500);
           b = i2c_eeprom_read_byte(0x50, addr); //access an address from the memory
          tft.println(b);
           punt = false;
        }
        addr++; //increase address
        
        delay(500);
       
      }
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


void i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, byte data ) {
    int rdata = data;
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddress >> 8)); // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.write(rdata);
    Wire.endTransmission();
}


byte i2c_eeprom_read_byte( int deviceaddress, unsigned int eeaddress ) {
    byte rdata = 0xFF;
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddress >> 8)); // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(deviceaddress,1);
    if (Wire.available()) rdata = Wire.read();
    return rdata;
}
