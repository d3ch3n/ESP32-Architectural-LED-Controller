let gateway = `ws://${window.location.hostname}/ws`;
let websocket;

window.addEventListener('load', () => {
    initWebSocket();
    initTabEngine();
    initFormEngine();
    initControlEvents();
    initCommissioning();
});

function initWebSocket() {
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen() {
    const badge = document.getElementById('status-badge');
    badge.innerText = "Online";
    badge.className = "badge online";
    console.log("WebSocket connected");
}

function onClose() {
    const badge = document.getElementById('status-badge');
    badge.innerText = "Offline";
    badge.className = "badge offline";
    console.log("WebSocket disconnected");
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    console.log("WS RX:", event.data);

    let data = JSON.parse(event.data);

    if (data.type === "status") {
        updateStatus(data);
        return;
    }

    if (data.type === "commissioning") {
        document.getElementById('commission-led').value = data.led;
        document.getElementById('commission-led-display').innerText = data.led;
        document.getElementById('commission-auto-state').innerText = data.autoScan ? "Running" : "Stopped";
        document.getElementById('commission-blink-state').innerText = data.blink ? "Running" : "Stopped";
        return;
    }
}

function updateStatus(data) {
    document.getElementById('profile-name').innerText = data.profileName;
    document.getElementById('power-switch').checked = data.power;
    document.getElementById('brightness-slider').value = data.brightness;

    let percent = Math.round((data.brightness / 255) * 100);
    document.getElementById('brightness-val').innerText = `${percent}%`;

    if (typeof data.colorHex === "number") {
        document.getElementById('color-picker').value = numberToHexColor(data.colorHex);
    }

    if (data.strips) {
        for (let i = 0; i < data.strips.length; i++) {
            document.getElementById(`strip${i}-gpio`).value = data.strips[i].gpio;
            document.getElementById(`strip${i}-count`).value = data.strips[i].ledCount;
        }
    }
}

function initTabEngine() {
    const ctrlBtn = document.getElementById('tab-control-btn');
    const engBtn = document.getElementById('tab-eng-btn');
    const commissionBtn = document.getElementById('tab-commission-btn');

    const viewCtrl = document.getElementById('view-control');
    const viewEng = document.getElementById('view-engineering');
    const viewCommission = document.getElementById('view-commissioning');

    function activate(button, view) {
        ctrlBtn.classList.remove('active');
        engBtn.classList.remove('active');
        commissionBtn.classList.remove('active');

        viewCtrl.classList.remove('active');
        viewEng.classList.remove('active');
        viewCommission.classList.remove('active');

        button.classList.add('active');
        view.classList.add('active');
    }

    ctrlBtn.addEventListener('click', () => activate(ctrlBtn, viewCtrl));
    engBtn.addEventListener('click', () => activate(engBtn, viewEng));
    commissionBtn.addEventListener('click', () => activate(commissionBtn, viewCommission));
}

function initFormEngine() {
    document.getElementById('eng-form').addEventListener('submit', (e) => {
        e.preventDefault();

        let configPayload = {
            strips: [
                {
                    gpio: parseInt(document.getElementById('strip0-gpio').value),
                    ledCount: parseInt(document.getElementById('strip0-count').value)
                },
                {
                    gpio: parseInt(document.getElementById('strip1-gpio').value),
                    ledCount: parseInt(document.getElementById('strip1-count').value)
                },
                {
                    gpio: parseInt(document.getElementById('strip2-gpio').value),
                    ledCount: parseInt(document.getElementById('strip2-count').value)
                }
            ]
        };

        fetch('/api/config', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(configPayload)
        })
        .then(response => {
            if (response.ok) {
                alert("Configuração salva. O ESP32 será reiniciado.");
            } else {
                alert("Erro ao salvar configuração.");
            }
        });
    });
}

function initControlEvents() {
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
        sendNetworkCommand('color', hexColorToNumber(e.target.value));
    });
}

