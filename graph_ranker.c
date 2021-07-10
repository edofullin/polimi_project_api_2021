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
    char buffer[50];
    graph_t graphs[100];
    int last_used = 0;

    scanf("%s", buffer);
    parse_head(buffer, &N, &K);

    while (1)
    {
        scanf("%s", buffer);
        if (!strcmp(buffer, "AggiungiGrafo"))
        {
            // aggiungi grafo

            graphs[last_used+1].col = N;
            graphs[last_used+1].matrix = malloc(N * N * sizeof(int));
            int index = 0;
            char *tok;

            for (int i = 0; i < N; i++)
            {
                scanf("%d", buffer);
                tok = strtok(buffer, ",");
                *(graphs[last_used+1].matrix + index) = atoi(tok);

                for (int j = 0; j < N - 1; ++j)
                {

                    tok = strtok(NULL, ",");
                    *(graphs[last_used+1].matrix + index) = atoi(tok);
                    index++;
                }
            }
        }
        else if (!strcmp(buffer, "TopK"))
        {
            // top K
        }
        else
        {
            exit(0);
        }
    }
}

void parse_head(char *str, int *N, int *K)
{
    char *tok = strtok(str, ",");
    N = atoi(tok);

    tok = strtok(NULL, ",");
    K = atoi(tok);
}