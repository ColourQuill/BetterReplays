#ifndef __INSTANCE_HPP__
#define __INSTANCE_HPP__

// pipewire
#include <spa/param/video/format-utils.h>
#include <spa/pod/builder.h>

// std
#include <vector>

// forward declarations
struct Stream;
struct ScreenCapture;

struct Instance {
    ScreenCapture* capture = nullptr;
    int fd;

    pw_thread_loop* threadLoop;
    pw_context* context;

    pw_core* core;
    spa_hook coreListener;
    int sync_id;

    pw_registry* registry;
    spa_hook registryListener;

    std::vector<Stream*> streams;
};

#endif