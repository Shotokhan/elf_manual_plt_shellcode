#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>


uintptr_t get_libc_base_address() {
    FILE *maps_file = fopen("/proc/self/maps", "r");
    if (maps_file == NULL) {
        perror("Error opening /proc/self/maps");
        exit(1);
    }
    uintptr_t libc_base_address = 0;
    char line[256];
    while (fgets(line, sizeof(line), maps_file)) {
        // Look for the line containing libc
        if (strstr(line, "/libc")) {
            // Extract the start address from the line
            sscanf(line, "%lx", &libc_base_address);
            break;
        }
    }
    fclose(maps_file);
    return libc_base_address;
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <path_to_compiled_shellcode>\n", argv[0]);
        exit(1);
    }
    const char *filename = argv[1];
    FILE *file = fopen(filename, "rb");

    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Determine the size of the file
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the file content
    unsigned char *buffer = (unsigned char*)mmap(NULL, file_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (buffer == MAP_FAILED) {
        perror("Error allocating memory");
        fclose(file);
        return 1;
    }

    // Read the file content into the buffer
    size_t bytes_read = fread(buffer, 1, file_size, file);
    fclose(file);

    if (bytes_read != file_size) {
        perror("Error reading file");
        return 1;
    }

    uintptr_t libc_base_address = get_libc_base_address();
    if (libc_base_address == 0) {
        perror("Error: could not get libc base address");
        return 1;
    }

    // Search for the pattern 0xdeadbeefcafebabe in the buffer (little endian)
    const unsigned char pattern[] = {0xbe, 0xba, 0xfe, 0xca, 0xef, 0xbe, 0xad, 0xde};
    for (size_t i = 0; i < file_size - sizeof(pattern) + 1; ++i) {
        if (memcmp(buffer + i, pattern, sizeof(pattern)) == 0) {
            // Replace the pattern with the base address of libc
            memcpy(buffer + i, &libc_base_address, sizeof(libc_base_address));
            // printf("Pattern found and replaced at offset %zu\n", i);
            break; 
        }
    }

    // Call the buffer
    (*(void (*)()) buffer)();
}
