#include "rpcWiFi.h" //Wi-Fi library 
#include "TFT_eSPI.h" //TFT LCD library 
#include "Free_Fonts.h" //free fonts library 
#include <SoftwareSerial.h>;
#include "DHT.h"
#include <OneWire.h>                
#include <DallasTemperature.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <Wire.h>
#include "SHT31.h"

SHT31 sht31 = SHT31();
#define Debug 0 // 1 - включён, 0 выключен. Отключение серийных портов для финальной прошивки
int ledPin = 7;
int buttonPin = BCM20;
int ledState = 0;
int buttonOld = 1;
int buttonNew;
//declarated valors
char buffer[10];
char charPayload[50];
float t1,t2,t3,h1,h2,h3,temp1,temp2,temp3;
const int motor_encender = 24; // ocaso valor minimo de luz para el cual haremos funcionar el sistema
const int temp_minima = 20; //rango de luz deseado (salida del sistema) éstos valores los debe asignar el usuario de acuerdo a su necesidad y situación.
const int temp_maxima = 26;
int dato1;
int dato2;
int luz0;
//#define switch3 BCM20 //38        Switch Techo
//int pin1Motor;
//int pin2Motor;
//int boton = 3;//Pin 3 para activar manualmente la bomba
//const int bomba = 13; //Pin 13 para la bomba
int botonest = 0; //Inicializamos a 0 el estado del boton
const int ManualAutomatic = D1; //15  

unsigned int co2;
const int Relay1 = BCM1;  //28    Control Calefaccion
const int Relay2 = BCM4;  //7     Control Riego
const int Relay3 = BCM17; //11    Control ventilacion
const int Relay4 = BCM18; //12    Control Tanque de agua llenado
const int Relay5 = BCM10; //19    Luz Roja
const int Relay6 = BCM9; //21     Luz Verde
const int Relay7 = BCM11; //23    Luz Amarilla

#define switch1 BCM8 //24         Switch CALEFACCION
#define switch2 BCM7  //26        Switch Riego
#define switch3 BCM20 //38        Switch ventilacion
#define switch4 BCM21 //40        Switch Tanque de agua llenado
#define RX BCM14 //8              TXD CO2
#define TX BCM15 //10             RXD CO2
#define finalCarrera1 BCM5//29    Tope de carrera superior
#define finalCarrera2 BCM6//31    Tope de carrera inferior
#define pin2 D2 //16              DHt22 
#define pin3 D3 //18              DHT22
#define suelo0 D4 //22            Humedad de suelo 1
#define suelo1 D5 //32            Humedad de suelo 2
#define nivelAgua D7 //36         Nivel de agua
#define ValorLuz D0  //13           LUZ
//OneWire ourWire(D4);    //A1-i04-4           

int ndispositivos = 0;
float tempC;

const char* topic_T_0 = "DarMal/Karinskoe_T1/T1T0";
const char* topic_T_1 = "DarMal/Karinskoe_T1/T1T1";
const char* topic_T_2 = "DarMal/Karinskoe_T1/T1T2";
const char* topic_T_3 = "DarMal/Karinskoe_T1/T1T3";
const char* topic_H_0 = "DarMal/Karinskoe_T1/T1H0";
const char* topic_H_1 = "DarMal/Karinskoe_T1/T1H1";
const char* topic_H_2 = "DarMal/Karinskoe_T1/T1H2";
const char* topic_co2 = "DarMal/Karinskoe_T1/T1C0";
#define sub1 "DarMal/Karinskoe_T1/switch1"
#define sub2 "DarMal/Karinskoe_T1/switch2"
#define sub3 "DarMal/Karinskoe_T1/switch3"
#define sub4 "DarMal/Karinskoe_T1/switch4"

#define pub1 "DarMal/Karinskoe_T1/switch1_status"
#define pub2 "DarMal/Karinskoe_T1/switch2_status"
#define pub3 "DarMal/Karinskoe_T1/switch3_status"
#define pub4 "DarMal/Karinskoe_T1/switch4_status"

int switch_ON_Flag1_previous_I = 0;
int switch_ON_Flag2_previous_I = 0;
int switch_ON_Flag3_previous_I = 0;
int switch_ON_Flag4_previous_I = 0;

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
unsigned long interval = 60000;
unsigned long interval2 = 60000;
unsigned long interval3 = 60000;
unsigned long interval4 = 60000;
unsigned long interval5 = 60000;

