#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

int L; // latura matricei
int N; // numarul de elemente de sortat
int P;
int *v;
int *vQSort;
int **M;

// A fost adăugată o barieră globală pentru sincronizare
pthread_barrier_t barrier;

void compare_vectors(int *a, int *b) {
    for (int i = 0; i < N; i++) {
        if (a[i] != b[i]) {
            printf("Sortare incorecta\n");
            return;
        }
    }
    printf("Sortare corecta\n");
}

void display_vector(int *v) {
    int display_width = 2 + log10(N);
    for (int i = 0; i < N; i++) {
        printf("%*i", display_width, v[i]);
    }
    printf("\n");
}

void display_matrix(int **M) {
    int display_width = 2 + log10(N);
    for (int i = 0; i < L; i++) {
        for (int j = 0; j < L; j++) {
            printf("%*i", display_width, M[i][j]);
        }
        printf("\n");
    }
}

void copy_matrix_in_vector(int *v, int **M) {
    for (int i = 0; i < L; i++) {
        if (i % 2 == 0) {
            for (int j = 0; j < L; j++) {
                v[i * L + j] = M[i][j];
            }
        } else {
            for (int j = L - 1; j >= 0; j--) {
                v[i * L + (L - 1 - j)] = M[i][j];
            }
        }
    }
}

int cmp(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

int cmpdesc(const void *a, const void *b) {
    return (*(int*)b - *(int*)a);
}

void get_args(int argc, char **argv) {
    if (argc < 3) {
        printf("Numar insuficient de parametri: ./shear L P (L = latura matricei)\n");
        exit(1);
    }
    L = atoi(argv[1]);
    N = L * L;
    P = atoi(argv[2]);
}

void init() {
    v = malloc(sizeof(int) * N);
    vQSort = malloc(sizeof(int) * N);
    M = malloc(sizeof(int*) * L);
    if (v == NULL || vQSort == NULL || M == NULL) {
        printf("Eroare la malloc!");
        exit(1);
    }
    for (int i = 0; i < L; i++) {
        M[i] = malloc(sizeof(int) * L);
    }
    srand(42);
    for (int i = 0; i < L; i++) {
        for (int j = 0; j < L; j++) {
            M[i][j] = rand() % N;
        }
    }
}

void print() {
    printf("M:\n");
    display_matrix(M);

    copy_matrix_in_vector(v, M);
    printf("v:\n");
    display_vector(v);
    printf("vQSort:\n");
    display_vector(vQSort);
    compare_vectors(v, vQSort);
}

void *thread_function(void *arg) {
    int thread_id = *(int *)arg;
    int aux[L];

    // Shear sort paralel
    for (int k = 0; k < log(N) + 1; k++) {
        // Sortarea rândurilor în paralel
        for (int i = thread_id; i < L; i += P) {
            if (i % 2) {
                qsort(M[i], L, sizeof(int), cmpdesc);
            } else {
                qsort(M[i], L, sizeof(int), cmp);
            }
        }

        // CORECTAT: Sincronizare cu barieră
        pthread_barrier_wait(&barrier);

        // Sortarea coloanelor în paralel
        for (int i = thread_id; i < L; i += P) {
            for (int j = 0; j < L; j++) {
                aux[j] = M[j][i];
            }
            qsort(aux, L, sizeof(int), cmp);
            for (int j = 0; j < L; j++) {
                M[j][i] = aux[j];
            }
        }

        // CORECTAT: Sincronizare cu barieră
        pthread_barrier_wait(&barrier);
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    get_args(argc, argv);
    init();

    pthread_t tid[P];
    int thread_id[P];

    // se sorteaza etalonul
    copy_matrix_in_vector(vQSort, M);
    qsort(vQSort, N, sizeof(int), cmp);

    // CORECTAT: Se inițializează bariera
    pthread_barrier_init(&barrier, NULL, P);

    // se creeaza thread-urile
    for (int i = 0; i < P; i++) {
        thread_id[i] = i;
        pthread_create(&tid[i], NULL, thread_function, &thread_id[i]);
    }

    // se asteapta thread-urile
    for (int i = 0; i < P; i++) {
        pthread_join(tid[i], NULL);
    }
    // CORECTAT: Se distruge bariera
    pthread_barrier_destroy(&barrier);

    // CORECTAT: S-a eliminat blocul de cod care executa sortarea secvențială
    // după ce thread-urile și-au terminat treaba.

    print();

    free(v);
    free(vQSort);
    for (int i = 0; i < L; i++) {
        free(M[i]);
    }
    free(M);

    return 0;
}