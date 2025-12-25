#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "traffic_simulation.h"

// Global queues for lanes
Queue laneQueues[4];

// SOFT PASTEL colors for vehicles
const SDL_Color VEHICLE_COLORS[] = {
    {255, 182, 193, 255},   // Light Pink
    {173, 216, 230, 255},   // Light Blue
    {144, 238, 144, 255},   // Light Green
    {255, 228, 196, 255},   // Bisque
    {221, 160, 221, 255},   // Plum
    {176, 224, 230, 255},   // Powder Blue
    {255, 218, 185, 255},   // Peach Puff
    {230, 230, 250, 255},   // Lavender
};

void initializeTrafficLights(TrafficLight *lights) {
    // North light - small bar
    lights[0] = (TrafficLight){
        .state = RED, .timer = 0,
        .position = {INTERSECTION_X - TRAFFIC_LIGHT_WIDTH/2, 
                    INTERSECTION_Y - LANE_WIDTH - TRAFFIC_LIGHT_HEIGHT - 5, 
                    TRAFFIC_LIGHT_WIDTH, TRAFFIC_LIGHT_HEIGHT},
        .direction = DIRECTION_NORTH
    };
    
    // South light - small bar
    lights[1] = (TrafficLight){
        .state = RED, .timer = 0,
        .position = {INTERSECTION_X - TRAFFIC_LIGHT_WIDTH/2, 
                    INTERSECTION_Y + LANE_WIDTH + 5, 
                    TRAFFIC_LIGHT_WIDTH, TRAFFIC_LIGHT_HEIGHT},
        .direction = DIRECTION_SOUTH
    };
    
    // East light - small bar
    lights[2] = (TrafficLight){
        .state = GREEN, .timer = 0,
        .position = {INTERSECTION_X + LANE_WIDTH + 5, 
                    INTERSECTION_Y - TRAFFIC_LIGHT_WIDTH/2, 
                    TRAFFIC_LIGHT_HEIGHT, TRAFFIC_LIGHT_WIDTH},
        .direction = DIRECTION_EAST
    };
    
    // West light - small bar
    lights[3] = (TrafficLight){
        .state = GREEN, .timer = 0,
        .position = {INTERSECTION_X - LANE_WIDTH - TRAFFIC_LIGHT_HEIGHT - 5, 
                    INTERSECTION_Y - TRAFFIC_LIGHT_WIDTH/2, 
                    TRAFFIC_LIGHT_HEIGHT, TRAFFIC_LIGHT_WIDTH},
        .direction = DIRECTION_WEST
    };
}

void updateTrafficLights(TrafficLight *lights) {
    static Uint32 lastUpdate = 0;
    static int cycle = 0;
    Uint32 current = SDL_GetTicks();

    if (current - lastUpdate >= 5000) {
        lastUpdate = current;
        cycle = !cycle;

        if (cycle == 0) {
            lights[DIRECTION_NORTH].state = GREEN;
            lights[DIRECTION_SOUTH].state = GREEN;
            lights[DIRECTION_EAST].state = RED;
            lights[DIRECTION_WEST].state = RED;
        } else {
            lights[DIRECTION_NORTH].state = RED;
            lights[DIRECTION_SOUTH].state = RED;
            lights[DIRECTION_EAST].state = GREEN;
            lights[DIRECTION_WEST].state = GREEN;
        }
    }
}

Vehicle *createVehicle(Direction direction) {
    Vehicle *vehicle = (Vehicle *)malloc(sizeof(Vehicle));
    vehicle->direction = direction;
    vehicle->type = REGULAR_CAR;
    vehicle->active = true;
    vehicle->speed = 2.0f;
    vehicle->state = STATE_MOVING;
    
    // 15% chance to turn left
    int turnChance = rand() % 100;
    if (turnChance < 15) {
        vehicle->turnDirection = TURN_LEFT;
    } else {
        vehicle->turnDirection = TURN_STRAIGHT;
    }
    
    vehicle->colorIndex = rand() % 8;
    vehicle->isTurning = false;
    vehicle->turnProgress = 0.0f;
    vehicle->hasPassedCenter = false;

    if (direction == DIRECTION_NORTH || direction == DIRECTION_SOUTH) {
        vehicle->rect.w = 20;
        vehicle->rect.h = 30;
    } else {
        vehicle->rect.w = 30;
        vehicle->rect.h = 20;
    }

    switch (direction) {
        case DIRECTION_NORTH:
            vehicle->x = INTERSECTION_X + LANE_WIDTH / 2 - vehicle->rect.w / 2;
            vehicle->y = WINDOW_HEIGHT + 10;
            break;
        case DIRECTION_SOUTH:
            vehicle->x = INTERSECTION_X - LANE_WIDTH / 2 - vehicle->rect.w / 2;
            vehicle->y = -40;
            break;
        case DIRECTION_EAST:
            vehicle->x = -40;
            vehicle->y = INTERSECTION_Y + LANE_WIDTH / 2 - vehicle->rect.h / 2;
            break;
        case DIRECTION_WEST:
            vehicle->x = WINDOW_WIDTH + 10;
            vehicle->y = INTERSECTION_Y - LANE_WIDTH / 2 - vehicle->rect.h / 2;
            break;
    }

    vehicle->rect.x = (int)vehicle->x;
    vehicle->rect.y = (int)vehicle->y;

    enqueue(&laneQueues[direction], *vehicle);
    return vehicle;
}

