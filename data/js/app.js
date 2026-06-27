let gateway = `ws://${window.location.hostname}/ws`;
let websocket;

window.addEventListener('load', () => {
    initWebSocket();
    initTabEngine();
    initFormEngine();
});

function initWebSocket() {
    console.log("Opening WebSocket pipeline...");
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    const badge = document.getElementById('status-badge');
    badge.innerText = "Online";
    badge.className = "badge online";
}

function onClose(event) {
    const badge = document.getElementById('status-badge');
    badge.innerText = "Offline";
    badge.className = "badge offline";
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    let data = JSON.parse(event.data);
    
    // Sincroniza o painel do usuário comum
    if (data.type === "status") {
        document.getElementById('profile-name').innerText = data.profileName;
        document.getElementById('power-switch').checked = data.power;
        document.getElementById('brightness-slider').value = data.brightness;
        
        let percent = Math.round((data.brightness / 255) * 100);
        document.getElementById('brightness-val').innerText = `${percent}%`;
        
        // PULO DO GATO: Se o pacote trouxer o mapa completo, preenche os inputs do Instalador
        if (data.strips) {
            for (let i = 0; i < data.strips.length; i++) {
                document.getElementById(`strip${i}-gpio`).value = data.strips[i].gpio;
                document.getElementById(`strip${i}-count`).value = data.strips[i].ledCount;
            }
        }
    }
}

// --- ENGINE DE INTERMUTABILIDADE DE ABAS ---
function initTabEngine() {
    const ctrlBtn = document.getElementById('tab-control-btn');
    const engBtn = document.getElementById('tab-eng-btn');
    const viewCtrl = document.getElementById('view-control');
    const viewEng = document.getElementById('view-engineering');

    ctrlBtn.addEventListener('click', () => {
        ctrlBtn.classList.add('active');
        engBtn.classList.remove('active');
        viewCtrl.classList.add('active');
        viewEng.classList.remove('active');
    });

    engBtn.addEventListener('click', () => {
        engBtn.classList.add('active');
        ctrlBtn.classList.remove('active');
        viewEng.classList.add('active');
        viewCtrl.classList.remove('active');
    });
}

// --- CAPTURA E ENVIO DOS DADOS DE ENGENHARIA VIA POST ---
function initFormEngine() {
    document.getElementById('eng-form').addEventListener('submit', (e) => {
        e.preventDefault(); // Impede o recarregamento clássico da página
        
        let configPayload = {
            strips: [
                { gpio: parseInt(document.getElementById('strip0-gpio').value), ledCount: parseInt(document.getElementById('strip0-count').value) },
                { gpio: parseInt(document.getElementById('strip1-gpio').value), ledCount: parseInt(document.getElementById('strip1-count').value) },
                { gpio: parseInt(document.getElementById('strip2-gpio').value), ledCount: parseInt(document.getElementById('strip2-count').value) }
            ]
        };

        // Envia o JSON estruturado diretamente para a rota REST do Back-End assíncrono
        fetch('/api/config', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(configPayload)
        })
        .then(response => {
            if (response.ok) {
                alert("Hardware parameters committed safely! ESP32 is rebooting...");
            } else {
                alert("Core rejected structural parameters. Operations aborted.");
            }
        });
    });
}

// --- CONTROLES DE ROTEAMENTO USER END ---
document.getElementById('power-switch').addEventListener('change', (e) => sendNetworkCommand('power', e.target.checked));
document.getElementById('brightness-slider').addEventListener('input', (e) => {
    let percent = Math.round((e.target.value / 255) * 100);
    document.getElementById('brightness-val').innerText = `${percent}%`;
});
document.getElementById('brightness-slider').addEventListener('change', (e) => sendNetworkCommand('brightness', parseInt(e.target.value)));
document.getElementById('color-picker').addEventListener('change', (e) => {
    let hexString = e.target.value.replace('#', '0x');
    sendNetworkCommand('color', parseInt(hexString, 16));
});

function sendNetworkCommand(commandName, commandValue) {
    if (websocket.readyState === WebSocket.OPEN) {
        let payload = { cmd: commandName, value: commandValue };
        websocket.send(JSON.stringify(payload));
    }
}

function triggerLedFinder(stripIndex) {
    let targetCount = parseInt(document.getElementById(`strip${stripIndex}-count`).value);
    
    if (websocket.readyState === WebSocket.OPEN && !isNaN(targetCount)) {
        let payload = {
            cmd: "finder",
            strip: stripIndex,
            count: targetCount
        };
        websocket.send(JSON.stringify(payload));
        console.log(`Dispatched finder frame to strip ${stripIndex} target index ${targetCount}`);
    }
}