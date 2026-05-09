// aya abdullah 1220782
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INITIAL_CAPACITY 1024  // Initial capacity for dynamic arrays
#define MAX_WORD_LENGTH 50     // Maximum length of each word

// Structure to store a word and its frequency
typedef struct {
    char word[MAX_WORD_LENGTH];
    int frequency;
} WordFreq;

// Function to dynamically grow an array if needed
void grow_array_if_needed(int *count, int *capacity, void **array, size_t element_size) {
    if (*count >= *capacity) {
        *capacity *= 2;  // Double the capacity
        void *new_array = realloc(*array, (*capacity) * element_size);
        if (!new_array) {
            fprintf(stderr, "Memory allocation failed during resizing.\n");
            exit(EXIT_FAILURE);
        }
        *array = new_array;
    }
}

// Read words from a file into an array
int read_words(const char *filename, char ***words, int *word_count) {
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
    return 0;
}

// Calculate word frequencies
int calculate_frequencies(char **words, int word_count, WordFreq **frequencies, int *unique_count) {
    int capacity = INITIAL_CAPACITY;
    *frequencies = malloc(capacity * sizeof(WordFreq));
    if (!*frequencies) {
        fprintf(stderr, "Memory allocation failed for word frequencies.\n");
        return -1;
    }

    *unique_count = 0;
    for (int i = 0; i < word_count; i++) {
        int found = 0;
        for (int j = 0; j < *unique_count; j++) {
            if (strcmp(words[i], (*frequencies)[j].word) == 0) {
                (*frequencies)[j].frequency++;
                found = 1;
                break;
            }
        }
        if (!found) {
            grow_array_if_needed(unique_count, &capacity, (void **)frequencies, sizeof(WordFreq));
            strcpy((*frequencies)[*unique_count].word, words[i]);
            (*frequencies)[*unique_count].frequency = 1;
            (*unique_count)++;
        }
    }

    return 0;
}

// Sort word frequencies in descending order
int compare_frequencies(const void *a, const void *b) {
    WordFreq *wf1 = (WordFreq *)a;
    WordFreq *wf2 = (WordFreq *)b;
    return wf2->frequency - wf1->frequency;
}

// Print the top N most frequent words
void print_top_frequencies(WordFreq *frequencies, int unique_count, int top_n) {
    printf("Top %d most frequent words:\n", top_n);
    for (int i = 0; i < top_n && i < unique_count; i++) {
        printf("%s: %d\n", frequencies[i].word, frequencies[i].frequency);
    }
}

// Free memory used by words
void free_words(char **words, int word_count) {
    for (int i = 0; i < word_count; i++) {
        free(words[i]);
    }
    free(words);
}

// Free memory used by word frequencies
void free_frequencies(WordFreq *frequencies) {
    free(frequencies);
}

// Main function
int main() {
    char **words;
    WordFreq *frequencies;
    int word_count, unique_count;

    // Read words from the file
    if (read_words("data.txt", &words, &word_count) < 0) {
        return EXIT_FAILURE;
    }
    printf("Total number of words: %d\n", word_count);

    // Start timing
    clock_t start_time = clock();

    // Calculate word frequencies
    if (calculate_frequencies(words, word_count, &frequencies, &unique_count) < 0) {
        free_words(words, word_count);
        return EXIT_FAILURE;
    }

    // End timing
    clock_t end_time = clock();
    double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("Number of unique words: %d\n", unique_count);
    printf("Time taken to calculate frequencies: %.6f seconds\n", time_taken);

    // Sort and print top 10 most frequent words
    qsort(frequencies, unique_count, sizeof(WordFreq), compare_frequencies);
    print_top_frequencies(frequencies, unique_count, 10);

    // Free allocated memory
    free_words(words, word_count);
    free_frequencies(frequencies);

    return EXIT_SUCCESS;
}
