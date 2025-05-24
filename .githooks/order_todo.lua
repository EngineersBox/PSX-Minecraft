-- Meta Methods

local strdefi = getmetatable('').__index
getmetatable('').__index = function(str,i)
    if type(i) == "number" then
        return string.sub(str,i,i)
    else
        return strdefi[i]
    end
end

-- Ordered table implementation from: <https://stackoverflow.com/a/30970276>
ordered_table = {}

function ordered_table.insert(t, k, v)
  if not rawget(t._values, k) then -- new key 
    t._keys[#t._keys + 1] = k
  end
  if v == nil then -- delete key too.
    ordered_table.remove(t, k)
  else -- update/store value
    t._values[k] = v 
  end
end

local function find(t, value)
  for i,v in ipairs(t) do
    if v == value then
      return i
    end
  end
end  

function ordered_table.remove(t, k)
  local v = t._values[k]
  if v ~= nil then
    table.remove(t._keys, find(t._keys, k))
    t._values[k] = nil
  end
  return v
end

function ordered_table.index(t, k)
    return rawget(t._values, k)
end

function ordered_table.pairs(t)
  local i = 0
  return function()
    i = i + 1
    local key = t._keys[i]
    if key ~= nil then
      return key, t._values[key]
    end
  end
end

function ordered_table.new(init)
  init = init or {}
  local t = {_keys={}, _values={}}
  local n = #init
  if n % 2 ~= 0 then
    error("in ordered_table initialization: key is missing value")
  end
  for i=1,n/2 do
    local k = init[i * 2 - 1]
    local v = init[i * 2]
    if t._values[k] ~= nil then
      error("duplicate key:"..k)
    end
    t._keys[#t._keys + 1]  = k
    t._values[k] = v
  end
  return setmetatable(t,
    {__newindex=ordered_table.insert,
    __len=function(t) return #t._keys end,
    __pairs=ordered_table.pairs,
    __index=t._values
    })
end

-- Utilities

local stringTrim = function(s, c)
   return (s:gsub("^" .. c .. "*(.-)" .. c .. "*$", "%1"))
end

-- Logic

local sections = ordered_table.new({});
local current_section = "";
local current_content = "";
local current_checked = "";

local todo_or_empty_line_pattern = "(^#%s)|(^[%s]+$)";
local section_header_pattern = "^##";
local list_item_pattern = "^%*%s%[";
local list_item_continuation_pattern = "^%s+[^%s]+.*$";

local infile = assert(io.open("TODO.md", "r"), "Unable to open TODO.md to read");
for l in infile:lines("*l") do
    if l:find(todo_or_empty_line_pattern) ~= nil then
        goto continue;
    elseif l:find(section_header_pattern) ~= nil then
        current_section = stringTrim(l, "\r");
        ordered_table.insert(
            sections,
            current_section, {
                checked = {},
                pending = {},
                unchecked = {}
            }
        );
    elseif l:find(list_item_pattern) ~= nil then
        current_checked = string.lower(l[4])
        local c = "unchecked";
        if current_checked == "x" then
            c = "checked"
        elseif current_checked == "-" then
            c = "pending"
        end
        current_content = stringTrim(l, "\r");
        table.insert(sections[current_section][c], current_content);
    elseif l:find(list_item_continuation_pattern) ~= nil then
        local c = "unchecked";
        if current_checked == "x" then
            c = "checked"
        elseif current_checked == "-" then
            c = "pending"
        end
        local target_section = sections[current_section][c];
        target_section[#target_section] = current_content .. " " .. stringTrim(stringTrim(l, "%s"), "\r");
    end
    ::continue::
end
infile:close();

local outfile = assert(io.open("TODO.md", "w+b"), "Unable to open TODO.md to write");
outfile:write("# TODO\n");
for section_name, section in pairs(sections) do
    outfile:write("\n" .. section_name .. "\n\n");
    if section.checked ~= nil then
        for _, checked in ipairs(section.checked) do
            outfile:write(checked .. "\n");
        end
    end
    if section.pending ~= nil then
        for _, pending in ipairs(section.pending) do
            outfile:write(pending .. "\n");
        end
    end
    if section.unchecked ~= nil then
        for _, unchecked in ipairs(section.unchecked) do
            outfile:write(unchecked .. "\n");
        end
    end
end
outfile:close();
