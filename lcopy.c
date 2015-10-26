#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <time.h>

/* Each chunk is SIZE_OF_CHUNK bytes. */
#define SIZE_OF_CHUNK 131072 /* 128*1024 */

/* Each digest of a chunk is SIZE_OF_DIGEST bytes. */
#define SIZE_OF_DIGEST 16

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
        NONDIRDEST, /* Cannot overwrite non-directory destination
                with directory source.*/
        OMITDIGS, /* Omit regular source files with extension .digs. */
} Exception;

/* String representation of defined exceptions. */
const char* exception_str[] = {
        "Omitting source directory", 
        "Destination is not a directory",
        "Source does not exists",
        "Cannot overwrite non-directory destination with directory source.",
        "Omitting source file with extension .digs"
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

/* Is it a regular file. */
int is_regularfile(const char *path) {
        struct stat info;
        if (stat(path, &info) != 0)
                return 0;
                
        return S_ISREG(info.st_mode);
}

/* Directory check, returns non-zero if the path is a directory. */
int is_directory(const char *path) {
        struct stat info;
        if (stat(path, &info) != 0)
                return 0;

        return S_ISDIR(info.st_mode);
}

/* Compare modification time of given paths. */
double compare_mtime(const char *path1, const char *path2) {
        struct stat info1;
        struct stat info2;
        double seconds;
        
        if (stat(path1, &info1) != 0)
                handle_error("stat");
        
        if (stat(path2, &info2) != 0)
                handle_error("stat");
        
        seconds = difftime(info1.st_mtime, info2.st_mtime);
        
        printf("compare_mtime:%f\n", seconds);
        
        return seconds;
}

/* Returns extension of a file. */
const char *get_extension(const char *path) {
    const char *dot = strrchr(path, '.');
    if (!dot || dot == path)
        return "";
    
    return dot + 1;
}

/* Returns basename from file path. */
char* get_basename (const char *path) {
        printf("get_basename:%s\n", path);
        char *basename;
        char *copy;
        int l = 0;
        char *ssc;
        
        basename = malloc(500);
        strcpy(basename, path);
        
        while (ssc = strstr(basename, "/")) {
                l = strlen(ssc) + 1;
                copy = basename;
                basename = &basename[strlen(basename)-l+2];
        }
        
        /* Like "/dir/" */
        if (!strcmp(basename, "")) {
                char *temp;
                temp = malloc(500);
                /* Drop last '/' and take "dir". */
                strncpy(temp, copy, (strlen(copy)-1));
                return temp;
        }
        
        return basename;
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

        digs_path = malloc(strlen(path)+1+5);
        
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
       
       rewind(src);
       rewind(digsfile);
       
        while((fread_src_length = 
                        fread(readbuffer, 1, SIZE_OF_CHUNK, src)) > 0) {
                
                md5_length = digmd5(readbuffer, digest, fread_src_length);
                
                printf("writing digest:%02x\n", digest);
                
                /*fwrite_dest_length = fwrite(digest, 1, md5_length, digsfile);
                if (fwrite_dest_length < md5_length)
                        return -1;
                */
                
                fprintf(digsfile, "%02x\n", digest);
        }
        
        return 0;
}

/*
 * Copies source file to destination target.
 * Should be called if destination target does not exists.
 */
int copy_file_raw (FILE *src, FILE *dest) {
        unsigned char buffer;
        size_t fread_src_length;
        size_t fwrite_dest_length;
        
        rewind(src);
        rewind(dest);
        
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

        return 0;
}

/*
 * Copies source file to destination in a lazy way.
 * Returns 0 on success, 
 */
int lcopy (char *src, char *dest, int rflag) {
        printf("lcopy src: %s and dest: %s, rflag: %d\n",
                src, dest, rflag);
        
        /* Source does not exist. */
        if (!is_file_exist(src)) {
                exception = SRCNOTEXIST;
                return -1;
        }

        /* Source is a directory. */
        if (is_directory(src)) {
                printf("src: %s is a directory\n", src);
                
                /* Directory sources will be omitted 
                        unless recursive copy set. */
                if (!rflag) {
                        exception = OMITSRC;
                        return -1;
                }
                /* Recursively copy the directory. */
                else {
                        /* Check that destination is not a regular file. */
                        if (is_regularfile(dest)) {
                                exception = NONDIRDEST;
                                return -1;
                        }
                        
                        /* Destination is an existent directory. */
                        if (is_directory(dest)) {
                                printf("dest:%s is a directory, too.\n", dest);
                                
                                char *src_basename = get_basename(src);
                                char *dest_basename = get_basename(dest);
                                char *new_dest_dir = malloc(500);
                                
                                if (strcmp(src_basename, dest_basename)) {
                                        strcpy(new_dest_dir, dest);
                                        strcat(new_dest_dir, "/");
                                        strcat(new_dest_dir, src_basename);
                                } else {
                                        strcpy(new_dest_dir, dest);
                                }
                                
                                printf("Checking %s exist?\n", new_dest_dir);
                                
                                /* If not exist mkdir new destination directory. */
                                if (!is_directory(new_dest_dir)) {
                                        printf("not exist, mkdir it.\n");
                                        if (mkdir(new_dest_dir,0777) == -1) {
                                                handle_error("mkdir");
                                        }
                                }
                                
                                
                                /* 
                                 * For each file/directory in source directory,
                                 * call again lcopy with file/directory source
                                 * and destination target.
                                 */
                                DIR * d;
                                struct dirent *entry;

                                /*ret = chdir(MANIFEST_DIR);
                                if (ret!=0) {
                                perror("Error:");
                                }*/

                                d = opendir(src);

                                if (!d) {
                                        handle_error("opendir");
                                }

                                while ((entry = readdir(d)) != NULL) {

                                        if (!strcmp (entry->d_name, "."))
                                                continue;
                                        if (!strcmp (entry->d_name, ".."))    
                                                continue;
                                        
                                        printf("Working with direntry %s\n", 
                                                entry->d_name);
                                        
                                        char *src_path, *dest_path;
                                        
                                        src_path = malloc(500);
                                        dest_path = malloc(500);
                                        
                                        strcpy(src_path, src);
                                        strcat(src_path, "/");
                                        strcat(src_path, entry->d_name);
                                        
                                        strcpy(dest_path, new_dest_dir);
                                        strcat(dest_path, "/");
                                        strcat(dest_path, entry->d_name);
                                        
                                        printf("Calling lcopy(%s,%s,%d)\n",
                                                src_path, dest_path, rflag);
                                        
                                        int retcon = lcopy(src_path, 
                                                        dest_path, rflag);
                                        if (!retcon)
                                                printf("Copied from %s to %s.\n",
                                                        src_path, dest_path);
                                        else
                                                printf("%s: src:%s dest:%s.\n", 
                                                        exception_str[exception], 
                                                        src_path, dest_path);
                                }
                                
                        } 
                        /* Destination is assumed not exist. */
                        else {
                                printf("Destination is assumed not exist.D2\n");
                                if (mkdir(dest,0777) == -1) {
                                        handle_error("mkdir");
                                }
                                
                                /* 
                                 * For each file/directory in source directory,
                                 * call again lcopy with file/directory source
                                 * and destination target.
                                 */
                                DIR * d;
                                struct dirent *entry;

                                /*ret = chdir(MANIFEST_DIR);
                                if (ret!=0) {
                                perror("Error:");
                                }*/

                                d = opendir(src);

                                if (!d) {
                                        handle_error("opendir");
                                }

                                while ((entry = readdir(d)) != NULL) {

                                        if (!strcmp (entry->d_name, "."))
                                                continue;
                                        if (!strcmp (entry->d_name, ".."))    
                                                continue;
                                        
                                        printf("D2 Working with direntry %s\n", 
                                                entry->d_name);
                                        
                                        char *src_path, *dest_path;
                                        
                                        src_path = malloc(500);
                                        dest_path = malloc(500);
                                        
                                        strcpy(src_path, src);
                                        strcat(src_path, "/");
                                        strcat(src_path, entry->d_name);
                                        
                                        strcpy(dest_path, dest);
                                        strcat(dest_path, "/");
                                        strcat(dest_path, entry->d_name);
                                        
                                        printf("D2 Calling lcopy(%s,%s,%d)\n",
                                                src_path, dest_path, rflag);
                                        
                                        int retcon = lcopy(src_path, 
                                                        dest_path, rflag);
                                        if (!retcon)
                                                printf("Copied from %s to %s.\n",
                                                        src_path, dest_path);
                                        else
                                                printf("%s: src:%s dest:%s.\n", 
                                                        exception_str[exception], 
                                                        src_path, dest_path);
                                }
                                
                                
                        }

                }
        } 
        /* Source is assumed as a regular file. */
        else {
                /* Omit <>.digs files while copying. */
                if (!strcmp(get_extension(src), "digs")) {
                        exception = OMITDIGS;
                        return -1;
                }
                
                printf("some copy job from regular file:%s to %s\n",
                        src, dest);
                
                /* Destination is not exist. */
                if (!is_file_exist(dest)) {
                        printf("R4,R8,R7(?)destination is not exist\n"
                                "raw copy from: %s to:%s.\n", src, dest);
                        FILE *src_file = fopen(src, "r");
                        FILE *dest_file = fopen(dest, "w");
                        FILE *src_digs_file;
                        FILE *dest_digs_file;
                        
                        if (src_file == NULL)
                                handle_error("fopen");
                        if (dest_file == NULL) {
                                fclose(src_file);
                                handle_error("fopen");
                        }
                        
                        copy_file_raw(src_file, dest_file);
                        
                        /* Create source digs file. */
                        char *src_digs_path = get_digs_filepath(src);
                        if (!is_file_exist(src_digs_path) || 
                                compare_mtime(src, src_digs_path) > 0) {
                                src_digs_file = fopen(src_digs_path, "w+"); 
                                if (src_digs_file == NULL)
                                        handle_error("fopen");
                                
                                printf("creating src:%s\n", src_digs_path);
                                write_digest_file(src_file, src_digs_file);
                                fclose(src_digs_file);
                        }
                        fclose(src_file);
                        
                        /* Create destination digs file. */
                        char *dest_digs_path = get_digs_filepath(dest);
                        dest_digs_file = fopen(dest_digs_path, "w");
                        if (dest_digs_file == NULL)
                                handle_error("fopen");
                        
                        fclose(dest_file);
                        dest_file = fopen(dest, "r");
                        if (dest_file == NULL)
                                handle_error("fopen");
                                
                        printf("creating destdigs:%s\n", dest_digs_path);
                        write_digest_file(dest_file, dest_digs_file);
                        fclose(dest_file);
                        fclose(dest_digs_file);
                        
                }
                /* Destination exist and it is a directory. */
                else if (is_directory(dest)) {
                        printf("R6 Destination exist and it is a directory\n");
                        char *new_dest = malloc(500);
                        char *src_basename = get_basename(src);
                        
                        strcpy(new_dest, dest);
                        strcat(new_dest, "/");
                        strcat(new_dest, src_basename);
                        
                        int retcon = lcopy(src, new_dest, rflag);
                        
                        if (!retcon)
                                printf("Copied from %s to %s.\n",
                                        src, new_dest);
                        else
                                printf("%s: src:%s dest:%s.\n", 
                                        exception_str[exception], 
                                        src, new_dest);
                        
                }
                /* Destination exist and it is a regular file. */
                else if (is_regularfile(dest)) {
                        printf("R3 Destination exist and it is a regular file.\n");
                        /* Check that source digest file exist. */
                        char *src_digs_path = get_digs_filepath(src);
                        char *dest_digs_path = get_digs_filepath(dest);
                        
                        FILE *src_f = NULL;
                        FILE *src_digs_f = NULL;
                        FILE *dest_f = NULL;
                        FILE *dest_digs_f = NULL;
                        
                        src_f = fopen(src, "r");
                        if (src_f == NULL)
                                handle_error("fopen1");
                                
                        dest_f = fopen(dest, "r+");
                        if (dest_f == NULL)
                                handle_error("fopen2");
                                
                        /* 
                        * If <>.digs file do not exist or its modification
                        * time is before the modification time of file, 
                        * update <>.digs.
                        */
                        if (!is_file_exist(src_digs_path) || 
                                compare_mtime(src, src_digs_path) > 0) {
                                src_digs_f = fopen(src_digs_path, "w+"); 
                                if (src_digs_f == NULL)
                                        handle_error("fopen3");
                                
                                printf("creating src:%s\n", src_digs_path);
                                write_digest_file(src_f, src_digs_f);
                        }
                        if (!is_file_exist(dest_digs_path) || 
                                compare_mtime(dest, dest_digs_path) > 0) {
                                dest_digs_f = fopen(dest_digs_path, "w+"); 
                                if (dest_digs_f == NULL)
                                        handle_error("fopen4");
                                
                                printf("creating dest:%s\n", dest_digs_path);
                                write_digest_file(dest_f, dest_digs_f);
                        }
                        
                        if (src_digs_f == NULL) {
                                src_digs_f = fopen(src_digs_path, "r"); 
                                if (src_digs_f == NULL)
                                        handle_error("fopen5");
                        }
                        
                        if (dest_digs_f == NULL) {
                                dest_digs_f = fopen(dest_digs_path, "r"); 
                                if (dest_digs_f == NULL)
                                        handle_error("fopen6");
                        }
                        
                        rewind(src_f);
                        rewind(src_digs_f);
                        rewind(dest_f);
                        rewind(dest_digs_f);

                        int src_len, dest_len;
                        unsigned char *src_buf = malloc(SIZE_OF_DIGEST);
                        unsigned char *dest_buf = malloc(SIZE_OF_DIGEST);
                        
                        int chunk_index = 0;
                        int diff_flag;
                        int rchunk_size = 0;
                        int wchunk_size = 0;
                        
                        while(fgets(src_buf, SIZE_OF_DIGEST, src_digs_f)) {

                                dest_len = fread(dest_buf, 1, SIZE_OF_DIGEST,
                                        dest_digs_f);
                                        
                                fgets(dest_buf, SIZE_OF_DIGEST, dest_digs_f);
                                
                                printf("srcdigs:%s\ndestdigs:%s\n", 
                                        src_buf, dest_buf);
                                        
                                diff_flag = strcmp(src_buf, dest_buf);
                                
                                printf("diff_flag: %d\n", diff_flag);
                                
                                /* Copy chunk from source to destination. */
                                if (diff_flag) {
                                        printf("copying %s chunk %d to %s.\n",
                                                src, chunk_index, dest);
                                        
                                        int retval;
                                        unsigned char *chunk = malloc(SIZE_OF_CHUNK);
                                        memset(chunk, 0, sizeof(chunk));
                                        /* Copy chunk from source to buffer.*/
                                        retval = fseek(src_f, (chunk_index*SIZE_OF_CHUNK), SEEK_SET);
                                        printf("retval1:%d\n", retval);
                                        rchunk_size = fread(chunk, 1, SIZE_OF_CHUNK, src_f);
                                        
                                        printf("rchunk_size:%d\n", rchunk_size);
                                        
                                        /* Write chunk to dest from buffer. */
                                        retval = fseek(dest_f, (chunk_index*SIZE_OF_CHUNK), SEEK_SET);
                                        printf("retval2:%d\n", retval);
                                        
                                        wchunk_size = fwrite(chunk, 1, rchunk_size, dest_f);
                                        
                                        printf("wchunk_size:%d\n", wchunk_size);
                                        
                                        if (wchunk_size < rchunk_size) {
                                                /* if (ferror(fd2)) */
                                                handle_error("fwrite");
                                        }
                                }
                                
                                chunk_index++;
                        }
                        
                        fclose(src_f);
                        fclose(src_digs_f);
                        fclose(dest_f);
                        fclose(dest_digs_f);
                }
                /* Erronous condition. */
                else {
                        handle_error(dest);
                }
                
        }
                
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
                rc = lcopy(sources[i], dest, rflag);
                if (!rc)
                        printf("Copied from %s to %s.\n", sources[i], dest);
                else
                        printf("%s: src:%s dest:%s.\n", 
                                exception_str[exception], sources[i], dest);
        }
        
        exit(EXIT_SUCCESS);
}
