#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR_BUFF_SIZE 8192

void parse_head(char *str, int *N, int *K);

typedef struct
{
    int vert_num;
    u_int32_t *matrix;

} graph_t;

typedef struct {
    u_int64_t score;
    u_int32_t position;
} pos_tuple_t;

typedef struct {
    u_int64_t distance;
    u_int64_t vert;
} minheap_node_t;

typedef struct {
    u_int32_t maxsize;
    u_int32_t size;
    minheap_node_t **data;
    u_int32_t *node_pos;
} minheap_t;

void swap(void **x, void **y) {
    void *tmp = *x;
    *x = *y;
    *y = tmp;
}

void min_heapify(minheap_t *heap, u_int32_t index) {
    u_int32_t min = index;
    u_int32_t left = 2 * index + 1, right = 2 * index + 2;

    if (left < heap->size && heap->data[left]->distance < heap->data[min]->distance)
        min = left;

    if (right < heap->size && heap->data[right]->distance < heap->data[min]->distance)
        min = right;

    if (min != index) {
        minheap_node_t *min_node = heap->data[min];
        minheap_node_t *index_node = heap->data[index];

        heap->node_pos[min_node->vert] = index;
        heap->node_pos[index_node->vert] = min;

        swap((void**)&heap->data[min],(void**) &heap->data[index]);

        min_heapify(heap, min);
    }
}

minheap_node_t *heap_min_node(minheap_t *heap) {
    if (!heap->size) return NULL;

    minheap_node_t *root = heap->data[0];

    minheap_node_t *end = heap->data[heap->size - 1];
    heap->data[0] = end;

    heap->node_pos[root->vert] = heap->size - 1;
    heap->node_pos[end->vert] = 0;

    heap->size--;

    min_heapify(heap, 0);

    return root;
}

void update_key(minheap_t *heap, u_int32_t v, u_int32_t dist) {
    u_int32_t i = heap->node_pos[v];
    heap->data[i]->distance = dist;

    while (i && heap->data[i]->distance < heap->data[(i - 1) / 2]->distance) {
        heap->node_pos[heap->data[i]->vert] =
            (i - 1) / 2;
        heap->node_pos[heap->data[(i - 1) / 2]->vert] = i;
        swap((void**)&heap->data[i],
                        (void**)&heap->data[(i - 1) / 2]);

        i = (i - 1) / 2;
    }
}

int in_heap(minheap_t *heap, int v) {
    return heap->node_pos[v] < heap->size;
}

void dijkstra_h(const graph_t *graph, int src, u_int64_t *distances) {
    u_int32_t verts = graph->vert_num;

    minheap_t *heap = (minheap_t *)malloc(sizeof(minheap_t)); // FREE

    heap->maxsize = verts;
    heap->data = (minheap_node_t **)malloc(verts * sizeof(minheap_node_t)); // FREE
    heap->size = 0;
    heap->node_pos = (u_int32_t *)malloc(verts * sizeof(__uint32_t));

    for (u_int32_t i = 0; i < verts; i++) {
        minheap_node_t *node = (minheap_node_t *)malloc(sizeof(minheap_node_t));

        distances[i] = UINT64_MAX;
        node->distance = UINT64_MAX;
        node->vert = i;
        heap->data[i] = node;

        heap->node_pos[i] = i;
    }

    heap->data[src]->distance = 0;
    heap->data[src]->distance = distances[src];

    heap->node_pos[src] = src;
    distances[src] = 0;

    update_key(heap, src, distances[src]);

    heap->size = verts;

    while (heap->size > 0) {
        minheap_node_t *min = heap_min_node(heap);

        u_int32_t *matr_pointer = graph->matrix + (min->vert * verts);

        for (u_int32_t i = 0; i < verts; ++i) {
            if (!*(matr_pointer + i)) continue;

            if (in_heap(heap, i) && distances[min->vert] != UINT64_MAX && *(matr_pointer + i) + distances[min->vert] < distances[i]) {
                distances[i] = distances[min->vert] + *(matr_pointer + i);
                update_key(heap, i, distances[i]);
            }
        }
    }

    for (u_int32_t i = 0; i < verts; i++) {
        if (distances[i] == UINT64_MAX) distances[i] = 0;
    }
}

