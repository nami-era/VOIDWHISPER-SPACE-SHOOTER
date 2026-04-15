#include "iGraphics.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <mmsystem.h>
#include <windows.h>


// Link against winmm.lib for mciSendString
#pragma comment(lib, "winmm.lib")

/* -------------------- CONSTANTS -------------------- */
int currentScrollSpeed = 20;
#define MAX_BULLET 50
#define BULLET_SPEED 25
#define MAX_ENEMY 10        // Increased for higher density in Medium/Hard
#define MAX_ENEMY_BULLET 60 // Increased for circular patterns
// ENEMY_BULLET_SPEED converted to variable
#define ENEMY_BULLET_SIZE 40
#define ENEMY_SIZE 80
#define MAX_STAR 3 // maximum stars on screen
#define STAR_SIZE 50
// STAR_DURATION and TICKS can remain constants
#define STAR_DURATION_TICKS 50

/* -------------------- OBSTACLE CONSTANTS -------------------- */
#define MAX_OBSTACLE 1
#define OBSTACLE_SPEED 7
#define OBSTACLE_SIZE 100

/* -------------------- SCREEN SIZE -------------------- */
int SCREEN_WIDTH;
int SCREEN_HEIGHT;

/* -------------------- GLOBALS -------------------- */
int bg1, bg2;
int bgM1, bgM2;
int bgEnv2_1, bgEnv2_2; // Environment 2 exclusive backgrounds
int bgX = 0;
int env1Sequence[4];     // Extended 4-image sequence for variety
int env2Sequence[4];     // Extended 4-image sequence for Environment 2
int bgCycleIndex = 0;

int charX, charY;
int idx = 0;
int charImg[3];

int bulletX[MAX_BULLET], bulletY[MAX_BULLET];
bool bulletActive[MAX_BULLET];
int bulletDX[MAX_BULLET], bulletDY[MAX_BULLET];
int bulletAngle[MAX_BULLET];
int bulletImg;

int enemyX[MAX_ENEMY], enemyY[MAX_ENEMY];
bool enemyActive[MAX_ENEMY];
int enemySpawnType[MAX_ENEMY];   // 0 = from right, 1 = from bottom
int enemyBulletCount[MAX_ENEMY]; // Track shots fired per enemy
int enemyImg;
int bottomEnemyImg;

int score = 0;
bool gameOver = false;
bool isSoundOn = true; // Master Audio Toggle
int soundOptionBg;     // Image handle for Sound Menu
int scoreOptionBg;     // Image handle for Score Menu

/* -------------------- DIFFICULTY VARIABLES -------------------- */
int enemyBulletSpeed = 3;
int enemiesToWin = 10;
int starsToWin = 5;
int enemySpeed = 5; // Global enemy speed
char selectedLevel[20] = "EASY";

// --- HIGH SCORE SYSTEM ---
int highScores[3] = {0, 0, 0}; // 0: EASY, 1: MEDIUM, 2: HARD
void saveScores() {
  FILE *fp;
  fopen_s(&fp, "highscores.txt", "w");
  if (fp) {
    fprintf(fp, "%d %d %d", highScores[0], highScores[1], highScores[2]);
    fclose(fp);
  }
}
void loadScores() {
  FILE *fp;
  fopen_s(&fp, "highscores.txt", "r");
  if (fp) {
    fscanf_s(fp, "%d %d %d", &highScores[0], &highScores[1], &highScores[2]);
    fclose(fp);
  }
}
void checkHighScore() {
  int levelIdx = 0;
  if (strcmp(selectedLevel, "MEDIUM") == 0) levelIdx = 1;
  else if (strcmp(selectedLevel, "HARD") == 0) levelIdx = 2;

  if (score > highScores[levelIdx]) {
    highScores[levelIdx] = score;
    saveScores();
    printf("New High Score for %s: %d\n", selectedLevel, score);
  }
}
void resetScores() {
  highScores[0] = highScores[1] = highScores[2] = 0;
  saveScores();
}

int explosionImg[5];
int explosionFrame = 0;
bool explosionActive = false;
int explosionX, explosionY;

int enemyBulletX[MAX_ENEMY_BULLET], enemyBulletY[MAX_ENEMY_BULLET];
int enemyBulletDX[MAX_ENEMY_BULLET], enemyBulletDY[MAX_ENEMY_BULLET];
bool enemyBulletActive[MAX_ENEMY_BULLET];
int enemyBulletType[MAX_ENEMY_BULLET]; // 0 = standard, 1 = special
int enemyBulletImg;
int specialBulletImg;

int playerLives = 5;        // total lives
bool respawnActive = false; // player is currently respawning
int respawnTimer = 0;       // timer for respawn delay
int enemiesDestroyed = 0;
int starsCollected = 0;     // stars collected by player
bool isShielded = false;    // Shield status
int currentEnvironment = 1; // Stage Environment Tracker

bool gameWin = false;
int starX[MAX_STAR], starY[MAX_STAR];
bool starActive[MAX_STAR];
int starImg;
int starTimer[MAX_STAR];
DWORD starSpawnTime[MAX_STAR];
int starScale[MAX_STAR]; // current size
bool starGrow[MAX_STAR]; // growing or shrinking

// Game State: -1 = Intro, -2 = Story, 0 = Menu, 1 = Gameplay, 2 = Level
// Selection
int gameState = -2; // Start with Story
int introImg;
int levelSelectionImg;
int pauseMenuImg;

/* -------------------- OBSTACLE GLOBALS -------------------- */
int obstacleX[MAX_OBSTACLE], obstacleY[MAX_OBSTACLE];
bool obstacleActive[MAX_OBSTACLE];
double obstacleAngle[MAX_OBSTACLE];
int obstacleImg;

int storyTimer = 0;
int storyStep = 0;
int storyBg;
int menuBg;

/* -------------------- MEDIUM STORY GLOBALS -------------------- */
int mediumStoryStep = 0;
int env2StoryStep = 0;
int playerAmmo = 100; // HARD MODE BULLETS
bool isSpecialEnemy[MAX_ENEMY];
int enemyStunTimer[MAX_ENEMY]; // Stun effect for Void Pulse

// OVERHEAT SYSTEM
float weaponHeat = 0;
float engineHeat = 0;
bool weaponOverheated = false;
bool engineOverheated = false;

int enemyTargetY[MAX_ENEMY];       // For random vertical movement
int enemyAITimer[MAX_ENEMY];       // Timer for changing target
int enemyDodgeCooldown[MAX_ENEMY]; // Prevents rapid flickering
int enemyAIStates[MAX_ENEMY];      // 0: Normal, 1: Retreat, 2: Strike
int enemyStateTimer[MAX_ENEMY];    // For cycle durations
int specialEnemyImg;

/* -------------------- BOSS VARIABLES -------------------- */
bool bossActive = false;
int bossHP = 10;
int maxBossHP = 10;
int bossX = -100, bossY = -100;
int bossWidth = 240; // 3x enemy size (80*3)
int bossHeight = 240;
int bossDirection = 1; // 1 = down, -1 = up
int bossImg; // We will reuse enemyImg or load a bigger one if available, for
             // now reuse enemyImg

/* -------------------- PLAYER SUPERPOWER (MEDIUM) -------------------- */
int superCooldown = 0;              // Cooldown timer (ticks)
const int MAX_SUPER_COOLDOWN = 300; // ~15 seconds at 50ms ticks
bool pulseActive = false;
int pulseDuration = 0; // NEW: For persistence
int pulseRadius = 0;
const int MAX_PULSE_RADIUS = 600;

/* -------------------- ECHO MIND MINI BOSS -------------------- */
bool isEchoMind = false;
int echoTeleportTimer = 0;
int echoShieldTimer = 0;
bool echoShieldActive = false;
int echoCircularTimer = 0;
int echoTeleportAlpha = 255; // For fade effect

// BLACK HOLE SEEDS MECHANIC
const int MAX_SEEDS = 3;
float seedX[MAX_SEEDS], seedY[MAX_SEEDS];
bool seedActive[MAX_SEEDS];
int seedTimer[MAX_SEEDS];
int seedState[MAX_SEEDS]; // 0 = Arming, 1 = Chasing, 2 = Pulling
int seedDropCooldown = 0;
int blackHoleImg;

/* -------------------- AMMO PICKUP -------------------- */
float ammoX, ammoY;
bool ammoActive = false;
int ammoImg;

/* -------------------- SMART SHOOTING (MEDIUM) -------------------- */
int smartShotTimer = 80; // 4 seconds at 50ms ticks
bool warningActive = false;
int warningTicks = 0; // 1 second duration
int predictedX, predictedY;
int lastCharX, lastCharY;

/* -------------------- HARD STORY GLOBALS -------------------- */
int hardStoryStep = 0;

/* -------------------- PROTOTYPES -------------------- */
void resetGame();
void checkGameWin();
void triggerVoidPulse();
void spawnSmartBullet(int targetX, int targetY);

