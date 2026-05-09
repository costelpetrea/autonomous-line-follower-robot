#include <Arduino.h>
#include <QTRSensors.h>

// -------------------- QTR sensor pins --------------------
const uint8_t SENSOR_COUNT = 8;
const uint8_t QTR_PINS[SENSOR_COUNT] = {4, 5, 6, 7, 8, 10, 2, 1}; // left to right
const uint8_t QTR_LED_EN = 9;

QTRSensors qtr;

// -------------------- Buttons --------------------
const int BTN_CALIB = 0;
const int BTN_START = 42;

// -------------------- LEDs --------------------
const int LED_CALIB = 15;
const int LED_RUN = 18;

// -------------------- Motor driver pins --------------------
const int AIN1 = 35; // left motor
const int AIN2 = 36; // left motor
const int BIN1 = 37; // right motor
const int BIN2 = 38; // right motor
const int NSLEEP = 39; // motor driver enable

// -------------------- PWM settings --------------------
const int PWM_FREQ = 20000;
const int PWM_RESOLUTION = 8;
const int PWM_MAX = 255;

// -------------------- Motor minimum effective PWM --------------------
const int MOTOR_MIN_PWM = 140;

// -------------------- Motor direction correction --------------------
const bool INVERT_LEFT_MOTOR = true;
const bool INVERT_RIGHT_MOTOR = true;

// -------------------- Sensor variables --------------------
uint16_t qtrValues[SENSOR_COUNT];
int calValues[SENSOR_COUNT];

bool isCalibrated = false;
bool robotRunning = false;

// -------------------- Line follower settings --------------------
int baseSpeed = 255;
int maxTurn = 505;

float Kp = 0.27;
float Kd = 2.7;

// If robot turns away from the line, change this to -1
int lineDirection = 1;

// Used when line is lost
int lastError = 0;

// Used for derivative control
int previousError = 0;

// -------------------- Button memory --------------------
bool lastStartButtonState = HIGH;

// -------------------- Serial timing --------------------
unsigned long lastPrintTime = 0;
const unsigned long printInterval = 150;

// -------------------- QTR calibrated sensor reading --------------------
void calculateCalibratedValues() {
  qtr.readCalibrated(qtrValues);

  for (int i = 0; i < SENSOR_COUNT; i++) {
    calValues[i] = qtrValues[i] / 10;
    calValues[i] = constrain(calValues[i], 0, 100);
  }
}

// -------------------- Calibration with Pololu QTR library --------------------
void runCalibration() {
  Serial.println();
  Serial.println("QTR calibration started.");
  Serial.println("Move sensors over white and black.");
  Serial.println("Calibration runs for 6 seconds.");
  Serial.println();

  qtr.resetCalibration();

  unsigned long startTime = millis();

  while (millis() - startTime < 6000) {
    qtr.calibrate();

    digitalWrite(LED_CALIB, (millis() / 150) % 2);

    delay(10);
  }

  digitalWrite(LED_CALIB, HIGH);
  isCalibrated = true;

  previousError = 0;
  lastError = 0;

  Serial.println("QTR calibration finished.");
  Serial.println();
}

// -------------------- Position calculation --------------------
int calculateLinePosition() {
  long weightedSum = 0;
  long sum = 0;

  for (int i = 0; i < SENSOR_COUNT; i++) {
    weightedSum += (long)calValues[i] * (i * 1000);
    sum += calValues[i];
  }

  if (sum < 20) {
    return -1;
  }

  return weightedSum / sum;
}

// -------------------- Motor minimum PWM compensation --------------------
int applyMotorMinimumPWM(int pwm) {
  if (pwm > 0 && pwm < MOTOR_MIN_PWM) {
    return MOTOR_MIN_PWM;
  }

  if (pwm < 0 && pwm > -MOTOR_MIN_PWM) {
    return -MOTOR_MIN_PWM;
  }

  return pwm;
}

// -------------------- Motor function --------------------
void setMotors(int leftPWM, int rightPWM) {
  if (INVERT_LEFT_MOTOR == true) {
    leftPWM = -leftPWM;
  }

  if (INVERT_RIGHT_MOTOR == true) {
    rightPWM = -rightPWM;
  }

  leftPWM = applyMotorMinimumPWM(leftPWM);
  rightPWM = applyMotorMinimumPWM(rightPWM);

  leftPWM = constrain(leftPWM, -PWM_MAX, PWM_MAX);
  rightPWM = constrain(rightPWM, -PWM_MAX, PWM_MAX);

  // Left motor
  if (leftPWM > 0) {
    ledcWrite(AIN1, leftPWM);
    ledcWrite(AIN2, 0);
  } else if (leftPWM < 0) {
    ledcWrite(AIN1, 0);
    ledcWrite(AIN2, -leftPWM);
  } else {
    ledcWrite(AIN1, 0);
    ledcWrite(AIN2, 0);
  }

  // Right motor
  if (rightPWM > 0) {
    ledcWrite(BIN1, rightPWM);
    ledcWrite(BIN2, 0);
  } else if (rightPWM < 0) {
    ledcWrite(BIN1, 0);
    ledcWrite(BIN2, -rightPWM);
  } else {
    ledcWrite(BIN1, 0);
    ledcWrite(BIN2, 0);
  }
}

