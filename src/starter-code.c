#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
typedef struct Block {
    size_t start;
    size_t size;
    int free;//1=hole and 0= allocated
    char name[64];//name of the process
    struct Block *next;
} Block;

static Block *head = NULL;
static size_t MAX_MEM = 0;



static Block* new_block(size_t start, size_t size, int free_flag, const char *name){
    Block *b = (Block*)malloc(sizeof(Block));
    if (!b) return NULL;
    b->start = start;
    b->size  = size;
    b->free  = free_flag;
    b->next  = NULL;
    b->name[0] = '\0';
    if (name){
        strncpy(b->name,name,sizeof(b->name) - 1);
        b->name[sizeof(b->name) - 1] ='\0';
    }
    return b;  }


static void merging(Block *a) {
 //merge next into the current
            Block *b =a->next;
            a->size += b->size;
            a->next = b->next;
            free(b);
}

void printError(char *error){
        printf("ERROR: %s \n",error);
}



static int name_exists(const char *proc) {
    for(Block *cur = head; cur; cur = cur->next) {
        if(!cur->free && strcmp(cur->name, proc) ==0) return 1;
    }
    return 0;
}
static void lower_str(char *s) {
    for (; *s; s++) *s = (char)tolower((unsigned char)*s);
}

void allocate(char* processName, size_t size, char *type){

/* TODO:
Allocates memory from a hole to a process based on the algorithm chosen.
Type = 'F' or 'f' for first fit, 'B' or 'b' for best fit, 'W' or 'w' for worst fit.
Must support just a number (to represent bytes) or a number followed by Kb,Mb,Gb (ans all upper lower case variants) to represent kilobytes,megabytes and gigabytes.
*/

    if (!processName || !type) {
	printf("ERROR: invalid parameters\n");
        return;
    }
    if (size <= 0) {
	printf("ERROR: size must be more than 0\n");
        return;
    }
    size_t req=size;

    if(name_exists(processName)){
	printf("ERROR: process name exists\n");
        return;
    }

    char algo =(char)tolower((unsigned char)type[0]);
    if (!(algo == 'f' || algo == 'b' || algo == 'w')) {
	printf("ERROR: algo must be one of the 3\n");
        return;
    }

    Block *best= NULL;
    Block *best_prev= NULL;
    Block *prev =NULL ;

    for(Block *cur = head; cur; prev =cur, cur= cur->next){
        if (!cur->free) continue;
        if (cur->size < req) continue;

//first fit: first hole that works
        if (algo == 'f') {
            best = cur;
            best_prev = prev;
            break;
//best fit: smallest hole that fits
        } else if (algo == 'b') {
            if (!best || cur->size < best->size) {
                best =cur;
                best_prev= prev;
            }
//worse fit: largest hole that fits
        } else if (algo == 'w'){
            if (!best || cur->size > best->size) {
                best= cur;
                best_prev = prev;
            }
        }
    }

    if (!best) {
	printf("ERROR: no hole fits\n");
        return;
    }
//if they are exact size convert hole to process
    if(best->size == req){
        best->free = 0;
        strncpy(best->name, processName, sizeof(best->name) - 1);
        best->name[sizeof(best->name) - 1] = '\0';
        return;
    }
//split the hole into allocated and remaining hole
    Block *allocated = new_block(best->start, req, 0, processName);
    if (!allocated){
	printf("ERROR: failed malloc");

        return;
    }

    best->start += req;
    best->size  -= req;
//insert allocated before the best in the list
    if (best_prev == NULL) {
        allocated->next = head;
        head = allocated;
    } else {
        allocated->next = best_prev->next;
        best_prev->next = allocated;
    }
}


void deallocate(char* processName){

/* TODO:
Deallocates memory from a process to a hole.
If the hole is adjacent to another hole, the two holes should be merged(!).
*/
    if (!processName) {
printf("ERROR: invalid name for process\n");

        return;
    }

    Block *prev = NULL;
    Block *cur  = head;

    while (cur) {
        if (!cur->free && strcmp(cur->name, processName)== 0){
            cur->free = 1;
            cur->name[0] ='\0';
//merge with next if next is hole
            if (cur->next && cur->next->free){
                merging(cur);
            }
//merge with previ if its hole
            if (prev && prev->free){
                merging(prev);
            }

            return;
        }
        prev = cur;
        cur = cur->next;
    }
printf("ERROR:  process not found");
}


