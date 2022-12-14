/*
 * Author: Khadija Diallo
 *khadi@tingene.com
 *Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
 *The above copyright notice and this permission notice shall be included in all copies or substantial portions of
 */

#include <FS.h> //ette ligne doit etre au debut du programme sinon celui ci pourrait crasher
#include <MFRC522.h> // librairie pour RFID
#include <ESP8266WiFi.h> // librairie pour le wifi de l'esp8266
#include <NTPClient.h> //protocole pour le temps
#include <WiFiUdp.h> // librairie pour l7ecoute en UDP
#include <WiFiManager.h> //gestion du wifi https://github.com/tzapu/WiFiManager
#include <WiFiClient.h> // librairie pour le client wifi
#include <ESP8266HTTPClient.h> //librairie pour envoyer les requets http
#include <ESP8266WebServer.h> //librairie pour recevoir les requets http
#include <ArduinoJson.h> // librairie pour wifi manager// https://github.com/bblanchon/ArduinoJson



//serveur wifi
WiFiServer server(80);
const char* host = "tingene.com";
const int httpPort = 80;

String HTTP_METHOD = "GET";

//client wifi
WiFiClient client;

//Cablages
/*
 *  RC1             wemos
 * ------------------------
 *  GND              GND
 *  3.3V             3.3V
 *  RST              D0
 *  SDA              D8
 *  SCK              D5
 *  MOSI             D7
 *  MISO             D6
 */


// definir les pins pour RFID
#define CS_RFID D8 
#define RST_RFID D0



//les parametres wifi

const char* ssid = "your_wifi_name";
const char* password = "your wifi_password";

MFRC522::MIFARE_Key key;

// Instance de classe pour RFID
MFRC522 rfid(CS_RFID, RST_RFID);

// Variable de d'UID (identifiant de la carte)
String uidString;


// Temps par defaut de pointage (Apres 9h 0, l'utilisateur est en retard)
const int checkInHour = 9; 
const int checkInMinute = 0;

//Variables pour l'heure de pointage
int userCheckInHour;
int userCheckInMinute;

// definir le temps actuelle
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Pins pour lesLEDs
const int redLED = D3;//EREUR DANS LE POINTAGE: multi-ENTREES ou multi-sorties successives
const int greenLED = D4;//no error

bool shouldSaveConfig = false;

String etat = ""; ///entree ou sortie





//WIfImanager


#define NUL '\0'
HTTPClient http;

// WiFiManager nécessitant un rappel de sauvegarde de la configuration
void saveConfigCallback () {
shouldSaveConfig = true;
}

// WiFiManager entrant dans le rappel du mode de configuration
void configModeCallback(WiFiManager *myWiFiManager) {

}

// sauvegarder les parametres personnalisés dans /config.json sur SPIFFS
void save_settings() {
DynamicJsonBuffer jsonBuffer;
JsonObject& json = jsonBuffer.createObject();


File configFile = SPIFFS.open("/config.json", "w");
json.printTo(configFile);
configFile.close();
}

// Charge les paramètres personnalisés à partir de /config.json sur SPIFFS
void load_settings() {
if (SPIFFS.begin() && SPIFFS.exists("/config.json")) {
File configFile = SPIFFS.open("/config.json", "r");

if (configFile) {
size_t size = configFile.size();
// Allouer une mémoire tampon pour stocker le contenu du fichier.
std::unique_ptr<char[]> buf(new char[size]);

configFile.readBytes(buf.get(), size);
DynamicJsonBuffer jsonBuffer;
JsonObject& json = jsonBuffer.parseObject(buf.get());

if (json.success()) {
Serial.print("json success");
}
}
}
}


//endWIFImanager

void setup() {

Serial.begin(115200);

// Connexion WiFi
Serial.println();
Serial.println();
Serial.print("Connecting to ");
Serial.println(ssid);
WiFi.hostname("Name");
WiFi.begin(ssid, password);

while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");
}
Serial.println("");
Serial.println("WiFi connected");

// Ecrire l'address IP
Serial.print("IP address: ");
Serial.print(WiFi.localIP());


// Définir les LED comme sorties
pinMode(redLED, OUTPUT);
pinMode(greenLED, OUTPUT);

// Initialiser le Serial port

SPI.begin();


