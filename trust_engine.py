def evaluate_event(door_state, motion, camera_value, security_state):

    door_trust = 100
    motion_trust = 100
    camera_trust = 100

    conflict = False
    suspicious_device = "NONE"
    event_status = "EXPECTED"

    # Unexpected door opening while the house is secured
    if security_state == "SECURED" and door_state == "OPEN":
        event_status = "SUSPICIOUS"
        door_trust -= 30
        suspicious_device = "door_sensor"

    # Strong conflict condition for our prototype
    if door_state == "OPEN" and not motion and camera_value < 30:
        conflict = True
        event_status = "CRITICAL"

        door_trust -= 20
        motion_trust -= 20
        camera_trust -= 20

        suspicious_device = "door_sensor"

    # Supporting PIR evidence
    if door_state == "OPEN" and motion:
        door_trust += 10

    # Camera proxy
    if camera_value >= 70:
        camera_status = "ACTIVITY_DETECTED"
    else:
        camera_status = "NO_ACTIVITY"

    # Keep scores between 0 and 100
    door_trust = max(0, min(100, door_trust))
    motion_trust = max(0, min(100, motion_trust))
    camera_trust = max(0, min(100, camera_trust))

    return {
        "event_status": event_status,
        "door_trust": door_trust,
        "motion_trust": motion_trust,
        "camera_trust": camera_trust,
        "conflict": conflict,
        "suspicious_device": suspicious_device,
        "camera_status": camera_status
    }