void status(){

/* TODO:
Print the status of the memory manager.
Includes each hole / process address, the amount of free memory, and the amount of allocated memory; ordered by starting memory addresses.
See the design document for the exact format of the output
*/
   	 for (Block *cur = head; cur; cur = cur->next){
        	size_t start = cur->start;
        	size_t end   = (cur->size == 0) ? cur->start : (cur->start + cur->size - 1);

       		 if (cur->free){
          		 printf("[0x%08zx - 0x%08zx] Unused\n",start,end) ;
       		 }else {
        		    printf("[0x%08zx - 0x%08zx] Process %s\n", start,end, cur->name);
        }
    }
}


void compact(){

/* TODO:
Compacts the memory allocations by shifting all current allocations to the top (lower numerical memory address value) and combining all holes into a single hole on the bottom.
*/
  	  size_t next_addr = 0;
  	  size_t used_total = 0;
//lets build a new list of allocated blocks in current order
  	  Block *new_head = NULL;
  	  Block *new_tail = NULL;

  	  for(Block *cur = head; cur; cur= cur->next) {
  	      if (cur->free) continue;
  	      Block *copy = new_block(next_addr, cur->size, 0, cur->name);
  	      if ( !copy){
  	          printError("compact: malloc failed");
  	          return;
  	      }

  	      next_addr += cur->size;
  	      used_total += cur->size;
	
        	if (!new_head) new_head = copy;
        	else new_tail->next = copy;
       			 new_tail = copy;
    }
//the one big hole with the remaining space
   		 if (used_total < MAX_MEM){
     		   Block *hole = new_block(next_addr, MAX_MEM - used_total, 1, NULL);
        	if (!hole) {
        	    printError("compact: malloc failed");
        	    return;
        }
        	if(!new_head){
        		    new_head = hole;
     		 }else{
      		      new_tail->next = hole;
        }
    }
//free the old list
   		 Block *tmp = head;
   		 while (tmp) {
        	Block *n = tmp->next;
       		 free(tmp);
       		 tmp = n;
    }

   		 head = new_head;
}

void init_func(size_t total_bytes){
//clean up any previous memory lists
 	   Block *cur =head;
	   while (cur){
	        Block *next =cur->next;
	        free(cur);
	        cur = next;
    }
//create one big hole for the start
	    head = (Block *)malloc(sizeof(Block));
	    if (!head) {
	        printf("ERROR:failed to initialize memory\n");
	        exit(1);
	    }

	    head->start = 0; //starting address 
	    head->size  = total_bytes;
	    head->free  = 1;
	    head->name[0] = '\0';
	    head->next  = NULL;
}


static long parse(const char *s) {
	    if (!s || !*s) return -1;

	    char buf[64];
	    snprintf(buf, sizeof(buf), "%s", s);
//make a copy in lowercase
	    for (size_t i = 0; buf[i]; i++) buf[i] = (char)tolower((unsigned char)buf[i]);
	    size_t n = strlen(buf);

//dealing with the suffixes
	    long mult = 1;
	    if(n>= 2 &&strcmp(buf+ (n-2),"kb")==0){
	        mult =1024L;
	        buf[n - 2]= '\0' ;

	    }else if(n >=2 && strcmp(buf +(n -2), "mb")== 0){

	        mult= 1024L * 1024L;
	        buf[n- 2] ='\0';
	    }else if(n>=2 && strcmp(buf + (n-2),"gb") ==0){
	        mult =1024L * 1024L* 1024L;

	        buf[n - 2]= '\0';
	       }

	    char *endp = NULL;
	    long val = strtol(buf, &endp, 10);
	    if (endp == buf || *endp != '\0' || val <= 0) return -1;

	    return val * mult;
	}


