#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR_BUFF_SIZE 4096

typedef struct
{
    uint32_t vert_num;
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

uint64_t compute_score(graph_t *graph, uint64_t *points);
scores_list_node_t *make_node(uint32_t position, uint64_t score, scores_list_node_t *next, scores_list_node_t *prev);
void list_insert_in_order_capped(scores_list_t *list, uint64_t score, uint32_t position, uint32_t cap);
void destroy_list(scores_list_t *list);
void destroy_list_from(scores_list_t *list, scores_list_node_t *from);
uint64_t djk_minimum_dist_node(uint32_t num, u_int64_t *dist, bool *seen);
void djk_score_from(graph_t *graph, uint32_t start, uint64_t *dist);
uint32_t fgets_unlocked(char* buffer, uint32_t size);

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
    char* token;

    uint32_t N, K;

    uint32_t current_num = 0;
    graph_t g_current;

    uint64_t *points;

    scores_list_t score_list = {.head = NULL, .tail = NULL, .length = 0};

    bool first_topk = true;
    bool exit_flag = 0;

    fgets_unlocked(buffer, STR_BUFF_SIZE);

    token = strtok(buffer, " ");
    N = atoi(token);

    token = strtok(NULL, " ");
    K = atoi(token);

    points = (uint64_t *)malloc(N * sizeof(uint64_t));

    g_current.vert_num = N;
    g_current.matrix = (uint32_t *)malloc(N * N * sizeof(uint32_t));

    while (!exit_flag) {
        if (!fgets_unlocked(buffer, STR_BUFF_SIZE)) {
            break;
        }

        if (buffer[0] == 'A') {
            uint32_t index = 0;
            uint32_t read;
            uint32_t num = 0;
            char *it;

            for (uint32_t i = 0; i < N; i++) {

                read = fgets_unlocked(buffer, STR_BUFF_SIZE) + 1;

                if (read == 0) {
                    exit_flag = true;
                    break;
                }

#ifdef DEBUG
                printf("%s\n", buffer);
#endif

                it = buffer;

                while (read > 0) {

                    if (*it == ',' || read == 1) {
                        g_current.matrix[index] = num;
                        num = 0;

                        index++;
                    } else {
                        num = num * 10 + (*it - '0');
                    }

                    it++;
                    read--;
                }
            }

            if (exit_flag) goto exit;

            uint64_t score = compute_score(&g_current, points);

            list_insert_in_order_capped(&score_list, score, current_num, K);

#ifdef DEBUG
            printf("score for graph %d is %lu\n", current_num, score);
            printf("list tail is pointing %d(%ld) length is %d\n", score_list.tail->position, score_list.tail->score, score_list.length);
            //print_list(&score_list);
#endif

            current_num++;

        } else if (buffer[0] == 'T') {
            scores_list_node_t *it = score_list.head;

            if (!first_topk) {
                printf("\n");
            }

            for (uint32_t i = 0; it != NULL; i++) {
                if (i) printf(" ");
                printf("%d", it->position);
                it = it->next;
            }

            first_topk = false;
        } else {
            break;
        }
    }

exit:
    // usano tempo, in ogni caso la mamoria viene liberata in uscita
    // free(points);
    // free(g_current.matrix);
    // destroy_list(&score_list);
    printf("\n");
    return 0;
}

uint32_t fgets_unlocked(char* buffer, uint32_t size) {
    uint32_t i = 0;

    for (i = 0; i < size; ++i) {
        char read = (char)getchar_unlocked();
        buffer[i] = read;

        if(!read || read == '\n') {
            buffer[i] = 0;
            break;
        }
    }

    return i;
}

uint64_t compute_score(graph_t *graph, uint64_t *points) {
    uint64_t sum = 0;

    djk_score_from(graph, 0, points);  // uso dijkstra con le matrici, i grafi non sono sparsi in genere.

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

uint64_t djk_minimum_dist_node(uint32_t num, u_int64_t *dist, bool *seen) {
    uint64_t mpos = -1, old = UINT64_MAX;

    for (uint32_t i = 0; i < num; i++)
        if (dist[i] <= old && !seen[i]) {
            mpos = i;
            old = dist[i];
        }

    return mpos;
}

void djk_score_from(graph_t *graph, uint32_t start, uint64_t *dist) {
    uint32_t N = graph->vert_num;

    bool seen[N];

    // non ho visitato nulla quindi ho distanza massima da tutto...
    for (uint32_t i = 0; i < N; i++)
        seen[i] = false, dist[i] = UINT64_MAX;

    // tranne che da me stesso
    dist[start] = 0;

    for (uint32_t i = 0; i < N - 1; i++) {
        uint64_t min = djk_minimum_dist_node(N, dist, seen); // ottengo nodo più vicino

        seen[min] = true; // l'ho visitato

        // aggiorno tutti i nodi con la nuova informazione
        for (uint32_t j = 0; j < N; j++) {
            if (!seen[j] && graph->matrix[min * N + j] && dist[min] != UINT64_MAX && dist[min] + graph->matrix[min * N + j] < dist[j])
                dist[j] = dist[min] + graph->matrix[min * N + j];
        }
    }
}