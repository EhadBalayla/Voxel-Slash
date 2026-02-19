
#include "core managers/app.h"

int main() {
    App app;
    app.Init();
    app.Loop();
    app.Terminate();
    return 0;
}