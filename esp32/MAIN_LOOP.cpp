// ======================================================
// ======================================================
// SYSTEM EXECUTION
// loop()
// ======================================================
// ======================================================

void loop() {

  // ====================================================
  // 1. SERVE DASHBOARD
  // ====================================================

  server.handleClient();


  // ====================================================
  // 2. READ PHYSICAL SENSORS
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
      == HIGH
    );


  doorState =
    doorOpen
    ?
    "OPEN"
    :
    "CLOSED";


  attackMode =
    (
      digitalRead(
        ATTACK_PIN
      )
      == LOW
    );


  tamperDetected =
    (
      digitalRead(
        TAMPER_PIN
      )
      == LOW
    );


  // ====================================================
  // 3. TRUST ENGINE
  // ====================================================

  calculateRiskScores();


  // ====================================================
  // 4. DECISION ENGINE
  // ====================================================

  makeSecurityDecision();


  // ====================================================
  // 5. EVENT / AUDIT LOGGING
  // ====================================================

  if (
    securityState !=
    previousSecurityState
  ) {

    addEvent(
      "SECURITY STATE -> " +
      securityState
    );

    previousSecurityState =
      securityState;
  }


  // ====================================================
  // 6. PHYSICAL RESPONSE
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
    securityState ==
    "SAFE"
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
    securityState ==
    "SUSPICIOUS"
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


  // ====================================================
  // LCD
  // ====================================================

  lcd.clear();

  lcd.setCursor(
    0,
    0
  );

  lcd.print(
    securityState
  );

  lcd.setCursor(
    0,
    1
  );

  lcd.print("P:");
  lcd.print(personState);

  lcd.print(" M:");
  lcd.print(
    motionDetected
    ?
    "Y"
    :
    "N"
  );

  lcd.print(" D:");
  lcd.print(
    doorOpen
    ?
    "O"
    :
    "C"
  );


  // ====================================================
  // WEB SERVER
  // ====================================================

  server.handleClient();


  delay(500);
}