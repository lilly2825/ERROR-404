#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

// ======================================================
// TRUSTGUARD
// FC3: SENSOR HISTORY + REPUTATION + DECISION ENGINE
// ======================================================


// ======================================================
// LCD
// ======================================================

LiquidCrystal_I2C lcd(0x27, 16, 2);


// ======================================================
// PIN DEFINITIONS
// ======================================================

// Sensors
#define POT_PIN       34
#define PIR_PIN       26
#define DOOR_PIN      27
#define ATTACK_PIN    25
#define TAMPER_PIN    33

// LEDs
#define GREEN_LED     18
#define YELLOW_LED    19
#define RED_LED       23

// Buzzer
#define BUZZER_PIN     5

// Servo
#define SERVO_PIN     13

// I2C
#define SDA_PIN       21
#define SCL_PIN       22


// ======================================================
// UNIQUE DEVICE IDs
// ======================================================

#define POT_ID        1
#define PIR_ID        2
#define DOOR_ID       3
#define ATTACK_ID     4
#define TAMPER_ID     5


// ======================================================
// CONTROLLER ID
// ======================================================

const char* CONTROLLER_ID = "CTRL-001";


// ======================================================
// HISTORY CONFIGURATION
// ======================================================

#define HISTORY_SIZE 10


// ======================================================
// SENSOR HISTORY ARRAYS
// ======================================================

// POT
bool potHistory[HISTORY_SIZE];

// PIR
bool pirHistory[HISTORY_SIZE];

// DOOR
bool doorHistory[HISTORY_SIZE];

// ATTACK
bool attackHistory[HISTORY_SIZE];

// TAMPER
bool tamperHistory[HISTORY_SIZE];


// ======================================================
// HISTORY INDEX
// ======================================================

int potIndex    = 0;
int pirIndex    = 0;
int doorIndex   = 0;
int attackIndex = 0;
int tamperIndex = 0;


// ======================================================
// HISTORY COUNTERS
// ======================================================

int potCount    = 0;
int pirCount    = 0;
int doorCount   = 0;
int attackCount = 0;
int tamperCount = 0;


// ======================================================
// SERVO
// ======================================================

Servo doorServo;

const int LOCK_ANGLE   = 0;
const int UNLOCK_ANGLE = 90;


// ======================================================
// SECURITY STATES
// ======================================================

enum SecurityState {

  STATE_SAFE,
  STATE_SUSPICIOUS,
  STATE_CRITICAL,
  STATE_ATTACK,
  STATE_TAMPER

};


// ======================================================
// GLOBAL CURRENT SECURITY STATE
// ======================================================

SecurityState currentSecurityState = STATE_SAFE;


// ======================================================
// HISTORY FUNCTIONS
// ======================================================


// ------------------------------------------------------
// ADD READING TO HISTORY
// ------------------------------------------------------

void addHistory(
  bool history[],
  int &index,
  int &count,
  bool value
) {

  history[index] = value;

  index++;

  if (index >= HISTORY_SIZE) {

    index = 0;

  }

  if (count < HISTORY_SIZE) {

    count++;

  }

}


// ------------------------------------------------------
// CALCULATE STABILITY / REPUTATION
// ------------------------------------------------------
//
// This is a prototype reputation score.
//
// It measures how stable the recent readings are.
// It does NOT claim cryptographic trust.
//
// ------------------------------------------------------

int calculateReputation(
  bool history[],
  int count
) {

  if (count <= 1) {

    return 100;

  }


  int transitions = 0;


  for (int i = 1; i < count; i++) {

    if (history[i] != history[i - 1]) {

      transitions++;

    }

  }


  int reputation =
    100 - ((transitions * 100) / (count - 1));


  reputation = constrain(
    reputation,
    0,
    100
  );


  return reputation;

}


// ======================================================
// CONVERT SECURITY STATE TO TEXT
// ======================================================

