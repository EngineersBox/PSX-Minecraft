local strdefi = getmetatable('').__index
getmetatable('').__index = function(str,i)
    if type(i) == "number" then
        return string.sub(str,i,i)
    else
        return strdefi[i]
    end
end

local stringTrim = function(s, c)
   return (s:gsub("^" .. c .. "*(.-)" .. c .. "*$", "%1"))
end

local sections = {};
local current_section = "";
local current_content = "";
local current_checked = false;

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
        sections[current_section] = {
          checked = {},
          unchecked = {}
        };
    elseif l:find(list_item_pattern) ~= nil then
        current_checked = l[4] == "x"
        local c = "unchecked";
        if current_checked then
            c = "checked"
        end
        current_content = stringTrim(l, "\r");
        table.insert(sections[current_section][c], current_content);
    elseif l:find(list_item_continuation_pattern) ~= nil then
        local c = "unchecked";
        if current_checked then
            c = "checked"
        end
        local target_section = sections[current_section][c];
        target_section[#target_section] = current_content .. " " .. stringTrim(stringTrim(l, "%s"), "\r");
    end
    ::continue::
end
infile:close();

local outfile = assert(io.open("TODO.md", "w+b"), "Unable to open TODO.md to write");
outfile:write("# TODO");
for section_name, section in pairs(sections) do
    outfile:write("\n\n" .. section_name .. "\n\n");
    for _, checked in ipairs(section.checked) do
        outfile:write(checked .. "\n");
    end
    for _, unchecked in ipairs(section.unchecked) do
        outfile:write(unchecked .. "\n");
    end
end
outfile:close();