bool shouldStopForVehicleInQueue(Vehicle *vehicle, Direction lane) {
    float criticalDistance = 100.0f;
    
    Node *current = laneQueues[lane].front;
    while (current != NULL) {
        Vehicle *other = &current->vehicle;
        
        if (!other->active || other == vehicle) {
            current = current->next;
            continue;
        }
        
        float distance = 0;
        bool ahead = false;
        
        switch (lane) {
            case DIRECTION_NORTH:
                distance = vehicle->y - other->y;
                ahead = (other->y < vehicle->y) && (distance > 0);
                break;
            case DIRECTION_SOUTH:
                distance = other->y - vehicle->y;
                ahead = (other->y > vehicle->y) && (distance > 0);
                break;
            case DIRECTION_EAST:
                distance = other->x - vehicle->x;
                ahead = (other->x > vehicle->x) && (distance > 0);
                break;
            case DIRECTION_WEST:
                distance = vehicle->x - other->x;
                ahead = (other->x < vehicle->x) && (distance > 0);
                break;
        }
        
        if (ahead && distance < criticalDistance) {
            return true;
        }
        
        current = current->next;
    }
    
    return false;
}

void updateVehicle(Vehicle *vehicle, TrafficLight *lights) {
    if (!vehicle->active) return;

    float stopLine = 0;
    bool shouldStopLight = false;
    bool reachedCenter = false;

    switch (vehicle->direction) {
        case DIRECTION_NORTH:
            stopLine = INTERSECTION_Y + LANE_WIDTH;
            reachedCenter = (vehicle->y <= INTERSECTION_Y && !vehicle->hasPassedCenter);
            if (lights[DIRECTION_NORTH].state == RED && !vehicle->isTurning) {
                if (vehicle->turnDirection == TURN_STRAIGHT) {
                    shouldStopLight = (vehicle->y > stopLine - 80 && vehicle->y < stopLine + 10);
                } else {
                    shouldStopLight = false;
                }
            }
            break;
        case DIRECTION_SOUTH:
            stopLine = INTERSECTION_Y - LANE_WIDTH;
            reachedCenter = (vehicle->y >= INTERSECTION_Y && !vehicle->hasPassedCenter);
            if (lights[DIRECTION_SOUTH].state == RED && !vehicle->isTurning) {
                if (vehicle->turnDirection == TURN_STRAIGHT) {
                    shouldStopLight = (vehicle->y < stopLine + 80 && vehicle->y > stopLine - 10);
                } else {
                    shouldStopLight = false;
                }
            }
            break;
        case DIRECTION_EAST:
            stopLine = INTERSECTION_X - LANE_WIDTH;
            reachedCenter = (vehicle->x >= INTERSECTION_X && !vehicle->hasPassedCenter);
            if (lights[DIRECTION_EAST].state == RED && !vehicle->isTurning) {
                if (vehicle->turnDirection == TURN_STRAIGHT) {
                    shouldStopLight = (vehicle->x < stopLine + 80 && vehicle->x > stopLine - 10);
                } else {
                    shouldStopLight = false;
                }
            }
            break;
        case DIRECTION_WEST:
            stopLine = INTERSECTION_X + LANE_WIDTH;
            reachedCenter = (vehicle->x <= INTERSECTION_X && !vehicle->hasPassedCenter);
            if (lights[DIRECTION_WEST].state == RED && !vehicle->isTurning) {
                if (vehicle->turnDirection == TURN_STRAIGHT) {
                    shouldStopLight = (vehicle->x > stopLine - 80 && vehicle->x < stopLine + 10);
                } else {
                    shouldStopLight = false;
                }
            }
            break;
    }

    if (vehicle->turnDirection == TURN_LEFT && reachedCenter) {
        vehicle->isTurning = true;
        vehicle->hasPassedCenter = true;
        vehicle->turnProgress = 0.0f;
        const char* lightState = lights[vehicle->direction].state == RED ? "RED" : "GREEN";
        printf(">>> Vehicle TURNING LEFT on %s LIGHT from direction %d\n", lightState, vehicle->direction);
    }

    bool shouldStopVehicle = shouldStopForVehicleInQueue(vehicle, vehicle->direction);
    bool shouldStop = shouldStopLight || shouldStopVehicle;

    if (shouldStop) {
        vehicle->speed = 0;
        vehicle->state = STATE_STOPPED;
    } else if (vehicle->state == STATE_STOPPED) {
        vehicle->state = STATE_MOVING;
        vehicle->speed = 0.5f;
    } else if (vehicle->speed < 2.0f) {
        vehicle->speed += 0.1f;
        if (vehicle->speed > 2.0f) vehicle->speed = 2.0f;
    }

    if (vehicle->speed > 0) {
        if (vehicle->isTurning && vehicle->turnProgress < 1.0f) {
            vehicle->turnProgress += 0.03f;
            float angle = vehicle->turnProgress * 1.57f;
            
            Direction originalDir = vehicle->direction;
            
            switch (originalDir) {
                case DIRECTION_NORTH:
                    vehicle->x -= sin(angle) * 1.5f;
                    vehicle->y -= cos(angle) * 1.5f;
                    if (vehicle->turnProgress >= 1.0f) {
                        vehicle->direction = DIRECTION_WEST;
                        vehicle->rect.w = 30;
                        vehicle->rect.h = 20;
                        vehicle->isTurning = false;
                        printf("<<< Completed turn: NORTH -> WEST\n");
                    }
                    break;
                    
                case DIRECTION_SOUTH:
                    vehicle->x += sin(angle) * 1.5f;
                    vehicle->y += cos(angle) * 1.5f;
                    if (vehicle->turnProgress >= 1.0f) {
                        vehicle->direction = DIRECTION_EAST;
                        vehicle->rect.w = 30;
                        vehicle->rect.h = 20;
                        vehicle->isTurning = false;
                        printf("<<< Completed turn: SOUTH -> EAST\n");
                    }
                    break;
                    
                case DIRECTION_EAST:
                    vehicle->x += cos(angle) * 1.5f;
                    vehicle->y -= sin(angle) * 1.5f;
                    if (vehicle->turnProgress >= 1.0f) {
                        vehicle->direction = DIRECTION_NORTH;
                        vehicle->rect.w = 20;
                        vehicle->rect.h = 30;
                        vehicle->isTurning = false;
                        printf("<<< Completed turn: EAST -> NORTH\n");
                    }
                    break;
                    
                case DIRECTION_WEST:
                    vehicle->x -= cos(angle) * 1.5f;
                    vehicle->y += sin(angle) * 1.5f;
                    if (vehicle->turnProgress >= 1.0f) {
                        vehicle->direction = DIRECTION_SOUTH;
                        vehicle->rect.w = 20;
                        vehicle->rect.h = 30;
                        vehicle->isTurning = false;
                        printf("<<< Completed turn: WEST -> SOUTH\n");
                    }
                    break;
            }
        } else if (!vehicle->isTurning) {
            switch (vehicle->direction) {
                case DIRECTION_NORTH: vehicle->y -= vehicle->speed; break;
                case DIRECTION_SOUTH: vehicle->y += vehicle->speed; break;
                case DIRECTION_EAST: vehicle->x += vehicle->speed; break;
                case DIRECTION_WEST: vehicle->x -= vehicle->speed; break;
            }
        }
    }

    if (vehicle->y < -50 || vehicle->y > WINDOW_HEIGHT + 50 ||
        vehicle->x < -50 || vehicle->x > WINDOW_WIDTH + 50) {
        vehicle->active = false;
    }

    vehicle->rect.x = (int)vehicle->x;
    vehicle->rect.y = (int)vehicle->y;
}

