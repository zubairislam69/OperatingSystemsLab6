#include <stdio.h>

#define PAGE_SIZE 28 // Define the page size
#define PAGE_TABLE_SIZE 28 // Define the size of the page table
#define TLB_SIZE 16 // Define the size of the TLB

int main() {
    int logical_addresses[] = {1, 256, 32768, 32769, 128, 65534, 33153}; // Define the integer numbers
    
    // Initialize the page table with random data
    int page_table[PAGE_TABLE_SIZE];
    for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
        page_table[i] = i * PAGE_SIZE;
    }
    
    // Initialize the TLB with invalid page numbers
    int tlb[TLB_SIZE][2];
    for (int i = 0; i < TLB_SIZE; i++) {
        tlb[i][0] = -1;
    }
    
    // Initialize the counter for LRU policy
    int counter = 0;
    
    // Loop through each number
    for (int i = 0; i < sizeof(logical_addresses) / sizeof(logical_addresses[0]); i++) {
       // int logical_address = numbers[i];
        int page_number = logical_addresses[i] >> 8; // Shift right by 8 bits to get the upper 8 bits (page number)
        int offset = logical_addresses[i] & 0xFF; // Mask the lower 8 bits to get the offset
           
        int physical_address;
        
        // Check the TLB for the page number
        int tlb_hit = 0;
        for (int j = 0; j < TLB_SIZE; j++) {
            if (tlb[j][0] == page_number) {
                // TLB hit
                physical_address = tlb[j][1] + offset;
                // Update the counter for LRU policy
                tlb[j][1] = counter++;
                tlb_hit = 1;
                break;
            }
        }
        
        if (!tlb_hit) { 
       		// TLB miss, consult the page table
		    // Check if the page number is valid
		    if (page_number >= PAGE_TABLE_SIZE) {
		        printf("Logical address: %d\tPage number is out of range\n", logical_addresses[i]);
		        continue;
		    }      
        	// Calculate the physical address using the page table
        	physical_address = page_table[page_number] + offset;
          	
          	// Update the TLB with the new mapping and counter
            int oldest = 0;
            for (int j = 1; j < TLB_SIZE; j++) {
                if (tlb[j][1] < tlb[oldest][1]) {
                    oldest = j;
                }
            }
            tlb[oldest][0] = page_number;
            tlb[oldest][1] = counter++;
        }
        
        // Print the page number and offset
        //printf("Logical address: %d\tPage Number: %d\tOffset: %d\n", number, page_number, offset);
        printf("Logical address: %d,\t Physical address: %d\n", logical_addresses[i], physical_address);
        
       
        //printf("Physical address: %d\n", physical_address);
    }
    
    return 0;
}
