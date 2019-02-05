#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <LedControl.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <PubSubClient.h>

//flag para indicar se foi salva uma nova configuração de rede
bool shouldSaveConfig = false;

String ID = "Placar1/";
const char* mqttServer = "m15.cloudmqtt.com";
const int mqttPort = 17812;
const char* mqttUser = "ruzejjkv";
const char* mqttPassword = "e0FRh1Ve8RhG";
String To1 = String(ID) + "Temperatura";
const char* Topico1 = To1.c_str(); //Temperatura
String To2 = String(ID) + "Score_A";
const char* Topico2 = To2.c_str(); //Score_A
String To3 = String(ID) + "Set1_A";
const char* Topico3 = To3.c_str(); //Set1_A
String To4 = String(ID) + "Set2_A";
const char* Topico4 = To4.c_str(); //Set2_A
String To5 = String(ID) + "Set3_A";
const char* Topico5 = To5.c_str(); //Set3_A
String To6 = String(ID) + "Score_B";
const char* Topico6 = To6.c_str(); //Score_B
String To7 = String(ID) + "Set1_B";
const char* Topico7 = To7.c_str(); //Set1_B
String To8 = String(ID) + "Set2_B";
const char* Topico8 = To8.c_str(); //Set2_B
String To9 = String(ID) + "Set3_B";
const char* Topico9 = To9.c_str(); //Set3_B
String To10 = String(ID) + "Horas";
const char* Topico10 = To10.c_str(); //Horas
String To11 = String(ID) + "Minutos";
const char* Topico11 = To11.c_str(); //Minutos
String To12 = String(ID) + "Segundos";
const char* Topico12 = To12.c_str(); //Segundos
String To13 = String(ID) + "Inicio";
const char* Topico13 = To13.c_str(); //Inicio
String To14 = String(ID) + "Zera";
const char* Topico14 = To14.c_str(); //Zera
String To15 = String(ID) + "Sacador";
const char* Topico15 = To15.c_str(); //Sacador
String To16 = String(ID) + "Inte";
const char* Topico16 = To16.c_str(); //Inte
String To17 = String(ID) + "Crono";
const char* Topico17 = To17.c_str(); //Crono

WiFiClient espClient;
PubSubClient client(espClient);

#define DEBUG
#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1 = { 0x28, 0x4C, 0xE3, 0x70, 0x9, 0x0, 0x0, 0xA4 };
LedControl lc = LedControl(23, 18, 15, 1);
WiFiManager wifiManager;

//pino do botão
const int PIN_AP = 2;

//Variáveis que indicam o núcleo
static uint8_t Nucleo_Zero = 0;
static uint8_t Nucleo_Um  = 1;

//Variáveis compartilhadas pelos núcleos
int Score_A = 0, Set1_A = 0, Set2_A = 0, Set3_A = 0; //Jogador A
int Score_B = 0, Set1_B = 0, Set2_B = 0, Set3_B = 0; //Jogador B
int Horas = 0, Minutos = 0, Segundos = 0, Inicio = 0, Zera = 0; // Contagem de tempo
int Sacador = 0;
int Inte = 15; // Intensidade dos displays 0~15

//Variáveis locais
int i = 0, z = 1, esq = 0, cr = 0, Inicr = 0, infoA = 0;
float passaTemp;
float by = 0;

String RAP = String(ID) + "AP";
const char* RedeAp = RAP.c_str();

void setAPCallback(void (*func)(WiFiManager*));
void setSaveConfigCallback(void (*func)(void));

