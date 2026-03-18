#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <MPU6050.h>
#include <math.h>

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

MPU6050 mpu;



// I2C pins
const int SDA_PIN = 4; // GPIO4 (SDA)
const int SCL_PIN = 5; // GPIO5 (SCL)

// Structure to send data
typedef struct struct_anglesMessage {
  float tiltAngleX;
  float tiltAngleY;
  int uForward;
  int uSteering;
} struct_anglesMessage;


volatile bool dataReceived = false;

struct_anglesMessage tiltData;

// Structure of received data
typedef struct struct_encoderMessage {
  int rpm1;
  int rpm2;
} struct_encoderMessage;

struct_encoderMessage encoderData;

unsigned long lastTime = 0;
unsigned long timerDelay = 2000;  // send readings timer

const float ENCODER_1_MAX_RPM = 16000.0f;
const float ENCODER_2_MAX_RPM = 10000.0f;
const float KP_FORWARD = 0.1f;
const float KP_STEERING = 0.1f;

// Replace with your receiver's exact MAC Address (e.g., { 0x08, 0x3A, 0x8D, 0xCF, 0xA9, 0xEE })
uint8_t broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

// Callback function when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Delivery success");
  } else {
    Serial.println("Delivery fail");
  }
}


void OnDataRecv(const uint8_t *mac_addr,const uint8_t *incomingData,int len){
  
  memcpy(&encoderData, incomingData, sizeof(encoderData));
  dataReceived = true;
}

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN); // Initialize I2C with specified SDA and SCL pins
  Serial.begin(115200);

  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed");
    while (1);
  }
  else{
    Serial.println("MPU6050 connection successful");
    
  }

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register the callback function for sending data
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Register the receiver
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

    // Now set up two tasks to run independently.
  xTaskCreatePinnedToCore(
    TaskReceiving
    ,  "Receive & PID"   // A name just for humans
    ,  2048  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskSending
    ,  "MPU & Sending"
    ,  2048  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);

    Serial.println("Transmitter tasks started");

}

void loop() {

}

void TaskReceiving(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  // initialization

  while (1) 
  {
    if(dataReceived){
      dataReceived = false;
      Serial.print("Motor 1:");
      Serial.print(encoderData.rpm1);
      Serial.print("  Motor 2:");
      Serial.println(encoderData.rpm2);

      float act1 = encoderData.rpm1 * 45 / ENCODER_1_MAX_RPM; // empirically observed encoder scale
      float act2 = encoderData.rpm2 * 45 / ENCODER_2_MAX_RPM; // empirically observed encoder scale


      // adjusting signs
      if(tiltData.tiltAngleX + tiltData.tiltAngleY <0){
        act1*=-1;
      }

      if(tiltData.tiltAngleX - tiltData.tiltAngleY < 0){
        act2*=1;
      }



      if(act1>45)
        act1 =45; //limit fluctuations
      else if(act1<-45)
        act1 = -45;
      if(act2>45)
        act2 = 45; //limit fluctuations
      else if (act2<-45)
        act2 = -45;

      float avgDes = tiltData.tiltAngleX;
      float avgAct = (act1 + act2)/2;

      float eForward = avgDes - avgAct;
      float uForward = KP_FORWARD * eForward;

      float diffDes = tiltData.tiltAngleY;
      float diffAct = act1 - act2;

      float eSteering = diffDes - diffAct;
      float uSteering = KP_STEERING * eSteering;

      tiltData.uSteering = map(uSteering, -45, 45, -1023, 1023);
      tiltData.uForward = map(uForward, -45, 45, -1023, 1023);
    }
    vTaskDelay(pdMS_TO_TICKS(100)); 
  }
}

void TaskSending(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  
  while(1){
    int16_t accelX, accelY, accelZ;
    mpu.getAcceleration(&accelX, &accelY, &accelZ);

    float accelerationX = (float)accelX / 16384.0 * 9.81;
    float tiltAngleX = atan2(accelerationX, 9.81) * 180.0 / PI;

    float accelerationY = (float)accelY / 16384.0 * 9.81;
    float tiltAngleY = atan2(accelerationY, 9.81) * 180.0 / PI;

    tiltData.tiltAngleX = tiltAngleX;
    tiltData.tiltAngleY = tiltAngleY;

    esp_now_send(broadcastAddress, (uint8_t *)&tiltData, sizeof(tiltData));

    // Serial.print("Tilt Angle X: ");
    // Serial.print(tiltAngleX);
    // Serial.print(" Tilt Angle Y: ");
    // Serial.println(tiltAngleY);
    
    vTaskDelay(pdMS_TO_TICKS(100)); 

  }
}
