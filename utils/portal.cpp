#include <portal.hpp>

struct Data {
    int* fd;
    int* node;
    GMainLoop* loop;
    XdpSession* session;
};

struct SelectData {
    DisplayConfig* config;
    GMainLoop* loop;
    XdpSession* session;
};

void Portal::request(int& fd, int& node, const std::string& restoreToken) {
    g_autoptr(XdpPortal) portal = xdp_portal_new();

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
        XDP_PERSIST_MODE_PERSISTENT,
        restoreToken.empty() ? nullptr : restoreToken.c_str(),
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

std::vector<DisplayConfig> Portal::selectDisplays() {
    std::vector<DisplayConfig> displays;
    int index = 1;

    while (true) {
        Logger::logInfo("Portal", "Select a monitor to add (close picker when done)...");
        DisplayConfig config = selectDisplay(index);
        if (config.restoreToken.empty()) break;
        displays.push_back(config);
        index++;
    }

    return displays;
}

DisplayConfig Portal::selectDisplay(int index) {
    DisplayConfig config;
    config.name = "Display-" + std::to_string(index);

    g_autoptr(XdpPortal) portal = xdp_portal_new();

    Logger::logInfo("Portal", "Select monitor " + std::to_string(index) + " in the picker...");

    SelectData data = {
        .config  = &config,
        .loop    = g_main_loop_new(nullptr, FALSE),
        .session = nullptr
    };

    xdp_portal_create_screencast_session(
        portal,
        XDP_OUTPUT_MONITOR,
        XDP_SCREENCAST_FLAG_NONE,
        XDP_CURSOR_MODE_EMBEDDED,
        XDP_PERSIST_MODE_PERSISTENT,
        nullptr,
        nullptr,
        onSelectSessionCreated,
        &data
    );

    g_main_loop_run(data.loop);
    g_main_loop_unref(data.loop);
    return config;

}

void Portal::onSelectSessionCreated(GObject* source, GAsyncResult* res, void* userData) {
    auto* data = static_cast<SelectData*>(userData);
    GError* error = nullptr;

    data->session = xdp_portal_create_screencast_session_finish(XDP_PORTAL(source), res, &error);
    if (error) {
        Logger::logError("Portal", "Failed to create selection session -> " + std::string(error->message));
        g_error_free(error);
        g_main_loop_quit(data->loop);
        return;
    }

    Logger::logInfo("Portal", "Successfully created selection session.");
    xdp_session_start(data->session, nullptr, nullptr, onSelectSessionStarted, userData);
}

void Portal::onSelectSessionStarted(GObject* source, GAsyncResult* res, void* userData) {
    auto* data = static_cast<SelectData*>(userData);
    GError* error = nullptr;

    if (!xdp_session_start_finish(XDP_SESSION(source), res, &error)) {
        if (error) {
            Logger::logError("Portal", "Failed to start selection session -> " + std::string(error->message));
            g_error_free(error);
        }
        g_main_loop_quit(data->loop);
        return;
    }

    const char* token = xdp_session_get_restore_token(data->session);
    if (!token) {
        Logger::logError("Portal", "No restore token received.");
        g_main_loop_quit(data->loop);
        return;
    }

    data->config->restoreToken = token;

    g_autoptr(GVariant) streams = xdp_session_get_streams(data->session);
    if (streams) {
        GVariantIter iter;
        g_variant_iter_init(&iter, streams);
        uint32_t node;
        GVariant* options;
        if (g_variant_iter_next(&iter, "(u@a{sv})", &node, &options)) {
            g_variant_unref(options);
        }
    }

    Logger::logInfo("Portal", "Selected display -> " + data->config->name + " token -> " + data->config->restoreToken);
    g_main_loop_quit(data->loop);
}