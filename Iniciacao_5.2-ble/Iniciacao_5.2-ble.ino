#include <LedControl.h>
#include <OneWire.h>
#include <DallasTemperature.h>
//#include <WiFi.h>
//#include <PubSubClient.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
float txValue = 0;

const char* ssid = "Eberton 2";
const char* password =  "12345678";
const char* mqttServer = "m15.cloudmqtt.com";
const int mqttPort = 17812;
const char* mqttUser = "ruzejjkv";
const char* mqttPassword = "e0FRh1Ve8RhG";
const char* Topico1 = "Temperatura";
const char* Topico2 = "Score_A";
const char* Topico3 = "Set1_A";
const char* Topico4 = "Set2_A";
const char* Topico5 = "Set3_A";
const char* Topico6 = "Score_B";
const char* Topico7 = "Set1_B";
const char* Topico8 = "Set2_B";
const char* Topico9 = "Set3_B";
const char* Topico10 = "Horas";
const char* Topico11 = "Minutos";
const char* Topico12 = "Segundos";
const char* Topico13 = "Inicio";
const char* Topico14 = "Zera";
const char* Topico15 = "Sacador";
const char* Topico16 = "Inte";

//WiFiClient espClient;
//PubSubClient client(espClient);

#define DEBUG
#define ONE_WIRE_BUS 4
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1 = { 0x28, 0x4C, 0xE3, 0x70, 0x9, 0x0, 0x0, 0xA4 };
LedControl lc = LedControl(23, 18, 15, 1);

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      if (rxValue.length() > 0) {
        for (int i = 0; i < rxValue.length(); i++) {
        }
        // Do stuff based on the command received from the app
        if (rxValue.find("A") != -1) {
          digitalWrite(5, HIGH);
        }
        else if (rxValue.find("B") != -1) {
          digitalWrite(5, LOW);
        }
      }
    }
};


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
int i = 0, z = 1, caso = 0;
float passaTemp;
float by = 0;

void setup() {
  lc.shutdown(0, false);
  lc.clearDisplay(0);
  lc.shutdown(1, false);
  lc.clearDisplay(1);
  lc.shutdown(2, false);
  lc.clearDisplay(2);
  sensors.begin();
  //Serial.begin(115200);
  pinMode(5, OUTPUT);
  pinMode(21, OUTPUT);
  pinMode(22, OUTPUT);
  /*client.subscribe(Topico1);
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
  client.setCallback(callback);*/
  xTaskCreatePinnedToCore(Tempo_Jogo, "Tempo", 1000, NULL, 3, NULL, Nucleo_Zero);
  xTaskCreatePinnedToCore(Jogador_A, "Jogador_A", 1000, NULL, 2, NULL, Nucleo_Zero);
  xTaskCreatePinnedToCore(Jogador_B, "Jogador_B", 1000, NULL, 2, NULL, Nucleo_Zero);
  xTaskCreatePinnedToCore(Saque, "Saque", 1000, NULL, 2, NULL, Nucleo_Zero);
  xTaskCreatePinnedToCore(Temperatura, "Temperatura", 1000, NULL, 1, NULL, Nucleo_Zero);
  delay(500);
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
/*void MQTT(void) {
  deviceConnected = false;
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(22, LOW);
    delay(1000);
    WiFi.begin(ssid, password);
    client.setServer(mqttServer, mqttPort);
    while (z) {
      digitalWrite(21, LOW);
      delay(200);
      digitalWrite(21, HIGH);
      delay(200);
      if (WiFi.status() == WL_CONNECTED) {
        z = 0;
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
        digitalWrite(22, HIGH);
      }
    }
    z = 1;
  }
  if (passaTemp != by) {
    by = passaTemp;
    i++;
    if (i == 5) {
      char b[6];
      char convertido[16];
      sprintf(b, "%.1f", by);
      client.publish(Topico1, b);
      i = 0;
    }
  }
  delay(100);
}
*/
/*void callback(char* topic, byte* payload, unsigned int length) {
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
*/
void BLE() {
  if (caso == 0) {
    // Create the BLE Device
    BLEDevice::init("ESP32 UART Test"); // Give it a name
    // Create the BLE Server
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);
    // Create a BLE Characteristic
    pCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
    pCharacteristic->addDescriptor(new BLE2902());
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                           CHARACTERISTIC_UUID_RX,
                                           BLECharacteristic::PROPERTY_WRITE
                                         );
    pCharacteristic->setCallbacks(new MyCallbacks());
    // Start the service
    pService->start();
    // Start advertising
    pServer->getAdvertising()->start();
    caso = 1;
  }
  if (deviceConnected) {
    txValue =  passaTemp;
    char txString[8]; // make sure this is big enuffz
    dtostrf(txValue, 1, 2, txString); // float_val, min_width, digits_after_decimal, char_buffer
    pCharacteristic->setValue(txString);
    pCharacteristic->notify(); // Send the value to the app!
  }
  digitalWrite(5, HIGH);
  delay(500);
  digitalWrite(5, LOW);
  delay(500);
}

void loop() {
  lc.setIntensity(0, Inte);
  lc.setIntensity(1, Inte);
  lc.setIntensity(2, Inte);
  BLE();
  //MQTT();
  //client.loop();
}
