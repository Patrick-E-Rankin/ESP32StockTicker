//This code came from https://github.com/Patrick-E-Rankin/ESP32StockTicker
//Your free to do what you want with it, if you have any useful changes/code clean up, then please let me know.

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LEDMatrixDriver.hpp>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

const char* ssid = "YOUR WIFI SSID";
const char* password = "YOUR WIFI PASSWORD";
String serverName = "https://query1.finance.yahoo.com/v8/finance/chart/";
String token = "?interval=1d";
const uint8_t LEDMATRIX_CS_PIN = 15;
const int LEDMATRIX_SEGMENTS = 4;
const int LEDMATRIX_WIDTH    = LEDMATRIX_SEGMENTS * 8;
LEDMatrixDriver lmd(LEDMATRIX_SEGMENTS, LEDMATRIX_CS_PIN);
const int ANIM_DELAY = 75; //75 is slow enough
String ticker1 = "none";
char displayString[30] = "";
const char* PARAM_INPUT_1 = "input1";
//Website Design
const char Index[] = "<!DOCTYPE HTML><html><head><title>ESP32 Stock Ticker</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><style>body{display:flex;flex-direction:column;justify-content:center;align-items:center;background-color:#333333;color:#FFFFFF;font-family:Arial,sans-serif}header{display:flex;flex-direction:column;justify-content:center;align-items:center;margin-bottom:10px;font-size:32px;font-weight:bold;text-transform:uppercase;margin-top:-5px}img{max-width:150px;max-height:150px}form{display:flex;flex-direction:column;justify-content:center;align-items:center}input[type=\"text\"],input[type=\"submit\"]{margin-bottom:10px;background:linear-gradient(to bottom right,#add8e6,#c9a0dc);color:#FFFFFF;border:none;padding:8px 16px;font-size:24px;font-family:Arial,sans-serif;border-radius:4px;opacity:0.9}input[type=\"submit\"]{cursor:pointer}a{color:#B19CD9;display:flex;flex-direction:column;justify-content:center;align-items:center}</style></head><body><div class=\"logo-container\"><img src=\"https://bafybeif4fh2vnsitnisirlcaeru6sbf6h3qvkbpjgbsckymkufttlkgmbe.ipfs.nftstorage.link/Multi_Logo_Simple-removebg-preview.png\" alt=\"Logo Placeholder\"></div><header>Ticker Symbol</header><form action=\"/get\"><input type=\"text\" name=\"input1\"><br><input type=\"submit\" value=\"Submit\"></form><br><a href=\"https://github.com/Patrick-E-Rankin/ESP32StockTicker\">ESP32StockTicker</a></body></html>";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

TaskHandle_t Task1;

