typedef struct {
    int* data;
    int size;
    int capacity;
} Vector;

void vector_init(Vector* v) {
    v->size = 0;
    v->capacity = 4;
    v->data = (int*)malloc(v->capacity * sizeof(int));
}

void vector_push(Vector* v, int val) {
    if (v->size == v->capacity) {
        v->capacity *= 2;
        v->data = (int*)realloc(v->data, v->capacity * sizeof(int));
    }
    v->data[v->size++] = val;
}
typedef struct {
    int stop;
    int bus_count;
} QueueNode;
int get_max_stop(int** routes, int routesSize, int* routesColSize) {
    int max_stop = 0;
    for (int i = 0; i < routesSize; i++) {
        for (int j = 0; j < routesColSize[i]; j++) {
            if (routes[i][j] > max_stop) {
                max_stop = routes[i][j];
            }
        }
    }
    return max_stop;
}
int numBusesToDestination(int** routes, int routesSize, int* routesColSize, int source, int target) {
    if (source == target) return 0;
    int max_stop = get_max_stop(routes, routesSize, routesColSize);
    if (source > max_stop || target > max_stop) return -1;
    Vector* stop_to_buses = (Vector*)malloc((max_stop + 1) * sizeof(Vector));
    for (int i = 0; i <= max_stop; i++) {
        vector_init(&stop_to_buses[i]);
    }
    for (int i = 0; i < routesSize; i++) {
        for (int j = 0; j < routesColSize[i]; j++) {
            vector_push(&stop_to_buses[routes[i][j]], i);
        }
    }
    QueueNode* queue = (QueueNode*)malloc((max_stop + routesSize + 1) * sizeof(QueueNode));
    int head = 0, tail = 0;
    bool* visited_buses = (bool*)calloc(routesSize, sizeof(bool));
    bool* visited_stops = (bool*)calloc(max_stop + 1, sizeof(bool));
    queue[tail++] = (QueueNode){source, 0};
    visited_stops[source] = true;
    while (head < tail) {
        QueueNode curr = queue[head++];
        int curr_stop = curr.stop;
        int curr_count = curr.bus_count;
        if (curr_stop == target) {
            for (int i = 0; i <= max_stop; i++) free(stop_to_buses[i].data);
            free(stop_to_buses); free(queue); free(visited_buses); free(visited_stops);
            return curr_count;
        }
        Vector buses = stop_to_buses[curr_stop];
        for (int i = 0; i < buses.size; i++) {
            int bus = buses.data[i];
            if (visited_buses[bus]) continue;
            visited_buses[bus] = true; 
            for (int j = 0; j < routesColSize[bus]; j++) {
                int next_stop = routes[bus][j];
                if (!visited_stops[next_stop]) {
                    visited_stops[next_stop] = true;
                    queue[tail++] = (QueueNode){next_stop, curr_count + 1};
                }
            }
        }
    }

    for (int i = 0; i <= max_stop; i++) free(stop_to_buses[i].data);
    free(stop_to_buses); free(queue); free(visited_buses); free(visited_stops);

    return -1;
}
