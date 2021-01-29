#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define DHTPIN D8

#define DHTTYPE DHT22   // DHT 22
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);

// Initialize Wifi connection to the router
char ssid[] = "hostpot";     // nama wifi
char password[] = "123456789b"; // password wifi
// Initialize Telegram BOT4
#define BOTtoken "1120793855:AAHVSsygLTBGooc0pF8y4tJHJ1EsDkztOH0" // Token Bot (Dapat dari Telegram Botfather)
//#define chat_id "640002329"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
int relay = D6;
int h = 0;
float t = 0;
int val = 0;

int pompa = 1;
bool autoMode = true;

int krm1 =0;
int krm2 =0;
long duration;
String chat_id="";
boolean krmPesan1 = true;
boolean krmPesan2 = false;

//Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    //Kontrol Modul Relay (nyala Pompa)
    if (text == "/on") {
      bot.sendChatAction(chat_id, "typing");
      autoMode = false;
      pompa = 0;
      digitalWrite(relay, HIGH);
      delay(1500);
      bot.sendMessage(chat_id, "spray dinyalakan", "");
    }
    if (text == "/off") {
      bot.sendChatAction(chat_id, "typing");
      autoMode = false;
      pompa = 1;
      bot.sendMessage(chat_id, "spray dimatikan", "");
    }
    if (text == "/auto") {
      bot.sendChatAction(chat_id, "typing");
      autoMode = true;
      bot.sendMessage(chat_id, "AUTO MODE ON", "");
    }
    digitalWrite(relay, pompa);
    //Cek Pembacaan Sensor
    if (text == "/cek") {
      bot.sendChatAction(chat_id, "typing");
      String welcome = "Data sensor untuk " + from_name + ".\n\nKeadaan Ruangan: ";
      welcome += "\nSuhu ";
      welcome += t;
      welcome += "C";
      welcome += "\nKelembaban ";
      welcome += h;
      welcome += "%";
      welcome += "\nspray dalam kondisi ";
      boolean a = pompa;
      if (!a)welcome += "menyala.";
      else welcome += "mati.";
      bot.sendMessage(chat_id, welcome);
    }
    //Cek Command untuk setiap aksi
    if (text == "/start") {
      bot.sendChatAction(chat_id, "typing");
      String welcome = "Selamat datang  " + from_name + ".\n";
      welcome += "/cek : cek Kondisi Ruangan\n";
      welcome += "/on : Nyalakan spray\n";
      welcome += "/off : Matikan spray\n";
      welcome += "/auto : otomatis\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}


void setup() {

 
  pinMode(relay, OUTPUT);
  digitalWrite(relay , HIGH);
  dht.begin();
  lcd.init();
  lcd.backlight();
  Serial.begin(115200);
  // This is the simplest way of getting this working
  // if you are passing sensitive information, or controlling
  // something important, please either use certStore or at
  // least client.setFingerPrint
  client.setInsecure();

  // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  lcd.print("Connecting...");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connected");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(500);
  lcd.clear();
}

void loop() {

   h = dht.readHumidity();
   t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(10);
  }
  if (autoMode) {
    otomatis();
    Serial.println("AUTO ON");
  }
   if (millis() > lastTimeBotRan + botRequestDelay)  {
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  
      while (numNewMessages) {
        Serial.println("got response");
        handleNewMessages(numNewMessages);
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }
  
      lastTimeBotRan = millis();
    }

  Serial.println("spray : " + String(pompa));
  Serial.println("AUTO : " + String(autoMode));
  Serial.println("suhu : " + String(analogRead(t)));
  Serial.println("kelembaban : " + String(analogRead(h)));
  delay(500);
  tampilan();
}
void tampilan() {

  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(t);
  lcd.print(" C");
  lcd.setCursor(10, 0);
  lcd.print(" H:");
  lcd.print(h);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("spray     :");
  lcd.setCursor(12, 1);
  boolean a = pompa;
  if (!a) {
    lcd.print("ON   ");
  } else {
    lcd.print("OFF   ");

  } 

}

void otomatis() {
 
  if (( t >= 30)||(h <= 70 ))
    {
    pompa = 0;
    digitalWrite(relay, HIGH);
      delay(1500);
    Serial.println("spray Nyala");
    krmPesan1 = false;
    krmPesan2 = true;
    krm1=1;
    }
  else
    {
    krmPesan2;
    pompa = 1;    
    Serial.println("spray Mati");
    krmPesan1 = true;
    krmPesan2 = false;
    krm2=1;
    }
  
    digitalWrite(relay, pompa);
}