int x=0, y=0;   // start top left
byte font[95][8] = { {0,0,0,0,0,0,0,0}, // SPACE
                     {0x10,0x18,0x18,0x18,0x18,0x00,0x18,0x18}, // EXCL
                     {0x28,0x28,0x08,0x00,0x00,0x00,0x00,0x00}, // QUOT
                     {0x00,0x0a,0x7f,0x14,0x28,0xfe,0x50,0x00}, // #
                     {0x10,0x38,0x54,0x70,0x1c,0x54,0x38,0x10}, // $
                     {0x00,0x60,0x66,0x08,0x10,0x66,0x06,0x00}, // %
                     {0,0,0,0,0,0,0,0}, // &
                     {0x00,0x10,0x18,0x18,0x08,0x00,0x00,0x00}, // '
                     {0x02,0x04,0x08,0x08,0x08,0x08,0x08,0x04}, // (
                     {0x40,0x20,0x10,0x10,0x10,0x10,0x10,0x20}, // )
                     {0x00,0x10,0x54,0x38,0x10,0x38,0x54,0x10}, // *
                     {0x00,0x08,0x08,0x08,0x7f,0x08,0x08,0x08}, // +
                     {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x08}, // COMMA
                     {0x00,0x00,0x00,0x00,0x7e,0x00,0x00,0x00}, // -
                     {0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x06}, // DOT
                     {0x00,0x04,0x04,0x08,0x10,0x20,0x40,0x40}, // /
                     {0x00,0x38,0x44,0x4c,0x54,0x64,0x44,0x38}, // 0
                     {0x00,0x04,0x0c,0x14,0x04,0x04,0x04,0x04}, // 1
                     {0x00,0x38,0x44,0x04,0x04,0x38,0x40,0x7c}, // 2
                     {0x00,0x38,0x44,0x04,0x18,0x04,0x44,0x38}, // 3
                     {0x00,0x04,0x0c,0x14,0x24,0x7e,0x04,0x04}, // 4
                     {0x00,0x7c,0x40,0x40,0x78,0x04,0x04,0x78}, // 5
                     {0x00,0x38,0x40,0x40,0x78,0x44,0x44,0x38}, // 6
                     {0x00,0x7c,0x04,0x08,0x10,0x10,0x10,0x10}, // 7
                     {0x00,0x38,0x44,0x44,0x38,0x44,0x44,0x38}, // 8
                     {0x00,0x38,0x44,0x44,0x3c,0x04,0x04,0x38}, // 9
                     {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00}, // :
                     {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x08}, // ;
                     {0x00,0x10,0x20,0x40,0x80,0x40,0x20,0x10}, // <
                     {0x00,0x00,0x7e,0x00,0x00,0xfc,0x00,0x00}, // =
                     {0x00,0x08,0x04,0x02,0x01,0x02,0x04,0x08}, // >
                     {0x00,0x38,0x44,0x04,0x08,0x10,0x00,0x10}, // ?
                     {0x00,0x30,0x48,0xba,0xba,0x84,0x78,0x00}, // @
                     {0x00,0x3c,0x42,0x42,0x42,0x7e,0x42,0x42}, // A
                     {0x00,0x78,0x44,0x44,0x78,0x44,0x44,0x78}, // B
                     {0x00,0x38,0x44,0x40,0x40,0x40,0x44,0x38}, // C
                     {0x00,0x7c,0x42,0x42,0x42,0x42,0x42,0x7c}, // D
                     {0x00,0x78,0x40,0x40,0x70,0x40,0x40,0x78}, // E {0x00,0x78,0x40,0x40,0x70,0x40,0x40,0x78}
                     {0x00,0x7c,0x40,0x40,0x7c,0x40,0x40,0x40}, // F
                     {0x00,0x38,0x44,0x40,0x5c,0x44,0x44,0x38}, // G
                     {0x00,0x42,0x42,0x42,0x7e,0x42,0x42,0x42}, // H
                     {0x00,0x7c,0x10,0x10,0x10,0x10,0x10,0x7c}, // I
                     {0x00,0x04,0x04,0x04,0x04,0x04,0x24,0x18}, // J
                     {0x00,0x44,0x48,0x50,0x60,0x50,0x48,0x44}, // K
                     {0x00,0x40,0x40,0x40,0x40,0x40,0x40,0x7c}, // L
                     {0x00,0x82,0xc6,0xaa,0x92,0x82,0x82,0x82}, // M
                     {0x00,0x42,0x42,0x62,0x52,0x4a,0x46,0x42}, // N
                     {0x00,0x3c,0x42,0x42,0x42,0x42,0x42,0x3c}, // O
                     {0x00,0x78,0x44,0x44,0x44,0x78,0x40,0x40}, // P
                     {0x00,0x3c,0x42,0x42,0x52,0x4a,0x44,0x3a}, // Q
                     {0x00,0x78,0x44,0x44,0x78,0x50,0x48,0x44}, // R
                     {0x00,0x3c,0x40,0x40,0x38,0x04,0x04,0x78}, // S
                     {0x00,0xfe,0x10,0x10,0x10,0x10,0x10,0x10}, // T
                     {0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x3c}, // U
                     {0x00,0x42,0x42,0x42,0x42,0x42,0x24,0x18}, // V
                     {0x00,0x82,0x82,0x92,0x92,0x92,0x54,0x38}, // W
                     {0x00,0x82,0x44,0x28,0x10,0x28,0x44,0x82}, // X
                     {0x00,0x82,0x44,0x28,0x10,0x10,0x10,0x10}, // Y
                     {0x00,0xfe,0x04,0x08,0x10,0x20,0x40,0xfe}, // Z
                      // (the font does not contain any lower case letters. you can add your own.)
                  };    // {}, //

