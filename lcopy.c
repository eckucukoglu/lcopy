#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Each chunk is 128KB, i.e, 128*1024 bytes. */
#define SIZE_OF_CHUNK 131072

/* Defined exceptions. */
typedef enum {
        OMITSRC, /* Omit source if it is directory and 
                no recursively copy option selected. */
        DESTNOTDIR, /* Multiple sources exists, however 
                destination is not a directory. */
        SRCNOTEXIST, /* Source does not exists. */
                
} exception;

/* String representation of defined exceptions. */
const char* exception_str[] = {
        "Omitting source directory.\n", 
        "Destination is not a directory.\n",
        "Source does not exists.\n"
}; 

/*
 * Generates target's digest file as <filename>.digs next to file.
 */
void generate_digest_file () {

}

/*
 * Checks file's digests with digest file. 
 * 
 * Returns 0 if exists and valid, -1 if not exists digest file,
 * else positive if exists but modification time is before the file.
 */
int check_digest_file () {
        
}

/*
 * Compares 2 chunks according to digest values.
 */
int compare_digests () {

}

/*
 * Compares 2 files according to digest files.
 * Returns different chunks' number in an array.
 */
int compare_digest_files () {

}

/*
 * Copies source file's chunk to destination file's chunk.
 */
int copy_chunk () {

}

/*
 * Copies source file to destination target.
 * Should be called if destination target does not exists.
 */
int copy_file_raw () {

}

/*
 * Copies source file to destination in a lazy way.
 */
int lazy_copy_file () {

}

/*
 * Prints usage information.
 */
void usage () {
        printf("Usage:\n"
               "\t[OPTIONS] SOURCE... DEST\n"
               "\tCopy SOURCE(s) to DEST.\n"
               "\nOptions:\n"
               "\t-R,-r   Recursively copy\n"
}

/*
 * Program main.
 */
int main (int argc, char *argv[]) {
        int ch;
        int rflag = 0; /* Recursively copy flag. */
        
        /* Missing arguments. */
        if (argc < 3) {
                usage();
                exit(EXIT_SUCCESS);
        }
        
        /* Parse command line options. */
        while ((ch = getopt(argc, argv, "Rr")) != -1) {
                switch (ch) {
                case 'R':
                case 'r':
                        rflag = 1;
                        break;
                default:
                        usage();
                        break;
                }
        }
        
        /* Missing destination target argument. */
        if (rflag && argc == 3) {
                usage();
                exit(EXIT_SUCCESS);
        }
        
        exit(EXIT_SUCCESS);
}
