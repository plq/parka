#ifndef PARKA_ASYNC_FILE_READER
#define PARKA_ASYNC_FILE_READER

/*
 * Part of the Parka project (c) Parka contributors. All rights reserved.
 * This file is being distributed under BSD license. See LICENSE file
 * at the project root for the full text. See git log for the list of
 * contributors.
 */

/* based on uws example */

#include <cstring>
#include <fstream>
#include <future>
#include <iostream>
#include <map>
#include <sstream>

#include <App.h>

#include <loguru/loguru.hpp>

inline bool has_extension(std::string_view file, std::string_view ext) {
    if (ext.size() > file.size()) {
        return false;
    }
    return std::equal(ext.rbegin(), ext.rend(), file.rbegin());
}

/* This is just a very simple and inefficient demo of async responses,
 * please do roll your own variant or use a database or Node.js's async
 * features instead of this really bad demo */
struct AsyncFileReader {
private:
    /* The cache we have in memory for this file */
    std::string cache;
    int cache_offset;
    bool has_cache;

    /* The pending async file read (yes we only support one pending read) */
    std::function<void(std::string_view)> pendingReadCb;

    int file_size;
    std::string file_name;
    std::ifstream fin;
    uWS::Loop *loop;

public:
    /* Construct a demo async. file reader for fileName */
    AsyncFileReader(std::string file_name) : file_name(file_name) {
        fin.open(file_name, std::ios::binary);

        // get fileSize
        fin.seekg(0, fin.end);
        file_size = fin.tellg();

        DLOG_F(INFO, "File size is: {}", file_size);

        // cache up 1 mb!
        cache.resize(1024 * 1024);

        DLOG_F(INFO, "Caching 1 MB at offset = 0");
        fin.seekg(0, fin.beg);
        fin.read(cache.data(), cache.length());
        cache_offset = 0;
        has_cache = true;

        // get loop for thread
        loop = uWS::Loop::get();
    }

    /* Returns any data already cached for this offset */
    std::string_view peek(int offset) {
        /* Did we hit the cache? */
        if (has_cache && offset >= cache_offset && ((offset - cache_offset) < cache.length())) {
            /* Cache hit */
            DLOG_F(INFO, "Cache hit");

            if (file_size - offset < cache.length()) {
                DLOG_F(WARNING, "LESS THAN WHAT WE HAVE!");
            }

            int chunkSize =
                    std::min<int>(file_size - offset, cache.length() - offset + cache_offset);

            return std::string_view(cache.data() + offset - cache_offset, chunkSize);
        }
        else {
            /* Cache miss */
            DLOG_F(WARNING, "Cache miss!");
            return std::string_view(nullptr, 0);
        }
    }

    /* Asynchronously request more data at offset */
    void request(int offset, std::function<void(std::string_view)> cb) {
        // in this case, what do we do?
        // we need to queue up this chunk request and callback!
        // if queue is full, either block or close the connection via abort!
        if (! has_cache) {
            // already requesting a chunk!
            LOG_F(ERROR, "ERROR: already requesting a chunk!");
            return;
        }

        // disable cache
        has_cache = false;

        std::async(std::launch::async, [this, cb, offset]() {
            DLOG_F(INFO, "ASYNC Caching 1 MB at offset = {}", offset);

            // it has been closed! open again!
            if (! fin.good()) {
                fin.close();
                LOG_F(WARNING, "Reopening fin!");
                fin.open(file_name, std::ios::binary);
            }
            fin.seekg(offset, fin.beg);
            fin.read(cache.data(), cache.length());

            cache_offset = offset;

            loop->defer([this, cb, offset]() {
                int chunk_size = std::min<int>(cache.length(), file_size - offset);

                // both of these happen, wtf?
                if (chunk_size == 0) {
                    LOG_F(WARNING, "Zero size!?");
                }

                if (chunk_size != cache.length()) {
                    LOG_F(WARNING, "LESS THAN A CACHE 1 MB!");
                }

                has_cache = true;
                cb(std::string_view(cache.data(), chunk_size));
            });
        });
    }

    /* Abort any pending async. request */
    void abort() {}
    int getFileSize() { return file_size; }
};

#endif
