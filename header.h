#include <GL/glut.h>
#include <GL/freeglut.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define MAX_ANTS 1000
#define FOOD_RADIUS 10
#define MAX_SPEED 10
#define SPEED_FACTOR 0.15
#define DIRECTION_CHANGE_TIME 2 // Time interval in seconds for direction change
#define RUN_TIME 1 // Time interval in seconds for generating food
#define FOOD_SIZE 100 // Time interval in seconds for generating food
#define FOOD_INTERVAL 5

typedef struct {
    int antNumber;     // Ant number
    float x, y;          // Ant position
    float speed;         // Ant speed
    float angle;         // Ant movement angle
    float changeTimer;   // Timer for direction change
    int excretedStrong;
    int excretedWeak;
    int isStopped;
    int drawnWeak;
    int foodIndex;
    int eatingStartTime;

} Ant;

typedef struct {
    float x, y;  // Pheromone position
    float currentX, currentY;
    int pos;
    float radius;
    int foodIndex;
} Pheromone;

typedef struct {
    float x, y;  // Pheromone position
    float currentX, currentY;
    int pos;
    float radius;
    int foodIndex;
} WeakPheromone;

typedef struct {
    float x, y;  // Pheromone position
    int foodIndex;
    int isEat;
    int piecesRemaining;
} Food;


float pheromoneRadius;
float smellRadius;
int foodCount = 0; // Number of food items
int numAnts;
int foodInterval;
int runTime;
int eatingPortion;
int weakPheromoneRadius;
time_t startTime; // Start time for tracking elapsed time

time_t startRunning;
Pheromone pheromones[MAX_ANTS];
WeakPheromone weakPheromone[MAX_ANTS];
Ant ants[MAX_ANTS];
pthread_t threads[MAX_ANTS];
pthread_mutex_t mutex;
Food food[(RUN_TIME * 60) / FOOD_INTERVAL];

float randFloat(float min, float max);

float randAngle();

void initializeAnts();

void moveAnt(Ant *ant);

void *moveAntThread(void *arg);

void generateFood();

void updateScene(int value);

void drawAnt(Ant *ant, float circleRadius, float secondCircleRadius);

void drawFood();

void drawScene();

void handleKeypress(unsigned char key, int x, int y);

void initOpenGLWindow(int argc, char **argv);

void startAntThreads();

void joinAntThreads();

void exitDisplay();

int antEnteredPheromone(Ant *ant);

int antEnteredWeakPheromone(Ant *ant);

int antEnteredFood(Ant *ant, float radius);

int ScalingRadius(Ant *ant, float radius);

int ScalingWeakRadius(Ant *ant);

void userDefined();