/* -------------------- DRAW -------------------- */
void iDraw() {
  iClear();

  // Check Game State
  if (gameState == -1) {
    iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, introImg);
    iSetColor(255, 255, 255);
    // Pulsing text
    if ((GetTickCount() / 500) % 2 == 0)
      iText(SCREEN_WIDTH / 2 - 100, 50, (char *)"Press 'Space' to Continue",
            GLUT_BITMAP_HELVETICA_18);
    return;
  }

  if (gameState == -2) {
    // Storyline Background
    iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, storyBg);

    iSetColor(255, 255, 255); // White Text

    storyTimer++;
    if (storyTimer > 150) {
      // Timer logic placeholder
    }

    int centerX = SCREEN_WIDTH / 2 - 150;
    int centerY = SCREEN_HEIGHT / 2;

    if (storyStep == 0)
      iText(centerX, centerY, "Year 2147.", GLUT_BITMAP_HELVETICA_18);
    if (storyStep == 1) {
      iText(centerX - 50, centerY + 20, "Humanity's greatest defense AI...",
            GLUT_BITMAP_HELVETICA_18);
      iText(centerX, centerY - 20, "VOIDWHISPER.", GLUT_BITMAP_HELVETICA_18);
    }
    if (storyStep == 2)
      iText(centerX - 60, centerY,
            "Built to protect us from the darkness of space.",
            GLUT_BITMAP_HELVETICA_18);

    if (storyStep == 3) // Glitch
    {
      if (rand() % 50 != 0) // Flicker
      {
        iSetColor(255, 0, 0); // Red for danger
        iText(centerX, centerY + 20, "It heard something.",
              GLUT_BITMAP_HELVETICA_18);
        iText(centerX, centerY - 20, "A signal from the void.",
              GLUT_BITMAP_HELVETICA_18);
      }
      // Glitch lines
      iSetColor(rand() % 255, rand() % 255, rand() % 255);
      iLine(rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT,
            rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT);
    }

    if (storyStep == 4) // Enemy Silhouette
    {
      iSetColor(255, 0, 0);
      iText(centerX, centerY + 120, "Now it adapts. It learns.",
            GLUT_BITMAP_HELVETICA_18);
      iText(centerX, centerY + 80, "And it has turned against us.",
            GLUT_BITMAP_HELVETICA_18);

      // Show Enemy
      iShowImage(SCREEN_WIDTH / 2 - 40, SCREEN_HEIGHT / 2 - 40, 80, 80,
                 enemyImg);
    }

    if (storyStep == 5) // Player Power up
    {
      iSetColor(0, 200, 255);
      iText(centerX, centerY + 120, "Pilot... enter the simulation.",
            GLUT_BITMAP_HELVETICA_18);

      // Show Player
      iShowImage(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 100, 200, 200,
                 charImg[0]);
    }

    if (storyStep == 6) {
      iSetColor(0, 255, 0); // Keep Training Mode Green
      iText(SCREEN_WIDTH / 2 - 140, SCREEN_HEIGHT / 2,
            "TRAINING MODE - INITIALIZING", GLUT_BITMAP_HELVETICA_18);
    }

    iSetColor(100, 100, 100);
    iText(SCREEN_WIDTH / 2 - 80, 50, "Press 'Space' to Continue",
          GLUT_BITMAP_HELVETICA_12);
    return;
  }

  if (gameState == 0) {
    // Main Menu
    iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, menuBg);

    // Title (If not in image) - Optional, user might have it in image
    // iSetColor(255, 255, 255);
    // iText(SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT - 100, "VOID WHISPER",
    // GLUT_BITMAP_TIMES_ROMAN_24);

    // Hardcoded buttons removed as per user request to use background image
    // buttons instead.
    return;
  }

  if (gameState == 2) {
    // Level Selection Screen
    iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, levelSelectionImg);

    iSetColor(255, 255, 255);
    iText(SCREEN_WIDTH / 2 - 100, 30, "Press Backspace to back",
          GLUT_BITMAP_HELVETICA_18);
    return;
  }

  if (gameState == 7) {
    // Sound Options Sub-menu
    iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, soundOptionBg);
    return;
  }

  if (gameState == 8) {
    // High Score Menu - Calibration Mode
    iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, scoreOptionBg); 
    
    // Header
    iSetColor(0, 255, 255); 
    iText(50, SCREEN_HEIGHT - 60, "VOID COMMAND: ARCHIVED RECORDS", GLUT_BITMAP_TIMES_ROMAN_24);

    char scoreStr[50];
    int textX = SCREEN_WIDTH * 0.62; 
    int bx = SCREEN_WIDTH * 0.56;
    int bw = SCREEN_WIDTH * 0.33;
    int bh = SCREEN_HEIGHT * 0.08;

    // --- Slot 1: EASY ---
    iSetColor(0, 200, 255); // Cyan Border
    iRectangle(bx, SCREEN_HEIGHT * 0.67, bw, bh);
    iSetColor(255, 255, 255);
    sprintf_s(scoreStr, "EASY BEST:    %d", highScores[0]);
    iText(textX, SCREEN_HEIGHT * 0.71, scoreStr, GLUT_BITMAP_TIMES_ROMAN_24);
    
    // --- Slot 2: MEDIUM ---
    iSetColor(0, 200, 255);
    iRectangle(bx, SCREEN_HEIGHT * 0.55, bw, bh);
    iSetColor(255, 255, 255);
    sprintf_s(scoreStr, "MEDIUM BEST:  %d", highScores[1]);
    iText(textX, SCREEN_HEIGHT * 0.59, scoreStr, GLUT_BITMAP_TIMES_ROMAN_24);
    
    // --- Slot 3: HARD ---
    iSetColor(0, 200, 255);
    iRectangle(bx, SCREEN_HEIGHT * 0.43, bw, bh);
    iSetColor(255, 255, 255);
    sprintf_s(scoreStr, "HARD BEST:    %d", highScores[2]);
    iText(textX, SCREEN_HEIGHT * 0.47, scoreStr, GLUT_BITMAP_TIMES_ROMAN_24);
    
    // --- Slot 4: RESET ---
    iSetColor(255, 100, 100); // Red Border for Reset
    iRectangle(bx, SCREEN_HEIGHT * 0.31, bw, bh);
    iText(textX, SCREEN_HEIGHT * 0.35, "RESET RECORDS", GLUT_BITMAP_TIMES_ROMAN_24);
    
    // --- Slot 5: BACK ---
    iSetColor(200, 200, 200); // Silver Border for Back
    iRectangle(bx, SCREEN_HEIGHT * 0.19, bw, bh);
    iText(textX, SCREEN_HEIGHT * 0.23, "BACK TO MENU", GLUT_BITMAP_TIMES_ROMAN_24);

    return;
  }

  if (gameState == -3) // Instructions State
  {
    // Instructions Screen - Fixed background
    iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, storyBg);

    iSetColor(255, 255, 255);
    // Title
    iText(SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT - 100,
          "VOID WHISPER - ADVANCED TACTICAL MANUAL",
          GLUT_BITMAP_TIMES_ROMAN_24);

    int col1X = SCREEN_WIDTH * 0.05;
    int col2X = SCREEN_WIDTH * 0.35;
    int col3X = SCREEN_WIDTH * 0.65;
    int textY = SCREEN_HEIGHT - 200;

    // COLUMN 1: BASIC FLIGHT
    iSetColor(0, 255, 255); // Cyan header
    iText(col1X, textY, "BASIC FLIGHT", GLUT_BITMAP_HELVETICA_18);
    iSetColor(255, 255, 255);
    iText(col1X + 10, textY - 40, "Arrow Keys   : Move Ship", GLUT_BITMAP_HELVETICA_12);
    iText(col1X + 10, textY - 70, "Spacebar     : Plasma Cannon", GLUT_BITMAP_HELVETICA_12);
    iText(col1X + 10, textY - 100, "Blue Stars   : Energy Shields", GLUT_BITMAP_HELVETICA_12);
    iShowImage(col1X + 10, textY - 200, 100, 100, charImg[0]);
    iShowImage(col1X + 120, textY - 180, 40, 40, starImg);

    // COLUMN 2: MEDIUM PROTOCOLS
    iSetColor(0, 255, 255);
    iText(col2X, textY, "MEDIUM TACTICS", GLUT_BITMAP_HELVETICA_18);
    iSetColor(255, 255, 255);
    iText(col2X + 10, textY - 40, "'V' Key      : VOID PULSE", GLUT_BITMAP_HELVETICA_12);
    iText(col2X + 10, textY - 70, "- Explodes bullets & stuns enemies", GLUT_BITMAP_HELVETICA_12);
    iText(col2X + 10, textY - 110, "Red Circles : Enemy is tracking you", GLUT_BITMAP_HELVETICA_12);
    iText(col2X + 10, textY - 140, "- DODGE when the circle appears!", GLUT_BITMAP_HELVETICA_12);
    iShowImage(col2X + 10, textY - 230, 80, 80, specialEnemyImg);

    // COLUMN 3: HARD MODE ADVISORY
    iSetColor(0, 255, 255);
    iText(col3X, textY, "HARD MODE HAZARDS", GLUT_BITMAP_HELVETICA_18);
    iSetColor(255, 255, 255);
    iText(col3X + 10, textY - 40, "Ammo      : Finite. Collect Crates", GLUT_BITMAP_HELVETICA_12);
    iText(col3X + 10, textY - 70, "Gravity   : Ship is pulled in Stage 2", GLUT_BITMAP_HELVETICA_12);
    iText(col3X + 10, textY - 100, "Boss Bombs: Chase & sabotage ammo", GLUT_BITMAP_HELVETICA_12);
    iText(col3X + 10, textY - 130, "Heat      : Rapid fire = Overheating", GLUT_BITMAP_HELVETICA_12);
    iShowImage(col3X + 10, textY - 220, 60, 60, ammoImg);
    iShowImage(col3X + 100, textY - 220, 60, 60, blackHoleImg);

    // Back Button
    int btnW = SCREEN_WIDTH * 0.15;
    int btnH = SCREEN_HEIGHT * 0.06;
    int btnX = SCREEN_WIDTH / 2 - btnW / 2;
    int btnY = SCREEN_HEIGHT * 0.1;

    iSetColor(50, 50, 50);
    iFilledRectangle(btnX, btnY, btnW, btnH);
    iSetColor(255, 50, 50);
    iRectangle(btnX, btnY, btnW, btnH);
    iSetColor(255, 255, 255);
    iText(btnX + btnW / 2 - 25, btnY + btnH / 2 - 5, "BACK",
          GLUT_BITMAP_HELVETICA_18);

    return;
  }

  // Gameplay State
  // Background
  iSetColor(255, 255, 255); // Reset color to prevent image tinting glitches
  if (strcmp(selectedLevel, "HARD") == 0 && currentEnvironment == 2) {
    iShowImage(bgX, 0, SCREEN_WIDTH + 2, SCREEN_HEIGHT, env2Sequence[bgCycleIndex]);
    iShowImage(bgX + SCREEN_WIDTH, 0, SCREEN_WIDTH + 2, SCREEN_HEIGHT, env2Sequence[(bgCycleIndex + 1) % 4]);
  } else if (strcmp(selectedLevel, "MEDIUM") == 0) {
    iShowImage(bgX, 0, SCREEN_WIDTH + 2, SCREEN_HEIGHT, bgM1);
    iShowImage(bgX + SCREEN_WIDTH, 0, SCREEN_WIDTH + 2, SCREEN_HEIGHT, bgM2);
  } else if (strcmp(selectedLevel, "HARD") == 0 && currentEnvironment == 1) {
    // Stage 1 / Hard Env 1 - Using the 4-image Cycle
    iShowImage(bgX, 0, SCREEN_WIDTH + 2, SCREEN_HEIGHT, env1Sequence[bgCycleIndex]);
    iShowImage(bgX + SCREEN_WIDTH, 0, SCREEN_WIDTH + 2, SCREEN_HEIGHT, env1Sequence[(bgCycleIndex + 1) % 4]);
  } else {
    // Easy mode or fallback - Using the original bg1/bg2
    iShowImage(bgX, 0, SCREEN_WIDTH + 2, SCREEN_HEIGHT, bg1);
    iShowImage(bgX + SCREEN_WIDTH, 0, SCREEN_WIDTH + 2, SCREEN_HEIGHT, bg2);
  }

  // Player bullets
  for (int i = 0; i < MAX_BULLET; i++)
    if (bulletActive[i]) {
      // Glow/Highlight effect - Gold for metallic bullet
      iSetColor(255, 215, 0); // Gold outer glow
      iFilledCircle(bulletX[i] + 10, bulletY[i] + 10, 8);

      iSetColor(255, 255, 224); // Light Yellow/White core
      iFilledCircle(bulletX[i] + 10, bulletY[i] + 10, 5);

      iShowImage(bulletX[i], bulletY[i], 20, 20, bulletImg);
    }

  // Enemies
  for (int i = 0; i < MAX_ENEMY; i++)
    if (enemyActive[i]) {
      if (enemySpawnType[i] == 1) {
        iShowImage(enemyX[i], enemyY[i], ENEMY_SIZE, ENEMY_SIZE,
                   bottomEnemyImg);
      } else if (isSpecialEnemy[i])
        iShowImage(enemyX[i], enemyY[i], ENEMY_SIZE, ENEMY_SIZE,
                   specialEnemyImg);
      else
        iShowImage(enemyX[i], enemyY[i], ENEMY_SIZE, ENEMY_SIZE, enemyImg);
    }

  // Enemy bullets
  for (int i = 0; i < MAX_ENEMY_BULLET; i++)
    if (enemyBulletActive[i]) {
      if (enemyBulletType[i] == 1)
        iShowImage(enemyBulletX[i], enemyBulletY[i], ENEMY_BULLET_SIZE,
                   ENEMY_BULLET_SIZE, specialBulletImg);
      else
        iShowImage(enemyBulletX[i], enemyBulletY[i], ENEMY_BULLET_SIZE,
                   ENEMY_BULLET_SIZE, enemyBulletImg);
    }

  // Obstacles
  for (int i = 0; i < MAX_OBSTACLE; i++)
    if (obstacleActive[i]) {
      iRotate(obstacleX[i] + OBSTACLE_SIZE / 2,
              obstacleY[i] + OBSTACLE_SIZE / 2, obstacleAngle[i]);
      iShowImage(obstacleX[i], obstacleY[i], OBSTACLE_SIZE, OBSTACLE_SIZE,
                 obstacleImg);
      iUnRotate();
    }

  // Ammo Crate
  if (ammoActive) {
      // Glow effect
      iSetColor(0, 200, 255);
      iCircle(ammoX + 30, ammoY + 30, 40 + (GetTickCount() / 100) % 10);
      iShowImage(ammoX, ammoY, 60, 60, ammoImg);
  }

  // Score
  char str[50];
  sprintf_s(str, "Score: %d", score);
  iSetColor(255, 255, 255);
  iText(80, SCREEN_HEIGHT - 45, str, GLUT_BITMAP_TIMES_ROMAN_24);

  // Lives
  char lifeStr[50];
  sprintf_s(lifeStr, "Lives: %d", playerLives);
  iSetColor(255, 255, 0);
  iText(SCREEN_WIDTH - 150, SCREEN_HEIGHT - 45, lifeStr,
        GLUT_BITMAP_TIMES_ROMAN_24);

  // Level Indicator & Objectives
  iSetColor(0, 255, 255);
  char modeStr[50];
  sprintf_s(modeStr, "Level: %s", selectedLevel);
  iText(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT - 45, modeStr,
        GLUT_BITMAP_TIMES_ROMAN_24);

  // Environment Indicator (Hard Mode Only)
  if (strcmp(selectedLevel, "HARD") == 0) {
    iSetColor(100, 255, 150);
    char envStr[50];
    sprintf_s(envStr, "ENVIRONMENT: %d (STABLE ZONE)", currentEnvironment);
    iText(SCREEN_WIDTH - 320, SCREEN_HEIGHT - 80, envStr,
          GLUT_BITMAP_HELVETICA_18);

    // --- ENVIRONMENT LABELS ---
    if (currentEnvironment == 1) {
      iSetColor(0, 255, 255);
      iText(SCREEN_WIDTH - 320, SCREEN_HEIGHT - 105,
            (char *)"[PRECISE MOVEMENT | CLEAN COMBAT]",
            GLUT_BITMAP_HELVETICA_12);
    } else if (currentEnvironment == 2) {
      iSetColor(255, 100, 100);
      iText(SCREEN_WIDTH - 320, SCREEN_HEIGHT - 105,
            (char *)"[GRAVITY ACTIVE | NO HEAT]", GLUT_BITMAP_HELVETICA_12);
    }

    // --- PORTED MEDIUM LEVEL ATMOSPHERE (Environment 1) ---
    if (currentEnvironment == 1) {
      // 1. Narrator Glitch Lines (Red flickering lines showing system
      // compromise)
      if (rand() % 100 < 4) // Periodic flashes
      {
        iSetColor(255, 0, 0);
        iLine(0, rand() % SCREEN_HEIGHT, SCREEN_WIDTH, rand() % SCREEN_HEIGHT);
        iLine(rand() % SCREEN_WIDTH, 0, rand() % SCREEN_WIDTH, SCREEN_HEIGHT);
      }

      // 2. Narrator Glitch Text (Eerie periodic prompts)
      int cycle = (GetTickCount() / 2500) % 6;
      if (cycle == 1 || cycle == 4) {
        iSetColor(255, 255, 255);
        if (rand() % 10 < 3)
          iSetColor(255, 50, 50); // Glitch to red slightly

        // Use static to avoid re-initialization overhead and fix iText type
        // mismatch
        static char *glitchTexts[] = {
            (char *)"VOID DETECTED...", (char *)"SYSTEM COMPROMISED",
            (char *)"IT IS LEARNING",   (char *)"SURVIVAL: 0%",
            (char *)"THE VOID SPEAKS",  (char *)"STABLE ZONE???"};
        iText(SCREEN_WIDTH / 2 - 120, 80, glitchTexts[cycle],
              GLUT_BITMAP_HELVETICA_18);
      }
    }
  }

  if (isShielded) {
    iSetColor(0, 100, 255);
    iText(SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT - 80, "SHIELD ACTIVE",
          GLUT_BITMAP_HELVETICA_18);
  }

  char enemyStr[50];
  int displayTarget = enemiesToWin;
  if (strcmp(selectedLevel, "HARD") == 0) {
      if (currentEnvironment == 1) displayTarget = 10;
      else if (currentEnvironment == 2) displayTarget = 25; // 10 from Env 1 + 15 from Env 2
  }
  
  sprintf_s(enemyStr, "Enemies: %d/%d", enemiesDestroyed, displayTarget);
  iSetColor(255, 100, 100);
  iText(SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT - 110, enemyStr,
        GLUT_BITMAP_HELVETICA_18);

  // Superpower Gauge (Medium/Hard)
  if (strcmp(selectedLevel, "MEDIUM") == 0 ||
      strcmp(selectedLevel, "HARD") == 0) {
    int gaugeW = 200;
    int gaugeH = 15;
    int gx = 80;
    int gy = SCREEN_HEIGHT - 80;

    iSetColor(50, 50, 50);
    iFilledRectangle(gx, gy, gaugeW, gaugeH);

    if (superCooldown == 0)
      iSetColor(255, 165, 0); // Orange when ready
    else
      iSetColor(100, 100, 100); // Grey during cooldown

    double fill = 1.0 - ((double)superCooldown / (double)MAX_SUPER_COOLDOWN);
    iFilledRectangle(gx, gy, (int)(gaugeW * fill), gaugeH);
    iSetColor(255, 255, 255);
    iRectangle(gx, gy, gaugeW, gaugeH);

    if (bossActive)
      iText(gx, gy - 20, (char *)"VOID PULSE [DISABLED DURING BOSS]",
            GLUT_BITMAP_HELVETICA_12);
    else
      iText(gx, gy - 20, (char *)"VOID PULSE ['V']", GLUT_BITMAP_HELVETICA_12);
  }

  // Overheat Gauges (Drawn for all levels)
  int hx = 80;
  int hy = SCREEN_HEIGHT - 130;
  int hw = 150;
  int hh = 10;

  // Weapon Heat Bar - Only shown and active outside normal Environment 1 and 2
  if (currentEnvironment > 2) {
    iSetColor(40, 40, 40);
    iFilledRectangle(hx, hy, hw, hh);
    iSetColor(255, 50, 50);
    iFilledRectangle(hx, hy, (int)(hw * (weaponHeat / 100.0f)), hh);
    iSetColor(255, 255, 255);
    iRectangle(hx, hy, hw, hh);

    if (weaponOverheated) {
      iSetColor(255, 0, 0);
      iText(hx + hw + 10, hy, (char *)"OVERHEATED!", GLUT_BITMAP_HELVETICA_12);
    } else {
      iSetColor(200, 200, 200);
      iText(hx + hw + 10, hy, (char *)"WEAPON HEAT", GLUT_BITMAP_HELVETICA_10);
    }
  } else {
    iSetColor(100, 255, 100);
    iText(hx, hy, (char *)"WEAPONS: NOMINAL (UNLIMITED)",
          GLUT_BITMAP_HELVETICA_10);
  }

  // Hard Mode Ammo Counter
  if (strcmp(selectedLevel, "HARD") == 0) {
    hy -= 30; // Shift down below engine heat
    char ammoStr[50];
    sprintf_s(ammoStr, "AMMO: %d", playerAmmo);
    
    if (playerAmmo <= 0) {
      iSetColor(255, 0, 0);
      if ((GetTickCount() / 200) % 2 == 0) iText(hx, hy, (char *)"OUT OF AMMO!", GLUT_BITMAP_HELVETICA_12);
    } else if (playerAmmo <= 20) {
      iSetColor(255, 100, 100);
      iText(hx, hy, ammoStr, GLUT_BITMAP_HELVETICA_12);
    } else {
      iSetColor(100, 255, 100);
      iText(hx, hy, ammoStr, GLUT_BITMAP_HELVETICA_12);
    }
  }

  // Engine Heat Bar - Only active outside Environment 1 and 2
  hy -= 20;
  if (currentEnvironment > 2) {
    iSetColor(40, 40, 40);
    iFilledRectangle(hx, hy, hw, hh);
    iSetColor(255, 165, 0);
    iFilledRectangle(hx, hy, (int)(hw * (engineHeat / 100.0f)), hh);
    iSetColor(255, 255, 255);
    iRectangle(hx, hy, hw, hh);

    if (engineOverheated) {
      iSetColor(255, 165, 0);
      iText(hx + hw + 10, hy, (char *)"OVERHEATED!", GLUT_BITMAP_HELVETICA_12);
    } else {
      iSetColor(200, 200, 200);
      iText(hx + hw + 10, hy, (char *)"ENGINE HEAT", GLUT_BITMAP_HELVETICA_10);
    }
  } else {
    iSetColor(255, 200, 0);
    iText(hx, hy, (char *)"ENGINES: NOMINAL (PRECISE)",
          GLUT_BITMAP_HELVETICA_10);
  }

  // Smart Shooting Warning (Medium Only)
  if (warningActive) {
    iSetColor(255, 0, 0); // Targeted RED
    // Flashing effect
    if ((GetTickCount() / 200) % 2 == 0) {
      iCircle(predictedX + 100, predictedY + 100, 70);
      iCircle(predictedX + 100, predictedY + 100, 75);

      iSetColor(255, 255, 255);
      iText(predictedX + 60, predictedY + 180, (char *)"TARGET ACQUIRED",
            GLUT_BITMAP_HELVETICA_12);
    }
  }

  // [Void Pulse rendering moved below Blackout mask to ensure visibility]

  // Explosion or Ship
  if (explosionActive) {
    // Dynamic Scaling for Player Explosion
    int baseSize = 50;
    int maxSize = 250; // Reduced from 450
    int currentSize;

    if (explosionFrame < 4) // Rapid Expand
      currentSize = baseSize + (explosionFrame * (maxSize - baseSize) / 4);
    else // Slow Expand
      currentSize = maxSize + ((explosionFrame - 4) * 10);

    // Center the explosion
    int drawX = explosionX - (currentSize / 2) + 100;
    int drawY = explosionY - (currentSize / 2) + 100;

    int frame = idx; // Cycle frames for variety or just use frame 0
    if (frame >= 3)
      frame = 0;

    iShowImage(drawX, drawY, currentSize, currentSize,
               explosionImg[explosionFrame % 5]);
  } else if (!gameOver) {
    iShowImage(charX, charY, 200, 200, charImg[idx]);

    // Draw Shield
    if (isShielded) {
      // Shiny Effect: Cycle colors based on time
      int shine = (GetTickCount() / 50) % 3; // Change every 50ms

      if (shine == 0)
        iSetColor(0, 100, 255); // Deep Blue
      else if (shine == 1)
        iSetColor(0, 255, 255); // Cyan
      else
        iSetColor(200, 200, 255); // White-ish

      iCircle(charX + 100, charY + 100, 90);     // Main Shield
      iCircle(charX + 100, charY + 100, 90 - 4); // Inner Ring (thicker look)

      // Random "Sparks" on the edge
      if (rand() % 2 == 0) {
        iSetColor(255, 255, 255);
        double angle = (rand() % 360) * 3.14159 / 180.0;
        int sparkX = charX + 100 + 90 * cos(angle);
        int sparkY = charY + 100 + 90 * sin(angle);
        iPoint(sparkX, sparkY, 3); // Draw spark
      }
    }
  }

  // Boss
  if (bossActive) {
    iShowImage(bossX, bossY, bossWidth, bossHeight, bossImg);

    // Draw Cracks if boss is damaged
    if (bossHP <= maxBossHP / 2) {
      iSetColor(255, 255, 255); // White cracks
      // Stage 1 Cracks
      iLine(bossX + 40, bossY + 40, bossX + bossWidth - 80,
            bossY + bossHeight - 40);
      iLine(bossX + 60, bossY + bossHeight - 60, bossX + bossWidth - 40,
            bossY + 100);

      if (bossHP <= maxBossHP / 5) {
        // Stage 2 Cracks (Extra damage)
        iSetColor(200, 200, 200);
        iLine(bossX + bossWidth / 2, bossY + 20, bossX + 30,
              bossY + bossHeight - 30);
        iLine(bossX + bossWidth - 30, bossY + bossHeight / 2, bossX + 50,
              bossY + 50);
      }
    }

    if (isEchoMind && echoShieldActive) {
      // Pulsing Cyan Shield
      int pulse = (GetTickCount() / 100) % 255;
      iSetColor(0, 255, 255);
      iCircle(bossX + bossWidth / 2, bossY + bossHeight / 2,
              bossWidth / 2 + 10);
      iCircle(bossX + bossWidth / 2, bossY + bossHeight / 2, bossWidth / 2 + 5);
    }

    // Phase 2 Flickering/Echo Effect
    if (isEchoMind && bossHP <= maxBossHP / 2) {
      if (rand() % 5 == 0) {
        iSetColor(0, 255, 255);
        iRectangle(bossX + (rand() % 40 - 20), bossY + (rand() % 40 - 20),
                   bossWidth, bossHeight);
      }
    }

    // Boss HP Bar
    iSetColor(255, 0, 0);
    iRectangle(bossX, bossY + bossHeight + 10, bossWidth, 10);
    iFilledRectangle(bossX, bossY + bossHeight + 10,
                     bossWidth * ((double)bossHP / (double)maxBossHP), 10);
  }

  // Draw Black Hole Seeds
  for (int i = 0; i < MAX_SEEDS; i++) {
    if (seedActive[i]) {
      // Rotating effect
      iRotate(seedX[i], seedY[i], (GetTickCount() / 10) % 360);
      if (seedState[i] == 0) {
        // Arming state (Smallest)
        iShowImage(seedX[i] - 30, seedY[i] - 30, 60, 60, blackHoleImg);
      } else if (seedState[i] == 1) {
        // Chasing state (Medium)
        iShowImage(seedX[i] - 45, seedY[i] - 45, 90, 90, blackHoleImg);
      } else {
        // Active Black Hole (Full size)
        iShowImage(seedX[i] - 75, seedY[i] - 75, 150, 150, blackHoleImg);
      }
      iUnRotate();
    }
  }

  // Void Pulse Visual Effect (Drawn OVER background)
  if (pulseActive) {
    // Inner pulsing rings
    int ringCount = 3;
    for (int r = 0; r < ringCount; r++) {
      // Radius pulses over time
      int baseR = 100 + r * 50;
      int offset = (GetTickCount() / 5) % 100;
      int finalR = baseR + offset;
      if (finalR < pulseRadius && finalR > 0) {
        iSetColor(255, 255, 255);
        iCircle(charX + 100, charY + 100, finalR);
      }
    }
  }

  // Game Over
  if (gameOver && !gameWin) {
    iSetColor(255, 0, 0);
    iText(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 20, (char *)"GAME OVER",
          GLUT_BITMAP_TIMES_ROMAN_24);
    iSetColor(255, 255, 255);
    iText(SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 - 20,
          (char *)"Press 'r' to RESTART", GLUT_BITMAP_HELVETICA_18);
    iText(SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 - 50,
          (char *)"Press 'e' for EXIT", GLUT_BITMAP_HELVETICA_18);
  }
  // Game Win
  if (gameWin) {
    iSetColor(0, 255, 0);
    iText(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 20, (char *)"YOU WIN!",
          GLUT_BITMAP_TIMES_ROMAN_24);
    iSetColor(255, 255, 255);
    iText(SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 - 20,
          (char *)"Press 'r' to RESTART", GLUT_BITMAP_HELVETICA_18);
    iText(SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 - 50,
          (char *)"Press 'e' for EXIT", GLUT_BITMAP_HELVETICA_18);
  }
  // Draw Stars
  for (int i = 0; i < MAX_STAR; i++) {
    if (starActive[i]) {
      iShowImage(starX[i], starY[i], starScale[i], starScale[i], starImg);
    }
  }
  // Hard Level Story Narration
  if (gameState == 5) {
    iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, storyBg);
    iSetColor(255, 255, 255);

    int centerX = SCREEN_WIDTH / 2 - 150;
    int centerY = SCREEN_HEIGHT / 2;

    if (hardStoryStep == 0) {
      iText(centerX - 50, centerY, (char *)"The Void has destabilized. The sector is fractured.",
            GLUT_BITMAP_HELVETICA_18);
    }
    if (hardStoryStep == 1) {
      iText(centerX, centerY, (char *)"Zone 1: The Stable Sector. Eliminate 10 hostiles.",
            GLUT_BITMAP_HELVETICA_18);
    }
    if (hardStoryStep == 2) {
      iText(centerX - 60, centerY, (char *)"Zone 2: The Gravity Well. Warning: Extreme gravity.",
            GLUT_BITMAP_HELVETICA_18);
    }
    if (hardStoryStep == 3) {
      iText(centerX, centerY, (char *)"Survive the gauntlet. The Echo Mind awaits beyond.",
            GLUT_BITMAP_HELVETICA_18);

      // Narrator Glitch Lines - More aggressive
      if (rand() % 10 < 6) {
        iSetColor(255, 0, 0);
        iLine(rand() % SCREEN_WIDTH, 0, rand() % SCREEN_WIDTH, SCREEN_HEIGHT);
      }
    }

    iSetColor(100, 100, 100);
    iText(SCREEN_WIDTH / 2 - 80, 50, "Press 'Space' to Continue",
          GLUT_BITMAP_HELVETICA_12);
    return;
  }
  // Medium Level Story Narration
  if (gameState == 4) {
    iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, storyBg);
    iSetColor(255, 255, 255);

    int centerX = SCREEN_WIDTH / 2 - 150;
    int centerY = SCREEN_HEIGHT / 2;

    if (mediumStoryStep == 0)
      iText(centerX - 50, centerY, (char *)"The void is no longer silent...",
            GLUT_BITMAP_HELVETICA_18);

    if (mediumStoryStep == 1) {
      iText(centerX, centerY, (char *)"It has begun to learn you.",
            GLUT_BITMAP_HELVETICA_18);

      // Narrator Glitch Lines
      if (rand() % 10 < 3) {
        iSetColor(255, 0, 0);
        iLine(rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT,
              rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT);
      }
    }

    iSetColor(100, 100, 100);
    iText(SCREEN_WIDTH / 2 - 80, 50, (char *)"Press 'Space' to Continue",
          GLUT_BITMAP_HELVETICA_12);
    return;
  }

  // Environment 2 Transition Story Screen
  if (gameState == 6) {
    iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, storyBg);
    iSetColor(255, 255, 255);
    int centerX = SCREEN_WIDTH / 2 - 150;
    int centerY = SCREEN_HEIGHT / 2;

    if (env2StoryStep == 0) {
      iSetColor(255, 100, 100);
      iText(centerX, centerY, (char *)"STABLE ZONE BREACHED.", GLUT_BITMAP_TIMES_ROMAN_24);
    }
    if (env2StoryStep == 1) {
      iText(centerX - 100, centerY, (char *)"You are entering the Gravity Well. The environment here is unnaturally dense.", GLUT_BITMAP_HELVETICA_18);
    }
    if (env2StoryStep == 2) {
      iText(centerX, centerY, (char *)"All heat regulators have failed.", GLUT_BITMAP_HELVETICA_18);
    }

    iSetColor(100, 100, 100);
    iText(SCREEN_WIDTH / 2 - 80, 50, (char *)"Press 'Enter' to Continue", GLUT_BITMAP_HELVETICA_12);
    return;
  }

  // Exit the simulation if we are just drawing the HUD or something
  if (gameState != 1 && gameState != 3)
    return;

  // 3 Dash Icon - Top Left
  int dashW = 40;
  int dashH = 5;
  int dashX = 20;
  int dashY = SCREEN_HEIGHT - 40;

  iSetColor(200, 200, 200);
  iFilledRectangle(dashX, dashY, dashW, dashH);
  iFilledRectangle(dashX, dashY - 10, dashW, dashH);
  iFilledRectangle(dashX, dashY - 20, dashW, dashH);

  // Pause Menu Overlay
  if (gameState == 3) {
    // Display the transparent Pause Image provided by user
    iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, pauseMenuImg);
  }
}

