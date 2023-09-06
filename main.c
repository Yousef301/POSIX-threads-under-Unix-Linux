#include "header.h"

// Function to generate a random float between min and max
float randFloat(float min, float max) {
    return min + ((float) rand() / RAND_MAX) * (max - min);
}

// Function to generate a random angle between 0 and 360 degrees
float randAngle() {
    return randFloat(0, 360);
}

// Function to initialize ants with random positions, speeds, and angles
void initializeAnts() {
    srand(time(NULL));
    for (int i = 0; i < numAnts; i++) {
        ants[i].antNumber = i;  // Assign the ant number (starting from 1)
        ants[i].excretedStrong = 0;  // Assign the ant number (starting from 1)
        ants[i].excretedWeak = 0;  // Assign the ant number (starting from 1)
        ants[i].x = randFloat(0, WINDOW_WIDTH);
        ants[i].y = randFloat(0, WINDOW_HEIGHT);
        ants[i].speed = randFloat(1, MAX_SPEED) * SPEED_FACTOR;
        ants[i].angle = randAngle();
        ants[i].changeTimer = DIRECTION_CHANGE_TIME;
        ants[i].isStopped = 0;
        ants[i].drawnWeak = 0;
        ants[i].foodIndex = -1;
    }
}

// Function to move an ant in its current direction
void moveAnt(Ant *ant) {
    float dx = ant->speed * cos(ant->angle * M_PI / 180.0);
    float dy = ant->speed * sin(ant->angle * M_PI / 180.0);

    float newX = ant->x + dx;
    float newY = ant->y + dy;
    if (ant->speed == 0.0) {
        // Ant is already stopped, no need to move
        return;
    }
    // Check if ant hits the wall
    if (newX < 0 || newX > WINDOW_WIDTH || newY < 0 || newY > WINDOW_HEIGHT) {
        float hitX = newX < 0 ? 0 : (newX > WINDOW_WIDTH ? WINDOW_WIDTH : newX);
        float hitY = newY < 0 ? 0 : (newY > WINDOW_HEIGHT ? WINDOW_HEIGHT : newY);

        float angleToHit = atan2(hitY - ant->y, hitX - ant->x) * 180.0 / M_PI;
        float cwAngle = angleToHit - 45.0;
        float ccwAngle = angleToHit + 45.0;

        ant->angle = (rand() % 2 == 0) ? cwAngle : ccwAngle; // Randomly choose CW or CCW (±45 degrees)
        ant->angle = fmod(ant->angle, 360.0); // Ensure angle stays within 0-360 range

        newX = ant->x + ant->speed * cos(ant->angle * M_PI / 180.0);
        newY = ant->y + ant->speed * sin(ant->angle * M_PI / 180.0);

        ant->x = fmax(0, fmin(WINDOW_WIDTH, newX)); // Clamp x position within window boundaries
        ant->y = fmax(0, fmin(WINDOW_HEIGHT, newY)); // Clamp y position within window boundaries
    } else {
        ant->x = newX;
        ant->y = newY;
    }
}

int antEnteredPheromone(Ant *ant) {
    for (int i = 0; i < numAnts; i++) {
        float greenCenterX = pheromones[i].x; // x-coordinate of the center of the green circle
        float greenCenterY = pheromones[i].y; // y-coordinate of the center of the green circle
        float dx = ant->x - greenCenterX;
        float dy = ant->y - greenCenterY;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance <= pheromones[i].radius && pheromones[i].foodIndex != -1) {
            // Ant entered the pheromone circle
            return pheromones[i].foodIndex;
        }
    }

    return -1;
}

int antEnteredWeakPheromone(Ant *ant) {
    for (int i = 0; i < numAnts; i++) {
        float greenCenterX = weakPheromone[i].x; // x-coordinate of the center of the green circle
        float greenCenterY = weakPheromone[i].y; // y-coordinate of the center of the green circle
        float dx = ant->x - greenCenterX;
        float dy = ant->y - greenCenterY;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance <= weakPheromone[i].radius && weakPheromone[i].foodIndex != -1) {
            // Ant entered the pheromone circle
            return weakPheromone[i].foodIndex;
        }
    }

    return -1;
}

