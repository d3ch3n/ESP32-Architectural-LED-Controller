## Sprint 1: Motor Gráfico & FSM [2026-06-27]
### 1. Implementações
- Desenvolvido o motor assíncrono FSM em `AnimationEngine.cpp` tratando 3 estados de transição complexos.
- Criado o algoritmo dinâmico de compensação `passoVarredura - offset` garantindo chegada síncrona no teto.
- Encapsulado a biblioteca FastLED dentro do driver isolador `LedController`.

### 2. Decisões de Arquitetura
- Abandono de flags booleanas soltas, concentrando toda a integridade lógica na FSM `SystemState`.
- Uso do tipo explícito `int16_t` na indexação para prevenir estouro negativo (`underflow`) durante laços de recuo.
- Inclusão do limitador `applyBrightnessSafety` atuando na última camada antes da injeção física de corrente.

## Sprint 2: Armazenamento Estruturado & LittleFS [2026-06-27]
### 1. Implementações
- Montagem bem-sucedida do driver `LittleFS` com capacidade de auto-formatação em caso de falha de partição.
- Implementação de parse dinâmico usando buffers estáticos na pilha stack (`StaticJsonDocument<768>`) para mitigar a fragmentação de memória Heap.
- Ajuste das diretivas de inicialização no `main.cpp` para carregar a geometria das fitas direto do disco antes da instanciação do FastLED.

### 2. Problemas Encontrados & Soluções
- **Bug de Porta Presa (Port Busy / Resource Unavailable):** O monitor serial local travava a escuta do chip `/dev/cu.SLAB_USBtoUART`, impedindo a ferramenta `esptool.py` de abrir o canal de gravação do LittleFS (`uploadfs`). Resolvido adicionando rotina operacional de derrubada do monitor (`Ctrl + C`) antes de disparar novos builds de disco.

## Sprint 3: Conectividade Assíncrona & Web Sockets [2026-06-27]
### 1. Implementações
- Criado o módulo `WebService.cpp` isolando a pilha TCP/IP assíncrona do loop principal de renderização.
- Ativado o canal de comunicação full-duplex via **WebSockets** (`/ws`) para tráfego instantâneo de comandos e telemetria.
- Integrado o driver `WiFiManager` para provisionamento de rede via portal cativo dinâmico ("Ripado Setup"), eliminando credenciais salvas no código.
- Ativado o resolvedor de nomes local por protocolo **mDNS** para acesso amigável através do link `http://ripado.local`.

### 2. Decisões de Arquitetura
- Processamento assíncrono total: uso do método `cleanupClients()` no loop de background para purga de conexões mortas sem bloquear a CPU.
- Acoplamento limpo: mensagens recebidas via rede injetam comandos diretamente nas assinaturas públicas do `AnimationEngine` (`Animation_StartOpening`, `Animation_StartColorChange`), mantendo o isolamento de escopo.

## Sprint 4: O Painel SPA (Interface Web) [2026-06-27]
### 1. Implementações
- Construída a arquitetura de Front-End Single Page Application (SPA) baseada em HTML5, CSS3 estrutural e JavaScript Vanilla puro.
- Desenvolvido isolamento completo de recursos estáticos servidos diretamente via cache LittleFS pelo `AsyncWebServer`.
- Implementada a engenharia de sincronismo reativo WebSocket: atualizações de estado aplicadas em um painel refletem instantaneamente nos seletores de todos os clientes conectados via broadcast.

### 2. Decisões de Arquitetura
- Abandono de frameworks pesados (React/Vue) para eliminar sobrecarga de requisições HTTP e otimizar o tempo de renderização na CPU limitada do ESP32.
- Conversão binária de strings Hex de cor para inteiros numéricos de 24-bits diretamente na camada do cliente (JavaScript), reduzindo o uso de strings e buffers na memória ram do microcontrolador.

### 3. Status de Homologação em Rede [2026-06-27]
- **Resultado:** SUCESSO DEFINITIVO.
- **Métricas:** Conexão assíncrona estabelecida com o AP local sob o IP `192.168.0.38`. WebSocket ativo canalizando pacotes de controle bidirecional (`/ws`) em tempo real.
- **Estabilidade:** Consumo de memória Heap estável, mDNS operando em `http://ripadomestre.local`. Isolamento de escopo garantido (a renderização gráfica dos LEDs não sofreu jitter ou congelamento durante as requisições HTTP).

## Sprint 6: Painel de Engenharia & LittleFS Writer [2026-06-27]
### 1. Implementações
- Desenvolvida a interface visual expandida com navegação por abas assíncronas (Control / Installer).
- Criada a API REST HTTP POST `/api/config` no Core para recepção dinâmica de parâmetros de hardware.
- Implementado o motor **"Led Finder"** via barramento WebSocket, permitindo injeção de pulso de luz no último pixel para calibração de contagem de fitas em tempo real.
- Acoplada a rotina de auto-salvamento via `StorageManager` seguida de Reboot controlado por software (`ESP.restart()`).

### 2. Decisões de Arquitetura
- Migração completa para a estrutura moderna `JsonDocument` (ArduinoJson v7), eliminando todos os warnings de depreciação de memória da pilha (Stack) gerados pela v6.
- Correção crítica de hardware: remanejamento do barramento lógico da Fita 2 do pino de boot GPIO 0 para o pino seguro GPIO 16, eliminando interrupções elétricas no momento de pico de RF do Wi-Fi.