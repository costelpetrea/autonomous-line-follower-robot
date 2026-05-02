const int NUM_SENSORS = 8;
const int QTR_LED_EN = 9;
const int BTN_CALIB = 0;
const int BEC_15 = 15;
const int BEC_18 = 18;

const int sensorPins[8] = {4, 5, 6, 7, 8, 10, 2, 1};
int sensorMin[8];
int sensorMax[8];
int sensorCalibrated[8];
unsigned long ultimaApasare = 0;

int readSensor(int pin) {
int val = 0;
for (int i = 0; i < 4; i++) {
val += analogReadMilliVolts(pin);
}
return val >> 2;
}

void calibrate() {
for (int i = 0; i < NUM_SENSORS; i++) {
sensorMin[i] = 4095;
sensorMax[i] = 0;
}

unsigned long t = millis();
while (millis() - t < 5000) {
if ((millis() / 150) % 2 == 0) {
analogWrite(BEC_15, 255);
} else {
analogWrite(BEC_15, 0);
}

for (int i = 0; i < NUM_SENSORS; i++) {
  int v = readSensor(sensorPins[i]);
  if (v < sensorMin[i]) sensorMin[i] = v;
  if (v > sensorMax[i]) sensorMax[i] = v;
}
}
analogWrite(BEC_15, 0);
}

void setup() {
Serial.begin(2000000);
analogReadResolution(12);

for (int i = 0; i < NUM_SENSORS; i++) {
analogSetPinAttenuation(sensorPins[i], ADC_11db);
}

pinMode(QTR_LED_EN, OUTPUT);
digitalWrite(QTR_LED_EN, HIGH);
pinMode(BTN_CALIB, INPUT_PULLUP);
pinMode(BEC_15, OUTPUT);
pinMode(BEC_18, OUTPUT);

calibrate();
}

void loop() {
if (digitalRead(BTN_CALIB) == LOW) {
if (millis() - ultimaApasare > 500) {
calibrate();
ultimaApasare = millis();
}
}

long sum = 0;
int active = 0;

for (int i = 0; i < NUM_SENSORS; i++) {
sensorCalibrated[i] = constrain(map(readSensor(sensorPins[i]), sensorMin[i], sensorMax[i], 0, 100), 0, 100);
int val = sensorCalibrated[i];
sum += val * (i * 1000);
active += val;
}

int position = 3500;
if (active > 50) {
position = sum / active;
}

int error = position - 3500;
int pwmStang = 0;
int pwmDrept = 0;

if (error < 0) {
pwmStang = map(-error, 0, 3500, 0, 255);
} else if (error > 0) {
pwmDrept = map(error, 0, 3500, 0, 255);
}

analogWrite(BEC_18, pwmStang);
analogWrite(BEC_15, pwmDrept);

for (int i = 0; i < NUM_SENSORS; i++) {
Serial.print(sensorCalibrated[i]);
Serial.print(" ");
}
Serial.print("Poz");
Serial.print(" ");
Serial.println(position);
}