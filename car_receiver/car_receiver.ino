#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include <math.h>

const int motor_1_speed_pin = D1; // GPIO5
const int motor_2_speed_pin = D2; // GPIO4
const int motor_1_dir_pin1 = D3;  // GPIO0
const int motor_1_dir_pin2 = D4;  // GPIO2
const int motor_2_dir_pin1 = D5;  // GPIO14
const int motor_2_dir_pin2 = D6;  // GPIO12


const int encoder1Pin = D7;
const int encoder2Pin = D8;  // Encoder output to Arduino digital pin 2
const unsigned long interval = 1000;  // Interval to measure RPM (1 second)

// Variables
volatile int pulseCount1 = 0;
volatile int pulseCount2 = 0;
unsigned long previousMillis = 0;
long rpm1 = 0;
long rpm2 = 0;

void ICACHE_RAM_ATTR countPulse1() {
  pulseCount1++;
}

void ICACHE_RAM_ATTR countPulse2() {
  pulseCount2++;
}


// Define PWM frequency
const int PWM_FREQUENCY = 1000; // 1 kHz frequency

uint8_t broadcastAddress[] = { 0xEC, 0x62, 0x60, 0xA1, 0xC9, 0x28  };



typedef struct struct_anglesMessage {
  float tiltAngleX;
  float tiltAngleY;
  int uForward;
  int uSteering;
} struct_anglesMessage;

struct_anglesMessage tiltData;

typedef struct struct_encoderMessage {
  int rpm1;
  int rpm2;
} struct_encoderMessage;

struct_encoderMessage encoderData;

// Variable to store received data
volatile bool dataReceived = false;

// Callback function to be executed when data is received
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&tiltData, incomingData, sizeof(tiltData));
  dataReceived = true;
}

// Callback function when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  // Serial.print("Last Packet Send Status: ");
  // if (sendStatus == 0) {
  //   Serial.println("Delivery success");
  // } else {
  //   Serial.println("Delivery fail");
  // }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(motor_1_speed_pin, OUTPUT);
  pinMode(motor_2_speed_pin, OUTPUT);
  pinMode(motor_1_dir_pin1, OUTPUT);
  pinMode(motor_1_dir_pin2, OUTPUT);
  pinMode(motor_2_dir_pin1, OUTPUT);
  pinMode(motor_2_dir_pin2, OUTPUT);

  pinMode(encoder1Pin, INPUT_PULLUP);  // Use internal pull-up resistor
  pinMode(encoder2Pin, INPUT_PULLUP);  // Use internal pull-up resistor
  attachInterrupt(digitalPinToInterrupt(encoder1Pin), countPulse1, RISING);
  attachInterrupt(digitalPinToInterrupt(encoder2Pin), countPulse2, RISING);

  // Set the PWM frequency
  analogWriteFreq(PWM_FREQUENCY);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register the callback function to receive data
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);

  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
}

void loop() {
  if (dataReceived) {
    dataReceived = false;
    float tiltAngleX = tiltData.tiltAngleX;
    float tiltAngleY = tiltData.tiltAngleY;

    int PWMForward = map(tiltAngleX, -45, 45, -1023, 1023);
    int PWMSteering = map(tiltAngleY, -45, 45, -1023, 1023);

    int motor_1_speed = PWMForward + PWMSteering + tiltData.uForward + tiltData.uSteering;
    int motor_2_speed = PWMForward - PWMSteering + tiltData.uForward - tiltData.uSteering;

    if (motor_1_speed < 0) {
      motor_1_speed = abs(motor_1_speed);
      digitalWrite(motor_1_dir_pin1, LOW);
      digitalWrite(motor_1_dir_pin2, HIGH);
    } else {
      digitalWrite(motor_1_dir_pin1, HIGH);
      digitalWrite(motor_1_dir_pin2, LOW);
    }

    if (motor_2_speed < 0) {
      motor_2_speed = abs(motor_2_speed);
      digitalWrite(motor_2_dir_pin1, LOW);
      digitalWrite(motor_2_dir_pin2, HIGH);
    } else {
      digitalWrite(motor_2_dir_pin1, HIGH);
      digitalWrite(motor_2_dir_pin2, LOW);
    }

    motor_1_speed = min(motor_1_speed, 1023);
    motor_2_speed = min(motor_2_speed, 1023);

    analogWrite(motor_1_speed_pin, motor_1_speed);
    analogWrite(motor_2_speed_pin, motor_2_speed);

    // Serial.print("Tilt Angle X: ");
    // Serial.print(tiltAngleX);
    // Serial.print(" Tilt Angle Y: ");
    // Serial.println(tiltAngleY);

    uint8_t status = esp_now_send(broadcastAddress, (uint8_t *)&encoderData, sizeof(encoderData));
  }
    unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    encoderData.rpm1 = pulseCount1 * (60000 / interval);  // Calculate RPM
    encoderData.rpm2 = pulseCount2 * (60000 / interval);  // Calculate RPM
    Serial.print("RPM1: ");
    Serial.print(encoderData.rpm1);
    Serial.print("  RPM2: ");
    Serial.println(encoderData.rpm2);
    pulseCount1 = 0;  // Reset pulse count for next interval
    pulseCount2 = 0;  // Reset pulse count for next interval
  
    previousMillis = currentMillis;
  }
}
