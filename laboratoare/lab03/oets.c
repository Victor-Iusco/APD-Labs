#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

int N;
int P;
int *v;
int *vQSort;
pthread_barrier_t barrier;

void compare_vectors(int *a, int *b) {
    int i;

    for (i = 0; i < N; i++) {
        if (a[i] != b[i]) {
            printf("Sortare incorecta\n");
            return;
        }
    }

    printf("Sortare corecta\n");
}

void display_vector(int *v) {
    int i;
    int display_width = 2 + log10(N);

    for (i = 0; i < N; i++) {
        printf("%*i", display_width, v[i]);
    }

    printf("\n");
}

int cmp(const void *a, const void *b) {
    int A = *(int*)a;
    int B = *(int*)b;
    return A - B;
}

void get_args(int argc, char **argv)
{
    if(argc < 3) {
        printf("Numar insuficient de parametri: ./oets N P\n");
        exit(1);
    }

    N = atoi(argv[1]); // Nr elem din vector
    P = atoi(argv[2]); // Nr thread-uri
}

void init()
{
    int i;
    v = malloc(sizeof(int) * N);
    vQSort = malloc(sizeof(int) * N);

    if (v == NULL || vQSort == NULL) {
        printf("Eroare la malloc!");
        exit(1);
    }

    srand(42);

    for (i = 0; i < N; i++)
        v[i] = rand() % N;
}

void print()
{
    printf("v:\n");
    display_vector(v);
    printf("vQSort:\n");
    display_vector(vQSort);
    compare_vectors(v, vQSort);
}

void *thread_function(void *arg)
{
    int thread_id = *(int *)arg;
    int aux;

    // Odd-Even Transposition Sort paralel
    // Algoritmul are N faze pentru a garanta sortarea
    for (int k = 0; k < N; k++) {
        // Faza para: comparam elementele de pe pozitiile (0,1), (2,3), ...
        if (k % 2 == 0) {
            int num_pairs = N / 2;
            int start = thread_id * (num_pairs / P);
            int end = (thread_id + 1) * (num_pairs / P);
            if (thread_id == P - 1) {
                end = num_pairs;
            }

            for (int j = start; j < end; j++) {
                int i = 2 * j;
                if (v[i] > v[i + 1]) {
                    aux = v[i];
                    v[i] = v[i + 1];
                    v[i + 1] = aux;
                }
            }
        } else { // Faza impara: comparam elementele de pe pozitiile (1,2), (3,4), ...
            int num_pairs = (N - 1) / 2;
            int start = thread_id * (num_pairs / P);
            int end = (thread_id + 1) * (num_pairs / P);
            if (thread_id == P - 1) {
                end = num_pairs;
            }

            for (int j = start; j < end; j++) {
                int i = 2 * j + 1;
                if (v[i] > v[i + 1]) {
                    aux = v[i];
                    v[i] = v[i + 1];
                    v[i + 1] = aux;
                }
            }
        }

        // Sincronizam thread-urile la finalul fiecarei faze
        pthread_barrier_wait(&barrier);
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    get_args(argc, argv);
    init();

    int i;
    pthread_t tid[P];
    int thread_id[P];

    // se sorteaza vectorul etalon
    for (i = 0; i < N; i++)
        vQSort[i] = v[i];
    qsort(vQSort, N, sizeof(int), cmp);

    // se initializeaza bariera
    pthread_barrier_init(&barrier, NULL, P);

    // se creeaza thread-urile
    for (i = 0; i < P; i++) {
        thread_id[i] = i;
        pthread_create(&tid[i], NULL, thread_function, &thread_id[i]);
    }

    // se asteapta thread-urile
    for (i = 0; i < P; i++) {
        pthread_join(tid[i], NULL);
    }

    // se distruge bariera
    pthread_barrier_destroy(&barrier);

    // se afiseaza vectorul etalon
    // se afiseaza vectorul curent
    // se compara cele doua
    print();

    free(v);
    free(vQSort);

    return 0;
}