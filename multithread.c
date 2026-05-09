#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/time.h>

#define INITIAL_CAPACITY 1024
#define MAX_WORD_LENGTH 50
#define NUM_THREADS 2

typedef struct {
    char word[MAX_WORD_LENGTH];
    int frequency;
} WordFreq;

typedef struct {
    char **words;
    int start_index;
    int end_index;
    WordFreq **local_frequencies;
    int *local_unique_count;
    int *local_capacity;
    double start_time;
    double end_time;
} ThreadData;

sem_t frequency_semaphore; // Semaphore for synchronizing access to global frequencies

// Function to get current time in seconds, including microseconds.
double get_time_seconds() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec + (time.tv_usec / 1000000.0);
}

void grow_array_if_needed(int *count, int *capacity, void **array, size_t element_size) {
    double func_start_time = get_time_seconds(); // Start time for grow_array_if_needed
    if (*count >= *capacity) {
        *capacity *= 2;
        void *new_array = realloc(*array, (*capacity) * element_size);
        if (!new_array) {
            fprintf(stderr, "Memory allocation failed during resizing.\n");
            exit(EXIT_FAILURE);
        }
        *array = new_array;
    }
    double func_end_time = get_time_seconds(); // End time for grow_array_if_needed
    printf("Function 'grow_array_if_needed' took %.4f seconds.\n", func_end_time - func_start_time);
}

int read_words(const char *filename, char ***words, int *word_count) {
    double func_start_time = get_time_seconds(); // Start time for read_words

    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file '%s'.\n", filename);
        return -1;
    }

    int capacity = INITIAL_CAPACITY;
    *words = malloc(capacity * sizeof(char *));
    if (!*words) {
        fprintf(stderr, "Memory allocation failed for words array.\n");
        fclose(file);
        return -1;
    }

    char buffer[MAX_WORD_LENGTH];
    *word_count = 0;
    while (fscanf(file, "%49s", buffer) == 1) {
        grow_array_if_needed(word_count, &capacity, (void **)words, sizeof(char *));
        (*words)[*word_count] = malloc(strlen(buffer) + 1);
        if (!(*words)[*word_count]) {
            fprintf(stderr, "Memory allocation failed for word '%s'.\n", buffer);
            fclose(file);
            return -1;
        }
        strcpy((*words)[*word_count], buffer);
        (*word_count)++;
    }

    fclose(file);
    double func_end_time = get_time_seconds(); // End time for read_words
    printf("Function 'read_words' took %.4f seconds.\n", func_end_time - func_start_time);

    return 0;
}

void *calculate_frequencies_thread(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    data->start_time = get_time_seconds(); // Start time for thread

    double func_start_time = get_time_seconds(); // Start time for calculate_frequencies_thread
    int local_unique_count = 0;
    WordFreq *local_frequencies = malloc(INITIAL_CAPACITY * sizeof(WordFreq));

    for (int i = data->start_index; i < data->end_index; i++) {
        int found = 0;
        for (int j = 0; j < local_unique_count; j++) {
            if (strcmp(local_frequencies[j].word, data->words[i]) == 0) {
                local_frequencies[j].frequency++;
                found = 1;
                break;
            }
        }
        if (!found) {
            grow_array_if_needed(&local_unique_count, data->local_capacity, (void **)&local_frequencies, sizeof(WordFreq));
            strcpy(local_frequencies[local_unique_count].word, data->words[i]);
            local_frequencies[local_unique_count].frequency = 1;
            local_unique_count++;
        }
    }

    data->end_time = get_time_seconds(); // End time for thread
    data->local_frequencies = local_frequencies;
    data->local_unique_count = local_unique_count;

    double func_end_time = get_time_seconds(); // End time for calculate_frequencies_thread
    printf("Function 'calculate_frequencies_thread' took %.4f seconds.\n", func_end_time - func_start_time);

    return NULL;
}

