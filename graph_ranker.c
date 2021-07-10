#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parse_head(char *str, int *N, int *K);

typedef struct
{

    int col;
    int *matrix;

} graph_t;

int main()
{

    int N, K;
    char buffer[4000];
    graph_t graphs[100];
    int next = 0;

    scanf("%d", &N);
    scanf("%d", &K);

    while (1)
    {
        scanf("%s", buffer);
        if (!strcmp(buffer, "AggiungiGrafo"))
        {
            // aggiungi grafo
            printf("Aggiungo Grafo #%d", next);

            graphs[next].col = N;
            graphs[next].matrix = malloc(N * N * sizeof(int));
            int index = 0;
            char *tok;

            for (int i = 0; i < N; i++)
            {
                scanf("%s", buffer);
                tok = strtok(buffer, ",");
                *(graphs[next].matrix + index++) = atoi(tok);

                for (int j = 0; j < N - 1; ++j)
                {

                    tok = strtok(NULL, ",");
                    *(graphs[next].matrix + index++) = atoi(tok);
                }
            }

            next++;
        }
        else if (!strcmp(buffer, "TopK"))
        {
            // top K
            exit(0);
        }
        else
        {
            exit(0);
        }
    }
}