void setup() {
  Serial.begin(9600);
  pinMode(PIN_AP, INPUT);
  WiFiManager wifiManager;
  //  wifiManager.resetSettings(); //caso tiver salvo alguma rede para conectar automaticamente, ela é apagada.
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  lc.shutdown(0, false);
  lc.clearDisplay(0);
  lc.shutdown(1, false);
  lc.clearDisplay(1);
  lc.shutdown(2, false);
  lc.clearDisplay(2);
  sensors.begin();
  pinMode(21, OUTPUT);
  pinMode(22, OUTPUT);
  pinMode(5, OUTPUT);
  client.subscribe(Topico1);
  client.subscribe(Topico2);
  client.subscribe(Topico3);
  client.subscribe(Topico4);
  client.subscribe(Topico5);
  client.subscribe(Topico6);
  client.subscribe(Topico7);
  client.subscribe(Topico8);
  client.subscribe(Topico9);
  client.subscribe(Topico10);
  client.subscribe(Topico11);
  client.subscribe(Topico12);
  client.subscribe(Topico13);
  client.subscribe(Topico14);
  client.subscribe(Topico15);
  client.subscribe(Topico16);
  client.subscribe(Topico17);
  client.setCallback(callback);
  xTaskCreatePinnedToCore(Tempo_Jogo, "Tempo", 1000, NULL, 3, NULL, Nucleo_Zero);
  xTaskCreatePinnedToCore(Jogador_A, "Jogador_A", 1000, NULL, 2, NULL, Nucleo_Zero);
  xTaskCreatePinnedToCore(Jogador_B, "Jogador_B", 1000, NULL, 2, NULL, Nucleo_Zero);
  xTaskCreatePinnedToCore(Saque, "Saque", 1000, NULL, 2, NULL, Nucleo_Zero);
  xTaskCreatePinnedToCore(Temperatura, "Temperatura", 1000, NULL, 1, NULL, Nucleo_Zero);
  xTaskCreatePinnedToCore(SetIntensidade, "SetIntensidade", 1000, NULL, 1, NULL, Nucleo_Zero);
  delay(500);
}

void loop() {
  WiFiManager wifiManager;
  if ( digitalRead(PIN_AP) == HIGH ) {
    Serial.println("resetar");
    digitalWrite(5, HIGH);
    if (!wifiManager.startConfigPortal(RedeAp) ) {
      Serial.println("Falha ao conectar");
      delay(2000);
      ESP.restart();
      delay(1000);
    }
    Serial.println(wifiManager.getSSID());
    Serial.println(wifiManager.getPassword());
    Serial.println("Conectou!!!");
    digitalWrite(5, LOW);
    ESP.restart();
  }
  MQTT();
  if (esq == 100) {
    by = passaTemp;
    char b[6];
    char convertido[16];
    sprintf(b, "%.1f", by);
    client.publish(Topico1, b);
    esq = 0;
  }
  esq ++;
  if (cr == 10) {
    if (Segundos != Inicr) {
      Inicr = Segundos;
      char c[10];
      sprintf(c, "%d:%d:%d", Horas, Minutos, Inicr);
      client.publish(Topico17, c);
    }
    cr = 0;
  }
  cr ++;
  client.loop();
}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entrou no modo de configuração");
  digitalWrite(5, HIGH);
  Serial.println(WiFi.softAPIP()); //imprime o IP do AP
  Serial.println(myWiFiManager->getConfigPortalSSID()); //imprime o SSID criado da rede

}

