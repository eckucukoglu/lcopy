all:
	$(CC) -o lcopy lcopy.c digmd5.c -lssl -lcrypto
debug:
	$(CC) -o lcopy lcopy.c digmd5.c -lssl -lcrypto -DDEBUG
clean:
	rm lcopy
