//  Wireless Control via WIFI  //

#include <WiFi.h>

// Set the credentials for your access point
const char* ssid = "ESP32_SG";       // Set your AP SSID
const char* password = "12345678";   // Set your AP password

WiFiServer server(80);               // Web server port number is 80
// Variable to store the HTTP request
String header;

// Variables to store the current output state of GPIO pins
String output2State = "off";   // GPIO 2 (Onboard LED)
String output13State = "off";  // GPIO 13 (External LED)

// Assign GPIO pins for controlling output
const int LED_BUILTIN = 13;  // Onboard LED (GPIO 2)
const int LED_PIN = 13;     // External LED (GPIO 13)

String slider_value = "0";  // Default slider value is 0 (off)

// Auxiliary variables for time control
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  Serial.println("Configuring access point...");

  // Set GPIO pins for output
  pinMode(LED_BUILTIN, OUTPUT);  // Onboard LED (GPIO 2)
  pinMode(LED_PIN, OUTPUT);      // External LED (GPIO 13)

  // Set initial states of GPIO pins
  analogWrite(LED_BUILTIN, 0);  // Onboard LED OFF (PWM 0)
  analogWrite(LED_PIN, 0);      // External LED OFF (PWM 0)

  // Create the Wi-Fi access point
  if (!WiFi.softAP(ssid, password)) {
    Serial.println("AP Creation Failed.");
    while (1);  // Halt if the AP creation fails
  }

  // Print the IP address of the access point
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // Start the server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  WiFiClient client = server.accept();   // Listen for incoming clients

  if (client) {  // If a new client connects
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");
    String currentLine = "";  // Hold incoming data from the client

    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // Loop while the client is connected
      currentTime = millis();
      if (client.available()) {  // If there is data from the client
        char c = client.read();  // Read the byte
        Serial.write(c);         // Print it to the serial monitor
        header += c;

        if (c == '\n') {  // If the byte is a newline character
          if (currentLine.length() == 0) {  // Blank line means end of HTTP request
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Control GPIO pins based on requests
            if (header.indexOf("GET /2/on") >= 0) {
              output2State = "on";
              analogWrite(LED_BUILTIN, 255);  // Turn on Onboard LED (GPIO 2) with full brightness
            } else if (header.indexOf("GET /2/off") >= 0) {
              output2State = "off";
              analogWrite(LED_BUILTIN, 0);   // Turn off Onboard LED (GPIO 2)
            }

            if (header.indexOf("GET /13/on") >= 0) {
              output13State = "on";
              analogWrite(LED_PIN, 255);  // Turn on External LED (GPIO 13) with full brightness
            } else if (header.indexOf("GET /13/off") >= 0) {
              output13State = "off";
              analogWrite(LED_PIN, 0);   // Turn off External LED (GPIO 13)
            }

            // Slider control for GPIO 2 (Onboard LED) brightness
            if (header.indexOf("GET /slider?value=") >= 0) {
              int startIndex = header.indexOf("GET /slider?value=") + 18;
              int endIndex = header.indexOf(" ", startIndex);
              String value = header.substring(startIndex, endIndex);
              slider_value = value;  // Update slider value
              int brightness = slider_value.toInt();
              if (output2State = "on"){
                analogWrite(LED_BUILTIN, brightness);
              }
              else{
                analogWrite(LED_BUILTIN, 0);
              }     // Adjust Onboard LED (GPIO 2) brightness
            }

            // Display the HTML page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}"); 
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}"); 
            client.println(".button2 {background-color: #555555;}");
            // Added style for slider to make it wider
            client.println("input[type=range] { width: 80%; }");
            client.println("</style></head>");
            client.println("<body><h1>Sassan's ESP32 Web Server</h1>");

            // Onboard LED (GPIO 2) control buttons
            client.println("<p>GPIO 2 - State " + output2State + "</p>");
            if (output2State == "off") {
              client.println("<p><a href=\"/2/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/2/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            // External LED (GPIO 13) control buttons
            client.println("<p>GPIO 13 - State " + output13State + "</p>");
            if (output13State == "off") {
              client.println("<p><a href=\"/13/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/13/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            // Slider for controlling GPIO 2 brightness
            client.println("<p>Brightness (GPIO 2) - " + slider_value + "</p>");
            client.println("<input type=\"range\" min=\"0\" max=\"255\" value=\"" + slider_value + "\" onchange=\"location.href='/slider?value=' + this.value\">");

            client.println("</body></html>");
            client.println();  // End of the response
            break;  // Break out of the loop
          } else {
            currentLine = "";  // Reset current line
          }
        } else if (c != '\r') {
          currentLine += c;  // Add character to current line
        }
      }
    }
    header = "";  // Clear the header variable
    client.stop();  // Close the client connection
    Serial.println("Client disconnected.");
  }
}
