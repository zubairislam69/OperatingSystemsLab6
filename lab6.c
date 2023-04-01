#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define PAGE_SIZE 256
#define NUM_PAGES 256
#define FRAME_SIZE 256
#define NUM_FRAMES 256
#define TLB_SIZE 16

int pageTable[NUM_PAGES];
signed char physicalMemory[NUM_FRAMES][FRAME_SIZE];
int tlbPageNumber[TLB_SIZE];
int tlbFrameNumber[TLB_SIZE];
int tlbCounter[TLB_SIZE];
int pageFault = 0;
int tlbHits = 0;
int numOfFrames = 0;

int calculatePhysicalAddress(int logicalAddress) {
	//initialize frame number and physical address
	int frameNumber;
    int physicalAddress;

	//right shift the logical address by 8 bits and bitwise and the address with 0xff
    int pageNumber = (logicalAddress >> 8) & 0xff;
    
    //bitwise and the address with 0xff
    int offset = logicalAddress & 0xff;

    // Check if the page number is in the TLB
    bool tlbHit = false;

    //for loop to check through the size of the TLB
    for (int i = 0; i < TLB_SIZE; i++) {
    
    	//if the page number is inside the tlb
    	//set frame number to the tlb's frame number at the index
        if (tlbPageNumber[i] == pageNumber) {
            frameNumber = tlbFrameNumber[i];      
            
            //increment the tlb counter
            tlbCounter[i]++;

            //set the tlb hit to true
            tlbHit = true;
            tlbHits++;

            //break from the loop
            break;
        }
    }

    // If the page number is not in the TLB, look it up in the page table
    if (!tlbHit) {

    	//set the frame number to the page number inside the page table
        frameNumber = pageTable[pageNumber];

        // If the page table entry is -1, there is a page fault
        //so increment the page fault 
        if (frameNumber == -1) {
            pageFault++;

            // Load the page from the backing store file
            FILE* backingStore = fopen("BACKING_STORE.bin", "rb");
            
            //if backing store cannot be opened, output an error message
            if (!backingStore) {
                printf("Error: could not open backing store\n");
                exit(1);
            }		

            //Seek for the appropriate position in the backing store
            fseek(backingStore, pageNumber * PAGE_SIZE, SEEK_SET);
            
		    //Read the page into the physical memory
            fread(physicalMemory[numOfFrames], sizeof(signed char), PAGE_SIZE, backingStore);

            //Update the page table to the number of frames
            pageTable[pageNumber] = numOfFrames;

            //update the frame number to the number of frames
            frameNumber = numOfFrames;

            //Increment the number of frames
            numOfFrames++;

            //if the number of frames is equal to the max declaration
            //set number of frames to 0
            if (numOfFrames == NUM_FRAMES) {
                numOfFrames = 0;
            }

            //Close the backing store file
            fclose(backingStore);
        }

        //initialize oldest to 0
        int oldestVal = 0;
        
		//for loop to count through the size of the tlb starting from first index
        for (int j = 1; j < TLB_SIZE; j++) {
        
        	//if the tlb counter at the index is less than the tlb counter at the oldest position
        	//set oldest to the tlb counter at the lower index
            if (tlbCounter[j] < tlbCounter[oldestVal]) {
                oldestVal = j;
            }

            //if the tlb page number is a -1
            //update the page number, frame number and tlb counter
            if (tlbPageNumber[j] == -1) {
                tlbPageNumber[j] = pageNumber;
                tlbFrameNumber[j] = frameNumber;
                tlbCounter[j] = 0;
                break;
            }
        }
        
        //if the oldest index of the tlb page number is not equal to -1
        //update the tlb page number, frame number and counter
        if (tlbPageNumber[oldestVal] != -1) {
            tlbPageNumber[oldestVal] = pageNumber;
            tlbFrameNumber[oldestVal] = frameNumber;
            tlbCounter[oldestVal] = 0;
        }
    }

    //Calculate the physical address using the page table
    physicalAddress = frameNumber * PAGE_SIZE + offset;
    return physicalAddress;
}

int main(int argc, char** argv) {

	//Initialize the page table to all -1's
	for (int i = 0; i < NUM_PAGES; i++) {
		pageTable[i] = -1;
	}

	//Initialize the TLB page number, frame number and counter to all -1's
	for (int i = 0; i < TLB_SIZE; i++) {
		tlbPageNumber[i] = -1;
		tlbFrameNumber[i] = -1;
		tlbCounter[i] = -1;
	}
	
	// Open the input file provided from the command line argument
	FILE* addressFile = fopen(argv[1], "r");
	
	//if argument not provided, print error and exit
	if (argv[1] == NULL) {
		printf("Error: provide a file argument\n");
		exit(1);
	}
	
	//if file does not exist, print error and exit
	if (!addressFile) {
		printf("Error: could not open input file\n");
		exit(1);
	}
	
	// Open the output file
	FILE* output = fopen("output.txt", "w");
	
	//if file cannot be opened, print error and exit
	if (!output) {
		printf("Error: could not open output file\n");
		exit(1);
	}

	//initialize variables to hold addresses and nums
	char buffer[10];
    int logicalAddress;
    int physicalAddress;
    int numOfTranslatedAddresses = 0;
	signed char value;
	

	// Read the logical addresses from the input file and translate them to physical addresses
	tlbHits -= 2;
	//get each address from address file
	while (fgets(buffer, 10, addressFile)) {
	
		//convert the address into an integer and store inside variable
	 	logicalAddress = atoi(buffer);
	 	
	 	//calculate the physical address and store inside variable
        physicalAddress = calculatePhysicalAddress(logicalAddress);
        
        //increment the number of translated addresses
        numOfTranslatedAddresses++;     
        
        //set the value  
		value = physicalMemory[physicalAddress / PAGE_SIZE][physicalAddress % PAGE_SIZE];
		
		//print
		fprintf(output, "Logical address: %d\t\t Physical address: %d\t\t value: %d\n", logicalAddress, physicalAddress, value);
	}

	//Close the input and output files
	fclose(addressFile);
	fclose(output);

	//Print the statistics
	printf("Number of Translated Addresses = %d\n", numOfTranslatedAddresses);
	printf("Page Faults = %d\n", pageFault);
	printf("Page Fault Rate = %.3f\n", (float) pageFault / numOfTranslatedAddresses);
	printf("TLB Hits = %d\n", tlbHits);
	printf("TLB Hit Rate = %.3f\n", (float) tlbHits / numOfTranslatedAddresses);

	return 0;
}
