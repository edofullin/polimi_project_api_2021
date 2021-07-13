#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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
} heap_node_t;

typedef struct {
    u_int32_t maxsize;
    u_int32_t size;
    heap_node_t** heap;
    u_int32_t* posix;
} min_heap_t;

min_heap_t* allocate_minheap(u_int32_t _maxsize) {
    min_heap_t* heap = (min_heap_t*)malloc(sizeof(min_heap_t));

    heap->maxsize = _maxsize;
    heap->heap = (heap_node_t**)malloc(_maxsize*sizeof(heap_node_t));
    heap->size = 0;
    heap->posix = (u_int32_t*)malloc(_maxsize * sizeof(__uint32_t));

    return heap;
}

void swap_heap_nodes(heap_node_t** x, heap_node_t** y) {
    heap_node_t* tmp = *x;
    *x = *y;
    *y = tmp;
}

void min_heapify(min_heap_t* heap, u_int32_t index) {
    u_int32_t min = index;
    u_int32_t left = 2*index + 1, right = 2*index + 2;

    if (left < heap->size && heap->heap[left]->distance < heap->heap[min]->distance)
        min = left;
    
    if (right < heap->size && heap->heap[right]->distance < heap->heap[min]->distance)
        min = right;

    if(min != index) {
        heap_node_t* min_node = heap->heap[min];
        heap_node_t* index_node = heap->heap[index];
        
        heap->posix[min_node->vert] = index;
        heap->posix[index_node->vert] = min;

        swap_heap_nodes(&heap->heap[min], &heap->heap[index]);

        min_heapify(heap, min);
    }

}

heap_node_t* heap_min_node(min_heap_t* heap) {
    if(!heap->size) return NULL;

    heap_node_t* root = heap->heap[0];

    heap_node_t* end = heap->heap[heap->size - 1];
    heap->heap[0] = end;

    heap->posix[root->vert] = heap->size-1;
    heap->posix[end->vert] = 0;

    heap->size--;

    min_heapify(heap, 0);

    return root;
}

void update_key(min_heap_t* heap, u_int32_t v, u_int32_t dist) {

    u_int32_t i = heap->posix[v];
    heap->heap[i]->distance = dist;

    while (i && heap->heap[i]->distance < heap->heap[(i-1)/2]->distance) {
        heap->posix[heap->heap[i]->vert] =
									(i-1)/2;
		heap->posix[heap->heap[
							(i-1)/2]->vert] = i;
		swap_heap_nodes(&heap->heap[i],
				&heap->heap[(i - 1) / 2]);

		// move to parent index
		i = (i - 1) / 2;
    }
    
}

int in_heap(min_heap_t* heap, int v) {
    return heap->posix[v] < heap->size;
}

void dijkstra_h(const graph_t* graph, int src, u_int64_t* distances) {

    u_int32_t V = graph->vert_num;

    min_heap_t* minheap = allocate_minheap(V);

    for (u_int32_t i = 0; i < V; i++)
    {
        heap_node_t* node = (heap_node_t*)malloc(sizeof(heap_node_t));

        distances[i] = UINT64_MAX;
        node->distance = UINT64_MAX;
        node->vert = i;
        minheap->heap[i] = node;

        minheap->posix[i] = i;
    }

    minheap->heap[src]->distance = 0;
    minheap->heap[src]->distance = distances[src];

    minheap->posix[src] = src;
    distances[src] = 0;

    update_key(minheap, src, distances[src]);

    minheap->size = V; 

    while(minheap->size > 0) {
        heap_node_t* min = heap_min_node(minheap);

        u_int32_t* matr_pointer = graph->matrix + (min->vert * V);

        for(u_int32_t i = 0; i < V; ++i) {
            if(!*(matr_pointer + i)) continue;

            if(in_heap(minheap, i) && distances[min->vert] != UINT64_MAX && *(matr_pointer+i) + distances[min->vert] < distances[i]) {
                distances[i] = distances[min->vert] + *(matr_pointer+i);
                update_key(minheap, i, distances[i]);
            }
        }

    }

    for (u_int32_t i = 0; i < V; i++)
    {

        free(minheap->heap[i]);
        free(minheap->heap[i]);

        if(distances[i] == UINT64_MAX) distances[i] = 0;
    }
    
    
}

u_int32_t my_stoi(const char *, u_int32_t len);
u_int32_t get_distance(u_int32_t dist[], bool sptSet[], int V);
void dijkstra(const graph_t *graph, int src, u_int32_t *);
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
                        u_int32_t num = 0;
                        u_int32_t len = it_end - it_start;

                        while (len > 0) {
                            num = num * 10 + (*it_start++ - '0');
                            len--;
                        }

#ifdef DEBUG
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

u_int32_t my_stoi(const char *str, u_int32_t len) {
    u_int32_t val = 0;
    while (len > 0) {
        val = val * 10 + (*str++ - '0');
        len--;
    }
    return val;
}

u_int32_t get_distance(u_int32_t dist[], bool sptSet[], int V) {
    int min = INT_MAX, min_index = -1;

    for (int v = 0; v < V; v++)
        if (sptSet[v] == false && dist[v] <= min)
            min = dist[v], min_index = v;

    return min_index;
}

void dijkstra(const graph_t *graph, int src, u_int32_t *dist) {
    int V = graph->vert_num;

    bool sptSet[V];

    for (int i = 0; i < V; i++)
        dist[i] = INT_MAX, sptSet[i] = false;

    dist[src] = 0;

    for (int count = 0; count < V - 1; count++) {
        u_int64_t u = get_distance(dist, sptSet, V);

        sptSet[u] = true;

        for (int v = 0; v < V; v++) {
            if (!sptSet[v] && graph->matrix[u * V + v] && dist[u] != INT_MAX && dist[u] + graph->matrix[u * V + v] < dist[v])
                dist[v] = dist[u] + graph->matrix[u * V + v];
        }
    }
}

u_int64_t compute_score(const graph_t *graph) {
    u_int64_t *points = (u_int64_t *)malloc(graph->vert_num * sizeof(u_int64_t));
    u_int64_t sum = 0;

    dijkstra_h(graph, 0, points);

    for (u_int32_t i = 0; i < graph->vert_num; i++) {
        sum += points[i];
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