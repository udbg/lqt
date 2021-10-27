
add_rules("mode.debug", "mode.release")

add_requires("lua 5.4", {configs = {shared = true}})

local function generate_target(...)
    local args = {...}
    local name = args[1]:lower()

    local QT_LIBRARIES = {}
    for i, n in ipairs(args) do
        QT_LIBRARIES[i] = n:gsub('^Qt', '')
    end

    target(name)
        add_rules("qt.shared")
        add_frameworks(...)
        add_packages('lua')

        if is_plat 'windows' then
            add_defines(
                '_CRT_SECURE_NO_DEPRECATE',
                '_CRT_SECURE_NO_WARNINGS',
                '_SCL_SECURE_NO_WARNINGS',
                'LUA_BUILD_AS_DLL',
                '_XKEYCHECK_H'
            )
        end
        add_defines 'LUA_COMPAT_5_3'
        add_defines('MODULE_'..name)
        if is_mode 'release' then
            set_symbols("debug")
            set_optimize("smallest")
        end

        if is_mode 'debug' then
            set_symbols("debug")
            set_runtimes("MDd")
        end

        -- local LQT_GEN_DIR = '$(buildir)/' .. name .. '_src'
        local LQT_GEN_DIR = 'build/' .. name .. '_src'
        on_load(function(target)
            os.mkdir(LQT_GEN_DIR)

            local GEN_XML = LQT_GEN_DIR .. '/' .. name .. '.xml'
            if not os.exists(GEN_XML) then
                local ABS = path.absolute
                local QT_SCHEMA_DIR = 'generator/schema'
                os.execv('cpptoxml', {
                    args[1], '-C', QT_SCHEMA_DIR .. '/lqt5.lqt',
                    '-Q', QT_SCHEMA_DIR, '-o', GEN_XML,
                })
                local types = {}
                for i = 2, #args do
                    local DEPNAME = args[i]:lower()
                    table.insert(types, ABS('build/'..DEPNAME..'_src/'..DEPNAME..'_types.lua'))
                end

                local gen_args = {
                    ABS 'generator/generator.lua',
                    ABS(GEN_XML),
                    '-i', args[1], '-i', 'lqt_qt.hpp',
                    '-i', table.concat(QT_LIBRARIES, ';'),
                    '-n', name,
                    '-t', ABS 'generator/qtypes.lua',
                    -- '-t', types,
                    '-f', ABS 'generator/qt_internal.lua',
                }
                for _, t in ipairs(types) do
                    table.insert(gen_args, #gen_args - 1, '-t')
                    table.insert(gen_args, #gen_args - 1, t)
                end
                os.cd 'build'
                os.execv('lua', gen_args)
                os.cd '..'
            end
        end)

        add_includedirs('common')
        add_files(LQT_GEN_DIR .. '/' .. name .. '_slot.hpp')    -- for moc_*.cpp
        add_files(LQT_GEN_DIR .. '/' .. name .. '_slot.cpp')

        if name == 'qtcore' then
            add_files 'common/*.cpp'
        else
            add_deps 'qtcore'
            add_files 'common/*.cpp|lqt_common.cpp'
        end

        if name == 'qtuitools' then
            set_runtimes 'MD'
        end

        add_files(LQT_GEN_DIR .. '/*.cpp')
end

generate_target('QtCore')
generate_target('QtGui', 'QtCore')
generate_target('QtWidgets', 'QtGui', 'QtCore')
generate_target('QtUiTools', 'QtWidgets', 'QtGui', 'QtCore')