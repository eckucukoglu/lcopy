#include <openssl/evp.h>
#include <stdio.h>

#define ALGO EVP_md5()

/** get a bufffer and calculate md5 sum of the buffer
 @buffer to find checksum
 @dgst an array with at least EVP_MD_size(md) bytes to put the result
 @n size of the buffer
*/
int digmd5(const char *buffer, char *dgst, int n) {
        size_t r;
        int m = 0;
        int ds;

        EVP_MD_CTX ctx;
        const EVP_MD *md;

        md = ALGO;
        if (md == NULL) /* invalid ALGO */
                return -2;

        EVP_DigestInit(&ctx, md);
        ds = EVP_MD_size(md);
        EVP_DigestUpdate(&ctx, buffer, n);
        EVP_DigestFinal(&ctx, dgst, &ds);

        return ds;
}

#ifdef DIGMD5_TEST
char readbuffer[128*1024]; /* at most 128 MBytes */

int main(int argc, char *argv[]) {
        OpenSSL_add_all_digests();
        int i, j, n, ds;
        unsigned char digest[128];

        if (argc > 1) {
                printf("only first %d bytes are read for digest!!\n",
                        1024*128);
                for (i = 1; i < argc ; i++) {
                        FILE *fp;

                        fp = fopen(argv[i], "r");
                        if (fp == NULL) {
                                fprintf(stderr, "cannot open %s\n", argv[i]);
                                continue;
                        }

                        n = fread(readbuffer, 1, 1024*128, fp);
                        ds = digmd5(readbuffer, digest, n);

                        for (j = 0; j < ds ; j++)
                                printf("%02x", digest[j]);
                                
                        printf("\t%s\n", ds,argv[i]);
                        fclose(fp);
                }
        }
}
#endif
