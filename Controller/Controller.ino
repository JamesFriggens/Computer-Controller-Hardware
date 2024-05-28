#define ESP8266_BAUD 115200

// Use Serial1 on the Pico for communication with the ESP8266
#define RX_PIN 1
#define TX_PIN 0

// Button pins
#define BUTTON1_PIN 15
#define BUTTON2_PIN 14
#define BUTTON3_PIN 13

String ssid = "";
String password = "";

String host = ""; // Replace with your server's IP or domain name
int port = 10001;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  // Start communication with the Pico
  Serial.begin(115200);
  // while (!Serial) {
  //   ; // wait for serial port to connect. Needed for native USB
  // }
  Serial.println("Serial communication started.");

  // Start communication with the ESP8266
  Serial1.begin(ESP8266_BAUD);
  delay(1000);

  // Initialize ESP8266
  sendCommand("AT", 2000, "OK");
  sendCommand("AT+CWMODE=1", 2000, "OK");
  sendCommand("AT+CWJAP=\"" + ssid + "\",\"" + password + "\"", 10000, "OK");

  // Initialize buttons as input with pull-up resistors
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);

  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  if (digitalRead(BUTTON1_PIN) == LOW) {
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("Button 1 pressed");
    sendMessage("1");
    delay(200); // debounce
  }

  if (digitalRead(BUTTON2_PIN) == LOW) {
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("Button 2 pressed");
    sendMessage("2");
    delay(200);
  }

  if (digitalRead(BUTTON3_PIN) == LOW) {
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("Button 3 pressed");
    sendMessage("3");
    delay(200);
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
  if (sendCommand("AT+CIPSTART=\"TCP\",\"" + host + "\"," + port, 10000, "OK")) {
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
