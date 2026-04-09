#ifndef __PORTAL_HPP__
#define __PORTAL_HPP__

// better replays
#include <logger.hpp>
#include <display_config.hpp>

// portal
#include <libportal/portal.h>
#include <libportal/remote.h>
#include <libportal/session.h>

// glib
#include <glib.h>

// std
#include <iostream>
#include <vector>

struct Portal {
    public:
        static void request(int& fd, int& node, const std::string& restoreToken);
        static std::vector<DisplayConfig> selectDisplays();
        static DisplayConfig selectDisplay(int index);
    private:
        static void onSessionCreated(GObject* source, GAsyncResult* res, void* userData);
        static void onSessionStarted(GObject* source, GAsyncResult* res, void* userData);

        static void onSelectSessionCreated(GObject* source, GAsyncResult* res, void* userData);
        static void onSelectSessionStarted(GObject* source, GAsyncResult* res, void* userData);
};

#endif