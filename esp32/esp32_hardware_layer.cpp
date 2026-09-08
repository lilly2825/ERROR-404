// ======================================================
// ======================================================
// TRUSTGUARD
// 1. ESP32 / HARDWARE LAYER
// ======================================================
// ======================================================

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WebServer.h>


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
// ESP32 PIN DEFINITIONS
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
// SENSOR IDENTIFICATION
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
// RAW SENSOR STATE
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
String previousSecurityState = "SAFE";

String servoState = "LOCKED";

bool buzzerState = false;