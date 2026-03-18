#include <Arduino.h>
#include <BluetoothSerial.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
  #error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

String ssid = "";
String password = "";
String teamId = "A15";
bool connected_WiFi;
#define CONNECT_TIMEOUT 15000 
long connectStart = 0;
bool connected;

#define btcServerName "A15_BTC"

BluetoothSerial SerialBT;
String data = "";

void deviceConnected(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    Serial.println("Device is Connected");
    connected = true;
  }

  if (event == ESP_SPP_CLOSE_EVT) {
    Serial.println("Device is disconnected");
    connected = false;
  }
}

void receivedData() {
  while (SerialBT.available()) {
    data = SerialBT.readStringUntil('\n');
  }

  Serial.println(data); 
  
  String url = "http://proiectia.bogdanflorea.ro/api/cheapshark/deals";
  String url1 = "http://proiectia.bogdanflorea.ro/api/cheapshark/deal?dealID=";
  String api_key = "";
  
  DynamicJsonDocument jsonDoc(2048);
  DeserializationError error = deserializeJson(jsonDoc, data);
  
  if (error) {
    Serial.print("JSON deserialization failed: ");
    Serial.println(error.c_str());
  }

  String message_action = jsonDoc["action"];

  if (message_action == "getNetworks") {
    String message_teamId = jsonDoc["teamId"];
    String teamId = message_teamId;
  
    Serial.println("Start scan");
    int networksFound = WiFi.scanNetworks();
    Serial.println("Scan complete");

    if (networksFound == 0) {
      Serial.println("No networks found");
    } else {
      Serial.print(networksFound);
      Serial.println(" networks found");

      for (int i = 0; i < networksFound; i++) {
        DynamicJsonDocument network(1024);
        network["ssid"] = WiFi.SSID(i);
        network["strength"] = WiFi.RSSI(i);
        network["encryption"] = WiFi.encryptionType(i);
        network["teamId"] = teamId;
        
        String jsonString;
        serializeJson(network, jsonString);
        Serial.println(jsonString);
        SerialBT.println(jsonString);
        delay(100);
      }
    }
  } else if (message_action == "connect") {
    String message_ssid = jsonDoc["ssid"];
    String message_password = jsonDoc["password"];
    ssid = message_ssid;
    password = message_password;

    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.println("Connecting");
    
    connectStart = millis();
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
      if (millis() - connectStart > CONNECT_TIMEOUT) {
        break;
      }
    }
    Serial.println("");
    connected_WiFi = false;

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Connection failed");
    } else {
      Serial.print("Connected to network: ");
      Serial.println(WiFi.SSID());
      Serial.print("Local IP address: ");
      Serial.println(WiFi.localIP());
      connected_WiFi = true;
    }
    
    DynamicJsonDocument Connected(200);
    Connected["ssid"] = ssid;
    Connected["connected"] = connected_WiFi;
    Connected["teamId"] = teamId;
    String output1;
    serializeJson(Connected, output1);
    SerialBT.println(output1);
  } else if (message_action == "getData") {
    Serial.println("Retrieving data from server....");

    HTTPClient http;
    http.begin(url);
    http.setConnectTimeout(3000); 
    http.setTimeout(3000); 

    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String payload = http.getString();
      DynamicJsonDocument jsonDoc(15000);
      DeserializationError error = deserializeJson(jsonDoc, payload);
      
      if (error) {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }

      JsonArray recordsArray = jsonDoc.as<JsonArray>();
      DynamicJsonDocument jsonDoc2(15000);

      for (JsonObject record : recordsArray) {
        String name = record["title"];
        String id = record["dealID"];
        String image = record["thumb"];

        Serial.print("Record title: ");
        Serial.println(name);
        Serial.print("dealID: ");
        Serial.println(id);
        Serial.print("Image URL: ");
        Serial.println(image);
        Serial.print("teamId: ");
        Serial.println(teamId);

        jsonDoc2["name"] = name;
        jsonDoc2["id"] = id;
        jsonDoc2["image"] = image;
        jsonDoc2["teamId"] = teamId;

        String jsonResponse2;
        serializeJson(jsonDoc2, jsonResponse2);

        SerialBT.println(jsonResponse2);
      }
    } else {
      Serial.println("Failed to retrieve data from server");
    }
    http.end();
  } else if (message_action == "getDetails") {
    String selected_dealID = jsonDoc["id"];
    Serial.print(F("ID Details: "));
    Serial.println(selected_dealID);

    Serial.println("Sending GET request to remote server....");

    HTTPClient http1;
    String full_url = String(url1) + String(selected_dealID);
    Serial.println(full_url);

    http1.begin(full_url);
    http1.setConnectTimeout(30000); 
    http1.setTimeout(30000); 
    int httpResponseCode1 = http1.GET();
  
    if (httpResponseCode1 > 0) {
      String payload1 = http1.getString();
      DynamicJsonDocument doc3(15000);
      deserializeJson(doc3, payload1);
      Serial.println(payload1);

      if (error) {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode1);
      }
 
      String name = doc3["internalName"];
      String title = doc3["title"];
      String meta_Link = doc3["metacriticLink"];
      String deal = doc3["dealID"];
      int storeID = doc3["storeID"];
      int gameID = doc3["gameID"];
      float salePrice = doc3["salePrice"];
      float normalPrice = doc3["normalPrice"];
      int isOnSale = doc3["isOnSale"];
      float savings = doc3["savings"];
      int score = doc3["metacriticScore"];
      String steamtext = doc3["steamRatingText"];
      String steamPercent = doc3["steamRatingPercent"];
      int steamcount = doc3["SteamRatingCount"];
      int steamid = doc3["steamAppID"];
      long int releaseDate = doc3["releaseDate"];
      long int lastChange = doc3["lastChange"];
      float dealRating = doc3["dealRating"];
      String image = doc3["thumb"].as<String>();

      DynamicJsonDocument jsonDoc3(4096);
      
      Serial.print("internalName: ");
      Serial.println(name);
      Serial.print("title: ");
      Serial.println(title);
      Serial.print("Image URl: ");
      Serial.println(image);
      Serial.print("metacriticLink: ");
      Serial.println(meta_Link);
      Serial.print("dealID: ");
      Serial.println(deal);
      Serial.print("storeId: ");
      Serial.println(storeID);
      Serial.print("gameID: ");
      Serial.println(gameID);
      Serial.print("salePrice: ");
      Serial.println(salePrice);
      Serial.print("normalPrice: ");
      Serial.println(normalPrice);
      Serial.print("isOnSale: ");
      Serial.println(isOnSale);
      Serial.print("savings: ");
      Serial.println(savings);
      Serial.print("metaCriticsScore: ");
      Serial.println(score);
      Serial.print("steamRatingText: ");
      Serial.println(steamtext);
      Serial.print("steamRatingPercent: ");
      Serial.println(steamPercent);
      Serial.print("steamRatingCount: ");
      Serial.println(steamcount);
      Serial.print("steamAppID: ");
      Serial.println(steamid);
      Serial.print("releaseDate: ");
      Serial.println(releaseDate);
      Serial.print("lastChange: ");
      Serial.println(lastChange);
      Serial.print("dealRating: ");
      Serial.println(dealRating);
       
      String description = ("internalName: " + String(name) + "\n" "title" + String(title) + "\n"
                            + "metacriticLink" + String(meta_Link) +"\n" + "dealID" + String(deal) +"\n"
                            + "storeID" + String(storeID) +"\n" + "gameID" + String(gameID) +"\n" 
                            + "salePrice" + String(salePrice) + "\n" + "normalPrice" + String(normalPrice) + "\n"
                            + "isOnSale" + String(isOnSale) + "\n" + "savings" + String(savings) + "\n"
                            + "metacriticsScore" + String(score) + "\n" + "steamRatingText" + String(steamtext) + "\n"
                            + "steamRatingPercent" + String(steamPercent) + "\n" + 
                            + "steamRatingCount" + String(steamcount) + "\n" + "steamAppID" + String(steamid) + "\n"
                            + "releaseDate" + String(releaseDate) + "\n" + "lastChange" + String(lastChange) + "\n"
                            + "dealRating" + String(dealRating) + "\n");
                              
      Serial.print("description: ");
      Serial.println(description);            
      jsonDoc3["image"] = doc3["thumb"].as<String>();
      jsonDoc3["description"] = description;
      jsonDoc3["teamId"] = teamId;

      String jsonResponse3;
      serializeJson(jsonDoc3, jsonResponse3);

      SerialBT.println(jsonResponse3);
    } else {
      Serial.println("failed to get details from server");
    }

    http1.end();
  }

  data = "";
}

void setup() {
  Serial.begin(115200);

  SerialBT.begin(btcServerName); 
  Serial.println("The device started, now you can pair it with bluetooth!");

  SerialBT.register_callback(deviceConnected);
  Serial.println("The bluetooth device is ready to pair");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(2000);
}

void loop() {
  if (SerialBT.available()) {
    Serial.println("Bluetooth Active");
    receivedData();
  }
}