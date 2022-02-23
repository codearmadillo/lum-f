#include <memory>
#include <iostream>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "stb_image.h"

#include "lumiere/lua.h"

/**
 * Testing Lua script
 */
const char* code = R"(
    print("Hello from Lua")
)";

namespace LuM {

    int main() {
        LuaBridge::StateOpen();

        LuaBridge::OpenLibraries();
        LuaBridge::ProtectedLoadScript(code);

        LuaBridge::DumpStack();

        LuaBridge::StateClose();

        return 0;
    }
}

int main() {
    LuM::main();
}