unsigned long prevTime_T1 = millis();
unsigned long prevTime_T2 = millis();
unsigned long prevTime_T3 = millis(); 
unsigned long prevTime_T4 = millis(); 
unsigned long prevTime_T5 = millis();


TFT_eSPI tft; //initialize TFT LCD
TFT_eSprite spr = TFT_eSprite(&tft); //initialize sprite buffer

//DHT dht1(pin1, DHT22);    //D2-i026-26
DHT dht2(pin2, DHT22);    //D6-i027-27
DHT dht3(pin3, DHT22);    //D6-i027-27

SoftwareSerial myCo2(RX, TX); // RX=D10-io5-naranja, TX=D11-io23-cafe

byte request[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; 
unsigned char response[9];
// Configuraciones del sistema----------------------
const char* ssid = "Medina";
const char* password = "belgorod25"; 
const char* mqtt_server = "agriru.ru";
const char* username = "";
const char* pass = "";
const char* topic = "api/request";
const char* clientID = "wio_Rudy";
//String msgStr = "";      // MQTT message buffer

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (300)
char msg[MSG_BUFFER_SIZE];
int value = 0;

//-----------------------------------------------------------------------------------------------
void setup() {
  pinMode(finalCarrera1,INPUT);
  pinMode(finalCarrera2,INPUT);
  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
  pinMode(Relay3, OUTPUT);
  pinMode(Relay4, OUTPUT);
  pinMode(Relay5, OUTPUT);
  pinMode(Relay6, OUTPUT);
  pinMode(Relay7, OUTPUT);
  //apagamos relays
  digitalWrite(Relay1, HIGH);
  digitalWrite(Relay2, HIGH);
  digitalWrite(Relay3, HIGH);
  digitalWrite(Relay4, HIGH);
  digitalWrite(Relay5, HIGH);
  digitalWrite(Relay6, HIGH);
  digitalWrite(Relay7, HIGH);
  
  pinMode(switch1, INPUT);
  pinMode(switch2, INPUT);
  pinMode(switch3, INPUT);
  pinMode(switch4, INPUT);
  pinMode(ManualAutomatic, INPUT);
    Serial.begin(115200);
    Serial.println("*******Dary Malinovky*******");
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    sht31.begin();
    dht2.begin();
    dht3.begin();
    myCo2.begin(9600);

    //  ----------------LCD------------------------

  //LCD setup
  tft.begin(); //start TFT LCD
  tft.setRotation(3); //set TFT LCD rotation
  tft.fillScreen(TFT_BLACK); //fill background

  //header title 
  tft.fillRect(0,0,320,50,TFT_GREEN); //fill rectangle 
  tft.setFreeFont(FSB12); //set font type 
  tft.setTextColor(TFT_BLACK); //set text color
  tft.drawString("**DARY MALINOVKY**", 10, 20); //draw string 
    
  //text strings 
  tft.setTextColor(TFT_WHITE); 
  tft.setFreeFont(FS18); 
  tft.drawString("Temperature:", 10,60);
  tft.drawString("Humidity:", 10,110);
  tft.drawString("Co2:", 10,160);
  
  tft.setTextColor(TFT_GREEN); 
  tft.setFreeFont(FMB24);  
  tft.drawString("C",260,60);
  tft.drawString("%", 215,100);
  tft.drawString("PPM", 200,150);
  tft.drawFastHLine(0,140,320,TFT_GREEN); //draw horizontal line
} 
void loop() {
  unsigned long currentTime = millis();
  if (!client.connected()) {
    reconnect();  
    }
      else{
    manual_control();
   }
  client.loop();
  if (digitalRead(ManualAutomatic) == HIGH) //Si esta pulsado ENCENDEMOS
 {
  //Serial.println(" Automatic");  
 }
 else if(digitalRead(ManualAutomatic) == LOW)
 {
  Serial.println(" Manual");
  manual_control();
  }
  if (currentTime - prevTime_T1 > interval) {
    leerdht1();
    prevTime_T1 = currentTime;
    }
  if (currentTime - prevTime_T2 > interval) {
    leerdht2();
    prevTime_T2 = currentTime;
    }
  if (currentTime - prevTime_T3 > interval) {
    leerdht3();
    prevTime_T3 = currentTime;
    }
  if (currentTime - prevTime_T4 > interval) {
    leerco2();
    prevTime_T4 = currentTime;
    }
 // delay(50); 
}
void setup_wifi() {
  
  delay(10);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
    digitalWrite(Relay1, digitalRead(switch1));
    digitalWrite(Relay2, digitalRead(switch2));
    digitalWrite(Relay3, digitalRead(switch3));
    digitalWrite(Relay4, digitalRead(switch4));
    delay(250);
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void reconnect() {
  while (!client.connected()) {
    if (client.connect(clientID, username, pass)) {
      Serial.println("MQTT connected");
      client.subscribe(sub1);
      client.subscribe(sub2);
      client.subscribe(sub3);
      client.subscribe(sub4);    
      //Serial.println(sub1);     
    } else {
      Serial.print("failed-reconnect1, rc=");
      Serial.print(client.state());
      Serial.println(" try again in few seconds");
      delay(200);// wait few sec and retry
//manual control when internet is not connected
    digitalWrite(Relay1, digitalRead(switch1));
    digitalWrite(Relay2, digitalRead(switch2));
    digitalWrite(Relay3, digitalRead(switch3));
    digitalWrite(Relay4, digitalRead(switch4));   
    }
  }
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("-----------------------");
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i< length; i++) {
  Serial.println((char)payload[i]);
  }
  if (strstr(topic, sub1))
  {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '0') {
      digitalWrite(Relay1, LOW);   // Turn the LED on (Note that LOW is the voltage level
      Serial.println("Relay1 ON");
    } else {
      digitalWrite(Relay1, HIGH);  // Turn the LED off by making the voltage HIGH
      Serial.println("Relay1 OFF");
    }
    Serial.println("-----------------------");
  }
  else if ( strstr(topic, sub2))
  {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '0') {
      digitalWrite(Relay2, LOW);   // Turn the LED on (Note that LOW is the voltage level
      Serial.println("Relay2 ON");

    } else {
      digitalWrite(Relay2, HIGH);  // Turn the LED off by making the voltage HIGH
      Serial.println("Relay2 OFF");
    }
  Serial.println("-----------------------");
  }
  else if ( strstr(topic, sub3))
  {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '0') {
      digitalWrite(Relay3, LOW);   // Turn the LED on (Note that LOW is the voltage level
      Serial.println("Relay3 ON");
    } else {
      digitalWrite(Relay3, HIGH);  // Turn the LED off by making the voltage HIGH
      Serial.println("Relay3 OFF");
    }
  Serial.println("-----------------------");    
  }
  else if ( strstr(topic, sub4))
  {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '0') {
      digitalWrite(Relay4, LOW);   // Turn the LED on (Note that LOW is the voltage level
      Serial.println("Relay4 ON"); 
    } else {
      digitalWrite(Relay4, HIGH);  // Turn the LED off by making the voltage HIGH
      Serial.println("Relay4 OFF");
    }
  }
  else
  {
    Serial.println("unsubscribed topic");
  }
}

