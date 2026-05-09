#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#define INITIAL_CAPACITY 1024  // Initial capacity for dynamic arrays
#define MAX_WORD_LENGTH 50     // Maximum length of each word
#define MAX_PROCESSES 6

typedef struct {
    char word[MAX_WORD_LENGTH];
    int frequency;
} WordFreq;

void grow_array_if_needed(int *count, int *capacity, void **array, size_t element_size) {
    if (*count >= *capacity) {
        *capacity *= 2;
        void *new_array = realloc(*array, (*capacity) * element_size);
        if (!new_array) {
            fprintf(stderr, "Memory allocation failed during resizing.\n");
            exit(EXIT_FAILURE);
        }
        *array = new_array;
    }
}

int read_words(const char *filename, char ***words, int *word_count) {
    clock_t start_time = clock();

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

    clock_t end_time = clock();
    printf("Time taken in read_words: %.6f seconds\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);
    return 0;
}

void calculate_frequencies_in_child(char **words, int start, int end, int process_id) {
    clock_t start_time = clock();

    WordFreq *frequencies = malloc(INITIAL_CAPACITY * sizeof(WordFreq));
    int unique_count = 0;
    int capacity = INITIAL_CAPACITY;

    for (int i = start; i < end; i++) {
        int found = 0;
        for (int j = 0; j < unique_count; j++) {
            if (strcmp(words[i], frequencies[j].word) == 0) {
                frequencies[j].frequency++;
                found = 1;
                break;
            }
        }
        if (!found) {
            grow_array_if_needed(&unique_count, &capacity, (void **)&frequencies, sizeof(WordFreq));
            strcpy(frequencies[unique_count].word, words[i]);
            frequencies[unique_count].frequency = 1;
            unique_count++;
        }
    }

    char filename[20];
    snprintf(filename, sizeof(filename), "frequencies_%d.txt", process_id);
    FILE *file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: Could not open file '%s' for writing.\n", filename);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < unique_count; i++) {
        fprintf(file, "%s %d\n", frequencies[i].word, frequencies[i].frequency);
    }

    fclose(file);
    free(frequencies);

    clock_t end_time = clock();
    printf("Time taken by process %d: %.6f seconds\n", process_id, (double)(end_time - start_time) / CLOCKS_PER_SEC);
}

int merge_frequencies(WordFreq **frequencies, int *unique_count, int num_processes) {
    clock_t start_time = clock();

    int capacity = INITIAL_CAPACITY;
    *frequencies = malloc(capacity * sizeof(WordFreq));
    if (!*frequencies) {
        fprintf(stderr, "Memory allocation failed for merged frequencies.\n");
        return -1;
    }

    for (int i = 0; i < num_processes; i++) {
        char filename[20];
        snprintf(filename, sizeof(filename), "frequencies_%d.txt", i);

        FILE *file = fopen(filename, "r");
        if (!file) {
            fprintf(stderr, "Error: Could not open file '%s' for reading.\n", filename);
            return -1;
        }

        char word[MAX_WORD_LENGTH];
        int frequency;
        while (fscanf(file, "%49s %d", word, &frequency) == 2) {
            int found = 0;
            for (int j = 0; j < *unique_count; j++) {
                if (strcmp(word, (*frequencies)[j].word) == 0) {
                    (*frequencies)[j].frequency += frequency;
                    found = 1;
                    break;
                }
            }
            if (!found) {
                grow_array_if_needed(unique_count, &capacity, (void **)frequencies, sizeof(WordFreq));
                strcpy((*frequencies)[*unique_count].word, word);
                (*frequencies)[*unique_count].frequency = frequency;
                (*unique_count)++;
            }
        }

        fclose(file);
    }

    clock_t end_time = clock();
    printf("Time taken in merge_frequencies: %.6f seconds\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);
    return 0;
}
int compare_frequencies(const void *a, const void *b) {
    WordFreq *wf1 = (WordFreq *)a;
    WordFreq *wf2 = (WordFreq *)b;
    return wf2->frequency - wf1->frequency; // Descending order
}
void print_top_frequencies(WordFreq *frequencies, int unique_count, int top_n) {
    clock_t start_time = clock();

    printf("Top %d most frequent words:\n", top_n);
    for (int i = 0; i < top_n && i < unique_count; i++) {
        printf("%s: %d\n", frequencies[i].word, frequencies[i].frequency);
    }

    clock_t end_time = clock();
    printf("Time taken in print_top_frequencies: %.6f seconds\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);
}

void free_words(char **words, int word_count) {
    for (int i = 0; i < word_count; i++) {
        free(words[i]);
    }
    free(words);
}

void free_frequencies(WordFreq *frequencies) {
    free(frequencies);
}

int main() {
    char **words;
    WordFreq *frequencies = NULL;
    int word_count, unique_count = 0;
    int num_processes =4;

    if (read_words("data.txt", &words, &word_count) < 0) {
        return EXIT_FAILURE;
    }
    printf("Total number of words: %d\n", word_count);

    clock_t process_start_time = clock();

    pid_t pids[num_processes];
    int words_per_process = word_count / num_processes;

    for (int i = 0; i < num_processes; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            int start = i * words_per_process;
            int end = (i == num_processes - 1) ? word_count : start + words_per_process;
            calculate_frequencies_in_child(words, start, end, i);
            exit(0);
        }
    }

    for (int i = 0; i < num_processes; i++) {
        wait(NULL);
    }

    clock_t process_end_time = clock();
    printf("Time taken by parent process: %.6f seconds\n", (double)(process_end_time - process_start_time) / CLOCKS_PER_SEC);

    if (merge_frequencies(&frequencies, &unique_count, num_processes) < 0) {
        free_words(words, word_count);
        return EXIT_FAILURE;
    }

    qsort(frequencies, unique_count, sizeof(WordFreq), compare_frequencies);
    print_top_frequencies(frequencies, unique_count, 10);

    free_words(words, word_count);
    free_frequencies(frequencies);

    return EXIT_SUCCESS;
}
