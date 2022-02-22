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
    lumiere.game.start = function()
        print("Hello from Lua!")
    end
)";

namespace LuM {

    int main() {

        auto lua = luaL_newstate();

        LuaBridge::OpenLibraries(lua);

        LuaBridge::AddNestedMember(lua, "game.config", [](lua_State* lua){
            lua_pushcfunction(lua, [](lua_State* lua){
                std::cout << "Hello from C++!" << std::endl;
                return 0;
            });
        });
        if(LuaBridge::GetNestedMember(lua, "game.config")) {
            lua_call(lua, 0, 0);
        }

        LuaBridge::DumpStack(lua);

        luaL_dostring(lua, code);

        if(LuaBridge::GetNestedMember(lua, "game.start")) {
            lua_call(lua, 0, 0);
        }

        lua_close(lua);

        return 0;

    }
}

int main() {
    LuM::main();
}