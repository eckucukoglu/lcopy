all:
	$(CC) -o lcopy lcopy.c digmd5.c -lssl -lcrypto
debug:
	$(CC) -o lcopy lcopy.c digmd5.c -lssl -lcrypto -DDEBUG
digmd5:
	$(CC) -o digmd5 digmd5.c -lssl -lcrypto -DDIGMD5_TEST
