module('virtuals', package.seeall)

--- Retrieves the virtual method for each class. Also retrieves the virtual
-- methods for all superclasses.
function fill_virtuals(classes)
	local byname = {}
	for c in pairs(classes) do
		byname[c.xarg.fullname] = c
	end
	-- check if func exists in list
	local function exists(list, func)
		for _,f in ipairs(list) do
			if f == func then
				return true
			end
		end
		return false
	end

	local function get_virtuals(c, includePrivate)
		local methods = {}
		for _, f in ipairs(c) do
			if f.label=='Function' and f.xarg.virtual ~= '1' then
				if not methods[f.xarg.name] then
					methods[f.xarg.name] = { f }
				-- do not insert same function twice
				elseif not exists(methods[f.xarg.name], f) then
					table.insert(methods[f.xarg.name], f)
				end
			end
		end

		local ret = {}
		local function add_overload(name, func)
			-- support for multiple virtual overload functions
			assert(func.xarg)
			if not ret[name] then
				ret[name] = { func }
			-- do not insert same function twice
			elseif not exists(ret[name], func) then
				table.insert(ret[name], func)
			end
		end

		-- add virtual methods declared in the class
		for _, f in ipairs(c) do
			if f.label=='Function' and f.xarg.virtual=='1' then
				local n = string.match(f.xarg.name, '~') or f.xarg.name
				if n~='~' and n~='metaObject' then
					add_overload(n, f)
				end
			end
		end

		-- find virtual methods in base classes
		for b in string.gmatch(c.xarg.bases or '', '([^;]+);') do
			local base = byname[b]
			if type(base)=='table' then
				local bv = get_virtuals(base, true)
				for n, list in pairs(bv) do
					-- print('found', n, 'in', b, 'for', c.xarg.name, 'have', not not ret[n])
					if not ret[n] then
						if methods[n] then
							-- print('has', n, 'which is not marked virtual')
							for _,f in ipairs(methods[n]) do
								f.xarg.virtual = '1'
								add_overload(n, f)
							end
						else
							for _,f in ipairs(list) do
								-- print('does not have', n)
								add_overload(n, f)
							end
						end
					end
				end
			end
		end

		-- mark methods in class not explicitly marked as virtual, as C++
		-- does not require that overriden methods are marked virtual
		for _, f in ipairs(c) do
			local n = string.match(f.xarg.name, '~') or f.xarg.name
			if f.label=='Function'
				and (includePrivate or f.xarg.access~='private')
				and ret[n]
			then
				-- print('adding', c.xarg.name, n)
				f.xarg.virtual = '1'
				add_overload(n, f)
			end
		end

		local virtual_index = 0
		for n, list in pairs(ret) do
			for _,f in ipairs(list) do
				f.virtual_index = virtual_index
			end
			virtual_index = virtual_index + 1
		end

		return ret, virtual_index
	end
	for c in pairs(classes) do
		c.virtuals, c.nvirtuals = get_virtuals(c)
	end
end

function parse_return_type(return_type)
	if not return_type then
		return ''
	elseif return_type:find('%*$') then
		return 'nullptr';
	end

	local map = {
		['bool'] = 'false',
		['int'] = '-1',
		['long long'] = '-1',
		['unsigned int'] = 0,
	}

	if not map[return_type] and not return_type:find('^Q') then
		print('return_type', return_type)
	end
	return map[return_type] or (return_type .. '()')
end