String securityStateText(
  SecurityState state
) {

  switch (state) {

    case STATE_SAFE:
      return "SAFE";

    case STATE_SUSPICIOUS:
      return "SUSPICIOUS";

    case STATE_CRITICAL:
      return "CRITICAL";

    case STATE_ATTACK:
      return "ATTACK";

    case STATE_TAMPER:
      return "TAMPER";

  }

  return "UNKNOWN";

}


// ======================================================
// FUTURE CARD EXTENSION POINT
// ======================================================
//
// These functions currently return TRUE.
//
// Later Future Cards can be inserted here:
//
// FC4 -> Freshness / Replay protection
// FC5 -> Authentication / Challenge-response
// FC6 -> Advanced Trust Engine
//
// We don't need to rewrite the main decision engine.
//
// ======================================================


// ------------------------------------------------------
// FUTURE CARD: FRESHNESS
// ------------------------------------------------------

bool freshnessCheck() {

  // Future Card #4 will be implemented here.

  return true;

}


// ------------------------------------------------------
// FUTURE CARD: DEVICE AUTHENTICATION
// ------------------------------------------------------

bool authenticationCheck() {

  // Future Card #5 will be implemented here.

  return true;

}


// ------------------------------------------------------
// FUTURE CARD: ADVANCED TRUST
// ------------------------------------------------------

bool advancedTrustCheck() {

  // Future Card #6 will be implemented here.

  return true;

}


// ======================================================
// DECISION ENGINE
// ======================================================

SecurityState makeSecurityDecision(

  bool motionDetected,

  String personState,

  bool doorOpen,

  bool attackMode,

  bool tamperDetected,

  int pirReputation,

  int potReputation,

  int doorReputation,

  int attackReputation,

  int tamperReputation

) {


  // ====================================================
  // PRIORITY 1
  // TAMPER
  // ====================================================

  if (tamperDetected) {

    return STATE_TAMPER;

  }


  // ====================================================
  // PRIORITY 2
  // ATTACK SWITCH
  // ====================================================

  if (attackMode) {

    return STATE_ATTACK;

  }


  // ====================================================
  // FUTURE CARD CHECKS
  // ====================================================

  bool fresh = freshnessCheck();

  bool authenticated =
    authenticationCheck();

  bool advancedTrust =
    advancedTrustCheck();


  // ----------------------------------------------------
  // If future security verification fails,
  // treat the evidence as suspicious.
  // ----------------------------------------------------

  if (!fresh ||
      !authenticated ||
      !advancedTrust) {

    return STATE_SUSPICIOUS;

  }


  // ====================================================
  // CRITICAL CURRENT CONDITION
  // ====================================================

  bool criticalCondition =

    motionDetected &&

    personState == "HIGH" &&

    doorOpen;


  if (criticalCondition) {

    return STATE_CRITICAL;

  }


  // ====================================================
  // REPUTATION-AWARE DECISION
  // ====================================================
  //
  // Reputation is currently used as a supporting signal.
  //
  // We don't allow history alone to override a real
  // physical security event.
  //
  // ====================================================


  bool weakPIR =
    pirReputation < 40;

  bool weakPOT =
    potReputation < 40;

  bool weakDoor =
    doorReputation < 40;


  // ----------------------------------------------------
  // Suspicious if current evidence exists
  // ----------------------------------------------------

  if (
    motionDetected ||
    personState == "MID" ||
    doorOpen
  ) {

    return STATE_SUSPICIOUS;

  }


  // ----------------------------------------------------
  // Suspicious if sensor history is unstable
  // ----------------------------------------------------

  if (
    weakPIR ||
    weakPOT ||
    weakDoor
  ) {

    return STATE_SUSPICIOUS;

  }


  // ====================================================
  // SAFE
  // ====================================================

  return STATE_SAFE;

}


// ======================================================
// SETUP
// ======================================================

