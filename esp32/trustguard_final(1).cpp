#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

// ======================================================
// TRUSTGUARD
// CYBER-PHYSICAL SECURITY DECISION SYSTEM
// ======================================================
//
// FEATURES
// ------------------------------------------------------
// 1. Multi-sensor cyber-physical monitoring
// 2. Individual sensor risk scores
// 3. Overall weighted risk
// 4. Overall trust score
// 5. Rule-based decision engine
// 6. Decision evidence explanation
// 7. Circular web gauges
// 8. LED status visualization
// 9. Persistent event history
// 10. Decision transition logging
// ======================================================


// ======================================================
// LCD
// ======================================================

LiquidCrystal_I2C lcd(0x27, 16, 2);


// ======================================================
// WIFI ACCESS POINT
// ======================================================

const char* AP_SSID = "TRUSTGUARD_AP";
const char* AP_PASSWORD = "12345678";

WebServer server(80);


// ======================================================
// PERSISTENT STORAGE
// ======================================================

Preferences historyPrefs;


// ======================================================
// PIN DEFINITIONS
// ======================================================

#define POT_PIN       34
#define PIR_PIN       26
#define DOOR_PIN      27
#define ATTACK_PIN    25
#define TAMPER_PIN    33

#define GREEN_LED     18
#define YELLOW_LED    19
#define RED_LED       23

#define BUZZER_PIN    5
#define SERVO_PIN     13


// ======================================================
// SENSOR IDS
// ======================================================

const char* POT_ID    = "POT-001";
const char* PIR_ID    = "PIR-002";
const char* DOOR_ID   = "DOOR-003";
const char* ATTACK_ID = "ATTACK-004";
const char* TAMPER_ID = "TAMPER-005";


// ======================================================
// SERVO
// ======================================================

Servo doorServo;

const int LOCK_ANGLE   = 0;
const int UNLOCK_ANGLE = 90;


// ======================================================
// SENSOR VALUES
// ======================================================

int potValue = 0;
int confidence = 0;

String personState = "LOW";

bool motionDetected = false;
bool doorOpen = false;
bool attackMode = false;
bool tamperDetected = false;

String doorState = "CLOSED";

String securityState = "SAFE";

String servoState = "LOCKED";

bool buzzerState = false;


// ======================================================
// CYBER-PHYSICAL RISK SCORES
// ======================================================

int personRisk = 0;
int motionRisk = 0;
int doorRisk = 0;
int attackRisk = 0;
int tamperRisk = 0;

int overallRisk = 0;

int trustScore = 100;

int activeSignals = 0;


// ======================================================
// DECISION ENGINE INFORMATION
// ======================================================

String decisionRule =
  "RULE 01: NORMAL SENSOR CONSENSUS -> SAFE";

String decisionReason =
  "No elevated physical or cyber threat evidence is present.";

String evidence1 =
  "PIR-002: no movement";

String evidence2 =
  "DOOR-003: access path CLOSED";

String evidence3 =
  "POT-001: LOW person signal";


// ======================================================
// EVENT HISTORY
// ======================================================

const int MAX_HISTORY = 20;

String eventHistory[MAX_HISTORY];

int historyCount = 0;

unsigned long eventNumber = 0;


// ------------------------------------------------------
// Previous decision signature
// ------------------------------------------------------

String previousDecisionSignature = "";


// ======================================================
// CREATE DECISION SIGNATURE
// ======================================================

String createDecisionSignature() {

  String signature = "";

  signature += securityState;
  signature += "|";

  signature += personState;
  signature += "|";

  signature += motionDetected ? "M1" : "M0";
  signature += "|";

  signature += doorOpen ? "D1" : "D0";
  signature += "|";

  signature += attackMode ? "A1" : "A0";
  signature += "|";

  signature += tamperDetected ? "T1" : "T0";
  signature += "|";

  signature += decisionRule;

  return signature;
}


// ======================================================
// SAVE HISTORY TO ESP32 NON-VOLATILE MEMORY
// ======================================================

void saveHistory() {

  historyPrefs.begin("tg_history", false);

  historyPrefs.putUInt(
    "eventNo",
    eventNumber
  );

  historyPrefs.putUInt(
    "count",
    historyCount
  );


  for (
    int i = 0;
    i < MAX_HISTORY;
    i++
  ) {

    String key = "h" + String(i);

    if (i < historyCount) {

      historyPrefs.putString(
        key.c_str(),
        eventHistory[i]
      );

    }

    else {

      historyPrefs.remove(
        key.c_str()
      );

    }

  }


  historyPrefs.end();
}


// ======================================================
// LOAD HISTORY FROM ESP32 NON-VOLATILE MEMORY
// ======================================================

void loadHistory() {

  historyPrefs.begin(
    "tg_history",
    true
  );


  eventNumber =
    historyPrefs.getUInt(
      "eventNo",
      0
    );


  historyCount =
    historyPrefs.getUInt(
      "count",
      0
    );


  if (
    historyCount > MAX_HISTORY
  ) {

    historyCount =
      MAX_HISTORY;

  }


  for (
    int i = 0;
    i < historyCount;
    i++
  ) {

    String key =
      "h" + String(i);

    eventHistory[i] =
      historyPrefs.getString(
        key.c_str(),
        ""
      );

  }


  historyPrefs.end();
}


// ======================================================
// ADD EVENT
// ======================================================

void addEvent(
  String eventText
) {

  eventNumber++;


  String entry =
    "#" +
    String(eventNumber) +
    "  " +
    eventText;


  if (
    historyCount < MAX_HISTORY
  ) {

    historyCount++;

  }


  // Shift old events

  for (
    int i = MAX_HISTORY - 1;
    i > 0;
    i--
  ) {

    eventHistory[i] =
      eventHistory[i - 1];

  }


  // Newest event at top

  eventHistory[0] =
    entry;


  // Save immediately

  saveHistory();
}


// ======================================================
// LOG CURRENT DECISION
// ======================================================

void logDecisionEvent() {

  String eventText = "";


  eventText +=
    securityState;

  eventText +=
    " | ";

  eventText +=
    decisionRule;

  eventText +=
    " | TRUST ";

  eventText +=
    String(trustScore);

  eventText +=
    "% | RISK ";

  eventText +=
    String(overallRisk);

  eventText +=
    "%";


  addEvent(
    eventText
  );
}


// ======================================================
// CALCULATE SENSOR RISK
// ======================================================

