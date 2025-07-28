/**
 * @file transcript_cleaner.c
 * @brief A high-performance C program for cleaning transcript files on Windows.
 *
 * This program performs a two-pass cleanup on a given text file:
 * 1. Removes consecutive duplicate lines.
 * 2. Merges sentences that are split across multiple lines.
 *
 * It is designed to be memory-efficient, processing large files without
 * loading them entirely into memory.
 *
 * Compilation (MSVC): cl /W4 /O2 /GS transcript_cleaner.c
 * Compilation (MinGW): gcc -Wall -O2 -o transcript_cleaner.exe transcript_cleaner.c -luser32
 *
 * Usage:
 *   transcript_cleaner.exe -i <input.txt> -o <output.txt>
 *   transcript_cleaner.exe -i <input.txt>
 *   transcript_cleaner.exe
 *
 * File Handling Logic:
 * - Input:
 *   - Use '-i <path>' to specify the input file.
 *   - If '-i' is omitted, the program searches the current directory. If exactly
 *     one .txt file is found, it's used as the input. Otherwise, it's an error.
 * - Output:
 *   - Use '-o <path>' to specify the output file.
 *   - If '-o' is omitted, the output path is derived from the input file path
 *     by appending "_resolved.txt".
 *
 * Crucial Implementation Detail:
 * This program uses the Windows API (<windows.h>) for directory scanning to
 * ensure full compatibility with the Windows environment, avoiding POSIX-specific
 * headers and functions. It uses wide-character (Unicode) APIs for robust
 * handling of file paths.
 */

 #define _CRT_SECURE_NO_WARNINGS // Required by some compilers for `_wcsdup`
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <wchar.h>   // For wide-character functions
 #include <windows.h> // For Windows API file searching
 #include <wctype.h>  // For iswspace
 
 // --- Constants ---
 #define MAX_LINE_LENGTH 8192 // Generous buffer for a single line of a transcript
 #define MAX_PATH_LENGTH MAX_PATH // From Windows headers, typically 260
 #define OUTPUT_SUFFIX L"_resolved.txt"
 
 // --- Function Prototypes ---
 static int find_single_txt_file(wchar_t **filename);
 static void print_usage(const wchar_t *program_name);
 
 /**
  * @brief Main entry point of the program.
  *
  * Uses wmain to handle Unicode command-line arguments, which is the
  * standard for modern Windows applications dealing with file paths.
  */
 int wmain(int argc, wchar_t *argv[]) {
     // --- Variable Declarations ---
     wchar_t *input_filepath = NULL;
     wchar_t *output_filepath = NULL;
     wchar_t *allocated_input_path = NULL;
     wchar_t *allocated_output_path = NULL;
     
     FILE *in_file = NULL;
     FILE *out_file = NULL;
     FILE *tmp_file = NULL;
 
     // Statistics counters
     int duplicate_lines_removed = 0;
     int sentences_merged = 0;
 
     // --- 1. Argument Parsing ---
     // A simple loop to parse -i and -o flags.
     for (int i = 1; i < argc; ++i) {
         if (wcscmp(argv[i], L"-i") == 0 && i + 1 < argc) {
             input_filepath = argv[++i];
         } else if (wcscmp(argv[i], L"-o") == 0 && i + 1 < argc) {
             output_filepath = argv[++i];
         } else {
             fwprintf(stderr, L"Error: Invalid argument '%ls'.\n", argv[i]);
             print_usage(argv[0]);
             return EXIT_FAILURE;
         }
     }
 
     // --- 2. Determine Input File ---
     if (input_filepath == NULL) {
         // If -i is not provided, scan the current directory for a single .txt file.
         fwprintf(stdout, L"No input file specified. Searching for a single .txt file in current directory...\n");
         if (!find_single_txt_file(&allocated_input_path)) {
             // Error message is printed inside the function.
             return EXIT_FAILURE;
         }
         input_filepath = allocated_input_path;
         fwprintf(stdout, L"Found input file: %ls\n", input_filepath);
     }
 
     // --- 3. Determine Output File ---
     if (output_filepath == NULL) {
         // If -o is not provided, generate the output filename from the input.
         size_t len = wcslen(input_filepath) + wcslen(OUTPUT_SUFFIX) + 1;
         allocated_output_path = (wchar_t *)malloc(len * sizeof(wchar_t));
         if (!allocated_output_path) {
             fwprintf(stderr, L"Error: Failed to allocate memory for output path.\n");
             free(allocated_input_path);
             return EXIT_FAILURE;
         }
         wcscpy_s(allocated_output_path, len, input_filepath);
         
         // Remove .txt extension if it exists before appending suffix
         wchar_t *ext = wcsrchr(allocated_output_path, L'.');
         if (ext != NULL && _wcsicmp(ext, L".txt") == 0) {
             *ext = L'\0';
         }
 
         wcscat_s(allocated_output_path, len, OUTPUT_SUFFIX);
         output_filepath = allocated_output_path;
     }
     
     fwprintf(stdout, L"Processing '%ls' -> '%ls'\n", input_filepath, output_filepath);
 
     // --- 4. Pass 1: Remove Consecutive Duplicate Lines ---
     // This pass reads the input file and writes a de-duplicated version to a
     // temporary file, avoiding high memory usage.
     
     // Open the input file
     if (_wfopen_s(&in_file, input_filepath, L"r, ccs=UTF-8") != 0 || in_file == NULL) {
         fwprintf(stderr, L"Error: Cannot open input file '%ls'.\n", input_filepath);
         goto cleanup;
     }
 
     // Create a temporary file for intermediate results.
     // Using tmpfile_s is safe as it's automatically deleted on fclose.
     if (tmpfile_s(&tmp_file) != 0 || tmp_file == NULL) {
         fwprintf(stderr, L"Error: Cannot create temporary file.\n");
         goto cleanup;
     }
 
     wchar_t prev_line[MAX_LINE_LENGTH] = {0};
     wchar_t curr_line[MAX_LINE_LENGTH] = {0};
 
     // Read the first line to initialize the comparison buffer.
     if (fgetws(prev_line, MAX_LINE_LENGTH, in_file)) {
         fputws(prev_line, tmp_file);
 
         // Process the rest of the file
         while (fgetws(curr_line, MAX_LINE_LENGTH, in_file)) {
             if (wcscmp(curr_line, prev_line) != 0) {
                 // Lines are different, write the new line and update prev_line
                 fputws(curr_line, tmp_file);
                 wcscpy_s(prev_line, MAX_LINE_LENGTH, curr_line);
             } else {
                 // Lines are identical, increment counter
                 duplicate_lines_removed++;
             }
         }
     }
     fclose(in_file);
     in_file = NULL; // Mark as closed
 
     // --- 5. Pass 2: Merge Split Sentences ---
     // This pass reads the de-duplicated temporary file character-by-character
     // and writes the final, sentence-merged version to the output file.
 
     rewind(tmp_file); // Rewind temporary file to read from the beginning.
 
     // Open the final output file
     if (_wfopen_s(&out_file, output_filepath, L"w, ccs=UTF-8") != 0 || out_file == NULL) {
         fwprintf(stderr, L"Error: Cannot open output file '%ls'.\n", output_filepath);
         goto cleanup;
     }
 
     wint_t ch;
     wchar_t last_non_whitespace = L' '; // Used to check for sentence-ending punctuation
 
     while ((ch = fgetwc(tmp_file)) != WEOF) {
         if (ch == L'\n') {
             // When a newline is found, check the last significant character.
             if (last_non_whitespace == L'.' || last_non_whitespace == L'?' || last_non_whitespace == L'!') {
                 // It was a sentence end, so keep the newline.
                 fputwc(L'\n', out_file);
             } else {
                 // Not a sentence end, replace newline with a space to merge.
                 fputwc(L' ', out_file);
                 sentences_merged++;
             }
             // Reset for the new line.
             last_non_whitespace = L' ';
         } else {
             fputwc(ch, out_file);
             // Keep track of the last character that wasn't whitespace.
             if (!iswspace(ch)) {
                 last_non_whitespace = ch;
             }
         }
     }
 
     // --- 6. Final Reporting ---
     wprintf(L"\nProcessing complete.\n");
     wprintf(L"- Duplicate lines removed: %d\n", duplicate_lines_removed);
     wprintf(L"- Sentences merged: %d\n", sentences_merged);
 
 cleanup:
     // --- 7. Cleanup ---
     // Close all file handles and free all allocated memory.
     if (in_file) fclose(in_file);
     if (out_file) fclose(out_file);
     if (tmp_file) fclose(tmp_file); // tmpfile_s handles deletion automatically
 
     free(allocated_input_path);
     free(allocated_output_path);
 
     return EXIT_SUCCESS;
 }
 
 /**
  * @brief Scans the current directory for a single file ending in ".txt".
  *
  * This function uses the Windows API (FindFirstFileW, FindNextFileW) to perform
  * the directory scan. It is specifically designed to meet the requirement of
  * not using POSIX-like directory functions.
  *
  * @param[out] filename A pointer to a wchar_t pointer. If a single .txt file
  *                      is found, this function allocates memory for its name
  *                      and assigns it to *filename. The caller must free this.
  * @return 1 on success (found exactly one .txt file), 0 on failure.
  */
 static int find_single_txt_file(wchar_t **filename) {
     WIN32_FIND_DATAW find_data;
     HANDLE h_find = INVALID_HANDLE_VALUE;
     int file_count = 0;
     wchar_t found_filename[MAX_PATH_LENGTH] = {0};
 
     // Start searching for files matching "*.txt" in the current directory.
     // The 'W' functions (e.g., FindFirstFileW) are for wide characters (Unicode).
     h_find = FindFirstFileW(L"*.txt", &find_data);
 
     if (h_find == INVALID_HANDLE_VALUE) {
         fwprintf(stderr, L"Error: No .txt files found in the current directory.\n");
         return 0;
     }
 
     do {
         // Ignore directories to avoid counting "." and ".." or other subdirs
         if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
             file_count++;
             if (file_count == 1) {
                 // Store the name of the first text file found
                 wcscpy_s(found_filename, MAX_PATH_LENGTH, find_data.cFileName);
             }
         }
     } while (FindNextFileW(h_find, &find_data) != 0);
 
     FindClose(h_find); // Always close the find handle.
 
     if (file_count == 1) {
         // Exactly one file found, allocate memory and return its name.
         *filename = _wcsdup(found_filename);
         if (!*filename) {
             fwprintf(stderr, L"Error: Memory allocation failed for filename.\n");
             return 0;
         }
         return 1;
     } else if (file_count == 0) {
         // This case should have been caught by INVALID_HANDLE_VALUE, but is here for completeness.
         fwprintf(stderr, L"Error: No .txt files found in the current directory.\n");
         return 0;
     } else {
         fwprintf(stderr, L"Error: Found %d .txt files. Please specify one using the -i flag.\n", file_count);
         return 0;
     }
 }
 
 /**
  * @brief Prints the program's usage instructions to stdout.
  * @param program_name The name of the executable (argv[0]).
  */
 static void print_usage(const wchar_t *program_name) {
     wprintf(L"\nUsage: %ls [-i inputfile] [-o outputfile]\n\n", program_name);
     wprintf(L"  -i <path>   Specify the path to the input transcript file.\n");
     wprintf(L"              If omitted, searches for a single .txt file in the current directory.\n");
     wprintf(L"  -o <path>   Specify the path for the cleaned output file.\n");
     wprintf(L"              If omitted, creates a file named '[input]_resolved.txt'.\n");
 }