#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <PubSubClient.h>



//EDIT THESE LINES TO MATCH YOUR SETUP
#define MQTT_SERVER_LAN "192.168.1.106"
#define MQTT_SERVER_WAN "giovanazzi.ddns.net"

char* SERVER_LAN = " ";
char* SERVER_WAN =" ";

/////////// antirebote /////////////
volatile int contador = 0;   // Somos de lo mas obedientes
int n = contador ;
long T0 = 0 ;  // Variable global para tiempo

volatile int contador2 = 0;   // Somos de lo mas obedientes
int n2 = contador2 ;
long T02 = 0 ;  // Variable global para tiempo

const int Boton_EEPROM=0;
const int Boton=2;
const int LED=16;
const int sw=12;
const int swB=13;
const int relay=4;
const int relayB=5;

volatile int tiempoLed=800000000;
int address = 0;
byte value;
byte modo=0;
/* Set these to your desired credentials. */
const char* ssid_AP = "ESP8266";
const char* password_AP = "PASSWORD";
const int channel=11;
const int hidden=0;

int cont_mqtt=0;
String Wan=" ";

ESP8266WebServer server(80);

char ssid[20];
char pass[20];
char Topic1[20];
char Topic2[20];
char ServerWan[30];
char ServerLan[20];

String ssid_leido;
String pass_leido;
String Topic1_leido;
String Topic2_leido;
String ServerWan_leido;
String ServerLan_leido;

String scanWifi;

int ssid_tamano = 0;
int pass_tamano = 0;
int Topic1_tamano = 0;
int Topic2_tamano = 0;
int ServerWan_tamano = 0;
int ServerLan_tamano = 0;

////// ADDRESS EEPROM
int dir_modo= 0; //0=normal 1 configuracion
int dir_conf = 70;
int dir_ssid = 1;
int dir_pass = 30;
int dir_topic1 = 100;
int dir_topic2 = 130;
int dir_serverwan = 150;
int dir_serverlan = 190;

String arregla_simbolos(String a) {
  a.replace("%C3%A1", "Ã¡");
  a.replace("%C3%A9", "Ã©");
  a.replace("%C3%A", "i");
  a.replace("%C3%B3", "Ã³");
  a.replace("%C3%BA", "Ãº");
  a.replace("%21", "!");
  a.replace("%23", "#");
  a.replace("%24", "$");
  a.replace("%25", "%");
  a.replace("%26", "&");
  a.replace("%27", "/");
  a.replace("%28", "(");
  a.replace("%29", ")");
  a.replace("%3D", "=");
  a.replace("%3F", "?");
  a.replace("%27", "'");
  a.replace("%C2%BF", "Â¿");
  a.replace("%C2%A1", "Â¡");
  a.replace("%C3%B1", "Ã±");
  a.replace("%C3%91", "Ã‘");
  a.replace("+", " ");
  a.replace("%2B", "+");
  a.replace("%22", "\"");
  return a;
}

String pral = "<html>"
              "<meta http-equiv='Content-Type' content='text/html  ; charset=utf-8'/>"
              "<title>CONFIG ESP8266</title> <style type='text/css'> body,td,th { color: #036; } body { background-color: #999; } </style> </head>"
              "<body> "
              "<h1>CONFIGURACION</h1><br>"
              "<form action='config' method='get' target='pantalla'>"
              "<fieldset align='center' style='border-style:solid; border-color:#336666; width:300px; height:500px; padding:10px; margin: 5px;'>"
              "<legend><strong>Configurar WI-FI</strong></legend>"
              "SSID: <br> <input name='ssid' type='text' size='15'/> <br><br>"
              "PASSWORD: <br> <input name='pass' type='password' size='15'/> <br><br>"
              "TOPIC1: <br> <input name='topic1' type='text' size='15'/> <br><br>"
              "TOPIC2: <br> <input name='topic2' type='text' size='15'/> <br><br>"
              "SERVER WAN: <br> <input name='serverwan' type='text' size='15'/> <br><br>"
              "SERVER LAN: <br> <input name='serverlan' type='text' size='15'/> <br><br>"
              "<input type='submit' value='Configurar Equipo' />"
              "</fieldset>"
              "</form>"
              "<iframe id='pantalla' name='pantalla' src='' width=900px height=400px frameborder='0' scrolling='no'></iframe>"
              "</body>"
              "</html>";



WiFiClient wifiClient;

/********************* FIN DECLARACION DE VARIABLES   *************************/




