const normalState = {
  door: {
    state: "CLOSED",
    reported: "Door Secure",
    trust: 95,
    health: "VERIFIED"
  },
  pir: {
    state: "NO MOTION",
    reported: "No movement detected",
    trust: 98,
    health: "VERIFIED"
  },
  camera: {
    state: "CLEAR",
    reported: "No threat detected",
    trust: 92,
    health: "VERIFIED"
  },
  network: {
    trust: 97
  },
  confidence: 96
};

const attackState = {
  door: {
    state: "INCONSISTENT",
    reported: "Reports CLOSED — evidence says OPEN",
    trust: 15,
    health: "FLAGGED"
  },
  pir: {
    state: "MOTION DETECTED",
    reported: "Movement detected",
    trust: 98,
    health: "VERIFIED"
  },
  camera: {
    state: "PERSON DETECTED",
    reported: "Human presence detected",
    trust: 92,
    health: "VERIFIED"
  },
  network: {
    trust: 97
  },
  confidence: 94
};

const $ = (id) => document.getElementById(id);

function setBar(id, value) {
  $(id).style.width = `${value}%`;
}

function setText(id, value) {
  $(id).textContent = value;
}

function addEvent(message, critical = false) {
  const log = $("eventLog");
  const now = new Date();
  const time = `[${now.toLocaleTimeString("en-GB", {
    hour12: false
  })}]`;

  const event = document.createElement("div");
  event.className = `event ${critical ? "alert-event" : ""}`;
  event.innerHTML = `
    <time>${time}</time>
    <span class="event-mark">${critical ? "!" : "›"}</span>
    <p>${message}</p>
  `;

  log.appendChild(event);
  log.scrollTop = log.scrollHeight;
}

function updateUI(state, attack = false) {
  setText("doorState", state.door.state);
  setText("doorReported", state.door.reported);
  setText("doorTrustValue", `${state.door.trust}%`);
  setBar("doorBar", state.door.trust);

  setText("pirState", state.pir.state);
  setText("pirReported", state.pir.reported);
  setText("pirTrustValue", `${state.pir.trust}%`);
  setBar("pirBar", state.pir.trust);

  setText("cameraState", state.camera.state);
  setText("cameraReported", state.camera.reported);
  setText("cameraTrustValue", `${state.camera.trust}%`);
  setBar("cameraBar", state.camera.trust);

  setText("networkTrustValue", `${state.network.trust}%`);
  setBar("networkBar", state.network.trust);

  setText("monitorDoor", `${state.door.trust}%`);
  setBar("monitorDoorBar", state.door.trust);

  setText("monitorPir", `${state.pir.trust}%`);
  setBar("monitorPirBar", state.pir.trust);

  setText("monitorCamera", `${state.camera.trust}%`);
  setBar("monitorCameraBar", state.camera.trust);

  setText("monitorNetwork", `${state.network.trust}%`);
  setBar("monitorNetworkBar", state.network.trust);

  setText("confidence", `${state.confidence}%`);

  $("doorCard").classList.toggle("alert", attack);
  $("doorHealth").classList.toggle("warning", attack);
  $("doorHealth").classList.toggle("secure", !attack);
  setText("doorHealth", state.door.health);

  $("decisionPanel").classList.toggle("alert", attack);
  setText("decisionIcon", attack ? "!" : "✓");
  setText("decisionTitle", attack ? "SECURITY ALERT" : "SYSTEM SECURE");
  setText(
    "decisionText",
    attack
      ? "Door data conflicts with PIR and camera evidence. The node has been flagged and a safe response has been activated."
      : "All reported states are currently consistent. No node requires isolation."
  );

  $("systemStatus").classList.toggle("alert", attack);
  setText("statusText", attack ? "SECURITY ALERT" : "SECURE");
  setText("modeText", attack ? "ANOMALY SCENARIO ACTIVE" : "NORMAL OPERATION");

  document.body.classList.toggle("attack", attack);
}

function simulateConflict() {
  updateUI(attackState, true);

  addEvent("ANOMALY: Door node data conflicts with corroborating sensor evidence", true);
  addEvent("PIR node: MOTION DETECTED");
  addEvent("Camera node: PERSON DETECTED");
  addEvent("Door trust score reduced: 95% → 15%", true);
  addEvent("ACTION: NODE FLAGGED / SAFE RESPONSE ACTIVATED", true);

  $("simulateBtn").disabled = true;
  $("simulateBtn").textContent = "CONFLICT ACTIVE";
}

function resetSystem() {
  updateUI(normalState, false);

  addEvent("Trust state reset — all nodes restored to verified");
  addEvent("System secure");

  $("simulateBtn").disabled = false;
  $("simulateBtn").innerHTML = "<span>⚠</span> SIMULATE DATA CONFLICT";
}

function updateClock() {
  setText(
    "currentTime",
    new Date().toLocaleTimeString("en-GB", { hour12: false }) + " LOCAL"
  );
}

$("simulateBtn").addEventListener("click", simulateConflict);
$("resetBtn").addEventListener("click", resetSystem);

updateUI(normalState);
updateClock();
setInterval(updateClock, 1000);
