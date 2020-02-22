//CODIGO PARA EL CONTROL DE 4 LOOP MAGNETIC ANTENNAS
//ESCRITO POR JOSÉ BASCÓN EA7HVO
//LICENCIA PARA USO NO COMERCIAL

#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#define UP 17      //pin 2 para boton up
#define DOWN  11   //pin 5 para boton down
#define memUP  15  // pin 12 para boton memUP
#define memDOWN  16  //pin 13 para memDOWN
#define menuPin  14   // para funciones de menu
#define desabilitar  8 //pin 8 que es el enable de los pololu en la shield
#define encoderPinA  10   // cable amarillo del encoder en el pin 10
#define encoderPinB  9    //cable verde del encoder en el pin 9

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

unsigned int lastEncoded = 0; //ultimo valor que teniamos del encoder para ver si hemos aumentado o disminuido la cuenta
unsigned int encoderValue = 0; //valor que extraemos del encoder
unsigned int lastencoderValue = 0;
unsigned int Eepromread = 0; //se pone a cero para que se lea la memoria al comenzar
unsigned int i = 0; //para los bucles
unsigned long Time = 0; // temporizador
unsigned int a = 0; //marcador de  memUP y memDOWN para que se tome como paso del encoder
unsigned int b = 0; //marcador de destino de encodervalue tras recuperar una memoria
int direccion = 5;  //toma el valor de el pin DIR del pololu diferente en cada antena
int pololu = 2;    //toma el valor de el pin STEP del pololu diferente en cada antena
int memstep = 0; //marcador para la seleccion de memoria de pasos
int savmem = 0;   //marcador para activar la gravacion en memoria
int funcionmenu = 0; //marcador para seleccionar el menu entre ajuste, memorias, memorizacion...
int ajuste = 0;   //para el ajuste fino de los pasos en caso de que haga falta
int antena = 1;   //para la seleccion de antena entre 1 y 4 
int mempointer = 32; //puntero para la seleccion de grupo de memorias
int upperlimit = 60000; //limite superior para que el condensador nunca pase de aqui
int nodata = 0;   // variable para cuando no existen datos en la memoria
int backlash = 0;   //Variable para backlash correction
int sp = 2;   //variable para la velocidad


void setup() {
 Serial.begin (9600); //solo por si es necesari visualizar algun proceso
 lcd.init();                      // initialize the lcd 
 lcd.backlight();                 // Encendemos el backlight
 pinMode(2, OUTPUT);  // pin 2 como salida para controlar pololu
 digitalWrite(2, LOW);
 pinMode(5, OUTPUT);   // pin 5 como salida para controlar pololu
 digitalWrite(5, LOW);
 pinMode(3, OUTPUT);  // pin 3 como salida para controlar pololu
 digitalWrite(3, LOW);
 pinMode(6, OUTPUT);   // pin 6 como salida para controlar pololu
 digitalWrite(6, LOW);
 pinMode(4, OUTPUT);  // pin 4 como salida para controlar pololu
 digitalWrite(4, LOW);
 pinMode(7, OUTPUT);   // pin 7 como salida para controlar pololu
 digitalWrite(7, LOW);
 pinMode(12, OUTPUT);  // pin 12 como salida para controlar pololu
 digitalWrite(12, LOW);
 pinMode(13, OUTPUT);   // pin 13 como salida para controlar pololu 
 digitalWrite(13, LOW);
 pinMode(desabilitar,OUTPUT); //pin 8 como salida para controlar pololu habilitar o desabilitar stepper
 pinMode(encoderPinA, INPUT_PULLUP); // cable verde del encoder como input puuup
 pinMode(encoderPinB, INPUT_PULLUP); // cable amarillo del encoder como input puuup
 pinMode(menuPin, INPUT_PULLUP);  // pulsador en pin 11 como input puuup
 pinMode(UP, INPUT_PULLUP); // pulsador en pin 2 como input puuup
 pinMode(DOWN, INPUT_PULLUP); // pulsador en pin 5 como input puuup
 pinMode(memUP, INPUT_PULLUP); // pulsador en pin 12 como input puuup
 pinMode(memDOWN, INPUT_PULLUP); // pulsador en pin 11  como input puuup
 
 int lastMSB = digitalRead(encoderPinA); //get starting position en el codificador rotatorio pin 3
 int lastLSB = digitalRead(encoderPinB); //get starting position en el codificador rotatorio pin 6

 //let start be lastEncoded so will index on first click
 lastEncoded = (lastMSB << 1) |lastLSB;

}