void saveConfigCallback () {
  //  Serial.println("Should save config");
  Serial.println("Configuração salva");
  digitalWrite(5, LOW);
  Serial.println(WiFi.softAPIP()); //imprime o IP do AP
}

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  if (strcmp(topic, Topico2) == 0) {
    String Sco_A = String((char*)payload);
    Score_A = atoi(Sco_A.c_str());
  }
  if (strcmp(topic, Topico3) == 0) {
    String Set1A = String((char*)payload);
    Set1_A = atoi(Set1A.c_str());
  }
  if (strcmp(topic, Topico4) == 0) {
    String Set2A = String((char*)payload);
    Set2_A = atoi(Set2A.c_str());
  }
  if (strcmp(topic, Topico5) == 0) {
    String Set3A = String((char*)payload);
    Set3_A = atoi(Set3A.c_str());
  }
  if (strcmp(topic, Topico6) == 0) {
    String Sco_B = String((char*)payload);
    Score_B = atoi(Sco_B.c_str());
  }
  if (strcmp(topic, Topico7) == 0) {
    String Set1B = String((char*)payload);
    Set1_B = atoi(Set1B.c_str());
  }
  if (strcmp(topic, Topico8) == 0) {
    String Set2B = String((char*)payload);
    Set2_B = atoi(Set2B.c_str());
  }
  if (strcmp(topic, Topico9) == 0) {
    String Set3B = String((char*)payload);
    Set3_B = atoi(Set3B.c_str());
  }
  if (strcmp(topic, Topico10) == 0) {
    String Hor = String((char*)payload);
    Horas = atoi(Hor.c_str());
  }
  if (strcmp(topic, Topico11) == 0) {
    String Minuts = String((char*)payload);
    Minutos = atoi(Minuts.c_str());
  }
  if (strcmp(topic, Topico12) == 0) {
    String Segunds = String((char*)payload);
    Segundos = atoi(Segunds.c_str());
  }
  if (strcmp(topic, Topico13) == 0) {
    String Inii = String((char*)payload);
    Inicio = atoi(Inii.c_str());
  }
  if (strcmp(topic, Topico14) == 0) {
    String Zer = String((char*)payload);
    Zera = atoi(Zer.c_str());
  }
  if (strcmp(topic, Topico15) == 0) {
    String Sac = String((char*)payload);
    Sacador = atoi(Sac.c_str());
  }
  if (strcmp(topic, Topico16) == 0) {
    String Intensidade = String((char*)payload);
    Inte = atoi(Intensidade.c_str());
  }
}

void MQTT(void) {
  if (WiFi.status() != WL_CONNECTED || infoA == 0) {
    digitalWrite(22, LOW);
    delay(1000);
    if (infoA == 0) {
      digitalWrite(5, HIGH);
      delay(100);
      wifiManager.autoConnect(RedeAp);
      digitalWrite(5, LOW);
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Reconectando a rede");
      digitalWrite(5, HIGH);
      WiFi.begin((wifiManager.getSSID()).c_str(), (wifiManager.getPassword()).c_str());
    }
    client.setServer(mqttServer, mqttPort);
    while (z) {
      digitalWrite(21, LOW);
      delay(200);
      digitalWrite(21, HIGH);
      delay(200);
      if (WiFi.status() == WL_CONNECTED) {
        z = 0;
        Serial.println("Conectado");
        digitalWrite(5, LOW);
      }
    }
    while (!client.connected()) {
      if (client.connect("ESP32Client", mqttUser, mqttPassword )) {
        client.subscribe(Topico1, 1);
        client.subscribe(Topico2, 1);
        client.subscribe(Topico3, 1);
        client.subscribe(Topico4, 1);
        client.subscribe(Topico5, 1);
        client.subscribe(Topico6, 1);
        client.subscribe(Topico7, 1);
        client.subscribe(Topico8, 1);
        client.subscribe(Topico9, 1);
        client.subscribe(Topico10, 1);
        client.subscribe(Topico11, 1);
        client.subscribe(Topico12, 1);
        client.subscribe(Topico13, 1);
        client.subscribe(Topico14, 1);
        client.subscribe(Topico15, 1);
        client.subscribe(Topico16, 1);
        client.subscribe(Topico17, 1);
        digitalWrite(22, HIGH);
      }
    }
    z = 1;
    infoA = 1;
  }
  delay(100);
}