int antEnteredFood(Ant *ant, float radius) {
    int i;
    for (i = 0; i < foodCount; i++) {
        float dx = ant->x - food[i].x;
        float dy = ant->y - food[i].y;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance < radius && food[i].foodIndex != -1) {
            return i; // Return the index of the entered circle
        }
    }
    return -1; // No food entered
}

int ScalingRadius(Ant *ant, float radius) {

    for (int i = 0; i < foodCount; i++) {
        float dx = ant->x - food[i].x;
        float dy = ant->y - food[i].y;
        float distance = sqrt(dx * dx + dy * dy);
        if (distance < radius) {
            return distance;  // Return the index of the entered circle
        }
    }

    return -1;  // No food entered
}

int ScalingWeakRadius(Ant *ant) {

    for (int i = 0; i < numAnts; i++) {
        float dx = ant->x - pheromones[i].x;
        float dy = ant->y - pheromones[i].y;
        float distance = sqrt(dx * dx + dy * dy);
        if (distance <= pheromones[i].radius && pheromones[i].pos != ant->antNumber && ant->speed != 0 &&
            pheromones[i].foodIndex != -1) {
            // Ant entered the pheromone circle
            return distance;
        }
    }

    return -1;  // No pheromomne entered
}

void *moveAntThread(void *arg) {
    Ant *ant = (Ant *) arg;
    int foodIndex = -1;
    int pheromoneIndex = -1;
    int weakPhromoneIndex = -1;

    while (1) {
        pthread_mutex_lock(&mutex);
        moveAnt(ant);

        // Check if ant entered a food circle
        foodIndex = antEnteredFood(ant, smellRadius + FOOD_RADIUS);
        // Check if ant entered a pheromone circle
        pheromoneIndex = antEnteredPheromone(ant);
        weakPhromoneIndex = antEnteredWeakPheromone(ant);

        if (foodIndex != -1) {
            // Ant entered a food circle
            if (!ant->isStopped) {
                // Ant was not previously stopped, handle movement
                if (foodIndex >= 0 && foodIndex < foodCount) {
                    // Ant entered a valid red circle
                    // Store ant's pheromone coordinates
                    if (ant->excretedStrong == 0) {
                        ant->excretedStrong = 1;
                        ant->excretedWeak = 0;
                        pheromones[ant->antNumber].x = ant->x;
                        pheromones[ant->antNumber].y = ant->y;
                        pheromones[ant->antNumber].currentX = ant->x;
                        pheromones[ant->antNumber].currentY = ant->y;
                        pheromones[ant->antNumber].pos = ant->antNumber;
                        pheromones[ant->antNumber].foodIndex = foodIndex;
                    }
                    float greenCenterX = food[foodIndex].x; // x-coordinate of the center of the green circle
                    float greenCenterY = food[foodIndex].y; // y-coordinate of the center of the green circle
                    float dx = greenCenterX - ant->x;
                    float dy = greenCenterY - ant->y;
                    float distance = sqrt(dx * dx + dy * dy);

                    if (distance > FOOD_RADIUS) {
                        // Ant is inside the red circle, move towards the corresponding green circle
                        ant->angle = atan2(dy, dx) * 180.0 / M_PI;
                    } else {
                        // Ant has moved to the center of the green circle, stop moving
                        ant->speed = 0.0;
                        ant->foodIndex = foodIndex;
                        ant->isStopped = 1;

                        ant->eatingStartTime = time(NULL); // Record the start time of eating

                        pthread_mutex_unlock(&mutex);

                        int foodPortion = (eatingPortion * FOOD_SIZE) / 100;

                        while (food[foodIndex].piecesRemaining >= 1) {
                            food[foodIndex].piecesRemaining -= foodPortion;
                            sleep(1);
                        }

                        pthread_mutex_lock(&mutex); // Re-acquire the lock

                        food[foodIndex].isEat = 1;
                        food[foodIndex].x = -1000; // Set x coordinate to a value less than the screen width
                        food[foodIndex].y = -1000; // Set y coordinate to a value less than the screen height
                        food[foodIndex].foodIndex = -1;

                        // Remove corresponding pheromones
                        for (int i = 0; i < numAnts; i++) {
                            if (pheromones[i].foodIndex == foodIndex) {
                                pheromones[i].x = -1000;
                                pheromones[i].y = -1000;
                                pheromones[i].currentX = -1000;
                                pheromones[i].currentY = -1000;
                                pheromones[i].foodIndex = -1;
                                pheromones[i].pos = -1;
                            }
                        }

                        ant->speed = randFloat(1, MAX_SPEED) * SPEED_FACTOR;
                        ant->angle = randAngle();
                        ant->isStopped = 0;

                        // Reset ants related to the removed food circle
                        for (int i = 0; i < numAnts; i++) {
                            if (ants[i].foodIndex == foodIndex) {
                                ants[i].excretedStrong = 0;
                                ants[i].excretedWeak = 0;
                                ants[i].speed = randFloat(1, MAX_SPEED) * SPEED_FACTOR;
                                ants[i].angle = randAngle();
                                ants[i].isStopped = 0;
                                ants[i].drawnWeak = 0;
                            }
                        }

                        // Reset pheromones related to the removed food circle
                        for (int i = 0; i < numAnts; i++) {
                            if (weakPheromone[i].foodIndex == foodIndex) {
                                weakPheromone[i].x = -1000;
                                weakPheromone[i].y = -1000;
                                weakPheromone[i].currentX = -1000;
                                weakPheromone[i].currentY = -1000;
                                weakPheromone[i].foodIndex = -1;
                                weakPheromone[i].pos = -1;
                            }
                        }
                    }
                }
            }
        } else if (pheromoneIndex != -1) {
            // Ant entered a pheromone circle
            if (!ant->isStopped) {
                // Ant was not previously stopped, handle movement
                if (ant->excretedWeak == 0) {
                    ant->excretedStrong = 0;
                    ant->excretedWeak = 1;
                    weakPheromone[ant->antNumber].x = ant->x;
                    weakPheromone[ant->antNumber].y = ant->y;
                    weakPheromone[ant->antNumber].currentX = ant->x;
                    weakPheromone[ant->antNumber].currentY = ant->y;
                    weakPheromone[ant->antNumber].pos = ant->antNumber;
                    weakPheromone[ant->antNumber].foodIndex = pheromoneIndex;
                }
                float redCenterX = food[pheromoneIndex].x; // x-coordinate of the center of the red circle
                float redCenterY = food[pheromoneIndex].y; // y-coordinate of the center of the red circle
                float dx = redCenterX - ant->x;
                float dy = redCenterY - ant->y;
                float distance = sqrt(dx * dx + dy * dy);

                if (distance > FOOD_RADIUS) {
                    // Ant is inside the pheromone circle, move towards the center of the red circle
                    ant->angle = atan2(dy, dx) * 180.0 / M_PI;
                }
            }
        } else if (weakPhromoneIndex != -1) {
            // Ant entered a weak pheromone circle
            if (!ant->isStopped) {
                // Ant was not previously stopped, handle movement
                float weakCenterX = food[weakPhromoneIndex].x; // x-coordinate of the center of the weak pheromone circle
                float weakCenterY = food[weakPhromoneIndex].y; // y-coordinate of the center of the weak pheromone circle
                float dx = weakCenterX - ant->x;
                float dy = weakCenterY - ant->y;
                float distance = sqrt(dx * dx + dy * dy);

                if (distance > FOOD_RADIUS) {
                    pthread_mutex_unlock(&mutex);
                    float foodCenterX = food[weakPhromoneIndex].x; // x-coordinate of the center of the weak pheromone circle
                    float foodCenterY = food[weakPhromoneIndex].y; // y-coordinate of the center of the weak pheromone circle
                    time_t start_time = time(NULL);
                    while (antEnteredWeakPheromone(ant) != -1) {
                        time_t current_time = time(NULL); // Get the current time
                        if (current_time - start_time >= 1) {
                            // Calculate the angle towards the food position
                            float dx = foodCenterX - ant->x;
                            float dy = foodCenterY - ant->y;
                            float desiredAngle = atan2(dy, dx) * 180.0 / M_PI;

                            // Adjust the angle by 5 degrees towards the food position
                            if (desiredAngle > ant->angle) {
                                ant->angle += 5;
                            } else {
                                ant->angle -= 5;
                            }
                            start_time = current_time;
                        }
                        moveAnt(ant);
                        usleep(10000);
                    }
                    pthread_mutex_lock(&mutex);
                }
            }
        }

        pthread_mutex_unlock(&mutex);

        usleep(10000);
    }
}