/* -------------------- INPUT -------------------- */
void iKeyboard(unsigned char key) {
  if (gameState == -2) {
    if (key == ' ') {
      storyStep++;
      if (storyStep > 6) {
        gameState = -1; // Go to Intro
      }
    }
    return;
  }

  if (gameState == -1) {
    if (key == ' ')
      gameState = 0; // Go to Main Menu
    else
      gameState = 0; // Any key goes to menu
    mciSendString("play menuMusic from 0 repeat", NULL, 0, NULL);
    return;
  }

  // Environment 2 Mid-Game Cutscene Input
  if (gameState == 6) {
    if (key == '\r') {
      env2StoryStep++;
      if (env2StoryStep > 2) {
        gameState = 1; // Resume Gameplay
        
        // Transition Music
        mciSendString("stop bgMusic", NULL, 0, NULL);
        mciSendString("play env2Music from 0 repeat", NULL, 0, NULL);
      }
    }
    return;
  }

  if (gameState == -3) // Instructions screen
  {
    if (key == 8) // Backspace
    {
      gameState = 0; // Return to Main Menu
    }
    return;
  }

  if (gameState == 5) // Hard Story
  {
    if (key == ' ') {
      hardStoryStep++;
      if (hardStoryStep > 3) {
        enemyBulletSpeed = 5;
        enemiesToWin = 25;
        enemySpeed = 9;
        strcpy_s(selectedLevel, "HARD");
        currentScrollSpeed = 40; // High speed as requested

        resetGame();
        gameState = 1;
        // Music is already playing
      }
    }
    return;
  }

  if (gameState == 2) // Level Selection screen
  {
    if (key == 8) // Backspace
    {
      gameState = 0; // Return to Main Menu
    }
    return;
  }

  if (gameState == 4) // Medium Story
  {
    if (key == ' ') {
      mediumStoryStep++;
      if (mediumStoryStep > 1) {
        enemyBulletSpeed = 4;
        enemiesToWin = 15; // Set to 15
        enemySpeed = 7;
        strcpy_s(selectedLevel, "MEDIUM");

        resetGame();
        gameState = 1;
        // Music is already playing from the start of story
      }
    }
    return;
  }

  if (key == 'r' || key == 'R') {
    if (gameOver || gameWin) {
      // Restart game logic
      resetGame();

      // Ensure bg music is playing
      mciSendString("stop menuMusic", NULL, 0, NULL);
      mciSendString("stop bgMusic", NULL, 0, NULL);
      mciSendString("stop bgMusicMedium", NULL, 0, NULL);
      mciSendString("stop bossMusic", NULL, 0, NULL);
      mciSendString("stop winMusic", NULL, 0, NULL);
      mciSendString("stop env2Music", NULL, 0, NULL);

      if (isSoundOn) {
        if (strcmp(selectedLevel, "MEDIUM") == 0)
          mciSendString("play bgMusicMedium from 0 repeat", NULL, 0, NULL);
        else
          mciSendString("play bgMusic from 0 repeat", NULL, 0, NULL);
      }
      return;
    }
  }

  if (key == 'e' || key == 'E') {
    if (gameOver || gameWin) {
      gameState = 0; // Return to Main Menu
      mciSendString("stop bgMusic", NULL, 0, NULL);
      mciSendString("stop bgMusicMedium", NULL, 0, NULL);
      mciSendString("stop bossMusic", NULL, 0, NULL);
      mciSendString("stop winMusic", NULL, 0, NULL);
      mciSendString("stop env2Music", NULL, 0, NULL);
      if (isSoundOn)
        mciSendString("play menuMusic from 0 repeat", NULL, 0, NULL);
    }
  }

  if (key == 'v' || key == 'V') {
    triggerVoidPulse();
  }

  if (key == ' ' && gameState == 1 && !gameOver && !respawnActive && !gameWin) {
    // Restricted ammo in Hard mode
    if (strcmp(selectedLevel, "HARD") == 0) {
      if (playerAmmo <= 0) {
        return; // Click empty, cannot fire
      }
    }

    // Weapon heat is disabled in Environment 1 and 2
    if (currentEnvironment > 2) {
      if (weaponOverheated)
        return;
      weaponHeat += 4.0f;
      if (weaponHeat >= 100.0f) {
        weaponHeat = 100.0f;
        weaponOverheated = true;
      }
    }

    if (strcmp(selectedLevel, "HARD") == 0) {
      playerAmmo--;
    }

    // Play shooting sound
    if (isSoundOn)
      mciSendString("play shoot from 0", NULL, 0, NULL);

    for (int i = 0; i < MAX_BULLET; i++) {
      if (!bulletActive[i]) {
        bulletActive[i] = true;
        bulletX[i] = charX + 220;
        bulletY[i] = charY + 100;
        bulletDX[i] = BULLET_SPEED;
        bulletDY[i] = rand() % 9 - 4;
        bulletAngle[i] = 0;
        break;
      }
    }
  }
}

