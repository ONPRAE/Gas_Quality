#include <PubSubClient.h>
#include <WiFi.h>
#include <DHT.h> //add
#include <Stepper.h>
// #include <TridentTD_LineNotify.h>
#include <HTTPClient.h> //Add

#define ON 1
#define OFF 0
#define MQ2_ANA 4
#define MQ2_DIG 2
// #define LINE_TOKEN  "i5Jl53UeTS0kDqzELnYcHDC04bFNHrDo2NPZu4MlHVm"// Add Tokeh Line ที่ต้องการให้แจ้งเตือน
#define DHTTYPE DHT22

const char* token = "i5Jl53UeTS0kDqzELnYcHDC04bFNHrDo2NPZu4MlHVm"; //Tokeh Line ที่ต้องการให้แจ้งเตือน


// bool notified = false;

// initial WIFI 
const char* ssid = "Wokwi-GUEST";
const char* password =  "";
WiFiClient espClient;
// initial MQTT client
const char* mqttServer = "mqtt.netpie.io";
const int mqttPort = 1883;
const char* clientID = "1a29f63e-65e1-4eec-9453-d4843059438c";
const char* mqttUser = "Vc4Bso1ZUXY2twvT9Dj41Gkh7Z9R1NDo";
const char* mqttPassword = "5xbcBUzDEMMXhxNX6ZgKhfZxg7cLpZz6";
const char* topic_pub = "@msg/lab_ict_kps/testproject/data";
const char* topic_sub = "@msg/lab_ict_kps/command";
// send buffer
String publishMessage;

PubSubClient client(espClient);

// LED initial pin
const int RED_LED_PIN = 32;
const int BLUE_LED_PIN = 25;

const int DHT_PIN = 13;
DHT dht(DHT_PIN, DHTTYPE);

const int mortor = 200; //รอบมอเตอร์
Stepper myStepper(mortor, 16, 17, 18, 19);

int redLED_status = OFF, blueLED_status = OFF, mortor_status = OFF;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  // Loop until we're reconnected
  char mqttinfo[80];
  snprintf (mqttinfo, 75, "Attempting MQTT connection at %s:%d (%s/%s)...", mqttServer, mqttPort, mqttUser, mqttPassword);
  while (!client.connected()) {
    Serial.println(mqttinfo);
    String clientId = clientID;
    if (client.connect(clientId.c_str(), mqttUser, mqttPassword)) {
      Serial.println("...Connected");
      client.subscribe(topic_sub);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void messageReceivedCallback(char* topic, byte* payload, unsigned int length) { 
  char payloadMsg[80];
  
  Serial.print("Message arrived in topic: ");
  Serial.println(topic); 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    payloadMsg[i] = (char) payload[i];
  }
  payloadMsg[length] = '\0';   // put end of string to buffer
  Serial.println();
  Serial.println("-----------------------");
  processMessage(payloadMsg);
}

void processMessage(String recvCommand) {
   if (recvCommand == "RED_ON") {
    digitalWrite(RED_LED_PIN, HIGH);
    redLED_status = ON;
  } else  if (recvCommand == "RED_OFF") {
    digitalWrite(RED_LED_PIN, LOW);
     redLED_status = OFF;
  } else  if (recvCommand == "BLUE_ON") {
    digitalWrite(BLUE_LED_PIN, HIGH);
    blueLED_status = ON;
  } else  if (recvCommand == "BLUE_OFF") {
    digitalWrite(BLUE_LED_PIN, LOW);
    blueLED_status = OFF;
  } else  if (recvCommand == "MORTOR_ON") {
    digitalWrite(mortor, HIGH);
    mortor_status = ON;
  } else  if (recvCommand == "MORTOR_OFF") {
    digitalWrite(mortor, LOW);
    mortor_status = OFF;
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(messageReceivedCallback); 
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);

  myStepper.setSpeed(60);
  pinMode(MQ2_ANA, INPUT);//input analog ฝุ่น
  pinMode(MQ2_DIG, INPUT);//input digital ฝุ่น
  pinMode(mortor, OUTPUT);
  redLED_status = OFF;
  blueLED_status = OFF;
  mortor_status = OFF;
  dht.begin();

  // LINE.setToken(LINE_TOKEN);
  // LINE.notify("Air Quality Notify");
}



void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();
  HTTPClient http; //Add

  http.begin("https://notify-api.line.me/api/notify"); //add
    http.addHeader("Authorization", "Bearer " + String(token)); //add
    http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //add

  dht.read(); // Read temperature and humidity from DHT sensor
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(h) || isnan(t)) {
  Serial.println("ERR: fail to read data from DHT sensor...!");
  delay(5000);
  return;
  }

  if(t  >= 27  )
  {
    myStepper.step(mortor); 
  }
  else{
    
  }

  if(digitalRead(MQ2_DIG) == 1){
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(BLUE_LED_PIN, LOW);

    http.POST("message=Air Quality Is Unhealthy please put on ypur mask!!");
      Serial.println("ส่งข้อความ : Air Quality Is healthy Enjoy your Activity:)!");
      Serial.println("==================================");

  }
  else{
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(BLUE_LED_PIN, HIGH);
   
    // http.POST("message=Air Quality Notyfy");
    //   Serial.println("ส่งข้อความ : Air Quality Is healthy Enjoy your Activity:)!");
    //   Serial.println("==================================");
  }  
  publishMessage = "{\"data\": {\"Temperature\": " + String(t) + ", \"Humidity\": "  + String(h) + " ,\"MQ\": " + String(digitalRead(MQ2_DIG)) + "}}";
  Serial.println(publishMessage);
  client.publish(topic_pub, publishMessage.c_str());

   int httpCode = http.POST("READY");
    if (httpCode > 0) {
      String response = http.getString();
      //Serial.print("HTTP Response code: ");
      //Serial.println(httpCode);
      //Serial.print("Server response: ");
     // Serial.println(response);
    } else {
      Serial.println(".....");
    }


http.end();

  Serial.println(t);
  Serial.println(h);
  Serial.print("Analog: ");
  Serial.println(MQ2_ANA);
  Serial.print("Digital: ");
  Serial.println(digitalRead(MQ2_DIG));
  delay(2000); 
}