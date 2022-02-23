#include <memory>
#include <iostream>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "stb_image.h"

#include "lumiere/lua.h"
#include "lumiere/logger.h"

namespace LuM {
    int main() {
        LuaBridge::StateOpen();
        LuaBridge::OpenLibraries();
        LuaBridge::ProtectedExecuteSource();
        LuaBridge::StateClose();

        return 0;
    }
}

int main() {
    LuM::main();
}