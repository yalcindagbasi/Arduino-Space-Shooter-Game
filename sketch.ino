#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "TM1637Display.h"
//tanımlamalar
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT);

#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_D1  37

#define BT_LEFT 11
#define BT_RIGHT 12
#define BT_FIRE 10

#define LED_BLT1 7
#define LED_BLT2 6
#define LED_BLT3 5
#define LED_HP1 2
#define LED_HP2 3
#define LED_HP3 4

#define BUZZER 13

#define CLK 9
#define DIO 8
TM1637Display segdisplay(CLK, DIO);

int game[16][8];
#define unit 8
byte ammo = 3;
char ship = '>';
char bullet = '-';
char meteor = '*';
char junk = 'x';
int ship_pos_x;
int ship_pos_y;
byte life = 3;
int skor = 0;

byte difficulty = 1;
byte meteorRate = 3;
byte junkRate = 9;
byte refillRate = 14;

boolean lightMode = false;
int ldrValue = 30;

int menuSelection = 0;
int gameState = 0; // 0: Ana menü, 1: Oyun,
//tanımlamalar sonu
void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  segdisplay.setBrightness(7);

  //pinlerin görevi belirlenmesi
  pinMode(BT_LEFT, INPUT_PULLUP);
  pinMode(BT_RIGHT, INPUT_PULLUP);
  pinMode(BT_FIRE, INPUT_PULLUP);
  pinMode(LED_BLT1, OUTPUT);
  pinMode(LED_BLT2, OUTPUT);
  pinMode(LED_BLT3, OUTPUT);
  pinMode(LED_HP1, OUTPUT);
  pinMode(LED_HP2, OUTPUT);
  pinMode(LED_HP3, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  //ekran 128x64 ama matris 16x8 olduğu için matrisin her birimi oled ekranda
  //8 pixele denk geliyor
  ship_pos_x = 0 * unit;
  ship_pos_y = 4 * unit;

  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0, 4 * unit);
  display.println(ship);

  display.display();
  delay(1000);
}
void restart() {//oyun bittiğinde tüm ayarlar eski haline getirilir.
  ship_pos_x = 0 * unit;
  ship_pos_y = 4 * unit;

  display.setCursor(0, 4 * unit);
  display.println(ship);

  life = 3;
  ammo = 3;
  skor = 0;

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 16; j++) {
      game[j][i] = 0;//matris temizleme işlemi
    }
  }

  gameState = 0;
  mainMenu();
}
void moveShip() {//geminin hareket fonksiyonu
  game[ship_pos_x / unit][ship_pos_y / unit] = 0;
  if (digitalRead(BT_FIRE) == 0  && ship_pos_x != OLED_WIDTH - unit && ammo > 0) {
    tone(BUZZER, NOTE_DS5);
    delay(10);
    noTone(BUZZER);
    game[ship_pos_x / unit + 1][ship_pos_y / unit] = 4;
    ammo--;
  }
  if (digitalRead(BT_RIGHT) == 0 && digitalRead(BT_LEFT) == 1 && ship_pos_y != OLED_HEIGHT - unit) {
    ship_pos_y += unit;
  }
  if (digitalRead(BT_LEFT) == 0 && digitalRead(BT_RIGHT) == 1 && ship_pos_y != 0 * unit) {
    ship_pos_y -= unit;
  }


  game[ship_pos_x / unit][ship_pos_y / unit] = 1;
}
void spawnMeteor() {//rastgele engel doğuran fonksiyon
  int ran = random() % 100;

  if (ran < meteorRate) {//meteorRate= meteor çıkma olasılığı
    int ran = random() % 8;
    game[15][ran] = 2;

  }
  else if (ran >= meteorRate && ran < junkRate) {//junkrate= çöp çıkma olasılığı
    int ran = random() % 8;
    game[15][ran] = 3;
  }
}