void iSpecialKeyboard(unsigned char key) {
  if (explosionActive || gameOver || respawnActive)
    return;

  // In Environment 1 and 2, engines run at peak efficiency with no heat
  int speed = 20;
  if (currentEnvironment > 2 && engineOverheated)
    speed = 7;

  // Up & Down
  if (key == GLUT_KEY_UP) {
    charY += speed;
    if (currentEnvironment > 2)
      engineHeat += 2.0f;
    if (charY > SCREEN_HEIGHT - 200)
      charY = SCREEN_HEIGHT - 200;
  }
  if (key == GLUT_KEY_DOWN) {
    charY -= speed;
    if (currentEnvironment > 2)
      engineHeat += 2.0f;
    if (charY < 0)
      charY = 0;
  }

  // Left & Right
  if (key == GLUT_KEY_LEFT) {
    charX -= speed;
    if (currentEnvironment > 2)
      engineHeat += 2.0f;
    if (charX < 0)
      charX = 0;
  }
  if (key == GLUT_KEY_RIGHT) {
    charX += speed;
    if (currentEnvironment > 2)
      engineHeat += 2.0f;
    if (charX > SCREEN_WIDTH - 200)
      charX = SCREEN_WIDTH - 200;
  }

  if (currentEnvironment > 2 && engineHeat >= 100.0f) {
    engineHeat = 100.0f;
    engineOverheated = true;
  }
}

/* -------------------- MOUSE FUNCTIONS (REQUIRED) -------------------- */

void iMouseMove(int mx, int my) {
  // Mouse move while button pressed
}

void iPassiveMouseMove(int mx, int my) {
  // Mouse move without button pressed
}

