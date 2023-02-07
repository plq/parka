#ifndef PARKA_ASYNC_FILE_STREAMER_H
#define PARKA_ASYNC_FILE_STREAMER_H

#include <filesystem>
#include <map>
#include <string>

#include "AsyncFileReader.h"

struct AsyncFileStreamer {
    std::map<std::string_view, AsyncFileReader *> async_file_feaders;
    std::string root;

    AsyncFileStreamer(std::string root) : root(root) {
        // for all files in this path, init the map of AsyncFileReaders
        updateRootCache();
    }

    void updateRootCache() {
        // todo: if the root folder changes, we want to reload the cache
        for (auto &p : std::filesystem::recursive_directory_iterator(root)) {
            std::string url = p.path().string().substr(root.length());
            if (url == "/index.html") {
                url = "/";
            }

            char *key = new char[url.length()];
            memcpy(key, url.data(), url.length());
            async_file_feaders[std::string_view(key, url.length())] =
                    new AsyncFileReader(p.path().string());
        }
    }

    template <bool SSL>
    void stream_file(uWS::HttpResponse<SSL> *res, std::string_view url) {
        auto it = async_file_feaders.find(url);
        if (it == async_file_feaders.end()) {
            LOG_F(ERROR, "Did not find file: {}", url);
            res->writeStatus("404 Not Found")->end("Not found");
            return;
        }

        res->writeStatus(uWS::HTTP_200_OK);

        if (has_extension(url, ".svg")) {
            res->writeHeader("Content-Type", "image/svg+xml");
        }

        stream_file(res, it->second);
    }

    template <bool SSL>
    static void stream_file(uWS::HttpResponse<SSL> *res, AsyncFileReader *async_file_feader) {
        /* Peek from cache */
        std::string_view chunk = async_file_feader->peek(res->getWriteOffset());
        if (! chunk.length() || res->tryEnd(chunk, async_file_feader->getFileSize()).first) {
            /* Request new chunk */
            // todo: we need to abort this callback if peer closed!
            // this also means Loop::defer needs to support aborting (functions should embedd an
            // atomic boolean abort or something)

            // Loop::defer(f) -> integer
            // Loop::abort(integer)

            // hmm? no?

            // us_socket_up_ref because we share the ownership

            if (chunk.length() < async_file_feader->getFileSize()) {
                async_file_feader->request(
                        res->getWriteOffset(), [res, async_file_feader](std::string_view chunk) {
                            // check if we were closed in the mean time
                            // if (us_socket_is_closed()) {
                            // free it here
                            // return;
                            //}

                            /* We were aborted for some reason */
                            if (! chunk.length()) {
                                // todo: make sure to check for is_closed internally after all
                                // callbacks!
                                res->close();
                            }
                            else {
                                AsyncFileStreamer::stream_file(res, async_file_feader);
                            }
                        });
            }
        }
        else {
            /* We failed writing everything, so let's continue when we can */
            res->onWritable([res, async_file_feader](int offset) {
                   // här kan skiten avbrytas!

                   AsyncFileStreamer::stream_file(res, async_file_feader);
                   // todo: I don't really know what this is supposed to mean?
                   return false;
               })->onAborted([]() { LOG_F(ERROR, "ABORTED!"); });
        }
    }
};

#endif
