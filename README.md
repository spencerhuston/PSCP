# Parallel Secure-Copy : CS422 Final Project
### (REQUIRES C++17 and g++ VERSION 8 ON LINUX)

Directions:
1. Simply clone and run *make -j8*
2. Server usage is *./pscp_server*
3. Client usage is *./pscp thread_num host:file_path local_path*

NOTE: 
- If copying a single file, it will be copied in parallel. 
- If copying a directory, each file will be copied by a single thread (among the pool)
