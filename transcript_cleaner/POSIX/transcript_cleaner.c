/**
 * @file transcript_cleaner.c
 * @brief A high-performance C program to clean up transcript files.
 *
 * This program processes large text files in two passes to clean them up:
 * 1. Pass 1: Removes consecutive duplicate lines.
 * 2. Pass 2: Merges sentences that are split across multiple lines.
 *
 * It features flexible file handling, memory-efficient processing of large
 * files using a temporary file, and reports statistics on completion.
 *
 * Compilation:
 *   gcc -Wall -Wextra -O2 -o transcript_cleaner transcript_cleaner.c
 *
 * Usage:
 *   ./transcript_cleaner [-i inputfile] [-o outputfile]
 */

 #define _GNU_SOURCE // For getline()
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <ctype.h>
 #include <dirent.h>
 #include <unistd.h> // For getopt()
 
 // --- Function Prototypes ---
 
 /**
  * @brief Finds the single .txt file in the current directory.
  * @return A dynamically allocated string with the filename if one is found, else NULL.
  *         The caller must free the returned string.
  */
 static char* find_single_txt_file();
 
 /**
  * @brief Pass 1: Removes consecutive duplicate lines from a file stream.
  * @param input_fp Pointer to the source file.
  * @param temp_fp Pointer to the temporary file for writing de-duplicated content.
  * @param duplicates_removed Pointer to a counter for removed duplicate lines.
  */
 static void remove_duplicates(FILE *input_fp, FILE *temp_fp, int *duplicates_removed);
 
 /**
  * @brief Pass 2: Merges sentences split across lines from a file stream.
  * @param temp_fp Pointer to the de-duplicated temporary file.
  * @param output_fp Pointer to the final output file.
  * @param sentences_merged Pointer to a counter for merged sentences.
  */
 static void merge_sentences(FILE *temp_fp, FILE *output_fp, int *sentences_merged);
 
 /**
  * @brief Displays usage information and exits the program.
  * @param prog_name The name of the executable (from argv[0]).
  */
 static void print_usage_and_exit(const char *prog_name);
 
 
 // --- Main Program Entry Point ---
 
 int main(int argc, char *argv[]) {
     // --- Variable Declarations ---
     char *input_filename = NULL;
     char *output_filename = NULL;
     int opt;
 
     // Counters for final statistics report
     int duplicate_lines_removed = 0;
     int sentences_merged = 0;
 
     // File pointers
     FILE *input_fp = NULL;
     FILE *output_fp = NULL;
     FILE *temp_fp = NULL;
 
     // --- 1. Argument Parsing ---
     // Use getopt to parse command-line flags -i (input) and -o (output).
     while ((opt = getopt(argc, argv, "i:o:h")) != -1) {
         switch (opt) {
             case 'i':
                 input_filename = strdup(optarg);
                 break;
             case 'o':
                 output_filename = strdup(optarg);
                 break;
             case 'h':
             case '?': // getopt returns '?' for an unknown option or missing argument
                 print_usage_and_exit(argv[0]);
                 break;
             default:
                 // This case should not be reached with the current optstring
                 abort();
         }
     }
     
     // --- 2. File Handling Logic ---
 
     // A. Determine Input File
     if (input_filename == NULL) {
         // If -i is not provided, try to find a single .txt file in the current directory.
         input_filename = find_single_txt_file();
         if (input_filename == NULL) {
             fprintf(stderr, "Error: No input file specified with -i and could not find a unique .txt file.\n");
             fprintf(stderr, "Please specify an input file with '-i <filename>' or place exactly one .txt file in the current directory.\n");
             exit(EXIT_FAILURE);
         }
         printf("Auto-detected input file: %s\n", input_filename);
     }
 
     // B. Determine Output File
     if (output_filename == NULL) {
         // If -o is not provided, derive the output name from the input name.
         // Allocate space for input_filename + "_resolved.txt" + null terminator.
         size_t len = strlen(input_filename) + strlen("_resolved.txt") + 1;
         output_filename = malloc(len);
         if (output_filename == NULL) {
             perror("Error allocating memory for output filename");
             free(input_filename); // Free memory allocated by strdup or find_single_txt_file
             exit(EXIT_FAILURE);
         }
         // Construct the output filename
         strcpy(output_filename, input_filename);
         strcat(output_filename, "_resolved.txt");
     }
 
     // C. Open Files
     input_fp = fopen(input_filename, "r");
     if (input_fp == NULL) {
         perror("Error opening input file");
         free(input_filename);
         free(output_filename);
         exit(EXIT_FAILURE);
     }
 
     output_fp = fopen(output_filename, "w");
     if (output_fp == NULL) {
         perror("Error opening output file");
         fclose(input_fp);
         free(input_filename);
         free(output_filename);
         exit(EXIT_FAILURE);
     }
     
     // D. Create Temporary File
     // tmpfile() creates a temporary file that is automatically removed on close.
     // This is ideal for intermediate processing without cluttering the filesystem.
     temp_fp = tmpfile();
     if (temp_fp == NULL) {
         perror("Error creating temporary file");
         fclose(input_fp);
         fclose(output_fp);
         free(input_filename);
         free(output_filename);
         exit(EXIT_FAILURE);
     }
     
     printf("Processing '%s' -> '%s'\n", input_filename, output_filename);
 
     // --- 3. Processing Passes ---
     
     // Pass 1: Remove consecutive duplicate lines and write to the temp file.
     remove_duplicates(input_fp, temp_fp, &duplicate_lines_removed);
     
     // Pass 2: Merge sentences from the temp file and write to the final output file.
     merge_sentences(temp_fp, output_fp, &sentences_merged);
 
     // --- 4. Cleanup and Reporting ---
     
     // Close all file handles. Closing temp_fp will also delete it from the filesystem.
     fclose(input_fp);
     fclose(output_fp);
     fclose(temp_fp);
 
     // Free all dynamically allocated memory for filenames.
     free(input_filename);
     free(output_filename);
     
     // Print the final statistics report to standard output.
     printf("\nProcessing complete.\n");
     printf("- Duplicate lines removed: %d\n", duplicate_lines_removed);
     printf("- Sentences merged: %d\n", sentences_merged);
 
     return EXIT_SUCCESS;
 }
 
 
 // --- Function Implementations ---
 
 /**
  * @brief Displays program usage information and exits.
  */
 static void print_usage_and_exit(const char *prog_name) {
     fprintf(stderr, "Usage: %s [-i inputfile] [-o outputfile]\n\n", prog_name);
     fprintf(stderr, "  Cleans transcript files by removing duplicate lines and merging sentences.\n\n");
     fprintf(stderr, "Options:\n");
     fprintf(stderr, "  -i <path>  Specify the input file.\n");
     fprintf(stderr, "             If omitted, the program searches for a single .txt file\n");
     fprintf(stderr, "             in the current directory.\n");
     fprintf(stderr, "  -o <path>  Specify the output file.\n");
     fprintf(stderr, "             If omitted, defaults to '[input]_resolved.txt'.\n");
     fprintf(stderr, "  -h         Display this help message and exit.\n");
     exit(EXIT_FAILURE);
 }
 
 /**
  * @brief Scans the current directory for a single file ending in ".txt".
  */
 static char* find_single_txt_file() {
     DIR *d;
     struct dirent *dir;
     char *found_filename = NULL;
     int count = 0;
 
     d = opendir(".");
     if (d == NULL) {
         perror("Error opening current directory");
         return NULL;
     }
 
     while ((dir = readdir(d)) != NULL) {
         // Use strrchr to find the last dot, robust against files like "archive.old.txt"
         const char *ext = strrchr(dir->d_name, '.');
         if (ext != NULL && strcmp(ext, ".txt") == 0) {
             count++;
             // If we find more than one, we will ultimately fail. Free any stored name.
             if (count > 1 && found_filename != NULL) {
                 free(found_filename);
                 found_filename = NULL;
             }
             // Store the name of the first .txt file found.
             if (count == 1) {
                 found_filename = strdup(dir->d_name);
                 if (found_filename == NULL) {
                     perror("Error: strdup failed in find_single_txt_file");
                     // Continue to ensure closedir is called, but we will likely fail.
                 }
             }
         }
     }
     closedir(d);
 
     // Only return a filename if exactly one was found.
     if (count == 1) {
         return found_filename;
     }
 
     // If count is 0 or >1, free any memory and return NULL.
     if (found_filename) {
         free(found_filename);
     }
     return NULL;
 }
 
 /**
  * @brief Pass 1: Removes consecutive duplicate lines from a file stream.
  */
 static void remove_duplicates(FILE *input_fp, FILE *temp_fp, int *duplicates_removed) {
     char *line = NULL;
     char *prev_line = NULL;
     size_t len = 0;
     ssize_t nread;
     
     // Initialize prev_line to an empty string. The first line read from a non-empty
     // file will never match this, correctly priming the loop.
     prev_line = strdup("");
     if (!prev_line) {
         perror("Memory allocation failed for prev_line");
         return; // Main will clean up and exit.
     }
 
     // Use getline for safe and efficient line-by-line reading.
     // It automatically handles buffer allocation for 'line'.
     while ((nread = getline(&line, &len, input_fp)) != -1) {
         if (strcmp(line, prev_line) == 0) {
             // If the current line is identical to the previous, it's a duplicate.
             (*duplicates_removed)++;
         } else {
             // Otherwise, write the new, unique line to the temp file.
             fputs(line, temp_fp);
             // Update prev_line for the next comparison. We must copy the string.
             free(prev_line);
             prev_line = strdup(line);
             if (!prev_line) {
                 perror("Memory allocation failed for prev_line");
                 free(line); // Free the buffer from getline before returning
                 return;
             }
         }
     }
 
     // Free the final buffers used by getline and our tracking pointer.
     free(line);
     free(prev_line);
 }
 
 /**
  * @brief Pass 2: Merges sentences split across lines from a file stream.
  */
 static void merge_sentences(FILE *temp_fp, FILE *output_fp, int *sentences_merged) {
     int ch;
     // Keep track of the last significant (non-whitespace) character seen.
     // Initialize to a newline to correctly handle the first line of the file.
     char last_significant_char = '\n';
     
     // Rewind the temporary file pointer to read its contents from the beginning.
     rewind(temp_fp);
 
     // Process the temporary file character-by-character for fine-grained control.
     while ((ch = fgetc(temp_fp)) != EOF) {
         if (ch == '\n') {
             // When a newline is found, check the last significant character.
             // Sentences end with '.', '!', or '?'.
             if (last_significant_char == '.' || last_significant_char == '?' || last_significant_char == '!') {
                 // It's a true sentence end, so preserve the newline.
                 fputc('\n', output_fp);
             } else {
                 // It's a split sentence. Replace the newline with a single space to merge it.
                 // We check if the line was not blank to avoid miscounting merges.
                 if (last_significant_char != '\n') {
                     fputc(' ', output_fp);
                     (*sentences_merged)++;
                 } else {
                     // If the original line was blank, just write a newline to preserve it.
                     fputc('\n', output_fp);
                 }
             }
             // A line break, processed or not, resets the context.
             last_significant_char = '\n';
         } else {
             // Write all other characters directly to the output.
             fputc(ch, output_fp);
             // If the character is not whitespace, update our tracker.
             // Cast to unsigned char is important for portability with ctype functions.
             if (!isspace((unsigned char)ch)) {
                 last_significant_char = ch;
             }
         }
     }
 }