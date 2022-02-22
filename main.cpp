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
    print(lumiere.n, lumiere.test.n, lumiere.x)
)";

namespace LuM {

    int main() {

        auto lua = luaL_newstate();

        LuaBridge::OpenLibraries(lua);

        LuaBridge::AddNestedMember(lua, "n", [](lua_State* lua){
            lua_pushnumber(lua, 5);
        });
        LuaBridge::AddNestedMember(lua, "x", [](lua_State* lua){
            lua_pushnumber(lua, 10);
        });
        LuaBridge::AddNestedMember(lua, "test.n", [](lua_State* lua){
            lua_pushnumber(lua, 9);
        });

        luaL_dostring(lua, code);
        lua_close(lua);

        return 0;

    }
}

int main() {
    LuM::main();
}