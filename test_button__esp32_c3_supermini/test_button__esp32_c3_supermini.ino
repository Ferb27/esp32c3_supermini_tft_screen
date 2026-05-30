#define BTN_PIN 5

void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT_PULLUP);
  delay(500);
  Serial.println("Button test start");
}

void loop() {
  static int lastState = HIGH;
  int s = digitalRead(BTN_PIN);

  if (s != lastState) {
    if (s == LOW) Serial.println("BUTTON PRESSED");
    else          Serial.println("BUTTON RELEASED");
    lastState = s;
    delay(30); // debounce nhẹ
  }
}