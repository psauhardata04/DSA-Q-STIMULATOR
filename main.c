#include <stdio.h>
#include "queue.h"
#include "traffic_light.h"
#include "lanes.h"

int main() {
    LightState light = RED;
    LaneSystem lanes;

    initLanes(&lanes);

    printf("Traffic Light Simulator Started\n");

    // Demo: add some vehicles
    enqueue(&lanes.A, 101);
    enqueue(&lanes.B, 202);

    printf("Front of lane A: %d\n", peek(&lanes.A));
    printf("Front of lane B: %d\n", peek(&lanes.B));

    // Cycle lights
    for (int i = 0; i < 5; i++) {
        printf("Current Light: %d\n", light);
        changeLight(&light);
    }

    return 0;
}
