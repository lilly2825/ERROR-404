// ======================================================
// ======================================================
// EVENT / AUDIT LOG
// ======================================================
// ======================================================

const int MAX_HISTORY = 10;

String eventHistory[MAX_HISTORY];

int historyCount = 0;

unsigned long eventNumber = 0;


// ======================================================
// ADD EVENT
// ======================================================

void addEvent(String eventText) {

  eventNumber++;

  String entry =
    "#" +
    String(eventNumber) +
    "  " +
    eventText;


  if (historyCount < MAX_HISTORY) {

    historyCount++;

  }


  for (
    int i = MAX_HISTORY - 1;
    i > 0;
    i--
  ) {

    eventHistory[i] =
      eventHistory[i - 1];

  }


  eventHistory[0] =
    entry;
}


// ======================================================
// UPDATE SECURITY SYSTEM
// ======================================================

void updateSecuritySystem() {

  // TRUST ENGINE
  calculateRiskScores();


  // DECISION ENGINE
  makeSecurityDecision();


  // AUDIT LOG
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
}