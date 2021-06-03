
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// REPLACE WITH YOUR NETWORK CREDENTIALS
const char *ssid = "MuAlpha";
const char *password = ".hack//197228";

const char *PARAM_INPUT = "speedValue";

char move_cmd = 's'; //initially rover is in stop condition
String distance = "0";
int speed = 0;

unsigned long last_time;
// HTML web page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
    <title>ESP01 Autonomous Robot Server</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,">
    <style>
        html {
            font-family: Helvetica;
            display: inline-block;
            margin: 0px auto;
            text-align: center;
        }

        h1 {
            color: #0F3376;
            padding: 2vh;
        }

        p {
            font-size: 1.5rem;
        }

        .button {
            display: inline-block;
            background-color: #e7bd3b;
            border: none;
            border-radius: 50%;
            color: white;
            padding: 16px 40px;
            text-decoration: none;
            font-size: 30px;
            margin: 2px;
            cursor: pointer;
        }

        .button2 {
            background-color: #f44242;
        }
        .button3 {
            background-color: #85e967;
        }
        .slidecontainer {
  width: 100%; /* Width of the outside container */
}

/* The slider itself */
.slider {
  -webkit-appearance: none;  /* Override default CSS styles */
  appearance: none;
  width: 50%; /* Full-width */
  height: 25px; /* Specified height */
  background: #d3d3d3; /* Grey background */
  outline: none; /* Remove outline */
  opacity: 0.7; /* Set transparency (for mouse-over effects on hover) */
  -webkit-transition: .2s; /* 0.2 seconds transition on hover */
  transition: opacity .2s;
}

/* Mouse-over effects */
.slider:hover {
  opacity: 1; /* Fully shown on mouse-over */
}

/* The slider handle (use -webkit- (Chrome, Opera, Safari, Edge) and -moz- (Firefox) to override default look) */
.slider::-webkit-slider-thumb {
  -webkit-appearance: none; /* Override default look */
  appearance: none;
  width: 25px; /* Set a specific slider handle width */
  height: 25px; /* Slider handle height */
  background: #04AA6D; /* Green background */
  cursor: pointer; /* Cursor on hover */
}

.slider::-moz-range-thumb {
  width: 25px; /* Set a specific slider handle width */
  height: 25px; /* Slider handle height */
  background: #04AA6D; /* Green background */
  cursor: pointer; /* Cursor on hover */
}
    </style>
</head>

<body>
    <h1>ESP01 Autonomous Robot Server</h1>
    </p>
    <p><button class="button" onmousedown="toggleCheckbox('forward');" ontouchstart="toggleCheckbox('forward');">&#8593;</button></a></p> 
    <p><button class="button" onmousedown="toggleCheckbox('left');" ontouchstart="toggleCheckbox('left');">&#8592;</button></a>
        <button class="button button2" onmousedown="toggleCheckbox('stop');" ontouchstart="toggleCheckbox('stop');">&#88;</button></a>
        <button class="button" onmousedown="toggleCheckbox('right');" ontouchstart="toggleCheckbox('right');">&#8594;</button></a></p>
    <p><button class="button" onmousedown="toggleCheckbox('back');" ontouchstart="toggleCheckbox('back');">&#8595;</button></a></p>
    <p><button class="button button3" onmousedown="toggleCheckbox('auto');" ontouchstart="toggleCheckbox('auto');">Autopilot</button></a></p>
    <p><div class="slidecontainer">
        Speed:<input type="range" onchange="updateSpeed(this)" min="0" max="100" value="0" class="slider" id="speedSlider">
      </div></p>
    <p>Distance from obstacle : %DISTANCE% m</p>
</body>
<script>
    function updateSpeed(element){
    var speedValue = document.getElementById("speedSlider").value;
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/slider?speedValue="+speedValue, true);
    xhr.send();

    }
    function toggleCheckbox(x) {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/" + x, true);
      xhr.send();
    }
   </script>
</html>)rawliteral";

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

AsyncWebServer server(80);

String processor(const String &var)
{
  //Serial.println(var);
  if (var == "DISTANCE")
  {
    return distance;
  }
}

  void setup()
  {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
      Serial.println("WiFi Failed!");
      return;
    }
    Serial.println();
    Serial.print("ESP IP Address: http://");
    Serial.println(WiFi.localIP());

    // Send web page to client
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send_P(200, "text/html", index_html);
    });

    // Receive an HTTP GET request
    server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request) {
      move_cmd = 'l';
      request->send(200, "text/plain", "ok");
    });

    // Receive an HTTP GET request
    server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request) {
      move_cmd = 'r';
      request->send(200, "text/plain", "ok");
    });

    // Receive an HTTP GET request
    server.on("/forward", HTTP_GET, [](AsyncWebServerRequest *request) {
      move_cmd = 'f';
      request->send(200, "text/plain", "ok");
    });

    // Receive an HTTP GET request
    server.on("/back", HTTP_GET, [](AsyncWebServerRequest *request) {
      move_cmd = 'b';
      request->send(200, "text/plain", "ok");
    });

    // Receive an HTTP GET request
    server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request) {
      move_cmd = 's';
      request->send(200, "text/plain", "ok");
    });

    // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
    server.on("/slider", HTTP_GET, [](AsyncWebServerRequest *request) {
      String inputMessage;
      // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
      if (request->hasParam(PARAM_INPUT))
      {
        inputMessage = request->getParam(PARAM_INPUT)->value();
        // sliderValue = inputMessage;
        speed = inputMessage.toInt();
      }
      else
      {
        inputMessage = "No message sent";
      }
      Serial.println(inputMessage);
      request->send(200, "text/plain", "OK");
    });

    server.onNotFound(notFound);
    server.begin();
    last_time = millis();
  }

  void loop()
  {
    unsigned long cur_time = millis();
    if (cur_time - last_time > 100)
    { 
      String speed_val = (String) speed;
      switch (move_cmd)
      {
      case 's':
        Serial.println("Stop");
        break;
      case 'f':
        Serial.print(speed_val);
        Serial.println(", forward");
        break;
      case 'b':
        Serial.print(speed_val);
        Serial.println(", backward");
        break;
      case 'l':
        Serial.print(speed_val);
        Serial.println(", left");
        break;
      case 'r':
        Serial.print(speed_val);
        Serial.println(", right");
        break;
      default:
        Serial.println("Stop");
      }
      distance = (String)random(6);
    last_time = cur_time;
    }
    
  }