void moveBlock(int y, int x) {//engelleri hareket ettiren fonksiyon
  if (x == 0) { //engel oyun alanının sonuna geldi.
    game[x][y] = 0;
    skor += 1;
    if (skor % refillRate == 0) {//belirli skorlarda can ve mermi eklenir.
      addAmmoHP();
    }
  }
  else if (game[x - 1][y] == 1) { //engel gemiye çarptı
    tone(BUZZER, 200);
    delay(50);
    noTone(BUZZER);
    life -= 1;
    game[x][y] = 0;
  }
  else if (game[x - 1, y] == 4) { //engel mermiye çarptı
    game[x][y] = 0;
    game[x - 1][y] = 0;
  }
  else {
    if (game[x][y] == 2) { //meteorsa bi ileri taşı
      game[x][y] = 0;
      game[x - 1][y] = 2;
    }
    else if (game[x][y] == 3) { //uzay çöpüyse bi ileri taşı
      game[x][y] = 0;
      game[x - 1][y] = 3;
    }
  }
}
void moveBullet(int y, int x) {//mermiyi hareket ettiren fonksiyon.
  if (x == 15) { //mermi oyun alanının sonuna geldi.
    game[x][y] = 0;
  }
  else if (game[x + 1][y] == 2 || game[x + 1][y] == 3) { //mermi engele çarptı
    game[x + 1][y] = 0;
    game[x][y] = 0;
  }
  else {//normal mermi hareketi
    game[x][y] = 0;
    game[x + 1][y] = 4;
  }
}
void printAll() {//her şeyi oled ekrana yazdırır. ve oyun akışını sağlar.
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 16; j++) {
      display.setCursor(j * unit, i * unit);
      switch (game[j][i]) {
        case 0:
          break;
        case 1:
          display.print(ship);
          break;
        case 2:
          moveBlock(i, j);
          display.print(meteor);
          break;
        case 3:
          moveBlock(i, j);
          display.print(junk);
          break;
        case 4:
          moveBullet(i, j);
          display.print(bullet);
          j += 1;
          break;
        default:
          break;
      }
    }
  }

  display.setCursor(12 * unit, 7 * unit);
  display.print(skor);
  display.print(':');
  display.print(life);
}
void checkLEDs() {//ledleri durumuna göre yakar.
  switch (ammo) {
    case 0:
      digitalWrite(LED_BLT1, LOW);
      digitalWrite(LED_BLT2, LOW);
      digitalWrite(LED_BLT3, LOW);
      break;
    case 1:
      digitalWrite(LED_BLT1, HIGH);
      digitalWrite(LED_BLT2, LOW);
      digitalWrite(LED_BLT3, LOW);
      break;
    case 2:
      digitalWrite(LED_BLT1, HIGH);
      digitalWrite(LED_BLT2, HIGH);
      digitalWrite(LED_BLT3, LOW);
      break;
    case 3:
      digitalWrite(LED_BLT1, HIGH);
      digitalWrite(LED_BLT2, HIGH);
      digitalWrite(LED_BLT3, HIGH);
      break;
    default:
      break;
  }
  switch (life) {
    case 0:
      digitalWrite(LED_HP1, LOW);
      digitalWrite(LED_HP2, LOW);
      digitalWrite(LED_HP3, LOW);
      break;
    case 1:
      digitalWrite(LED_HP1, HIGH);
      digitalWrite(LED_HP2, LOW);
      digitalWrite(LED_HP3, LOW);
      break;
    case 2:
      digitalWrite(LED_HP1, HIGH);
      digitalWrite(LED_HP2, HIGH);
      digitalWrite(LED_HP3, LOW);
      break;
    case 3:
      digitalWrite(LED_HP1, HIGH);
      digitalWrite(LED_HP2, HIGH);
      digitalWrite(LED_HP3, HIGH);
      break;
    default:
      break;
  }
}
void gameOver() {//oyun sonu ekranı
  display.clearDisplay();
  if (lightMode) {
    display.fillScreen(WHITE);
  }

  segdisplay.clear();
  segdisplay.showNumberDec(skor, false, 3, 0);

  display.setTextSize(2);
  display.setCursor(0, 16);
  display.println("OYUN BITTI");
  display.setCursor(16, 32);
  display.print("SKOR:");
  display.print(skor);
  display.display();

  tone(BUZZER, NOTE_DS5);
  delay(300);
  tone(BUZZER, NOTE_D5);
  delay(300);
  tone(BUZZER, NOTE_CS5);
  delay(300);
  for (byte i = 0; i < 10; i++) {
    for (int pitch = -10; pitch <= 10; pitch++) {
      tone(BUZZER, NOTE_C5 + pitch);
      delay(5);
    }
  }
  noTone(BUZZER);
  
  delay(5000);
  display.setTextSize(1);
  restart();
}
void addAmmoHP() {//can mermi ekler
  if (life != 3) {
    life++;
  }
  if (ammo != 3) {
    ammo++;
  }
}
void gameloop() {//oyun ana döngüsü
  display.clearDisplay();
  ldrValue = analogRead(A0);
  if (ldrValue < 700) {//ışık sensörü kontrolü
    lightMode = true;//beyaz arka plan için
    display.setTextColor(BLACK);
    display.fillScreen(WHITE);
  }
  else {//siyah arka plan için
    lightMode = false;
    display.fillScreen(BLACK);
    display.setTextColor(WHITE);
  }


  moveShip();
  spawnMeteor();
  printAll();
  /*display.setCursor(0,0);
    for(int i=0;i<8;i++){
    for(int j=0;j<16;j++){
      display.print(game[j][i]);

    }
    display.print('\n');
    }*/

  display.display();
  segdisplay.showNumberDec(skor, false);
  checkLEDs();
  if (life <= 0) {
    gameOver();
  }
  delay(50);
}
void mainMenu() {//ana ekran fonksiyonu
  while (true) {
    switch (gameState) {
      case 0:
        // Ana menü
        if (digitalRead(BT_LEFT) == LOW) {
          menuSelection = (menuSelection == 0) ? 1 : 0;
          delay(200);
        }
        if (digitalRead(BT_RIGHT) == LOW) {
          menuSelection = (menuSelection == 0) ? 1 : 0;
          delay(200);
        }
        if (digitalRead(BT_FIRE) == LOW) {
          if (menuSelection == 0) {
            gameState = 1;
            delay(500);
            gameloop();
          } else {
            difficulty = (difficulty == 1) ? 2 : 1;
            meteorRate = (difficulty == 1) ? 3 : 7;
            junkRate = (difficulty == 1) ? 9 : 21;
            refillRate = (difficulty == 1) ? 14 : 28;
            delay(200);
          }
        }
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        if (menuSelection == 0 && millis() % 100 <= 50)
          display.setTextColor(WHITE);
        if (menuSelection == 0) {
          display.drawRect(1, 1, 126, 11, WHITE);
        }
        else {
          display.drawRect(1, 11, 126, 11, WHITE);
        }
        display.setCursor(3, 3);
        display.println("Baslat");
        display.setCursor(3, 13);
        display.println((difficulty == 1) ? "Zorluk: Kolay" : "Zorluk: Zor");
        display.display();
        break;
      case 1:

        gameloop();
        break;
    }
  }
}
void loop() {
  mainMenu();
  gameloop();
}