void setup(){
  
  pinMode(Boton_EEPROM, INPUT);
  pinMode(Boton, INPUT_PULLUP);
  pinMode(LED,OUTPUT);
  pinMode(sw, INPUT_PULLUP);
  pinMode(swB, INPUT_PULLUP);
  pinMode(relay,OUTPUT);
  pinMode(relayB,OUTPUT);
  //digitalWrite(relay,false);
  
  attachInterrupt( digitalPinToInterrupt(Boton_EEPROM), ServicioBoton, RISING);
  attachInterrupt( digitalPinToInterrupt(sw), ServicioBoton2, RISING);
  
  Serial.begin(115200);
  Serial.println();

  EEPROM.begin(256);
  delay(10);
  
  value = EEPROM.read(0);//carga el valor 1 si no esta configurado o 0 si esta configurado
  delay(10);
  ReadDataEprom();
  
  Serial.print("Configuracion: ");
  Serial.println(lee(dir_conf));
  
  if(lee(dir_conf)!="configurado"){
    value=1;
    
    }
    
  if(value){
            delay(10);
            Serial.println("**********MODO CONFIGURACION************");
             scanWIFIS();
            Serial.print("Configuring access point...");
            WiFi.mode(WIFI_AP);
            WiFi.softAP(ssid_AP, password_AP,channel,hidden);// (*char SSID,*char PASS,int CHANNEL,int HIDDEN=1 NO_HIDDEN=0)
            IPAddress myIP = WiFi.softAPIP();
            Serial.print("AP IP address: ");
            Serial.println(myIP);
            
            server.on("/", []() {server.send(200, "text/html", pral);});
            server.on("/config", wifi_conf);
            server.begin();
            
            Serial.println("**********  Webserver iniciado ***************");
            Serial.print("ssid: "); Serial.println(ssid_AP);
            Serial.print("password: "); Serial.println(password_AP);
            Serial.print("channel: "); Serial.println(channel);
            Serial.print("hidden: "); Serial.println(hidden);
            Serial.println();

      }
  else{    Serial.println("**********MODO NORMAL************");  
         
           
           ServerLan_leido= lee(dir_serverlan);
           ServerWan_leido= lee(dir_serverwan);
           ServerLan_tamano = ServerLan_leido.length() + 1;  //Calculamos la cantidad de caracteres que tiene el ssid y la clave
           ServerWan_tamano = ServerWan_leido.length() + 1;
           ServerLan_leido.toCharArray(SERVER_LAN, ServerLan_tamano); //Transf. el String en un char array ya que es lo que nos pide WiFi.begin()
           ServerWan_leido.toCharArray(SERVER_WAN, ServerWan_tamano);
           
           WiFi.mode(WIFI_STA);
           
           intento_conexion();
    }
  
  modo=0;//normal
  EEPROM.write(0,modo);
  EEPROM.commit();
}

PubSubClient client(MQTT_SERVER_WAN, 1883, callback, wifiClient);

void loop(){
  if(value){
        server.handleClient();
        delay(500);
        digitalWrite(LED,!digitalRead(LED));
       
        }
  else{ 
       //maintain MQTT connection
       client.loop();
       delay(10);
       if (WiFi.status() == WL_CONNECTED) { 
            digitalWrite(LED,true);
            reconexionMQTT();
         }else{
           digitalWrite(LED,false);
           intento_conexion();
           }
       
        //MUST delay to allow ESP8266 WIFI functions to run
        delay(10);   
        if (n2 != contador2){
            n2 = contador2 ;
            digitalWrite(relay,!digitalRead(relay));
            if(digitalRead(relay)){client.publish("prueba/light1/confirm", "Light1 On");
                                        Serial.println("Relay ON!!");
                                      }
             else{client.publish("prueba/light1/confirm", "Light1 Off");
                     Serial.println("Relay OFF!!");}
             }
       
       }
    if (n != contador){
             blink50();
              n = contador ;
              modo=1;
              EEPROM.write(0,modo);
              EEPROM.commit();
              delay(10);
              ESP.reset();       
         }
}

/*******************  INICIO DECLARACION DE FUNCIONES        *********************************/

/////////////   MQTT //////////////////// 

void callback(char* topic, byte* payload, unsigned int length) {

  //convert topic to string to make it easier to work with
  String topicStr = topic; 

  //Print out some debugging info
  Serial.println("Callback update.");
  Serial.print("Topic: ");
  Serial.println(topicStr);
  
   if(topicStr == Topic1){

      //turn the light on if the payload is '1' and publish to the MQTT server a confirmation message
        if(payload[0] == '1'){
          digitalWrite(relay, HIGH);
          //client.publish(Topic1+'/confirm', "Light1 On");
          client.publish("prueba/light1/confirm", "Light1 On");
          }
      //turn the light off if the payload is '0' and publish to the MQTT server a confirmation message
        else// if (payload[0] == '0'){
          {digitalWrite(relay, LOW);
         // client.publish(Topic1+'/confirm', "Light1 Off");
         client.publish("prueba/light1/confirm", "Light1 Off");
        }
   }
  
}