void calculateRisk() {

  // ====================================================
  // PERSON SENSOR
  // ====================================================

  if (
    personState == "LOW"
  ) {

    personRisk = 0;

  }

  else if (
    personState == "MID"
  ) {

    personRisk = 45;

  }

  else {

    personRisk = 80;

  }


  // ====================================================
  // MOTION
  // ====================================================

  if (
    motionDetected
  ) {

    motionRisk = 70;

  }

  else {

    motionRisk = 0;

  }


  // ====================================================
  // DOOR
  // ====================================================

  if (
    doorOpen
  ) {

    doorRisk = 75;

  }

  else {

    doorRisk = 0;

  }


  // ====================================================
  // ATTACK
  // ====================================================

  if (
    attackMode
  ) {

    attackRisk = 100;

  }

  else {

    attackRisk = 0;

  }


  // ====================================================
  // TAMPER
  // ====================================================

  if (
    tamperDetected
  ) {

    tamperRisk = 100;

  }

  else {

    tamperRisk = 0;

  }


  // ====================================================
  // WEIGHTED CYBER-PHYSICAL RISK
  // ====================================================
  //
  // PERSON  = 20%
  // MOTION  = 20%
  // DOOR    = 20%
  // ATTACK  = 25%
  // TAMPER  = 15%
  //
  // TOTAL   = 100%
  // ====================================================

  float weightedRisk =

    (personRisk * 0.20) +

    (motionRisk * 0.20) +

    (doorRisk * 0.20) +

    (attackRisk * 0.25) +

    (tamperRisk * 0.15);


  overallRisk =
    constrain(
      (int)weightedRisk,
      0,
      100
    );


  // ====================================================
  // TRUST SCORE
  // ====================================================

  trustScore =
    100 - overallRisk;


  // ====================================================
  // HARD SECURITY EVIDENCE
  // ====================================================
  //
  // Explicit attack/tamper evidence overrides
  // weighted scoring.
  // ====================================================

  if (
    attackMode
  ) {

    overallRisk = 100;

    trustScore = 0;

  }

  else if (
    tamperDetected
  ) {

    overallRisk = 100;

    trustScore = 0;

  }

  else if (
    motionDetected &&
    personState == "HIGH" &&
    doorOpen
  ) {

    overallRisk = 90;

    trustScore = 10;

  }


  trustScore =
    constrain(
      trustScore,
      0,
      100
    );


  // ====================================================
  // ACTIVE EVIDENCE COUNT
  // ====================================================

  activeSignals = 0;


  if (
    personRisk > 0
  ) {

    activeSignals++;

  }


  if (
    motionDetected
  ) {

    activeSignals++;

  }


  if (
    doorOpen
  ) {

    activeSignals++;

  }


  if (
    attackMode
  ) {

    activeSignals++;

  }


  if (
    tamperDetected
  ) {

    activeSignals++;

  }

}


// ======================================================
// BUILD DECISION EVIDENCE
// ======================================================

void buildDecisionEvidence() {

  // ====================================================
  // DEFAULT EVIDENCE
  // ====================================================

  evidence1 =
    motionDetected
    ?
    "PIR-002: MOTION DETECTED"
    :
    "PIR-002: no movement";


  evidence2 =
    doorOpen
    ?
    "DOOR-003: access path OPEN"
    :
    "DOOR-003: access path CLOSED";


  if (
    personState == "HIGH"
  ) {

    evidence3 =
      "POT-001: HIGH person signal";

  }

  else if (
    personState == "MID"
  ) {

    evidence3 =
      "POT-001: MID person signal";

  }

  else {

    evidence3 =
      "POT-001: LOW person signal";

  }


  // ====================================================
  // TAMPER
  // ====================================================

  if (
    tamperDetected
  ) {

    decisionRule =
      "RULE 05: DEVICE INTEGRITY VIOLATION -> TAMPER";


    decisionReason =
      "TAMPER-005 reports physical interference. Device integrity is no longer trusted.";


    evidence1 =
      "TAMPER-005: physical integrity anomaly";


    evidence2 =
      "Device trust has been reduced";


    evidence3 =
      "Protective response: LOCK + ALARM";


    return;
  }


  // ====================================================
  // ATTACK
  // ====================================================

  if (
    attackMode
  ) {

    decisionRule =
      "RULE 04: EXPLICIT ATTACK SIGNAL -> ATTACK";


    decisionReason =
      "ATTACK-004 is active. Explicit threat evidence overrides normal sensor trust.";


    evidence1 =
      "ATTACK-004: threat signal ACTIVE";


    evidence2 =
      "High-confidence cyber security evidence";


    evidence3 =
      "Protective response: LOCK + ALARM";


    return;
  }


  // ====================================================
  // CRITICAL MULTI-SENSOR CORRELATION
  // ====================================================

  if (
    motionDetected &&
    personState == "HIGH" &&
    doorOpen
  ) {

    decisionRule =
      "RULE 03: MOTION + HIGH PERSON + OPEN DOOR -> CRITICAL";


    decisionReason =
      "Three independent physical indicators correlate: movement, high presence and an open access path.";


    evidence1 =
      "PIR-002: MOTION DETECTED";


    evidence2 =
      "POT-001: HIGH person signal";


    evidence3 =
      "DOOR-003: OPEN access path";


    return;
  }


  // ====================================================
  // SUSPICIOUS
  // ====================================================

  if (
    motionDetected ||
    personState == "MID" ||
    doorOpen
  ) {

    decisionRule =
      "RULE 02: ELEVATED PHYSICAL EVIDENCE -> SUSPICIOUS";


    if (
      doorOpen &&
      motionDetected
    ) {

      decisionReason =
        "Motion and an open access path were observed together. Verification is required.";


    }

    else if (
      doorOpen
    ) {

      decisionReason =
        "The monitored physical access path is open, creating an abnormal access condition.";


    }

    else if (
      motionDetected
    ) {

      decisionReason =
        "Physical movement was detected by PIR-002 and requires verification.";


    }

    else {

      decisionReason =
        "POT-001 reports an elevated person signal that requires verification.";

    }


    return;
  }


  // ====================================================
  // SAFE
  // ====================================================

  decisionRule =
    "RULE 01: NORMAL SENSOR CONSENSUS -> SAFE";


  decisionReason =
    "No elevated physical or cyber threat evidence is present.";


  evidence1 =
    "PIR-002: no movement";


  evidence2 =
    "DOOR-003: access path CLOSED";


  evidence3 =
    "POT-001: LOW person signal";

}


// ======================================================
// DETERMINE SECURITY STATE
// ======================================================

void determineSecurityState() {

  if (
    tamperDetected
  ) {

    securityState =
      "TAMPER";

  }

  else if (
    attackMode
  ) {

    securityState =
      "ATTACK";

  }

  else if (
    motionDetected &&
    personState == "HIGH" &&
    doorOpen
  ) {

    securityState =
      "CRITICAL";

  }

  else if (
    motionDetected ||
    personState == "MID" ||
    doorOpen
  ) {

    securityState =
      "SUSPICIOUS";

  }

  else {

    securityState =
      "SAFE";

  }

}


// ======================================================
// UPDATE SECURITY SYSTEM
// ======================================================

void updateSecuritySystem() {

  // Calculate risk

  calculateRisk();


  // Determine state

  determineSecurityState();


  // Build explanation

  buildDecisionEvidence();


  // ====================================================
  // DECISION HISTORY LOGGING
  // ====================================================

  String currentSignature =
    createDecisionSignature();


  if (
    previousDecisionSignature == ""
  ) {

    // First decision after startup

    previousDecisionSignature =
      currentSignature;

  }

  else if (
    currentSignature !=
    previousDecisionSignature
  ) {

    logDecisionEvent();


    previousDecisionSignature =
      currentSignature;

  }

}


// ======================================================
// APPLY PHYSICAL SECURITY RESPONSE
// ======================================================

void applySecurityResponse() {

  // ====================================================
  // RESET
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


  buzzerState =
    false;


  // ====================================================
  // SAFE
  // ====================================================

  if (
    securityState == "SAFE"
  ) {

    digitalWrite(
      GREEN_LED,
      HIGH
    );


    doorServo.write(
      UNLOCK_ANGLE
    );


    servoState =
      "UNLOCKED";

  }


  // ====================================================
  // SUSPICIOUS
  // ====================================================

  else if (
    securityState == "SUSPICIOUS"
  ) {

    digitalWrite(
      YELLOW_LED,
      HIGH
    );


    doorServo.write(
      LOCK_ANGLE
    );


    servoState =
      "LOCKED";


    // Short warning beep

    digitalWrite(
      BUZZER_PIN,
      HIGH
    );


    buzzerState =
      true;


    delay(80);


    digitalWrite(
      BUZZER_PIN,
      LOW
    );


    buzzerState =
      false;

  }


  // ====================================================
  // CRITICAL / ATTACK / TAMPER
  // ====================================================

  else {

    digitalWrite(
      RED_LED,
      HIGH
    );


    doorServo.write(
      LOCK_ANGLE
    );


    servoState =
      "LOCKED";


    digitalWrite(
      BUZZER_PIN,
      HIGH
    );


    buzzerState =
      true;

  }

}


