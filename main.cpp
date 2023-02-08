
/*
 * Part of the Parka project (c) Parka contributors. All rights reserved.
 * This file is being distributed under BSD license. See LICENSE file
 * at the project root for the full text. See git log for the list of
 * contributors.
 */

#include <App.h>

#include <parka/AsyncFileReader.h>
#include <parka/AsyncFileStreamer.h>

#define OPTPARSE_IMPLEMENTATION
#include <parka/optparse.h>

template <bool SSL>
static void
get_static(uWS::HttpResponse<SSL> *resp, uWS::HttpRequest *req, AsyncFileStreamer &afs) {
    auto scope = fmt::format("GET {}", req->getUrl());
    LOG_SCOPE_F(INFO, "%s", scope.data());
    afs.stream_file(resp, req->getUrl());
}

template <bool SSL>
static void put_location(uWS::HttpResponse<SSL> *resp, uWS::HttpRequest *req) {
    resp->onAborted([]() {});
    resp->onData([resp, sstrptr = std::make_unique<std::stringstream>()](
                         std::string_view data, bool last) mutable {
        auto &sstr = *sstrptr;
        if (! data.empty()) {
            sstr << data;
        }

        if (last) {
            DLOG_F(INFO, "data: {}", sstr.str());
            resp->writeStatus("200 OK")->end(fmt::format("{{id: {}}}", 5));
        }
    });
}

template <bool SSL>
static void build(uWS::TemplatedApp<SSL> &app, AsyncFileStreamer &afs) {
    app //
            .get("/*", [&afs](auto *resp, auto *req) { get_static(resp, req, afs); })
            .put("/api/location", [&afs](auto *resp, auto *req) { put_location(resp, req); });
}

template <bool SSL>
static auto run(uWS::TemplatedApp<SSL> &app, int port, const char *root) {
    return app
            .listen(port,
                    [port, root](auto *token) {
                        if (token) {
                            if constexpr (SSL) {
                                LOG_F(INFO, "Serving {} over HTTPS port {}", root, port);
                            }
                            else {
                                LOG_F(INFO, "Serving {} over HTTP port {}", root, port);
                            }
                        }
                    })
            .run();
}

int main(int argc, char **argv) {
    int option;
    struct optparse options;
    optparse_init(&options, argv);

    struct optparse_long longopts[] = {
            {"cert", 'c', OPTPARSE_REQUIRED},
            {"dh_params", 'd', OPTPARSE_REQUIRED},
            {"help", 'h', OPTPARSE_NONE},
            {"key", 'k', OPTPARSE_REQUIRED},
            {"passphrase", 'a', OPTPARSE_REQUIRED},
            {"port", 'p', OPTPARSE_REQUIRED},
            {0},
    };

    int port = 3000;
    struct uWS::SocketContextOptions ssl_options = {};

    while ((option = optparse_long(&options, longopts, nullptr)) != -1) {
        switch (option) {
        case 'p':
            port = atoi(options.optarg);
            break;

        case 'a':
            ssl_options.passphrase = options.optarg;
            break;

        case 'c':
            ssl_options.cert_file_name = options.optarg;
            break;

        case 'k':
            ssl_options.key_file_name = options.optarg;
            break;

        case 'd':
            ssl_options.dh_params_file_name = options.optarg;
            break;

        case 'h':
        case '?':
        fail:
            std::cout << "Usage:" << argv[0]
                      << " [--help]"
                         " [--port <port>]"
                         " [--key <ssl key>]"
                         " [--cert <ssl cert>]"
                         " [--passphrase <ssl key passphrase>]"
                         " [--dh_params <ssl dh params file>]"
                         " <public root>"
                      << std::endl;
            return 0;
        }
    }

    char *root = optparse_arg(&options);
    if (! root) {
        goto fail;
    }

    loguru::g_preamble_uptime = false;
    loguru::init(argc, argv);

    AsyncFileStreamer afs(root);

    /* Either serve over HTTP or HTTPS */
    struct us_socket_context_options_t empty_ssl_options = {};
    if (memcmp(&ssl_options, &empty_ssl_options, sizeof(empty_ssl_options))) {
        /* HTTPS */
        uWS::TemplatedApp<true> app(ssl_options);
        build(app, afs);
        run(app, port, root);
    }
    else {
        /* HTTP */
        uWS::TemplatedApp<false> app;
        build(app, afs);
        run(app, port, root);
    }

    LOG_F(ERROR, "Failed to listen to port {}", port);
}
