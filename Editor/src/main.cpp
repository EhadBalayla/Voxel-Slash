#include "core managers/Editor.h"

int main() {
    Editor editor;
    editor.Init();
    editor.Loop();
    editor.Terminate();
    return 0;
}