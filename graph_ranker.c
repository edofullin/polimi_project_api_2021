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

} graph_t;

typedef struct {
    uint64_t score;
    uint32_t position;
} pos_tuple_t;

typedef struct node {
    uint64_t score;
    uint32_t position;
    struct node *next;
    struct node *prev;
} scores_list_node_t;

typedef struct {
    scores_list_node_t *head;
    scores_list_node_t *tail;
    uint32_t length;
} scores_list_t;

void swap(void **x, void **y) {
    void *tmp = *x;
    *x = *y;
    *y = tmp;
}

uint32_t my_stoi(const char *str, uint32_t len);
uint64_t compute_score(graph_t *graph, uint64_t *points);
scores_list_node_t *make_node(uint32_t position, uint64_t score, scores_list_node_t *next, scores_list_node_t *prev);
void list_insert_in_order_capped(scores_list_t *list, uint64_t score, uint32_t position, uint32_t cap);
void destroy_list(scores_list_t *list);
void destroy_list_from(scores_list_t *list, scores_list_node_t *from);
uint64_t matr_distance(uint32_t num, u_int64_t *dist, bool *visited);
void matr_dijkstra(graph_t *graph, uint32_t start, uint64_t *dist);

#ifdef DEBUG

void print_list(scores_list_t *list) {
    scores_list_node_t *it = list->head;
    while (it) {
        printf("%d(%ld) ", it->position, it->score);
        it = it->next;
    }

    printf("\n");
}

#endif

int main() {
    char buffer[STR_BUFF_SIZE];
    int exit_flag = 0;

    uint32_t N, K;

    uint32_t current_num = 0;
    graph_t g_current;

    uint64_t *points;

    scores_list_t score_list = {.head = NULL, .tail = NULL, .length = 0};

    int first_topk = 1;

    if (scanf("%d", &N) == EOF) {
        exit(0);
    }
    if (scanf("%d", &K) == EOF) {
        exit(0);
    }

    points = (uint64_t *)malloc(N * sizeof(uint64_t));

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

                        g_current.matrix[index] = num;

                        it_start = it_end + 1;
                        index++;
                    }

                    it_end++;
                    read--;
                }
            }

            if (exit_flag) break;

            uint64_t score = compute_score(&g_current, points);

            list_insert_in_order_capped(&score_list, score, current_num, K);

#ifdef DEBUG
            printf("score for graph %d is %lu\n", current_num, score);
            printf("list tail is pointing %d(%ld) length is %d\n", score_list.tail->position, score_list.tail->score, score_list.length);
            print_list(&score_list);
#endif

            current_num++;

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

    // usano tempo, in ogni caso la mamoria viene liberata in uscita
    // free(points);
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

uint64_t compute_score(graph_t *graph, uint64_t *points) {
    uint64_t sum = 0;

    matr_dijkstra(graph, 0, points);  // uso dijkstra con le matrici, i grafi non sono sparsi in genere.

    for (uint32_t i = 0; i < graph->vert_num; i++) {
        if (points[i] != UINT64_MAX) sum += points[i];
    }

    return sum;
}

scores_list_node_t *make_node(uint32_t position, uint64_t score, scores_list_node_t *next, scores_list_node_t *prev) {
    scores_list_node_t *node = (scores_list_node_t *)malloc(sizeof(scores_list_node_t));

    node->next = next;
    node->position = position;
    node->score = score;
    node->prev = prev;

    return node;
}

void list_insert_in_order_capped(scores_list_t *list, uint64_t score, uint32_t position, uint32_t cap) {
    scores_list_node_t *it;
    scores_list_node_t *node;

    uint32_t curr = 0;

    if (list->length >= cap && score > list->tail->score) return;

    if (list->head == NULL) {
        node = make_node(position, score, NULL, NULL);
        list->head = node;
        list->tail = node;
        list->length++;
        return;
    }

    if (score < list->head->score) {
        node = make_node(position, score, list->head, NULL);
        list->head->prev = node;
        list->head = node;
        list->length++;

        if (list->length > cap) destroy_list_from(list, list->tail);

        return;
    }

    it = list->head;

    while (curr < cap && it != NULL) {
        if (it->next == NULL) {
            node = make_node(position, score, NULL, it);
            it->next = node;
            list->tail = node;
            list->length++;

            if (list->length > cap) destroy_list_from(list, list->tail);

            break;
        }

        if (it->score == score && it->next->score != score) {
            node = make_node(position, score, it->next, it);
            it->next->prev = node;
            it->next = node;
            list->length++;

            if (list->length > cap) destroy_list_from(list, list->tail);

            break;
        }

        if (it->score < score && it->next->score > score) {
            node = make_node(position, score, it->next, it);
            it->next->prev = node;
            it->next = node;
            list->length++;

            if (list->length > cap) destroy_list_from(list, list->tail);

            break;
        }

        curr++;
        it = it->next;
    }
}

void destroy_list(scores_list_t *list) {
    if (!list) return;

    destroy_list_from(list, list->head);
}

void destroy_list_from(scores_list_t *list, scores_list_node_t *from) {
    if (!list) return;

    if (list->head != from) {
        list->tail = from->prev;
        list->tail->next = NULL;
    }

    scores_list_node_t *it = from;

    while (it != NULL) {
        scores_list_node_t *next = it->next;
        free(it);
        list->length--;
        it = next;
    }
}

uint64_t matr_distance(uint32_t num, u_int64_t *dist, bool *visited) {
    uint64_t min = UINT64_MAX, min_index = -1;

    for (int i = 0; i < num; i++)
        if (!visited[i] && dist[i] <= min)
            min = dist[i], min_index = i;

    return min_index;
}

void matr_dijkstra(graph_t *graph, uint32_t start, uint64_t *dist) {
    uint32_t N = graph->vert_num;

    bool visited[N];

    for (int i = 0; i < N; i++)
        dist[i] = UINT64_MAX, visited[i] = false;

    dist[start] = 0;

    for (int count = 0; count < N - 1; count++) {
        u_int64_t u = matr_distance(N, dist, visited);

        visited[u] = true;

        for (int v = 0; v < N; v++) {
            // non vistato con distanza non nulla non già massima
            // e non maggiore di quella già eventualmente calcolata per altri nodi successivi
            if (!visited[v] && graph->matrix[u * N + v] && dist[u] != UINT64_MAX && dist[u] + graph->matrix[u * N + v] < dist[v])
                dist[v] = dist[u] + graph->matrix[u * N + v];
        }
    }
}