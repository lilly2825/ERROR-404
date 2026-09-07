PROJECT KERBEROS
Trust-Aware Smart Home Security — Frontend Hackathon Prototype

FILES
-----
index.html  -> dashboard structure
style.css   -> futuristic cybersecurity UI
app.js      -> static mock data + attack simulation

RUN IN VS CODE
--------------
1. Extract this ZIP.
2. Open the project folder in VS Code.
3. Open index.html with Live Server, or simply open index.html in a browser.
4. No Firebase, backend, authentication, database, or external API is required.

DEMO
----
Normal state:
- Door Node: CLOSED / 95%
- PIR Motion Node: NO MOTION / 98%
- Camera Node: CLEAR / 92%
- Network Security: NORMAL / 97%

Click "SIMULATE DATA CONFLICT":
- System Status -> SECURITY ALERT
- Door Node -> INCONSISTENT
- Door Trust -> 15%
- PIR -> MOTION DETECTED
- Camera -> PERSON DETECTED
- Event log records the anomaly
- Central Decision Engine -> SECURITY ALERT
- Action -> NODE FLAGGED / SAFE RESPONSE ACTIVATED

Click "RESET SYSTEM" to restore the normal state.

LATER ESP32 INTEGRATION
-----------------------
The mock values are intentionally isolated in app.js. Later, replace the normalState/attackState values with real sensor values from your chosen communication layer. The UI does not depend on Firebase or any backend.
