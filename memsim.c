
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef struct {
        int pageNo;
        int modified;
		int last_used;
		int reference;
} page;


enum    repl { ran, fifo, lru, clock};
// int     createMMU( int);
int     checkInMemory( int ) ;
int     allocateFrame( int ) ;
page    selectVictim( int, enum repl) ;
const   int pageoffset = 12;            /* Page size is fixed to 4 KB */
int     numFrames, pageNo, disk_reads, disk_writes ;




page *page_table;
int zero = 0;
int time = 0; // incrementing the time for each page in a LRU algorithm
int clock_hand = 0 ; // where the clock is currently


/* Creates the page table structure to record memory allocation */
int     createMMU (int frames)
{
		// 
		numFrames = frames;

		page_table = (page *)malloc(sizeof(page)*numFrames);  // allocating pages in memory
        // to do
		for (int i = 0; i < frames; i++){ // creating n number of pages and setting initial values

			page_table[i].pageNo = -1; // pageNo should be -1 so that we know if a table entry is empty
			page_table[i].reference = 1; // setting reference bit for clock replacement

			page_table[i].modified = 0;
			page_table[i].last_used = 0;
		}


        return 0;
}

/* Checks for residency: returns frame number,  or -1 if not found */
int     checkInMemory( int page_number)
{
        
		for (int i = 0; i < numFrames; i++){ // iterating across all pages and checking if the page is in the table
			if ( page_table[i].pageNo == page_number){ // if page number is found in table we return its entry key
				return i;
			}

		}
        return -1 ; // page was not found in table return -1 to indicate page fault
}

/* allocate page to the next free frame and record where it put it */
int     allocateFrame( int page_number)
{	
	
		for (int i = 0; i < numFrames; i++){
			if (page_table[i].pageNo == -1){ // if the current page entry is empty
				// we allocate the frame to the entry
				page_table[i].pageNo = page_number;
				pageNo++; // incrementing the number of pages
				disk_reads++;
				return i;
			}
			
			
		}
		return -1; // if there are no empty frames we return a fault
}

/* Selects a victim for eviction/discard according to the replacement algorithm,  returns chosen frame_no  */
page    selectVictim(int page_number, enum repl  mode )
{
		page    victim; // this is the page to evict
        // to do 
		disk_reads++;
		int victim_index;

		switch (mode){
			case clock: {

				while(page_table[clock_hand].reference == 1){
					// iterating until we find the last page with a reference = 1
					page_table[clock_hand].reference = 0;

					clock_hand++; //  incrementing the clock hand position
					if (clock_hand == numFrames){ clock_hand = 0;}
					// resetting the clock position to the first entry in the page table
						
				}
				// the victim is the last page entry with a reference bit = 1
				victim_index = clock_hand;
				victim = page_table[clock_hand];
				// replacing the victim page
				page_table[victim_index].pageNo = page_number;
				page_table[victim_index].modified = 0;
				page_table[victim_index].last_used = 0;
				page_table[victim_index].reference = 1;

				clock_hand++; // incrementing the hand position otherwise it would stay on the victim
				if (clock_hand == numFrames){clock_hand = 0;}
				return victim;
				}

			case lru: {
				// setting the inital victim
				victim = page_table[0]; 
				victim_index = 0;

				for (int i = 0; i < numFrames; i++){ 
					// iterating across all pages and finding the least recently used
					if (victim.last_used > page_table[i].last_used){
						// if the current page is lesser used than the current victim we replace the current victim
						victim = page_table[i];
						victim_index = i;
					}


				}

				// replacing the least recently used page with the page number we want
				
				break;
			}
			case ran: {
				victim_index = rand() % pageNo;
			

				// need to replace with new page
				victim = page_table[victim_index];


			break;
	
			}
			case fifo: {
				// FIFO implementation
				int fifo_index = 0; // setting index to zero
				victim_index = fifo_index; // always same
				fifo_index = (fifo_index + 1) % pageNo;
				break;
			}


		}


		page_table[victim_index].pageNo = page_number;
		// setting all bits to zero
		page_table[victim_index].modified = 0;
		page_table[victim_index].last_used = 0;
		page_table[victim_index].reference = 0;
		return (victim) ;
}

/*This updates the page table entry for read operations (incrementing the time counter and setting reference bit)*/
void read_incrementor(int page_number, int n){
	for (int i = 0; i < n; i++){
		if (page_table[i].pageNo == page_number){ // if we are at the page being read
		
			// update the last used to the current time, for LRU replacement
			page_table[i].last_used = time;
			time++;

			page_table[i].reference = 1; // set reference bit for clock replacement
		}
	}
}

