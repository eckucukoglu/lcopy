lcopy:
	$(CC) -o lcopy lcopy.c digmd5.c -lssl -lcrypto
clean:
	rm lcopy
