-- Moemon Fight: what the fight engine needs from Lua: the Vector class it keeps character
-- constants and positions in (Vector:new / vec2 / vec1 give x, y, z, unitsize, dimensions).

Vector = {}
Vector.__index = Vector

function Vector:new(x, y, z, unitsize)
  return setmetatable({ x = x or 0, y = y or 0, z = z or 0, unitsize = unitsize or 1, dimensions = 3 }, Vector)
end

function Vector:vec2(x, y, unitsize)
  local v = Vector:new(x, y, 0, unitsize)
  v.dimensions = 2
  return v
end

function Vector:vec1(x, unitsize)
  local v = Vector:new(x, 0, 0, unitsize)
  v.dimensions = 1
  return v
end
