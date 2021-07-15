#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR_BUFF_SIZE 4096

typedef struct
{
    int vert_num;
    uint32_t *matrix;
    float scatter_ratio;

} graph_t;

typedef struct {
    uint64_t score;
    uint32_t position;
} pos_tuple_t;

typedef struct {
    uint64_t distance;
    uint64_t vert;
} minheap_node_t;

typedef struct {
    uint32_t maxsize;
    uint32_t size;
    minheap_node_t **data;
    uint32_t *node_pos;
} minheap_t;

typedef struct node {
    uint64_t score;
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

uint32_t my_stoi(const char *str, uint32_t len);
uint64_t compute_score(const graph_t *graph, minheap_t* heap, uint64_t* points);
void min_heapify(minheap_t *heap, uint32_t index);
minheap_node_t *heap_min_node(minheap_t *heap);
void update_key(minheap_t *heap, uint32_t v, uint32_t dist);
int in_heap(minheap_t *heap, int v);
void dijkstra_h(const graph_t *graph, int src, uint64_t *distances, minheap_t* heap);
scores_list_node_t *make_node(uint32_t position, uint64_t score, scores_list_node_t *next);
void list_insert_in_order_capped(scores_list_t *list, uint64_t score, uint32_t position, uint32_t cap);
void destroy_list(scores_list_t *list);

int main() {
    char buffer[STR_BUFF_SIZE];
    int exit_flag = 0;

    uint32_t N, K;

    uint32_t current_num = 0;
    graph_t g_current;

    minheap_t* dheap = (minheap_t*)malloc(sizeof(minheap_t));
    uint64_t* points;

    scores_list_t score_list;
    score_list.head = NULL;

    int first_topk = 1;

    if (scanf("%d", &N) == EOF) {
        exit(0);
    }
    if (scanf("%d", &K) == EOF) {
        exit(0);
    }

    dheap->data = (minheap_node_t**)malloc(N*sizeof(minheap_node_t));
    dheap->node_pos = (uint32_t*)malloc(N*sizeof(uint32_t));

    points = (uint64_t*)malloc(N*sizeof(uint64_t));

    g_current.vert_num = N;
    g_current.matrix = (uint32_t *)malloc(N * N * sizeof(int));

    while (!exit_flag) {
        if (scanf("%s", buffer) == EOF) {
            break;
        }
        if (buffer[0] == 'A') {
            uint32_t index = 0;
            uint32_t read;
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
                        uint32_t num = my_stoi(it_start, it_end - it_start);

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

            uint64_t score = compute_score(&g_current, dheap, points);

            list_insert_in_order_capped(&score_list, score, current_num, K);

            current_num++;

#ifdef DEBUG
            printf("score for graph %d is %lu\n", current_num, score);
#endif

        } else if (buffer[0] == 'T') {
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

    // free(points)
    // free(dheap->node_pos);
    // free(dheap->data);
    // free(dheap);
    // free(g_current.matrix);
    // destroy_list(&score_list);
    printf("\n");
    return 0;
}

inline uint32_t my_stoi(const char *str, uint32_t len) {
    uint32_t val = 0;
    while (len > 0) {
        val = val * 10 + (*str++ - '0');
        len--;
    }
    return val;
}

uint64_t compute_score(const graph_t *graph, minheap_t* heap, uint64_t* points) {
    uint64_t sum = 0;

    dijkstra_h(graph, 0, points, heap);

    for (uint32_t i = 0; i < graph->vert_num; i++) {
        if (points[i] != UINT64_MAX) sum += points[i];
    }

    return sum;
}

void min_heapify(minheap_t *heap, uint32_t index) {
    uint32_t left = 2 * index + 1, right = 2 * index + 2;
    uint32_t posmin;

    if (left < heap->size && heap->data[left]->distance < heap->data[index]->distance)
        posmin = left;
    else
        posmin = index;

    if (right < heap->size && heap->data[right]->distance < heap->data[posmin]->distance)
        posmin = right;

    if (posmin != index) {
        minheap_node_t *min_node = heap->data[posmin];
        minheap_node_t *index_node = heap->data[index];

        heap->node_pos[min_node->vert] = index;
        heap->node_pos[index_node->vert] = posmin;

        swap((void **)&heap->data[posmin], (void **)&heap->data[index]);

        min_heapify(heap, posmin);
    }
}

minheap_node_t *heap_min_node(minheap_t *heap) {
    minheap_node_t *max = heap->data[0];

    minheap_node_t *end = heap->data[heap->size - 1];
    heap->data[0] = end;

    heap->node_pos[max->vert] = heap->size - 1;
    heap->node_pos[end->vert] = 0;

    heap->size--;

    min_heapify(heap, 0);

    return max;
}

void update_key(minheap_t *heap, uint32_t v, uint32_t dist) {
    uint32_t i = heap->node_pos[v];
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

void dijkstra_h(const graph_t *graph, int src, uint64_t *distances, minheap_t* heap) {
    uint32_t verts = graph->vert_num;

    heap->maxsize = verts;
    heap->size = 0;

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

        uint32_t *matr_pointer = graph->matrix + (min->vert * verts);

        for (uint32_t i = 0; i < verts; ++i) {
            uint32_t arc = *(matr_pointer + i);

            if (!arc) continue;

            if (in_heap(heap, i) && distances[min->vert] != UINT64_MAX && arc + distances[min->vert] < distances[i]) {
                uint32_t newdist = distances[min->vert] + arc;
                distances[i] = newdist == UINT32_MAX || newdist == UINT64_MAX ? 0 : newdist;

                update_key(heap, i, distances[i]);
            }
        }

        free(min);
    }
}

scores_list_node_t *make_node(uint32_t position, uint64_t score, scores_list_node_t *next) {
    scores_list_node_t *node = (scores_list_node_t *)malloc(sizeof(scores_list_node_t));

    node->next = next;
    node->position = position;
    node->score = score;

    return node;
}

void list_insert_in_order_capped(scores_list_t *list, uint64_t score, uint32_t position, uint32_t cap) {
    scores_list_node_t *it;

    uint32_t curr = 0;

    if (list->head == NULL) {
        list->head = make_node(position, score, NULL);
        list->length++;
        return;
    }

    if (score < list->head->score) {
        list->head = make_node(position, score, list->head);
        list->length++;
        return;
    }

    it = list->head;

    while (curr < cap) {
        if (it->next == NULL) {
            it->next = make_node(position, score, NULL);
            list->length++;
            break;
        }

        if (it->score == score && it->next->score != score) {
            it->next = make_node(position, score, it->next);
            list->length++;

            break;
        }

        if (it->score < score && it->next->score > score) {
            it->next = make_node(position, score, it->next);
            list->length++;

            break;
        }

        curr++;
        it = it->next;
    }
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