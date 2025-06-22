#include <DHT.h>
#include <DHT_U.h>
#include <LiquidCrystal.h>

#define DHTPIN 7
#define DHTTYPE DHT11

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
DHT dht(DHTPIN, DHTTYPE);

// Pinos e constantes
const int PINO_SENSOR = A0;
const int PINO_RELE = 8;
const int VALOR_MAXIMO = 500;
const int VALOR_MINIMO = 200;
const int CONCENTRACAO_MINIMA = 30;
const unsigned long IRRIGATION_TIME = 10000UL; // 10 segundos

// Variáveis de controle
unsigned long previousDisplayUpdate = 0;
unsigned long irrigationStart = 0;
unsigned long lastIrrigationEnd = 0;  // Quando a última irrigação terminou
unsigned long systemStart = 0;
bool irrigating = false;
bool systemReady = false;
bool firstIrrigationCheck = true;
int currentScreen = 0;

// Constantes de tempo
const unsigned long COOLDOWN_TIME = 300000UL;  // 5 minutos (300000ms) de espera após irrigação

// Caracteres customizados
byte dropChar[8] = {
  0b00100,
  0b00100,
  0b01010,
  0b01010,
  0b10001,
  0b10001,
  0b10001,
  0b01110
};

byte plantChar[8] = {
  0b00000,
  0b01010,
  0b01010,
  0b11111,
  0b01110,
  0b00100,
  0b00100,
  0b01110
};

byte thermometerChar[8] = {
  0b00100,
  0b01010,
  0b01010,
  0b01110,
  0b01110,
  0b11111,
  0b11111,
  0b01110
};

byte degreeChar[8] = {
  0b01000,
  0b10100,
  0b01000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

void setup() {
  Serial.begin(9600); // Para debug
  
  pinMode(PINO_SENSOR, INPUT);
  pinMode(PINO_RELE, OUTPUT);
  digitalWrite(PINO_RELE, HIGH);
  
  lcd.begin(16, 2);
  
  // Inicializar DHT
  dht.begin();
  
  // Criar caracteres customizados
  lcd.createChar(0, dropChar);
  lcd.createChar(1, plantChar);
  lcd.createChar(2, thermometerChar);
  lcd.createChar(3, degreeChar);
  
  Serial.println("=== SISTEMA DE IRRIGACAO ===");
  Serial.println("Para calibrar:");
  Serial.println("1. Solo seco = VALOR_MAXIMO");
  Serial.println("2. Solo umido = VALOR_MINIMO");
  Serial.println("=============================");
  
  systemStart = millis();
  initializeSystem();
}

void initializeSystem() {
  // Tela de inicialização
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("SISTEMA DE");
  lcd.setCursor(3, 1);
  lcd.print("IRRIGACAO");
  delay(2000);
  
  // Animação de carregamento
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Carregando...");
  
  for (int i = 0; i < 16; i++) {
    lcd.setCursor(i, 1);
    lcd.print("=");
    delay(100);
  }
  
  // Verificar sensores
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Testando DHT11");
  
  // Teste do DHT11
  float testTemp = dht.readTemperature();
  float testHum = dht.readHumidity();
  
  if (!isnan(testTemp) && !isnan(testHum)) {
    lcd.setCursor(0, 1);
    lcd.print("DHT11: OK");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("DHT11: ERRO");
  }
  delay(1500);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sistema Pronto!");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");
  delay(1500);
  
  systemReady = true;
}

