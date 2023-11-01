#include "LuaTask.h"
#include "Config/TaskData.h"
#include "Utils/WorkingDir.hpp"
#include "Vision/Matcher.h"

#include <lua.hpp>

inline void lua_add_path(lua_State* L, const std::vector<std::string>& paths)
{
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path");
    std::string cur_path = lua_tostring(L, -1);
    lua_pop(L, 1); // package.path
    for (const auto& s : paths) {
        cur_path += ";";
        cur_path += s;
    }
    lua_pushstring(L, cur_path.c_str());
    lua_setfield(L, -2 /*package*/, "path");
    lua_pop(L, 1);
}

bool asst::LuaTask::run()
{
    lua_State* l = luaL_newstate();

    luaL_openlibs(l);
    lua_add_path(l, {
                        ResDir.get().string() + "/lua/?.lua",
                        ResDir.get().string() + "/lua/?/init.lua",
                    });

    lua_newtable(l);
    lua_pushvalue(l, -1);      // duplicate it on stack
    lua_setglobal(l, "maav0"); // this pops one

    add_closure(l, [](lua_State* s, LuaTask* p) -> int {
            p->sleep(luaL_checknumber(s, 1));
            return 0;
            });
    lua_setfield(l, -2, "sleep");

    add_closure(l, [](lua_State* s, LuaTask* p) -> int {
        lua_pushstring(s, p->ctrler()->get_uuid().c_str());
        return 1;
    });
    lua_setfield(l, -2, "uuid");

    add_closure(l, [](lua_State* s, LuaTask* p) -> int {
        int x = static_cast<int>(luaL_checknumber(s, 1));
        int y = static_cast<int>(luaL_checknumber(s, 2));
        lua_pushboolean(s, static_cast<int>(p->ctrler()->click(Point { x, y })));
        return 1;
    });
    lua_setfield(l, -2, "click");

    add_closure(l, [](lua_State* s, LuaTask* p) -> int {
        int x1 = static_cast<int>(luaL_checknumber(s, 1));
        int y1 = static_cast<int>(luaL_checknumber(s, 2));
        int x2 = static_cast<int>(luaL_checknumber(s, 3));
        int y2 = static_cast<int>(luaL_checknumber(s, 4));
        lua_pushboolean(s, static_cast<int>(p->ctrler()->swipe(Point { x1, y1 }, Point { x2, y2 })));
        return 1;
    });
    lua_setfield(l, -2, "swipe");

    add_closure(l, [](lua_State* s, LuaTask* p) -> int {
        lua_pushboolean(s, static_cast<int>(p->ctrler()->screencap()));
        return 1;
    });
    lua_setfield(l, -2, "screencap");

    add_closure(l, [](lua_State* s, LuaTask* p) -> int {
        std::string task_name = luaL_checkstring(s, 1);
        Matcher matcher(p->ctrler()->get_image_cache());
        auto task_ptr = Task.get(task_name);
        if (!task_ptr) return luaL_error(s, "Task %s does not exist", task_name.c_str());
        matcher.set_task_info(task_ptr);

        if (!matcher.analyze()) return 0;
        auto result = matcher.get_result();
        lua_pushnumber(s, result.rect.x);
        lua_pushnumber(s, result.rect.y);
        lua_pushnumber(s, result.rect.width);
        lua_pushnumber(s, result.rect.height);
        lua_pushnumber(s, result.score);
        return 5;
    });
    lua_setfield(l, -2, "match_task");

    lua_pop(l, 1); // global maav0

    std::string script = R"(
print(package.path)
local maav0 = require("maav0")
print(maav0.click(100, 200))
print(maav0.swipe(200, 100, 800, 500))
print(maav0.screencap())
maav0.sleep(2000)
print(maav0.match_task("Return"))
print(maav0.match_task("Boom"))
print("done")
            )";

    int err = 0;
    err = luaL_loadbuffer(l, script.c_str(), script.size(), "script") || lua_pcall(l, 0, 0, 0);

    if (err != 0) {
        Log.error("lua:", lua_tostring(l, -1));
        lua_pop(l, 1);
    };

    lua_close(l);
    return true;
}