void renderRoads(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);

    SDL_Rect verticalRoad = {INTERSECTION_X - LANE_WIDTH, 0, LANE_WIDTH * 2, WINDOW_HEIGHT};
    SDL_RenderFillRect(renderer, &verticalRoad);

    SDL_Rect horizontalRoad = {0, INTERSECTION_Y - LANE_WIDTH, WINDOW_WIDTH, LANE_WIDTH * 2};
    SDL_RenderFillRect(renderer, &horizontalRoad);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i < WINDOW_HEIGHT; i += 40) {
        SDL_Rect dash = {INTERSECTION_X - 2, i, 4, 20};
        SDL_RenderFillRect(renderer, &dash);
    }

    for (int i = 0; i < WINDOW_WIDTH; i += 40) {
        SDL_Rect dash = {i, INTERSECTION_Y - 2, 20, 4};
        SDL_RenderFillRect(renderer, &dash);
    }
}

void renderSimulation(SDL_Renderer *renderer, TrafficLight *lights, Statistics *stats) {
    // PURPLE background
    SDL_SetRenderDrawColor(renderer, 147, 112, 219, 255);  // Medium Purple
    SDL_RenderClear(renderer);
    
    SDL_Rect fullScreen = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_SetRenderDrawColor(renderer, 147, 112, 219, 255);  // Medium Purple
    SDL_RenderFillRect(renderer, &fullScreen);

    renderRoads(renderer);

    // Draw CIRCULAR traffic lights
    for (int i = 0; i < 4; i++) {
        int centerX, centerY, radius;
        
        // Calculate center position based on direction
        switch (lights[i].direction) {
            case DIRECTION_NORTH:
                centerX = INTERSECTION_X;
                centerY = INTERSECTION_Y - LANE_WIDTH - 15;
                break;
            case DIRECTION_SOUTH:
                centerX = INTERSECTION_X;
                centerY = INTERSECTION_Y + LANE_WIDTH + 15;
                break;
            case DIRECTION_EAST:
                centerX = INTERSECTION_X + LANE_WIDTH + 15;
                centerY = INTERSECTION_Y;
                break;
            case DIRECTION_WEST:
                centerX = INTERSECTION_X - LANE_WIDTH - 15;
                centerY = INTERSECTION_Y;
                break;
        }
        
        radius = 12; // Circle radius
        
        // Draw black circle border
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        for (int w = 0; w < radius * 2; w++) {
            for (int h = 0; h < radius * 2; h++) {
                int dx = radius - w;
                int dy = radius - h;
                if ((dx*dx + dy*dy) <= ((radius + 2) * (radius + 2))) {
                    SDL_RenderDrawPoint(renderer, centerX + dx, centerY + dy);
                }
            }
        }
        
        // Draw colored circle
        if (lights[i].state == RED) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        }
        
        // Fill circle
        for (int w = 0; w < radius * 2; w++) {
            for (int h = 0; h < radius * 2; h++) {
                int dx = radius - w;
                int dy = radius - h;
                if ((dx*dx + dy*dy) <= (radius * radius)) {
                    SDL_RenderDrawPoint(renderer, centerX + dx, centerY + dy);
                }
            }
        }
    }

    // Draw vehicles
    for (int lane = 0; lane < 4; lane++) {
        Node *current = laneQueues[lane].front;
        while (current != NULL) {
            Vehicle *v = &current->vehicle;
            if (v->active) {
                SDL_Color color = VEHICLE_COLORS[v->colorIndex];
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
                SDL_RenderFillRect(renderer, &v->rect);
                
                SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
                SDL_RenderDrawRect(renderer, &v->rect);
            }
            current = current->next;
        }
    }

    SDL_RenderPresent(renderer);
}

// Queue functions
void initQueue(Queue *q) {
    q->front = q->rear = NULL;
    q->size = 0;
}

void enqueue(Queue *q, Vehicle vehicle) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->vehicle = vehicle;
    newNode->next = NULL;
    
    if (q->rear == NULL) {
        q->front = q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
    q->size++;
}

Vehicle dequeue(Queue *q) {
    if (q->front == NULL) {
        Vehicle empty = {0};
        return empty;
    }
    
    Node *temp = q->front;
    Vehicle vehicle = temp->vehicle;
    
    q->front = q->front->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }
    
    free(temp);
    q->size--;
    
    return vehicle;
}

int isQueueEmpty(Queue *q) {
    return q->front == NULL;
}

void removeFromQueue(Queue *q, Vehicle *vehicle) {
    // Not needed
}