int main(int argc, char *argv[]) {
	/* TODO: fill the line below with your names and ids */
	printf(" Group Name: can-eltepe  \n Student(s) Name: Can Kadri Eltepe  \n Student(s) ID:0083855 \n");
    if (argc!= 2 && argc != 3){
        printError("Invalid number of arguments.\n");
        return 1;
    }
    long total =parse(argv[1]);
    if(total <= 0){
        printError("Invalid initial memory size");
        return 1;
    }

    MAX_MEM = (size_t)total;
    init_func(MAX_MEM);
    // initialize first hole       
//printf("HOLE INITIALIZED AT ADDRESS %d WITH %d BYTES\n",/* TODO*/, /* TODO*/);
//scrİpted mode
   if (argc ==3){
        FILE *fp = fopen(argv[2], "r");
        if (!fp){
            printError("Could'nt open the file.");
            return 1;
        }

        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            line[strcspn(line, "\r\n")] = '\0';
//removes newline
//ignoring empty lines
        if (line[0] == '\0') continue;


        char *arguments[4];//because RQ needs 4 arguments
        int tokenCount = 0;

        char *token = strtok(line, " \t");
        while(token != NULL && tokenCount <4){
        arguments[tokenCount] = token;
        token = strtok(NULL, " \t");
	tokenCount++;

            }
       if (tokenCount == 0) continue;
//make case sensitive
       lower_str(arguments[0]);



       if(strcmp(arguments[0], "rq") == 0){
                        
            if(tokenCount==4 ){
                long bytes =parse(arguments[2]);
                if (bytes <=0){
                    printError("Invalid size.");
                    continue;
                }
                if ((size_t)bytes> MAX_MEM){
                    printError("Requested size too large.");
                    continue;
                }
                allocate(arguments[1], (int)bytes, arguments[3]);
            }
            else{
                printError("Expected expression: RQ \"process name\" \"Bytes\" \"Algorithm\".");
            }
        }
        // RL (Release Memory / Deallocate): Needs 2 arguments and must check if they are valid arguments
        else if(strcmp(arguments[0], "rl") == 0){

            if(tokenCount==2){
               deallocate(arguments[1]);
            }
            else{
                printError("Expected expression: RL \"process name\".");
            }
        }
        // STATUS: Needs 1 argument
        else if(strcmp(arguments[0], "stat") == 0){
		continue;
        }
        // C (Compact): Needs 1 argument
        else if(strcmp(arguments[0], "c") == 0){

            if(tokenCount==1){
                compact();
                        }
            else{
                printError("Expected expression: C.");
            }
        }
        // EXIT: Needs 1 argument
        else if(strcmp(arguments[0], "x") == 0){
            if(tokenCount == 1){
		status();
                exit(0);
            }
            else printError("Expected expression: EXIT without any parameters.");
        }
        // If command is not recognized, print error message and continue
        else printError("Invalid command.");        
    }
        fclose(fp);
        status();
return 0;
}


//İnteractve mode 
   if (argc == 2){
    while(1){
        char input[256];
        printf("allocator>");
        if(!fgets(input, sizeof(input),stdin)){
            printf("\n");
            break;
        }
        input[strcspn(input, "\r\n")] = '\0'; // remove newline from input, replace with EOL

		// empty input = do nothing 
        if(input[0] == '\0') continue; 

		
        char *arguments[4];
        char *token = strtok(input, " \t ");
        int tokenCount = 0;

		
        // get all arguments from input
        while(token != NULL && tokenCount <4){
            arguments[tokenCount] = token;
            token = strtok(NULL, " \t");
            tokenCount++;
        }
	if (tokenCount == 0) continue;
		// TODO: make commands case insensitive, i.e. should accept rq,RQ,rl,RL,stat,STAT,c,C,exit,EXIT
        lower_str(arguments[0]);
        // RQ (Request Memory / allocate): Needs 4 arguments and must check if they are valid arguments
        if(strcmp(arguments[0], "rq") == 0){
			
            if(tokenCount==4 ){
                long bytes =parse(arguments[2]);
                if (bytes <=0){
                    printError("Invalid size.");
                    continue;
                }
                if ((size_t)bytes> MAX_MEM){
                    printError("Requested size too large.");
                    continue;
                }
                allocate(arguments[1], (int)bytes, arguments[3]);
            }
            else{
                printError("Expected expression: RQ \"process name\" \"Bytes\" \"Algorithm\".");
            }			
        }
        // RL (Release Memory / Deallocate): Needs 2 arguments and must check if they are valid arguments
        else if(strcmp(arguments[0], "rl") == 0){

            if(tokenCount==2){
               deallocate(arguments[1]);
            }
            else{
                printError("Expected expression: RL \"process name\".");
            }
        }
        // STATUS: Needs 1 argument
        else if(strcmp(arguments[0], "stat") == 0){

            if(tokenCount==1){
                status();
            }
            else{
                printError("Expected expression: STATUS.");
            }
        }
        // C (Compact): Needs 1 argument
        else if(strcmp(arguments[0], "c") == 0){

            if(tokenCount==1){
      		compact();
			}
            else{
                printError("Expected expression: C.");
            }
        }
        // EXIT: Needs 1 argument
        else if(strcmp(arguments[0], "x") == 0){
            if(tokenCount == 1){
                printf("Exiting program.\n");
                exit(0);
            }
            else printError("Expected expression: EXIT without any parameters.");
        }
        // If command is not recognized, print error message and continue
        else printError("Invalid command.");        
    }
return 0;
}
}

