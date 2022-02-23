#include <memory>
#include <iostream>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "stb_image.h"

#include "lumiere/lua.h"
#include "lumiere/logger.h"

/**
 * Testing Lua script
 */
const char* code = R"(
    print("Hello from Lua")
)";

namespace LuM {
    int main() {

        Logger::GetInstance().Write("hello", LOG_INFO);

        /**
         * Lua
         */
        LuaBridge::StateOpen();
        LuaBridge::OpenLibraries();
        LuaBridge::ProtectedLoadScript(code);
        LuaBridge::StateClose();

        return 0;
    }
}

int main() {
    LuM::main();
}