void Jogador_A(void* pvParameters) {
  int display_A = 1;
  while (1) {
    int Score_inteiroA = Score_A / 10;
    int Score_sobraA = Score_A % 10;
    lc.setDigit(display_A, 0, Score_sobraA, false);
    lc.setDigit(display_A, 1, Score_inteiroA, false);
    lc.setDigit(display_A, 2, Set3_A, false);
    lc.setDigit(display_A, 3, Set2_A, false);
    lc.setDigit(display_A, 4, Set1_A, false);
    delay(100);
  }
}
void Jogador_B(void* pvParameters) {
  int display_B = 2;
  while (1) {
    int Score_inteiroB = Score_B / 10;
    int Score_sobraB = Score_B % 10;
    lc.setDigit(display_B, 0, Score_sobraB, false);
    lc.setDigit(display_B, 1, Score_inteiroB, false);
    lc.setDigit(display_B, 2, Set3_B, false);
    lc.setDigit(display_B, 3, Set2_B, false);
    lc.setDigit(display_B, 4, Set1_B, false);
    delay(100);
  }
}
void Saque(void* pvParameters) {
  int display_AS = 1;
  int display_BS = 2;
  while (1) {
    if (Sacador == 1) {
      lc.setRow(display_AS, 5, B00000001);
      lc.setRow(display_BS, 5, B00000000);
    }
    else if (Sacador == 2) {
      lc.setRow(display_AS, 5, B00000000);
      lc.setRow(display_BS, 5, B00000001);
    }
    else {
      lc.setRow(display_AS, 5, B00000000);
      lc.setRow(display_BS, 5, B00000000);
    }
    delay(100);
  }
}

void Temperatura(void* pvParameters) {
  int x = 45, loa = 0;
  int display_Temperatura = 0;
  while (1) {
    sensors.requestTemperatures();
    if (loa == 0) {
      lc.setRow(display_Temperatura, 1, B00010000);
      lc.setRow(display_Temperatura, 0, B00000000);
      delay(x);
      lc.setRow(display_Temperatura, 1, B00011000);
      lc.setRow(display_Temperatura, 0, B00000000);
      delay(x);
      lc.setRow(display_Temperatura, 1, B00011100);
      lc.setRow(display_Temperatura, 0, B00000000);
      delay(x);
      lc.setRow(display_Temperatura, 1, B00011110);
      lc.setRow(display_Temperatura, 0, B00000000);
      delay(x);
      lc.setRow(display_Temperatura, 1, B01011110);
      lc.setRow(display_Temperatura, 0, B00000000);
      delay(x);
      lc.setRow(display_Temperatura, 1, B01111110);
      lc.setRow(display_Temperatura, 0, B00000000);
      delay(x);
      lc.setRow(display_Temperatura, 1, B01111110);
      lc.setRow(display_Temperatura, 0, B00000100);
      delay(x);
      lc.setRow(display_Temperatura, 1, B01111110);
      lc.setRow(display_Temperatura, 0, B00001100);
      delay(x);
      lc.setRow(display_Temperatura, 1, B01111110);
      lc.setRow(display_Temperatura, 0, B00011100);
      delay(x);
      lc.setRow(display_Temperatura, 1, B01111110);
      lc.setRow(display_Temperatura, 0, B00111100);
      delay(x);
      lc.setRow(display_Temperatura, 1, B01111110);
      lc.setRow(display_Temperatura, 0, B01111100);
      delay(x);
      lc.setRow(display_Temperatura, 1, B01111110);
      lc.setRow(display_Temperatura, 0, B01111110);
      delay(x);
      lc.setRow(display_Temperatura, 1, B01101110);
      lc.setRow(display_Temperatura, 0, B01111110);
      delay(x);
      lc.setRow(display_Temperatura, 1, B01100110);
      lc.setRow(display_Temperatura, 0, B01111110);
      delay(x);
      lc.setRow(display_Temperatura, 1, B01100010);
      lc.setRow(display_Temperatura, 0, B01111110);
      delay(x);
      lc.setRow(display_Temperatura, 1, B01100000);
      lc.setRow(display_Temperatura, 0, B01111110);
      delay(x);
      lc.setRow(display_Temperatura, 1, B00100000);
      lc.setRow(display_Temperatura, 0, B01111110);
      delay(x);
      lc.setRow(display_Temperatura, 1, B00000000);
      lc.setRow(display_Temperatura, 0, B01111110);
      delay(x);
      lc.setRow(display_Temperatura, 1, B00000000);
      lc.setRow(display_Temperatura, 0, B01111010);
      delay(x);
      lc.setRow(display_Temperatura, 1, B00000000);
      lc.setRow(display_Temperatura, 0, B01110010);
      delay(x);
      lc.setRow(display_Temperatura, 1, B00000000);
      lc.setRow(display_Temperatura, 0, B01100010);
      delay(x);
      lc.setRow(display_Temperatura, 1, B00000000);
      lc.setRow(display_Temperatura, 0, B01000010);
      delay(x);
      lc.setRow(display_Temperatura, 1, B00000000);
      lc.setRow(display_Temperatura, 0, B00000010);
      delay(x);
      lc.setRow(display_Temperatura, 1, B00000000);
      lc.setRow(display_Temperatura, 0, B00000000);
      delay(x);
      loa = 1;
    }
    if (sensors.getTempC(sensor1) >= 10.0 && sensors.getTempC(sensor1) < 100) {
      int Cel = (int)sensors.getTempC(sensor1) / 10;
      int Cel2 = (int)sensors.getTempC(sensor1) % 10;
      lc.setDigit(display_Temperatura, 1, Cel, false);
      lc.setDigit(display_Temperatura, 0, Cel2, false);
      passaTemp = (sensors.getTempC(sensor1));
      delay(100);
    }
    else if (sensors.getTempC(sensor1) < 10.0 && sensors.getTempC(sensor1) > -1) {
      int u = (int)(sensors.getTempC(sensor1) * 10) / 10;
      int o = (int)(sensors.getTempC(sensor1) * 10) % 10;
      lc.setDigit(display_Temperatura, 1, u, true);
      lc.setDigit(display_Temperatura, 0, o, false);
      delay(100);
    }
    else {
      int neg = (int)(sensors.getTempC(sensor1) * (-1) * 10) / 10;
      lc.setRow(display_Temperatura, 1, B00000001);
      lc.setDigit(display_Temperatura, 0, neg, false);
      delay(100);
    }
  }
}

