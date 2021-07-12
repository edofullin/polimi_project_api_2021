#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR_BUFF_SIZE 4096

void parse_head(char *str, int *N, int *K);

typedef struct
{
    int vert_num;
    int *matrix;

} graph_t;

typedef struct {
    u_int64_t score;
    u_int32_t position;
} pos_tuple_t;

int minDistance(int dist[], bool sptSet[], int V) {
    // Initialize min value
    int min = INT_MAX, min_index = -1;

    for (int v = 0; v < V; v++)
        if (sptSet[v] == false && dist[v] <= min)
            min = dist[v], min_index = v;

    return min_index;
}

int *dijkstra(const graph_t *graph, int src) {
    int V = graph->vert_num;
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

u_int64_t compute_score(const graph_t *graph) {
    int *points = dijkstra(graph, 0);
    u_int64_t sum = 0;

    for (u_int32_t i = 0; i < graph->vert_num; i++) {
        if (points[i] != INT_MAX)
            sum += points[i];
    }

    return sum;
}

u_int64_t find_max_i(const pos_tuple_t *arr, u_int32_t len) {
    u_int64_t max = 0;
    u_int32_t max_i = 0;

    for (u_int32_t i = 0; i < len; i++) {
        if (arr[i].score > max) {
            max = arr[i].score;
            max_i = i;
        }
    }

    return max_i;
}

int main() {
    u_int32_t N, K;
    char buffer[STR_BUFF_SIZE];
    u_int32_t current_num = 0;
    graph_t g_current;
    pos_tuple_t *scores;

    int first_topk = 1;

    if (scanf("%d", &N) == EOF) {
        exit(0);
    };
    if (scanf("%d", &K) == EOF) {
        exit(0);
    };

    g_current.vert_num = N;
    g_current.matrix = (int *)malloc(N * N * sizeof(int));

    scores = (pos_tuple_t *)malloc(K * sizeof(pos_tuple_t));
    for (int i = 0; i < K; i++) {
        scores[i].score = -1;
        scores[i].position = -1;
    }

    while (1) {
        if (scanf("%s", buffer) == EOF) {
            exit(0);
        };
        if (!strcmp(buffer, "AggiungiGrafo")) {
            // aggiungi grafo
            // printf("Aggiungo Grafo #%d", next);
            int index = 0;
            char *tok;

            for (int i = 0; i < N; i++) {
                if (scanf("%s", buffer) == EOF) {
                    exit(0);
                };
                tok = strtok(buffer, ",");
                *(g_current.matrix + index++) = atoi(tok);

                for (int j = 0; j < N - 1; ++j) {
                    tok = strtok(NULL, ",");
                    *(g_current.matrix + index++) = atoi(tok);
                }
            }

            u_int64_t score = compute_score(&g_current);

#ifdef DEBUG
            printf("score for graph %d is %lu\n", current_num, score);
#endif
            int found_flag = 0;

            for (int i = 0; i < K; i++) {
                if (scores[i].score == -1) {
#ifdef DEBUG
                    printf("graph %d put in position %d\n", current_num, i);
#endif
                    scores[i].score = score;
                    scores[i].position = current_num;
                    found_flag = 1;
                    break;
                }
            }

            if (!found_flag)
                for (int i = 0; i < K; i++) {
                    u_int32_t maxi = find_max_i(scores, K);

                    if (score > scores[maxi].score) break;
#ifdef DEBUG
                    printf("graph %d put in position %d replacing %d (%lu)\n", current_num, maxi, scores[maxi].position, scores[maxi].score);
#endif
                    scores[maxi].score = score;
                    scores[maxi].position = current_num;
                    break;
                }

            current_num++;

        } else if (!strcmp(buffer, "TopK")) {
            int first_flag = 1;

            if (!first_topk) {
                printf("\n");
            }

            for (int i = 0; i < K; i++) {
                if (scores[i].score == -1) continue;
                if(!first_flag) printf(" ");
                printf("%d", scores[i].position);
                first_flag = 0;
            }

            first_topk = 0;
        } else {
            exit(0);
        }
    }
}