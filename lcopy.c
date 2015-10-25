#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

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

/* Compare modification time of given paths. */
int compare_mtime (const char *path1, const char *path2) {
        struct stat info1;
        struct stat info2;
        
        if (stat(path1, &info1) != 0)
                handle_error("stat");
        
        if (stat(path2, &info2) != 0)
                handle_error("stat");
               
        return (info1.st_mtime - info2.st_mtime);
}

/* 
 * Returns digs file path.
 * path + extension = digs_path
 */
char *get_digs_filepath(const char *path) {
        const char* extension = ".digs";
        char *digs_path = NULL;
        
        if (path == NULL)
                return;

        digs_path = malloc(strlen(path)+1+4);
        
        strcpy(digs_path, path);
        strcat(digs_path, extension);
        
        return digs_path;
}

/*
 * Writes src's digest values to digsfile.
 * Returns 0 on success.
 */
int write_digest_file (FILE *src, FILE *digsfile) {
        int fread_src_length = 0;
        int fwrite_dest_length = 0;
        int md5_length = 0;
        char readbuffer[SIZE_OF_CHUNK];
        unsigned char digest[128];
        int i = 0;
        
        if (src == NULL || digsfile == NULL)
                return -1;
       
        while((fread_src_length = 
                        fread(readbuffer, 1, SIZE_OF_CHUNK, src)) > 0) {
                
                md5_length = digmd5(readbuffer, digest, fread_src_length);
                
                fwrite_dest_length = fwrite(&digest, 1, md5_length, digsfile);
                if (fwrite_dest_length < md5_length)
                        return -1;
        }
        
        return 0;
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
int copy_chunk (FILE *from, FILE *to, int index) {
        
}

/*
 * Copies source file to destination target.
 * Should be called if destination target does not exists.
 */
int copy_file_raw (char *from, char *to) {
        FILE *src, *dest;
        unsigned char buffer;
        size_t fread_src_length;
        size_t fwrite_dest_length;
        
        src = fopen(from, "r");
        if (src == NULL)
                handle_error("fopen");

        dest = fopen(to, "w");
        if (dest == NULL) {
                fclose(src);
                handle_error("fopen");
        }

        while((fread_src_length = fread(&buffer, 1, 1, src)) > 0) {
                fwrite_dest_length = fwrite(&buffer, 1, fread_src_length, dest);
                if (fwrite_dest_length < fread_src_length) {
                        /* if (ferror(fd2)) */
                        handle_error("fwrite");
                }
        }

        /*char c;
        while ((c = fgetc(src)) != EOF)
                fputc(c, dest);*/

        fclose(src);
        fclose(dest);
        
        return 0;
}

/*
 * Copies source file to destination in a lazy way.
 * Returns 0 on success, 
 */
int lcopy_file (char *src, char *dest, int rflag) {
        char *digs_path = NULL;

        /* Source does not exist. */
        if (!is_file_exist(src)) {
                exception = SRCNOTEXIST;
                return -1;
        }

        /* Source is a directory. */
        if (is_directory(src)) {
                /* Directory sources will be omitted 
                        unless recursive copy set. */
                if (!rflag) {
                        exception = OMITSRC;
                        return -1;
                }
                /* Recursively copy the directory. */
                else {
                        printf("recursively copy\n");
                }
        } 
        /* Source is a regular file. */
        else {
                FILE *src_f;
                FILE *src_digs_f;
                
                digs_path = get_digs_filepath(src);
                
                src_f = fopen(src, "r");
                if (src_f == NULL)
                        handle_error("fopen");

                /* 
                 * If source.digs file do not exist or its modification
                 * time is before the modification time of source, 
                 * update source.digs.
                 */
                if (!is_file_exist(digs_path) || 
                        compare_mtime(src, digs_path) > 0) {
                        
                        src_digs_f = fopen(digs_path, "w"); /* TODO: don't forget to close. */
                        if (src_digs_f == NULL)
                                handle_error("fopen");
                        
                        write_digest_file(src_f, src_digs_f);
                }
                
                fclose(src_f);
        }
        
        
        
        
        /* Do some copy work... */
        // copy_file_raw(src, dest);
        /*FILE *a;
        FILE *b;
        a = fopen(src, "r");
        if (a == NULL)
                handle_error("fopen");

        b = fopen(dest, "w");
        if (b == NULL) {
                fclose(a);
                handle_error("fopen");
        }
        
        write_digest_file(a,b);
        
        fclose(a);
        fclose(b);*/
        
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
        int number_of_sources; /* Number of sources. */
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
                printf("Source: %s\n", sources[i]);
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
        
        /* Do lazy copy for each sources. */
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
