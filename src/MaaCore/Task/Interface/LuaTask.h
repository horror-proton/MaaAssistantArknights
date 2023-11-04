#pragma once
#include "Controller/Controller.h"
#include "Task/InterfaceTask.h"
#include "Utils/Logger.hpp"

#include <lua.hpp>

namespace asst
{
    class LuaTask : public InterfaceTask
    {
    public:
        inline static constexpr std::string_view TaskType = "Custom";

        LuaTask(const AsstCallback& callback, Assistant* inst) : InterfaceTask(callback, inst, TaskType) {}
        ~LuaTask() override = default;

        bool run() override;

        bool set_params(const json::value &params) override;

    private:
        template <typename Func>
        void add_closure(lua_State* L, Func&&)
        {
            lua_pushlightuserdata(L, this);
            lua_pushcclosure(
                L,
                [](lua_State* s) -> int {
                    const void* upval = lua_topointer(s, lua_upvalueindex(1));
                    auto* p = const_cast<LuaTask*>(reinterpret_cast<const LuaTask*>(upval));
                    return std::invoke(Func {}, s, p);
                },
                1);
        }
    };
} // namespace asst