// ======================================================
// HTML DASHBOARD
// ======================================================

const char MAIN_PAGE[] PROGMEM = R"rawliteral(

<!DOCTYPE html>

<html>

<head>

<meta charset="UTF-8">

<meta name="viewport"
content="width=device-width, initial-scale=1">

<title>TRUSTGUARD</title>


<style>

/* =====================================================
   GLOBAL
   ===================================================== */

* {

  box-sizing:
    border-box;

}


html {

  scroll-behavior:
    smooth;

}


body {

  margin: 0;

  font-family:
    Arial,
    Helvetica,
    sans-serif;

  color:
    #eaf7ff;

  min-height:
    100vh;

  overflow-x:
    hidden;

  background:

    radial-gradient(
      circle at 10% 10%,
      rgba(
        0,
        205,
        255,
        0.16
      ),
      transparent 25%
    ),

    radial-gradient(
      circle at 90% 15%,
      rgba(
        100,
        70,
        255,
        0.18
      ),
      transparent 28%
    ),

    radial-gradient(
      circle at 50% 90%,
      rgba(
        0,
        255,
        180,
        0.10
      ),
      transparent 30%
    ),

    linear-gradient(
      135deg,
      #030810,
      #07121e,
      #03070d
    );

}


/* =====================================================
   CYBER GRID
   ===================================================== */

body::before {

  content: "";

  position:
    fixed;

  inset:
    0;

  pointer-events:
    none;

  opacity:
    0.22;

  background-image:

    linear-gradient(
      rgba(
        70,
        180,
        255,
        0.08
      )
      1px,
      transparent 1px
    ),

    linear-gradient(
      90deg,
      rgba(
        70,
        180,
        255,
        0.08
      )
      1px,
      transparent 1px
    );

  background-size:
    45px 45px;

}


/* =====================================================
   HEADER
   ===================================================== */

.header {

  text-align:
    center;

  padding:
    28px 15px;

  background:

    linear-gradient(
      135deg,
      rgba(
        5,
        19,
        32,
        0.97
      ),
      rgba(
        4,
        10,
        20,
        0.97
      )
    );

  border-bottom:
    1px solid
    rgba(
      74,
      200,
      255,
      0.35
    );

  box-shadow:
    0 10px 45px
    rgba(
      0,
      160,
      255,
      0.12
    );

}


.header h1 {

  margin:
    0;

  font-size:
    38px;

  letter-spacing:
    7px;

  color:
    #62dcff;

  text-shadow:

    0 0 8px
    rgba(
      70,
      210,
      255,
      0.95
    ),

    0 0 25px
    rgba(
      70,
      210,
      255,
      0.55
    );

}


.header p {

  margin:
    8px 0 0;

  font-size:
    13px;

  letter-spacing:
    3px;

  color:
    #8faec5;

}


/* =====================================================
   CONTAINER
   ===================================================== */

.container {

  max-width:
    1250px;

  margin:
    auto;

  padding:
    22px;

}


/* =====================================================
   STATUS / TRUST AREA
   ===================================================== */

.top-panel {

  display:
    grid;

  grid-template-columns:
    300px 1fr;

  gap:
    22px;

  margin-bottom:
    20px;

}


@media(
  max-width: 800px
) {

  .top-panel {

    grid-template-columns:
      1fr;

  }

}


/* =====================================================
   TRUST SCORE CARD
   ===================================================== */

.trust-card {

  min-height:
    285px;

  display:
    flex;

  flex-direction:
    column;

  align-items:
    center;

  justify-content:
    center;

  border-radius:
    20px;

  background:
    linear-gradient(
      145deg,
      rgba(
        10,
        27,
        42,
        0.92
      ),
      rgba(
        5,
        13,
        23,
        0.96
      )
    );

  border:
    1px solid
    rgba(
      70,
      190,
      255,
      0.25
    );

  box-shadow:
    0 15px 50px
    rgba(
      0,
      0,
      0,
      0.28
    );

}


.trust-title {

  color:
    #70d9ff;

  font-size:
    13px;

  letter-spacing:
    3px;

  margin-bottom:
    15px;

}


/* =====================================================
   CIRCULAR TRUST GAUGE
   ===================================================== */

.gauge {

  --score:
    0;

  --gaugeColor:
    #45ef91;

  width:
    175px;

  height:
    175px;

  border-radius:
    50%;

  display:
    flex;

  align-items:
    center;

  justify-content:
    center;

  position:
    relative;

  background:

    conic-gradient(
      var(--gaugeColor)
      calc(
        var(--score) * 1%
      ),
      rgba(
        40,
        60,
        75,
        0.35
      )
      0
    );

  box-shadow:

    0 0 20px
    color-mix(
      in srgb,
      var(--gaugeColor)
      30%,
      transparent
    );

}


.gauge::before {

  content:
    "";

  position:
    absolute;

  width:
    135px;

  height:
    135px;

  border-radius:
    50%;

  background:
    #06101b;

  box-shadow:
    inset
    0 0 25px
    rgba(
      0,
      0,
      0,
      0.7
    );

}


.gauge-content {

  position:
    relative;

  z-index:
    2;

  text-align:
    center;

}


.gauge-number {

  font-size:
    40px;

  font-weight:
    bold;

  color:
    #ffffff;

  text-shadow:
    0 0 12px
    rgba(
      100,
      220,
      255,
      0.5
    );

}


.gauge-label {

  margin-top:
    2px;

  font-size:
    10px;

  letter-spacing:
    2px;

  color:
    #6eabc7;

}


/* =====================================================
   MAIN SECURITY STATUS
   ===================================================== */

.status {

  min-height:
    285px;

  padding:
    28px;

  border-radius:
    20px;

  background:
    linear-gradient(
      145deg,
      rgba(
        10,
        24,
        39,
        0.93
      ),
      rgba(
        5,
        13,
        23,
        0.96
      )
    );

  border:
    1px solid
    rgba(
      80,
      170,
      220,
      0.2
    );

  display:
    flex;

  flex-direction:
    column;

  justify-content:
    center;

  transition:
    0.3s ease;

}


.status h2 {

  margin:
    0;

  font-size:
    40px;

  letter-spacing:
    4px;

}


.status p {

  margin:
    8px 0 20px;

  color:
    #9bb8cc;

  font-size:
    14px;

}


.status.safe {

  border-color:
    rgba(
      55,
      235,
      140,
      0.75
    );

  box-shadow:
    0 0 35px
    rgba(
      55,
      235,
      140,
      0.12
    );

}


.status.safe h2 {

  color:
    #45ef91;

  text-shadow:
    0 0 18px
    rgba(
      69,
      239,
      145,
      0.65
    );

}


.status.warning {

  border-color:
    rgba(
      255,
      211,
      60,
      0.8
    );

  box-shadow:
    0 0 35px
    rgba(
      255,
      211,
      60,
      0.12
    );

}


