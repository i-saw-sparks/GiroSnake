#include "ListLib.h"
#include <MPU9250.h>
#include <SPI.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>


#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 8

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
//Declaración del objeto del acelerometro, pasa por parametro wire y la dirección que se le asignará al dispositivo
MPU9250 gyro(Wire, 0x68);
float x, y;
float pixX =20, pixY=20;
int ch = 0;
int lastMov = 4;
int puntos = 9;
bool fin = false, inicio = true, once = true, punt = true;
 
//Clase de los cuadritos que se van renderizando para crear a la serpiente
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
//Clase de las frutitas que hereda de la clase SQsnake
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
//Listas dinámicas donde se guardan todas las posiciones de todos los 
//cuadros de la serpiente y de las frutas
List<int> listaX;
List<int> listaY;
List<int> frutaX;
List<int> frutaY;
byte b = 120;
int addr = 0;
int puntuacion[6];

Fruta fruta(30, 30);
void setup() {
  gyro.begin();//Se inicia el giroscopio
  Wire.begin();
  tft.initR(INITR_BLACKTAB);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.fillScreen(ST77XX_WHITE);
  tft.setCursor(0, 140);
  tft.print(" puntos: ");
  tft.print(puntos/3);
}

void loop() {
  
  if(!fin){ // la variable fin controla el bloque que se ejecutará solo cuando el juego esté en partida

  if(inicio){    
    frutaX.Add(30); //Se añade la fruta inicial
    frutaY.Add(30);
    fruta.Draw();
    inicio = false;
  }
  
  if(gyro.readSensor() > 0){
      x = gyro.getAccelX_mss(); //Se lee el eje X y Y de aceleración del acelerometro
      y = gyro.getAccelY_mss();
  }

  Move(x, y); //Move asigna las posiciones donde se renderizará el nuevo cuadro de la serpiente

  for(int i=0; i < listaX.Count(); i++){ //Se verifica si no hay colisión con la propia serpiente
    if(pixX == listaX[i] && pixY==listaY[i]){
      fin = true;
    }    
  }


  for(int i = 0; i < frutaX.Count(); i++){ 
    if(pixX >= frutaX[0]-1 && pixX <= frutaX[0]+3 && pixY >= frutaY[0]-1 && pixY <= frutaY[0]+ 3){ // Se verifica si hubo colosión con las frutas
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
        frutaY.Add(tempy); //Se crea una nueva fruta y se añade a la lista dinámica
        tft.fillRect(100, 140, 30, 30, ST77XX_WHITE);
        tft.setCursor(100, 140);
        tft.print(puntos/3);
  }
  } 

     
  SQsnake snake((int)pixX, (int)pixY); //Se crea nuevo cuadro de la serpiente
    
  listaX.Add((int)pixX);
  listaY.Add((int)pixY);
  if(listaX.Count()>puntos){ //Si hay más cuadros de serpiente que puntos, se destruyen los últimos cuadros
    snake.Destroy(listaX[0], listaY[0]); 
    listaX.RemoveFirst();
    listaY.RemoveFirst();
  }
  snake.Draw();
  delay(50);
  }//Aqui termina el bloque de codigo de la partida
  
  if(fin){ //Aqui se entra cuando la partida se termina
      if(once){
        tft.fillScreen(ST77XX_WHITE);
        tft.setCursor(6,0);
        
        tft.println("Ganadores");
        tft.setCursor(50,15);
        
        once = false;
        i2c_eeprom_write_byte(0x50, 0x05, (byte)(puntos/3));//Se escribe tu puntaje en la EEPROM
        delay(100);
      }

      for(int i = 0; i<6; i++){
        puntuacion[i] = i2c_eeprom_read_byte(0x50, i); //Se leen los valores de la EEPROM
        delay(100);
      }

      for(int i = 0; i<6; i++){ //Algoritmo de burbuja
        for(int j=0; j<6; j++){
          if(puntuacion[i]>puntuacion[j]){
            int temp = puntuacion[j];
            puntuacion[j]= puntuacion[i];
            puntuacion[i]= temp;
          }
        }
      }
      int y = 30;
      for(int i = 0; i<5; i++){
        if(puntuacion[i]==puntos/3){
          tft.setTextColor(ST77XX_RED);
        }else{
          tft.setTextColor(ST77XX_BLACK);
        }
        tft.setCursor(50,y);
        y+=15;
        tft.println(puntuacion[i]);
        i2c_eeprom_write_byte(0x50, i, (byte)puntuacion[i]); //Se muestran los resultados y se escriben en la EEPROM en orden
        delay(100);
      }

      
      while(true);
  }
}
  

void Move(float x, float y){ //Move determina la posición de renderizado del próximo cuadro de serpiente
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


void i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, byte data ) { //funcion para escribit un byte en la EEPROM
    int rdata = data;
    Wire.beginTransmission(deviceaddress); //Se inicio la transmición de datos
    Wire.write((int)(eeaddress >> 8)); // se determina el bit mas significativo mediante un right shift
    Wire.write((int)(eeaddress & 0xFF)); // la mascara es usada para solo tomar en cuenta los primeros 8 bits
    Wire.write(rdata); //Se escribe el byte en la memoria
    Wire.endTransmission(); //Se termina la transmición
}


byte i2c_eeprom_read_byte( int deviceaddress, unsigned int eeaddress ) {
    byte rdata = 0xFF;
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddress >> 8)); 
    Wire.write((int)(eeaddress & 0xFF)); 
    Wire.endTransmission();
    Wire.requestFrom(deviceaddress,1); 
    if (Wire.available()) rdata = Wire.read();
    return rdata;
}