// Function to generate food at a random location
void generateFood() {
    if (foodCount < (runTime * 60) / foodInterval) {
        food[foodCount].x = randFloat(10, WINDOW_WIDTH - 10);
        food[foodCount].y = randFloat(10, WINDOW_HEIGHT - 10);
        food[foodCount].foodIndex = foodCount;
        food[foodCount].piecesRemaining = FOOD_SIZE;
        foodCount++;

    }
}

// Function to update the scene
void updateScene(int value) {
    pthread_mutex_lock(&mutex);

    time_t currentTime = time(NULL);
    double elapsed = difftime(currentTime, startTime);

    if (elapsed >= foodInterval) {
        generateFood();
        startTime = currentTime;
    }

    pthread_mutex_unlock(&mutex);

    if (time(NULL) - startRunning <= 60 * runTime) {
        glutPostRedisplay(); // Mark the current window as needing to be redisplayed
        glutTimerFunc(10, updateScene, 0); // Schedule the next update
    } else
        glutTimerFunc(0, exitDisplay, 0); // Schedule the next update

}

void drawAnt(Ant *ant, float circleRadius, float secondCircleRadius) {
    float antSize = 5.0; // Ant size

    glPushMatrix();
    glTranslatef(ant->x, ant->y, 0.0); // Translate to ant position

    // Draw first circle (pheromone circle)
    glColor3f(1.0, 1.0, 1.0); // White color for the circle edge
    glBegin(GL_LINE_LOOP);
    int numSegments = 100;
    for (int i = 0; i <= numSegments; i++) {
        float theta = 2.0 * M_PI * (float) i / numSegments;
        float px = circleRadius * cos(theta);
        float py = circleRadius * sin(theta);
        glVertex2f(px, py);
    }
    glEnd();

    // Draw ant body
    glColor3f(0.5, 0.5, 0.5); // Gray color
    glBegin(GL_POLYGON);
    glVertex2f(-antSize / 2, antSize / 2);
    glVertex2f(antSize / 2, antSize / 2);
    glVertex2f(antSize / 2, -antSize / 2);
    glVertex2f(-antSize / 2, -antSize / 2);
    glEnd();

    // Draw ant eyes
    glColor3f(1.0, 1.0, 1.0); // White color
    glBegin(GL_POINTS);
    glVertex2f(-antSize / 4, antSize / 4);
    glVertex2f(antSize / 4, antSize / 4);
    glEnd();

    // Draw second circle (additional circle)
    glColor3f(0.0, 1.0, 0.0); // Green color for the second circle
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= numSegments; i++) {
        float theta = 2.0 * M_PI * (float) i / numSegments;
        float px = secondCircleRadius * cos(theta);
        float py = secondCircleRadius * sin(theta);
        glVertex2f(px, py);
    }
    glEnd();

    glPopMatrix();
}