void updateDisplay() {
  // Ler sensores
  int soilMoisture = analogRead(PINO_SENSOR);
  soilMoisture = map(soilMoisture, VALOR_MINIMO, VALOR_MAXIMO, 100, 0);
  soilMoisture = constrain(soilMoisture, 0, 100);
  
  // Ler temperatura e umidade
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  // Verificar se as leituras são válidas
  if (isnan(temperature)) {
    temperature = 0.0;
  }
  if (isnan(humidity)) {
    humidity = 0.0;
  }
  
  // Alternar entre telas a cada 10 segundos
  unsigned long currentTime = millis();
  if (currentTime - previousDisplayUpdate >= 10000) {
    currentScreen = (currentScreen + 1) % 2;
    previousDisplayUpdate = currentTime;
    lcd.clear();
  }
  
  switch (currentScreen) {
    case 0: // Tela de clima (temperatura e umidade do ar)
      displayClimateScreen(temperature, humidity);
      break;
      
    case 1: // Tela do solo (umidade e status)
      displaySoilScreen(soilMoisture);
      break;
  }
}

void displayClimateScreen(float temperature, float humidity) {
  // Linha 1: Temperatura
  lcd.setCursor(0, 0);
  lcd.write((uint8_t)2); // Ícone termômetro
  lcd.print(" Temp: ");
  lcd.print(temperature, 1);
  lcd.write((uint8_t)3); // Ícone grau
  lcd.print("C");
  
  // Linha 2: Umidade do ar
  lcd.setCursor(0, 1);
  lcd.write((uint8_t)0); // Ícone gota
  lcd.print(" Umid: ");
  lcd.print(humidity, 1);
  lcd.print("%");
}

void displaySoilScreen(int soilMoisture) {
  // Linha 1: Umidade do solo com barra visual
  lcd.setCursor(0, 0);
  lcd.write((uint8_t)1); // Ícone da planta
  lcd.print(" Solo: ");
  lcd.print(soilMoisture);
  lcd.print("%");
  
  // Barra de umidade visual na primeira linha
  int barLength = map(soilMoisture, 0, 100, 0, 6);
  lcd.setCursor(10, 0);
  lcd.print("[");
  for (int i = 0; i < 6; i++) {
    if (i < barLength) {
      lcd.print("=");
    } else {
      lcd.print(" ");
    }
  }
  
  // Linha 2: Status do solo e irrigação
  lcd.setCursor(0, 1);
  
  bool inCooldown = false;
  if (lastIrrigationEnd > 0) {
    unsigned long timeSinceLastIrrigation = millis() - lastIrrigationEnd;
    inCooldown = timeSinceLastIrrigation < COOLDOWN_TIME;
  }
  
  if (irrigating) {
    lcd.print("IRRIGANDO");
    lcd.write((uint8_t)0);
  } else if (inCooldown) {
    unsigned long timeSinceLastIrrigation = millis() - lastIrrigationEnd;
    
    // Verificar se ainda está dentro do período de cooldown
    if (timeSinceLastIrrigation < COOLDOWN_TIME) {
      unsigned long remainingTime = COOLDOWN_TIME - timeSinceLastIrrigation;
      
      if (remainingTime > 60000) {
        // Mostrar em minutos se for mais de 1 minuto
        unsigned long remainingMin = remainingTime / 60000;
        lcd.print("Aguard:");
        lcd.print(remainingMin);
        lcd.print("min");
      } else {
        // Mostrar em segundos se for menos de 1 minuto
        unsigned long remainingSec = remainingTime / 1000;
        lcd.print("Aguard:");
        lcd.print(remainingSec);
        lcd.print("s");
      }
    } else {
      // Tempo de cooldown já passou
      if (soilMoisture < CONCENTRACAO_MINIMA) {
        lcd.print("SECO - PRONTO");
      } else {
        lcd.print("NORMAL");
      }
    }
  } else if (soilMoisture < CONCENTRACAO_MINIMA) {
    lcd.print("SECO - PRONTO");
  } else if (soilMoisture > 70) {
    lcd.print("UMIDO");
  } else {
    lcd.print("NORMAL");
  }
}