void loop(){ 
  if (Eepromread == 0)//Si es el encendido, el valor será siempre 0
                      //entonces leemos todos los valores almacenados en memoria
                      //Inicializamos todos los valores 
  {
  EEPROM.get(14,sp);  //lee la posicion 14 de la memoria y la pone como velocidad sp
  if(sp==0) sp = 2; //si no hay ningun valor le adjudicamos un 2 que es una velocidad alta
  EEPROM.get(4,antena);  //lee la posicion 4 de la memoria el valor de la antena antre 1 y 4
  if(antena > 4 || antena < 1 ){      //si la lectura de antena no es un numero entre 1 y 4 se entiende que es la primera vez que se utiliza el controlador  
                                      //es necesario poner a 0 todos los valores menos el de la antena que queda a 1
                  lcd.clear();                    
                  lcd.setCursor(0, 0);      // ubica cursor en columna 0 y linea 0
                  lcd.print("PREPARING FOR"); 
                  lcd.setCursor(0, 1);      // ubica cursor en columna 0 y linea 1  
                  lcd.print("THE FIRST USE"); 
                  delay(2000);                                     
        for (i=0; i <= 255; i++) {
          EEPROM.put(i,0);     //escribimos el valor o en todas las posiciones
          }  
        EEPROM.put(4,1);     //escribimos el valor de antena a 1 en la memoria 
        antena = 1;           //ponemos antena a 1                                
    
    }
                 switch(antena){
                                
                  case 1:         //
                  mempointer = 32;    
                  EEPROM.get(mempointer,Eepromread);  
                  EEPROM.get(mempointer+2,memstep); 
                  EEPROM.get(6,backlash); //backlash de la antena 1
                  EEPROM.get(mempointer+30,upperlimit);
                  if (upperlimit == 0) upperlimit = 65000; 
                  direccion =5;  //toma el valor de el pin DIR del pololu diferente en cada antena
                  pololu = 2;    //toma el valor de el pin STEP del pololu diferente en cada antena                           
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //                                  
                  break;
                  
                  case 2:         //
                  mempointer = 80;    
                  EEPROM.get(mempointer,Eepromread);  
                  EEPROM.get(mempointer+2,memstep);
                  EEPROM.get(8,backlash); //backlash de la antena 2
                  EEPROM.get(mempointer+30,upperlimit);
                  if (upperlimit == 0) upperlimit = 65000;
                  direccion =6;  //toma el valor de el pin DIR del pololu diferente en cada antena
                  pololu = 3;    //toma el valor de el pin STEP del pololu diferente en cada antena                           
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //                      
                  break;

                  case 3:         //
                  mempointer = 128;    
                  EEPROM.get(mempointer,Eepromread);  
                  EEPROM.get(mempointer+2,memstep);
                  EEPROM.get(10,backlash); //backlash de la antena 3 
                  EEPROM.get(mempointer+30,upperlimit);
                  if (upperlimit == 0) upperlimit = 65000;  
                  direccion =7;  //toma el valor de el pin DIR del pololu diferente en cada antena
                  pololu = 4;    //toma el valor de el pin STEP del pololu diferente en cada antena                           
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //  
                                    
                  break;

                  case 4:         //
                  mempointer = 176;    
                  EEPROM.get(mempointer,Eepromread);  
                  EEPROM.get(mempointer+2,memstep);
                  EEPROM.get(12,backlash); //backlash de la antena 4 
                  EEPROM.get(mempointer+30,upperlimit);
                  if (upperlimit == 0) upperlimit = 65000;  
                  direccion =13;  //toma el valor de el pin DIR del pololu diferente en cada antena
                  pololu = 12;    //toma el valor de el pin STEP del pololu diferente en cada antena                           
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //                      
                  break;
                  
                 }  
                
  //digitalWrite(desabilitar, HIGH); //desabilita el stepper para que no consuma
  encoderValue = Eepromread ;  //se iguala la lectura de memoria con la primera posicion de codificador
  Eepromread = 1; //se pone a 1 para que no repita este proceso de lectura
  lcd.setCursor(0, 1);      // ubica cursor en columna 8 y linea 1
  lcd.print("MEM    ");    
  if(encoderValue <= 1) encoderValue=1; 
                                                    
    }
  
 int MSB = digitalRead(encoderPinA); //MSB = most significant bit
 int LSB = digitalRead(encoderPinB); //LSB = least significant bit

 int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
 int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value
 if(digitalRead(memUP)==LOW || digitalRead(memDOWN)==LOW || savmem==1){
  Serial.println("pulsado");
  i=0;
  while(i<=200)
  {
  if(digitalRead(memUP)==LOW || digitalRead(memDOWN)==LOW || savmem==1){  //Seleccionamos botones memoria arriba y abajo
    if(digitalRead(memUP)==LOW && funcionmenu != 1){
                                memstep++;
                                delay (500);
                                if(memstep>=15) memstep = 1;
                                Serial.println (memstep );
                                i = 0;
                                }
    if(digitalRead(memDOWN)==LOW && funcionmenu != 1){
                                memstep--;
                                delay (500);
                                if(memstep<=0) memstep = 14;
                                Serial.println (memstep );
                                i = 0;
                                }    
    if(digitalRead(memUP)==LOW && funcionmenu == 1){
                                antena++;
                                delay (500);
                                if(antena>=5) antena = 1;
                                Serial.println (antena );
                                i = 0;
                                }
    if(digitalRead(memDOWN)==LOW && funcionmenu == 1){
                                antena--;
                                delay (500);
                                if(antena<=0) antena = 4;
                                Serial.println (antena );
                                i = 0;
                                }   
                 switch(antena){
                                
                  case 1:         //                                     
                  EEPROM.put(mempointer,encoderValue);  
                  EEPROM.put(mempointer+2,memstep);  
                  EEPROM.put(mempointer+30,upperlimit);
                  mempointer = 32;   
                  direccion =5;  //toma el valor de el pin DIR del pololu diferente en cada antena
                  pololu = 2;    //toma el valor de el pin STEP del pololu diferente en cada antena 
                  EEPROM.put(4,antena);     //grabamos el numero de memoria en que estamos
                  EEPROM.get(mempointer,encoderValue);  
                  EEPROM.get(mempointer+2,memstep); 
                  EEPROM.get(6,backlash); //backlash de la antena 1  
                  EEPROM.get(mempointer+30,upperlimit);
                  b = encoderValue;
                  if (upperlimit == 0) upperlimit = 65000; 
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep); 
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //                                  
                  break;
                  
                  case 2:         //                                     
                  EEPROM.put(mempointer,encoderValue);  
                  EEPROM.put(mempointer+2,memstep);  
                  EEPROM.put(mempointer+30,upperlimit);
                  mempointer = 80;   
                  direccion =6;  //toma el valor de el pin DIR del pololu diferente en cada antena
                  pololu = 3;    //toma el valor de el pin STEP del pololu diferente en cada antena
                  EEPROM.put(4,antena);     //grabamos el numero de memoria en que estamos 
                  EEPROM.get(mempointer,encoderValue);  
                  EEPROM.get(mempointer+2,memstep);
                  EEPROM.get(8,backlash); //backlash de la antena 2  
                  EEPROM.get(mempointer+30,upperlimit);                  
                  b = encoderValue;
                  if (upperlimit == 0) upperlimit = 65000; 
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep); 
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //                                  
                  break;
                  
                  case 3:         //                                     
                  EEPROM.put(mempointer,encoderValue);  
                  EEPROM.put(mempointer+2,memstep);  
                  EEPROM.put(mempointer+30,upperlimit); 
                  mempointer = 128;   
                  direccion =7;  //toma el valor de el pin DIR del pololu diferente en cada antena
                  pololu = 4;    //toma el valor de el pin STEP del pololu diferente en cada antena
                  EEPROM.put(4,antena);     //grabamos el numero de memoria en que estamos 
                  EEPROM.get(mempointer,encoderValue);  
                  EEPROM.get(mempointer+2,memstep); 
                  EEPROM.get(10,backlash); //backlash de la antena 3
                  EEPROM.get(mempointer+30,upperlimit); 
                  b = encoderValue;
                  if (upperlimit == 0) upperlimit = 65000; 
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep); 
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //                                  
                  break;
                  
                  case 4:         //                                     
                  EEPROM.put(mempointer,encoderValue);  
                  EEPROM.put(mempointer+2,memstep);  
                  EEPROM.put(mempointer+30,upperlimit);
                  mempointer = 176;      
                  direccion =13;  //toma el valor de el pin DIR del pololu diferente en cada antena
                  pololu = 12;    //toma el valor de el pin STEP del pololu diferente en cada antena                   
                  EEPROM.put(4,antena);     //grabamos el numero de memoria en que estamos 
                  EEPROM.get(mempointer,encoderValue);  
                  EEPROM.get(mempointer+2,memstep); 
                  EEPROM.get(12,backlash); //backlash de la antena 1
                  EEPROM.get(mempointer+30,upperlimit);                  
                  b = encoderValue;
                  if (upperlimit == 0) upperlimit = 65000; 
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep); 
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //                                
                  break;

                  
                 }                                                                                                           
                 switch(memstep){
                                
                  case 1:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+4,b);
                  } 
                  if (savmem == 1) {
                  EEPROM.put(mempointer+4,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+4,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE1  "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000); 
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;
                  
                  case 2:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+6,b);
                  } 
                  if (savmem == 1) {
                  EEPROM.put(mempointer+6,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+6,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE2  "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000);
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;
                  
                  case 3:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+8,b);
                  } 
                  if (savmem == 1) {
                  EEPROM.put(mempointer+8,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+8,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE3  "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000);  
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;;
                  
                  case 4:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+10,b);
                  } 
                  if (savmem == 1) {
                  EEPROM.put(mempointer+10,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+10,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE4  "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000); 
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;

                  case 5:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+12,b);
                  } 
                  if (savmem == 1) {
                  EEPROM.put(mempointer+12,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+12,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE5  "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000); 
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;
                  
                  case 6:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+14,b);
                  } 
                  if (savmem == 1) {
                  EEPROM.put(mempointer+14,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+14,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE6  "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000); 
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;                  
                  
                  case 7:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+16,b);
                  } 
                  if (savmem == 1) {
                  EEPROM.put(mempointer+16,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+16,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE7  "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000); 
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;
                  
                  case 8:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+18,b);
                  } 
                  if (savmem == 1) {
                  EEPROM.put(mempointer+18,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+18,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE8  "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000); 
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;

                  case 9:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+20,b);
                  } 
                  if (savmem == 1) {
                  EEPROM.put(mempointer+20,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+20,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE9  "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000); 
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;
                  
                  case 10:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+22,b);
                  } 
                  if (savmem == 1) {
                  EEPROM.put(mempointer+22,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+22,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE10 "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000);
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break; 

                  case 11:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+24,b);
                  } 
                  if (savmem == 1) {
                  EEPROM.put(mempointer+24,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+24,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE11 "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000);
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;
                  
                  case 12:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+26,b);
                  } 
                  if (savmem == 1) {
                  EEPROM.put(mempointer+26,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+26,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE12 "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000); 
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;

                  case 13:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+28,b);
                  } 
                  if (savmem == 1) {
                  EEPROM.put(mempointer+28,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+28,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE13 "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000);
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;
                  
                  case 14:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+30,b);
                  if(b==65000) { //si encontramos que b=65000 quiere decir que venimos de un borrado de memoria
                    b=encoderValue;
                    nodata=1;
                    }
                  } 
                  if (savmem == 1) {  //si se pulsa boton up and down a la vez grabamos encodervalue en la posicion de memoria
                  EEPROM.put(mempointer+30,encoderValue);
                  upperlimit = encoderValue;
                  savmem = 0;
                  EEPROM.get(mempointer+30,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("UPLIM "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000);
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("UPLIM");   
                  //lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;  
                 }
              }
    i++; 
    delay (10);                             
    }
    EEPROM.put(mempointer+2,memstep);     //grabamos el numero de memoria en que estamos
    
    if (encoderValue < b) 
                        {  //Comparamos encodervalue con el paso de destino que en este caso es superior en numero
    lcd.setCursor(9, 1);      // ubica cursor en columna 6 y linea 0
    lcd.print(b);   // escribe el valor de encodervalue 
    lcd.setCursor(14, 1);      // ubica cursor en columna 6 y linea 0
    lcd.print("UP");   // escribe el valor de encodervalue
    digitalWrite(desabilitar, LOW);
    
    i= b+backlash;
    if(i>upperlimit) backlash = upperlimit - b;
    i= b+backlash;      
    while ( encoderValue < i ) //bucle de pasos hasta que encodervalue y b se igualan
    {  
    encoderValue++;      
    digitalWrite(direccion, HIGH);        // giro en un sentido positivo
    digitalWrite(pololu, HIGH);       // nivel alto  
    delay(sp);                            
    digitalWrite(pololu, LOW);        // nivel bajo     
    if(digitalRead(DOWN)==LOW) break ;          
    }
    delay(500);
    while ( encoderValue > i-backlash ) //bucle de pasos hasta que encodervalue y b se igualan
    {  
    encoderValue--;   
    digitalWrite(direccion, LOW);        // giro en un sentido negativo  
    digitalWrite(pololu, HIGH);       // nivel alto  
    delay(sp);                            
    digitalWrite(pololu, LOW);        // nivel bajo     
    if(digitalRead(DOWN)==LOW) break ;          
    }
    delay(500);
    lcd.setCursor(9,1);      // ubica cursor en columna 6 y linea 0
    lcd.print("       ");   // escribe el valor de encodervalue      
                       
                       }
  
  if (encoderValue > b)
                      {  //Comparamos encodervalue con el paso de destino que en este caso es inferior en numero
    lcd.setCursor(7, 1);      // ubica cursor en columna 6 y linea 0
    lcd.print(b);   // escribe el valor de encodervalue 
    lcd.setCursor(12, 1);      // ubica cursor en columna 6 y linea 0
    lcd.print("DOWN");   // escribe el valor de encodervalue 
    digitalWrite(desabilitar, LOW);   
    if (backlash>b)  backlash = b-1; 
    i= b-backlash;      
    while ( encoderValue > i ) //bucle de pasos hasta que encodervalue y b se igualan
    { 
    encoderValue-- ;   
    digitalWrite(direccion, LOW);        // giro en un sentido negativo
    digitalWrite(pololu, HIGH);       // nivel alto  
    delay(sp);                            
    digitalWrite(pololu, LOW);        // nivel bajo 
    if(digitalRead(DOWN)==LOW) break ;                 
    }
    delay(500);
    while (encoderValue < i+backlash  ) //bucle de pasos hasta que encodervalue y b se igualan
    {    
    encoderValue++; 
    digitalWrite(direccion, HIGH);        // giro en un sentido positivo
    digitalWrite(pololu, HIGH);       // nivel alto  
    delay(sp);                            
    digitalWrite(pololu, LOW);        // nivel bajo 
    if(digitalRead(DOWN)==LOW) break ;                  
    }
    delay(500);
    lcd.setCursor(7, 1);      // ubica cursor en columna 6 y linea 0
    lcd.print("         ");   // escribe el valor de encodervalue     
                              
                       }
    if (nodata==1) {
      lcd.setCursor(7, 1);      // ubica cursor en columna 6 y linea 0
      lcd.print("NO DATA  ");   // escribe el valor de encodervalue
      delay (1000);
      lcd.setCursor(7, 1);      // ubica cursor en columna 6 y linea 0
      lcd.print("         ");   // escribe el valor de encodervalue
      nodata=0;
      }
                            
  } //final del bucle memUP-memDOWN
 

 if(digitalRead(UP)==LOW) //Si pulsamos el boton de UP
                            { if(ajuste == 1){          //si estamos en modo ajuste no cuenta pasos ( no suma a encodervalue)
                              
                              
                              digitalWrite(desabilitar, LOW);
                              digitalWrite(direccion, HIGH);        // giro en un sentido positivo
                              digitalWrite(pololu, HIGH);       // nivel alto  
                              delay(50);                            
                              digitalWrite(pololu, LOW);        // nivel bajo                      
                              }
                              
                              else if(funcionmenu == 2) savmem=1;

                              else if(funcionmenu == 4) {
                                lcd.setCursor(9, 1);      // ubica cursor en columna 6 y linea 1
                                lcd.print("    ");                              
                                backlash++;
                                if (backlash > 200) backlash = 200;
                                lcd.setCursor(9, 1);      // ubica cursor en columna 6 y linea 1
                                lcd.print(backlash); 
                                delay(100);}

                              else if(funcionmenu == 5) {
                                lcd.setCursor(9, 1);      // ubica cursor en columna 6 y linea 1
                                lcd.print("    ");                              
                                sp++;
                                if (sp > 20) sp = 20;
                                lcd.setCursor(9, 1);      // ubica cursor en columna 6 y linea 1
                                lcd.print(sp); 
                                delay(100);}
                                
                              
                              else {
                                a = 1;
                                b = encoderValue;                                
                                }                                                   
                              }
 if(digitalRead(DOWN)==LOW) //si Pulsamos el boton de down
                            { if(ajuste == 1){      //si estamos en modo ajuste no cuenta pasos ( no suma a encodervalue)
                              
                              
                              digitalWrite(desabilitar, LOW);
                              digitalWrite(direccion, LOW);        // giro en un sentido negativo
                              digitalWrite(pololu, HIGH);       // nivel alto  
                              delay(50);                              
                              digitalWrite(pololu, LOW);        // nivel bajo
                               }
                               
                               else if(funcionmenu == 2) savmem=1;

                               else if(funcionmenu == 4) {
                                lcd.setCursor(9, 1);      // ubica cursor en columna 6 y linea 1
                                lcd.print("    ");   
                                backlash--;
                                if (backlash <= 0) backlash = 0;
                                lcd.setCursor(9, 1);      // ubica cursor en columna 6 y linea 1
                                lcd.print(backlash); 
                                delay(100);}

                               else if(funcionmenu == 5) {
                                lcd.setCursor(9, 1);      // ubica cursor en columna 6 y linea 1
                                lcd.print("    ");   
                                sp--;
                                if (sp <= 1) sp = 1;
                                lcd.setCursor(9, 1);      // ubica cursor en columna 6 y linea 1
                                lcd.print(sp); 
                                delay(100);}
                              
                              else{
                              a = 2;    
                              b = encoderValue;                              
                              }                                         
                              }

 if(sum == 0b1101 || a == 1) 
                            {                               
                              if( b == encoderValue)  //el valor b es el que se pone como destino y aqui se alcanza
                             {                                                        
                                a = 0;
                                b = 0;                              
                               }
                              encoderValue ++; 
                              
                              if(encoderValue > upperlimit) {        //tope por arriba 
                                                        encoderValue=upperlimit;
                                                        lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                                                        lcd.print("UPLIM"); 
                                                        delay(2000);
                                                        lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                                                        lcd.print("MEM");   
                                                        lcd.print(memstep);        //
                                                        lcd.print("  ");
                                                        a = 0;
                                                        b = 0;
                                                        }
                              else {                      
                              digitalWrite(desabilitar, LOW);
                              digitalWrite(direccion, HIGH);        // giro en un sentido positivo
                              digitalWrite(pololu, HIGH);       // nivel alto  
                              delay(1);                            
                              digitalWrite(pololu, LOW);        // nivel bajo                                                                                          
                                    }                         
                              Time = millis();                                                                                                          
                              }
 if(sum == 0b1110 || a == 2) 
                            { 
                               
                                                   
                              if( b == encoderValue) //el valor b es el que se pone como destino y aqui se alcanza      
                                {
                                a = 0;
                                b = 0;                                
                                }
                              encoderValue --;    
                              
                              if(encoderValue < 1) {                  // por abajo
                                                     encoderValue=1; 
                                                     lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 1
                                                     lcd.print("DOWN LIMIT"); 
                                                     delay(1000);
                                                     lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 1
                                                     lcd.print("          ");
                                                     a = 0;
                                                     b = 0;
                                                    }
                              else {
                                
                              digitalWrite(desabilitar, LOW);
                              digitalWrite(direccion, LOW);        // giro en un sentido negativo
                              digitalWrite(pololu, HIGH);       // nivel alto  
                              delay(1);                              
                              digitalWrite(pololu, LOW);        // nivel bajo                            
                                }                
                              Time = millis();                                                                      
                              }

 lastEncoded = encoded; //store this value for next time
 
  
 if(encoderValue != lastencoderValue) //Condiciones que se cumplen cuando se ha contado algun paso
 {
 lcd.setCursor(0, 0);      // ubica cursor en columna 0 y linea 0
 if(encoderValue<10000 && encoderValue>=1000) lcd.print("0");
 if(encoderValue<1000 && encoderValue>=100) lcd.print("00");
 if(encoderValue<100 && encoderValue>=10) lcd.print("000");
 if(encoderValue<10) lcd.print("0000");
 lcd.print(encoderValue);   // escribe el valor de encodervalue
 

 lastencoderValue=encoderValue; //se iguala el valor obtenido como el ultimo leido
 } 
  if(digitalRead(UP)==LOW && digitalRead(DOWN)==LOW)   //reset encoderValue si pulsamos pin 17 y 11
              {
                  lcd.setCursor(0, 0);      // ubica cursor en columna 0 y linea 0
                  lcd.print("DELETTING MEMORY"); 
                  lcd.setCursor(0, 1);      // ubica cursor en columna 0 y linea 1  
                  lcd.print("    BANK"); 
                  lcd.print(antena);   
                  delay(3000); 
                  upperlimit=65000;                                  
        for (i=mempointer; i <= mempointer+30; i++) {
                  EEPROM.put(i,0);     //escribimos el valor o en todas las posiciones
                  }  
                  lcd.clear();
                  encoderValue=1;
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //  
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
               }

 if(digitalRead(menuPin)==LOW )   //Cambiamos el modo a MHZ de ajuste a amateur y a broadcast
              { 
                funcionmenu++;
                if(funcionmenu>=6) funcionmenu = 0; 
                switch(funcionmenu){
                                
                  case 0:         //ajuste para que UP y DOWN no cuenten pasos y nos permitan ajustar fino                          
                  ajuste = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 0 y linea 1
                  lcd.print("MEM    ");   // escribe el valor de encodervalue
                  lcd.print("          ");    
                  EEPROM.put(14,sp);                                                
                  break;
                  
                  case 1:                                  
                  ajuste = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 0 y linea 1
                  lcd.print("ANT     ");   // escribe el valor de encodervalue
                  lcd.print("  ");                  
                  break;

                  case 2:
                  ajuste = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 0 y linea 1
                  lcd.print("SAVE UP=YES");   // escribe el valor de encodervalue
                  lcd.print("    ");                  
                  break;
                  
                  case 3:
                  ajuste=1;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 0 y linea 1
                  lcd.print("ADJUST");   // escribe el valor de encodervalue
                  lcd.print("          ");
                  digitalWrite(desabilitar, LOW); //Habilita el stepper para que no se pierdan pasos                  
                  break;

                  case 4:
                  ajuste=0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 0 y linea 1
                  lcd.print("BACKLASH");   // escribe el valor de encodervalue
                  lcd.print("     U/D"); 
                  lcd.setCursor(9, 1);      // ubica cursor en columna 6 y linea 1
                  lcd.print(backlash);                              
                  break;

                  case 5:
                  ajuste=0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 0 y linea 1
                  lcd.print("SPEED   ");   // escribe el valor de encodervalue
                  lcd.print("     U/D"); 
                  lcd.setCursor(9, 1);      // ubica cursor en columna 6 y linea 1
                  lcd.print(sp);  
                  if (antena==1) EEPROM.put(6,backlash);      
                  if (antena==2) EEPROM.put(8,backlash);  
                  if (antena==3) EEPROM.put(10,backlash);  
                  if (antena==4) EEPROM.put(12,backlash);                             
                  break;
                } 
                                           
               delay(500);
                              
               }               
 if(millis() - Time>= 1000) //temporizador de cada ,2 segundos
  {
  Time = millis();
  
  
  EEPROM.get(mempointer,Eepromread);// leemos los ultimos datos que teniamos correspondientes a encoderValue
  if(ajuste == 0) digitalWrite(desabilitar, HIGH);
  if(int(Eepromread)!= encoderValue) //escribimos en la memoria solo si se han modificado los datos
  {
    Eepromread=encoderValue;//Igualamos eepromread a encodervalue
    EEPROM.put(mempointer,Eepromread);//lo guardamos
    EEPROM.put(mempointer+2,memstep);//tambien guardamos la memoria
    EEPROM.put(4,antena);//tambien guardamos la antena
    lcd.setCursor(0, 0);      // ubica cursor en columna 0 y linea 0                   
    if (ajuste == 0) digitalWrite(desabilitar, HIGH); //desabilita el stepper para que no consuma
    }
     
  lcd.setCursor(0, 0);      // ubica cursor en columna 0 y linea 0
  if(encoderValue<10000 && encoderValue>=1000) lcd.print("0");
  if(encoderValue<1000 && encoderValue>=100) lcd.print("00");
  if(encoderValue<100 && encoderValue>=10) lcd.print("000");
  if(encoderValue<10) lcd.print("0000");
  lcd.print(encoderValue);   // escribe el valor de encodervalue
 }
   
}
