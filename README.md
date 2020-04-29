# Parallel Secure-Copy : CS422 Final Project
### (REQUIRES C++17 and g++ VERSION 8 ON LINUX)

Directions:
1. Simply clone and run *make -j8*
2. Server usage is *./pscp_server*
3. Client usage is *./pscp thread_num host:file_path local_path*

NOTES: 
- If copying a single file, it will be copied in parallel. 
- If copying a directory, each file will be copied by a single thread (among the pool)
- **This actually isn't that secure**, more emphasis was placed on the parallelism aspect. For example, authentication doesn't actually do anything, we could have implemented it but wanted to focus more on the actual networking aspect of the project, given that it's for a networks course.
- **There may be some errors when running**. 99% of the time you can just run it again.
- **The file transfer is one way** from server to client. So you cannot do *./pscp thread_num local_path host:file_path*
