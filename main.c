#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_WORDS 3070      // Maximum number of words to store
#define MAX_WORD_LENGTH 70   // Maximum length of each word

// Structure to store a word and its frequency
typedef struct {
    char word[MAX_WORD_LENGTH];
    int frequency;
} WordFreq;

// Read up to MAX_WORDS from a file into an array
int read_words(const char *filename, char words[MAX_WORDS][MAX_WORD_LENGTH], int *word_count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file '%s'.\n", filename);
        return -1;
    }

    *word_count = 0;
    while (fscanf(file, "%49s", words[*word_count]) == 1) {
        (*word_count)++;
        if (*word_count >= MAX_WORDS) {
            printf("Reached maximum capacity of %d words. Stopping read.\n", MAX_WORDS);
            break;
        }
    }

    fclose(file);
    return 0;
}

// Compare function for sorting words (ascending order)
int compare_words(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
}

// Calculate word frequencies after sorting
int calculate_frequencies(char words[MAX_WORDS][MAX_WORD_LENGTH], int word_count, WordFreq frequencies[], int *unique_count) {
    *unique_count = 0;

    // Sort words
    qsort(words, word_count, MAX_WORD_LENGTH, compare_words);

    for (int i = 0; i < word_count; i++) {
        // Count consecutive words after sorting
        if (i == 0 || strcmp(words[i], words[i - 1]) != 0) {
            // New unique word, add to frequencies list
            strcpy(frequencies[*unique_count].word, words[i]);
            frequencies[*unique_count].frequency = 1;
            (*unique_count)++;
        } else {
            // Same as previous word, increment frequency
            frequencies[*unique_count - 1].frequency++;
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
void print_top_frequencies(WordFreq frequencies[], int unique_count, int top_n) {
    printf("Top %d most frequent words:\n", top_n);
    for (int i = 0; i < top_n && i < unique_count; i++) {
        printf("%s: %d\n", frequencies[i].word, frequencies[i].frequency);
    }
}

// Main function
int main() {
    char words[MAX_WORDS][MAX_WORD_LENGTH]; // Fixed array for words
    WordFreq frequencies[MAX_WORDS];       // Fixed array for word frequencies
    int word_count, unique_count;

    // Read words from the file
    if (read_words("input.txt", words, &word_count) < 0) {
        return EXIT_FAILURE;
    }
    printf("Total number of words read: %d\n", word_count);

    // Start timing
    clock_t start_time = clock();

    // Calculate word frequencies
    if (calculate_frequencies(words, word_count, frequencies, &unique_count) < 0) {
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

    return EXIT_SUCCESS;
}

