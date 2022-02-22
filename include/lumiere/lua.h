#pragma once

extern "C" {
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}

#include "lumiere/core.h"
#include "lumiere/utils/string.h"
#include <functional>

namespace LuM {
    namespace LuaBridge {
        /**
         * Dumps Lua stack
         * @param lua
         */
        void DumpStack(lua_State* lua) {
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
                    case LUA_TFUNCTION:
                        std::cout << "function";
                        break;
                    default:
                        std::cout << lua_topointer(lua, i);
                }
                std::cout << std::endl;
            }
        }
        void AddNestedMember(lua_State* lua, const char* memberName, std::function<void(lua_State*)> const& callback) {

            /**
             * Explode member namespace
             */
            auto scopes = Utils::String::Explode(memberName, '.');

            /**
             * Establish structure before adding literal
             * Try to get global namespace. If it doesn't exist, create it
             */
            lua_getglobal(lua, Constants::LUA_LIBRARY_NAME);
            if(lua_isnil(lua, 1)) {
                lua_pop(lua, 1);
                lua_newtable(lua);
                lua_setglobal(lua, Constants::LUA_LIBRARY_NAME);
                lua_getglobal(lua, Constants::LUA_LIBRARY_NAME);
            } else if (!lua_istable(lua, 1)) {
                throw std::runtime_error("Tried to get 'library' but it is not a table!");
            }

            /**
             * Iterate through scopes, and create them inside the global object
             */
            int iterator = 0;
            while(scopes.size() > 1) {
                iterator++;
                // Get scope name, and pop it from list
                std::string scope = scopes.back();
                scopes.pop_back();
                /**
                 * Check if the scope already exists. If it does and is not a table, throw an error
                 * If it does not, simply create it
                 */
                lua_getfield(lua, -1, scope.c_str());
                if(lua_isnil(lua, -1)) {
                    // Pop the 'nil' value from stack (we will not need it)
                    lua_pop(lua, 1);
                    // Create table
                    lua_pushstring(lua, scope.c_str());
                    lua_newtable(lua);
                    lua_settable(lua, -3);
                    // Get the field again to load it on top of the stack for next iteration
                    lua_getfield(lua, -1, scope.c_str());
                } else if (!lua_istable(lua, -1)) {
                    std::string error = "Tried to get scope '" + scope + "' but it is not a table!";
                    throw std::runtime_error(error.c_str());
                } else {
                    // Do nothing - field is currenly on top of the stack
                }
            }

            /**
             * Set literal for last member
             */
            lua_pushstring(lua, scopes.back().c_str());

            /**
             * Save stack state
            */
            const int initialStackSize = lua_gettop(lua);

            /**
             * Perform callback which should add the actual value
            */
            callback(lua);

            /**
             * Run a checksum verification to see if stack has changed. The stack size should be the same before and after 'callback' is performed to make the operation stack-safe
             */
            if(initialStackSize + 1 != lua_gettop(lua)) {
                std::string error = "Something went wrong when adding method '" + std::string(memberName) + "' (stack size is" + std::to_string(lua_gettop(lua)) + " but " + std::to_string(initialStackSize + 1) + "was expected)";
                throw std::runtime_error(error.c_str());
            } else {
                /**
                 * Set the table on the last scope
                 */
                lua_settable(lua, -3);
                /**
                 * Perform cleanup
                */
                lua_pop(lua, iterator);
                /**
                 * And finally - reset global
                 */
                lua_setglobal(lua, Constants::LUA_LIBRARY_NAME);
            }
        }
        /**
         * Traverses Lua global scope, and finds a member
         * @param lua
         * @param memberName
         * @return Returns a status flag indicating whether or not was the traverse successful
         */
        bool GetNestedMember(lua_State* lua, const char* memberName) {

            auto scopes = Utils::String::Explode(memberName, '.');

            /**
             * Get global object, or throw an error
             */
            lua_getglobal(lua, Constants::LUA_LIBRARY_NAME);
            if(lua_isnil(lua, 1)) {
                std::cerr << "Library '" << Constants::LUA_LIBRARY_NAME << "' is not defined." << std::endl;
                lua_pop(lua, 1);
                return false;
            }

            int iterator = 0;
            bool scope_found = false;
            while(scopes.size() > 1) {
                iterator++;

                std::string scope = scopes.back();
                scopes.pop_back();

                lua_getfield(lua, -1, scope.c_str());
                // if scope cannot be traversed further, break out
                if(lua_isnil(lua, -1) || !lua_istable(lua, -1)) {
                    break;
                } else {
                    if(scopes.size() == 1) {
                        scope_found = true;
                    }
                }
            }

            if(scope_found) {
                // load whatever is in the remaining field
                lua_getfield(lua, -1, scopes.at(0).c_str());
                // move the value from top of the stack behind the traversed tables
                lua_insert(lua, -(iterator + 2));
                // cleanup remaining stack values
                lua_pop(lua, iterator + 1);
                // return a flag
                return true;
            } else {
                // otherwise, just cleanup the stack and return false
                lua_pop(lua, iterator + 1);
                return false;
            }

                /*
                if(lua_isnil(lua, -1)) {
                    // Pop the 'nil' value from stack (we will not need it)
                    lua_pop(lua, 1);
                    // Create table
                    lua_pushstring(lua, scope.c_str());
                    lua_newtable(lua);
                    lua_settable(lua, -3);
                    // Get the field again to load it on top of the stack for next iteration
                    lua_getfield(lua, -1, scope.c_str());
                } else if (!lua_istable(lua, -1)) {
                    std::string error = "Tried to get scope '" + scope + "' but it is not a table!";
                    throw std::runtime_error(error.c_str());
                } else {
                    // Do nothing - field is currenly on top of the stack
                }
             */

        }
        void Call(lua_State* lua, const char* method, unsigned int nargs = 0, unsigned int nresults = 0) {
            lua_getglobal(lua, Constants::LUA_LIBRARY_NAME);
            lua_getfield(lua, -1, method);
            if(lua_isnil(lua, -1) || !lua_isfunction(lua, -1)) {
                std::cerr << "'" << method << "' is 'nil' or is not a callable member of '" << Constants::LUA_LIBRARY_NAME << "' object" << std::endl;
                // clean stack
                lua_pop(lua, 1);
            } else {
                std::cout << "'" << method << "' is a callable member" << std::endl;
                lua_call(lua, nargs, nresults);
            }
            // remove global from stack
            lua_pop(lua, 1);
        }
        /**
         * Opens standard Lua libraries while stopping Garbage collector to optimize performance
         * @param lua
         */
        void OpenLibraries(lua_State* lua) {
            lua_gc(lua, LUA_GCSTOP, 0);
            luaL_openlibs(lua);
            lua_gc(lua, LUA_GCRESTART, 0);
        }
        /**
         * Loads Main.lua file from CWD
         * @param lua
         */
        void LoadSource(lua_State* lua) {

        }
    }
}