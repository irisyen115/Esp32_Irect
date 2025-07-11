#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>

const uint16_t kRecvPin = 12;  // 接收腳位
const uint16_t kSendPin = 5;   // 發射腳位
// const uint16_t kRawTick = 10;  // 原始單位：10 微秒

IRrecv irrecv(kRecvPin, 40000);
IRsend irsend(kSendPin);
decode_results results;

uint16_t* savedRawData = nullptr;  // 暫存學到的原始訊號
uint16_t savedRawLen = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);

  irrecv.enableIRIn();  // 啟用接收
  irsend.begin();       // 啟用發射

  Serial.println("🧠 請用紅外線遙控器發送一段訊號讓我學習...");
}

void loop() {
  // 如果還沒學到任何訊號，就進入學習模式
  if (savedRawData == nullptr) {
    if (irrecv.decode(&results)) {
      if (results.rawlen >= 100) {
        Serial.println("✅ 成功學到一段紅外線訊號！");
        savedRawLen = results.rawlen - 1;
        savedRawData = (uint16_t*)malloc(savedRawLen * sizeof(uint16_t));
        Serial.print("rawlen = ");
        Serial.println(results.rawlen);
        if (savedRawData == nullptr) {
          Serial.println("❌ 記憶體不足，無法儲存訊號！");
          irrecv.resume();
          return;
        }
        for (int i = 1; i < results.rawlen; i++) {
          savedRawData[i - 1] = results.rawbuf[i] * kRawTick;
        }

        Serial.println("📝 已儲存以下原始波形（微秒）：");
        for (int i = 0; i < savedRawLen; i++) {
          Serial.print(savedRawData[i]);
          Serial.print(" ");
        }
        Serial.println();

        irrecv.resume();
        delay(1000);
      } else {
        Serial.println("⚠️ 訊號太短，忽略");
        irrecv.resume();
      }
    }
    return;
  }

  // 🟢 已學習完畢 → 進入「發射 & 接收回顯」模式
  Serial.println("📡 正在發射剛學到的訊號...");
  irrecv.disableIRIn();        // 發射前先關閉接收器
  delay(200);                  // 保險等待
  irsend.sendRaw(savedRawData, savedRawLen, 38);  // 發射一次
  Serial.println("📡 發射完成");

  delay(500);  // 訊號傳播緩衝
  irrecv.enableIRIn();  // 重啟接收器
  Serial.println("📥 等待接收回應...");

  unsigned long startTime = millis();
  bool received = false;

  while (millis() - startTime < 3000) {
    if (irrecv.decode(&results)) {
      Serial.println("✅ 成功接收到發射的訊號！");
      Serial.print("rawlen = ");
      Serial.println(results.rawlen);

      Serial.println("🔍 原始接收數據（微秒）：");
      for (int i = 1; i < results.rawlen; i++) {
        Serial.print(results.rawbuf[i] * kRawTick);
        Serial.print(" ");
      }
      Serial.println();

      Serial.println("🔍 解碼結果：");
      Serial.println(resultToHumanReadableBasic(&results));

      received = true;
      irrecv.resume();
      break;
    }
  }

  if (!received) {
    Serial.println("❌ 沒有接收到剛才發射的訊號。可能角度/距離不對？");
  }

  Serial.println("🔁 5 秒後將再次發射...");
  delay(5000);  // 每 5 秒循環一次
}
