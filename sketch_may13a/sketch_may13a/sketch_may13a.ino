#include <WiFi.h>
#include <WebServer.h>
#include "DHT.h"

#define DHTPIN 14
#define DHTTYPE DHT11

const char* ssid = "YenHouse";
const char* password = "077236105";

DHT dht(DHTPIN, DHTTYPE);

// --- Web Server ---
WebServer server(80);  // Prometheus 預設抓 port 80

// --- 處理 /metrics 路徑 ---
void handleMetrics() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    server.send(500, "text/plain", "Sensor read error");
    return;
  }

  String response = "";
  response += "# HELP temperature_celsius Current temperature in Celsius\n";
  response += "# TYPE temperature_celsius gauge\n";
  response += "temperature_celsius " + String(temperature) + "\n";

  response += "# HELP humidity_percent Current humidity in percent\n";
  response += "# TYPE humidity_percent gauge\n";
  response += "humidity_percent " + String(humidity) + "\n";

  server.send(200, "text/plain", response);
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  // 最多等 15 秒連線
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n❌ WiFi 連線失敗，請檢查 SSID 或密碼");
    return;
  }

  Serial.println("\n✅ WiFi 連線成功！");
  Serial.print("ESP32 IP 地址：");
  Serial.println(WiFi.localIP());

  // 註冊 /metrics 路由
  server.on("/metrics", handleMetrics);
  server.begin();
  Serial.println("✅ HTTP 服務啟動成功，路徑：/metrics");
}

void loop() {
  server.handleClient();
}