.status.warning h2 {

  color:
    #ffd33d;

  text-shadow:
    0 0 18px
    rgba(
      255,
      211,
      60,
      0.65
    );

}


.status.alert {

  border-color:
    rgba(
      255,
      65,
      90,
      0.85
    );

  box-shadow:
    0 0 40px
    rgba(
      255,
      65,
      90,
      0.15
    );

}


.status.alert h2 {

  color:
    #ff4f63;

  text-shadow:
    0 0 20px
    rgba(
      255,
      65,
      90,
      0.7
    );

}


/* =====================================================
   DECISION PIPELINE
   ===================================================== */

.pipeline {

  display:
    flex;

  align-items:
    center;

  gap:
    8px;

  flex-wrap:
    wrap;

}


.pipeline-box {

  padding:
    10px 13px;

  border-radius:
    9px;

  background:
    rgba(
      40,
      100,
      140,
      0.13
    );

  border:
    1px solid
    rgba(
      70,
      190,
      255,
      0.28
    );

  color:
    #8fe0ff;

  font-size:
    11px;

  letter-spacing:
    1px;

}


.pipeline-arrow {

  color:
    #4bcfff;

  font-size:
    18px;

}


/* =====================================================
   SENSOR GRID
   ===================================================== */

.grid {

  display:
    grid;

  grid-template-columns:
    repeat(
      auto-fit,
      minmax(
        215px,
        1fr
      )
    );

  gap:
    16px;

}


/* =====================================================
   SENSOR CARD
   ===================================================== */

.card {

  position:
    relative;

  padding:
    18px;

  min-height:
    250px;

  border-radius:
    17px;

  background:
    linear-gradient(
      145deg,
      rgba(
        18,
        35,
        51,
        0.92
      ),
      rgba(
        7,
        16,
        27,
        0.95
      )
    );

  border:
    1px solid
    rgba(
      90,
      160,
      200,
      0.2
    );

  box-shadow:
    0 10px 35px
    rgba(
      0,
      0,
      0,
      0.24
    );

  transition:
    0.25s ease;

}


.card:hover {

  transform:
    translateY(
      -4px
    );

  border-color:
    rgba(
      75,
      205,
      255,
      0.45
    );

  box-shadow:
    0 12px 45px
    rgba(
      0,
      190,
      255,
      0.10
    );

}


.card h3 {

  margin:
    0;

  color:
    #7cddff;

  font-size:
    14px;

  letter-spacing:
    1px;

}


.id {

  margin-top:
    5px;

  color:
    #58758a;

  font-size:
    10px;

  letter-spacing:
    1px;

}


/* =====================================================
   SMALL SENSOR GAUGE
   ===================================================== */

.sensor-gauge {

  --score:
    0;

  --gaugeColor:
    #45ef91;

  width:
    118px;

  height:
    118px;

  border-radius:
    50%;

  margin:
    14px auto 12px;

  display:
    flex;

  align-items:
    center;

  justify-content:
    center;

  background:

    conic-gradient(
      var(--gaugeColor)
      calc(
        var(--score) * 1%
      ),
      #152635
      0
    );

  box-shadow:
    0 0 18px
    rgba(
      0,
      200,
      255,
      0.06
    );

}


.sensor-gauge::before {

  content:
    "";

  position:
    absolute;

  width:
    88px;

  height:
    88px;

  border-radius:
    50%;

  background:
    #07111c;

}


.sensor-gauge-content {

  position:
    relative;

  z-index:
    2;

  text-align:
    center;

}


.sensor-risk {

  font-size:
    24px;

  font-weight:
    bold;

  color:
    #ffffff;

}


.sensor-risk-label {

  font-size:
    8px;

  letter-spacing:
    1px;

  color:
    #7293a8;

}


/* =====================================================
   SENSOR VALUE
   ===================================================== */

.value {

  text-align:
    center;

  font-size:
    20px;

  font-weight:
    bold;

  color:
    #eefaff;

}


.small {

  text-align:
    center;

  font-size:
    11px;

  color:
    #7896a8;

  margin-top:
    5px;

}


/* =====================================================
   DECISION EVIDENCE
   ===================================================== */

.evidence-panel {

  margin-top:
    20px;

  padding:
    22px;

  border-radius:
    18px;

  background:
    linear-gradient(
      145deg,
      rgba(
        8,
        24,
        38,
        0.95
      ),
      rgba(
        4,
        12,
        21,
        0.96
      )
    );

  border:
    1px solid
    rgba(
      76,
      180,
      235,
      0.24
    );

  box-shadow:
    0 10px 40px
    rgba(
      0,
      0,
      0,
      0.24
    );

}


.section-title {

  margin:
    0 0 16px;

  color:
    #69d8ff;

  font-size:
    20px;

  letter-spacing:
    1px;

}


.rule-box {

  padding:
    14px 16px;

  margin-bottom:
    14px;

  border-left:
    3px solid
    #48d5ff;

  background:
    rgba(
      55,
      140,
      190,
      0.08
    );

  font-family:
    monospace;

  font-size:
    12px;

  color:
    #bcecff;

}


.evidence-grid {

  display:
    grid;

  grid-template-columns:
    repeat(
      auto-fit,
      minmax(
        220px,
        1fr
      )
    );

  gap:
    10px;

}


.evidence-box {

  padding:
    13px;

  border-radius:
    10px;

  background:
    rgba(
      25,
      55,
      75,
      0.15
    );

  border:
    1px solid
    rgba(
      100,
      170,
      205,
      0.16
    );

}


.evidence-label {

  font-size:
    10px;

  letter-spacing:
    1px;

  color:
    #54d2ff;

  margin-bottom:
    7px;

}


.evidence-text {

  font-size:
    12px;

  color:
    #a9c7d7;

  line-height:
    1.5;

}


.reason-box {

  margin-top:
    12px;

  padding:
    14px;

  border-radius:
    10px;

  background:
    rgba(
      0,
      190,
      255,
      0.05
    );

  color:
    #9fc5d9;

  font-size:
    12px;

}


.reason-box strong {

  color:
    #6ddcff;

}


/* =====================================================
   LED PANEL
   ===================================================== */

.led-panel {

  display:
    flex;

  justify-content:
    center;

  align-items:
    center;

  gap:
    25px;

  margin-top:
    18px;

}


.led-unit {

  display:
    flex;

  flex-direction:
    column;

  align-items:
    center;

  gap:
    7px;

}


.led-circle {

  width:
    30px;

  height:
    30px;

  border-radius:
    50%;

  background:
    #15212b;

  border:
    2px solid
    #344655;

  box-shadow:
    inset
    0 0 8px
    rgba(
      0,
      0,
      0,
      0.7
    );

  transition:
    0.3s ease;

}


.led-circle.green.active {

  background:
    #28ed83;

  border-color:
    #9cffc8;

  box-shadow:

    0 0 8px
    #28ed83,

    0 0 20px
    #28ed83,

    0 0 45px
    rgba(
      40,
      237,
      131,
      0.65
    );

}


.led-circle.yellow.active {

  background:
    #ffd33d;

  border-color:
    #fff2a4;

  box-shadow:

    0 0 8px
    #ffd33d,

    0 0 20px
    #ffd33d,

    0 0 45px
    rgba(
      255,
      211,
      61,
      0.65
    );

}


.led-circle.red.active {

  background:
    #ff4055;

  border-color:
    #ffadb7;

  box-shadow:

    0 0 8px
    #ff4055,

    0 0 20px
    #ff4055,

    0 0 45px
    rgba(
      255,
      64,
      85,
      0.7
    );

  animation:
    pulseRed
    1s infinite;

}


