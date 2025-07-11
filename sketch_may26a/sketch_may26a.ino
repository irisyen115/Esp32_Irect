#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

const uint16_t kRecvPin = 12;
IRrecv irrecv(kRecvPin, 40000);
decode_results results;

void setup() {
  Serial.begin(115200);
  delay(2000);
  irrecv.enableIRIn();
  Serial.println("開始接收冷氣遙控訊號...");
}

void loop() {
  if (irrecv.decode(&results)) {
    Serial.println("收到紅外線訊號!");
    Serial.print("rawlen = ");
    Serial.println(results.rawlen);
    Serial.print("repeat = ");
    Serial.println(results.repeat);
    Serial.print("code = ");
    Serial.println(results.value, HEX);
    irrecv.resume();
  }
  delay(100); // 避免刷太快
}
