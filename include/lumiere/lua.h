#pragma once

extern "C" {
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}

#include "lumiere/core.h"
#include "lumiere/utils/string.h"
#include <functional>
#include <filesystem>

namespace LuM {
    namespace LuaBridge {
        class StateFactory {
            public:
                StateFactory(StateFactory const&) = delete;
                void operator=(StateFactory const&) = delete;
                static StateFactory& GetInstance() {
                    static StateFactory instance;
                    return instance;
                }
                static void* LuaAllocator(void* ud, void* ptr, size_t osize, size_t nsize)
                {
                    if (nsize == 0) {
                        free(ptr);
                        return nullptr;
                    }
                    return realloc(ptr, nsize);
                }
                lua_State* lua;
            private:
                StateFactory(): lua(nullptr) { }
        };
        void StateOpen() {
            if(StateFactory::GetInstance().lua == nullptr) {
                StateFactory::GetInstance().lua = lua_newstate(StateFactory::LuaAllocator, nullptr);
            } else {
                std::cerr << "Attempted to open lua_State but such state was already open." << std::endl;
            }
        }
        lua_State* StateGet() {
            if(StateFactory::GetInstance().lua == nullptr) {
                throw std::runtime_error("Attempted to get lua_State but not state was open.");
            }
            return StateFactory::GetInstance().lua;
        }
        void StateClose() {
            lua_close(StateGet());
        }
        /**
         *
         * Dumps Lua stack
         */
        void DumpStack() {
            const auto lua = LuaBridge::StateGet();
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
        /**
         * Adds a scoped member
         * @param memberName
         * @param callback Used to push the actual value of the member. Ensure this operation is stack-safe (the result should be increase 1 in stack size)
         */
        void AddNestedMember(const char* memberName, std::function<void(lua_State*)> const& callback) {

            const auto lua = LuaBridge::StateGet();

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
         * @param memberName
         * @return Returns a status flag indicating whether or not was the traverse successful
         */
        bool GetNestedMember(const char* memberName) {

            const auto lua = LuaBridge::StateGet();
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
        /**
         * Opens standard Lua libraries while stopping Garbage collector to optimize performance
         * @param lua
         */
        void OpenLibraries() {
            const auto lua = LuaBridge::StateGet();
            lua_gc(lua, LUA_GCSTOP, 0);
            luaL_openlibs(lua);
            lua_gc(lua, LUA_GCRESTART, 0);
        }
        /**
         * Performs a protected call and correctly reports errors. Lua expects that everything relevant was pushed onto the stack at this point
         * @param nargs
         * @param nresults
         */
        void ProtectedCall(unsigned int nargs = 0, unsigned int nresults = 0) {
            const auto lua = LuaBridge::StateGet();
            const int result = lua_pcall(lua, nargs, nresults, 0);
            if(result == LUA_OK) {
                return;
            } else {
                const char* error = lua_tostring(lua, -1);
                if(error == nullptr) {
                    switch(result) {
                        case LUA_ERRRUN:
                            luaL_traceback(lua, lua, "Unexpected runtime error occurred", 1);
                            break;
                        case LUA_ERRMEM:
                            luaL_traceback(lua, lua, "Unexpected memory error occurred", 1);
                            break;
                        case LUA_ERRERR:
                            luaL_traceback(lua, lua, "Unexpected error occurred while running error delegate", 1);
                            break;
                    }
                } else {
                    luaL_traceback(lua, lua, error, 1);
                }
                lua_pop(lua, 1);
            }
        }
        /**
         * Loads explicit string onto Lua stack and executes it
         * @param script
         */
        void ProtectedLoadScript(const char* script) {
            const auto lua = LuaBridge::StateGet();
            if(luaL_dostring(lua, script) != LUA_OK) {
                std::string error = lua_tostring(lua, -1);
                lua_pop(lua, 1);
                std::cerr << error << std::endl;
            }
        }
        /**
         * Loads Main.lua from CWD and executes it
         */
        void ProtectedExecuteSource() {

            const auto lua = LuaBridge::StateGet();

            std::filesystem::path dir(std::filesystem::current_path());
            std::filesystem::path file("main.lua");

            if(luaL_dofile(lua, (dir / file).string().c_str()) != LUA_OK) {
                std::string error = lua_tostring(lua, -1);
                lua_pop(lua, 1);
                std::cerr << error << std::endl;
            }

        }
    }
}