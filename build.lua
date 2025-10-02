-- Define a function to execute shell commands
local force = false
local newArgs = {}
for _, arg in ipairs(arg) do
    if arg == "-f" then
        force = true
    else
        table.insert(newArgs, arg)
    end
end

arg = newArgs
local INCLUDE_DIR = "includes/"

local function runCmd(cmd)
    print(cmd)
    local result, reason, code = os.execute(cmd)

    if result == nil or result == false or code ~= 0 then
        print("Command failed: " .. (reason or "unknown") .. " with code " .. (code or "unknown"))
        os.exit(1)
    end
end

local function fileModified(filename)
    local f = io.popen("stat -c %Y " .. filename .. " 2>/dev/null")
    local modifiedTime = f:read("*a")
    f:close()
    return tonumber(modifiedTime)
end

local function needsRebuild(source, target)
    if force then
        return true
    end
    local sourceTime = fileModified(source)
    local targetTime = fileModified(target)
    if sourceTime == nil or targetTime == nil then
        return true
    end
    return sourceTime > targetTime
end

local function isDirectory(path)
    local cmd = "test -d " .. path .. " && echo yes || echo no"
    local handle = io.popen(cmd)
    local result = handle:read("*a"):gsub("%s+", "") -- Trim whitespace
    handle:close()
    return result == "yes"
end

local function getFilesRecursively(dir)
    local files = {}
    
    local function traverseDirectory(currentDir)
        local handle = io.popen("ls -A1 " .. currentDir .. " 2>/dev/null")
        local listing = handle:read("*a")
        handle:close()
        
        for file in listing:gmatch("[^\r\n]+") do
            local filePath = currentDir .. "/" .. file
            if isDirectory(filePath) then
                traverseDirectory(filePath)
            else
                table.insert(files, filePath)
            end
        end
    end
    
    traverseDirectory(dir)
    return files
end

local function directoryNeedsRebuild(sourceDir, target)
    if force then
        return true
    end
    local targetTime = fileModified(target)
    if targetTime == nil then
        return true
    end
    for _, file in ipairs(getFilesRecursively(sourceDir)) do
        local fileTime = fileModified(file)
        if fileTime and fileTime > targetTime then
            return true
        end
    end
    return false
end

-- Array to hold targets
local targets = {}

local function buildStr(target)
    local selectedTarget = ""
    for argId = 1, #target do
        selectedTarget = selectedTarget .. target[argId]
        if argId < #target then selectedTarget = selectedTarget .. " " end
    end
    return selectedTarget
end

local function build(selectedTarget)
	if not isDirectory("bin") then runCmd("mkdir bin") end

    for _, target in ipairs(targets) do
        if target.name == selectedTarget then
            target.exec()
        end
    end
end

local function Target(name, dependsOn, fn, description)
    local self = {}
    self.name = name
    self.dependsOn = dependsOn
    self.fn = fn
    self.description = description or "target for " .. name
    self.exec = function()
        for _, dep in ipairs(self.dependsOn) do
            build(dep)
        end
        local success, err = pcall(self.fn)
        if not success then
            print(err)
        end
    end

    table.insert(targets, self)
    return self
end

-- Define targets
Target("rocket executable", {"fs.so", "raylib.so","curses.so"}, function()
    if needsRebuild("src/main.cpp", "bin/rocket") or needsRebuild("src/funcs.cpp", "bin/rocket") then
        print("Compiling rocket...")
        runCmd("clang++ -I"..INCLUDE_DIR.." src/main.cpp -o bin/rocket -llua -llua++ -lraylib")
    end
end, "rocket, or rocket, is a C++ executable that wraps functionality on top of Lua")

-- raylib.cpp includes libs/raygui/raygui.cpp
-- so you can use it with default Lua
Target("raylib.so", {}, function()
    if directoryNeedsRebuild("libs/raylib/", "bin/raylib.so") or directoryNeedsRebuild("libs/raygui/","bin/raylib.so") then
        print("Compiling raylib.so...")
        runCmd("clang++ -I"..INCLUDE_DIR.." libs/raylib/raylib.cpp -o bin/raylib.so -shared -fPIC -llua -llua++ -lraylib")
    end
end, "Compiles the raylib.so (with raygui) that you can use with default Lua")


Target("fs.so", {}, function()
    if needsRebuild("libs/fs/fs.cpp", "bin/fs.so") then
        print("Compiling fs.so...")
        runCmd("clang++ -I"..INCLUDE_DIR.." libs/fs/fs.cpp -o bin/fs.so -shared -fPIC -llua -llua++")
    end
end, "Compiles the fs.so that you can use with default Lua")

Target("all", {"rocket executable"}, function()
    -- Placeholder function, as per the original code
end, "Builds everything")

Target("install", {"all"}, function()
    if needsRebuild("bin/rocket", "~/bin/rocket") then
        runCmd("mkdir -p ~/bin")
        runCmd("cp bin/rocket ~/bin/rocket")
    end
end, "Installs rocket")

Target("curses.so", {},function()
    if directoryNeedsRebuild("libs/ncurses","bin/curses.so") then
        runCmd("clang++ -I"..INCLUDE_DIR.." libs/ncurses/curses.cpp -o bin/curses.so -shared -llua -llua++ -lncurses")
    end
end, "ncurses module")

Target("clean", {}, function()
    print("Cleaning build artifacts...")
    runCmd("rm -rf bin/*")
end, "Removes all built files from the bin directory")

Target("help", {}, function()
    print("Usage: [lua/rocket] build.lua [target]")
    print("Targets:")
    for _, target in ipairs(targets) do
        print("- " .. target.name)
        print(" ^- " .. target.description)
        for _, dep in ipairs(target.dependsOn) do
            print("  ^- depends On: " .. dep .. ".")
        end
        print("")
    end
end, "Shows a help message")

local selectedTarget = buildStr(arg)


build(selectedTarget)

-- this is just so rocket doesn't error out due to no main function being found
function main()
end