@keyframes pulseRed {

  0% {

    transform:
      scale(1);

  }

  50% {

    transform:
      scale(1.13);

  }

  100% {

    transform:
      scale(1);

  }

}


.led-label {

  font-size:
    9px;

  letter-spacing:
    1px;

  color:
    #7692a4;

}


/* =====================================================
   EVENT HISTORY
   ===================================================== */

.history {

  margin-top:
    20px;

  padding:
    22px;

  border-radius:
    18px;

  background:
    linear-gradient(
      145deg,
      rgba(
        9,
        23,
        36,
        0.95
      ),
      rgba(
        4,
        12,
        21,
        0.97
      )
    );

  border:
    1px solid
    rgba(
      80,
      165,
      205,
      0.2
    );

}


.history-header {

  display:
    flex;

  justify-content:
    space-between;

  align-items:
    center;

  gap:
    10px;

  flex-wrap:
    wrap;

}


.history-status {

  font-size:
    10px;

  letter-spacing:
    1px;

  color:
    #5ed9ff;

}


.event {

  padding:
    12px 14px;

  margin-bottom:
    7px;

  border-left:
    3px solid
    #3fcaff;

  background:
    rgba(
      60,
      125,
      165,
      0.07
    );

  border-radius:
    6px;

  font-family:
    monospace;

  font-size:
    11px;

  color:
    #b6d1df;

}


.event:first-child {

  border-left-color:
    #67edff;

  background:
    rgba(
      50,
      190,
      240,
      0.10
    );

}


/* =====================================================
   FOOTER
   ===================================================== */

.footer {

  text-align:
    center;

  padding:
    28px 10px;

  color:
    #426277;

  font-size:
    10px;

  letter-spacing:
    2px;

}


/* =====================================================
   MOBILE
   ===================================================== */

@media(
  max-width: 600px
) {

  .container {

    padding:
      12px;

  }


  .header h1 {

    font-size:
      27px;

    letter-spacing:
      5px;

  }


  .status h2 {

    font-size:
      30px;

  }

}

</style>

</head>


<body>


<!-- ===================================================
     HEADER
     =================================================== -->

<div class="header">

<h1>TRUSTGUARD</h1>

<p>
CYBER-PHYSICAL SECURITY DECISION SYSTEM
</p>

</div>


<div class="container">


<!-- ===================================================
     TRUST + SECURITY STATUS
     =================================================== -->

<div class="top-panel">


<!-- TRUST SCORE -->

<div class="trust-card">

<div class="trust-title">
OVERALL TRUST SCORE
</div>


<div
id="trustGauge"
class="gauge"
style="
--score:100;
--gaugeColor:#45ef91;
">


<div class="gauge-content">

<div
id="trustScore"
class="gauge-number">
100
</div>

<div class="gauge-label">
TRUST %
</div>

</div>

</div>

</div>


<!-- SECURITY STATUS -->

<div
id="mainStatus"
class="status safe">

<h2 id="securityState">
SAFE
</h2>

<p id="securityDescription">
System operating normally
</p>


<!-- DECISION PIPELINE -->

<div class="pipeline">

<div class="pipeline-box">
SENSOR EVIDENCE
</div>

<div class="pipeline-arrow">
->
</div>

<div class="pipeline-box">
RISK SCORING
</div>

<div class="pipeline-arrow">
->
</div>

<div class="pipeline-box">
RULE ENGINE
</div>

<div class="pipeline-arrow">
->
</div>

<div class="pipeline-box">
RESPONSE
</div>

</div>


<!-- LED REPRESENTATION -->

<div class="led-panel">


<div class="led-unit">

<div
id="greenLed"
class="led-circle green">
</div>

<div class="led-label">
SAFE
</div>

</div>


<div class="led-unit">

<div
id="yellowLed"
class="led-circle yellow">
</div>

<div class="led-label">
WARNING
</div>

</div>


<div class="led-unit">

<div
id="redLed"
class="led-circle red">
</div>

<div class="led-label">
ALERT
</div>

</div>


</div>


</div>


</div>


<!-- ===================================================
     SENSOR GRID
     =================================================== -->

<div class="grid">


<!-- ===================================================
     PERSON
     =================================================== -->

<div class="card">

<h3>
PERSON SENSOR
</h3>

<div class="id">
POT-001
</div>


<div
id="personGauge"
class="sensor-gauge"
style="
--score:0;
--gaugeColor:#45ef91;
">


<div class="sensor-gauge-content">

<div
id="personRisk"
class="sensor-risk">
0
</div>

<div class="sensor-risk-label">
RISK
</div>

</div>

</div>


<div
id="person"
class="value">
LOW
</div>


<div class="small">
ADC:
<span id="adc">
0
</span>
</div>


<div class="small">
Confidence:
<span id="confidence">
0
</span>%
</div>


</div>


<!-- ===================================================
     MOTION
     =================================================== -->

<div class="card">

<h3>
MOTION SENSOR
</h3>

<div class="id">
PIR-002
</div>


<div
id="motionGauge"
class="sensor-gauge"
style="
--score:0;
--gaugeColor:#45ef91;
">


<div class="sensor-gauge-content">

<div
id="motionRisk"
class="sensor-risk">
0
</div>

<div class="sensor-risk-label">
RISK
</div>

</div>

</div>


<div
id="motion"
class="value">
NO MOTION
</div>


<div class="small">
Physical movement evidence
</div>


</div>


<!-- ===================================================
     DOOR
     =================================================== -->

<div class="card">

<h3>
DOOR SENSOR
</h3>

<div class="id">
DOOR-003
</div>


<div
id="doorGauge"
class="sensor-gauge"
style="
--score:0;
--gaugeColor:#45ef91;
">


<div class="sensor-gauge-content">

<div
id="doorRisk"
class="sensor-risk">
0
</div>

<div class="sensor-risk-label">
RISK
</div>

</div>

</div>


<div
id="door"
class="value">
CLOSED
</div>


<div class="small">
Physical access state
</div>


</div>


<!-- ===================================================
     ATTACK
     =================================================== -->

<div class="card">

<h3>
ATTACK INPUT
</h3>

<div class="id">
ATTACK-004
</div>


<div
id="attackGauge"
class="sensor-gauge"
style="
--score:0;
--gaugeColor:#45ef91;
">


<div class="sensor-gauge-content">

<div
id="attackRisk"
class="sensor-risk">
0
</div>

<div class="sensor-risk-label">
RISK
</div>

</div>

</div>


<div
id="attack"
class="value">
OFF
</div>


<div class="small">
Explicit threat evidence
</div>


</div>


<!-- ===================================================
     TAMPER
     =================================================== -->

<div class="card">

<h3>
TAMPER SENSOR
</h3>

<div class="id">
TAMPER-005
</div>


<div
id="tamperGauge"
class="sensor-gauge"
style="
--score:0;
--gaugeColor:#45ef91;
">


<div class="sensor-gauge-content">

<div
id="tamperRisk"
class="sensor-risk">
0
</div>

<div class="sensor-risk-label">
RISK
</div>

</div>

</div>


<div
id="tamper"
class="value">
NORMAL
</div>


<div class="small">
Device integrity evidence
</div>


</div>


<!-- ===================================================
     SERVO
     =================================================== -->

<div class="card">

<h3>
LOCK / SERVO
</h3>

<div class="id">
PHYSICAL RESPONSE
</div>


<div class="sensor-gauge"
style="
--score:0;
--gaugeColor:#45ef91;
">


