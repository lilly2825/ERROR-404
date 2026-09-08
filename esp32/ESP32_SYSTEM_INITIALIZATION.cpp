// ======================================================
// ======================================================
// ESP32 SYSTEM INITIALIZATION
// setup()
// ======================================================
// ======================================================

void setup() {

  Serial.begin(115200);

  delay(1000);


  // ====================================================
  // GPIO INPUTS
  // ====================================================

  pinMode(PIR_PIN, INPUT);

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
  // GPIO OUTPUTS
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

  Wire.begin(
    21,
    22
  );

  lcd.init();

  lcd.backlight();

  lcd.clear();

  lcd.setCursor(0, 0);

  lcd.print("TRUSTGUARD");

  lcd.setCursor(0, 1);

  lcd.print("WiFi STARTING");


  // ====================================================
  // WIFI ACCESS POINT
  // ====================================================

  WiFi.mode(WIFI_AP);

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
  // INITIAL AUDIT EVENT
  // ====================================================

  addEvent(
    "SYSTEM STARTED"
  );


  previousSecurityState =
    "SAFE";


  // ====================================================
  // INITIAL DECISION
  // ====================================================

  calculateRiskScores();

  makeSecurityDecision();


  // ====================================================
  // LCD READY
  // ====================================================

  lcd.clear();

  lcd.setCursor(0, 0);

  lcd.print("TRUSTGUARD");

  lcd.setCursor(0, 1);

  lcd.print("SYSTEM READY");

  delay(2000);
}