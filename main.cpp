#include "lumiere/lib.h"
#include <memory>
#include <iostream>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "stb_image.h"

extern "C" {
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}





const char* globalFunctionName = "start";
const char* code = R"(
    print("hello from Lua")
    lua_print()
)";

namespace LuM {
    void dumpStack (lua_State* lua) {
        int top = lua_gettop(lua);
        for (int i = 1; i <= top; i++) {
            std::cout << luaL_typename(lua, i) << ": ";
            switch(lua_type(lua, i)) {
                case LUA_TNUMBER:
                    std::cout << lua_tonumber(lua, i);
                    break;
                case LUA_TSTRING:
                    std::cout << lua_tostring(lua, i);
                    break;
                case LUA_TBOOLEAN:
                    std::cout << (lua_toboolean(lua, i) ? "true" : "false");
                    break;
                case LUA_TNIL:
                    std::cout << "nil";
                    break;
                case LUA_TTABLE:
                    std::cout << "table";
                    break;
                default:
                    std::cout << lua_topointer(lua, i);
            }
            std::cout << std::endl;
        }
    }
    int lua_print(lua_State* lua)
    {
        std::cout << "hello there, Lua!" << std::endl;
        return 0;
    }
    int main() {
        auto lua = luaL_newstate();

        lua_gc(lua, LUA_GCSTOP, 0);
        luaL_openlibs(lua);
        lua_gc(lua, LUA_GCRESTART, 0);

        lua_pushcfunction(lua, lua_print);
        lua_setglobal(lua, "lua_print");

        luaL_dostring(lua, code);



        /*
        lua_pushboolean(lua, true);
        lua_pushstring(lua, "hello");
        lua_pushnumber(lua, 5);
        lua_pushnumber(lua, -5);

        lua_newtable(lua);
        lua_pushliteral(lua, "key");
        lua_pushliteral(lua, "value");

        lua_settable(lua, -3);

        dumpStack(lua);
        */

        lua_close(lua);
        return 0;

        // load code
        luaL_dostring(lua, code);

        // create new table

        lua_getglobal(lua, globalFunctionName);
        if(lua_isnil(lua, -1) && lua_isfunction(lua, -1))
        {
            // create global function
            lua_pushcfunction(lua, [](lua_State* lua){
                std::cout << "hey!" << std::endl;
                return 0;
            });
            lua_setglobal(lua, globalFunctionName);
            // get the global back onto the stack
            lua_getglobal(lua, globalFunctionName);
        }
        lua_call(lua, 0, 0);

        lua_close(lua);

        return 0;
    }
}

int main() {
    LuM::main();
}