void iMouse(int button, int state, int mx, int my) {
  if (gameState == -2 && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    storyStep++;
    if (storyStep > 6) {
      gameState = -1; // Go to Intro
    }
  } else if (gameState == -1 && button == GLUT_LEFT_BUTTON &&
             state == GLUT_DOWN) {
    gameState = 0; // Go to Main Menu
    if (isSoundOn)
      mciSendString("play menuMusic from 0 repeat", NULL, 0, NULL);
  } else if (gameState == 0 && button == GLUT_LEFT_BUTTON &&
             state == GLUT_DOWN) {
    // Coordinate-based hitboxes for the background image (options.jpg)
    printf("DEBUG: Menu Click at (%d, %d)\n", mx, my); 

    int minX = SCREEN_WIDTH * 0.55;
    int maxX = SCREEN_WIDTH * 0.85;

    // START (Button 1)
    if (mx >= minX && mx <= maxX && my >= SCREEN_HEIGHT * 0.65 &&
        my <= SCREEN_HEIGHT * 0.77) {
      gameState = 2; // Selection
    }
    // INSTRUCTIONS (Button 2)
    else if (mx >= minX && mx <= maxX && my >= SCREEN_HEIGHT * 0.53 &&
             my <= SCREEN_HEIGHT * 0.65) {
      gameState = -3;
    }
    // SCORE (Button 3)
    else if (mx >= minX && mx <= maxX && my >= SCREEN_HEIGHT * 0.41 &&
             my <= SCREEN_HEIGHT * 0.53) {
      gameState = 8; // Go to Scores Menu
    }
    // SOUND (Button 4)
    else if (mx >= minX && mx <= maxX && my >= SCREEN_HEIGHT * 0.29 &&
             my <= SCREEN_HEIGHT * 0.41) {
      printf("DEBUG: Sound Button Clicked. Setting state to 7.\n");
      gameState = 7; // Go to Sound Options
    }
    // EXIT (Button 5)
    else if (mx >= minX && mx <= maxX && my >= SCREEN_HEIGHT * 0.17 &&
             my <= SCREEN_HEIGHT * 0.29) {
      exit(0);
    }
  } else if (gameState == -3 && button == GLUT_LEFT_BUTTON &&
             state == GLUT_DOWN) {
    // Back Button on Instructions Screen - Matches drawing coordinates
    int btnW = SCREEN_WIDTH * 0.15;
    int btnH = SCREEN_HEIGHT * 0.06;
    int btnX = SCREEN_WIDTH / 2 - btnW / 2;
    int btnY = SCREEN_HEIGHT * 0.1;

    if (mx >= btnX && mx <= btnX + btnW && my >= btnY && my <= btnY + btnH) {
      printf(">> BACK button hit detected\n");
      gameState = 0; // Return to Menu
    } else {
      printf(">> BACK button click MISSED. Button area: x(%d-%d), y(%d-%d)\n",
             btnX, btnX + btnW, btnY, btnY + btnH);
    }
  } else if (gameState == 7 && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
      // Sound Options Menu (State 7) - High Precision Alignment with sound_option.jpg
      int minX = SCREEN_WIDTH * 0.55;
      int maxX = SCREEN_WIDTH * 0.82;

      // SOUND ON (Top Crystal)
      if (mx >= minX && mx <= maxX && my >= SCREEN_HEIGHT * 0.53 &&
          my <= SCREEN_HEIGHT * 0.64) {
          if (!isSoundOn) {
             isSoundOn = true;
             mciSendString("play menuMusic from 0 repeat", NULL, 0, NULL);
          }
      }
      // SOUND OFF (Middle Crystal)
      else if (mx >= minX && mx <= maxX && my >= SCREEN_HEIGHT * 0.40 &&
               my <= SCREEN_HEIGHT * 0.51) {
          if (isSoundOn) {
              isSoundOn = false;
              mciSendString("stop menuMusic", NULL, 0, NULL);
              mciSendString("stop bgMusic", NULL, 0, NULL);
              mciSendString("stop bgMusicMedium", NULL, 0, NULL);
              mciSendString("stop bossMusic", NULL, 0, NULL);
              mciSendString("stop env2Music", NULL, 0, NULL);
          }
      }
      // BACK (Bottom Crystal)
      else if (mx >= minX && mx <= maxX && my >= SCREEN_HEIGHT * 0.27 &&
               my <= SCREEN_HEIGHT * 0.38) {
          printf("DEBUG: Back Button Identified. Returning to menu.\n");
          gameState = 0;
      }
  } else if (gameState == 8 && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
      // Score Menu (State 8)
      int minX = SCREEN_WIDTH * 0.40;
      int maxX = SCREEN_WIDTH * 0.90;

      // RESET ALL (Button 4 slot height area)
      if (mx >= minX && mx <= maxX && my >= SCREEN_HEIGHT * 0.29 &&
          my <= SCREEN_HEIGHT * 0.41) {
          resetScores();
          printf("Scores Reset.\n");
      }
      // BACK (Button 5 slot height area)
      else if (mx >= minX && mx <= maxX && my >= SCREEN_HEIGHT * 0.17 &&
               my <= SCREEN_HEIGHT * 0.29) {
          gameState = 0;
      }
  } else if (gameState == 2 && button == GLUT_LEFT_BUTTON &&
             state == GLUT_DOWN) {
    // Level Selection (EASY, MEDIUM, HARD)
    int minX = SCREEN_WIDTH * 0.55;
    int maxX = SCREEN_WIDTH * 0.85;

    // EASY
    if (mx >= minX && mx <= maxX && my >= SCREEN_HEIGHT * 0.65 &&
        my <= SCREEN_HEIGHT * 0.77) {
      enemyBulletSpeed = 3;
      enemiesToWin = 20; // Updated to 20
      enemySpeed = 5;
      strcpy_s(selectedLevel, "EASY");

      resetGame();
      gameState = 1;
      mciSendString("stop menuMusic", NULL, 0, NULL);
      if (isSoundOn)
        mciSendString("play bgMusic from 0 repeat", NULL, 0, NULL);
    }
    // MEDIUM
    else if (mx >= minX && mx <= maxX && my >= SCREEN_HEIGHT * 0.53 &&
             my <= SCREEN_HEIGHT * 0.65) {
      mediumStoryStep = 0;
      gameState = 4; // Medium Story Narration
      mciSendString("stop menuMusic", NULL, 0, NULL);
      mciSendString("stop menuMusic", NULL, 0, NULL);
      if (isSoundOn)
        mciSendString("play bgMusicMedium from 0 repeat", NULL, 0, NULL);
    }
    // HARD
    else if (mx >= minX && mx <= maxX && my >= SCREEN_HEIGHT * 0.41 &&
             my <= SCREEN_HEIGHT * 0.53) {
      hardStoryStep = 0;
      gameState = 5; // Hard Story Narration
      mciSendString("stop menuMusic", NULL, 0, NULL);
      if (isSoundOn)
        mciSendString("play bgMusicMedium from 0 repeat", NULL, 0, NULL);
    }
    // BACK (Top Right Corner as clarified by user)
    else if (mx >= SCREEN_WIDTH - 200 && my >= SCREEN_HEIGHT - 100) {
      gameState = 0;
    }
  } else if (gameState == 4 && button == GLUT_LEFT_BUTTON &&
             state == GLUT_DOWN) {
    mediumStoryStep++;
    if (mediumStoryStep > 1) {
      enemyBulletSpeed = 4;
      enemiesToWin = 20; // Updated to 20
      enemySpeed = 7;
      strcpy_s(selectedLevel, "MEDIUM");

      resetGame();
      gameState = 1;
      // Music is already playing from the start of story
    }
  } else if (gameState == 1 && button == GLUT_LEFT_BUTTON &&
             state == GLUT_DOWN) {
    // Check for 3 Dash  Icon click - Top Left
    int dashX = 20;
    int dashY = SCREEN_HEIGHT - 40;
    // Increased hit area for better responsiveness
    if (mx >= dashX - 10 && mx <= dashX + 50 && my >= dashY - 40 &&
        my <= dashY + 20) {
      printf("Dash icon clicked - pausing\n");
      gameState = 3; // Pause Game
    }
  } else if (gameState == 3 && button == GLUT_LEFT_BUTTON &&
             state == GLUT_DOWN) {
    // 3-Button Crystalline Layout Hitboxes for pause.png
    int minX = SCREEN_WIDTH * 0.50;
    int maxX = SCREEN_WIDTH * 0.95;

    // RESUME (Top Button)
    if (mx >= minX && mx <= maxX && my >= SCREEN_HEIGHT * 0.68 &&
        my <= SCREEN_HEIGHT * 0.92) {
      gameState = 1;
    }
    // RESTART (Middle Button)
    else if (mx >= minX && mx <= maxX && my >= SCREEN_HEIGHT * 0.38 &&
             my <= SCREEN_HEIGHT * 0.62) {
      resetGame();
      gameState = 1;

      mciSendString("stop menuMusic", NULL, 0, NULL);
      mciSendString("stop bgMusic", NULL, 0, NULL);
      mciSendString("stop bgMusicMedium", NULL, 0, NULL);
      mciSendString("stop bossMusic", NULL, 0, NULL);
      mciSendString("stop env2Music", NULL, 0, NULL);
      
      if (isSoundOn) {
        if (strcmp(selectedLevel, "MEDIUM") == 0)
          mciSendString("play bgMusicMedium from 0 repeat", NULL, 0, NULL);
        else
          mciSendString("play bgMusic from 0 repeat", NULL, 0, NULL);
      }
    }
    // MAIN MENU (Bottom Button)
    else if (mx >= minX && mx <= maxX && my >= SCREEN_HEIGHT * 0.08 &&
             my <= SCREEN_HEIGHT * 0.32) {
      gameState = 0;
      mciSendString("stop bgMusic", NULL, 0, NULL);
      mciSendString("stop bgMusicMedium", NULL, 0, NULL);
      mciSendString("stop bossMusic", NULL, 0, NULL);
      mciSendString("stop winMusic", NULL, 0, NULL);
      mciSendString("play menuMusic from 0 repeat", NULL, 0, NULL);
    }
  }
}

void triggerVoidPulse() {
  if (strcmp(selectedLevel, "MEDIUM") != 0 &&
      strcmp(selectedLevel, "HARD") != 0)
    return;
  // Let the player pulse during the boss fight
  if (superCooldown > 0 || gameOver || gameWin || respawnActive)
    return;

  pulseActive = true;
  pulseRadius = 0;
  pulseDuration = 100; // 5 seconds at 50ms ticks
  superCooldown = MAX_SUPER_COOLDOWN;

  // 1. Clear all enemy bullets
  for (int i = 0; i < MAX_ENEMY_BULLET; i++) {
    enemyBulletActive[i] = false;
  }

  // 2. Push back enemies
  for (int i = 0; i < MAX_ENEMY; i++) {
    if (enemyActive[i]) {
      // If enemy is on the right half, push it further right
      if (enemyX[i] > charX) {
        enemyX[i] += 200;
        if (enemyX[i] > SCREEN_WIDTH)
          enemyX[i] = SCREEN_WIDTH - 100;
      }
    }
  }

  // 3. Damage boss slightly if active
  if (bossActive && !echoShieldActive) {
    bossHP -= 1;
  }

  mciSendString("play explode from 0", NULL, 0, NULL); // Pulse sound effect
}

/* -------------------- PLAYER HIT & RESPAWN -------------------- */
void playerHit() {
  if (gameWin)
    return;
  if (isShielded) {
    isShielded = false; // Consume shield
    return;             // No damage
  }

  if (playerLives <= 0)
    return;

  playerLives--;
  explosionActive = true;
  explosionFrame = 0;
  explosionX = charX;
  explosionY = charY;

  respawnActive = true;
  respawnTimer = 0;

  // Play explosion sound
  MCIERROR playErr = mciSendString("play explode from 0", NULL, 0, NULL);
  if (playErr) {
    char errBuf[256];
    mciGetErrorString(playErr, errBuf, sizeof(errBuf));
    printf("Player explosion play error: %s\n", errBuf);
  }

  if (playerLives <= 0) {
    gameOver = true;
    checkHighScore(); // Check for record on Game Over
    warningActive = false; // Clear targeting on game over
    // Stop all gameplay music
    mciSendString("stop bgMusic", NULL, 0, NULL);
    mciSendString("stop bgMusicMedium", NULL, 0, NULL);
    mciSendString("stop bossMusic", NULL, 0, NULL);
    mciSendString("stop env2Music", NULL, 0, NULL);
    mciSendString("play menuMusic repeat", NULL, 0, NULL);
  }
}

void handleRespawn() {
  if (!respawnActive)
    return;

  respawnTimer++;

  if (respawnTimer >= 60) {
    respawnActive = false;
    explosionActive = false;
    charX = 50;
    charY = SCREEN_HEIGHT / 2 - 100;
  }
}
/* -------------------- COLLISIONS -------------------- */
void checkShipCollision() {
  if (gameOver || respawnActive || gameWin)
    return;

  for (int i = 0; i < MAX_ENEMY; i++) {
    if (!enemyActive[i])
      continue;

    if (charX < enemyX[i] + ENEMY_SIZE && charX + 200 > enemyX[i] &&
        charY < enemyY[i] + ENEMY_SIZE && charY + 200 > enemyY[i]) {
      enemyActive[i] = false;
      playerHit();
      break;
    }
  }

  // Obstacle Collision
  for (int i = 0; i < MAX_OBSTACLE; i++) {
    if (!obstacleActive[i])
      continue;

    if (charX < obstacleX[i] + OBSTACLE_SIZE && charX + 200 > obstacleX[i] &&
        charY < obstacleY[i] + OBSTACLE_SIZE && charY + 200 > obstacleY[i]) {
      obstacleActive[i] = false;
      playerHit();
      break;
    }
  }
}

/* -------------------- ANIMATION -------------------- */
void animateExplosion() {
  if (!explosionActive)
    return;

  explosionFrame++;
  if (explosionFrame >= 10) // Increased frames for longer animation
    explosionActive = false;
}

/* -------------------- WIN CHECK -------------------- */
void checkGameWin() {
  if (enemiesDestroyed >= enemiesToWin) // Removed star requirement
  {
    if (!bossActive && bossHP > 0) {
      bossActive = true;

      if (strcmp(selectedLevel, "HARD") == 0) {
        isEchoMind = true;
        maxBossHP = 30;
        bossHP = maxBossHP;
        bossWidth = 350;
        bossHeight = 350;
        playerAmmo = 200; // Large restock for the Boss phase
      } else if (strcmp(selectedLevel, "MEDIUM") == 0) {
        isEchoMind = true;
        maxBossHP = 15;
        bossHP = maxBossHP;
        bossWidth = 300;
        bossHeight = 300;
      } else {
        isEchoMind = false;
        maxBossHP = 10;
        bossHP = maxBossHP;
        bossWidth = 240;
        bossHeight = 240;
      }

      bossX = SCREEN_WIDTH - bossWidth - 50;
      bossY = SCREEN_HEIGHT / 2 - bossHeight / 2;

      // Switch to Boss Music
      if (isSoundOn) {
        mciSendString("stop bgMusic", NULL, 0, NULL);
        mciSendString("stop bgMusicMedium", NULL, 0, NULL);
        mciSendString("stop env2Music", NULL, 0, NULL);
        mciSendString("play bossMusic repeat", NULL, 0, NULL);
      }

      // Deactivate regular mechanics for Boss Fight
      warningActive = false;
      smartShotTimer = 120;

      // Deactivate all regular enemies
      for (int i = 0; i < MAX_ENEMY; i++)
        enemyActive[i] = false;
    }
  }
}

