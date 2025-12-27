#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define SLEEP(ms) Sleep(ms)
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #define SLEEP(ms) usleep((ms) * 1000)
#endif

#include "traffic_simulation.h"

void writeVehicleToFile(FILE *file, Vehicle *vehicle) {
    fprintf(file, "%f %f %d %d %d %d %d %d\n",
            vehicle->x, vehicle->y,
            vehicle->direction,
            vehicle->type,
            vehicle->turnDirection,
            vehicle->state,
            vehicle->speed,
            vehicle->colorIndex);
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    
    // Create bin directory if it doesn't exist
    #ifdef _WIN32
        _mkdir("bin");
    #else
        mkdir("bin", 0777);
    #endif

    FILE *file = fopen("bin/vehicles.txt", "w");
    if (!file) {
        perror("Failed to open vehicles.txt for writing");
        printf("Make sure the 'bin' directory exists in the current directory.\n");
        return 1;
    }

    printf("========================================\n");
    printf("  Vehicle Generator Started - NEON THEME\n");
    printf("========================================\n");
    printf("Writing vehicles to: bin/vehicles.txt\n");
    printf("Colors: Neon (Hot Pink, Cyan, Yellow, etc.)\n");
    printf("Press Ctrl+C to stop\n\n");

    int vehicleCounter = 0;
    const int MAX_CONCURRENT_VEHICLES = 12;
    const int MIN_SPACING = 200;

    Vehicle activeVehicles[MAX_CONCURRENT_VEHICLES] = {0};
    int activeCount = 0;

    while (1) {
        // Generate new vehicle
        Direction spawnDirection = (Direction)(rand() % 4);
        Vehicle *newVehicle = createVehicle(spawnDirection);
        
        if (newVehicle) {
            // Check if there's enough space for this new vehicle
            bool canSpawn = true;
            for (int i = 0; i < activeCount; i++) {
                if (activeVehicles[i].direction == newVehicle->direction) {
                    float distance = 0;
                    switch (newVehicle->direction) {
                        case DIRECTION_NORTH:
                        case DIRECTION_SOUTH:
                            distance = fabs(newVehicle->y - activeVehicles[i].y);
                            break;
                        case DIRECTION_EAST:
                        case DIRECTION_WEST:
                            distance = fabs(newVehicle->x - activeVehicles[i].x);
                            break;
                    }
                    
                    if (distance < MIN_SPACING) {
                        canSpawn = false;
                        break;
                    }
                }
            }
            
            if (canSpawn) {
                vehicleCounter++;
                const char* dirNames[] = {"NORTH", "SOUTH", "EAST", "WEST"};
                const char* turnNames[] = {"STRAIGHT", "LEFT", "RIGHT"};
                const char* colorNames[] = {"Hot Pink", "Cyan", "Yellow", "Lime", 
                                           "Magenta", "Orange", "Purple", "Sky Blue"};
                
                printf("🚗 Vehicle #%d Generated:\n", vehicleCounter);
                printf("   Direction: %s\n", dirNames[newVehicle->direction]);
                printf("   Turn: %s\n", turnNames[newVehicle->turnDirection]);
                printf("   Color: %s\n", colorNames[newVehicle->colorIndex]);
                printf("   Position: (%.1f, %.1f)\n", newVehicle->x, newVehicle->y);
                printf("   Queue Size: %d\n\n", activeCount + 1);

                // Add to active vehicles list
                if (activeCount < MAX_CONCURRENT_VEHICLES) {
                    activeVehicles[activeCount] = *newVehicle;
                    activeCount++;
                } else {
                    // Replace oldest vehicle
                    for (int i = 0; i < activeCount - 1; i++) {
                        activeVehicles[i] = activeVehicles[i + 1];
                    }
                    activeVehicles[activeCount - 1] = *newVehicle;
                }

                // Rewrite entire file with all active vehicles
                fseek(file, 0, SEEK_SET);
                for (int i = 0; i < activeCount; i++) {
                    writeVehicleToFile(file, &activeVehicles[i]);
                }
                fflush(file);
                
                printf("✅ Written to file | Total Active: %d/%d\n", activeCount, MAX_CONCURRENT_VEHICLES);
                printf("----------------------------------------\n\n");
            } else {
                printf("⏸️  Skipping spawn - too close to existing vehicle\n\n");
            }

            free(newVehicle);
        }

        // Wait before generating next vehicle
        SLEEP(1500); // 1.5 seconds
    }

    fclose(file);
    return 0;
}