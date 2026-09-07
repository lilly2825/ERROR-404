#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

// ==============================
// LCD
// ==============================

LiquidCrystal_I2C lcd(0x27, 16, 2);


// ==============================
// PIN DEFINITIONS
// ==============================

#define POT_PIN     34
#define PIR_PIN     26
#define DOOR_PIN    27
#define ATTACK_PIN  25

#define GREEN_LED   18
#define YELLOW_LED  19
#define RED_LED     23

#define BUZZER_PIN  5
#define SERVO_PIN   13


// ==============================
// SERVO
// ==============================

Servo doorServo;

const int LOCK_ANGLE   = 0;
const int UNLOCK_ANGLE = 90;


// ==============================
// SETUP
// ==============================

void setup() {

  Serial.begin(115200);

  // I2C
  Wire.begin(21, 22);

  // Inputs
  pinMode(PIR_PIN, INPUT);
  pinMode(DOOR_PIN, INPUT_PULLUP);
  pinMode(ATTACK_PIN, INPUT_PULLUP);

  // Outputs
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Turn outputs OFF
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER_PIN, LOW);


  // ==============================
  // SERVO INITIALIZATION
  // ==============================

  doorServo.setPeriodHertz(50);
  doorServo.attach(SERVO_PIN, 500, 2400);

  // Start LOCKED
  doorServo.write(LOCK_ANGLE);


  // ==============================
  // LCD
  // ==============================

  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("TRUSTGUARD");

  lcd.setCursor(0, 1);
  lcd.print("STARTING...");


  // ==============================
  // SERIAL STARTUP
  // ==============================

  Serial.println();
  Serial.println("========================================");
  Serial.println("          TRUSTGUARD SYSTEM");
  Serial.println("========================================");
  Serial.println("PIR stabilizing...");
  Serial.println();


  // PIR stabilization
  delay(60000);


  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TRUSTGUARD");

  lcd.setCursor(0, 1);
  lcd.print("SYSTEM READY");


  Serial.println("SYSTEM READY");
  Serial.println();

  delay(2000);
}


// ==============================
// MAIN LOOP
// ==============================

