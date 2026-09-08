// ======================================================
// ======================================================
// DASHBOARD DATA API
// ESP32 -> Dashboard JSON
// ======================================================
// ======================================================

void handleData() {

  String json = "{";

  // ----------------------------------------------------
  // RAW SENSOR DATA
  // ----------------------------------------------------

  json += "\"adc\":" +
          String(potValue) + ",";

  json += "\"confidence\":" +
          String(confidence) + ",";

  json += "\"person\":\"" +
          personState + "\",";

  json += "\"motion\":" +
          String(
            motionDetected
            ? "true"
            : "false"
          ) + ",";

  json += "\"door\":" +
          String(
            doorOpen
            ? "true"
            : "false"
          ) + ",";

  json += "\"attack\":" +
          String(
            attackMode
            ? "true"
            : "false"
          ) + ",";

  json += "\"tamper\":" +
          String(
            tamperDetected
            ? "true"
            : "false"
          ) + ",";


  // ----------------------------------------------------
  // DECISION
  // ----------------------------------------------------

  json += "\"security\":\"" +
          securityState + "\",";

  json += "\"servo\":\"" +
          servoState + "\",";

  json += "\"buzzer\":" +
          String(
            buzzerState
            ? "true"
            : "false"
          ) + ",";


  // ----------------------------------------------------
  // TRUST ENGINE
  // ----------------------------------------------------

  json += "\"personRisk\":" +
          String(personRisk) + ",";

  json += "\"motionRisk\":" +
          String(motionRisk) + ",";

  json += "\"doorRisk\":" +
          String(doorRisk) + ",";

  json += "\"attackRisk\":" +
          String(attackRisk) + ",";

  json += "\"tamperRisk\":" +
          String(tamperRisk) + ",";

  json += "\"overallRisk\":" +
          String(overallRisk) + ",";

  json += "\"trust\":" +
          String(trustScore) + ",";


  // ----------------------------------------------------
  // DECISION ENGINE
  // ----------------------------------------------------

  json += "\"rule\":\"" +
          decisionRule + "\",";

  json += "\"reason\":\"" +
          decisionReason + "\",";

  json += "\"evidence1\":\"" +
          evidence1 + "\",";

  json += "\"evidence2\":\"" +
          evidence2 + "\",";

  json += "\"evidence3\":\"" +
          evidence3 + "\",";

  json += "\"evidenceCount\":" +
          String(evidenceCount) + ",";


  // ----------------------------------------------------
  // LED STATE
  // ----------------------------------------------------

  String ledState;

  if (
    securityState ==
    "SAFE"
  ) {

    ledState = "GREEN";

  }

  else if (
    securityState ==
    "SUSPICIOUS"
  ) {

    ledState = "YELLOW";

  }

  else {

    ledState = "RED";

  }

  json += "\"led\":\"" +
          ledState + "\",";


  // ----------------------------------------------------
  // EVENT HISTORY
  // ----------------------------------------------------

  json += "\"history\":[";

  for (
    int i = 0;
    i < historyCount;
    i++
  ) {

    String safeEvent =
      eventHistory[i];

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

    json += "\"" +
            safeEvent +
            "\"";

    if (
      i <
      historyCount - 1
    ) {

      json += ",";

    }
  }

  json += "]";

  json += "}";


  server.send(
    200,
    "application/json",
    json
  );
}