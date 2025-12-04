#include <stdio.h>
#include "queue.h"
#include "traffic_light.h"

int main() {
    LightState light = RED;

    printf("Traffic Light Simulator Started\n");

    for (int i = 0; i < 5; i++) {
        printf("Current Light: %d\n", light);
        changeLight(&light);
    }

    return 0;
}
