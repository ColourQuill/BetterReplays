#include <portal.hpp>

struct Data {
    int* fd;
    int* node;
    GMainLoop* loop;
    XdpSession* session;
};

void Portal::request(int& fd, int& node) {
    g_autoptr(XdpPortal) portal = xdp_portal_new();
    g_autoptr(GError) error = nullptr;

    Logger::logInfo("Portal", "Requesting screencast session...");

    Data data = {
        .fd = &fd,
        .node = &node,
        .loop = g_main_loop_new(nullptr, FALSE),
        .session = nullptr
    };

    xdp_portal_create_screencast_session(
        portal,
        XDP_OUTPUT_MONITOR,
        XDP_SCREENCAST_FLAG_NONE,
        XDP_CURSOR_MODE_EMBEDDED,
        XDP_PERSIST_MODE_NONE,
        nullptr,
        nullptr,
        onSessionCreated,
        &data
    );

    g_main_loop_run(data.loop);
    g_main_loop_unref(data.loop);
}

void Portal::onSessionCreated(GObject* source, GAsyncResult* res, void* userData) {
    auto* data = static_cast<Data*>(userData);
    GError* error = nullptr;
    data->session = xdp_portal_create_screencast_session_finish(XDP_PORTAL(source), res, &error);

    if (error) {
        Logger::logError("Portal", "Failed to create session -> " + std::string(error->message));
        g_error_free(error);
        g_main_loop_quit(data->loop);
        return;
    }
    Logger::logInfo("Portal", "Successfully created session.");

    xdp_session_start(data->session, nullptr, nullptr, onSessionStarted, userData);
}
void Portal::onSessionStarted(GObject* source, GAsyncResult* res, void* userData) {
    auto* data = static_cast<Data*>(userData);

    if (xdp_session_start_finish(data->session, res, nullptr)) {
        *data->fd = xdp_session_open_pipewire_remote(data->session);

        g_autoptr(GVariant) streams = xdp_session_get_streams(data->session);
        if (streams) {
            GVariantIter iter;
            g_variant_iter_init(&iter,  streams);
            uint32_t node;
            if (g_variant_iter_next(&iter, "(u@a{sv})", &node, nullptr)) {
                *data->node = (int)node;
            }
        }
    }
    g_main_loop_quit(data->loop);
}