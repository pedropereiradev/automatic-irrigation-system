# Sistema de Irrigação Automática com Arduino

## 🌱 Descrição do Projeto

Este sistema de irrigação automática foi desenvolvido para monitorar e controlar automaticamente a irrigação de plantas com base na umidade do solo. O sistema utiliza sensores para medir a umidade do solo e as condições ambientais (temperatura e umidade do ar), exibindo as informações em um display LCD e acionando automaticamente uma álvula solenóide para liberar o fluxo de água quando necessário.

### Características Principais:
- **Monitoramento contínuo** da umidade do solo
- **Controle automático** de irrigação
- **Display LCD** com informações em tempo real
- **Sensor de temperatura e umidade** ambiente
- **Sistema de cooldown** para evitar irrigação excessiva
- **Interface amigável** com ícones customizados

## ⚙️ Funcionalidades

### 🔧 Automação
- Irrigação automática quando umidade do solo < 30%
- Tempo de irrigação configurável (padrão: 10 segundos)
- Período de espera entre irrigações (padrão: 5 minutos)
- Verificação imediata na inicialização do sistema

### 📊 Monitoramento
- **Tela 1**: Temperatura e umidade do ar
- **Tela 2**: Umidade do solo com barra visual e status
- Alternância automática entre telas a cada 10 segundos
- Ícones customizados para melhor visualização

### 🛡️ Segurança
- Sistema de cooldown para evitar irrigação excessiva
- Verificação de sensores na inicialização
- Controle adequado do relé (HIGH=OFF, LOW=ON)
- Monitoramento via Serial para debug

## 🔧 Componentes Necessários

### Hardware Principal
| Componente | Quantidade | Especificação |
|------------|------------|---------------|
| Arduino Uno | 1 | Microcontrolador principal |
| Sensor DHT11 | 1 | Temperatura e umidade do ar |
| Sensor de Umidade do Solo | 1 | Analógico |
| Display LCD 16x2 | 1 | Com pinos diretos |
| Módulo Relé | 1 | 5V, 1 canal |
| Válvula Solenóide | 1 | 12V |
| Fonte de Alimentação | 1 | 12V para Arduino e Válvula Solenóide |

## 🔌 Esquema de Ligação

### Pinagem do Arduino
```
Arduino Uno → Componente
├── Pino 2  → LCD (D4)
├── Pino 3  → LCD (D5)
├── Pino 4  → LCD (D6)
├── Pino 5  → LCD (D7)
├── Pino 7  → DHT11 (DATA)
├── Pino 8  → Relé (IN)
├── Pino 11 → LCD (Enable)
├── Pino 12 → LCD (RS)
├── Pino A0 → Sensor de Umidade (Analógico)
├── 5V      → Alimentação sensores
└── GND     → Terra comum
```

### Ligações Detalhadas

#### DHT11 (Sensor de Temperatura/Umidade)
```
DHT11    → Arduino
VCC      → 5V
DATA     → Pino 7
GND      → GND
```

#### LCD 16x2
```
LCD      → Arduino
VSS      → GND
VDD      → 5V
V0       → Potenciômetro (contraste)
RS       → Pino 12
Enable   → Pino 11
D4       → Pino 5
D5       → Pino 4
D6       → Pino 3
D7       → Pino 2
A        → 5V (backlight)
K        → GND (backlight)
```

#### Sensor de Umidade do Solo
```
Sensor   → Arduino
VCC      → 5V
GND      → GND
A0       → Pino A0
```

#### Módulo Relé
```
Relé     → Arduino/Válvula
VCC      → 5V
GND      → GND
IN       → Pino 8
COM      → Fonte positiva
NO       → Válvula positiva
```

## 📥 Instalação

### 1. Preparação do Ambiente

#### Instalar Arduino IDE
1. Baixe a Arduino IDE do site oficial
2. Instale as bibliotecas necessárias:
   - `DHT sensor library` by Adafruit
   - `Adafruit Unified Sensor`
   - `LiquidCrystal` (já incluída)

#### Instalar Bibliotecas
```cpp
// No Arduino IDE:
// Sketch → Include Library → Manage Libraries
// Buscar e instalar:
// - "DHT sensor library" by Adafruit
// - "Adafruit Unified Sensor"
```

### 2. Montagem do Hardware

#### Ordem de Montagem
1. **Monte o LCD** primeiro e teste a exibição
2. **Conecte o DHT11** e verifique leituras
3. **Instale o sensor de umidade** no solo
4. **Configure o relé** com a bomba
5. **Teste cada componente** individualmente

### 3. Upload do Código
1. Conecte o Arduino ao computador
2. Abra o código no Arduino IDE
3. Selecione a porta correta (Tools → Port)
4. Faça o upload (Ctrl+U)