u_int32_t my_stoi(const char *, u_int32_t len);
u_int64_t get_distance(u_int64_t dist[], bool sptSet[], int V);
void dijkstra(const graph_t *graph, int src, u_int64_t *);
u_int64_t compute_score(const graph_t *graph);
u_int32_t find_max_i(const pos_tuple_t *arr, u_int32_t len);

int pos_tuple_cmp_funct(const void *a, const void *b) {
    pos_tuple_t *t1 = (pos_tuple_t *)a;
    pos_tuple_t *t2 = (pos_tuple_t *)b;

    return (t1->position - t2->position);
}

int main() {
    char buffer[STR_BUFF_SIZE];
    int exit_flag = 0;

    u_int32_t N, K;

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
    g_current.matrix = (u_int32_t *)malloc(N * N * sizeof(int));

    scores = (pos_tuple_t *)malloc(K * sizeof(pos_tuple_t));
    for (int i = 0; i < K; i++) {
        scores[i].score = -1;
        scores[i].position = -1;
    }

    while (!exit_flag) {
        if (scanf("%s", buffer) == EOF) {
            break;
        };
        if (!strcmp(buffer, "AggiungiGrafo")) {
            u_int32_t index = 0, read;
            char *it_start, *it_end;

            for (int i = 0; i < N; i++) {
                if (scanf("%s%n", buffer, &read) == EOF) {
                    exit_flag = 1;
                    break;
                }

                it_start = buffer;
                it_end = buffer;

                while (read > 0) {
                    if (*it_end == ',' || read == 1) {
                        u_int32_t num = my_stoi(it_start, it_end-it_start);

#ifdef VERBOSE
                        printf("read num %d adding to matrix in pos %d\n", num, index);
#endif

                        g_current.matrix[index] = num;

                        it_start = it_end + 1;
                        index++;
                    }

                    it_end++;
                    read--;
                }
            }

            if (exit_flag) break;

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

#ifdef ORDERED

            qsort(scores, K, sizeof(pos_tuple_t), pos_tuple_cmp_funct);
#endif

            if (!first_topk) {
                printf("\n");
            }

            for (int i = 0; i < K; i++) {
                if (scores[i].score == -1) continue;
                if (!first_flag) printf(" ");
                printf("%d", scores[i].position);
                first_flag = 0;
            }

            first_topk = 0;
        } else {
            break;
        }
    }

    free(g_current.matrix);
    free(scores);
    printf("\n");
    return 0;
}

inline u_int32_t my_stoi(const char *str, u_int32_t len) {
    u_int32_t val = 0;
    while (len > 0) {
        val = val * 10 + (*str++ - '0');
        len--;
    }
    return val;
}

u_int64_t get_distance(u_int64_t dist[], bool sptSet[], int V) {
    uint64_t min = UINT64_MAX, min_index = -1;

    for (int v = 0; v < V; v++)
        if (sptSet[v] == false && dist[v] <= min)
            min = dist[v], min_index = v;

    return min_index;
}

void dijkstra(const graph_t *graph, int src, u_int64_t *dist) {
    int V = graph->vert_num;

    bool sptSet[V];

    for (int i = 0; i < V; i++)
        dist[i] = UINT64_MAX, sptSet[i] = false;

    dist[src] = 0;

    for (int count = 0; count < V - 1; count++) {
        u_int64_t u = get_distance(dist, sptSet, V);

        sptSet[u] = true;

        for (int v = 0; v < V; v++) {
            if (!sptSet[v] && graph->matrix[u * V + v] && dist[u] != UINT64_MAX && dist[u] + graph->matrix[u * V + v] < dist[v])
                dist[v] = dist[u] + graph->matrix[u * V + v];
        }
    }
}

u_int64_t compute_score(const graph_t *graph) {
    u_int64_t *points = (u_int64_t *)malloc(graph->vert_num * sizeof(u_int64_t));
    u_int64_t sum = 0;

    dijkstra(graph, 0, points);

    for (u_int32_t i = 0; i < graph->vert_num; i++) {
        if(points[i] != UINT64_MAX) sum += points[i];
    }

    free(points);

    return sum;
}

u_int32_t find_max_i(const pos_tuple_t *arr, u_int32_t len) {
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