void Tempo_Jogo(void* pvParameters) {
  int display_Tempo = 0;
  while (1) {
    if (Zera == 0) {
      if (Inicio == 0) {
        int Minutos_int = Minutos / 10;
        int Minutos_sobra = Minutos % 10;
        int Horas_int = Horas / 10;
        int Horas_sobra = Horas % 10;
        lc.setDigit(display_Tempo, 2, Minutos_sobra, false);
        lc.setDigit(display_Tempo, 3, Minutos_int, false);
        lc.setDigit(display_Tempo, 5, Horas_sobra, false);
        lc.setDigit(display_Tempo, 6, Horas_int, false);
        lc.setRow(display_Tempo, 4, B01001000);
        delay(100);
      }
      else if (Inicio == 1) {
        if (Segundos == 60) {
          Minutos ++;
          Segundos = 0;
        }
        else if (Minutos == 60) {
          Horas ++;
          Minutos = 0;
        }
        else if (Horas == 24) {
          Horas = 0;
        }
        int Minutos_int = Minutos / 10;
        int Minutos_sobra = Minutos % 10;
        int Horas_int = Horas / 10;
        int Horas_sobra = Horas % 10;
        lc.setDigit(display_Tempo, 2, Minutos_sobra, false);
        lc.setDigit(display_Tempo, 3, Minutos_int, false);
        lc.setDigit(display_Tempo, 5, Horas_sobra, false);
        lc.setDigit(display_Tempo, 6, Horas_int, false);
        Segundos ++;
        lc.setRow(display_Tempo, 4, B01001000);
        delay(500);
        lc.setRow(display_Tempo, 4, B00000000);
        delay(500);
      }
    }
    else if (Zera == 1) {
      Horas = 0;
      Minutos = 0;
      Segundos = 0;
      Zera = 0;
      delay(100);
    }
  }
}

void SetIntensidade(void* pvParameters) {
  while (1) {
    lc.setIntensity(0, Inte);
    lc.setIntensity(1, Inte);
    lc.setIntensity(2, Inte);
    delay(100);
  }
}