void loop() {

  // ==============================
  // READ POTENTIOMETER
  // ==============================

  int potValue = analogRead(POT_PIN);

  int confidence = map(
    potValue,
    0,
    4095,
    0,
    100
  );

  confidence = constrain(
    confidence,
    0,
    100
  );


  String personState;

  if (potValue <= 1351) {

    personState = "LOW";

  }
  else if (potValue <= 2702) {

    personState = "MID";

  }
  else {

    personState = "HIGH";
  }


  // ==============================
  // READ PIR
  // ==============================

  bool motionDetected = digitalRead(PIR_PIN);


  // ==============================
  // READ DOOR SWITCH
  // ==============================

  bool doorInput = digitalRead(DOOR_PIN);

  String doorState;

  if (doorInput == HIGH) {

    doorState = "OPEN";

  }
  else {

    doorState = "CLOSED";
  }


  // ==============================
  // READ ATTACK SWITCH
  // ==============================

  bool attackMode = (digitalRead(ATTACK_PIN) == LOW);


  // ==============================
  // SECURITY DECISION
  // ==============================

  String securityState;


  if (attackMode) {

    securityState = "ATTACK";

  }

  else if (
    motionDetected &&
    personState == "HIGH" &&
    doorInput == HIGH
  ) {

    securityState = "CRITICAL";

  }

  else if (
    motionDetected ||
    personState == "MID" ||
    doorInput == HIGH
  ) {

    securityState = "SUSPICIOUS";

  }

  else {

    securityState = "SAFE";
  }


  // ==============================
  // RESET OUTPUTS
  // ==============================

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER_PIN, LOW);


  // ==============================
  // SERVO + SECURITY RESPONSE
  // ==============================

  if (securityState == "SAFE") {

    digitalWrite(GREEN_LED, HIGH);

    // UNLOCK
    doorServo.write(UNLOCK_ANGLE);
  }


  else if (securityState == "SUSPICIOUS") {

    digitalWrite(YELLOW_LED, HIGH);

    // LOCK
    doorServo.write(LOCK_ANGLE);

    // Short warning beep
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
  }


  else if (securityState == "CRITICAL") {

    digitalWrite(RED_LED, HIGH);

    // LOCK
    doorServo.write(LOCK_ANGLE);

    // Alarm
    digitalWrite(BUZZER_PIN, HIGH);
  }


  else if (securityState == "ATTACK") {

    digitalWrite(RED_LED, HIGH);

    // LOCK
    doorServo.write(LOCK_ANGLE);

    // Alarm
    digitalWrite(BUZZER_PIN, HIGH);
  }


  // ==============================
  // SERIAL MONITOR
  // ==============================

  Serial.println();
  Serial.println("----------------------------------------");
  Serial.println("         TRUSTGUARD STATUS");
  Serial.println("----------------------------------------");

  // Potentiometer
  Serial.print("POT ADC          : ");
  Serial.println(potValue);

  // Confidence
  Serial.print("CONFIDENCE       : ");
  Serial.print(confidence);
  Serial.println("%");

  // Person
  Serial.print("PERSON STATE     : ");
  Serial.println(personState);

  // Motion
  Serial.print("MOTION STATUS    : ");

  if (motionDetected) {
    Serial.println("DETECTED");
  }
  else {
    Serial.println("NO MOTION");
  }

  // ==============================
  // DOOR STATUS
  // ==============================

  Serial.print("DOOR STATUS      : ");
  Serial.println(doorState);

  // Attack switch
  Serial.print("ATTACK SWITCH    : ");

  if (attackMode) {
    Serial.println("ON");
  }
  else {
    Serial.println("OFF");
  }

  // Security decision
  Serial.print("SECURITY STATE   : ");
  Serial.println(securityState);

  // ==============================
  // SERVO / LOCK STATUS
  // ==============================

  Serial.print("LOCK STATUS      : ");

  if (securityState == "SAFE") {

    Serial.print("UNLOCKED");

    Serial.print(" (");
    Serial.print(UNLOCK_ANGLE);
    Serial.println(" deg)");

  }
  else {

    Serial.print("LOCKED");

    Serial.print(" (");
    Serial.print(LOCK_ANGLE);
    Serial.println(" deg)");
  }

  // ==============================
  // LED STATUS
  // ==============================

  Serial.print("GREEN LED        : ");

  if (securityState == "SAFE")
    Serial.println("ON");
  else
    Serial.println("OFF");


  Serial.print("YELLOW LED       : ");

  if (securityState == "SUSPICIOUS")
    Serial.println("ON");
  else
    Serial.println("OFF");


  Serial.print("RED LED          : ");

  if (
    securityState == "CRITICAL" ||
    securityState == "ATTACK"
  )
    Serial.println("ON");
  else
    Serial.println("OFF");


  // ==============================
  // BUZZER STATUS
  // ==============================

  Serial.print("BUZZER           : ");

  if (
    securityState == "CRITICAL" ||
    securityState == "ATTACK"
  )
    Serial.println("ON");
  else
    Serial.println("OFF");


  Serial.println("----------------------------------------");


  // ==============================
  // LCD
  // ==============================

  lcd.clear();

  lcd.setCursor(0, 0);

  if (securityState == "SAFE") {

    lcd.print("SAFE");

  }

  else if (securityState == "SUSPICIOUS") {

    lcd.print("WARNING");

  }

  else {

    lcd.print("ALERT");

  }


  lcd.setCursor(0, 1);

  lcd.print("P:");
  lcd.print(personState);

  lcd.print(" M:");

  if (motionDetected)
    lcd.print("Y");
  else
    lcd.print("N");

  lcd.print(" D:");

  if (doorInput == HIGH)
    lcd.print("O");
  else
    lcd.print("C");


  delay(500);
}