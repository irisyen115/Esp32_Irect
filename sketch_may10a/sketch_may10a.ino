#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

const uint16_t kRecvPin = 12;  // 接收腳位
IRrecv irrecv(kRecvPin);
decode_results results;

const int TICK_DURATION = 50;  // 每個 tick 是 50 微秒

void setup() {
  Serial.begin(115200);
  delay(2000);  // 等待序列埠開啟
  irrecv.enableIRIn();
  Serial.println("🔍 準備顯示紅外線原始波形...");
}

void loop() {
  if (irrecv.decode(&results)) {
    delay(200);  // 增加延遲時間，讓接收器不會在同一時間接收到太多訊號
    if (results.rawlen >= 100) {
      Serial.println("✅ 成功接收訊號");
      
      // 顯示解碼結果
      Serial.println("🔍 原始數據（微秒）：");
      for (int i = 1; i < results.rawlen; i++) {
        Serial.print(results.rawbuf[i] * TICK_DURATION);
        Serial.print(" ");
      }
      Serial.println();
      
      // 使用 IRremoteESP8266 库的解碼功能，將原始數據轉換為可讀的格式
      Serial.println("🔍 解碼結果：");
      Serial.println(resultToHumanReadableBasic(&results));
    }
    irrecv.resume();  // 準備接收下一個訊號
  }
}
