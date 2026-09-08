// ======================================================
// ======================================================
// TRUST ENGINE
// Converts sensor observations into risk/trust scores
// ======================================================
// ======================================================


// ======================================================
// INDIVIDUAL SENSOR RISK
// ======================================================

int personRisk = 0;
int motionRisk = 0;
int doorRisk = 0;
int attackRisk = 0;
int tamperRisk = 0;


// ======================================================
// GLOBAL TRUST VALUES
// ======================================================

int overallRisk = 0;
int trustScore = 100;


// ======================================================
// CALCULATE SENSOR RISK
// ======================================================

void calculateRiskScores() {

  // ----------------------------------------------------
  // PERSON SENSOR
  // ----------------------------------------------------

  if (personState == "LOW") {

    personRisk = 0;

  }

  else if (personState == "MID") {

    personRisk = 40;

  }

  else {

    personRisk = 80;

  }


  // ----------------------------------------------------
  // MOTION SENSOR
  // ----------------------------------------------------

  if (motionDetected) {

    motionRisk = 65;

  }

  else {

    motionRisk = 0;

  }


  // ----------------------------------------------------
  // DOOR SENSOR
  // ----------------------------------------------------

  if (doorOpen) {

    doorRisk = 75;

  }

  else {

    doorRisk = 0;

  }


  // ----------------------------------------------------
  // ATTACK INPUT
  // ----------------------------------------------------

  if (attackMode) {

    attackRisk = 100;

  }

  else {

    attackRisk = 0;

  }


  // ----------------------------------------------------
  // TAMPER SENSOR
  // ----------------------------------------------------

  if (tamperDetected) {

    tamperRisk = 100;

  }

  else {

    tamperRisk = 0;

  }


  // ====================================================
  // WEIGHTED CYBER-PHYSICAL RISK MODEL
  //
  // Person = 15%
  // Motion = 20%
  // Door   = 20%
  // Attack = 25%
  // Tamper = 20%
  //
  // Total = 100%
  // ====================================================

  overallRisk =

    (personRisk * 15) / 100 +

    (motionRisk * 20) / 100 +

    (doorRisk * 20) / 100 +

    (attackRisk * 25) / 100 +

    (tamperRisk * 20) / 100;


  overallRisk =
    constrain(
      overallRisk,
      0,
      100
    );


  // ====================================================
  // TRUST SCORE
  // ====================================================

  trustScore =
    100 - overallRisk;


  // ====================================================
  // HIGH-CONFIDENCE CYBER EVIDENCE OVERRIDE
  // ====================================================

  if (attackMode) {

    trustScore = 5;

  }

  else if (tamperDetected) {

    trustScore = 5;

  }

  else if (
    motionDetected &&
    personState == "HIGH" &&
    doorOpen
  ) {

    trustScore =
      max(
        10,
        trustScore - 25
      );

  }


  trustScore =
    constrain(
      trustScore,
      0,
      100
    );
}