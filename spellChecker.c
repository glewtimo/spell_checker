#define _CRT_SECURE_NO_WARNINGS

#include "hashMap.h"
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Allocates a string for the next word in the file and returns it. This string
 * is null terminated. Returns NULL after reaching the end of the file.
 * @param file
 * @return Allocated string or NULL.
 */
char* nextWord(FILE* file)
{
    int maxLength = 16;
    int length = 0;
    char* word = malloc(sizeof(char) * maxLength);
    while (1)
    {
        char c = fgetc(file);
        if ((c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            c == '\'')
        {
            if (length + 1 >= maxLength)
            {
                maxLength *= 2;
                word = realloc(word, maxLength);
            }
            word[length] = c;
            length++;
        }
        else if (length > 0 || c == EOF)
        {
            break;
        }
    }
    if (length == 0)
    {
        free(word);
        return NULL;
    }
    word[length] = '\0';
    return word;
}

/**
 * Loads the contents of the file into the hash map.
 * @param file
 * @param map
 */
void loadDictionary(FILE* file, HashMap* map)
{
	char* word = nextWord(file);

	while (word != NULL) {
		hashMapPut(map, word, -1);
		word = nextWord(file);
	}
}

/**
* Description: Function to calculate Levenshtein Difference between two strings
* Citation: Adapted from pseudocode found at https://en.wikipedia.org/wiki/Levenshtein_distance */
int levenshteinDistance(char* str1, char* str2) {
	//Allocate memory for matrix
	int rows = strlen(str1) + 1;
	int cols = strlen(str2) + 1;
	int** matrix = (int**)malloc(rows * sizeof(int*));
	for (int i = 0; i < rows; i++) {
		matrix[i] = (int*)malloc(cols * sizeof(int));
	}

	//Set all elements in matrix to 0
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			matrix[i][j] = 0;
		}
	}

	//Fill out top row of matrix
	for (int i = 1; i < cols; i++) {
		matrix[0][i] = i;
	}

	//Fill out first column of matrix
	for (int i = 1; i < rows; i++) {
		matrix[i][0] = i;
	}

	//Fill in the rest of the matrix
	for (int i = 1; i < rows; i++) {
		for (int j = 1; j < cols; j++) {
			//Calc substitution cost adder
			int subAdder;
			if (str1[i-1] == str2[j-1]) {
				subAdder = 0;
			}
			else {
				subAdder = 1;
			}

			//Calc cost of different operations
			int deleteCost = matrix[i - 1][j] + 1;
			int insertCost = matrix[i][j - 1] + 1;
			int subCost = matrix[i - 1][j - 1] + subAdder;

			//Calc min of operations
			int min = deleteCost;
			if (insertCost < min) {
				min = insertCost;
			}
			if (subCost < min) {
				min = subCost;
			}

			//Set matrix value to min of operations
			matrix[i][j] = min;
		}
	}

	//Find levenshtein distance
	int dist = matrix[rows-1][cols-1];

	//Free matrix
	for (int i = 0; i < rows; i++) {
		free(matrix[i]);
	}
	free(matrix);

	//Return distance
	return dist;
}



/**
 * Checks the spelling of the word provded by the user. If the word is spelled incorrectly,
 * print the 5 closest words as determined by a metric like the Levenshtein distance.
 * Otherwise, indicate that the provded word is spelled correctly. Use dictionary.txt to
 * create the dictionary.
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char** argv)
{
    HashMap* map = hashMapNew(1000);

    FILE* file = fopen("dictionary.txt", "r");
    clock_t timer = clock();
    loadDictionary(file, map);
    timer = clock() - timer;
    printf("Dictionary loaded in %f seconds\n", (float)timer / (float)CLOCKS_PER_SEC);
    fclose(file);

    char inputBuffer[256];
    int quit = 0;
    while (!quit)
    {
        printf("Enter a word or \"quit\" to quit: ");
        scanf("%s", inputBuffer);

		//Convert input to lowercase
		char bufferLower[256];
		strcpy(bufferLower, inputBuffer);
		for (int i = 0; bufferLower[i]; i++) {
			bufferLower[i] = tolower(bufferLower[i]);
		}

		//Traverse map of words and add lev distance between buffer and word
		for (int i = 0; i < hashMapSize(map); i++) {
			HashLink* currLink = map->table[i];
			while (currLink != 0) {
				//Calc the lev distance between buffer and word
				int levDist = levenshteinDistance(bufferLower, currLink->key);
				
				//Assign lev distance to word
				hashMapPut(map, currLink->key, levDist);
				
				//Update currLink
				currLink = currLink->next;
			}
		}

		//Check if word is spelled correctly
		if (hashMapContainsKey(map, bufferLower)) {
			printf("The inputted word '%s' is spelled correctly\n", inputBuffer);
		}
		else {
			//Array to store words that are a close match
			char closeMatches[5][255];
			int closeWordsFound = 0;

			//First search for words that are leven dist of 1, then 2, then 3....
			for (int j = 1; j < 255; j++) {
				//Traverse map
				for (int i = 0; i < hashMapSize(map); i++) {
					HashLink* currLink = map->table[i];
					while (currLink != 0) {
						//If word with current lev distance search value is found, add
						//to array
						if (currLink->value == j && closeWordsFound < 5) {
							strcpy(closeMatches[closeWordsFound], currLink->key);
							closeWordsFound++;
						}
						currLink = currLink->next;
					}
				}
			}

			//Print 5 closest words
			printf("The inputted word '%s' is spelled incorrectly\n", inputBuffer);
			printf("Did you mean %s, %s, %s, %s, %s?\n", closeMatches[0], closeMatches[1], closeMatches[2], closeMatches[3], closeMatches[4]);
		}



        if (strcmp(inputBuffer, "quit") == 0)
        {
            quit = 1;
        }
    }

    hashMapDelete(map);
    return 0;
}