--- Generates a virtual overload for function 'v'.
-- Returns nil if a parameter or return type is of unknown/ignored type. Normal
-- virtual methods call original virtual method if no corresponding Lua function is
-- found, pure virtual (abstract) methods throw Lua error.
function virtual_overload(v)
	if v.virtual_overload then return v end
	-- make return type
	if v.return_type and not typesystem[v.return_type] then
		ignore(v.xarg.fullname, 'unknown return type', v.return_type)
		return nil, 'return: '..v.return_type
	end
	local ret = ''
	local rget, rn, ret_as = '', 0
	if v.return_type then rget, rn, ret_as = typesystem[v.return_type].get'oldtop+2' end
	local retget = ''
	if v.return_type then
		local atest, an = typesystem[v.return_type].test('oldtop+2')
		retget = [[if (!(]]..atest..[[)) {
        luaL_error(L, "Unexpected virtual method return type: %s; expecting %s\nin: %s",
          lqtL_typename(L,oldtop+2), "]]..v.return_type..[[", lqtL_source(L,oldtop+1));
      }
      ]]
		retget = retget .. argument_name(ret_as or v.return_type, 'ret') .. ' = ' .. rget .. ';\n      '
	end
	retget = retget .. 'lua_settop(L, oldtop);\n      return' .. (v.return_type and ' ret' or '')
	-- make argument push
	local pushlines, stack = make_pushlines(v.arguments)
	if not pushlines then
		ignore(v.xarg.fullname, 'unknown argument type', stack)
		return nil, 'argument: '..stack
	end
	-- make lua call
	local luacall = 'lqtL_pcall(L, '..(stack+1)..', '..rn..', 0)'
	-- make prototype and fallback
	local proto = (v.return_type or 'void')..' ;;'..v.xarg.name..' ('
	local fallback = ''
	for i, a in ipairs(v.arguments) do
		proto = proto .. (i>1 and ', ' or '')
		.. argument_name(a.xarg.type_name, 'arg'..i)
		fallback = fallback .. (i>1 and ', arg' or 'arg') .. i
	end
	proto = proto .. ')' .. (v.xarg.constant=='1' and ' const' or '')
	fallback = (v.return_type and 'return this->' or 'this->') .. v.xarg.fullname .. '(' .. fallback .. ');' ..
				(v.return_type and '' or ' return;')
	local fallback_thread = fallback
	if v.xarg.abstract then
		fallback = string.format([[luaL_error(L, "Abstract method %%s:%%s not implemented!"
				, "%s"
				, "%s"
			);
			return %s;
]]
			, v.xarg.member_of_class
			, v.xarg.name
			, parse_return_type(v.return_type)
		)

		fallback_thread = string.format('return %s;', parse_return_type(v.return_type))
	end
	ret = proto .. ' {\n'
	if VERBOSE_BUILD then
		ret = ret .. '  printf("[%p; %p] call virtual %s :: %s (%d) => %d\\n", ' ..
				'QThread::currentThreadId(), this, ' ..
				'"'..v.xarg.member_of_class.. '", ' ..
				'"'..v.xarg.name..'", '..
				'VIRTUAL_INDEX, '..
				'(int)(bool)hasOverride[VIRTUAL_INDEX]'..
				');\n'
	end

	-- add thread check code
	--	print warning when override virtual method in lua and calling it in a child thread
	ret = ret .. string.format([[  if (!lqtL_isMainThread()) {
    if (hasOverride[VIRTUAL_INDEX])
      printf("Warning: call virtual %s::%s() from thread!\n");
    %s
  }
]]
			, v.xarg.member_of_class
			, v.xarg.name
			, fallback_thread
)

	ret = ret .. '  int oldtop = lua_gettop(L);\n'
	ret = ret .. '  if (!hasOverride[VIRTUAL_INDEX]) { \n'
	ret = ret .. '    ' .. fallback .. '\n  }\n'
	if VERBOSE_BUILD then
		ret = ret .. '  printf("[%p; %p] lua virtual %s :: %s (%d) => %d\\n", ' ..
				'QThread::currentThreadId(), this, ' ..
				'"'..v.xarg.member_of_class.. '", ' ..
				'"'..v.xarg.name..'", '..
				'VIRTUAL_INDEX, '..
				'(int)(bool)hasOverride[VIRTUAL_INDEX]'..
				');\n'
	end
	ret = ret .. [[
  lqtL_pushudata(L, this, "]]..string.gsub(v.xarg.member_of_class, '::', '.')..[[*");
  lqtL_getoverload(L, -1, "]]..v.xarg.name..[[");
  lua_pushvalue(L, -1); // copy of function
  if (lua_isfunction(L, -1)) {
    lua_insert(L, -3);
    lua_insert(L, -3);
]] .. pushlines .. [[
    if (!]]..luacall..[[) {
      ]]..retget..[[;
    } else {
      if (lqtL_is_super(L, lua_gettop(L))) {
        lua_settop(L, oldtop);
        ]]..fallback..[[ 
      } else
        lua_error(L);
        ]]..fallback.. -- [avoid] warning: control may reach end of non-void function [-Wreturn-type]'
[[

      }
  } else {
    lua_settop(L, oldtop);
    ]]..fallback..[[
  }
}
]]
	v.virtual_overload = ret
	v.virtual_proto = string.gsub(proto, ';;', '', 1)
	return v
end



function fill_virtual_overloads(classes)
	for c in pairs(classes) do
		if c.virtuals then
			local vidx = 0
			for i, list in pairs(c.virtuals) do
				for _,v in ipairs(list) do
					if v.xarg.access~='private' then
						local vret, err = virtual_overload(v)
						if not vret and v.xarg.abstract then
							-- cannot create instance without implementation of an abstract method
							c.abstract = true
						end
					end
				end
			end
		end
	end
end