void manual_control(){
         
 if (digitalRead(switch1) == HIGH)  {
    if (switch_ON_Flag1_previous_I == 0 )
    {
      digitalWrite(Relay1, HIGH);
      client.publish(pub1, "0");
      Serial.println("Relay1- ON");
      switch_ON_Flag1_previous_I = 1;
    }
    }
 if (digitalRead(switch1) == LOW ) {
    if (switch_ON_Flag1_previous_I == 1)
    {
      digitalWrite(Relay1, LOW);
      client.publish(pub1, "1");
      Serial.println("Relay1 OFF");
     switch_ON_Flag1_previous_I = 0;
    }
    }
 if (digitalRead(switch2) == HIGH)  {
    if (switch_ON_Flag2_previous_I == 0 )
    {
      digitalWrite(Relay2, HIGH);
      client.publish(pub2, "0");
      Serial.println("Relay2- ON");
      switch_ON_Flag2_previous_I = 1;
    }
    }
 if (digitalRead(switch2) == LOW)  {
    if (switch_ON_Flag2_previous_I == 1)
    {
      digitalWrite(Relay2, LOW);
      client.publish(pub2, "1");
      Serial.println("Relay2 OFF");
     switch_ON_Flag2_previous_I = 0;
    }
    }
 if (digitalRead(switch3) == HIGH) {
    if (switch_ON_Flag3_previous_I == 0 )    {
      digitalWrite(Relay3, HIGH);
      client.publish(pub3, "0");
      Serial.println("Relay3- ON");
      switch_ON_Flag3_previous_I = 1;
    }
    }
 if (digitalRead(switch3) == LOW ) {
    if (switch_ON_Flag3_previous_I == 1)
    {
      digitalWrite(Relay3, LOW);
      client.publish(pub3, "1");
      Serial.println("Relay3 OFF");
      switch_ON_Flag3_previous_I = 0;
    }
   }
 if (digitalRead(switch4) == HIGH) {
    if (switch_ON_Flag4_previous_I == 0 )
    {
      digitalWrite(Relay4, HIGH);
      client.publish(pub4, "0");
      Serial.println("Relay4- ON");
      switch_ON_Flag4_previous_I = 1;
    }
    }
 if (digitalRead(switch4) == LOW ) {
    if (switch_ON_Flag4_previous_I == 1)
    {
      digitalWrite(Relay4, LOW);
      client.publish(pub4, "1");
      Serial.println("Relay4 OFF");
      switch_ON_Flag4_previous_I = 0;
    }
   }
    delay(100);
}
void ControlAlarma() {
  if(t1<10 && t1>29)
  {
        Relay4_ON();
  } 
  
  if(t1<10 || t1>29)
  {
        Relay5_ON();
  }

}
void ControlVentilador() {
  if(t1>=36)
  {
        Relay4_ON();
  }
  else
  {
        Relay3_OFF();
  }
}
void ControlRiego() {
  if (nivelAgua == 0 && botonest == 1) { 
    Serial.println("Nivel bajo de Agua. Rellenar el tanque");
  }
  if (nivelAgua == 0 && botonest == 0) { 
    Serial.println("Imposible regar, nivel bajo de agua. Debe rellenar el tanque");
  }
  if (nivelAgua == 1) {
    Serial.println("Nivel de agua correcto, se puede regar");

    if (suelo0 <= 300) // el valor que considero seco y hay que regar es de 700
    {
      // Si la tierra está seca, comenzara a regar
      Serial.println("La tierra está seca, comienza el riego automático");
      Relay3_ON ();
      if (botonest == 0)  { 
        Serial.println("No se puede activar riego manual. Riego automático activo");
      }
    }
    if (suelo0 >=20 || suelo0 <=26) {//no es necesario regar, pero puede regar de manera manual
      if (botonest == 0) {
        Serial.println("Regando de forma manual");
        Relay3_ON();
      }
      if (botonest == 1) {
        Relay3_OFF();
      }
    }
  }
  }