/* -------------------- MOVE BULLETS -------------------- */
void moveBullet() {
  for (int i = 0; i < MAX_BULLET; i++) {
    if (!bulletActive[i])
      continue;

    bulletX[i] += bulletDX[i];
    bulletY[i] += bulletDY[i] + 5 * sin(bulletAngle[i] * 0.1);
    bulletAngle[i]++;

    for (int j = 0; j < MAX_ENEMY; j++) {
      if (!enemyActive[j])
        continue;

      // Adjusted collision for smaller bullet (20x20)
      if (bulletX[i] < enemyX[j] + ENEMY_SIZE && bulletX[i] + 20 > enemyX[j] &&
          bulletY[i] < enemyY[j] + ENEMY_SIZE && bulletY[i] + 20 > enemyY[j]) {
        bulletActive[i] = false;
        enemyActive[j] = false;
        score += 10;
        enemiesDestroyed++;

        // Transition to Environment 2 at 10 kills in Hard Mode
        if (strcmp(selectedLevel, "HARD") == 0 && enemiesDestroyed == 10 &&
            currentEnvironment == 1) {
          currentEnvironment = 2; // Jump to Gravity Zone
          gameState = 6;          // Enter Story Transition
          env2StoryStep = 0;      // Reset cutscene progress
          playerAmmo = 200;       // Restock Ammo for Env 2 (Expanded capacity)
          
          // Clear active bullets to prevent unfair hits naturally from gravity shift
          for (int k = 0; k < MAX_ENEMY_BULLET; k++) enemyBulletActive[k] = false;
          for (int k = 0; k < MAX_BULLET; k++) bulletActive[k] = false;

          // Play a sound to signify environmental shift
          if (isSoundOn)
            mciSendString("play collect from 0", NULL, 0, NULL);
        }

        // Play explosion sound for enemy hit
        if (isSoundOn) {
          MCIERROR playErr = mciSendString("play explode from 0", NULL, 0, NULL);
          if (playErr) {
            char errBuf[256];
            mciGetErrorString(playErr, errBuf, sizeof(errBuf));
            printf("Enemy explosion play error: %s\n", errBuf);
          }
        }

        checkGameWin();

        // Spawn Ammo Crate chance (15%) in HARD Mode
        if (strcmp(selectedLevel, "HARD") == 0 && !ammoActive) {
            if (rand() % 100 < 15) {
                ammoActive = true;
                ammoX = enemyX[j];
                ammoY = enemyY[j];
            }
        }

        break;
      }
    }

    if (bulletX[i] > SCREEN_WIDTH || bulletY[i] > SCREEN_HEIGHT ||
        bulletY[i] < 0)
      bulletActive[i] = false;
  }
}

/* -------------------- ENEMY -------------------- */
void moveEnemy() {
  if (gameWin)
    return;
  for (int i = 0; i < MAX_ENEMY; i++) {
    if (enemyActive[i]) {
      if (enemyStunTimer[i] > 0) {
        enemyStunTimer[i]--;
        continue; // Skip movement while stunned
      }

      // --- BASIC MOVEMENT WITH AI MODIFIERS ---
      float speedModifier = 1.0f;
      bool isHard = (strcmp(selectedLevel, "HARD") == 0);
      bool isMedium = (strcmp(selectedLevel, "MEDIUM") == 0);

      if (isHard) {
        if (enemyAIStates[i] == 1)
          speedModifier = -1.5f; // Feint: Retreat back
        else if (enemyAIStates[i] == 2)
          speedModifier = 3.5f; // Strike: Rapid Charge
      }

      if (enemySpawnType[i] == 1) // Bottom Spawner (Void Interceptor)
      {
        enemyY[i] += (int)(enemySpeed * 1.5 * speedModifier);
        // High-frequency tremor effect (vibration)
        enemyX[i] += (int)(3 * sin(enemyY[i] * 0.2));

        if (enemyY[i] > SCREEN_HEIGHT + 80)
          enemyActive[i] = false;
        if (enemyY[i] < -ENEMY_SIZE - 100)
          enemyActive[i] = false; // Cleanup if pushed/feinted below
      } else                      // Side Spawner
      {
        enemyX[i] -= (int)(enemySpeed * speedModifier);
        if (enemyX[i] < -ENEMY_SIZE)
          enemyActive[i] = false;
        if (enemyX[i] > SCREEN_WIDTH + 100)
          enemyActive[i] = false; // Cleanup if feinted off-screen
      }

      // --- ADVANCED REACTIVE AI (Medium/Hard) ---
      if (isMedium || isHard) {
        // 1. RANDOM DRIFTING (PATHING)
        if (enemyAITimer[i] > 0)
          enemyAITimer[i]--;
        else {
          enemyTargetY[i] = rand() % (SCREEN_HEIGHT - ENEMY_SIZE);
          enemyAITimer[i] =
              40 + rand() % 60; // Pick new target every few seconds
        }

        // Move toward target Y
        if (enemyY[i] < enemyTargetY[i])
          enemyY[i] += 2;
        else if (enemyY[i] > enemyTargetY[i])
          enemyY[i] -= 2;

        // 2. DECEPTIVE FEINT & STRIKE (Hard Mode)
        if (isHard) {
          if (enemyStateTimer[i] > 0)
            enemyStateTimer[i]--;
          else {
            if (enemyAIStates[i] == 0) // Normal
            {
              if (rand() % 100 < 2) // 2% chance per frame check to feint
              {
                enemyAIStates[i] = 1;                  // Retreat
                enemyStateTimer[i] = 30 + rand() % 30; // 1-3 seconds
              }
            } else if (enemyAIStates[i] == 1) // End Retreat -> Begin Strike
            {
              enemyAIStates[i] = 2;                  // Strike
              enemyStateTimer[i] = 40 + rand() % 40; // 2-4 seconds

              // Play a sound or visual indicator?
              // Logic: Fires more in Strike state (handled in enemyFire)
            } else // End Strike -> Normal
            {
              enemyAIStates[i] = 0;
              enemyStateTimer[i] = 0;
            }
          }
        }

        // 3. SMARTER REACTIVE DODGING (Hard Mode Logic)
        if (enemyDodgeCooldown[i] > 0)
          enemyDodgeCooldown[i]--;

        // Hard Mode always dodges, Medium only "Special" enemies
        if (isHard || (isMedium && isSpecialEnemy[i])) {
          for (int b = 0; b < MAX_BULLET; b++) {
            if (bulletActive[b]) {
              // Bullet is in front and horizontally close
              if (bulletX[b] < enemyX[i] && bulletX[b] > enemyX[i] - 180) {
                // Bullet is on a collision course vertically
                if (abs((bulletY[b] + 10) - (enemyY[i] + ENEMY_SIZE / 2)) <
                    60) {
                  if (enemyDodgeCooldown[i] <= 0) {
                    // PERFORM DODGE LEAP (High-Speed shift)
                    int shift = 40;
                    if (enemyY[i] < SCREEN_HEIGHT / 2)
                      enemyY[i] += shift;
                    else
                      enemyY[i] -= shift;

                    enemyDodgeCooldown[i] = 15; // Cooldown for 0.7s
                    enemyTargetY[i] =
                        enemyY[i]; // Update pathing target to new spot
                    break;
                  }
                }
              }
            }
          }
        }
      }

      // Boundary fix for any AI shifts
      if (enemyY[i] < 0)
        enemyY[i] = 0;
      if (enemyY[i] > SCREEN_HEIGHT - ENEMY_SIZE)
        enemyY[i] = SCREEN_HEIGHT - ENEMY_SIZE;
    }
  }
}

/* -------------------- OBSTACLES -------------------- */
void spawnObstacle() {
  if (gameWin || bossActive || gameOver || gameState != 1)
    return;
  for (int i = 0; i < MAX_OBSTACLE; i++) {
    if (!obstacleActive[i]) {
      obstacleActive[i] = true;
      obstacleX[i] = SCREEN_WIDTH;
      obstacleY[i] = rand() % (SCREEN_HEIGHT - OBSTACLE_SIZE);
      obstacleAngle[i] = 0;
      break;
    }
  }
}

void moveObstacle() {
  if (gameWin || gameOver || gameState != 1)
    return;
  for (int i = 0; i < MAX_OBSTACLE; i++) {
    if (obstacleActive[i]) {
      obstacleX[i] -= OBSTACLE_SPEED;
      obstacleAngle[i] += 5; // Rotate 5 degrees per frame
      if (obstacleX[i] < -OBSTACLE_SIZE)
        obstacleActive[i] = false;
    }
  }
}

void spawnEnemy() {
  if (gameWin || bossActive || gameOver || gameState != 1)
    return;

  // Level-based spawn limits
  int activeCount = 0;
  for (int i = 0; i < MAX_ENEMY; i++)
    if (enemyActive[i])
      activeCount++;

  int limit = 3; // EASY default
  if (strcmp(selectedLevel, "MEDIUM") == 0)
    limit = 3;
  if (strcmp(selectedLevel, "HARD") == 0)
    limit = 8; // Hard Mode has more enemies (Increased from 6)

  if (activeCount >= limit)
    return;

  for (int i = 0; i < MAX_ENEMY; i++) {
    if (!enemyActive[i]) {
      enemyActive[i] = true;
      enemyBulletCount[i] = 0;
      enemyAITimer[i] = 20 + rand() % 40; // Decision every 1-3 seconds
      enemyTargetY[i] = rand() % (SCREEN_HEIGHT - ENEMY_SIZE);
      enemyDodgeCooldown[i] = 0;
      enemyAIStates[i] = 0;
      enemyStateTimer[i] = 0;

      if (strcmp(selectedLevel, "HARD") == 0 && rand() % 10 == 0) {
        // BOTTOM SPAWN (Reduced chance further, 10%)
        enemySpawnType[i] = 1;
        enemyX[i] =
            (SCREEN_WIDTH / 2) + rand() % (SCREEN_WIDTH / 2 - ENEMY_SIZE);
        enemyY[i] = -ENEMY_SIZE;
        isSpecialEnemy[i] =
            true; // Bottom enemies are always special behavioral class
      } else {
        // SIDE SPAWN (Default)
        enemySpawnType[i] = 0;
        enemyX[i] = SCREEN_WIDTH;
        enemyY[i] = rand() % (SCREEN_HEIGHT - ENEMY_SIZE);
      }

      // Special Enemy Chance on Medium/Hard
      isSpecialEnemy[i] = false;
      if (strcmp(selectedLevel, "MEDIUM") == 0 ||
          strcmp(selectedLevel, "HARD") == 0) {
        if (rand() % 10 < 3)
          isSpecialEnemy[i] = true;
      }

      break;
    }
  }
}

void spawnEnemyBullet(int enemyIdx) {
  if (!enemyActive[enemyIdx])
    return;
  int shotLimit = (enemyAIStates[enemyIdx] == 2)
                      ? 3
                      : 1; // Striking enemies fire 3x as many shots
  if (enemyBulletCount[enemyIdx] >= shotLimit)
    return;

  for (int i = 0; i < MAX_ENEMY_BULLET; i++) {
    if (!enemyBulletActive[i]) {
      enemyBulletActive[i] = true;
      enemyBulletType[i] = (enemySpawnType[enemyIdx] == 1 ? 1 : 0);
      enemyBulletCount[enemyIdx]++;      // Increment shot count
      if (enemySpawnType[enemyIdx] == 1) // Bottom Spawner - Firing UP
      {
        enemyBulletX[i] = enemyX[enemyIdx]; // Left side
        enemyBulletY[i] =
            enemyY[enemyIdx] + ENEMY_SIZE; // Still from the 'front' top edge

        enemyBulletDX[i] = rand() % 5 - 2;       // Slight horizontal spread
        enemyBulletDY[i] = enemyBulletSpeed + 4; // Fast upward shot
      } else                                     // Side Spawner - Firing LEFT
      {
        enemyBulletX[i] =
            enemyX[enemyIdx] - ENEMY_BULLET_SIZE; // Front side (Left)
        enemyBulletY[i] =
            enemyY[enemyIdx] + (ENEMY_SIZE / 2) - (ENEMY_BULLET_SIZE / 2);

        enemyBulletDX[i] = -enemyBulletSpeed - 3;
        enemyBulletDY[i] = 0;
      }

      break;
    }
  }
}

void moveEnemyBullets() {
  for (int i = 0; i < MAX_ENEMY_BULLET; i++) {
    if (!enemyBulletActive[i])
      continue;

    enemyBulletX[i] += enemyBulletDX[i];
    enemyBulletY[i] += enemyBulletDY[i];

    if (enemyBulletX[i] < 0 || enemyBulletX[i] > SCREEN_WIDTH ||
        enemyBulletY[i] < 0 || enemyBulletY[i] > SCREEN_HEIGHT) {
      enemyBulletActive[i] = false;
    }

    if (!gameOver && !respawnActive) {
      if (enemyBulletX[i] < charX + 200 &&
          enemyBulletX[i] + ENEMY_BULLET_SIZE > charX &&
          enemyBulletY[i] < charY + 200 &&
          enemyBulletY[i] + ENEMY_BULLET_SIZE > charY) {
        enemyBulletActive[i] = false;
        playerHit();
      }
    }
  }
}