void setup() {

  Serial.begin(115200);


  // ====================================================
  // I2C
  // ====================================================

  Wire.begin(
    SDA_PIN,
    SCL_PIN
  );


  // ====================================================
  // INPUTS
  // ====================================================

  pinMode(
    PIR_PIN,
    INPUT
  );

  pinMode(
    DOOR_PIN,
    INPUT_PULLUP
  );

  pinMode(
    ATTACK_PIN,
    INPUT_PULLUP
  );

  pinMode(
    TAMPER_PIN,
    INPUT_PULLUP
  );


  // ====================================================
  // OUTPUTS
  // ====================================================

  pinMode(
    GREEN_LED,
    OUTPUT
  );

  pinMode(
    YELLOW_LED,
    OUTPUT
  );

  pinMode(
    RED_LED,
    OUTPUT
  );

  pinMode(
    BUZZER_PIN,
    OUTPUT
  );


  // ====================================================
  // OUTPUT RESET
  // ====================================================

  digitalWrite(
    GREEN_LED,
    LOW
  );

  digitalWrite(
    YELLOW_LED,
    LOW
  );

  digitalWrite(
    RED_LED,
    LOW
  );

  digitalWrite(
    BUZZER_PIN,
    LOW
  );


  // ====================================================
  // SERVO
  // ====================================================

  doorServo.setPeriodHertz(50);

  doorServo.attach(
    SERVO_PIN,
    500,
    2400
  );

  doorServo.write(
    LOCK_ANGLE
  );


  // ====================================================
  // LCD
  // ====================================================

  lcd.init();

  lcd.backlight();

  lcd.clear();

  lcd.setCursor(
    0,
    0
  );

  lcd.print(
    "TRUSTGUARD"
  );

  lcd.setCursor(
    0,
    1
  );

  lcd.print(
    "STARTING..."
  );


  // ====================================================
  // SERIAL STARTUP
  // ====================================================

  Serial.println();
  Serial.println("========================================");
  Serial.println("          TRUSTGUARD SYSTEM");
  Serial.println("========================================");

  Serial.println();
  Serial.println("DEVICE IDENTITIES");
  Serial.println("----------------------------------------");

  Serial.print("ID ");
  Serial.print(POT_ID);
  Serial.println(" -> POT-001");

  Serial.print("ID ");
  Serial.print(PIR_ID);
  Serial.println(" -> PIR-002");

  Serial.print("ID ");
  Serial.print(DOOR_ID);
  Serial.println(" -> DOOR-003");

  Serial.print("ID ");
  Serial.print(ATTACK_ID);
  Serial.println(" -> ATTACK-004");

  Serial.print("ID ");
  Serial.print(TAMPER_ID);
  Serial.println(" -> TAMPER-005");

  Serial.println("----------------------------------------");

  Serial.print("CONTROLLER -> ");
  Serial.println(CONTROLLER_ID);

  Serial.println();
  Serial.println("FC3 HISTORY ENGINE READY");
  Serial.println("PIR stabilizing...");
  Serial.println();


  // ====================================================
  // PIR STABILIZATION
  // ====================================================

  delay(60000);


  // ====================================================
  // INITIALIZE HISTORY
  // ====================================================

  bool initialPIR =
    digitalRead(PIR_PIN);

  bool initialDoor =
    digitalRead(DOOR_PIN) == HIGH;

  bool initialAttack =
    digitalRead(ATTACK_PIN) == LOW;

  bool initialTamper =
    digitalRead(TAMPER_PIN) == LOW;


  int initialPot =
    analogRead(POT_PIN);


  bool initialPotState =
    initialPot > 2702;


  for (int i = 0; i < HISTORY_SIZE; i++) {

    potHistory[i] =
      initialPotState;

    pirHistory[i] =
      initialPIR;

    doorHistory[i] =
      initialDoor;

    attackHistory[i] =
      initialAttack;

    tamperHistory[i] =
      initialTamper;

  }


  potCount =
    HISTORY_SIZE;

  pirCount =
    HISTORY_SIZE;

  doorCount =
    HISTORY_SIZE;

  attackCount =
    HISTORY_SIZE;

  tamperCount =
    HISTORY_SIZE;


  // ====================================================
  // SYSTEM READY
  // ====================================================

  lcd.clear();

  lcd.setCursor(
    0,
    0
  );

  lcd.print(
    "TRUSTGUARD"
  );

  lcd.setCursor(
    0,
    1
  );

  lcd.print(
    "SYSTEM READY"
  );


  Serial.println("SYSTEM READY");
  Serial.println();

  delay(2000);

}