// Function to draw the food items
void drawFood() {
    float squareSize = 2 * FOOD_RADIUS; // Size of the food square
    float outerRadius = FOOD_RADIUS + smellRadius; // User-defined radius for the outer circle
    int numSegments = 100;

    for (int i = 0; i < foodCount; i++) {
        if (!food[i].isEat) {
            float x = food[i].x;
            float y = food[i].y;

            glPushMatrix();
            glTranslatef(x, y, 0.0); // Translate to food position

            // Draw outer red circle (foodSmell)
            glColor3f(1.0, 0.0, 0.0); // Red color
            float lineWidth = 2.0; // Line width for the red circle
            glLineWidth(lineWidth);
            glBegin(GL_LINE_LOOP);
            for (int j = 0; j < numSegments; j++) {
                float theta = 2.0 * M_PI * (float) j / numSegments;
                float px = outerRadius * cos(theta);
                float py = outerRadius * sin(theta);
                glVertex2f(px, py);
            }
            glEnd();

            // Draw food circle
            glColor3f(0.0, 1.0, 0.0); // Green color
            glBegin(GL_POLYGON);
            glVertex2f(-squareSize / 4, squareSize / 4);
            glVertex2f(squareSize / 4, squareSize / 4);
            glVertex2f(squareSize / 4, -squareSize / 4);
            glVertex2f(-squareSize / 4, -squareSize / 4);
            glEnd();

            glPopMatrix();
        }
    }
}