void spawnSmartBullet(int targetX, int targetY) {
  if (gameState != 1 || gameOver || gameWin)
    return;

  int shooterIdx = -1;
  for (int i = 0; i < MAX_ENEMY; i++) {
    if (enemyActive[i]) {
      shooterIdx = i;
      break;
    }
  }
  if (shooterIdx == -1)
    return;

  for (int i = 0; i < MAX_ENEMY_BULLET; i++) {
    if (!enemyBulletActive[i]) {
      enemyBulletActive[i] = true;
      enemyBulletX[i] = enemyX[shooterIdx] + 40;
      enemyBulletY[i] = enemyY[shooterIdx] + 40;

      float dx = (float)targetX - enemyBulletX[i];
      float dy = (float)targetY - enemyBulletY[i];
      float dist = sqrt(dx * dx + dy * dy);

      if (dist > 0) {
        enemyBulletDX[i] = (dx / dist) * 10;
        enemyBulletDY[i] = (dy / dist) * 10;
      } else {
        enemyBulletDX[i] = -10;
        enemyBulletDY[i] = 0;
      }
      break;
    }
  }
}

void enemyFire() {
  if (gameState != 1 || gameOver || gameWin)
    return;
  for (int i = 0; i < MAX_ENEMY; i++) {
    if (enemyActive[i]) {
      spawnEnemyBullet(i); // fire directly
    }
  }
}
void spawnStar() {
  if (gameOver || gameWin || gameState != 1)
    return;

  for (int i = 0; i < MAX_STAR; i++) {
    if (!starActive[i]) {
      starActive[i] = true;
      // LEFT SIDE SPAWN
      starX[i] = rand() % (SCREEN_WIDTH / 3); // left third of screen
      starY[i] = rand() % (SCREEN_HEIGHT - STAR_SIZE);

      starTimer[i] = 0; // Reset timer for new star
      starSpawnTime[i] = GetTickCount();
      break;
    }
  }
}

// Star collection
void checkStarCollection() {
  for (int i = 0; i < MAX_STAR; i++) {
    if (!starActive[i])
      continue;

    if (charX < starX[i] + STAR_SIZE && charX + 200 > starX[i] &&
        charY < starY[i] + STAR_SIZE && charY + 200 > starY[i]) {
      starActive[i] = false;
      score += 5;        // extra points for collecting star
      isShielded = true; // Give shield
      if (isSoundOn)
        mciSendString("play collect from 0", NULL, 0, NULL);
    }
  }
}

void updateStars() {
  if (gameState != 1)
    return;
  for (int i = 0; i < MAX_STAR; i++) {
    if (!starActive[i])
      continue;

    starTimer[i]++;

    if (starTimer[i] >= STAR_DURATION_TICKS) {
      starActive[i] = false;
    }
  }
}

void animateStars() {
  for (int i = 0; i < MAX_STAR; i++) {
    if (!starActive[i])
      continue;

    if (starGrow[i]) {
      starScale[i] += 1;
      if (starScale[i] >= STAR_SIZE + 10)
        starGrow[i] = false;
    } else {
      starScale[i] -= 1;
      if (starScale[i] <= STAR_SIZE - 10)
        starGrow[i] = true;
    }
  }
}

/* -------------------- INIT -------------------- */
void loadAssets() {
  mciSendString("close all", NULL, 0, NULL);
  
  // Environment 1 Cycle (4 custom images for maximum variety) - Used in Hard Mode
  env1Sequence[0] = iLoadImage("Images//env1_1.jpg");
  env1Sequence[1] = iLoadImage("Images//env1_2.jpg");
  env1Sequence[2] = iLoadImage("Images//env1_3.jpg");
  env1Sequence[3] = iLoadImage("Images//env1_4.jpg");

  // Load original backgrounds for Easy Mode
  bg1 = iLoadImage("Images//bg1.jpg");
  bg2 = iLoadImage("Images//bg2.jpg");

  bgM1 = iLoadImage("Images//bg_m1.jpg");
  bgM2 = iLoadImage("Images//bg_m2.jpg");
  
  // Environment 2 Cycle (4 custom images)
  env2Sequence[0] = iLoadImage("Images//env2_1.png"); 
  env2Sequence[1] = iLoadImage("Images//env2_2.png"); 
  env2Sequence[2] = iLoadImage("Images//env2_3.png"); 
  env2Sequence[3] = iLoadImage("Images//env2_4.png"); 

  // Keep legacy for compatibility
  bgEnv2_1 = env2Sequence[0];
  bgEnv2_2 = env2Sequence[1];

  bulletImg = iLoadImage("Images//bullet.png");
  enemyImg = iLoadImage("Images//enemy.png");
  bottomEnemyImg = iLoadImage("Images//enemy1.png");
  enemyBulletImg = iLoadImage("Images//enemy_bullet.png");
  specialBulletImg = iLoadImage("Images//special_bullet.png");

  for (int i = 0; i < 3; i++) {
    char path[50];
    sprintf_s(path, "Images//c%d.png", i + 1);
    charImg[i] = iLoadImage(path);
  }

  for (int i = 0; i < 5; i++) {
    char path[50];
    sprintf_s(path, "Images//exp%d.png", i + 1);
    explosionImg[i] = iLoadImage(path);
  }

  starImg = iLoadImage("Images//star.png");
  obstacleImg = iLoadImage("Images//obstacle.png");
  bossImg = iLoadImage("Images//enemy.png");  // Reusing enemy image for now
  introImg = iLoadImage("Images//intro.jpg"); // Corrected filename
  storyBg = iLoadImage("Images//bg11.jpg");
  menuBg = iLoadImage("Images//options.jpg");
  specialEnemyImg = iLoadImage("Images//special_enemy.png");
  levelSelectionImg = iLoadImage("Images//level_selection.jpg");
  pauseMenuImg = iLoadImage("Images//pause2.png");
  blackHoleImg = iLoadImage("Images//bomb.png"); // The new Black Hole visual
  ammoImg = iLoadImage("Images//ammo.png");       // The new Ammo Supply visual
  soundOptionBg = iLoadImage("Images/sound_option.jpg");
  scoreOptionBg = iLoadImage("Images/score_option.jpg");

  // Initialize Audio
  char mciErr[256];

  // Open Menu Music (Background Sound) - use mpegvideo for concurrent playback
  MCIERROR err1 = mciSendString(
      "open \"Audio/background_sound.wav\" type mpegvideo alias menuMusic",
      mciErr, sizeof(mciErr), NULL);
  if (err1) {
    mciGetErrorString(err1, mciErr, sizeof(mciErr));
    printf("menuMusic open error: %s\n", mciErr);
  }

  // Open Gameplay Music (Space Shooter Soundtrack) - use mpegvideo for
  // concurrent playback
  MCIERROR err2 = mciSendString("open \"Audio/space_shooter_soundtrack.wav\" "
                                "type mpegvideo alias bgMusic",
                                mciErr, sizeof(mciErr), NULL);
  if (err2) {
    mciGetErrorString(err2, mciErr, sizeof(mciErr));
    printf("bgMusic open error: %s\n", mciErr);
  }

  // Open Medium Gameplay Music
  MCIERROR err5 = mciSendString("open \"Audio/background_medium_sound.wav\" "
                                "type mpegvideo alias bgMusicMedium",
                                mciErr, sizeof(mciErr), NULL);
  if (err5) {
    mciGetErrorString(err5, mciErr, sizeof(mciErr));
    printf("bgMusicMedium open error: %s\n", mciErr);
  }

  // Open Boss Music
  MCIERROR err4 = mciSendString(
      "open \"Audio/boss_fight_sound.wav\" type mpegvideo alias bossMusic",
      mciErr, sizeof(mciErr), NULL);
  if (err4) {
    mciGetErrorString(err4, mciErr, sizeof(mciErr));
    printf("bossMusic open error: %s\n", mciErr);
  }

  // Open Environment 2 Music
  MCIERROR errEnv2 = mciSendString(
      "open \"Audio/env2.wav\" type mpegvideo alias env2Music",
      mciErr, sizeof(mciErr), NULL);
  if (errEnv2) {
    mciGetErrorString(errEnv2, mciErr, sizeof(mciErr));
    printf("env2Music open error: %s\n", mciErr);
  }

  // Open Sound Effects
  MCIERROR errS = mciSendString(
      "open \"Audio/shooting_sound.wav\" type mpegvideo alias shoot", mciErr,
      sizeof(mciErr), NULL);
  if (errS) {
    mciGetErrorString(errS, mciErr, sizeof(mciErr));
    printf("shoot open error: %s\n", mciErr);
  }

  MCIERROR errE = mciSendString(
      "open \"Audio/explosion_sound.wav\" type mpegvideo alias explode", mciErr,
      sizeof(mciErr), NULL);
  if (errE) {
    mciGetErrorString(errE, mciErr, sizeof(mciErr));
    printf("explode open error: %s\n", mciErr);
  }

  MCIERROR errC =
      mciSendString("open \"Audio/collect.wav\" type mpegvideo alias collect",
                    mciErr, sizeof(mciErr), NULL);
  if (errC) {
    mciGetErrorString(errC, mciErr, sizeof(mciErr));
    printf("collect open error: %s\n", mciErr);
  }

  mciSendString("setaudio collect volume to 1000", NULL, 0, NULL);
  mciSendString("setaudio explode volume to 1000", NULL, 0, NULL);
  mciSendString("setaudio shoot volume to 1000", NULL, 0, NULL);

  MCIERROR errW = mciSendString(
      "open \"Audio/win_sound.wav\" type mpegvideo alias winMusic", mciErr,
      sizeof(mciErr), NULL);
  if (errW) {
    mciGetErrorString(errW, mciErr, sizeof(mciErr));
    printf("winMusic open error: %s\n", mciErr);
  }

  // Start playing Menu Music (Game Over sound) loop immediately
  MCIERROR err3 =
      mciSendString("play menuMusic repeat", mciErr, sizeof(mciErr), NULL);
  if (err3) {
    mciGetErrorString(err3, mciErr, sizeof(mciErr));
    printf("menuMusic play error: %s\n", mciErr);
  }
}

void resetGame() {
  score = 0;
  gameOver = false;
  gameWin = false;
  playerLives = 5;
  respawnActive = false;
  explosionActive = false;
  enemiesDestroyed = 0;
  starsCollected = 0;
  isShielded = false;
  currentEnvironment = 1;

  bossActive = false;
  bossHP = 10;
  maxBossHP = 10;
  bossX = -100;
  bossY = -100;
  bossDirection = 1;

  isEchoMind = false;
  echoTeleportTimer = 0;
  echoShieldTimer = 0;
  echoShieldActive = false;
  echoCircularTimer = 0;
  echoTeleportAlpha = 255;

  for (int i = 0; i < MAX_BULLET; i++)
    bulletActive[i] = false;
  for (int i = 0; i < MAX_ENEMY; i++) {
    enemyActive[i] = false;
    enemyStunTimer[i] = 0;
    enemyTargetY[i] = SCREEN_HEIGHT / 2;
    enemyAITimer[i] = 0;
    enemyDodgeCooldown[i] = 0;
    enemyAIStates[i] = 0;
    enemyStateTimer[i] = 0;
  }
  for (int i = 0; i < MAX_ENEMY_BULLET; i++)
    enemyBulletActive[i] = false;
  for (int i = 0; i < MAX_OBSTACLE; i++) {
    obstacleActive[i] = false;
    obstacleAngle[i] = 0;
  }

  charX = 50;
  charY = SCREEN_HEIGHT / 2 - 100;

  for (int i = 0; i < MAX_STAR; i++) {
    starActive[i] = false;
    starTimer[i] = 0;
    starScale[i] = STAR_SIZE;
    starGrow[i] = true;
  }

  // Reset Music
  mciSendString("stop bossMusic", NULL, 0, NULL);
  mciSendString("stop winMusic", NULL, 0, NULL);

  // Smart Shooting Reset
  smartShotTimer = 120; // Increased cooldown
  warningActive = false;
  warningTicks = 0;
  lastCharX = charX;
  lastCharY = charY;

  // Superpower Reset
  pulseDuration = 0;

  // Overheat and Ammo Reset
  weaponHeat = 0;
  engineHeat = 0;
  weaponOverheated = false;
  engineOverheated = false;
  playerAmmo = 100;
  
  // Reset Seeds
  for (int i = 0; i < MAX_SEEDS; i++) seedActive[i] = false;
  seedDropCooldown = 0;

  // Reset Ammo Pickup
  ammoActive = false;

  // Reset Background Cycle
  bgCycleIndex = 0;
}