void ControlTecho() { 
  
  
}

void leerdht1() {  
    t1 = sht31.getTemperature();
    dtostrf(t1,0, 0, buffer);
    client.publish(topic_T_0, buffer); 
    h1 = sht31.getHumidity();
    dtostrf(h1,0, 0, buffer);
    client.publish(topic_H_0, buffer); 
#if (Debug == 1)
   Serial.print("Temperatura SHT_0: ");
   Serial.print(t1);
   Serial.println(" ºC.");
   Serial.print("Humedad SHT_0: ");
   Serial.print(h1);
   Serial.println(" %."); 
#endif
   //sprite buffer for temperature
  spr.createSprite(55, 40); //create buffer
  spr.fillSprite(TFT_BLACK); //fill background color of buffer
  spr.setFreeFont(FMB24); //set font type 
  spr.setTextColor(TFT_WHITE); //set text color
  spr.drawNumber(t1, 0, 0); //display number 
  spr.pushSprite(200, 50); //push to LCD 
  spr.deleteSprite(); //clear buffer

  //sprite buffer for humidity 
  spr.createSprite(55, 40);
  spr.fillSprite(TFT_BLACK);
  spr.setFreeFont(FMB24);
  spr.setTextColor(TFT_WHITE);
  spr.drawNumber(h1, 0, 0); //display number 
  spr.pushSprite(155, 100);
  spr.deleteSprite(); 
}
void leerdht2() { 
  t2 = dht2.readTemperature();    
  dtostrf(t2,0, 0, buffer);
  client.publish(topic_T_1, buffer); 
  h2 = dht2.readHumidity();
  dtostrf(h2,0, 0, buffer);
  client.publish(topic_H_1, buffer); 
#if (Debug == 1)
  Serial.print("Temperatura DHT_1: ");
  Serial.print(t2);
  Serial.println(" ºC.");
  Serial.print("Humedad DHT_2: ");
  Serial.print(h2);
  Serial.println(" %."); 
#endif
}
void leerdht3() { 
  t3 = dht3.readTemperature();    
  dtostrf(t3,0, 0, buffer);
  client.publish(topic_T_2, buffer); 
  h3 = dht3.readHumidity();
  dtostrf(h3,0, 0, buffer);
  client.publish(topic_H_2, buffer); 
#if (Debug == 1)
  Serial.print("Temperatura DHT_3: ");
  Serial.print(t3);
  Serial.println(" ºC.");
  Serial.print("Humedad DHT_3: ");
  Serial.print(h3);
  Serial.println(" %."); 
#endif
}
void leerco2() { 
  
  
  myCo2.write(request, 9);
  memset(response, 0, 9);
  myCo2.readBytes(response, 9);
  int i;
  byte crc = 0;
  for (i = 1; i < 8; i++) crc+=response[i];
  crc = 255 - crc;
  crc++;
  
  if ( !(response[0] == 0xFF && response[1] == 0x86 && response[8] == crc) ) {
 #if (Debug == 1)
   Serial.println("CRC error");
 #endif
  } else {
    unsigned int HLconcentration = (unsigned int) response[2];
    unsigned int LLconcentration = (unsigned int) response[3];
    co2 = (256*HLconcentration) + LLconcentration;
    dtostrf(co2,0, 0, buffer);
    client.publish(topic_co2, buffer); 
#if (Debug == 1)
    Serial.println(co2);
#endif
    for (i = 0; i < 9; i++) {
      Serial.print("0x");
      Serial.print(response[i],HEX);
      Serial.print("  ");
    }
#if (Debug == 1)
    Serial.println("  ");
#endif
  }  
  //sprite buffer for Co2 
  spr.createSprite(55, 40);
  spr.fillSprite(TFT_BLACK);
  spr.setFreeFont(FMB24);
  spr.setTextColor(TFT_WHITE);
  //spr.drawNumber(co2, 0, 0); 
  spr.drawNumber(co2, 0, 0, 4);
  spr.pushSprite(100, 160);
  spr.deleteSprite();
}
void leerLuz_0() { 
  luz0 = analogRead(ValorLuz);
#if (Debug == 1)
  Serial.print("luz_0 = ");
  Serial.println(valorLDR0);
  dtostrf(suelo0,0, 0, buffer);
  client.publish(topic_luz, buffer);
#endif
}
void leerMoisture0() {
  
  
}

