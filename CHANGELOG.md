# Changelog

Todos os marcos notáveis e modificações estruturais desenvolvidas no firmware do projeto **Ripado OS** serão documentados e catalogados cronologicamente neste arquivo.

O formato é baseado no padrão [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) e este projeto adota estritamente o [Semantic Versioning (SemVer) v2.0.0](https://semver.org/).

---

## [0.2.0] - 2026-06-27

### Added
- Desenvolvimento do módulo de persistência e gerência de arquivos `StorageManager` (`src/Storage/`).
- Integração e montagem automática da partição de memória Flash utilizando o driver nativo **LittleFS**.
- Criação do manifesto estruturado `data/config.json` para desacoplamento de propriedades físicas e de rede.
- Suporte a buffers dinâmicos na pilha estática da stack através da biblioteca `ArduinoJson` (`StaticJsonDocument<768>`).

### Changed
- Refatoração do fluxo de boot do sistema no `main.cpp` para carregar as dimensões, GPIOs e propriedades geométricas do JSON em disco antes de alocar a memória física dos LEDs.

### Fixed
- Resolução do erro crítico de barramento ocupado (`Port Busy / Resource Unavailable`) no script de deploy (`esptool.py`), tratando o encerramento manual da escuta do monitor serial (`Ctrl + C`) antes de disparar o comando de upload de sistema de arquivos (`uploadfs`).

---

## [0.1.0-alpha] - 2026-06-27

### Added
- Configuração e homologação do ambiente de desenvolvimento profissional no **PlatformIO** para a placa ESP32 Dev Module.
- Arquivo de isolamento de lixo eletrônico, binários e caches de IDE através de regras abrangentes por diretório no `.gitignore`.
- Desenvolvimento do motor gráfico isolado de hardware `LedController` (`src/Led/`), abstraindo e encapsulando dependências da biblioteca FastLED.
- Criação do motor assíncrono e não-bloqueante de animações `AnimationEngine` (`src/Animation/`) controlado por uma Máquina de Estados Finita (FSM) determinística.
- Desenvolvimento da álgebra linear de alinhamento geométrico (`Config_CalculateGeometry`), compensando tamanhos diferentes de fita para garantir a chegada síncrona dos feixes de luz no teto do ripado.
- Orquestração do laço de execução principal com rotina de autodiagnóstico automatizada por linha do tempo em bancada (`src/main.cpp`).

---

## [0.0.1-init] - 2026-06-26

### Added
- Inicialização oficial do repositório Git do projeto.
- Definição da árvore de diretórios padrão de engenharia corporativa (`src/`, `data/`, `docs/`).
- Criação dos documentos de governança primordiais (`README.md`).

## [0.2.0] - 2026-06-27
### Added
- Módulo `StorageManager` encapsulando rotas assíncronas de leitura/escrita via LittleFS.
- Arquivo de manifesto dinâmico `data/config.json` para parametrização física.
- Desacoplamento completo dos tamanhos e GPIOs das fitas em relação ao código C++.

### Fixed
- Resolução de travamento e conflito de barramento serial (Port Busy) através da liberação de canal do monitor de depuração.

## [0.3.0] - 2026-06-27

### Added
- Módulo assíncrono de conectividade `WebService` (`src/Web/`).
- Barramento de transmissão bidirecional assíncrono via protocolo **WebSockets** (`/ws`).
- Suporte a ponto de acesso captivo dinâmico (`WiFiManager`) isolado de hardcode de rede.
- Ativação do resolvedor de nomes local por protocolo **mDNS** (`ripado.local`).