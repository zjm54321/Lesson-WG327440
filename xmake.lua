add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = "build"})

-- 芯片内存参数（按需改）
local xram_size = 0  
local iram_size = 256  
local code_size = 8192 

-- 工具链/架构
set_toolchains("sdcc")
set_arch("mcs51")

option("chip")
    -- 默认 8051
    set_default("STC89")
    set_showmenu(true)
    set_description("Select chip model")
    -- 可选值列表，必须与 stc51.h 中的宏一致
    set_values(
        "MCS51",
        -- STC 系列
        "STC89" , "STC90" , "STC90AD" , "STC10" , "STC11" , "STC12C52" , "STC12C5A" ,
        "STC12C20" , "STC12C54" , "STC12C56" , "STC12H" , "STC15F104E" , "STC15F204EA" ,
        "STC15" , "STC8A" , "STC8F" , "STC8C" , "STC8G" , "STC8H" , "STC8A8K64D4"
    )
option_end()

-- 库目标：公开 include/ 头，编译 lib/ 源码
target("lib")
    set_kind("static")
    add_includedirs("include", {public = true})
    add_includedirs("lib/headers", {public = true})
    add_includedirs("lib", {public = false})
    add_defines(get_config("chip"))
    add_files("lib/**.c")

-- 应用目标：依赖库并链接
target("mcs51")
    set_kind("binary")
    set_extension(".ihx")
    add_deps("lib")
    add_files("src/**.c")
    add_defines(get_config("chip"))
    -- SDCC 相关优化/链接选项（按需启用/调整）
    add_ldflags("--model-small")
    add_ldflags("--xram-size "..xram_size)
    add_ldflags("--iram-size "..iram_size)
    add_ldflags("--code-size "..code_size)
    add_ldflags("--opt-code-size")
    add_cflags("--opt-code-size")

    before_build(function (target)
        local task = import("core.project.task")
        task.run("format")
    end)

    after_build(function (target)
        os.cp(target:targetfile(), "build/")
    end)

task("clean")
    on_run(function ()
        os.tryrm("build")
        print("Clean all build files.")
    end)
    set_menu {
        usage = "xmake clean",
        description = "Clean all build files.",
    }

task("format")
    on_run(function ()
        import("core.base.option")
        import("lib.detect.find_tool")
        
        local tool = find_tool("clang-format")
        
        local files = option.get("files") or {"src/**.c", "include/**.h|include/headers/**.h" , "lib/**.h|lib/headers/**.h"}
        for _, pattern in ipairs(files) do
            local filelist = os.files(pattern)
            for _, file in ipairs(filelist) do
                os.execv(tool.program, {"-i", file})
            end
        end
        print("Formatted files.")
    end)
    
    set_menu {
        usage = "xmake format [options]",
        description = "格式化代码文件",
        options = {
            {'f', "files", "vs", nil, "要格式化的文件模式"}
        }
    }

task("flash")
    on_run(function ()
        os.exec("xmake build")
        import("core.project.config")
        config.load()
        local chip = get_config("chip")
        import("core.base.option")

        if chip:startswith("STC") then
            tool = "stcflash"
        else
            raise("Unsupported chip for flashing: " .. chip)
        end

        local port = option.get("port") or "/dev/ttyUSB0"
        local bin = "$(builddir)/mcs51.ihx"

        import("privilege.sudo")
        sudo.run("%s -p %s %s", tool, port, bin)

    end)

    set_menu {
        usage = "xmake flash [options]",
        description = "Flash the compiled binary to the microcontroller",
    }

task("emulate")
    on_run(function ()
        os.exec("xmake build")
        local bin = "$(builddir)/mcs51.ihx"
        os.exec("emu8051 " .. bin)
    end)

    set_menu {
        usage = "xmake emulate",
        description = "Run the compiled binary in an 8051 emulator",
    }