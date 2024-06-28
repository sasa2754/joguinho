#ifndef SCORES
#define SCORES

#include <stdio.h>
#include <stdlib.h>

#include "structs.h"

typedef struct {
    int capacity;
    int size;
    Player * array;
} Queue;

Queue construct_fila() {
    Queue newqueue;
    newqueue.array = NULL;
    newqueue.capacity = 0;
    newqueue.size = 0;
    return newqueue;
}

void enqueue(Queue * queue, Player player) {
    if (queue->array == NULL) {
        queue->capacity = 1;
        queue->array = (Player*) malloc(queue->capacity * sizeof(Player));
        queue->array[queue->size++] = player;
        return;
    }

    if (queue->capacity == queue->size + 1) {
        queue->capacity *= 2;
        queue->array = (Player*) realloc(queue->array, queue->capacity * sizeof(Player));
    }

    queue->array[queue->size++] = player;
}

// Merge two subarrays
void merge(Player arr[], int l, int m, int r) {
    int n1 = m - l + 1;
    int n2 = r - m;

    Player* L = (Player*) malloc(n1 * sizeof(Player));
    Player* R = (Player*) malloc(n2 * sizeof(Player));

    for (int i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (int j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        if (L[i].score >= R[j].score)
            arr[k++] = L[i++];
        else
            arr[k++] = R[j++];
    }

    while (i < n1)
        arr[k++] = L[i++];

    while (j < n2)
        arr[k++] = R[j++];

    free(L);
    free(R);
}

// Merge Sort function
void mergeSort(Player arr[], int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;
        mergeSort(arr, l, m);
        mergeSort(arr, m + 1, r);
        merge(arr, l, m, r);
    }
}


#endif // !SCORES
