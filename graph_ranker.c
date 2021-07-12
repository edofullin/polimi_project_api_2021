#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parse_head(char *str, int *N, int *K);

typedef struct
{
    int col;
    int *matrix;

} graph_t;



int minDistance(int dist[], bool sptSet[], int V) {
    // Initialize min value
    int min = INT_MAX, min_index = -1;

    for (int v = 0; v < V; v++)
        if (sptSet[v] == false && dist[v] <= min)
            min = dist[v], min_index = v;

    return min_index;
}

int *dijkstra(graph_t *graph, int src) {
    int V = graph->col;
    int *dist = malloc(V * sizeof(int));  // The output array.  dist[i] will hold the shortest
    // distance from src to i

    bool sptSet[V];  // sptSet[i] will be true if vertex i is included in shortest
    // path tree or shortest distance from src to i is finalized

    // Initialize all distances as INFINITE and stpSet[] as false
    for (int i = 0; i < V; i++)
        dist[i] = INT_MAX, sptSet[i] = false;

    // Distance of source vertex from itself is always 0
    dist[src] = 0;

    // Find shortest path for all vertices
    for (int count = 0; count < V - 1; count++) {
        // Pick the minimum distance vertex from the set of vertices not
        // yet processed. u is always equal to src in the first iteration.
        int u = minDistance(dist, sptSet, V);

        // Mark the picked vertex as processed
        sptSet[u] = true;

        // Update dist value of the adjacent vertices of the picked vertex.
        for (int v = 0; v < V; v++) {
            // Update dist[v] only if is not in sptSet, there is an edge from
            // u to v, and total weight of path from src to  v through u is
            // smaller than current value of dist[v]
            if (!sptSet[v] && graph->matrix[u * V + v] && dist[u] != INT_MAX && dist[u] + graph->matrix[u * V + v] < dist[v])
                dist[v] = dist[u] + graph->matrix[u * V + v];
        }
    }

    return dist;
}

int find_min(u_int64_t *arr, int len) {
    int min_i = 0;
    u_int64_t min_val = __UINT64_MAX__;

    for (int i = 0; i < len; i++) {
        if (arr[i] < min_val) {
            min_i = i;
            min_val = arr[i];
        }
    }

    return min_i;
}

int main() {
    int N, K;
    char buffer[4000];
    graph_t graphs[100];
    int next = 0;
    u_int64_t points[100];

    if (scanf("%d", &N) == EOF) {
        exit(-1);
    };
    if (scanf("%d", &K) == EOF) {
        exit(-1);
    };

    while (1) {
        if (scanf("%s", buffer) == EOF) {
            exit(-1);
        };
        if (!strcmp(buffer, "AggiungiGrafo")) {
            // aggiungi grafo
            // printf("Aggiungo Grafo #%d", next);

            graphs[next].col = N;
            graphs[next].matrix = malloc(N * N * sizeof(int));
            int index = 0;
            char *tok;

            for (int i = 0; i < N; i++) {
                if (scanf("%s", buffer) == EOF) {
                    exit(-1);
                };
                tok = strtok(buffer, ",");
                *(graphs[next].matrix + index++) = atoi(tok);

                for (int j = 0; j < N - 1; ++j) {
                    tok = strtok(NULL, ",");
                    *(graphs[next].matrix + index++) = atoi(tok);
                }
            }

            next++;
        } else if (!strcmp(buffer, "TopK")) {
            for (int g = 0; g < next; ++g) {
                points[g] = 0;

                int *dist = dijkstra(&(graphs[g]), 0);

                for (int i = 0; i < graphs->col; ++i) {
                    if (dist[i] != INT_MAX)
                        points[g] += dist[i];
                }

                free(dist);
            }

            for (int i = 0; i < K; i++) {
                int val = find_min(points, next);
                points[val] = INT_MAX;

                printf("%d", val);
                if (i != K - 1) printf(" ");
            }

            exit(0);
        } else {
            printf("I'm done, good night.");
            exit(0);
        }
    }
}