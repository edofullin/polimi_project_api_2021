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

    if(!scanf("%d", &N)) { exit(-1); };
    if(!scanf("%d", &K)) { exit(-1); };

    while (1)
    {
        if(!scanf("%s", buffer)) { exit(-1); };
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
                if(!scanf("%s", buffer)) { exit(-1); };
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
            // Not implemented
            printf("TopK not implmented");
            exit(3);
        }
        else
        {
            printf("I'm done, good night.");
            exit(0);
        }
    }
}