<div class="sensor-gauge-content">

<div
id="servoGaugeText"
class="sensor-risk">
LOCK
</div>

<div class="sensor-risk-label">
ACTUATOR
</div>

</div>

</div>


<div
id="servo"
class="value">
LOCKED
</div>


<div class="small">
Decision response actuator
</div>

</div>


<!-- ===================================================
     BUZZER
     =================================================== -->

<div class="card">

<h3>
BUZZER
</h3>

<div class="id">
AUDIO RESPONSE
</div>


<div class="sensor-gauge"
style="
--score:0;
--gaugeColor:#45ef91;
">


<div class="sensor-gauge-content">

<div
id="buzzerGaugeText"
class="sensor-risk">
OFF
</div>

<div class="sensor-risk-label">
ALARM
</div>

</div>

</div>


<div
id="buzzer"
class="value">
OFF
</div>


<div class="small">
Security alert response
</div>

</div>


</div>


<!-- ===================================================
     DECISION EVIDENCE
     =================================================== -->

<div class="evidence-panel">

<h2 class="section-title">
DECISION EVIDENCE
</h2>


<div
id="decisionRule"
class="rule-box">

RULE 01:
NORMAL SENSOR CONSENSUS -> SAFE

</div>


<div class="evidence-grid">


<div class="evidence-box">

<div class="evidence-label">
EVIDENCE 01
</div>

<div
id="evidence1"
class="evidence-text">

PIR-002:
no movement

</div>

</div>


<div class="evidence-box">

<div class="evidence-label">
EVIDENCE 02
</div>

<div
id="evidence2"
class="evidence-text">

DOOR-003:
access path CLOSED

</div>

</div>


<div class="evidence-box">

<div class="evidence-label">
EVIDENCE 03
</div>

<div
id="evidence3"
class="evidence-text">

POT-001:
LOW person signal

</div>

</div>


<div class="evidence-box">

<div class="evidence-label">
ACTIVE SIGNALS
</div>

<div
class="evidence-text">

<span id="evidenceCount">
0
</span>

security evidence items detected

</div>

</div>


</div>


<div class="reason-box">

<strong>
WHY THIS DECISION?
</strong>

<span id="decisionReason">
All monitored conditions are normal.
</span>

</div>


</div>


<!-- ===================================================
     EVENT HISTORY
     =================================================== -->

<div class="history">

<div class="history-header">

<h2 class="section-title">
EVENT HISTORY
</h2>

<div class="history-status">
PERSISTENT ESP32 LOG
</div>

</div>


<div id="history">

SYSTEM INITIALIZING...

</div>


</div>


<!-- ===================================================
     FOOTER
     =================================================== -->

<div class="footer">

TRUSTGUARD |
ESP32 REAL-TIME CYBER-PHYSICAL SECURITY MONITOR

</div>


</div>


<script>


// =====================================================
// GAUGE COLOR
// =====================================================

function gaugeColor(score) {

  if (
    score <= 30
  ) {

    return "#45ef91";

  }

  else if (
    score <= 60
  ) {

    return "#ffd33d";

  }

  else {

    return "#ff4f63";

  }

}


// =====================================================
// UPDATE SENSOR GAUGE
// =====================================================

function updateGauge(
  id,
  scoreElement,
  score
) {

  let gauge =
    document.getElementById(
      id
    );


  let color =
    gaugeColor(
      score
    );


  gauge.style.setProperty(
    "--score",
    score
  );


  gauge.style.setProperty(
    "--gaugeColor",
    color
  );


  document.getElementById(
    scoreElement
  ).innerHTML =
    score;

}


// =====================================================
// UPDATE TRUST GAUGE
// =====================================================

function updateTrustGauge(
  score
) {

  let gauge =
    document.getElementById(
      "trustGauge"
    );


  let color;


  if (
    score >= 80
  ) {

    color =
      "#45ef91";

  }

  else if (
    score >= 50
  ) {

    color =
      "#ffd33d";

  }

  else {

    color =
      "#ff4f63";

  }


  gauge.style.setProperty(
    "--score",
    score
  );


  gauge.style.setProperty(
    "--gaugeColor",
    color
  );

}


// =====================================================
// UPDATE DASHBOARD
// =====================================================

function updateDashboard() {

  fetch(
    "/data"
  )

  .then(
    response =>
      response.json()
  )

  .then(
    data => {


      // ================================================
      // TRUST SCORE
      // ================================================

      document.getElementById(
        "trustScore"
      ).innerHTML =
        data.trust;


      updateTrustGauge(
        data.trust
      );


      // ================================================
      // SECURITY STATE
      // ================================================

      document.getElementById(
        "securityState"
      ).innerHTML =
        data.security;


      // ================================================
      // DESCRIPTION
      // ================================================

      let description =
        "System operating normally";


      if (
        data.security ==
        "SUSPICIOUS"
      ) {

        description =
          "Potential security anomaly detected";

      }

      else if (
        data.security ==
        "CRITICAL"
      ) {

        description =
          "Multiple physical indicators correlate";

      }

      else if (
        data.security ==
        "ATTACK"
      ) {

        description =
          "Explicit attack evidence detected";

      }

      else if (
        data.security ==
        "TAMPER"
      ) {

        description =
          "Device integrity compromise detected";

      }


      document.getElementById(
        "securityDescription"
      ).innerHTML =
        description;


      // ================================================
      // STATUS COLOR
      // ================================================

      let status =
        document.getElementById(
          "mainStatus"
        );


      status.className =
        "status";


      if (
        data.security ==
        "SAFE"
      ) {

        status.classList.add(
          "safe"
        );

      }

      else if (
        data.security ==
        "SUSPICIOUS"
      ) {

        status.classList.add(
          "warning"
        );

      }

      else {

        status.classList.add(
          "alert"
        );

      }


      // ================================================
      // PERSON
      // ================================================

      document.getElementById(
        "person"
      ).innerHTML =
        data.person;


      document.getElementById(
        "adc"
      ).innerHTML =
        data.adc;


      document.getElementById(
        "confidence"
      ).innerHTML =
        data.confidence;


      updateGauge(
        "personGauge",
        "personRisk",
        data.personRisk
      );


      // ================================================
      // MOTION
      // ================================================

      document.getElementById(
        "motion"
      ).innerHTML =

        data.motion
        ?
        "MOTION DETECTED"
        :
        "NO MOTION";


      updateGauge(
        "motionGauge",
        "motionRisk",
        data.motionRisk
      );


      // ================================================
      // DOOR
      // ================================================

      document.getElementById(
        "door"
      ).innerHTML =

        data.door
        ?
        "OPEN"
        :
        "CLOSED";


      updateGauge(
        "doorGauge",
        "doorRisk",
        data.doorRisk
      );


      // ================================================
      // ATTACK
      // ================================================

      document.getElementById(
        "attack"
      ).innerHTML =

        data.attack
        ?
        "ATTACK ACTIVE"
        :
        "OFF";


      updateGauge(
        "attackGauge",
        "attackRisk",
        data.attackRisk
      );


      // ================================================
      // TAMPER
      // ================================================

      document.getElementById(
        "tamper"
      ).innerHTML =

        data.tamper
        ?
        "TAMPER DETECTED"
        :
        "NORMAL";


      updateGauge(
        "tamperGauge",
        "tamperRisk",
        data.tamperRisk
      );


      // ================================================
      // SERVO
      // ================================================

      document.getElementById(
        "servo"
      ).innerHTML =
        data.servo;


      document.getElementById(
        "servoGaugeText"
      ).innerHTML =
        data.servo ==
        "UNLOCKED"
        ?
        "OPEN"
        :
        "LOCK";


      // ================================================
      // BUZZER
      // ================================================

      document.getElementById(
        "buzzer"
      ).innerHTML =

        data.buzzer
        ?
        "ON"
        :
        "OFF";


      document.getElementById(
        "buzzerGaugeText"
      ).innerHTML =

        data.buzzer
        ?
        "ON"
        :
        "OFF";


      // ================================================
      // LED CIRCLES
      // ================================================

      document.getElementById(
        "greenLed"
      ).classList.remove(
        "active"
      );

      document.getElementById(
        "yellowLed"
      ).classList.remove(
        "active"
      );

      document.getElementById(
        "redLed"
      ).classList.remove(
        "active"
      );


      if (
        data.led ==
        "GREEN"
      ) {

        document.getElementById(
          "greenLed"
        ).classList.add(
          "active"
        );

      }

      else if (
        data.led ==
        "YELLOW"
      ) {

        document.getElementById(
          "yellowLed"
        ).classList.add(
          "active"
        );

      }

      else {

        document.getElementById(
          "redLed"
        ).classList.add(
          "active"
        );

      }


      // ================================================
      // DECISION RULE
      // ================================================

      document.getElementById(
        "decisionRule"
      ).innerHTML =
        data.rule;


      document.getElementById(
        "decisionReason"
      ).innerHTML =
        data.reason;


      // ================================================
      // EVIDENCE
      // ================================================

      document.getElementById(
        "evidence1"
      ).innerHTML =
        data.evidence1;


      document.getElementById(
        "evidence2"
      ).innerHTML =
        data.evidence2;


      document.getElementById(
        "evidence3"
      ).innerHTML =
        data.evidence3;


      document.getElementById(
        "evidenceCount"
      ).innerHTML =
        data.active;


      // ================================================
      // EVENT HISTORY
      // ================================================

      let historyHTML =
        "";


      if (
        data.history.length ==
        0
      ) {

        historyHTML =
          '<div class="event">No events recorded.</div>';

      }

      else {

        data.history.forEach(
          function(event) {

            historyHTML +=
              '<div class="event">' +
              event +
              '</div>';

          }
        );

      }


      document.getElementById(
        "history"
      ).innerHTML =
        historyHTML;

    }
  )

  .catch(
    error => {

      console.log(
        "Dashboard connection error:",
        error
      );

    }
  );

}


