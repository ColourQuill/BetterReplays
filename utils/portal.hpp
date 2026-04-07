#ifndef __PORTAL_HPP__
#define __PORTAL_HPP__

// better replays
#include <logger.hpp>

// portal
#include <libportal/portal.h>
#include <libportal/remote.h>
#include <libportal/session.h>

// glib
#include <glib.h>

// std
#include <iostream>

struct Portal {
    public:
        static void request(int& fd, int& node);
    private:
        static void onSessionCreated(GObject* source, GAsyncResult* res, void* userData);
        static void onSessionStarted(GObject* source, GAsyncResult* res, void* userData);
};

#endif