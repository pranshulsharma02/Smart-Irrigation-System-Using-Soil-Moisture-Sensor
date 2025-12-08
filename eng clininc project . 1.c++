// Smart Irrigation - Basic (Arduino Uno)
// Soil moisture analog -> A0
// Relay control -> D8
// Calibrate dry and wet values for your sensor

const int sensorPin = A0;
const int relayPin = 8;

int dryValue = 800;   // calibrate: value when soil is dry
int wetValue = 300;   // calibrate: value when soil is fully wet
int moisturePercent = 0;

const int ON_THRESHOLD = 30;   // percent: start watering if below this
const int OFF_THRESHOLD = 45;  // percent: stop watering when above this
const unsigned long WATER_DURATION_MS = 20UL * 1000UL; // 20 seconds default watering
const unsigned long WAIT_AFTER_WATER_MS = 60UL * 60UL * 1000UL; // 1 hour cooldown

unsigned long lastWaterMillis = 0;
bool watering = false;

void setup() {
  Serial.begin(9600);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH); // assuming HIGH = relay OFF for active LOW modules
}

void loop() {
  int raw = analogRead(sensorPin);
  moisturePercent = map(raw, dryValue, wetValue, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);

  Serial.print("Raw: ");
  Serial.print(raw);
  Serial.print(" -> Moisture%: ");
  Serial.println(moisturePercent);

  unsigned long now = millis();

  if (!watering) {
    if (moisturePercent < ON_THRESHOLD && (now - lastWaterMillis > WAIT_AFTER_WATER_MS)) {
      // Start watering
      Serial.println("Starting pump...");
      digitalWrite(relayPin, LOW); // activate relay (active LOW)
      watering = true;
      lastWaterMillis = now;
    }
  } else {
    // Currently watering - either stop after duration or when moisture reaches OFF_THRESHOLD
    if (moisturePercent >= OFF_THRESHOLD || (now - lastWaterMillis >= WATER_DURATION_MS)) {
      Serial.println("Stopping pump...");
      digitalWrite(relayPin, HIGH); // deactivate relay
      watering = false;
      lastWaterMillis = now;
    }
  }

  delay(2000); // read every 2s
}
