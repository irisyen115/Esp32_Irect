#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>

const uint16_t kRecvPin = 12;    // IR 接收腳位
const uint16_t kIrLedPin = 13;    // IR 發射腳位
const uint16_t kButtonNext = 14; // 按鍵切換訊號
const uint16_t kButtonSend = 4; // 按鍵發射訊號

const uint16_t kCaptureBufferSize = 2048;
const uint8_t kTimeout = 50;
const uint32_t kFrequency = 38000;
const int kMaxSignals = 5;       // 最多儲存幾組訊號

IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
IRsend irsend(kIrLedPin);
decode_results results;

struct IRSignal {
  uint16_t rawbuf[1024]; // 儲存 raw 訊號 (max 1024)
  uint16_t rawlen;       // 長度
  bool valid;            // 是否有訊號
};

IRSignal learnedSignals[kMaxSignals];
int currentSignalIndex = 0;

unsigned long lastButtonNextPress = 0;
unsigned long lastButtonSendPress = 0;
const unsigned long debounceDelay = 200;

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(kButtonNext, INPUT_PULLUP);
  pinMode(kButtonSend, INPUT_PULLUP);

  irrecv.enableIRIn();
  irsend.begin();

  // 初始化所有訊號為無效
  for (int i = 0; i < kMaxSignals; i++) {
    learnedSignals[i].valid = false;
    learnedSignals[i].rawlen = 0;
  }

  Serial.println("🔍 系統啟動，等待紅外線訊號...");
  Serial.println("📌 按 GPIO14 切換訊號，GPIO13 發射當前選擇訊號");
}

void loop() {
  // 接收 IR 訊號
  if (irrecv.decode(&results)) {
    delay(100); // 避免雜訊

    if (results.rawlen >= 100) {
      Serial.print("✅ 接收成功，長度: ");
      Serial.println(results.rawlen);

      // 儲存到目前選擇的訊號槽
      IRSignal &sig = learnedSignals[currentSignalIndex];
      sig.rawlen = results.rawlen - 1;
      // 複製 rawbuf (略過第0個占位)
      for (int i = 1; i < results.rawlen; i++) {
        sig.rawbuf[i - 1] = results.rawbuf[i];
      }
      sig.valid = true;

      Serial.print("📡 訊號已儲存於槽 ");
      Serial.println(currentSignalIndex);

      // 印出 raw 時間
      Serial.print("🔍 RAW data: ");
      for (int i = 0; i < sig.rawlen; i++) {
        Serial.print(sig.rawbuf[i] * kRawTick);
        Serial.print(" ");
      }
      Serial.println();
    }
    irrecv.resume();
  }

  // 按鍵讀取
  if (digitalRead(kButtonNext) == LOW) {
    if (millis() - lastButtonNextPress > debounceDelay) {
      // 切換訊號槽
      currentSignalIndex++;
      if (currentSignalIndex >= kMaxSignals) currentSignalIndex = 0;

      Serial.print("🔄 切換到訊號槽：");
      Serial.println(currentSignalIndex);

      if (!learnedSignals[currentSignalIndex].valid) {
        Serial.println("⚠️ 該槽尚未學習任何訊號");
      }
      lastButtonNextPress = millis();
    }
  }

  if (digitalRead(kButtonSend) == LOW) {
    if (millis() - lastButtonSendPress > debounceDelay) {
      IRSignal &sig = learnedSignals[currentSignalIndex];
      if (sig.valid) {
        Serial.print("📡 發射訊號槽 ");
        Serial.println(currentSignalIndex);
        irsend.sendRaw(sig.rawbuf, sig.rawlen, kFrequency);
        Serial.println("✅ 發射完成");
      } else {
        Serial.println("❌ 該槽無有效訊號，請先學習");
      }
      lastButtonSendPress = millis();
    }
  }
}
