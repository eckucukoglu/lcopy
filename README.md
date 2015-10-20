# lcopy
lcopy is a copy utility which aims to reduce number of I/O operations for large files.

The utility will be called lcopy standing for “lazy copy”. The lcopy considers a large file is divided into 128KB chunks (128*1024 bytes), and keep digest (hashsum) of chunks on a seperate file named filename.digs. Assumming md5 algorithm, digest of a chunk is a 128 bit (16 bytes) summary of the content.

# Algorithm:
1. Make sure source exists and destination path exists.
2. If source.digs file do not exist or its modification time is before the modification time of source, update source.digs.
3. If dest file do not exits, make a normal copy and create dest.digs. Then exit.
4. If dest.digs does not exist or its modification time is before the modification time of dest, update dest.digs.
5. For each chunk i:
        (a) read the digest of [i]th block of source, s[i],
        (b) read the digest of [i]th block of destination d[i],
        (c) if s[i] = d[i] skip to next block,
        (d) else, copy [i]th block from source to destination.

# Usage:
lcopy [-r] source ... dest

* -r means recursive, if one of the source is a directory, it is recursively copied as a directory on target preserving lcopy semantics.
* There can be multiple source parameters if dest is a directory, otherwise only one file is allowed. In directory case file name will be same, i.e. source is copied on dest/source/.
