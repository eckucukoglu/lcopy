#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

/* Each chunk is 128KB, i.e, 128*1024 bytes. */
#define SIZE_OF_CHUNK 131072

#define handle_error_en(en, msg) \
        do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0);
        
#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0);

/* Defined exceptions. */
typedef enum {
        OMITSRC, /* Omit source if it is directory and 
                no recursively copy option selected. */
        DESTNOTDIR, /* Multiple sources exists, however 
                destination is not a directory. */
        SRCNOTEXIST, /* Source does not exists. */
} Exception;

/* String representation of defined exceptions. */
const char* exception_str[] = {
        "Omitting source directory", 
        "Destination is not a directory",
        "Source does not exists"
}; 

/* Global program exception. */
Exception exception;

/* File existence check, returns non-zero if path exists. */
int is_file_exist(const char *path) {
        struct stat info;
        if (stat(path, &info) != 0) {
                if (errno == ENOENT) {
                        return 0;
                }
                /* Ignore other errors that returned from stat call. */
                
                return 0;
        }
        
        return 1;
}

/* Directory check, returns non-zero if the path is a directory. */
int is_directory(const char *path) {
        struct stat info;
        if (stat(path, &info) != 0)
                return 0;

        return S_ISDIR(info.st_mode);
}

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
 * Returns 0 on success, 
 */
int lcopy_file (char *src, char *dest, int rflag) {
        /* Source does not exist. */
        if (!is_file_exist(src)) {
                exception = SRCNOTEXIST;
                return -1;
        }

        /* Directory sources will be omitted
                unless recursive copy set. */
        if (!rflag && is_directory(src)) {
                exception = OMITSRC;
                return -1;
        }
        
        /* Do some copy work... */
        
        
        return 0;
}

/*
 * Prints usage information.
 */
void usage () {
        printf("Usage:\n"
               "\t[OPTIONS] SOURCE... DEST\n"
               "\tCopy SOURCE(s) to DEST.\n"
               "\nOptions:\n"
               "\t-R,-r   Recursively copy\n");
}

/*
 * Program main.
 */
int main (int argc, char *argv[]) {
        int ch;
        int rflag = 0; /* Recursively copy flag. */
        int number_of_sources; /* Number of source targets. */
        int i, j;
        int rc;
        char **sources;
        char *dest = NULL;
        
        /* Missing arguments, early control. */
        if (argc < 3) {
                usage();
                exit(EXIT_SUCCESS);
        }
        
        /* Decrease 1 for name-for-executable, 1 for destination target. */
        number_of_sources = argc - 2; 
        
        /* Parse command line options. */
        while ((ch = getopt(argc, argv, "Rr")) != -1) {
                switch (ch) {
                case 'R':
                case 'r':
                        rflag = 1;
                        number_of_sources--;
                        break;
                default:
                        usage();
                        break;
                }
        }
        
        /* Missing arguments. */
        if (number_of_sources == 0) {
                usage();
                exit(EXIT_SUCCESS);
        }
        
        sources = malloc(sizeof(char*) * number_of_sources);   
        
        for (j = 0, i = argc-1; i > 0; i--) {
                /* Means that it is an option. */
                if (argv[i][0] == '-') {
                   continue;     
                }
                
                if (dest == NULL) {
                        dest = argv[i];
                } else {
                        sources[j++] = argv[i];
                }
        }  
        
#ifdef DEBUG
        printf("Destination target: %s\n", dest);
        printf("Recursively copy: %s\n", rflag ? "On" : "Off");
        printf("Number of sources: %d\n", number_of_sources);
        for (i = 0; i < number_of_sources; i++) {
                printf("Source target: %s\n", sources[i]);
        }
        fflush(stdout);
#endif /* DEBUG */
        
        /* 
         * If multiple sources selected, 
         * then destination target has to be a directory.
         */
        if (number_of_sources > 1 && !is_directory(dest)) {
                exception = DESTNOTDIR;
                printf("%s: %s.\n", exception_str[exception], dest);
                exit(EXIT_FAILURE);
        }
        
        /* Do lazy copy for each source target. */
        for (i = 0; i < number_of_sources; i++) {
                rc = lcopy_file(sources[i], dest, rflag);
                if (!rc)
                        printf("Copied from %s to %s.\n", sources[i], dest);
                else
                        printf("%s: %s.\n", 
                                exception_str[exception], sources[i]);
        }
        
        exit(EXIT_SUCCESS);
}