void stopMotors() {
  setMotors(0, 0);
}

// -------------------- Printing --------------------
void printCalibratedValues() {
  Serial.print("CAL: ");

  for (int i = 0; i < SENSOR_COUNT; i++) {
    Serial.print(calValues[i]);

    if (i < SENSOR_COUNT - 1) {
      Serial.print('\t');
    }
  }

  Serial.println();
}

// -------------------- Start button --------------------
void updateStartButton() {
  bool currentStartButtonState = digitalRead(BTN_START);

  if (lastStartButtonState == HIGH && currentStartButtonState == LOW) {
    delay(30);

    if (digitalRead(BTN_START) == LOW) {
      if (isCalibrated == true) {
        robotRunning = !robotRunning;

        if (robotRunning == true) {
          previousError = 0;
          lastError = 0;

          Serial.println("Robot STARTED.");
        } else {
          Serial.println("Robot STOPPED.");
          stopMotors();
        }
      } else {
        Serial.println("Calibrate first.");
      }
    }
  }

  lastStartButtonState = currentStartButtonState;
}

// -------------------- Simple PD line follower --------------------
void followLinePD() {
  calculateCalibratedValues();

  int position = calculateLinePosition();

  if (position == -1) {
    if (lastError < 0) {
      setMotors(-200, 225);
    } else {
      setMotors(225, -200);
    }

    previousError = lastError;

    if (millis() - lastPrintTime >= printInterval) {
      lastPrintTime = millis();

      Serial.print("Line lost. Searching. Last error: ");
      Serial.println(lastError);
    }

    return;
  }

  int error = position - 3500;
  int derivative = error - previousError;

  previousError = error;
  lastError = error;

  int turn = lineDirection * ((Kp * error) + (Kd * derivative));
  turn = constrain(turn, -maxTurn, maxTurn);

  int absError = abs(error);
  int currentBaseSpeed = baseSpeed;

  if (absError > 1200) {
    currentBaseSpeed = 145;
  } else if (absError > 600) {
    currentBaseSpeed = 180;
  }

int leftPWM = currentBaseSpeed + turn;
int rightPWM = currentBaseSpeed - turn;

  leftPWM = constrain(leftPWM, -PWM_MAX, PWM_MAX);
  rightPWM = constrain(rightPWM, -PWM_MAX, PWM_MAX);

  setMotors(leftPWM, rightPWM);

  if (millis() - lastPrintTime >= printInterval) {
    lastPrintTime = millis();

    printCalibratedValues();

    Serial.print("Position: ");
    Serial.print(position);

    Serial.print("    Error: ");
    Serial.print(error);

    Serial.print("    Derivative: ");
    Serial.print(derivative);

    Serial.print("    Turn: ");
    Serial.print(turn);

    Serial.print("    LeftPWM: ");
    Serial.print(leftPWM);

    Serial.print("    RightPWM: ");
    Serial.println(rightPWM);
  }
}

// -------------------- Setup --------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BTN_CALIB, INPUT_PULLUP);
  pinMode(BTN_START, INPUT_PULLUP);

  pinMode(LED_CALIB, OUTPUT);
  pinMode(LED_RUN, OUTPUT);

  digitalWrite(LED_CALIB, LOW);
  digitalWrite(LED_RUN, LOW);

  pinMode(NSLEEP, OUTPUT);
  digitalWrite(NSLEEP, HIGH);

  ledcAttach(AIN1, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(AIN2, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(BIN1, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(BIN2, PWM_FREQ, PWM_RESOLUTION);

  qtr.setTypeAnalog();
  qtr.setSensorPins(QTR_PINS, SENSOR_COUNT);
  qtr.setEmitterPin(QTR_LED_EN);

  for (int i = 0; i < SENSOR_COUNT; i++) {
    pinMode(QTR_PINS[i], INPUT);
    analogSetPinAttenuation(QTR_PINS[i], ADC_11db);
  }

  stopMotors();

  Serial.println("Simple PD line follower using Pololu QTR library calibration.");
  Serial.println("Only speed, Kp, Kd, maxTurn and minimum motor PWM are used for control.");
  Serial.println("Both motors positive = forward.");
  Serial.println("1. Press BTN_CALIB and calibrate.");
  Serial.println("2. Put robot on the line.");
  Serial.println("3. Press BTN_START.");
  Serial.println();
}

// -------------------- Loop --------------------
void loop() {
  if (digitalRead(BTN_CALIB) == LOW) {
    delay(30);

    if (digitalRead(BTN_CALIB) == LOW) {
      robotRunning = false;
      stopMotors();

      runCalibration();

      while (digitalRead(BTN_CALIB) == LOW) {
        delay(10);
      }
    }
  }

  updateStartButton();

  if (robotRunning == true && isCalibrated == true) {
    digitalWrite(LED_RUN, HIGH);
    followLinePD();
  } else {
    digitalWrite(LED_RUN, LOW);
    stopMotors();
  }
}