void reconexionMQTT(){

    int cuenta=0;
    
    while (!client.connected()) {
      
       if (WiFi.status() != WL_CONNECTED) {
        ESP.reset();
       }
        
       if (n != contador){ // consulta si se oprimio el boton de MODO CONFIGURACION
              blink50();
              n = contador ;
              modo=1;
              EEPROM.write(0,modo);
              EEPROM.commit();
              delay(10);
              ESP.reset();       
         }
 
       BotonSW();
      
       Serial.println("Attempting MQTT connection...");

      // Generate client name based on MAC address and last 8 bits of microsecond counter
      String clientName;
      
      clientName += "esp8266-";
      uint8_t mac[6];
      WiFi.macAddress(mac);
      clientName += macToStr(mac);
       Serial.println(clientName);

      //if connected, subscribe to the topic(s) we want to be notified about
      if (client.connect((char*) clientName.c_str(),"diego","24305314")){
        Serial.println("MTQQ Connected");
        
         client.subscribe(Topic1);
         client.subscribe(Topic2);
         digitalWrite(LED,true);// wifi + mqtt ok !!!
      }
      //otherwise print failed for debugging
      else{
           digitalWrite(LED,true);
           Serial.println("Failed  connection mqtt.");
           Serial.print("Client state: ");
           Serial.println(client.state());
           Serial.println(" try again in 3 seconds");
           // Wait 3 seconds before retrying
           blinkLento();
           cuenta++;   
           Serial.print("cuenta: ");Serial.println(cuenta);
           if(cuenta>4){
            Serial.println("abortooo");
              ESP.reset();
             }
         }
    }
      
  
  }

/////////////// FIN MQTT //////////////////////