// =====================================================
// UPDATE EVERY 500 ms
// =====================================================

setInterval(
  updateDashboard,
  500
);


// =====================================================
// INITIAL UPDATE
// =====================================================

updateDashboard();


</script>


</body>

</html>

)rawliteral";


// ======================================================
// JSON DATA ENDPOINT
// ======================================================

void handleData() {

  String json = "{";


  // ====================================================
  // SENSOR DATA
  // ====================================================

  json +=
    "\"adc\":" +
    String(potValue) +
    ",";


  json +=
    "\"confidence\":" +
    String(confidence) +
    ",";


  json +=
    "\"person\":\"" +
    personState +
    "\",";


  json +=
    "\"motion\":" +
    String(
      motionDetected
      ?
      "true"
      :
      "false"
    ) +
    ",";


  json +=
    "\"door\":" +
    String(
      doorOpen
      ?
      "true"
      :
      "false"
    ) +
    ",";


  json +=
    "\"attack\":" +
    String(
      attackMode
      ?
      "true"
      :
      "false"
    ) +
    ",";


  json +=
    "\"tamper\":" +
    String(
      tamperDetected
      ?
      "true"
      :
      "false"
    ) +
    ",";


  // ====================================================
  // SECURITY
  // ====================================================

  json +=
    "\"security\":\"" +
    securityState +
    "\",";


  json +=
    "\"trust\":" +
    String(trustScore) +
    ",";


  json +=
    "\"risk\":" +
    String(overallRisk) +
    ",";


  // ====================================================
  // SENSOR RISK
  // ====================================================

  json +=
    "\"personRisk\":" +
    String(personRisk) +
    ",";


  json +=
    "\"motionRisk\":" +
    String(motionRisk) +
    ",";


  json +=
    "\"doorRisk\":" +
    String(doorRisk) +
    ",";


  json +=
    "\"attackRisk\":" +
    String(attackRisk) +
    ",";


  json +=
    "\"tamperRisk\":" +
    String(tamperRisk) +
    ",";


  // ====================================================
  // ACTUATORS
  // ====================================================

  json +=
    "\"servo\":\"" +
    servoState +
    "\",";


  json +=
    "\"buzzer\":" +
    String(
      buzzerState
      ?
      "true"
      :
      "false"
    ) +
    ",";


  // ====================================================
  // DECISION ENGINE
  // ====================================================

  json +=
    "\"rule\":\"" +
    decisionRule +
    "\",";


  json +=
    "\"reason\":\"" +
    decisionReason +
    "\",";


  json +=
    "\"evidence1\":\"" +
    evidence1 +
    "\",";


  json +=
    "\"evidence2\":\"" +
    evidence2 +
    "\",";


  json +=
    "\"evidence3\":\"" +
    evidence3 +
    "\",";


  json +=
    "\"active\":" +
    String(activeSignals) +
    ",";


  // ====================================================
  // LED
  // ====================================================

  String ledState;


  if (
    securityState ==
    "SAFE"
  ) {

    ledState =
      "GREEN";

  }

  else if (
    securityState ==
    "SUSPICIOUS"
  ) {

    ledState =
      "YELLOW";

  }

  else {

    ledState =
      "RED";

  }


  json +=
    "\"led\":\"" +
    ledState +
    "\",";


  // ====================================================
  // EVENT HISTORY
  // ====================================================

  json +=
    "\"history\":[";


  for (
    int i = 0;
    i < historyCount;
    i++
  ) {

    json +=
      "\"";


    String safeEvent =
      eventHistory[i];


    // JSON escape

    safeEvent.replace(
      "\\",
      "\\\\"
    );


    safeEvent.replace(
      "\"",
      "\\\""
    );


    safeEvent.replace(
      "\n",
      " "
    );


    safeEvent.replace(
      "\r",
      " "
    );


    json +=
      safeEvent;


    json +=
      "\"";


    if (
      i <
      historyCount - 1
    ) {

      json +=
        ",";

    }

  }


  json +=
    "]";


  json +=
    "}";


  server.send(
    200,
    "application/json",
    json
  );

}


// ======================================================
// WEB ROOT
// ======================================================

void handleRoot() {

  server.send_P(
    200,
    "text/html",
    MAIN_PAGE
  );

}


// ======================================================
// SETUP
// ======================================================