// Initialiser le MFRC522
rfid.PCD_Init(); // Initialiser MFRC522
rfid.PCD_DumpVersionToSerial(); // Afficher les détails de PCD - Détails du lecteur de carte MFRC522
Serial.println(F("Scannez le PICC pour voir l'UID, le SAK, le type et les blocs de données..."));
Serial.println("Configuration terminée");



//Configurer le NTP
timeClient.begin();
timeClient.setTimeOffset(0);
Serial.println("time got");


load_settings();

WiFiManager wifiManager;


wifiManager.setAPCallback(configModeCallback);
wifiManager.setSaveConfigCallback(saveConfigCallback);


// essayez de se connecter ou de revenir au mode de configuration ESP + ChipID AP.
if ( ! wifiManager.autoConnect()) {
// rréinitialisez et réessayez, ou mettre en veille profonde
ESP.reset();
delay(1000);
}

}

void loop() {

//réglage de l'heure
timeClient.update();
userCheckInHour = timeClient.getHours();
userCheckInMinute = timeClient.getMinutes();
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};


String heure = timeClient.getFormattedTime();
time_t epochTime = timeClient.getEpochTime();

//definir un format pour le temps
struct tm *ptm = gmtime ((time_t *)&epochTime);

int monthDay = ptm->tm_mday;

int currentMonth = ptm->tm_mon+1;

String currentMonthName = months[currentMonth-1];

int currentYear = ptm->tm_year+1900;

//Afficher la date complete:
String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);


//chercher de nouvelles cartes
if(rfid.PICC_IsNewCardPresent()) {
//Lire la carte RFID
rfid.PICC_ReadCardSerial();
Serial.print("Tag UID: ");
uidString = String(rfid.uid.uidByte[0]) + " " + String(rfid.uid.uidByte[1]) + " " +
String(rfid.uid.uidByte[2]) + " " + String(rfid.uid.uidByte[3]);
Serial.println(uidString);
//delay(10);
//Verifier l'heure du pointage
verifyCheckIn();
Serial.print("Current date: ");
Serial.println(currentDate);
Serial.print("Current hour: ");
Serial.println(heure);


etat = "entree"; // mettre entree ou sortie selon l'etat que vous decidez pour la carte
Serial.println("Etat = "+etat);

//Requete
String url = "/insert_arduino_data.php?uidString='"+uidString+"'&currentDate='"+currentDate+"'&heure='"+heure+"'&etat='"+etat+"'";
 

//Serial.print("url: ");
// se connecter au serveur Web sur le port httpPort :
if(client.connect(host, httpPort)) {
// si c'est connecté :
Serial.println("connecté au server");
// faire une requête HTTP
client.print(String("GET ") + url + " HTTP/1.1\r\n" +"Host: " + host +"\r\n" +"User-Agent: BuildFailureDetectorESP8266\r\n" +"Connection: close\r\n\r\n");
while(client.connected()){
 String line = client.readString();

 if ((line.indexOf("New record created successfully")) >=0){////si line (la reponse http) contient "New...." ce qui vient du serveur nginx (ce code sera dans un autre blog. InshAllah)
  Serial.println("Led Verte");
  /////LED Verte
  digitalWrite(greenLED, HIGH); 
  delay(3000);
  digitalWrite(greenLED, LOW); 
}else{
  Serial.println("Led Rouge");
  /////LED Verte
  digitalWrite(redLED, HIGH); 
  delay(3000);
  digitalWrite(redLED, LOW); 
}
client.stop();

}
}else {
Serial.println("Echec de la connexion");
}
}
}


void readRFID() {
rfid.PICC_ReadCardSerial();
Serial.print("Tag UID: ");
uidString = String(rfid.uid.uidByte[0]) + " " + String(rfid.uid.uidByte[1]) + " " +
String(rfid.uid.uidByte[2]) + " " + String(rfid.uid.uidByte[3]);
Serial.println(uidString);
}


void verifyCheckIn(){
if((userCheckInHour < checkInHour)||((userCheckInHour==checkInHour) && (userCheckInMinute <= checkInMinute))){
Serial.println("Bienvenue! Vous etes a l'heure");
}
else{
//digitalWrite(redLED, HIGH);
Serial.println("Tu es en retard...");
}
}
