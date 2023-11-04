local M = {}

if maav0 ~= nil then
    return maav0
end

---@param ms number
function M.sleep(ms)end

---@return boolean
function M.uuid() end

---@param x number
---@param y number
---@return boolean
function M.click(x, y) end

---@param x1 number
---@param y1 number
---@param x2 number
---@param y2 number
---@return boolean
function M.swipe(x1, y1, x2, y2) end

---@return boolean
function M.screencap() end

---@param name string
---@return number, number, number, number, number
function M.match_task(name) end

return M
