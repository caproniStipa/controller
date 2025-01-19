#include <ESP32Servo.h>
#include <WiFi.h>
#include <WebServer.h>

// Replace with your network credentials
const char* ssid = "ESP32_AP";
const char* password = "12345678"; // Minimum 8 characters for WPA2

// Create an instance of the WebServer on port 80
WebServer server(80);

// Pin assignments
const int EN1 = 0; // PWM control for motor M1
const int PH1 = 1; // Direction control for motor M1
const int EN2 = 2; // PWM control for motor M2
const int PH2 = 10; // Direction control for motor M2

Servo myservo;

// Embedded CSS
const char css[] PROGMEM = R"rawliteral(
body {
    display: flex;
    overflow: hidden;
    justify-content: center;
    align-items: center;
    height: 100vh;
    margin: 0;
    padding: 0;
    background-color: #edebeb;
    font-weight: bold;
    font-family: Monospace, sans-serif;
}
.controller {
    display: flex;
            justify-content: space-between;
            align-items: center;
            width: 800px;
            height: 275px;
            background: linear-gradient(145deg, #ffffff, #f0f0f0);
            border: none;
            border-radius: 20px;
            padding: 20px;
            box-shadow: 20px 20px 60px #d0d0d0,
                        -20px -20px 60px #ffffff;
}
.slider-container {
    display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            height: 100%;
    }
  input[type="range"] {
            -webkit-appearance: none;
            appearance: none;
            writing-mode: bt-lr;
            transform: rotate(-90deg);
            width: 250px;
            height: 25px;
            background: linear-gradient(90deg, #8e8d94, #44434a);
            outline: none;
            border-radius: 25px;
            margin: 115px 0;
            box-shadow: inset 2px 2px 5px #d0d0d0,
                        inset -2px -2px 5px #ffffff; }
    input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: 75px;
            height: 75px;
            background: linear-gradient(145deg, #bdbdbd, #aba9a9);
            cursor: pointer;
            border-radius: 50%;
            box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.2);
            transition: all 0.3s ease;
        }
        .button-container button {
            width: 150px;
            height: 80px;
            margin: 12px 0;
            background: linear-gradient(145deg, #e6e6e6, #ffffff);
            color: #333;
            border: none;
            border-radius: 15px;
            cursor: pointer;
            font-size: 18px;
            font-weight: bold;
            font-family: Monospace, monospace;
            transition: all 0.3s ease;
            box-shadow: 5px 5px 10px #d0d0d0,
                        -5px -5px 10px #ffffff;
        }
button {
    margin: 10px;
    padding: 10px 20px;
    font-size: 16px;
    cursor: pointer;
    border: none;
    background-color: #007bff;
    color: white;
    border-radius: 5px;
}
button:hover {
    background-color: #0056b3;
}
)rawliteral";

// Embedded JavaScript
const char js[] PROGMEM = R"rawliteral(
function moveR(value) { fetch(`/right?value=${value}`); }
function moveL(value) { fetch(`/left?value=${value}`); }
function servoOpen() { fetch('/servoO'); }
function servoClose() { fetch('/servoC'); }
)rawliteral";

void handleRoot() {
  const char html[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
  <html lang="en">
  <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>RC Tank Controller</title>
      <link rel="stylesheet" href="/style.css">
      <script src="/script.js"></script>
  </head>
  <body>

      <div class="controller">
          <!-- Left Slider -->
          <div class="slider-container">
              <label for="left-slider">LEFT</label>
              <input id="left-slider" type="range" min="-255" max="255" oninput="moveL(this.value)" value="0">
          </div>
          <!-- Buttons -->
          <div class="button-container">
              <button onclick="servoOpen()">OPEN</button>
              <button onclick="servoClose()">CLOSE</button>
              <button>SPECIAL</button>
          </div>
          <!-- Right Slider -->
          <div class="slider-container">
              <label for="right-slider">RIGHT</label>
              <input id="right-slider" type="range" min="-255" max="255" oninput="moveR(this.value)" value="0">
          </div>
      </div>
  <script>
        const leftSlider = document.getElementById('left-slider');
        const rightSlider = document.getElementById('right-slider');
        leftSlider.addEventListener('input', (e) => {
            console.log(`Left slider value: ${e.target.value}`);
        });
        rightSlider.addEventListener('input', (e) => {
            console.log(`Right slider value: ${e.target.value}`);
        });
        leftSlider.addEventListener('touchend', () => {
            leftSlider.style.transition = 'all 0.3s ease';
            leftSlider.value = 0;
            moveL(0)
            console.log('Left slider reset to 0');
            setTimeout(() => {
                leftSlider.style.transition = 'none';
            }, 300);
        });
        rightSlider.addEventListener('touchend', () => {
            rightSlider.style.transition = 'all 0.3s ease';
            rightSlider.value = 0;
            moveR(0)
            console.log('Right slider reset to 0');
            setTimeout(() => {
                rightSlider.style.transition = 'none';
            }, 300);
        });        
        // Button interactions
        document.getElementById('button1').addEventListener('click', () => {
            console.log('Button 1 clicked');
        });
        document.getElementById('button2').addEventListener('click', () => {
            console.log('Button 2 clicked');
        });
        document.getElementById('button3').addEventListener('click', () => {
            console.log('Button 3 clicked');
        });
    </script>
  </body>
  </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

void handleCSS() {
  server.send(200, "text/css", css);
}

void handleJS() {
  server.send(200, "application/javascript", js);
}

void handleRightM() {
  if (server.hasArg("value")) {
    int speedR = server.arg("value").toInt();
    if (speedR > 0) {
      ledcWrite(EN1, speedR);
      digitalWrite(PH1, LOW);
    } else if (speedR < 0) {
      ledcWrite(EN1, -speedR); // Use absolute value for negative speeds
      digitalWrite(PH1, HIGH);
    } else {
      ledcWrite(EN1, 0);
    }
    server.send(200, "text/plain", "Right motor speed updated.");
  } else {
    server.send(400, "text/plain", "Missing 'value' parameter.");
  }
}

void handleLeftM() {
  if (server.hasArg("value")) {
    int speedL = server.arg("value").toInt();
    if (speedL > 0) {
      ledcWrite(EN2, speedL);
      digitalWrite(PH2, LOW);
    } else if (speedL < 0) {
      ledcWrite(EN2, -speedL); // Use absolute value for negative speeds
      digitalWrite(PH2, HIGH);
    } else {
      ledcWrite(EN2, 0);
    }
    server.send(200, "text/plain", "Left motor speed updated.");
  } else {
    server.send(400, "text/plain", "Missing 'value' parameter.");
  }
}

void handleServoOpen() {
  myservo.write(0);
  server.send(200, "text/plain", "Servo opened.");
}

void handleServoClose() {
  myservo.write(180);
  server.send(200, "text/plain", "Servo closed.");
}

void setup() {
  Serial.begin(115200);

  // Set motor pins as outputs
  pinMode(PH1, OUTPUT);
  pinMode(PH2, OUTPUT);

  // Setup PWM for motors
  ledcAttachChannel(EN1, 1000, 8, 0); // Channel 0, 1kHz, 8-bit resolution
  ledcAttachChannel(EN2, 1000, 8, 1); // Channel 1, 1kHz, 8-bit resolution

  // Setup Servo
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50); // Standard 50 Hz servo
  myservo.attach(6, 1000, 2000);

  // Start Access Point
  Serial.println("Setting up Access Point...");
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.println("Access Point established.");
  Serial.print("IP address: ");
  Serial.println(IP);

  // Define routes
  server.on("/", handleRoot);
  server.on("/style.css", handleCSS);
  server.on("/script.js", handleJS);
  server.on("/left", handleLeftM);
  server.on("/right", handleRightM);
  server.on("/servoO", handleServoOpen);
  server.on("/servoC", handleServoClose);

  // Start the server
  server.begin();
}

void loop() {
  server.handleClient();
}
