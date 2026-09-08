// ======================================================
// ======================================================
// DECISION ENGINE
// Converts risk + sensor evidence into security state
// ======================================================
// ======================================================


// ======================================================
// DECISION EVIDENCE
// ======================================================

String decisionRule =
  "RULE 05: NO SIGNIFICANT THREAT EVIDENCE";

String decisionReason =
  "All monitored physical and cyber conditions are normal.";

String evidence1 =
  "No abnormal motion detected";

String evidence2 =
  "Door access state normal";

String evidence3 =
  "Attack and tamper signals inactive";

int evidenceCount = 0;


// ======================================================
// DECISION ENGINE
// ======================================================

void makeSecurityDecision() {

  evidenceCount = 0;


  // ====================================================
  // RULE 01
  // TAMPER HAS HIGHEST PRIORITY
  // ====================================================

  if (tamperDetected) {

    securityState = "TAMPER";

    decisionRule =
      "RULE 01: DEVICE TAMPER -> TAMPER";

    decisionReason =
      "Device integrity evidence indicates tampering.";

    evidence1 =
      "TAMPER-005: physical integrity anomaly";

    evidence2 =
      "Device trust has been reduced";

    evidence3 =
      "Protective response required";

    evidenceCount = 3;
  }


  // ====================================================
  // RULE 02
  // EXPLICIT ATTACK
  // ====================================================

  else if (attackMode) {

    securityState = "ATTACK";

    decisionRule =
      "RULE 02: ATTACK SIGNAL -> ATTACK";

    decisionReason =
      "Explicit attack evidence detected.";

    evidence1 =
      "ATTACK-004: threat signal ACTIVE";

    evidence2 =
      "High-confidence security evidence";

    evidence3 =
      "Lock and alarm response required";

    evidenceCount = 3;
  }


  // ====================================================
  // RULE 03
  // MULTI-SENSOR CORRELATION
  // ====================================================

  else if (
    motionDetected &&
    personState == "HIGH" &&
    doorOpen
  ) {

    securityState = "CRITICAL";

    decisionRule =
      "RULE 03: MOTION + HIGH PERSON + OPEN DOOR";

    decisionReason =
      "Multiple independent physical signals correlate.";

    evidence1 =
      "PIR-002: motion detected";

    evidence2 =
      "POT-001: high person-level signal";

    evidence3 =
      "DOOR-003: unauthorized physical path open";

    evidenceCount = 3;
  }


  // ====================================================
  // RULE 04
  // SUSPICIOUS
  // ====================================================

  else if (
    motionDetected ||
    personState == "MID" ||
    doorOpen
  ) {

    securityState = "SUSPICIOUS";


    if (
      motionDetected &&
      doorOpen
    ) {

      decisionRule =
        "RULE 04: MOTION + OPEN DOOR -> SUSPICIOUS";

    }

    else if (motionDetected) {

      decisionRule =
        "RULE 04: MOTION EVIDENCE -> SUSPICIOUS";

    }

    else if (doorOpen) {

      decisionRule =
        "RULE 04: OPEN ACCESS PATH -> SUSPICIOUS";

    }

    else {

      decisionRule =
        "RULE 04: ELEVATED PERSON SIGNAL -> SUSPICIOUS";

    }


    evidence1 =
      motionDetected
      ?
      "PIR-002: movement detected"
      :
      "PIR-002: no movement";


    evidence2 =
      doorOpen
      ?
      "DOOR-003: access path OPEN"
      :
      "DOOR-003: access path CLOSED";


    evidence3 =
      personState == "MID"
      ?
      "POT-001: elevated person signal"
      :
      "POT-001: baseline person signal";


    decisionReason =
      "One or more physical indicators require verification.";

    evidenceCount = 1;

    if (motionDetected)
      evidenceCount++;

    if (doorOpen)
      evidenceCount++;

    if (personState == "MID")
      evidenceCount++;
  }


  // ====================================================
  // RULE 05
  // SAFE BASELINE
  // ====================================================

  else {

    securityState = "SAFE";

    decisionRule =
      "RULE 05: NO SIGNIFICANT THREAT EVIDENCE";

    decisionReason =
      "All monitored physical and cyber conditions are normal.";

    evidence1 =
      "PIR-002: no motion detected";

    evidence2 =
      "DOOR-003: access path closed";

    evidence3 =
      "ATTACK/TAMPER channels inactive";

    evidenceCount = 0;
  }
}