// Function to draw the scene
void drawScene() {
    glClear(GL_COLOR_BUFFER_BIT);  // Clear the current buffer

    pthread_mutex_lock(&mutex);

    drawFood();

    // Draw ants
    for (int i = 0; i < numAnts; i++) {
        int distanceToPheromone = ScalingWeakRadius(&ants[i]);
        int distanceToRedCircle = ScalingRadius(&ants[i], FOOD_RADIUS + smellRadius);

        if (distanceToRedCircle != -1 && distanceToPheromone == -1) {
            // Ant entered a red circle, calculate the circle radius based on distance to food
            float maxDistance = FOOD_RADIUS + smellRadius;
            float maxCircleRadius = 1.0 + (maxDistance * 2); // Adjust the scaling factor as needed
            float minCircleRadius = 1.0; // Assuming initial circle radius is 1.0
            float circleRadius =
                    maxCircleRadius - (distanceToRedCircle / maxDistance) * (maxCircleRadius - minCircleRadius);
            pheromones[i].radius = circleRadius;
            // No need to modify the weak pheromone circle radius here
            pheromones[i].x = ants[i].x;
            pheromones[i].y = ants[i].y;
            drawAnt(&ants[i], pheromones[i].radius, weakPheromone[i].radius);
        } else if (distanceToPheromone != -1 && distanceToRedCircle == -1) {
            // Ant is inside another ant's pheromone circle and not inside a red circle
            // Calculate the weak pheromone circle radius based on distance to the other ant's pheromone circle
            float maxDistance1 = 3 * (weakPheromoneRadius + smellRadius); // Adjust the scaling factor as needed
            float minDistance1 = 0.0;
            float maxCircleRadius1 = 1.0 + (maxDistance1 * 2); // Adjust the scaling factor as needed
            float minCircleRadius1 = 1.0; // Assuming initial circle radius is 1.0
            float circleRadius1 =
                    maxCircleRadius1 - (distanceToPheromone / maxDistance1) * (maxCircleRadius1 - minCircleRadius1);
            weakPheromone[i].radius = circleRadius1;
            pheromones[i].radius = 0.0; // Vanish the ant's own pheromone circle when inside another ant's pheromone circle
            weakPheromone[i].x = ants[i].x;
            weakPheromone[i].y = ants[i].y;
            drawAnt(&ants[i], 2.0, circleRadius1);
        } else if (distanceToPheromone != -1 && distanceToRedCircle != -1) {
            // Ant is inside both a red circle and another ant's pheromone circle
            // Calculate the pheromone circle radius based on distance to food
            float maxDistance = FOOD_RADIUS + smellRadius;
            float minDistance = 0.0;
            float maxCircleRadius = 1.0 + (maxDistance * 2); // Adjust the scaling factor as needed
            float minCircleRadius = 1.0; // Assuming initial circle radius is 1.0
            float circleRadius =
                    maxCircleRadius - (distanceToRedCircle / maxDistance) * (maxCircleRadius - minCircleRadius);
            pheromones[i].radius = circleRadius;
            weakPheromone[i].radius = 0.0; // Vanish weak pheromone circle when inside a red circle and another ant's pheromone circle
            pheromones[i].x = ants[i].x;
            pheromones[i].y = ants[i].y;
            drawAnt(&ants[i], pheromones[i].radius, weakPheromone[i].radius);
        } else {
            // Ant didn't enter any red circle or is not inside another ant's pheromone circle
            // Keep the circle radius unchanged
            drawAnt(&ants[i], 2.0, 2.0);
            weakPheromone[i].radius = 2.0;
            pheromones[i].radius = 2.0;
        }

        int enteredFoodIndex = antEnteredFood(&ants[i], FOOD_RADIUS);

        if (enteredFoodIndex != -1) {
            // Ant entered the green circle, stop its movement
            ants[i].speed = 0.0;  // Set ant speed to 0 to stop moving
        }
    }

    pthread_mutex_unlock(&mutex);

    glFlush();  // Flush the OpenGL buffers
    glutSwapBuffers();  // Swap the buffers to display the scene
}