void drawSprite( byte* sprite, int x, int y, int width, int height )
{
  // The mask is used to get the column bit from the sprite row
  byte mask = B10000000;

  for( int iy = 0; iy < height; iy++ )
  {
    for( int ix = 0; ix < width; ix++ )
    {
      lmd.setPixel(x + ix, y + iy, (bool)(sprite[iy] & mask ));

      // shift the mask by one pixel to the right
      mask = mask >> 1;
    }

    // reset column mask
    mask = B10000000;
  }
}

void drawString(char* text, int len, int x, int y )
{
  for( int idx = 0; idx < len; idx ++ )
  {
    int c = text[idx] - 32;

    // stop if char is outside visible area
    if( x + idx * 8  > LEDMATRIX_WIDTH )
      return;

    // only draw if char is visible
    if( 8 + x + idx * 8 > 0 )
      drawSprite( font[c], x + idx * 8, y, 8, 8 );
  }
}

void LEDDisplay(void * parameter){

            while(true){

            int len = strlen(displayString);
  
            drawString(displayString, len, x, 0);
           
            lmd.display();
            delay(ANIM_DELAY);
            if( --x < len * -8 ) {
              x = LEDMATRIX_WIDTH;
              }
            }
}

void setup() 
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.println(".");
    delay(500);
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  lmd.setEnabled(true);
  lmd.setIntensity(0);   // 0 = low, 10 = high
  char text[30];
  String IP = WiFi.localIP().toString();
  strcat(text,IP.c_str());
  memcpy(displayString,text,30); 
  xTaskCreatePinnedToCore(LEDDisplay,"LedDisplay",1000,NULL,1,&Task1,0);
  delay(1000);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){request->send(200, "text/html", Index);});
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) 
  {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) 
    {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      ticker1 = inputMessage;
    }
      request->redirect("/");
   });
    server.onNotFound(notFound);
    server.begin();
}

void loop() {
    if((WiFi.status()== WL_CONNECTED) && (ticker1 != "none"))
    {
      HTTPClient http;
      http.begin(serverName+ticker1+token);
      int httpResponseCode = http.GET();
      

      if (httpResponseCode > 0) 
      {
        Serial.print("HTTP Response Code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();

        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, payload);
        if (error) 
        {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          return;
        }
        JsonObject chart_result_0 = doc["chart"]["result"][0];
        JsonObject chart_result_0_meta = chart_result_0["meta"];
        const char* tempsymbol = chart_result_0_meta["symbol"];
        float tempmarketprice = chart_result_0_meta["regularMarketPrice"]; 
        Serial.println(tempsymbol);
        Serial.println(tempmarketprice);
        char pricebuffer[sizeof(tempmarketprice)+1];
        dtostrf(tempmarketprice, sizeof(tempmarketprice), 2, pricebuffer);
        char text[30] = "";
        strcat(text,tempsymbol);
        strcat(text," ");
        strcat(text,pricebuffer);
        strcat(text," ");     
        memcpy(displayString,text,30);
        Serial.println(displayString);
      }
      else 
      {
        Serial.print("Error Code: ");
        Serial.println(httpResponseCode);
      }
      http.end();
      delay(5000);
    }
      
}
