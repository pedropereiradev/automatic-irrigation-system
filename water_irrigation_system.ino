#include <LiquidCrystal.h>
#include <dht11.h>

#define DHT11PIN 7

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
dht11 DHT11;

// Pinos e constantes
const int PINO_SENSOR = A0;
const int PINO_RELE = 8;
const int VALOR_MAXIMO = 500;
const int VALOR_MINIMO = 200;
const int CONCENTRACAO_MINIMA = 30;
const int IRRIGATION_TIME = 10000; // 10 segundos

// Variáveis de controle
unsigned long previousDisplayUpdate = 0;
unsigned long irrigationStart = 0;
unsigned long systemStart = 0;
bool irrigating = false;
bool systemReady = false;
// int displayMode = 0; // 0=main, 1=details, 2=status
int currentScreen = 0;

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

void setup() {
  pinMode(PINO_SENSOR, INPUT);
  pinMode(PINO_RELE, OUTPUT);
  digitalWrite(PINO_RELE, LOW);
  
  lcd.begin(16, 2);
  
  // Criar caracteres customizados
  lcd.createChar(0, dropChar);
  lcd.createChar(1, plantChar);
  lcd.createChar(2, thermometerChar);
  
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
  
  int chk = DHT11.read(DHT11PIN);
  if (chk == DHTLIB_OK) {
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
  
  int chk = DHT11.read(DHT11PIN);
  float temperature = DHT11.temperature;
  float humidity = DHT11.humidity;
  
  // Alternar entre telas a cada 4 segundos
  unsigned long currentTime = millis();
  if (currentTime - previousDisplayUpdate >= 4000) {
    currentScreen = (currentScreen + 1) % 3;
    previousDisplayUpdate = currentTime;
    lcd.clear();
  }
  
  switch (currentScreen) {
    case 0: // Tela principal
      displayMainScreen(soilMoisture, temperature);
      break;
      
    case 1: // Tela de detalhes
      displayDetailsScreen(soilMoisture, humidity);
      break;
      
    case 2: // Tela de status
      displayStatusScreen();
      break;
  }
}

void displayMainScreen(int soilMoisture, float temperature) {
  // Linha 1: Umidade do solo
  lcd.setCursor(0, 0);
  lcd.write(1); // Ícone da planta
  lcd.print("Solo:");
  lcd.print(soilMoisture);
  lcd.print("%");
  
  // Barra de umidade visual
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
  
  // Linha 2: Temperatura
  lcd.setCursor(0, 1);
  lcd.write(2); // Ícone termômetro
  lcd.print("Temp:");
  lcd.print(temperature, 1);
  lcd.print("C");
  
  // Status da irrigação
  if (irrigating) {
    lcd.setCursor(12, 1);
    lcd.write(0); // Ícone gota
    lcd.print("ON");
  } else {
    lcd.setCursor(11, 1);
    lcd.print("OFF");
  }
}

void displayDetailsScreen(int soilMoisture, float humidity) {
  // Linha 1: Umidade do ar
  lcd.setCursor(0, 0);
  lcd.write(0); // Ícone gota
  lcd.print("Umid:");
  lcd.print(humidity, 1);
  lcd.print("%");
  
  // Linha 2: Status do solo
  lcd.setCursor(0, 1);
  if (soilMoisture < CONCENTRACAO_MINIMA) {
    lcd.print("Solo: SECO");
  } else if (soilMoisture > 70) {
    lcd.print("Solo: UMIDO");
  } else {
    lcd.print("Solo: NORMAL");
  }
}

void displayStatusScreen() {
  // Linha 1: Tempo de funcionamento
  lcd.setCursor(0, 0);
  unsigned long uptime = (millis() - systemStart) / 1000;
  lcd.print("Uptime:");
  if (uptime < 60) {
    lcd.print(uptime);
    lcd.print("s");
  } else if (uptime < 3600) {
    lcd.print(uptime / 60);
    lcd.print("min");
  } else {
    lcd.print(uptime / 3600);
    lcd.print("h");
  }
  
  // Linha 2: Status do sistema
  lcd.setCursor(0, 1);
  if (irrigating) {
    unsigned long remaining = (IRRIGATION_TIME - (millis() - irrigationStart)) / 1000;
    lcd.print("Irrigando:");
    lcd.print(remaining);
    lcd.print("s");
  } else {
    lcd.print("Monitorando...");
  }
}

void controlIrrigation() {
  int soilMoisture = analogRead(PINO_SENSOR);
  soilMoisture = map(soilMoisture, VALOR_MINIMO, VALOR_MAXIMO, 100, 0);
  soilMoisture = constrain(soilMoisture, 0, 100);
  
  // Iniciar irrigação se necessário
  if (!irrigating && soilMoisture < CONCENTRACAO_MINIMA) {
    digitalWrite(PINO_RELE, HIGH);
    irrigating = true;
    irrigationStart = millis();
    
    // Mostrar notificação
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("IRRIGANDO!");
    lcd.setCursor(0, 1);
    lcd.write(0);
    lcd.print(" Solo seco ");
    lcd.write(0);
    delay(1000);
    lcd.clear();
  }
  
  // Parar irrigação após o tempo definido
  if (irrigating && millis() - irrigationStart >= IRRIGATION_TIME) {
    digitalWrite(PINO_RELE, LOW);
    irrigating = false;
    
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
  
  delay(100); // Pequeno delay para estabilidade
}