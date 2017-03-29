// Librería WiFiManager https://github.com/tzapu/WiFiManager
// Comunicación con Telegram https://create.arduino.cc/projecthub/Arduino_Genuino/telegram-bot-library-ced4d4
// Twitter + Arduino arduino-tweet.appspot.com
// Si necesitas ayuda con este código escríbeme en twitter.com/spamloco

#include <ESP8266WiFi.h>
#include <Twitter.h>
#include <DNSServer.h>            //Servidor DNS local para redireccionar peticiones al portal de configuración
#include <ESP8266WebServer.h>     //Webserver local para el portal de configuración
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <SPI.h>  
#include <TelegramBot.h> 

byte sensorPin = 2; // Pin para el sensor de movimiento
byte movimiento = 0; // Variable para activar alarma
byte sinmovimiento = 0; // Resetea el contador a 0 cuando no hay movimiento durante un tiempo
long Numerorandom; // Número ramdom para enviar por Twitter
char msg[140]; // Mensaje para Twitter
Twitter twitter("xxxxxxxxxxxxxxxxxxxxxxxx"); // Token de Twitter se obtiene de arduino-tweet.appspot.com
int repiteloop = false;
const char* BotToken = "xxxxxxxxxxxxxxxxxxx";    // Token de Telegram se obtiene configurando el Bot

WiFiClientSecure client;
TelegramBot bot(BotToken,client);

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Accediendo al Modo de Configuración");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

//flag para guardar datos
bool shouldSaveConfig = false;

//callback para guardar datos
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setup()
{
  
  Serial.begin(115200); 
  delay(10);
  pinMode(sensorPin,INPUT);
   
  WiFiManager wifiManager;
  
  wifiManager.setAPCallback(configModeCallback); //Cuando la conexión al WiFi falla, se entra en el modo AP
  wifiManager.setConnectTimeout(30); // Se da un tiempo de 30 segundos para iniciar la conexion a la red WiFi
  wifiManager.autoConnect("ESP01-SpamLoco"); // Nombre de la red WiFi creada por la ESP8266-01

  // Se intenta una autoconexión y si falla se espera por una configuración
  if(!wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  } 

  // Conexión a la red wifi exitosa
  Serial.println("Conectado de forma exitosa a la red WiFi");

 //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

}


void loop()
{  

  message m = bot.getUpdates(); // A la espera de nuevos mensajes de Telegram

  // Si el mensaje es Alarmaon se activa la alarma, es sensible a mayúsculas
  if (m.text.equals("Alarmaon")) 
  {  
    
    Serial.println("Mensaje recibido se activa la alarma");  
    bot.sendMessage(m.chat_id, "La Alarma está preparada y lista.");
    movimiento = 0;
    repiteloop = false;

    while (!repiteloop) 
    {
      byte state = digitalRead(sensorPin);
 
      if(state == 1) {
        Serial.println("Se ha detectado movimiento!");
        delay(50);
        movimiento++;
        Serial.println(movimiento);
      }
      else if (state == 0) {
        delay(10);
        sinmovimiento++;
        Serial.println("Sin movimiento!");
      }

    // Se resetea el contador de movimientos si no hay movimiento continuo
   if (sinmovimiento == 10) {
    sinmovimiento = 0;
    movimiento = 0;
   }

   // Se activa la alarma y se envía el mensaje a Telegram 
   
    if (movimiento == 3) 
    {   
      bot.sendMessage(m.chat_id, "Alarma SpamLoco activada por movimiento!!"); //Mensaje de alerta Telegram

      // Envía el Tweet junto a un número random para que los tweets sean diferentes
     // Si son siempre iguales Twitter los bloquea

     Numerorandom = random(101,1565);
     Serial.println(Numerorandom);
     sprintf(msg, "Alarma spamloco activada!: %d.", Numerorandom);
     Serial.println(msg);
     Serial.println("Conectando a Twitter…");

      if (twitter.post(msg)) {
        int status = twitter.wait(&Serial);
        if (status == 200) {
        Serial.println("200 OK");
        delay(60001); // Si el mensaje se envía se espera 1 minuto para evitar spam en arduino-tweet.appspot.com
      } 
      else {
        Serial.print("Error : code ");
        Serial.println(status);
      }
  } else {
    Serial.println("Fallo en la conexión.");
  }
      
      movimiento = 0; // Variable de movimientos vuelve a cero
    } 

     message m = bot.getUpdates(); // Leer nuevos mensajes de Telegram
     if (m.text.equals("Alarmaoff")) 
     {
      repiteloop = true;
      Serial.println("Mensaje recibido, se apaga la alarma.");
      bot.sendMessage(m.chat_id, "La Alarma está desactivada.");
     }
  }     
     
  }
  
  else if (m.text.equals("Alarmaoff")) 
       {  
   Serial.println("Mensaje recibido, se apaga la alarma.");  
   bot.sendMessage(m.chat_id, "La Alarma está desactivada.");
   repiteloop = false;  
 }   
   
} // fin loop
