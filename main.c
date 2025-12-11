#include <stdio.h>
#include "queue.h"
#include "traffic_light.h"
#include "lanes.h"

int main() {
    LaneSystem lanes;
    initLanes(&lanes);

    // simulate vehicles in A2
    for (int i = 0; i < 12; i++)
        enqueue(&lanes.A2, i);

    updatePriority(&lanes);

    printf("Priority status: %d\n", lanes.priorityActive);

    return 0;
}