// Function to handle key press events
void handleKeypress(unsigned char key, int x, int y) {
    if (key == 27) { // 27 is the ASCII code for the ESC key
        exit(0); // Exit the program
    }
}

// Function to initialize the OpenGL window
void initOpenGLWindow(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Ants");

    glutDisplayFunc(drawScene);
    glutKeyboardFunc(handleKeypress);

    glClearColor(0.0, 0.0, 0.0, 1.0); // Set the background color to black
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT); // Set the 2D coordinate system
}

// Function to create and start threads for moving ants
void startAntThreads() {
    for (int i = 0; i < numAnts; i++) {
        pthread_create(&threads[i], NULL, moveAntThread, &ants[i]);
    }
}

// Function to join threads for moving ants
void joinAntThreads() {
    for (int i = 0; i < numAnts; i++) {
        pthread_join(threads[i], NULL);
    }
}

void exitDisplay() {
    exit(0);
}

void userDefined() {
    FILE *file = fopen("data.txt", "r");
    if (file == NULL) {                                     //reading data for txt file
        printf("Failed to open the file.\n");
        return;
    }
    char line[100];

    while (fgets(line, sizeof(line), file)) {
//         Split the line into key and value
        char *key = strtok(line, ":");
        char *value = strtok(NULL, ":");

//         Remove leading/trailing whitespaces from key and value
        key = strtok(key, " \t\n");
        value = strtok(value, " \t\n");

//         Check the key and update the corresponding variable
        if (strcmp(key, "NumberAnt") == 0) {
            numAnts = atoi(value);
        } else if (strcmp(key, "FoodInterval") == 0) {                                         //data
            foodInterval = atoi(value);
        } else if (strcmp(key, "SmellRadius") == 0) {
            smellRadius = atof(value);
        } else if (strcmp(key, "EatingPortion") == 0) {
            eatingPortion = atoi(value);
        } else if (strcmp(key, "RunTime") == 0) {
            runTime = atoi(value);
        } else if (strcmp(key, "PheromoneRadius") == 0) {
            pheromoneRadius = atoi(value);
        } else if (strcmp(key, "WeakPheromoneRadius") == 0) {
            weakPheromoneRadius = atoi(value);
        }
    }
    printf("Ants: %d\nFood timer: %d\nFood smell radius: %.2f\n"
           "Portion size: %d\nPheromone range: %.2f\nSimulation time: %d min\nWeak pheromone radius: %d\n", numAnts,
           foodInterval, smellRadius,
           eatingPortion, pheromoneRadius, runTime, weakPheromoneRadius);
}

int main(int argc, char **argv) {
    userDefined();
    initializeAnts();

    startRunning = time(NULL);

    for (int i = 0; i < numAnts; i++) {
        pheromones[i].x = 0.0;
        pheromones[i].y = 0.0;
        pheromones[i].radius = pheromoneRadius;
        pheromones[i].foodIndex = -1;  // Initialize with an invalid index
    }
    for (int i = 0; i < numAnts; i++) {
        weakPheromone[i].x = 0.0;
        weakPheromone[i].y = 0.0;
        weakPheromone[i].radius = pheromoneRadius;
        weakPheromone[i].foodIndex = -1;  // Initialize with an invalid index
    }
// Set x and y coordinates outside the screen
    for (int i = 0; i < (runTime * 60) / foodInterval; i++) {
        food[i].x = -2000; // Set x coordinate to a value less than the screen width
        food[i].y = -2000; // Set y coordinate to a value less than the screen height
        food[i].isEat = 0;
        food[i].foodIndex = -1;
    }

    pthread_mutex_init(&mutex, NULL);

    startTime = time(NULL);

    initOpenGLWindow(argc, argv);

    startAntThreads();

    glutTimerFunc(10, updateScene, 0); // Start the update timer

    glutMainLoop(); // Enter the OpenGL main loop

    joinAntThreads();

    pthread_mutex_destroy(&mutex);

    return 0;
}