## ⚙️ Configuração

### Parâmetros Principais
```cpp
// Configurações no código
const int VALOR_MAXIMO = 500;        // Sensor solo seco
const int VALOR_MINIMO = 200;        // Sensor solo úmido
const int CONCENTRACAO_MINIMA = 30;  // Umidade mínima (%)
const int IRRIGATION_TIME = 10000;   // Tempo irrigação (ms)
const int COOLDOWN_TIME = 300000;    // Espera entre irrigações (ms)
```

### Personalização
- **Tempo de irrigação**: Ajuste `IRRIGATION_TIME` (em milissegundos)
- **Umidade mínima**: Modifique `CONCENTRACAO_MINIMA` (0-100%)
- **Cooldown**: Altere `COOLDOWN_TIME` (em milissegundos)
- **Alternância de telas**: Mude o valor 10000 na função `updateDisplay()`

## 🚀 Como Usar

### Primeira Inicialização
1. **Conecte** todos os componentes conforme o esquema
2. **Ligue** o sistema - verá tela de inicialização
3. **Sistema testa** os sensores automaticamente
4. **Aguarde** "Sistema Pronto!" aparecer
5. **Verificação imediata** da umidade do solo

### Operação Normal
- **Tela 1**: Mostra temperatura e umidade do ar
- **Tela 2**: Mostra umidade do solo e status
- **Irrigação automática** quando solo < 30% umidade
- **Cooldown de 5 minutos** após cada irrigação

### Status do Sistema
| Status | Significado |
|--------|-------------|
| `IRRIGANDO` | Válvula acionada, irrigando |
| `Aguard: Xmin` | Tempo restante para próxima irrigação |
| `Aguard: Xs` | Menos de 1 minuto restante |
| `SECO - PRONTO` | Solo seco, pronto para irrigar |
| `UMIDO` | Solo com boa umidade (>70%) |
| `NORMAL` | Solo com umidade adequada |

## 🔧 Calibração

### Calibração do Sensor de Umidade

#### Procedimento
1. **Abra o Serial Monitor** (Tools → Serial Monitor)
2. **Observe os valores** "Sensor A0 = XXX"
3. **Teste em solo seco**: Anote o valor máximo
4. **Teste em solo úmido**: Anote o valor mínimo

#### Ajuste no Código
```cpp
// Substitua pelos valores obtidos
const int VALOR_MAXIMO = XXX;  // Valor solo seco
const int VALOR_MINIMO = XXX;  // Valor solo úmido
```

### Exemplo de Calibração
```
Solo Seco:    Sensor A0 = 850
Solo Úmido:   Sensor A0 = 300

Configuração:
VALOR_MAXIMO = 850
VALOR_MINIMO = 300
```

## 💻 Código Fonte

### Estrutura do Código
```cpp
// Principais funções:
setup()           // Inicialização do sistema
initializeSystem() // Tela de boot e testes
updateDisplay()   // Atualização do LCD
controlIrrigation() // Controle da irrigação
displayClimateScreen() // Tela do clima
displaySoilScreen()    // Tela do solo
loop()            // Loop principal
```

### Variáveis Importantes
```cpp
bool irrigating           // Status da irrigação
bool firstIrrigationCheck // Primeira verificação
unsigned long lastIrrigationEnd // Última irrigação
int currentScreen        // Tela atual (0 ou 1)
```

### Constantes Configuráveis
```cpp
IRRIGATION_TIME   // Tempo de irrigação (ms)
COOLDOWN_TIME     // Tempo entre irrigações (ms)
CONCENTRACAO_MINIMA // Umidade mínima (%)
VALOR_MAXIMO      // Calibração sensor seco
VALOR_MINIMO      // Calibração sensor úmido
```

## 📝 Notas Importantes

### Segurança
- ⚠️ **Nunca** deixe o sistema sem supervisão por longos períodos
- ⚠️ **Proteja** componentes eletrônicos da água
- ⚠️ **Use** fonte adequada para a válvula

### Limitações
- Sistema adequado para plantas pequenas/médias
- Requer calibração específica para cada tipo de solo
- Não adequado para uso externo sem proteção
- Necessita supervisão regular

### Melhorias Futuras
- [ ] Conectividade WiFi para monitoramento remoto
- [ ] Log de dados em cartão SD
- [ ] Aplicativo mobile
- [ ] Múltiplas zonas de irrigação
- [ ] Sensor de pH do solo

---

**Versão:** 1.0  
**Data:** Junho 2025  
**Compatibilidade:** Arduino Uno/Nano/Pro Mini  
**Licença:** Código aberto para uso educacional