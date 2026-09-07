from trust_engine import evaluate_event
import requests


FIREBASE_URL = "https://truthmesh-5c342-default-rtdb.asia-southeast1.firebasedatabase.app"

def make_decision(trust_result, command):

    event_status = trust_result["event_status"]

    # Critical event
    if event_status == "CRITICAL":
        return {
            "action": "LOCK",
            "alarm": True,
            "system_state": "THREAT",
            "reason": "CRITICAL_EVENT"
        }

    # Suspicious event
    elif event_status == "SUSPICIOUS":
        return {
            "action": "KEEP_LOCKED",
            "alarm": False,
            "system_state": "VERIFYING",
            "reason": "SUSPICIOUS_EVENT"
        }

    # Expected event
    elif event_status == "EXPECTED":

        if command == "UNLOCK":
            return {
                "action": "UNLOCK",
                "alarm": False,
                "system_state": "NORMAL",
                "reason": "VALIDATED_UNLOCK"
            }

        return {
            "action": "NO_ACTION",
            "alarm": False,
            "system_state": "NORMAL",
            "reason": "NO_COMMAND"
        }

    # Unknown event
    else:
        return {
            "action": "KEEP_LOCKED",
            "alarm": True,
            "system_state": "UNKNOWN",
            "reason": "UNKNOWN_EVENT"
        }


# ==========================================
# 1. READ SENSOR DATA FROM FIREBASE
# ==========================================

response = requests.get(
    f"{FIREBASE_URL}/devices.json"
)


sensor_data = response.json()

print("Sensor Data:")
print(sensor_data)


# ==========================================
# 2. TRUST ENGINE
# ==========================================

door_state = sensor_data["door_sensor"]["state"]
motion = sensor_data["pir_motion"]["motion"]

# Temporary camera proxy:
# Her Firebase currently doesn't have the potentiometer value.
# We'll add/use that later.
camera_value = 50

trust_result = evaluate_event(
    door_state=door_state,
    motion=motion,
    camera_value=camera_value,
    security_state="SECURED"
)

print("\nTrust Result:")
print(trust_result)
# -----------------------------------------
# WRITE TRUST RESULT TO FIREBASE
# -----------------------------------------

trust_url = f"{FIREBASE_URL}/trust.json"

requests.patch(
    trust_url,
    json={
        "event_status": trust_result["event_status"],
        "door_score": trust_result["door_trust"],
        "motion_score": trust_result["motion_trust"],
        "camera_score": trust_result["camera_trust"],
        "conflict": trust_result["conflict"],
        "suspicious_device": trust_result["suspicious_device"]
    }
)

print("\nTrust data written to Firebase.")

# ==========================================
# 3. COMMAND
# ==========================================

command_response = requests.get(
    f"{FIREBASE_URL}/command/requested_action.json"
)

command = command_response.json()

print("\nCommand:")
print(command)


# ==========================================
# 4. DECISION ENGINE
# ==========================================

decision = make_decision(
    trust_result,
    command
)

print("\nFinal Decision:")
print(decision)
# -----------------------------------------
# WRITE FINAL DECISION TO FIREBASE
# -----------------------------------------

decision_url = f"{FIREBASE_URL}/decision.json"

response = requests.patch(
    decision_url,
    json=decision
)

print("\nFirebase Decision Updated:")
print(response.json())