function fill_shell_class(c)
	local shellname = 'lqt_shell_'..c.xarg.cname
	local shell = 'class ' .. shellname .. ' : public ' .. c.xarg.fullname .. ' {\n'
	shell = shell .. '  lua_State *L;\n'
	shell = shell .. '  ::QBitArray hasOverride;\n'
	shell = shell .. 'public:\n'
	shell = shell .. '  static int lqtAddOverride(lua_State *L);\n'
	for _, constr in ipairs(c.constructors) do
		if constr.xarg.access~='private' then
			local cline = '  '..shellname..' (lua_State *l'
			local argline = ''
			for i, a in ipairs(constr.arguments) do
				cline = cline .. ', ' .. argument_name(a.xarg.type_name, 'arg'..i)
				argline = argline .. (i>1 and ', arg' or 'arg') .. i
			end
			cline = cline .. ') : ' .. c.xarg.fullname
				.. '(' .. argline .. '), L(l), hasOverride(' .. c.nvirtuals .. ') '
				.. '{\n    lqtL_register(L, this, "' .. c.xarg.fullname .. '*");\n'
			if c.protected_enums then
				cline = cline .. '    registerEnums();\n'
			end
			cline = cline .. '  }\n'
			shell = shell .. cline
		end
	end
	if c.copy_constructor==nil and c.public_constr then
		local cline = '  '..shellname..' (lua_State *l, '..c.xarg.fullname..' const& arg1)'
		cline = cline .. ' : ' .. c.xarg.fullname .. '(arg1), L(l) {}\n'
		shell = shell .. cline
	end
	for i, list in pairs(c.virtuals) do
		for _,v in ipairs(list) do
			if v.xarg.access~='private' then
				if v.virtual_proto then shell = shell .. '  virtual ' .. v.virtual_proto .. ';\n' end
			end
		end
	end
	shell = shell .. '  ~'..shellname..'() { lqtL_unregister(L, this, "' .. c.xarg.fullname .. '*"); }\n'
	if c.shell and c.qobject then
		shell = shell .. '  virtual const QMetaObject *metaObject() const;\n'
		shell = shell .. '  virtual int qt_metacall(QMetaObject::Call, int, void **);\n'
		shell = shell .. 'private:\n'
		shell = shell .. '      Q_DISABLE_COPY('..shellname..');\n'
	end
	if c.protected_enums then
		shell = shell .. '  void registerEnums() {\n'
		for _,e in ipairs(c.protected_enums) do
			shell = shell .. e.enum_table
			shell = shell .. '    lqtL_createenum(L, lqt_enum'..e.xarg.id..', "'..string.gsub(e.xarg.fullname, "::", ".")..'");\n'
		end
		shell = shell .. '  }\n'
	end
	shell = shell .. '};\n'
	c.shell_class = shell
	return c
end


function fill_shell_classes(classes)
	for c in pairs(classes) do
		if c.shell then
			local nc = fill_shell_class(c)
			if not nc then
				 -- FIXME: useless, but may change
				ignore(c.xarg.fullname, 'failed to generate shell class')
				classes[c] = nil
			end
		end
	end
end

----------------------------------------------------------------------

function print_shell_classes(classes)
	for c in pairs(classes) do
		local n = c.xarg.cname
		local fhead = assert(io.open(module_name.._src..module_name..'_head_'..n..'.hpp', 'w'))
		local print_head = function(...)
			fhead:write(...)
			fhead:write'\n'
		end
		print_head('#ifndef LQT_HEAD_'..n)
		print_head('#define LQT_HEAD_'..n)
		-- print_head('/* ugly ugly ugly, but needed to access protected members from outside */')
		-- print_head('#define protected public')
		print_head(output_includes)
		--print_head('#include <'..string.match(c.xarg.fullname, '^[^:]+')..'>')
		print_head''
		if c.shell then
			print_head('#include "'..module_name..'_slot.hpp'..'"\n\n')
			if c.shell_class then
				print_head(c.shell_class)
			else
				dump(c)
			end
		end
		
		print_head('extern "C" LQT_EXPORT int luaopen_'..n..' (lua_State *);')
		print_head('\n\n#endif // LQT_HEAD_'..n)
		fhead:close()
	end
	return classes
end

function sort_by_index(c)
	local res = {}
	local vidx = 0
	for name, list in pairs(c.virtuals) do
		for _,virt in ipairs(list) do
			virt.virtual_index = vidx
			res[#res + 1] = virt
		end
		vidx = vidx + 1
	end
	table.sort(res, function(v1, v2) return v1.virtual_index < v2.virtual_index end)
	c.nvirtuals = vidx
	return res
end