void intento_conexion() {
  
  if (lee(dir_conf).equals("configurado")) {
     
    Serial.print("SSID: ");  //Para depuracion
    Serial.println(ssid_leido);  //Para depuracion
    Serial.print("PASS: ");  //Para depuracion
    Serial.println(pass_leido);
    Serial.print("TOPIC 1: ");  //Para depuracion
    Serial.println(Topic1_leido);  //Para depuracion
    Serial.print("TOPIC 2: ");  //Para depuracion
    Serial.println(Topic2_leido);
    Serial.print("Server Lan MQTT: ");  //Para depuracion
    Serial.println(ServerLan_leido);  //Para depuracion
    Serial.print("Server Wan MQTT: ");  //Para depuracion
    Serial.println(ServerWan_leido);

    ssid_tamano = ssid_leido.length() + 1;  //Calculamos la cantidad de caracteres que tiene el ssid y la clave
    pass_tamano = pass_leido.length() + 1;
    Topic1_tamano = Topic1_leido.length() + 1;  //Calculamos la cantidad de caracteres que tiene el ssid y la clave
    Topic2_tamano = Topic2_leido.length() + 1;

    ssid_leido.toCharArray(ssid, ssid_tamano); //Transf. el String en un char array ya que es lo que nos pide WiFi.begin()
    pass_leido.toCharArray(pass, pass_tamano);
    Topic1_leido.toCharArray(Topic1, Topic1_tamano); //Transf. el String en un char array ya que es lo que nos pide WiFi.begin()
    Topic2_leido.toCharArray(Topic2, Topic2_tamano);

    int cuenta = 0;
    WiFi.begin(ssid, pass);      //Intentamos conectar
    while (WiFi.status() != WL_CONNECTED) {
      delay(400);
      blink50();
      cuenta++;
      if (cuenta > 20) {
        Serial.println("Fallo al conectar");
        return;
      }
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Conexion exitosa a: ");
    Serial.println(ssid);
    Serial.println(WiFi.localIP());
    digitalWrite(LED,true);
    reconexionMQTT();
   }
 
 }

//**** CONFIGURACION WIFI  *******
void wifi_conf() {
  int cuenta = 0;

  String getssid = server.arg("ssid"); //Recibimos los valores que envia por GET el formulario web
  String getpass = server.arg("pass");
  String getTopic1 = server.arg("topic1"); //Recibimos los valores que envia por GET el formulario web
  String getTopic2 = server.arg("topic2");
  String getServerWan = server.arg("serverwan"); //Recibimos los valores que envia por GET el formulario web
  String getServerLan = server.arg("serverlan");

  getssid = arregla_simbolos(getssid); //Reemplazamos los simbolos que aparecen cun UTF8 por el simbolo correcto
  getpass = arregla_simbolos(getpass);
  getTopic1 = arregla_simbolos(getTopic1); //Reemplazamos los simbolos que aparecen cun UTF8 por el simbolo correcto
  getTopic2 = arregla_simbolos(getTopic2);
  getServerWan = arregla_simbolos(getServerWan); //Reemplazamos los simbolos que aparecen cun UTF8 por el simbolo correcto
  getServerLan = arregla_simbolos(getServerLan);

  ssid_tamano = getssid.length() + 1;  //Calculamos la cantidad de caracteres que tiene el ssid y la clave
  pass_tamano = getpass.length() + 1;
  Topic1_tamano = getTopic1.length() + 1;  //Calculamos la cantidad de caracteres que tiene el ssid y la clave
  Topic2_tamano = getTopic2.length() + 1;
  ServerWan_tamano = getServerWan.length() + 1;  //Calculamos la cantidad de caracteres que tiene el ssid y la clave
  ServerLan_tamano = getServerLan.length() + 1;

  getssid.toCharArray(ssid, ssid_tamano); //Transformamos el string en un char array ya que es lo que nos pide WIFI.begin()
  getpass.toCharArray(pass, pass_tamano);
  getTopic1.toCharArray(Topic1, Topic1_tamano); //Transformamos el string en un char array ya que es lo que nos pide WIFI.begin()
  getTopic2.toCharArray(Topic2, Topic2_tamano);
  getServerWan.toCharArray(ServerWan, ServerWan_tamano); //Transformamos el string en un char array ya que es lo que nos pide WIFI.begin()
  getServerLan.toCharArray(ServerLan, ServerLan_tamano);
  
  Serial.print("ssid: ");
  Serial.println(ssid);     //para depuracion
  Serial.print("pass: ");
  Serial.println(pass);
  Serial.print("Topic1: ");
  Serial.println(Topic1);     //para depuracion
  Serial.print("Topic2: ");
  Serial.println(Topic2);
  Serial.print("Server Wan MQTT: ");
  Serial.println(ServerWan);
  Serial.print("Server Lan MQTT: ");
  Serial.println(ServerLan);
 
  WiFi.begin(ssid, pass); 
  //Intentamos conectar
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(400);
    blink50();
    Serial.print(".");
    cuenta++;
    if (cuenta > 20) {
      graba(dir_conf, "noconfigurado");
      EEPROM.write(dir_modo,1);
      EEPROM.commit();;
      server.send(200, "text/html", String("<h2>No se pudo realizar la conexion<br>no se guardaron los datos.</h2>"));
      return;
    }
  }
  
  Serial.print("Cuenta: ");
  Serial.println(cuenta);
  Serial.print(WiFi.localIP());
  graba(dir_conf, "configurado");
  EEPROM.write(dir_modo,0);
  EEPROM.commit();
  graba(dir_ssid, getssid);
  graba(dir_pass, getpass);
  graba(dir_topic1, getTopic1);
  graba(dir_topic2, getTopic2);
  graba(dir_serverwan, getServerWan);
  graba(dir_serverlan, getServerLan);
  server.send(200, "text/html", String("<h2>Conexion exitosa a: "+ getssid + "<br> Pass: '" + getpass + "' .<br>" 
  + "<br> Topic1: " + getTopic1 + ".<br>" 
  + "<br> Topic2: " + getTopic2 + ".<br>" 
  + "<br> Server Wan MQTT: " + getServerWan + ".<br>" 
  + "<br> Server Lan MQTT: " + getServerLan + ".<br>" 
  + "<br>El equipo se reiniciara Conectandose a la red configurada."));
  delay(50);
  ESP.reset();
}

void ServicioBoton(){
       if ( millis() > T0  + 250)
          {   contador++ ;
              T0 = millis();
            
             }
    }

void ServicioBoton2(){ if ( millis() > T02  + 250){ 
               contador2++ ;
              T02 = millis();}
    }

void scanWIFIS(){
  Serial.println("scan start");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      delay(10);
     
    }
  }

}

String macToStr(const uint8_t* mac){

  String result;

  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);

    if (i < 5){
      result += ':';
    }
  }

  return result;
}