void controlIrrigation() {
  int soilMoisture = analogRead(PINO_SENSOR);
  soilMoisture = map(soilMoisture, VALOR_MINIMO, VALOR_MAXIMO, 100, 0);
  soilMoisture = constrain(soilMoisture, 0, 100);
  
  bool inCooldown = false;
  if (!firstIrrigationCheck && lastIrrigationEnd > 0) {
    unsigned long timeSinceLastIrrigation = millis() - lastIrrigationEnd;
    inCooldown = timeSinceLastIrrigation < COOLDOWN_TIME;
  }
  
  // Debug do controle de irrigação
  static unsigned long lastDebugPrint = 0;
  if (millis() - lastDebugPrint >= 5000) { // Debug a cada 5 segundos
    lastDebugPrint = millis();
    Serial.print("Solo: ");
    Serial.print(soilMoisture);
    Serial.print("% | Irrigando: ");
    Serial.print(irrigating ? "SIM" : "NAO");
    
    if (lastIrrigationEnd > 0) {
      unsigned long timeSinceLastIrrigation = millis() - lastIrrigationEnd;
      Serial.print(" | Tempo desde ultima: ");
      Serial.print(timeSinceLastIrrigation / 1000);
      Serial.print("s");
      
      if (inCooldown) {
        unsigned long remainingCooldown = (COOLDOWN_TIME - timeSinceLastIrrigation) / 1000;
        Serial.print(" | Cooldown restante: ");
        Serial.print(remainingCooldown);
        Serial.print("s");
      } else {
        Serial.print(" | Cooldown terminado");
      }
    }
    
    if (firstIrrigationCheck) {
      Serial.print(" | Primeira verificacao - sem cooldown");
    }
    
    Serial.println();
  }
  
  // Iniciar irrigação se necessário
  if (!irrigating && soilMoisture < CONCENTRACAO_MINIMA && !inCooldown) {
    digitalWrite(PINO_RELE, LOW);  // LOW para ligar o relé
    irrigating = true;
    irrigationStart = millis();
    firstIrrigationCheck = false;  // Marcar que primeira verificação foi feita
    
    Serial.println(">>> INICIANDO IRRIGACAO <<<");
    Serial.print("Umidade do solo: ");
    Serial.print(soilMoisture);
    Serial.println("%");
    Serial.print("Tempo atual (millis): ");
    Serial.println(millis());
    
    // Mostrar notificação
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("IRRIGANDO!");
    lcd.setCursor(0, 1);
    lcd.write((uint8_t)0);
    lcd.print(" Solo seco ");
    lcd.write((uint8_t)0);
    delay(1000);
    lcd.clear();
  }
  
  // Parar irrigação após o tempo definido
  if (irrigating && millis() - irrigationStart >= IRRIGATION_TIME) {
    digitalWrite(PINO_RELE, HIGH);  // HIGH para desligar o relé
    irrigating = false;
    lastIrrigationEnd = millis(); // Marcar quando a irrigação terminou
    
    Serial.println(">>> IRRIGACAO FINALIZADA <<<");
    Serial.print("Iniciando cooldown de ");
    Serial.print(COOLDOWN_TIME / 60000);
    Serial.println(" minutos");
    Serial.print("Timestamp fim irrigacao: ");
    Serial.println(lastIrrigationEnd);
    
    // Mostrar notificação
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("IRRIGACAO");
    lcd.setCursor(2, 1);
    lcd.print("CONCLUIDA!");
    delay(1500);
    lcd.clear();
  }
}

void loop() {
  if (!systemReady) return;
  
  // Atualizar display a cada 1 segundo
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate >= 1000) {
    lastUpdate = millis();
    updateDisplay();
  }
  
  // Controlar irrigação
  controlIrrigation();
  
  // Print dos valores do sensor para calibração
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 2000) { // Print a cada 2 segundos
    lastPrint = millis();
    int rawValue = analogRead(PINO_SENSOR);
    Serial.print("Sensor A0 = ");
    Serial.println(rawValue);
  }
  
  delay(100); // Pequeno delay para estabilidade
}