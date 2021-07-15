#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR_BUFF_SIZE 8192
#define SCATTER_RATIO_LIMIT 0.3

void parse_head(char *str, int *N, int *K);

typedef struct
{
    int vert_num;
    u_int32_t *matrix;
    float scatter_ratio;

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

typedef struct node {
    uint32_t score;
    uint32_t position;
    struct node *next;
} scores_list_node_t;

typedef struct {
    scores_list_node_t *head;
    uint32_t length;
} scores_list_t;

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

        swap((void **)&heap->data[min], (void **)&heap->data[index]);

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
        swap((void **)&heap->data[i],
             (void **)&heap->data[(i - 1) / 2]);

        i = (i - 1) / 2;
    }
}

int in_heap(minheap_t *heap, int v) {
    return heap->node_pos[v] < heap->size;
}

void dijkstra_h(const graph_t *graph, int src, u_int64_t *distances) {
    u_int32_t verts = graph->vert_num;

    minheap_t *heap = (minheap_t *)malloc(sizeof(minheap_t));  // FREE

    heap->maxsize = verts;
    heap->data = (minheap_node_t **)malloc(verts * sizeof(minheap_node_t));  // FREE
    heap->size = 0;
    heap->node_pos = (u_int32_t *)malloc(verts * sizeof(__uint32_t));

    for (uint32_t i = 0; i < verts; i++) {
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
            uint32_t arc = *(matr_pointer + i);

            if (!arc) continue;

            if (in_heap(heap, i) && distances[min->vert] != UINT64_MAX && arc + distances[min->vert] < distances[i]) {
                if (distances[min->vert] != UINT64_MAX && arc != UINT32_MAX) {
                    uint32_t newdist = distances[min->vert] + arc;
                    distances[i] = newdist == UINT32_MAX || newdist == UINT64_MAX ? 0 : newdist;
                }

                update_key(heap, i, distances[i]);
            }
        }

        free(min);
    }

    free(heap->node_pos);
    free(heap->data);
    free(heap);
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

void list_insert_in_order_capped(scores_list_t *list, uint32_t score, uint32_t position, uint32_t cap) {
    scores_list_node_t *node;
    scores_list_node_t *it;

    uint32_t curr = 0;

    if (list->head == NULL) {
        node = (scores_list_node_t *)malloc(sizeof(scores_list_node_t));
        node->next = NULL;
        node->position = position;
        node->score = score;
        list->head = node;
        list->length++;
        return;
    }

    if (score < list->head->score) {
        node = (scores_list_node_t *)malloc(sizeof(scores_list_node_t));
        node->next = list->head;
        node->position = position;
        node->score = score;
        list->head = node;
        list->length++;
        return;
    }

    it = list->head;

    while (true) {
        if(curr > cap) break;
        
        if (it->next == NULL) {

            node = (scores_list_node_t *)malloc(sizeof(scores_list_node_t));
            node->next = NULL;
            node->position = position;
            node->score = score;
            it->next = node;
            list->length++;

            break;
        }


        if(it->score == score) {
            
            if(it->next->score != score) {
                node = (scores_list_node_t *)malloc(sizeof(scores_list_node_t));
                node->next = it->next;
                node->position = position;
                node->score = score;

                it->next = node;
                list->length++;

                break;
            }
            
        }

        if (it->score < score && it->next->score > score) {

            node = (scores_list_node_t *)malloc(sizeof(scores_list_node_t));
            node->next = it->next;
            node->position = position;
            node->score = score;

            it->next = node;
            list->length++;

            break;
        }

        curr++;
        it = it->next;
    }
}

void print_list(scores_list_t *list) {
    scores_list_node_t *it = list->head;

    while (it != NULL) {
        printf("%d(%d) ", it->score, it->position);
        it = it->next;
    }

    printf("\n");
}

void destroy_list(scores_list_t *list) {
    if (!list) return;

    scores_list_node_t *it = list->head;

    while (it != NULL) {
        scores_list_node_t *next = it->next;
        free(it);
        it = next;
    }
}

void destory_list_nodes(scores_list_node_t *node) {
    while (node != NULL) {
        scores_list_node_t *next = node->next;
        free(node);
        node = next;
    }
}

int main() {
    char buffer[STR_BUFF_SIZE];
    int exit_flag = 0;

    u_int32_t N, K;

    u_int32_t current_num = 0;
    graph_t g_current;

    pos_tuple_t *scores;
    scores_list_t score_list;

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
            uint32_t zeros = 0;

            for (int i = 0; i < N; i++) {
                if (scanf("%s%n", buffer, &read) == EOF) {
                    exit_flag = 1;
                    break;
                }

                it_start = buffer;
                it_end = buffer;

                while (read > 0) {
                    if (*it_end == ',' || read == 1) {
                        u_int32_t num = my_stoi(it_start, it_end - it_start);
                        if (num == 0) zeros++;

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

            g_current.scatter_ratio = ((float)zeros) / N * N;

            uint64_t score = compute_score(&g_current);

#ifdef DEBUG
            printf("score for graph %d is %lu\n", current_num, score);
#endif

            list_insert_in_order_capped(&score_list, score, current_num, K);

            current_num++;

        } else if (!strcmp(buffer, "TopK")) {
            int first_flag = 1;
            scores_list_node_t *it = score_list.head;

            if (!first_topk) {
                printf("\n");
            }

            for (int i = 0; i < K && it != NULL; i++) {
                if (!first_flag) printf(" ");
                printf("%d", it->position);
                first_flag = 0;
                it = it->next;
            }

            first_topk = 0;
        } else {
            break;
        }
    }

    free(g_current.matrix);
    free(scores);
    destroy_list(&score_list);
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

    if (graph->scatter_ratio < SCATTER_RATIO_LIMIT)
        dijkstra(graph, 0, points);
    else
        dijkstra_h(graph, 0, points);

    for (u_int32_t i = 0; i < graph->vert_num; i++) {
        if (points[i] != UINT64_MAX) sum += points[i];
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