local ConfigService = require("api/ConfigService")

local Service = ConfigService:new({
	-- delete = false,          -- Disable deletion of UCI sections
	-- create = false,          -- Disable creation of UCI sections
	-- general_section = "main",-- General UCI section name
	-- anonymous = true,        -- Create UCI anonymous sections
	-- increment_name = true,   -- Create UCI sections with numeric incremental names
})

local ConfigSection = Service:section(
	"tuya", -- UCI config name
	"tuya"  -- UCI section type
)
ConfigSection:make_primary()
ConfigSection.default_options.id.maxlength = 8 -- Default id option can also have validations
	function ConfigSection.default_options.id:validate(value)
		return (value == "config")
	end
-- ConfigSection.order_by = "option" -- Order UCI config by provided option
-- ConfigSection.sort_response_by = "option" -- Order API response by provided option

function ConfigSection:create_defaults(sid)
	-- Default values to be added with every creation
	return {
		productid = " ",
		deviceid = " ",
		devicesecret = " ",
		enabled = "0"
	}
end

local opt_productid = ConfigSection:option("productid")
	-- opt_productid.cfg_require = true -- Option is required
	opt_productid.maxlength = 100
	opt_productid.minlength = 5

local opt_deviceid = ConfigSection:option("deviceid")
	-- opt_deviceid.cfg_require = true -- Option is required
	opt_deviceid.maxlength = 100
	opt_deviceid.minlength = 5

local opt_devicesecret = ConfigSection:option("devicesecret")
	-- opt_devicesecret.cfg_require = true -- Option is required
	opt_devicesecret.maxlength = 100
	opt_devicesecret.minlength = 5

local opt_enabled = ConfigSection:option("enabled")
	function opt_enabled:validate(value)
		-- Here you can write a custom validation for this field
		return self.dt:is_bool(value)
	end

-- Uploads, Actions, GET_TYPE are also possible and are the same as in function_example.lua file.

-----------------------------------------------HOOKS-----------------------------------------------

-- function Service:GET_init_hook()
-- end

-- function Service:GET_section_init_hook()
-- end

-- function Service:GET_validate_section_hook()
-- end

-- function Service:GET_after_data_hook(data)
-- end


-- function Service:POST_init_hook()
-- end

-- function Service:POST_validate_hook()
-- end

-- function Service:POST_before_commit_hook()
-- end

-- function Service:POST_after_commit_hook()
-- end

-- function Service:POST_section_init_hook()
-- end

-- function Service:POST_validate_section_hook()
-- end

-- function Service:POST_after_data_hook()
-- end


-- function Service:PUT_init_hook()
-- end

-- function Service:PUT_validate_hook()
-- end

-- function Service:PUT_before_commit_hook()
-- end

-- function Service:PUT_after_commit_hook()
-- end

-- function Service:PUT_section_init_hook()
-- end

-- function Service:PUT_validate_section_hook()
-- end

-- function Service:PUT_after_validate_section_hook()
-- end

-- function Service:PUT_after_data_hook()
-- end


-- function Service:DELETE_init_hook()
-- end

-- function Service:DELETE_validate_hook()
-- end

-- function Service:DELETE_before_commit_hook()
-- end

-- function Service:DELETE_after_commit_hook()
-- end

-- function Service:DELETE_section_init_hook()
-- end

-- function Service:DELETE_before_section_delete_hook()
-- end

-- function Service:DELETE_after_data_hook(data)
-- end

return Service
