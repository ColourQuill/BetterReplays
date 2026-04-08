// better replays
#include <better_replays.hpp>

// NOTE
// Currently exists a slow memory leak overtime. Do intend to fix this.

int main() {
    BetterReplays app{};

    app.start();

    app.run();

    app.stop();

    return 0;
}