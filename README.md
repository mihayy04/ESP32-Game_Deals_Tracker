# ESP32-Game_Deals_Tracker
Proiect IoT cu ESP32 pentru configurarea rețelei Wi-Fi prin Bluetooth și preluarea ofertelor de jocuri via REST API.


## 🛠️ Tech Stack
* **Hardware:** ESP32 Development Board
* **Language:** C++ (written in Arduino IDE)
* **Main Libraries:** * `WiFi.h` & `HTTPClient.h` -> for connecting to the internet and making HTTP GET requests to the API.
  * `BluetoothSerial.h` -> for Classic Bluetooth communication with the phone.
  * `ArduinoJson` -> heavily used to build and parse (deserialize) the data packets moving between the phone, ESP32, and the web server.

## ⚙️ How it works
The code waits for JSON-formatted commands via Bluetooth from a mobile app. Depending on the received command, it does the following:

1. **`getNetworks`** - The ESP32 scans for nearby Wi-Fi networks and sends the list back to the phone.
2. **`connect`** - Receives the Wi-Fi name and password from the phone, connects to the router, and confirms the connection.
3. **`getData`** - Makes an HTTP request to the CheapShark API and fetches a list of the latest game deals (title, ID, thumbnail).
4. **`getDetails`** - If it receives a specific game ID, it makes another API request to get advanced details (sale price, normal price, Metacritic score, Steam rating, etc.), formats them nicely, and sends them back to the phone.

## 🚀 How to run the code
1. Upload the code to the ESP32 using Arduino IDE.
2. Turn on your phone's Bluetooth and pair with the device named `A15_BTC`.
3. Using a Bluetooth terminal app, send JSON commands, for example:
   `{"action": "getNetworks", "teamId": "A15"}`