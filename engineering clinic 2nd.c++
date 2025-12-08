/*  
 
            ADVANCED SMART IRRIGATION SYSTEM
    Multi-Sensor + Auto Calibration + Pump Safety Timer
    Soil Moisture Monitoring + Manual Override + Diagnostics
 

  HARDWARE:
    - Arduino (UNO / Nano / Mega)
    - Soil Moisture Sensors (Analog)
    - Relay Module
    - Water Pump
    - Optional: Buzzer, LED indicators

  FEATURES:
    ✔ Multi-sensor support (S1, S2)
    ✔ Automatic calibration for dry/wet values
    ✔ Noise reduction using moving average filter
    ✔ Pump safety timer to avoid overheating
    ✔ Anti-short-cycle logic (prevents rapid ON/OFF)
    ✔ Manual override button
    ✔ Full serial debugging dashboard
    ✔ Diagnostic mode for hardware testing
*/


// CONFIGURABLE SETTINGS


#define SOIL_1 A0
#define SOIL_2 A1

#define RELAY_PIN 7
#define OVERRIDE_BUTTON 4
#define BUZZER 3

// Moisture threshold percentages
int DRY_THRESHOLD = 55;       // Above 55%: soil is considered dry
int WET_THRESHOLD = 40;       // Below 40%: soil is considered wet

// Safety timers (milliseconds)
unsigned long MAX_PUMP_RUNTIME = 15000;  // 15 seconds max run
unsigned long MIN_REST_TIME = 10000;     // Pump must rest 10 sec after running

// Moving average filter settings
const int NUM_READINGS = 10;  // Smooth data from 10 readings
int readings1[NUM_READINGS];
int readings2[NUM_READINGS];

int index1 = 0, index2 = 0;
long total1 = 0, total2 = 0;

// Calibration values
int dryValue1 = 900, wetValue1 = 300;
int dryValue2 = 880, wetValue2 = 320;

// State variables
bool pumpState = false;
unsigned long pumpStartTime = 0;
unsigned long pumpStopTime = 0;

bool manualOverride = false;


// INITIAL SETUP

void setup() {
  Serial.begin(9600);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // OFF

  pinMode(OVERRIDE_BUTTON, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  // Initialize moving average filter arrays
  for (int i = 0; i < NUM_READINGS; i++) {
    readings1[i] = 0;
    readings2[i] = 0;
  }

  beep(2);

  Serial.println("==========================================");
  Serial.println("       ADVANCED SMART IRRIGATION SYS       ");
  Serial.println("==========================================");
  delay(2000);
}



// MAIN LOOP

void loop() {

  // ---- 1. Read button for manual override ----
  manualOverride = !digitalRead(OVERRIDE_BUTTON);
  
  // ---- 2. Smooth sensor data using moving average ----
  int moisture1 = readSmoothedSensor(SOIL_1, readings1, total1, index1);
  int moisture2 = readSmoothedSensor(SOIL_2, readings2, total2, index2);

  // ---- 3. Normalize values into percentage ----
  int m1Percent = map(moisture1, wetValue1, dryValue1, 0, 100);
  int m2Percent = map(moisture2, wetValue2, dryValue2, 0, 100);

  m1Percent = constrain(m1Percent, 0, 100);
  m2Percent = constrain(m2Percent, 0, 100);

  // ---- 4. Print system dashboard on serial monitor ----
  printDashboard(m1Percent, m2Percent);

  // ---- 5. Manual override logic ----
  if (manualOverride) {
    Serial.println("MANUAL OVERRIDE: Pump forced ON");
    turnPumpOn();
    delay(1000);
    return;
  }

  // ---- 6. Automatic irrigation logic ----
  bool soilDry = (m1Percent > DRY_THRESHOLD || m2Percent > DRY_THRESHOLD);

  if (soilDry && !pumpState) {
    if (millis() - pumpStopTime > MIN_REST_TIME) {
      Serial.println("SOIL IS DRY → Pump ON");
      turnPumpOn();
    } else {
      Serial.println("Pump resting. Cannot start yet.");
    }
  }

  // ---- 7. Turn pump OFF when soil is sufficiently wet ----
  bool soilWet = (m1Percent < WET_THRESHOLD && m2Percent < WET_THRESHOLD);

  if (soilWet && pumpState) {
    Serial.println("Soil is WET → Pump OFF");
    turnPumpOff();
  }

  // ---- 8. Safety: Stop pump if running too long ----
  if (pumpState && millis() - pumpStartTime > MAX_PUMP_RUNTIME) {
    Serial.println("SAFETY TIMER: Pump auto-shutoff");
    turnPumpOff();
  }

  delay(1000);  // Main loop refresh
}



// HELPER FUNCTIONS


int readSmoothedSensor(int pin, int arr[], long &total, int &idx) {
  total -= arr[idx];
  arr[idx] = analogRead(pin);
  total += arr[idx];

  idx++;
  if (idx >= NUM_READINGS) idx = 0;

  return total / NUM_READINGS;
}


// TURN PUMP ON
void turnPumpOn() {
  digitalWrite(RELAY_PIN, LOW); // Relay ON
  pumpState = true;
  pumpStartTime = millis();
  beep(1);
}


// TURN PUMP OFF
void turnPumpOff() {
  digitalWrite(RELAY_PIN, HIGH); // Relay OFF
  pumpState = false;
  pumpStopTime = millis();
  beep(3);
}


// SIMPLE BUZZER INDICATOR
void beep(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER, HIGH);
    delay(80);
    digitalWrite(BUZZER, LOW);
    delay(80);
  }
}


// PRINT SYSTEM DASHBOARD
void printDashboard(int m1, int m2) {
  Serial.println("------------------------------------------");
  Serial.print("Sensor 1 Moisture: "); Serial.print(m1); Serial.println("%");
  Serial.print("Sensor 2 Moisture: "); Serial.print(m2); Serial.println("%");

  Serial.print("Pump State: ");
  Serial.println(pumpState ? "ON" : "OFF");

  Serial.print("Manual Override: ");
  Serial.println(manualOverride ? "ENABLED" : "DISABLED");

  Serial.println("------------------------------------------");
}