// ======================================================
// MAIN LOOP
// ======================================================

void loop() {


  // ====================================================
  // READ POTENTIOMETER
  // ====================================================

  int potValue =
    analogRead(POT_PIN);


  int confidence =
    map(
      potValue,
      0,
      4095,
      0,
      100
    );


  confidence =
    constrain(
      confidence,
      0,
      100
    );


  String personState;


  if (potValue <= 1351) {

    personState =
      "LOW";

  }

  else if (potValue <= 2702) {

    personState =
      "MID";

  }

  else {

    personState =
      "HIGH";

  }


  // ====================================================
  // READ PIR
  // ====================================================

  bool motionDetected =
    digitalRead(PIR_PIN);


  // ====================================================
  // READ DOOR
  // ====================================================

  bool doorInput =
    digitalRead(DOOR_PIN);


  bool doorOpen =
    doorInput == HIGH;


  String doorState;


  if (doorOpen) {

    doorState =
      "OPEN";

  }

  else {

    doorState =
      "CLOSED";

  }


  // ====================================================
  // READ ATTACK
  // ====================================================

  bool attackMode =
    digitalRead(ATTACK_PIN) == LOW;


  // ====================================================
  // READ TAMPER
  // ====================================================

  bool tamperDetected =
    digitalRead(TAMPER_PIN) == LOW;


  // ====================================================
  // CONVERT READINGS FOR HISTORY
  // ====================================================

  bool potHistoryState =
    potValue > 2702;


  // ====================================================
  // UPDATE HISTORIES
  // ====================================================

  addHistory(
    potHistory,
    potIndex,
    potCount,
    potHistoryState
  );


  addHistory(
    pirHistory,
    pirIndex,
    pirCount,
    motionDetected
  );


  addHistory(
    doorHistory,
    doorIndex,
    doorCount,
    doorOpen
  );


  addHistory(
    attackHistory,
    attackIndex,
    attackCount,
    attackMode
  );


  addHistory(
    tamperHistory,
    tamperIndex,
    tamperCount,
    tamperDetected
  );


  // ====================================================
  // CALCULATE REPUTATIONS
  // ====================================================

  int potReputation =
    calculateReputation(
      potHistory,
      potCount
    );


  int pirReputation =
    calculateReputation(
      pirHistory,
      pirCount
    );


  int doorReputation =
    calculateReputation(
      doorHistory,
      doorCount
    );


  int attackReputation =
    calculateReputation(
      attackHistory,
      attackCount
    );


  int tamperReputation =
    calculateReputation(
      tamperHistory,
      tamperCount
    );


  // ====================================================
  // DECISION ENGINE
  // ====================================================

  currentSecurityState =
    makeSecurityDecision(

      motionDetected,

      personState,

      doorOpen,

      attackMode,

      tamperDetected,

      pirReputation,

      potReputation,

      doorReputation,

      attackReputation,

      tamperReputation

    );


  String securityState =
    securityStateText(
      currentSecurityState
    );


  // ====================================================
  // RESET OUTPUTS
  // ====================================================

  digitalWrite(
    GREEN_LED,
    LOW
  );

  digitalWrite(
    YELLOW_LED,
    LOW
  );

  digitalWrite(
    RED_LED,
    LOW
  );

  digitalWrite(
    BUZZER_PIN,
    LOW
  );


  // ====================================================
  // SECURITY RESPONSE
  // ====================================================


  // ----------------------------------------------------
  // SAFE
  // ----------------------------------------------------

  if (
    currentSecurityState ==
    STATE_SAFE
  ) {

    digitalWrite(
      GREEN_LED,
      HIGH
    );

    doorServo.write(
      UNLOCK_ANGLE
    );

  }


  // ----------------------------------------------------
  // SUSPICIOUS
  // ----------------------------------------------------

  else if (
    currentSecurityState ==
    STATE_SUSPICIOUS
  ) {

    digitalWrite(
      YELLOW_LED,
      HIGH
    );

    doorServo.write(
      LOCK_ANGLE
    );

    digitalWrite(
      BUZZER_PIN,
      HIGH
    );

    delay(100);

    digitalWrite(
      BUZZER_PIN,
      LOW
    );

  }


  // ----------------------------------------------------
  // CRITICAL
  // ----------------------------------------------------

  else if (
    currentSecurityState ==
    STATE_CRITICAL
  ) {

    digitalWrite(
      RED_LED,
      HIGH
    );

    doorServo.write(
      LOCK_ANGLE
    );

    digitalWrite(
      BUZZER_PIN,
      HIGH
    );

  }


  // ----------------------------------------------------
  // ATTACK
  // ----------------------------------------------------

  else if (
    currentSecurityState ==
    STATE_ATTACK
  ) {

    digitalWrite(
      RED_LED,
      HIGH
    );

    doorServo.write(
      LOCK_ANGLE
    );

    digitalWrite(
      BUZZER_PIN,
      HIGH
    );

  }


  // ----------------------------------------------------
  // TAMPER
  // ----------------------------------------------------

  else if (
    currentSecurityState ==
    STATE_TAMPER
  ) {

    digitalWrite(
      RED_LED,
      HIGH
    );

    doorServo.write(
      LOCK_ANGLE
    );

    digitalWrite(
      BUZZER_PIN,
      HIGH
    );

  }


  // ====================================================
  // SERIAL MONITOR
  // ====================================================

  Serial.println();
  Serial.println("========================================");
  Serial.println("       TRUSTGUARD DECISION ENGINE");
  Serial.println("========================================");


  // ----------------------------------------------------
  // CONTROLLER
  // ----------------------------------------------------

  Serial.print("CONTROLLER       : ");
  Serial.println(CONTROLLER_ID);


  // ----------------------------------------------------
  // POT
  // ----------------------------------------------------

  Serial.print("[ID ");
  Serial.print(POT_ID);
  Serial.println("] POT-001");

  Serial.print("ADC              : ");
  Serial.println(potValue);

  Serial.print("CONFIDENCE       : ");
  Serial.print(confidence);
  Serial.println("%");

  Serial.print("PERSON STATE     : ");
  Serial.println(personState);

  Serial.print("HISTORY          : ");
  Serial.print(potCount);
  Serial.println(" readings");

  Serial.print("REPUTATION       : ");
  Serial.print(potReputation);
  Serial.println("%");


  // ----------------------------------------------------
  // PIR
  // ----------------------------------------------------

  Serial.println();

  Serial.print("[ID ");
  Serial.print(PIR_ID);
  Serial.println("] PIR-002");

  Serial.print("MOTION           : ");

  if (motionDetected)
    Serial.println("DETECTED");
  else
    Serial.println("NO MOTION");

  Serial.print("HISTORY          : ");
  Serial.print(pirCount);
  Serial.println(" readings");

  Serial.print("REPUTATION       : ");
  Serial.print(pirReputation);
  Serial.println("%");


  // ----------------------------------------------------
  // DOOR
  // ----------------------------------------------------

  Serial.println();

  Serial.print("[ID ");
  Serial.print(DOOR_ID);
  Serial.println("] DOOR-003");

  Serial.print("STATE            : ");
  Serial.println(doorState);

  Serial.print("HISTORY          : ");
  Serial.print(doorCount);
  Serial.println(" readings");

  Serial.print("REPUTATION       : ");
  Serial.print(doorReputation);
  Serial.println("%");


  // ----------------------------------------------------
  // ATTACK
  // ----------------------------------------------------

  Serial.println();

  Serial.print("[ID ");
  Serial.print(ATTACK_ID);
  Serial.println("] ATTACK-004");

  Serial.print("STATE            : ");

  if (attackMode)
    Serial.println("ON");
  else
    Serial.println("OFF");

  Serial.print("HISTORY          : ");
  Serial.print(attackCount);
  Serial.println(" readings");

  Serial.print("REPUTATION       : ");
  Serial.print(attackReputation);
  Serial.println("%");


  // ----------------------------------------------------
  // TAMPER
  // ----------------------------------------------------

  Serial.println();

  Serial.print("[ID ");
  Serial.print(TAMPER_ID);
  Serial.println("] TAMPER-005");

  Serial.print("STATE            : ");

  if (tamperDetected)
    Serial.println("DETECTED");
  else
    Serial.println("NORMAL");

  Serial.print("HISTORY          : ");
  Serial.print(tamperCount);
  Serial.println(" readings");

  Serial.print("REPUTATION       : ");
  Serial.print(tamperReputation);
  Serial.println("%");


  // ====================================================
  // DECISION
  // ====================================================

  Serial.println();
  Serial.println("----------------------------------------");

  Serial.print("DECISION         : ");
  Serial.println(securityState);

  Serial.print("LOCK STATUS      : ");


  if (
    currentSecurityState ==
    STATE_SAFE
  ) {

    Serial.print("UNLOCKED (");
    Serial.print(UNLOCK_ANGLE);
    Serial.println(" deg)");

  }

  else {

    Serial.print("LOCKED (");
    Serial.print(LOCK_ANGLE);
    Serial.println(" deg)");

  }


  // ====================================================
  // OUTPUT STATUS
  // ====================================================

  Serial.print("GREEN LED        : ");

  if (
    currentSecurityState ==
    STATE_SAFE
  )
    Serial.println("ON");
  else
    Serial.println("OFF");


  Serial.print("YELLOW LED       : ");

  if (
    currentSecurityState ==
    STATE_SUSPICIOUS
  )
    Serial.println("ON");
  else
    Serial.println("OFF");


  Serial.print("RED LED           : ");

  if (
    currentSecurityState ==
    STATE_CRITICAL ||
    currentSecurityState ==
    STATE_ATTACK ||
    currentSecurityState ==
    STATE_TAMPER
  )
    Serial.println("ON");
  else
    Serial.println("OFF");


  Serial.print("BUZZER           : ");

  if (
    currentSecurityState ==
    STATE_CRITICAL ||
    currentSecurityState ==
    STATE_ATTACK ||
    currentSecurityState ==
    STATE_TAMPER
  )
    Serial.println("ON");
  else
    Serial.println("OFF");


  Serial.println("----------------------------------------");


  // ====================================================
  // LCD
  // ====================================================

  lcd.clear();


  lcd.setCursor(
    0,
    0
  );


  if (
    currentSecurityState ==
    STATE_SAFE
  ) {

    lcd.print("SAFE");

  }

  else if (
    currentSecurityState ==
    STATE_SUSPICIOUS
  ) {

    lcd.print("WARNING");

  }

  else if (
    currentSecurityState ==
    STATE_CRITICAL
  ) {

    lcd.print("CRITICAL");

  }

  else if (
    currentSecurityState ==
    STATE_ATTACK
  ) {

    lcd.print("ATTACK");

  }

  else if (
    currentSecurityState ==
    STATE_TAMPER
  ) {

    lcd.print("TAMPER");

  }


  lcd.setCursor(
    0,
    1
  );


  lcd.print("P:");
  lcd.print(personState);

  lcd.print(" M:");


  if (motionDetected)
    lcd.print("Y");
  else
    lcd.print("N");


  lcd.print(" D:");


  if (doorOpen)
    lcd.print("O");
  else
    lcd.print("C");


  delay(500);
}