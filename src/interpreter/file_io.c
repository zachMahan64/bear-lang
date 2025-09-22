#include "interpreter/file_io.h"
#include <stdio.h>
bool file_exist(const char* file_name) {
    FILE* file = fopen(file_name, "r");
    if (file) {
        printf("[DEBUG] File '%s' exists.\n", file_name);
        fclose(file); // Close the file if it was opened
        return true;
    }
    printf("[ERROR] File '%s' does not exist.\n", file_name);
    return false;
}
