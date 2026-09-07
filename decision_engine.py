from trust_engine import evaluate_event
import requests


FIREBASE_URL = "https://truthmesh-hackathon-default-rtdb.firebaseio.com"


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
    f"{FIREBASE_URL}/sensors.json"
)

sensor_data = response.json()

print("Sensor Data:")
print(sensor_data)


# ==========================================
# 2. TRUST ENGINE
# ==========================================

trust_result = evaluate_event(
    door_state=sensor_data["door"],
    motion=sensor_data["motion"],
    camera_value=sensor_data["camera_value"],
    security_state="SECURED"
)

print("\nTrust Result:")
print(trust_result)


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