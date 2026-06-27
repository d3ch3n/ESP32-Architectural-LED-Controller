let gateway = `ws://${window.location.hostname}/ws`;
let websocket;

window.addEventListener('load', initWebSocket);

function initWebSocket() {
    console.log("Opening WebSocket pipeline...");
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log("Connection established.");
    const badge = document.getElementById('status-badge');
    badge.innerText = "Online";
    badge.className = "badge online";
}

function onClose(event) {
    console.log("Connection lost. Retrying execution in 2 seconds...");
    const badge = document.getElementById('status-badge');
    badge.innerText = "Offline";
    badge.className = "badge offline";
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    // Structural sync engine payload receiver
    let data = JSON.parse(event.data);
    
    if (data.type === "status") {
        document.getElementById('profile-name').innerText = data.profileName;
        document.getElementById('power-switch').checked = data.power;
        document.getElementById('brightness-slider').value = data.brightness;
        
        // Calculate percentages mapping dynamically 
        let percent = Math.round((data.brightness / 255) * 100);
        document.getElementById('brightness-val').innerText = `${percent}%`;
    }
}

// --- INTERACTIVE CAPTURE AND EVENT ROUTING ---

document.getElementById('power-switch').addEventListener('change', (e) => {
    sendNetworkCommand('power', e.target.checked);
});

document.getElementById('brightness-slider').addEventListener('input', (e) => {
    let percent = Math.round((e.target.value / 255) * 100);
    document.getElementById('brightness-val').innerText = `${percent}%`;
});

document.getElementById('brightness-slider').addEventListener('change', (e) => {
    sendNetworkCommand('brightness', parseInt(e.target.value));
});

document.getElementById('color-picker').addEventListener('change', (e) => {
    // Converts hex format string (#ffffff) directly to 24-bit base integer values
    let hexString = e.target.value.replace('#', '0x');
    sendNetworkCommand('color', parseInt(hexString, 16));
});

function sendNetworkCommand(commandName, commandValue) {
    if (websocket.readyState === WebSocket.OPEN) {
        let payload = {
            cmd: commandName,
            value: commandValue
        };
        websocket.send(JSON.stringify(payload));
    }
}