/*Updates the page table entry for write operations*/
void write_incrementor(int page_number, int n){
	for (int i = 0; i < n; i++){ // iterate across all pages till we get to the page being written
		if (page_table[i].pageNo == page_number){ // if we are at the wanted page
			// we update the time counter and set the modify and reference bit
			page_table[i].last_used = time; // updating time for LRU
			time++;

			page_table[i].modified = 1; // marking page as being modified
			page_table[i].reference = 1; // updating reference for Clock
			
		}
	}
}



		
int main(int argc, char *argv[])
{
  
	char	*tracename;
	int	page_number,frame_no, done ;
	int	do_line, i;
	int	no_events, disk_writes, disk_reads;
	int     debugmode;
 	enum	repl  replace;
	int		pages_allocated; 
	int		victim_page;
	unsigned address;
	char 	rw;
	page	Pvictim;
	FILE	*trace;


        if (argc < 5) {
             printf( "Usage: ./memsim inputfile numberframes replacementmode debugmode \n");
             exit ( -1);
	}
	else {
        tracename = argv[1];	
		trace = fopen( tracename, "r");
	if (trace == NULL ) {
             printf( "Cannot open trace file %s \n", tracename);
             exit ( -1);
	}
	numFrames = atoi(argv[2]);
        if (numFrames < 1) {
            printf( "Frame number must be at least 1\n");
            exit ( -1);
        }

        if (strcmp(argv[3], "lru\0") == 0)
            replace = lru;
	    else if (strcmp(argv[3], "rand\0") == 0)
	     	replace = ran;
		else if (strcmp(argv[3], "clock\0") == 0)
			replace = clock;		 
		else if (strcmp(argv[3], "fifo\0") == 0)
			replace = fifo;		 
        else {
			printf( "Replacement algorithm must be rand/fifo/lru/clock  \n");
			exit ( -1);
	  		}

        if (strcmp(argv[4], "quiet\0") == 0)
            debugmode = 0;
		else if (strcmp(argv[4], "debug\0") == 0)
            debugmode = 1;
        else {
			printf( "Replacement algorithm must be quiet/debug  \n");
			exit ( -1);
	  		}
	}
	
	done = createMMU (numFrames);
	if ( done == -1 ) {
		printf( "Cannot create MMU" ) ;
		exit(-1);
	}

	no_events = 0 ;
	disk_writes = 0 ;
	disk_reads = 0 ;
	pages_allocated = 0;
	do_line = fscanf(trace,"%x %c",&address,&rw);

	while ( do_line == 2)
	{
		page_number =  address >> pageoffset;
		frame_no = checkInMemory( page_number) ;    /* ask for physical address */

		if ( frame_no == -1 )
		{
		  disk_reads++ ;			/* Page fault, need to load it into memory */
		  if (debugmode) 
		      printf( "Page fault %8d \n", page_number) ;
		  if (pages_allocated < numFrames)  			/* allocate it to an empty frame */
			{
			frame_no = allocateFrame(page_number);
			pages_allocated++;
			}
            else {

				Pvictim = selectVictim(page_number, replace) ;   /* returns page number of the victim  */
				frame_no = checkInMemory( page_number) ;    /* find out the frame the new page is in */
				if (Pvictim.modified)           /* need to know victim page and modified  */
					{
						disk_writes++;			    
						if (debugmode) printf( "Disk write %8d \n", Pvictim.pageNo) ;
					}
				else {
					if (debugmode) printf( "Discard    %8d \n", Pvictim.pageNo) ;
				}
		   }
		}



		if ( rw == 'R'){
			// if its a read operation we need to update the time counter for LRU
			// and reference bit for clock
			read_incrementor(page_number, pages_allocated);
		    if (debugmode) printf( "reading    %8d \n", page_number) ;
			
		}
		else if ( rw == 'W'){
			// if its a write operation we need to set the modify, bits
			// and update the time counter (for LRU) and reference bit (for clock)
			write_incrementor(page_number, pages_allocated); 
		    if (debugmode) printf( "writing   %8d \n", page_number) ;
			
		}
		 else {
		      printf( "Badly formatted file. Error on line %d\n", no_events+1); 
		      exit (-1);
		}

		no_events++;
		do_line = fscanf(trace,"%x %c",&address,&rw);
	}

	// printf( "total memory frames:  %d\n", numFrames);
	// printf( "events in trace:      %d\n", no_events);
	// printf( "total disk reads:     %d\n", disk_reads);
	// printf( "total disk writes:    %d\n", disk_writes);
	printf( "page fault rate:\t%.4f\n", (float) disk_reads/no_events);
	return 0;
}
				
