#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define PAGE_SIZE 256
#define NUM_PAGES 256
#define FRAME_SIZE 256
#define NUM_FRAMES 256
#define TLB_SIZE 16

int page_table[NUM_PAGES];
signed char physical_memory[NUM_FRAMES][FRAME_SIZE];
int tlb_page_number[TLB_SIZE];
int tlb_frame_number[TLB_SIZE];
int tlb_counter[TLB_SIZE];
int page_faults = 0;
int tlb_hits = 0;
int num_frames = 0;

int get_page_number(int logical_address) {
    return (logical_address >> 8) & 0xff;
}

int get_offset(int logical_address) {
    return logical_address & 0xff;
}

int get_physical_address(int logical_address) {
    int page_number = get_page_number(logical_address);
    int offset = get_offset(logical_address);
    int frame_number;
    int physical_address;

    // Check if the page number is in the TLB
    bool tlb_hit = false;
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb_page_number[i] == page_number) {
            frame_number = tlb_frame_number[i];
            tlb_counter[i]++;
            tlb_hit = true;
            break;
        }
    }

    // If the page number is not in the TLB, look it up in the page table
    if (!tlb_hit) {
        frame_number = page_table[page_number];

        // If the page table entry is -1, there is a page fault
        if (frame_number == -1) {
            page_faults++;

            // Load the page from the backing store
            FILE* backing_store = fopen("BACKING_STORE.bin", "rb");
            if (!backing_store) {
                printf("Error: could not open backing store\n");
                exit(1);
            }

            // Seek to the appropriate position in the backing store
            fseek(backing_store, page_number * PAGE_SIZE, SEEK_SET);

            // Read the page into the physical memory
            fread(physical_memory[num_frames], sizeof(signed char), PAGE_SIZE, backing_store);

            // Update the page table and frame number
            page_table[page_number] = num_frames;
            frame_number = num_frames;

            // Increment the frame number (circular buffer)
            num_frames++;
            if (num_frames == NUM_FRAMES) {
                num_frames = 0;
            }

            // Close the backing store
            fclose(backing_store);
        }

        // Update the TLB with the new mapping and counter
        int oldest = 0;
        for (int j = 1; j < TLB_SIZE; j++) {
            if (tlb_counter[j] < tlb_counter[oldest]) {
                oldest = j;
            }
            if (tlb_page_number[j] == -1) {
                tlb_page_number[j] = page_number;
                tlb_frame_number[j] = frame_number;
                tlb_counter[j] = 0;
                break;
            }
        }
        if (tlb_page_number[oldest] != -1) {
            tlb_page_number[oldest] = page_number;
            tlb_frame_number[oldest] = frame_number;
            tlb_counter[oldest] = 0;
        }
    }

    // Calculate the physical address using the page table
    physical_address = frame_number * PAGE_SIZE + offset;

    return physical_address;
}

int main(int argc, char** argv) {
// Initialize the page table to all -1's
	for (int i = 0; i < NUM_PAGES; i++) {
		page_table[i] = -1;
	}
	
	// Initialize the TLB to all -1's
for (int i = 0; i < TLB_SIZE; i++) {
    tlb_page_number[i] = -1;
    tlb_frame_number[i] = -1;
    tlb_counter[i] = -1;
}

// Open the input file
FILE* addresses = fopen("addresses.txt", "r");
if (!addresses) {
    printf("Error: could not open input file\n");
    exit(1);
}

// Open the output file
FILE* output = fopen("output.txt", "w");
if (!output) {
    printf("Error: could not open output file\n");
    exit(1);
}

// Read the logical addresses from the input file and translate them to physical addresses
char buffer[10];
while (fgets(buffer, 10, addresses)) {
    int logical_address = atoi(buffer);
    int physical_address = get_physical_address(logical_address);
    signed char value = physical_memory[physical_address / PAGE_SIZE][physical_address % PAGE_SIZE];
    fprintf(output, "logical address: %d physical address: %d value: %d\n", logical_address, physical_address, value);
}

// Close the input and output files
fclose(addresses);
fclose(output);

// Print the statistics
printf("Number of Translated Addresses = %d\n", page_faults + tlb_hits);
printf("Page Faults = %d\n", page_faults);
printf("Page Fault Rate = %.2f\n", (float) page_faults / (page_faults + tlb_hits));
printf("TLB Hits = %d\n", tlb_hits);
printf("TLB Hit Rate = %.2f\n", (float) tlb_hits / (page_faults + tlb_hits));

return 0;
}
	
