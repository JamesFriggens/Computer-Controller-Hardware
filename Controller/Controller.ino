#include "arduino_secrets.h"

#define ESP8266_BAUD 115200

// Use Serial1 on the Pico for communication with the ESP8266
#define RX_PIN 1
#define TX_PIN 0

#define BUTTON1_PIN 15
#define BUTTON2_PIN 14
#define BUTTON3_PIN 13

// WiFi SSID, PASSWORD, and IP Address respectively
//String SSID = "";
//String PASSWORD = "";
//String HOST = "";
int port = 10001;

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("Serial communication started.");

  // Start communication with the ESP8266
  Serial1.begin(ESP8266_BAUD);
  delay(1000);

  // Initialize ESP8266
  sendCommand("AT", 2000, "OK");
  sendCommand("AT+CWMODE=1", 2000, "OK");
  sendCommand("AT+CWJAP=\"" + String(SSID) + "\",\"" + String(PASSWORD) + "\"", 10000, "OK");

  // Initialize buttons as input with pull-up resistors
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);

  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  if (digitalRead(BUTTON1_PIN) == LOW) {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Button 1 pressed");
    sendMessage("1");
    delay(200); // debounce
    digitalWrite(LED_BUILTIN, LOW);
  }

  if (digitalRead(BUTTON2_PIN) == LOW) {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Button 2 pressed");
    sendMessage("2");
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
  }

  if (digitalRead(BUTTON3_PIN) == LOW) {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Button 3 pressed");
    sendMessage("3");
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
  }

  // Check for incoming data from ESP8266
  if (Serial1.available()) {
    while (Serial1.available()) {
      String inData = Serial1.readString();
      Serial.print(inData);
    }
  }

  // Forward commands from serial monitor to ESP8266
  if (Serial.available()) {
    delay(10);
    String cmd = "";
    while (Serial.available()) {
      char c = Serial.read();
      cmd += c;
    }
    Serial1.println(cmd);
  }
}

bool sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print("Sending command: ");
  Serial.println(command);
  Serial1.println(command);
  long int time = millis();
  while ((time + maxTime) > millis()) {
    while (Serial1.available()) {
      String inData = Serial1.readString();
      Serial.print(inData);
      if (inData.indexOf(readReplay) != -1) {
        Serial.println("Success");
        return true;
      }
    }
  }
  Serial.println("Failed");
  return false;
}

void sendMessage(String message) {
  if (sendCommand("AT+CIPSTART=\"TCP\",\"" + String(HOST) + "\"," + port, 10000, "OK")) {
    String sendCmd = "AT+CIPSEND=" + String(message.length());
    if (sendCommand(sendCmd, 2000, ">")) {
      Serial1.print(message);
      if (sendCommand("", 2000, "SEND OK")) {
        Serial.println("Message sent successfully");
      } else {
        Serial.println("Error sending message");
      }
    } else {
      Serial.println("Error initiating send command");
    }
    sendCommand("AT+CIPCLOSE", 2000, "OK");
  } else {
    Serial.println("Error starting TCP connection");
  }
}

// void buttonPress(int buttonPin){
//   digitalWrite(LED_BUILTIN, HIGH);
//   Serial.println("Button" + buttonPin + "pressed");
//   sendMessage((String)buttonPin);
//   delay(200);
//   digitalWrite(LED_BUILTIN, LOW);
// }