void setup() {

  Serial.begin(
    115200
  );


  delay(
    1000
  );


  // ====================================================
  // I2C
  // ====================================================

  Wire.begin(
    21,
    22
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

  doorServo.setPeriodHertz(
    50
  );


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
    "WiFi STARTING"
  );


  // ====================================================
  // LOAD OLD EVENT HISTORY
  // ====================================================

  loadHistory();


  // ====================================================
  // WIFI ACCESS POINT
  // ====================================================

  WiFi.mode(
    WIFI_AP
  );


  WiFi.softAP(
    AP_SSID,
    AP_PASSWORD
  );


  IPAddress IP =
    WiFi.softAPIP();


  // ====================================================
  // WEB SERVER
  // ====================================================

  server.on(
    "/",
    handleRoot
  );


  server.on(
    "/data",
    handleData
  );


  server.begin();


  // ====================================================
  // SERIAL INFORMATION
  // ====================================================

  Serial.println();

  Serial.println(
    "========================================"
  );

  Serial.println(
    "        TRUSTGUARD SYSTEM"
  );

  Serial.println(
    "========================================"
  );


  Serial.println();


  Serial.print(
    "WiFi Name       : "
  );

  Serial.println(
    AP_SSID
  );


  Serial.print(
    "WiFi Password   : "
  );

  Serial.println(
    AP_PASSWORD
  );


  Serial.print(
    "Dashboard IP    : "
  );

  Serial.println(
    IP
  );


  Serial.println();


  Serial.println(
    "Sensor IDs:"
  );


  Serial.println(
    "POT-001    -> Person Sensor"
  );


  Serial.println(
    "PIR-002    -> Motion Sensor"
  );


  Serial.println(
    "DOOR-003   -> Door Sensor"
  );


  Serial.println(
    "ATTACK-004 -> Attack Input"
  );


  Serial.println(
    "TAMPER-005 -> Tamper Sensor"
  );


  Serial.println();


  Serial.println(
    "DECISION ENGINE READY"
  );


  Serial.println(
    "PERSISTENT EVENT HISTORY READY"
  );


  Serial.println(
    "WEB SERVER READY"
  );


  Serial.println();


  // ====================================================
  // SYSTEM START EVENT
  // ====================================================

  addEvent(
    "SYSTEM STARTED | EVENT HISTORY RESTORED"
  );


  // ====================================================
  // INITIAL SENSOR ANALYSIS
  // ====================================================

  potValue =
    analogRead(
      POT_PIN
    );


  confidence =
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


  if (
    potValue <= 1351
  ) {

    personState =
      "LOW";

  }

  else if (
    potValue <= 2702
  ) {

    personState =
      "MID";

  }

  else {

    personState =
      "HIGH";

  }


  motionDetected =
    digitalRead(
      PIR_PIN
    );


  doorOpen =
    (
      digitalRead(
        DOOR_PIN
      )
      ==
      HIGH
    );


  attackMode =
    (
      digitalRead(
        ATTACK_PIN
      )
      ==
      LOW
    );


  tamperDetected =
    (
      digitalRead(
        TAMPER_PIN
      )
      ==
      LOW
    );


  // ====================================================
  // FIRST DECISION
  // ====================================================

  calculateRisk();

  determineSecurityState();

  buildDecisionEvidence();


  previousDecisionSignature =
    createDecisionSignature();


  // ====================================================
  // LCD READY
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


  delay(
    2000
  );

}


// ======================================================
// MAIN LOOP
// ======================================================

void loop() {

  // ====================================================
  // WEB SERVER
  // ====================================================

  server.handleClient();


  // ====================================================
  // POTENTIOMETER
  // ====================================================

  potValue =
    analogRead(
      POT_PIN
    );


  confidence =
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


  if (
    potValue <= 1351
  ) {

    personState =
      "LOW";

  }

  else if (
    potValue <= 2702
  ) {

    personState =
      "MID";

  }

  else {

    personState =
      "HIGH";

  }


  // ====================================================
  // PIR
  // ====================================================

  motionDetected =
    digitalRead(
      PIR_PIN
    );


  // ====================================================
  // DOOR
  // ====================================================

  bool doorInput =
    digitalRead(
      DOOR_PIN
    );


  doorOpen =
    (
      doorInput ==
      HIGH
    );


  if (
    doorOpen
  ) {

    doorState =
      "OPEN";

  }

  else {

    doorState =
      "CLOSED";

  }


  // ====================================================
  // ATTACK
  // ====================================================

  attackMode =
    (
      digitalRead(
        ATTACK_PIN
      )
      ==
      LOW
    );


  // ====================================================
  // TAMPER
  // ====================================================

  tamperDetected =
    (
      digitalRead(
        TAMPER_PIN
      )
      ==
      LOW
    );


  // ====================================================
  // SECURITY ANALYSIS
  // ====================================================

  updateSecuritySystem();


  // ====================================================
  // PHYSICAL RESPONSE
  // ====================================================

  applySecurityResponse();


  // ====================================================
  // SERIAL MONITOR
  // ====================================================

  Serial.println();

  Serial.println(
    "----------------------------------------"
  );

  Serial.println(
    "         TRUSTGUARD STATUS"
  );

  Serial.println(
    "----------------------------------------"
  );


  Serial.print(
    "POT-001 ADC       : "
  );

  Serial.println(
    potValue
  );


  Serial.print(
    "PERSON            : "
  );

  Serial.println(
    personState
  );


  Serial.print(
    "PERSON RISK       : "
  );

  Serial.println(
    personRisk
  );


  Serial.print(
    "PIR-002 MOTION    : "
  );

  Serial.println(
    motionDetected
    ?
    "DETECTED"
    :
    "NO MOTION"
  );


  Serial.print(
    "MOTION RISK       : "
  );

  Serial.println(
    motionRisk
  );


  Serial.print(
    "DOOR-003          : "
  );

  Serial.println(
    doorState
  );


  Serial.print(
    "DOOR RISK         : "
  );

  Serial.println(
    doorRisk
  );


  Serial.print(
    "ATTACK-004        : "
  );

  Serial.println(
    attackMode
    ?
    "ACTIVE"
    :
    "OFF"
  );


  Serial.print(
    "ATTACK RISK       : "
  );

  Serial.println(
    attackRisk
  );


  Serial.print(
    "TAMPER-005        : "
  );

  Serial.println(
    tamperDetected
    ?
    "DETECTED"
    :
    "NORMAL"
  );


  Serial.print(
    "TAMPER RISK       : "
  );

  Serial.println(
    tamperRisk
  );


  Serial.print(
    "OVERALL RISK      : "
  );

  Serial.println(
    overallRisk
  );


  Serial.print(
    "TRUST SCORE       : "
  );

  Serial.print(
    trustScore
  );

  Serial.println(
    "%"
  );


  Serial.print(
    "SECURITY          : "
  );

  Serial.println(
    securityState
  );


  Serial.print(
    "DECISION RULE     : "
  );

  Serial.println(
    decisionRule
  );


  Serial.print(
    "SERVO             : "
  );

  Serial.println(
    servoState
  );


  Serial.print(
    "BUZZER            : "
  );

  Serial.println(
    buzzerState
    ?
    "ON"
    :
    "OFF"
  );


  Serial.println(
    "----------------------------------------"
  );


  // ====================================================
  // LCD
  // ====================================================

  lcd.clear();


  lcd.setCursor(
    0,
    0
  );


  if (
    securityState ==
    "SAFE"
  ) {

    lcd.print(
      "SAFE"
    );

  }

  else if (
    securityState ==
    "SUSPICIOUS"
  ) {

    lcd.print(
      "WARNING"
    );

  }

  else if (
    securityState ==
    "CRITICAL"
  ) {

    lcd.print(
      "CRITICAL"
    );

  }

  else if (
    securityState ==
    "ATTACK"
  ) {

    lcd.print(
      "ATTACK"
    );

  }

  else {

    lcd.print(
      "TAMPER"
    );

  }


  lcd.setCursor(
    0,
    1
  );


  lcd.print(
    "T:"
  );

  lcd.print(
    trustScore
  );


  lcd.print(
    "% P:"
  );


  lcd.print(
    personState
  );


  // ====================================================
  // WEB SERVER
  // ====================================================

  server.handleClient();


  delay(
    500
  );

}