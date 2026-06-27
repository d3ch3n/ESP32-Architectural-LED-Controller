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