/* -------------------- BOSS LOGIC -------------------- */
void moveBoss() {
  if (!bossActive || gameOver || gameWin)
    return;

  if (isEchoMind) {
    // 1. Teleportation Logic - Faster in Phase 2 & Hard Mode
    echoTeleportTimer++;
    int teleportThreshold = (bossHP <= maxBossHP / 2) ? 40 : 80;
    if (strcmp(selectedLevel, "HARD") == 0)
      teleportThreshold -= 20; // Extra speed for Hard Mode

    if (echoTeleportTimer > teleportThreshold) {
      bossX = SCREEN_WIDTH - bossWidth - (rand() % 400);
      bossY = rand() % (SCREEN_HEIGHT - bossHeight);
      echoTeleportTimer = 0;

      // Reset Alpha for fade effect (visual only)
      echoTeleportAlpha = 100;
    }

    // 2. Shield Cycle
    echoShieldTimer++;
    if (!echoShieldActive && echoShieldTimer > 200) // Every 10 sec
    {
      echoShieldActive = true;
      echoShieldTimer = 0;
    } else if (echoShieldActive && echoShieldTimer > 60) // 3 sec duration
    {
      echoShieldActive = false;
      echoShieldTimer = 0;
    }

    // 3. Circular Pattern - Scaled for Phase 2 & Level
    echoCircularTimer++;
    if (echoCircularTimer > 80) // 4 sec
    {
      int numBullets = (bossHP <= maxBossHP / 2) ? 15 : 12;
      if (strcmp(selectedLevel, "HARD") == 0)
        numBullets += 5; // Extra bullets for Hard Mode

      for (int i = 0; i < numBullets; i++) {
        double angle = (360.0 / numBullets) * i * (3.14159 / 180.0);
        for (int j = 0; j < MAX_ENEMY_BULLET; j++) {
          if (!enemyBulletActive[j]) {
            enemyBulletActive[j] = true;
            enemyBulletX[j] = bossX + bossWidth / 2;
            enemyBulletY[j] = bossY + bossHeight / 2;
            enemyBulletDX[j] = (bossHP <= maxBossHP / 2 ? 10 : 8) * cos(angle);
            enemyBulletDY[j] = (bossHP <= maxBossHP / 2 ? 10 : 8) * sin(angle);
            break;
          }
        }
      }
      echoCircularTimer = 0;
      mciSendString("play shoot from 0", NULL, 0, NULL);
    }

    // 4. Black Hole Seed Dropping (Hard Mode)
    if (strcmp(selectedLevel, "HARD") == 0) {
      seedDropCooldown++;
      if (seedDropCooldown > 100 && !echoShieldActive) { // Every 5 seconds Drop a Seed
        for (int i = 0; i < MAX_SEEDS; i++) {
          if (!seedActive[i]) {
            seedActive[i] = true;
            seedX[i] = bossX + bossWidth / 2;
            seedY[i] = bossY + bossHeight / 2;
            seedState[i] = 0; // Arming
            seedTimer[i] = 40; // 2 seconds to arm (at 20 ticks/sec map loop)
            seedDropCooldown = 0;
            break;
          }
        }
      }
    }
  } else {
    bossY += 3 * bossDirection;
    if (bossY >= SCREEN_HEIGHT - bossHeight || bossY <= 0)
      bossDirection *= -1;

    if (rand() % 50 == 0) {
      for (int i = 0; i < MAX_ENEMY_BULLET; i++) {
        if (!enemyBulletActive[i]) {
          enemyBulletActive[i] = true;
          enemyBulletX[i] = bossX;
          enemyBulletY[i] = bossY + bossHeight / 2;
          enemyBulletDX[i] = -10;
          enemyBulletDY[i] = 0;
          break;
        }
      }
    }
  }
}

void checkBossCollision() {
  if (!bossActive || gameOver || gameWin)
    return;

  // Check Player Bullets hitting Boss
  for (int i = 0; i < MAX_BULLET; i++) {
    if (bulletActive[i]) {
      // Adjusted collision for smaller bullet (20x20)
      if (bulletX[i] + 20 >= bossX && bulletX[i] <= bossX + bossWidth &&
          bulletY[i] + 20 >= bossY && bulletY[i] <= bossY + bossHeight) {
        bulletActive[i] = false;

        if (isEchoMind && echoShieldActive) {
          // Direct hit on shield - deflected
          continue;
        }

        bossHP--;
        mciSendString("play explode from 0", NULL, 0,
                      NULL); // Reuse explosion sound

        if (bossHP <= 0) {
          bossActive = false;
          gameWin = true;
          checkHighScore(); // Check for record on Win
          warningActive = false; // Clear targeting on boss death
          mciSendString("stop bossMusic", NULL, 0, NULL);
          mciSendString("stop bgMusic", NULL, 0, NULL);
          mciSendString("stop bgMusicMedium", NULL, 0, NULL);
          mciSendString("play winMusic from 0", NULL, 0, NULL);
        }
      }
    }
  }

  // Check Boss colliding with Player
  if (charX + 200 >= bossX && charX <= bossX + bossWidth &&
      charY + 200 >= bossY && charY <= bossY + bossHeight) {
    playerHit();
  }
}

/* -------------------- BACKGROUND -------------------- */
void changeBackground() {
  bgX -= currentScrollSpeed;
  if (bgX <= -SCREEN_WIDTH) {
    bgX += SCREEN_WIDTH;
    bgCycleIndex = (bgCycleIndex + 1) % 4; // Advance to next segment in the 4-part loop
  }
}

/* -------------------- SHIP ANIMATION -------------------- */
void animateShip() {
  if (gameState == 0)
    return;
  if (gameOver)
    return;

  if (strcmp(selectedLevel, "HARD") == 0) {
    idx = 2; // Unique Ship C3 for Hard Mode
    return;
  }

  idx++;
  if (idx >= 3)
    idx = 0;
}

/* -------------------- TIMER WRAPPERS -------------------- */
void fastUpdate() {
  if (gameState != 1)
    return;
  moveBullet();
  moveEnemyBullets();
  handleRespawn();
  changeBackground();
  checkBossCollision(); // check boss hits
}

void mediumUpdate() {
  if (gameState != 1)
    return;

  // Mechanics for Environment 2: Gravity
  if (currentEnvironment == 2 && !gameOver && !respawnActive) {
    // Constant downward pull
    charY -= 4; // Moderate gravity force
    if (charY < 0)
      charY = 0;
  }

  // Processing Black Hole Seeds
  if (bossActive && isEchoMind) {
    for (int i = 0; i < MAX_SEEDS; i++) {
      if (seedActive[i]) {
        seedTimer[i]--;
        
        if (seedState[i] == 0) { // Arming (Stationary)
          if (seedTimer[i] <= 0) {
            seedState[i] = 1; // Start Chasing
            seedTimer[i] = 80; // Chase for 4 seconds (at 20 ticks/sec or similarly adjusted)
          }
        } else if (seedState[i] == 1) { // Chasing
            if (seedTimer[i] <= 0) {
                seedState[i] = 2; // Chase timeout, enter Pulling
                seedTimer[i] = 100;
                if (isSoundOn)
                  mciSendString("play explode from 0", NULL, 0, NULL);
            } else {
                // Homing Math
                float dx = (charX + 100) - seedX[i];
                float dy = (charY + 100) - seedY[i];
                float dist = sqrt(dx*dx + dy*dy);
                
                if (dist > 1.0f) {
                    float chaseSpeed = 10.0f;
                    seedX[i] += (dx / dist) * chaseSpeed;
                    seedY[i] += (dy / dist) * chaseSpeed;
                }
                
                // Collision with player ship
                if (dist < 80.0f && !gameOver && !respawnActive) {
                    seedState[i] = 2; // Impact! Transition to Pulling
                    seedTimer[i] = 100;
                    
                    // Sabotage: Drain Ammo
                    playerAmmo -= 15;
                    if (playerAmmo < 0) playerAmmo = 0;
                    
                    if (isSoundOn)
                      mciSendString("play explode from 0", NULL, 0, NULL);
                }
            }
        } else if (seedState[i] == 2 && !gameOver && !respawnActive) { // Pulling (Stationary)
          if (seedTimer[i] <= 0) {
            seedActive[i] = false; // Dissipate
          } else {
            // Pull the player
            float dx = seedX[i] - (charX + 100);
            float dy = seedY[i] - (charY + 100);
            float dist = sqrt(dx*dx + dy*dy);
            
            if (dist < 450.0f && dist > 20.0f) { // Within pull range
              float pullStrength = 15.0f; // Pull force
              charX += (dx / dist) * pullStrength;
              charY += (dy / dist) * pullStrength;
              
              // Bounds check
              if (charX < 0) charX = 0;
              if (charX > SCREEN_WIDTH - 200) charX = SCREEN_WIDTH - 200;
              if (charY < 0) charY = 0;
              if (charY > SCREEN_HEIGHT - 200) charY = SCREEN_HEIGHT - 200;
            }
          }
        }
      }
    }
  }

  // Processing Ammo Pickup Physics
  if (ammoActive) {
      ammoX -= 5;
      if (ammoX < -100) ammoActive = false;
  }

  moveEnemy();
  animateStars();
  animateShip();
  moveBoss(); // move boss
  moveObstacle();

  // Update Superpower Cooldown
  if (superCooldown > 0)
    superCooldown--;

  // Update Pulse Expansion
  if (pulseActive) {
    if (pulseRadius < MAX_PULSE_RADIUS)
      pulseRadius += 40;

    if (pulseDuration > 0) {
      pulseDuration--;

      // CONTINUOUS DESTRUCTION
      // 1. Clear all enemy bullets
      for (int i = 0; i < MAX_ENEMY_BULLET; i++)
        enemyBulletActive[i] = false;

      // 2. Push/Stun enemies instead of destroying
      for (int i = 0; i < MAX_ENEMY; i++) {
        if (enemyActive[i]) {
          // PUSH BACK
          if (enemySpawnType[i] == 1) // Bottom
          {
            enemyY[i] -= 15;
            if (enemyY[i] < ENEMY_SIZE)
              enemyY[i] = ENEMY_SIZE; // Don't push below bottom edge
          } else                      // Side
          {
            enemyX[i] += 15;
            if (enemyX[i] > SCREEN_WIDTH - ENEMY_SIZE)
              enemyX[i] = SCREEN_WIDTH - ENEMY_SIZE;
          }

          // STUN
          enemyStunTimer[i] = 20; // Stun for ~1 second during pulse
        }
      }

      // 3. Clear obstacles
      for (int i = 0; i < MAX_OBSTACLE; i++)
        obstacleActive[i] = false;
    } else {
      pulseActive = false;
      pulseRadius = 0;
    }
  }

  // Smart Shooting Logic (Medium/Hard)
  if ((strcmp(selectedLevel, "MEDIUM") == 0 ||
       strcmp(selectedLevel, "HARD") == 0) &&
      !bossActive && !gameOver && !gameWin) {
    int vx = charX - lastCharX;
    int vy = charY - lastCharY;
    lastCharX = charX;
    lastCharY = charY;

    if (!warningActive) {
      smartShotTimer--;
      if (smartShotTimer <= 0) {
        warningActive = true;
        warningTicks = 20;            // 1 second at 50ms ticks
        predictedX = charX + vx * 20; // Predict 1s ahead
        predictedY = charY + vy * 20;

        // Screen bounds check
        if (predictedX < 50)
          predictedX = 50;
        if (predictedX > SCREEN_WIDTH - 250)
          predictedX = SCREEN_WIDTH - 250;
        if (predictedY < 50)
          predictedY = 50;
        if (predictedY > SCREEN_HEIGHT - 250)
          predictedY = SCREEN_HEIGHT - 250;
      }
    } else {
      warningTicks--;
      if (warningTicks <= 0) {
        spawnSmartBullet(predictedX + 100, predictedY + 100);
        warningActive = false;

        // Difficulty-scaled cooldown: Hard is 2x faster
        if (strcmp(selectedLevel, "HARD") == 0)
          smartShotTimer = 60; // 3 second wait
        else
          smartShotTimer = 120; // 6 second wait
      }
    }
  }

  // Overheat System: Faster Cooling decay to make it easier
  if (weaponHeat > 0)
    weaponHeat -= 2.5f; // Increased from 1.2
  if (weaponHeat <= 0) {
    weaponHeat = 0;
    weaponOverheated = false;
  }

  if (engineHeat > 0)
    engineHeat -= 2.0f; // Increased from 1.0
  if (engineHeat <= 0) {
    engineHeat = 0;
    engineOverheated = false;
  }
}

void collisionUpdate() {
  if (gameState != 1)
    return;
  checkShipCollision();
  animateExplosion();
  checkStarCollection();

  // Ammo Pickup Collection
  if (ammoActive && !gameOver && !respawnActive) {
      if (charX < ammoX + 60 && charX + 200 > ammoX &&
          charY < ammoY + 60 && charY + 200 > ammoY) {
          ammoActive = false;
          playerAmmo += 25;
          if (isSoundOn)
            mciSendString("play collect from 0", NULL, 0, NULL);
      }
  }
}

/* -------------------- MAIN -------------------- */
int main() {
  SCREEN_WIDTH = GetSystemMetrics(SM_CXSCREEN);
  SCREEN_HEIGHT = GetSystemMetrics(SM_CYSCREEN);

  iInitialize(SCREEN_WIDTH, SCREEN_HEIGHT, "Space Shooter");

  loadAssets();
  loadScores(); // Load scores from file
  resetGame();  // Initialize variables

  /* Timers calibrated for EASY mode - Consolidated to under 10 timers */
  iSetTimer(20, fastUpdate);      // Moves bullets, respawn, background
  iSetTimer(50, mediumUpdate);    // Moves enemies, animates stars
  iSetTimer(80, collisionUpdate); // Checks collisions, explosions

  iSetTimer(2500, spawnEnemy);
  iSetTimer(2000, enemyFire); // Slower firing
  iSetTimer(4000, spawnStar);
  iSetTimer(5000, spawnObstacle);
  iSetTimer(100, updateStars);

  iStart();
  return 0;
}