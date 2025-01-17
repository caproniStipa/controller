#include <ESP32Servo.h>


#include <WiFi.h>
#include <WebServer.h>


// Replace with your network credentials
const char* ssid = "ESP32_AP";
const char* password = "12345678"; // Minimum 8 characters for WPA2
// Create an instance of the WebServer on port 80
WebServer server(80);

int speed = 220;
int EN1 = 0; // PWM control for motor M1
int PH1 = 1; // Direction control for motor M1
int EN2 = 2; // PWM control for motor M2
int PH2 = 10; // Direction control for motor M2

Servo myservo;


String valueString = String(0);

void handleRoot() {
  const char html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML><html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,"> 
    <style>
      html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center; }
      .button { -webkit-user-select: none; -moz-user-select: none; -ms-user-select: none; user-select: none; background-color: #4CAF50; border: none; color: white; padding: 12px 28px; text-decoration: none; font-size: 26px; margin: 1px; cursor: pointer; }
      .button2 {background-color: #555555;}
    </style>
    <script>
      function moveForward() { fetch('/forward'); }
      function moveLeft() { fetch('/left'); }
      function stopRobot() { fetch('/stop'); }
      function moveRight() { fetch('/right'); }
      function moveReverse() { fetch('/reverse'); }
      function servoOpen() { fetch('/servo'); }

      function updateMotorSpeed(pos) {
        document.getElementById('motorSpeed').innerHTML = pos;
        fetch(`/speed?value=${pos}`);
      }
    </script>
  </head>
  <body>
    <h1>ESP32 Motor Control</h1>
    <p><button class="button" onclick="moveForward()">FORWARD</button></p>
    <div style="clear: both;">
      <p>
        <button class="button" onclick="moveLeft()">LEFT</button>
        <button class="button button2" onclick="stopRobot()">STOP</button>
        <button class="button" onclick="moveRight()">RIGHT</button>
      </p>
    </div>
    <div>
    <p><button class="button" onclick="moveReverse()">REVERSE</button></p>
    </div>
    <div>
    <p><button class="button" onclick="servoOpen()">SERVO_OPEN</button></p>
    </div>
    <p>Motor Speed: <span id="motorSpeed">0</span></p>
    <input type="range" min="0" max="100" step="25" id="motorSlider" oninput="updateMotorSpeed(this.value)" value="0"/>
  </body>
  </html>)rawliteral";
  server.send(200, "text/html", html);
}

void handleForward() {
  analogWrite(EN1, speed);
  digitalWrite(PH1, LOW);
   analogWrite(EN2, speed);
  digitalWrite(PH2, LOW);
}

void handleLeft() {
  analogWrite(EN1, speed);
  digitalWrite(PH1, LOW);
   analogWrite(EN2, speed);
  digitalWrite(PH2, HIGH);
}

void handleStop() {
  analogWrite(EN1, 0);
  analogWrite(EN2, 0);
}

void handleRight() {
  analogWrite(EN1, speed);
  digitalWrite(PH1, HIGH);
   analogWrite(EN2, speed);
  digitalWrite(PH2, LOW);
}

void handleReverse() {
  analogWrite(EN1, speed);
  digitalWrite(PH1, HIGH);
  analogWrite(EN2, speed);
  digitalWrite(PH2, HIGH);
}

void handleServoOpen() {
  myservo.write(180);
}



void setup() {
  Serial.begin(115200);

  // Set the Motor pins as outputs
  ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
	myservo.setPeriodHertz(50);    // standard 50 hz servo
	myservo.attach(6, 1000, 2000); // attaches the servo on pin 18 to the servo object


  pinMode(EN1, OUTPUT);
  pinMode(PH1, OUTPUT);
  pinMode(EN2, OUTPUT);
  pinMode(PH2, OUTPUT);

  // Configure PWM Pins
    
  // Initialize PWM with 0 duty cycle
  
  // Connect to Wi-Fi
  Serial.println("Setting up Access Point...");
WiFi.softAP(ssid, password);

IPAddress IP = WiFi.softAPIP();
Serial.println("Access Point established.");
Serial.print("IP address: ");
Serial.println(IP);

  // Define routes
  server.on("/", handleRoot);
  server.on("/forward", handleForward);
  server.on("/left", handleLeft);
  server.on("/stop", handleStop);
  server.on("/right", handleRight);
  server.on("/reverse", handleReverse);
  server.on("/servo", handleServoOpen);

  // Start the server
  server.begin();
}

void loop() {
  server.handleClient();
}