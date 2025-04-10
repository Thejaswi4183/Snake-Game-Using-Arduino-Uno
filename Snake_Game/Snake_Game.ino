#include <U8g2lib.h>
#include <Wire.h>

// SH1106 128x64 using hardware I2C
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Joystick pins
#define pinX A0
#define pinY A1
#define pinBtn 2

#define RIGHT 0
#define LEFT  1
#define UP    2
#define DOWN  3

// Game variables
uint8_t snakeX[100], snakeY[100];
uint8_t snakeLength = 3;
uint8_t direction = RIGHT;
uint8_t foodX = 0, foodY = 0;
bool foodEaten = true;
bool gameOver = false;
int score = 0;

unsigned long lastUpdateTime = 0;
const int updateInterval = 150; // Game speed (ms)

// Debounce variables
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 1000; // milliseconds debounce time
bool buttonPressed = false;

void setup() {
  u8g2.begin();
  randomSeed(analogRead(A2));

  pinMode(pinX, INPUT);
  pinMode(pinY, INPUT);
  pinMode(pinBtn, INPUT_PULLUP);

  // Initialize snake in center
  uint8_t startX = 64;
  uint8_t startY = 32;
  for (int i = 0; i < snakeLength; i++) {
    snakeX[i] = startX - i * 4;
    snakeY[i] = startY;
  }

  generateFood();
}

void loop() {
  readJoystick();

  if (!gameOver && millis() - lastUpdateTime >= updateInterval) {
    lastUpdateTime = millis();
    moveSnake();
    checkCollision();
    drawEverything();
  } else if (gameOver) {
    drawGameOver();

    // Handle joystick button press to reset the game
    if (digitalRead(pinBtn) == LOW && millis() - lastButtonPress > debounceDelay) {
      lastButtonPress = millis();
      buttonPressed = true; // Set the button press flag

      // Reset game when button is pressed
      if (buttonPressed) {
        resetGame();
        buttonPressed = false;  // Reset the button press flag
      }
    }
  }
}

void resetGame() {
  snakeLength = 3;
  direction = RIGHT;
  score = 0;
  gameOver = false;

  uint8_t startX = 64;
  uint8_t startY = 32;
  for (int i = 0; i < snakeLength; i++) {
    snakeX[i] = startX - i * 4;
    snakeY[i] = startY;
  }

  generateFood();
}

void readJoystick() {
  int xVal = analogRead(pinX);
  int yVal = analogRead(pinY);

  if (xVal < 300 && direction != RIGHT) direction = LEFT;
  else if (xVal > 700 && direction != LEFT) direction = RIGHT;
  else if (yVal < 300 && direction != DOWN) direction = UP;
  else if (yVal > 700 && direction != UP) direction = DOWN;
}

void moveSnake() {
  for (int i = snakeLength - 1; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }

  if (direction == RIGHT) snakeX[0] += 4;
  else if (direction == LEFT) snakeX[0] -= 4;
  else if (direction == UP) snakeY[0] -= 4;
  else if (direction == DOWN) snakeY[0] += 4;

  if (snakeX[0] == foodX && snakeY[0] == foodY) {
    snakeLength++;
    score++;
    foodEaten = true;
    generateFood();
  }
}

void generateFood() {
  bool valid = false;

  while (!valid) {
    valid = true;

    // Food must spawn within the box area (X between 40 and 124, Y between 4 and 60)
    foodX = (random(40, 124) / 4) * 4;  // Food X position within the box
    foodY = (random(4, 60) / 4) * 4;    // Food Y position within the box

    // Check if the food is overlapping with the snake
    for (int i = 0; i < snakeLength; i++) {
      if (snakeX[i] == foodX && snakeY[i] == foodY) {
        valid = false;  // Food is overlapping the snake, try again
        break;
      }
    }
  }
}

void checkCollision() {
  // Wall collision (considering the box as the border)
  if (snakeX[0] < 40 || snakeX[0] >= 128 || snakeY[0] < 0 || snakeY[0] >= 64) {
    gameOver = true;
    return;
  }

  // Self collision (snake's head colliding with any part of its body)
  for (int i = 1; i < snakeLength; i++) {
    if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
      gameOver = true;
      return;
    }
  }
}

void drawEverything() {
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(2, 10, "Score:");
  u8g2.setCursor(2, 22);
  u8g2.print(score);

  u8g2.drawFrame(40, 0, 88, 64); // Border

  for (int i = 0; i < snakeLength; i++) {
    u8g2.drawBox(snakeX[i], snakeY[i], 4, 4);
  }

  u8g2.drawBox(foodX, foodY, 4, 4);

  u8g2.sendBuffer();
}

void drawGameOver() {
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(30, 28, "GAME OVER");
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(20, 45, "Score:");
  u8g2.setCursor(65, 45);
  u8g2.print(score);
  u8g2.drawStr(15, 60, "Click to restart");
  u8g2.sendBuffer();
}