void blink50(){
    digitalWrite(LED,true);
    delay(30);
    digitalWrite(LED,false);
    delay(30);
    digitalWrite(LED,true);
    delay(30);
    digitalWrite(LED,false);
    delay(30);
   
  }

 void blinkLento(){
    
    BotonSW();
    digitalWrite(LED,false);
    delay(250);
    BotonSW();
    delay(250);
    BotonSW();   
    digitalWrite(LED,true);
    delay(250);
    BotonSW();
    delay(250);
    BotonSW();
    digitalWrite(LED,false);
    delay(250);
    BotonSW();
    delay(250);
    BotonSW();
    digitalWrite(LED,true);
    delay(250);
    BotonSW();
    delay(250);  
  }

//*******  G R A B A R  EN LA  E E P R O M  ***********
void graba(int addr, String a) {
  int tamano = (a.length() + 1);
  Serial.print(tamano);
  char inchar[30];    //'30' TamaÃ±o maximo del string
  a.toCharArray(inchar, tamano);
  EEPROM.write(addr, tamano);
  for (int i = 0; i < tamano; i++) {
    addr++;
    EEPROM.write(addr, inchar[i]);
  }
  EEPROM.commit();
}

//*******  L E E R   EN LA  E E P R O M    **************
String lee(int addr) {
  String nuevoString;
  int valor;
  int tamano = EEPROM.read(addr);
  for (int i = 0; i < tamano; i++) {
    addr++;
    valor = EEPROM.read(addr);
    nuevoString += (char)valor;
  }
  return nuevoString;
}

void ReadDataEprom(){
  
  
  ssid_leido = lee(dir_ssid);      //leemos ssid y password
  pass_leido = lee(dir_pass);
  Topic1_leido=lee(dir_topic1);
  Topic2_leido=lee(dir_topic2);
  ServerWan_leido=lee(dir_serverwan);
  ServerLan_leido=lee(dir_serverlan);
  
  ServerWan_leido = arregla_simbolos(ServerWan_leido); //Reemplazamos los simbolos que aparecen cun UTF8 por el simbolo correcto
  ServerLan_leido = arregla_simbolos(ServerLan_leido);
  ssid_leido = arregla_simbolos(ssid_leido); //Reemplazamos los simbolos que aparecen cun UTF8 por el simbolo correcto
  pass_leido = arregla_simbolos(pass_leido);
  Topic1_leido = arregla_simbolos(Topic1_leido); //Reemplazamos los simbolos que aparecen cun UTF8 por el simbolo correcto
  Topic2_leido = arregla_simbolos(Topic2_leido);
  
  ServerWan_tamano = ServerWan_leido.length() + 1;  //Calculamos la cantidad de caracteres que tiene el ssid y la clave
  ServerLan_tamano = ServerLan_leido.length() + 1;
  ssid_tamano = ssid_leido.length() + 1;  //Calculamos la cantidad de caracteres que tiene el ssid y la clave
  pass_tamano = pass_leido.length() + 1;
  Topic1_tamano = Topic1_leido.length() + 1;  //Calculamos la cantidad de caracteres que tiene el ssid y la clave
  Topic2_tamano = Topic2_leido.length() + 1;

  ServerWan_leido.toCharArray(ServerWan, ServerWan_tamano); //Transformamos el string en un char array ya que es lo que nos pide WIFI.begin()
  ServerLan_leido.toCharArray(ServerLan, ServerLan_tamano);
  ssid_leido.toCharArray(ssid, ssid_tamano); //Transf. el String en un char array ya que es lo que nos pide WiFi.begin()
  pass_leido.toCharArray(pass, pass_tamano);
  Topic1_leido.toCharArray(Topic1, Topic1_tamano); //Transf. el String en un char array ya que es lo que nos pide WiFi.begin()
  Topic2_leido.toCharArray(Topic2, Topic2_tamano);

   
  Serial.print("ssid: ");
  Serial.println(ssid);     //para depuracion
  Serial.print("pass: ");
  Serial.println(pass);
  Serial.print("Topic1: ");
  Serial.println(Topic1);     //para depuracion
  Serial.print("Topic2: ");
  Serial.println(Topic2);
  Serial.print("Server Wan MQTT: ");
  Serial.println(ServerWan);
  Serial.print("Server Lan MQTT: ");
  Serial.println(ServerLan);
 
  
  
  }

/****************** FIN DECLARACION DE FUNCIONES  ****************************/
void BotonSW(){
  
   if (n2 != contador2){
              n2 = contador2 ;
            
              digitalWrite(relay,!digitalRead(relay));
              if(digitalRead(relay)){//client.publish("prueba/light1/confirm", "Light1 On");
                                        Serial.println("Relay ON!!");
                                      }
              else{//client.publish("prueba/light1/confirm", "Light1 Off");
                     Serial.println("Relay OFF!!");}
             }
  
  }