function initCommissioning() {
    const ledInput = document.getElementById('commission-led');
    const ledDisplay = document.getElementById('commission-led-display');

    function setLed(value) {
        value = parseInt(value);

        if (isNaN(value) || value < 0)
            value = 0;

        ledInput.value = value;
        ledDisplay.innerText = value;
    }

    ledInput.addEventListener('input', () => {
        setLed(ledInput.value);
    });

    document.getElementById('commission-minus-10').addEventListener('click', () => {
        setLed(parseInt(ledInput.value) - 10);
        sendCommissioningCommand('single');
    });

    document.getElementById('commission-minus-1').addEventListener('click', () => {
        setLed(parseInt(ledInput.value) - 1);
        sendCommissioningCommand('single');
    });

    document.getElementById('commission-plus-1').addEventListener('click', () => {
        setLed(parseInt(ledInput.value) + 1);
        sendCommissioningCommand('single');
    });

    document.getElementById('commission-plus-10').addEventListener('click', () => {
        setLed(parseInt(ledInput.value) + 10);
        sendCommissioningCommand('single');
    });

    document.getElementById('commission-single').addEventListener('click', () => {
        sendCommissioningCommand('single');
    });

    document.getElementById('commission-fill').addEventListener('click', () => {
        sendCommissioningCommand('fill');
    });

    document.getElementById('commission-off').addEventListener('click', () => {
        sendCommissioningCommand('off');
    });

    document.getElementById('commission-auto-start').addEventListener('click', () => {
        sendCommissioningCommand('autoStart');
    });

    document.getElementById('commission-auto-stop').addEventListener('click', () => {
        sendCommissioningCommand('autoStop');
    });

    document.getElementById('commission-blink-start').addEventListener('click', () => {
        sendCommissioningCommand('blinkStart');
    });

    document.getElementById('commission-blink-stop').addEventListener('click', () => {
        sendCommissioningCommand('blinkStop');
    });

    document.getElementById('commission-save-count').addEventListener('click', () => {
        let led = parseInt(ledInput.value);
        let count = led + 1;

        if (confirm(`Salvar ${count} LEDs para esta fita?`)) {
            sendCommissioningCommand('saveCount', count);
        }
    });

    document.querySelectorAll('[data-color]').forEach(button => {
        button.addEventListener('click', () => {
            document.getElementById('commission-color').value = button.dataset.color;
            sendCommissioningCommand('single');
        });
    });
}

function sendNetworkCommand(commandName, commandValue) {
    if (!websocket || websocket.readyState !== WebSocket.OPEN) {
        console.log("WebSocket not ready");
        return;
    }

    let payload = {
        cmd: commandName,
        value: commandValue
    };

    console.log("WS TX:", payload);
    websocket.send(JSON.stringify(payload));
}

function sendCommissioningCommand(action, count = null) {
    let strip = parseInt(document.getElementById('commission-strip').value);
    let led = parseInt(document.getElementById('commission-led').value);
    let color = hexColorToNumber(document.getElementById('commission-color').value);
    let interval = parseInt(document.getElementById('commission-interval').value);

    if (isNaN(interval) || interval < 30)
        interval = 120;

    let url = `/api/commissioning?action=${action}&strip=${strip}&led=${led}&color=${color}&interval=${interval}`;

    if (count !== null)
        url += `&count=${count}`;

    console.log("REST COMMISSIONING:", url);

    fetch(url)
        .then(response => response.json())
        .then(data => {
            console.log("REST COMMISSIONING RX:", data);
        })
        .catch(error => {
            console.log("Commissioning REST error:", error);
        });

}

function resetWiFi() {
    if (confirm("Deseja realmente apagar o Wi-Fi atual? O dispositivo reiniciará em modo de configuração.")) {
        fetch('/api/reset_wifi')
            .then(response => response.json())
            .then(() => {
                alert("Wi-Fi resetado. Conecte na rede 'Ripado Setup'.");
                window.location.reload();
            })
            .catch(() => {
                alert("Erro ao enviar comando.");
            });
    }
}

function hexColorToNumber(hexColor) {
    return parseInt(hexColor.replace('#', ''), 16);
}

function numberToHexColor(value) {
    return "#" + value.toString(16).padStart(6, "0");
}