void merge_frequencies(WordFreq **global_frequencies, int *global_unique_count, int *capacity, WordFreq *local_frequencies, int local_unique_count) {
    double func_start_time = get_time_seconds(); // Start time for merge_frequencies

    sem_wait(&frequency_semaphore); // Enter critical section

    for (int i = 0; i < local_unique_count; i++) {
        int found = 0;
        for (int j = 0; j < *global_unique_count; j++) {
            if (strcmp((*global_frequencies)[j].word, local_frequencies[i].word) == 0) {
                (*global_frequencies)[j].frequency += local_frequencies[i].frequency;
                found = 1;
                break;
            }
        }
        if (!found) {
            grow_array_if_needed(global_unique_count, capacity, (void **)global_frequencies, sizeof(WordFreq));
            strcpy((*global_frequencies)[*global_unique_count].word, local_frequencies[i].word);
            (*global_frequencies)[*global_unique_count].frequency = local_frequencies[i].frequency;
            (*global_unique_count)++;
        }
    }

    sem_post(&frequency_semaphore); // Exit critical section

    double func_end_time = get_time_seconds(); // End time for merge_frequencies
    printf("Function 'merge_frequencies' took %.4f seconds.\n", func_end_time - func_start_time);
}

int compare_frequencies(const void *a, const void *b) {
    WordFreq *wf1 = (WordFreq *)a;
    WordFreq *wf2 = (WordFreq *)b;
    return wf2->frequency - wf1->frequency;
}

void print_top_frequencies(WordFreq *frequencies, int unique_count, int top_n) {
    double func_start_time = get_time_seconds(); // Start time for print_top_frequencies

    printf("Top %d most frequent words:\n", top_n);
    for (int i = 0; i < top_n && i < unique_count; i++) {
        printf("%s: %d\n", frequencies[i].word, frequencies[i].frequency);
    }

    double func_end_time = get_time_seconds(); // End time for print_top_frequencies
    printf("Function 'print_top_frequencies' took %.4f seconds.\n", func_end_time - func_start_time);
}

void free_words(char **words, int word_count) {
    double func_start_time = get_time_seconds(); // Start time for free_words

    for (int i = 0; i < word_count; i++) {
        free(words[i]);
    }
    free(words);

    double func_end_time = get_time_seconds(); // End time for free_words
    printf("Function 'free_words' took %.4f seconds.\n", func_end_time - func_start_time);
}

void free_frequencies(WordFreq *frequencies) {
    double func_start_time = get_time_seconds(); // Start time for free_frequencies

    free(frequencies);

    double func_end_time = get_time_seconds(); // End time for free_frequencies
    printf("Function 'free_frequencies' took %.4f seconds.\n", func_end_time - func_start_time);
}

int main() {
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL); // Start time for the whole program

    char **words;
    WordFreq *frequencies;
    int word_count, unique_count;

    if (read_words("data.txt", &words, &word_count) < 0) {
        return EXIT_FAILURE;
    }
    printf("Total number of words: %d\n", word_count);

    frequencies = malloc(INITIAL_CAPACITY * sizeof(WordFreq));
    unique_count = 0;
    int capacity = INITIAL_CAPACITY;

    sem_init(&frequency_semaphore, 0, 1); // Initialize semaphore

    pthread_t threads[NUM_THREADS];
    int chunk_size = word_count / NUM_THREADS;

    ThreadData thread_data[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].words = words;
        thread_data[i].start_index = i * chunk_size;
        thread_data[i].end_index = (i == NUM_THREADS - 1) ? word_count : (i + 1) * chunk_size;
        thread_data[i].local_frequencies = NULL;
        thread_data[i].local_unique_count = 0;
        thread_data[i].local_capacity = malloc(sizeof(int));
        *thread_data[i].local_capacity = INITIAL_CAPACITY;
        pthread_create(&threads[i], NULL, calculate_frequencies_thread, &thread_data[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        merge_frequencies(&frequencies, &unique_count, &capacity, thread_data[i].local_frequencies, thread_data[i].local_unique_count);
    }

    qsort(frequencies, unique_count, sizeof(WordFreq), compare_frequencies);
    print_top_frequencies(frequencies, unique_count, 10);

    gettimeofday(&end_time, NULL); // End time for the whole program
    double total_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    printf("Total program execution time: %.4f seconds.\n", total_time);

    free_words(words, word_count);
    free_frequencies(frequencies);
    sem_destroy(&frequency_semaphore); // Clean up semaphore

    return EXIT_SUCCESS;
}