void boton1() {
  buttonNew = digitalRead(buttonPin);
  delay(100);
  if(buttonNew == 0 && buttonOld == 1){
    if (ledState == 0){
    digitalWrite(ledPin, 1);
    ledState = 1;
      Serial.println(" Automatic");
    }
    else {
      digitalWrite (ledPin,0);
      ledState = 0;
        Serial.println(" Manual");
    }
  }
  buttonOld=buttonNew;
  }
void boton2() {
  buttonNew = digitalRead(buttonPin);
  delay(100);
  if(buttonNew == 0 && buttonOld == 1){
    if (ledState == 0){
    digitalWrite(ledPin, 1);
    ledState = 1;
      Serial.println(" Automatic");
    }
    else {
      digitalWrite (ledPin,0);
      ledState = 0;
        Serial.println(" Manual");
    }
  }
  buttonOld=buttonNew;
  }
void boton3() {
  buttonNew = digitalRead(buttonPin);
  delay(100);
  if(buttonNew == 0 && buttonOld == 1){
    if (ledState == 0){
    digitalWrite(ledPin, 1);
    ledState = 1;
      Serial.println(" Automatic");
    }
    else {
      digitalWrite (ledPin,0);
      ledState = 0;
        Serial.println(" Manual");
    }
  }
  buttonOld=buttonNew;
  }
  
void Relay1_ON () {
  digitalWrite(Relay1, LOW);
}
void Relay1_OFF () {         
  digitalWrite(Relay1, HIGH);
}
void Relay2_ON () {
  digitalWrite(Relay2, LOW);  
}
void Relay2_OFF () {
  digitalWrite(Relay2, HIGH);  
}
void Relay3_ON () {
  digitalWrite(Relay3, LOW);  
}
void Relay3_OFF () {
  digitalWrite(Relay3, HIGH);    
}
void Relay4_ON () {
  digitalWrite(Relay4, LOW);  
}
void Relay4_OFF () {
  
  digitalWrite(Relay4, HIGH);  
}
void Relay5_ON () {
  digitalWrite(Relay5, LOW);
}
void Relay5_OFF () {         
  digitalWrite(Relay5, HIGH);
}
void Relay6_ON () {
  digitalWrite(Relay6, LOW);  
}
void Relay6_OFF () {
  digitalWrite(Relay6, HIGH);  
}
void Relay7_ON () {
  digitalWrite(Relay7, LOW);  
}
void Relay7_OFF () {
    digitalWrite(Relay7, HIGH);
}

  
