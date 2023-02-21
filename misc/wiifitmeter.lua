wfmtrlnk_protocol = Proto("fitmeterlink", "Wii Fit Meter Link Layer protocol")
wfmtrapp_protocol = Proto("FitMeterApplication", "Wii Fit Meter Application Layer Protocol")
-- CRC-8
crc_table = {}
di = 0x07

-- INIT CRC TABLE
-- Indices in Lua are stupid
for i = 1, 256, 1
do
	crc = i - 1
	for j = 1, 8, 1
	do
		--str = str.."(&: "..bit.band(crc, 0x80)..")"
		if bit.band(crc, 0x80) ~= 0 then
			xorval = di
		else
			xorval = 0
		end
		crc = bit.bxor(bit.lshift(crc, 1), xorval)
	end
	crc_table[i] = bit.band(crc, 0xFF)
end

function crc8(prev_crc, value)
	return bit.band(crc_table[bit.bxor(prev_crc, value) + 1], 0xFF)
end

function crc8_list(list)
	crc = 0
	for i = 0, list:len() - 1, 1
	do
		crc = crc8(crc, list:get_index(i))
	end
	return crc
end

function decrypt_data(list)
	size = list:len()
	i = 0
	for i = 0, size - 1, 1
	do
		list:set_index(i, bit.bxor(list:get_index(i), 0x55))
	end
	return list
	--[[while size >= 8 do
		for j = 8, 16, 1 
		do
			list:set_index(i + j, bit.bxor(list:get_index(i + j), 0x55))
		end
		i = i + 8
		size = size - 8
	end
	while size > 0 do
		list:set_index(i, bit.bxor(list:get_index(i), 0x55))
		i = i + 1
		size = size - 1
	end
	return list]]
end

-------------------------------------------------------------------------------
-- Fit Meter Link Protocol                                                   --
-------------------------------------------------------------------------------

--HEADERS
header_magic = ProtoField.uint8("fitmeterlink.magic", "Magic Number", base.HEX)
header_connection = ProtoField.uint8("fitmeterlink.connection", "Connection")
header_connection_setup = ProtoField.bool("fitmeterlink.connection_setup", "Connection Setup", 8, nil, 0x80)
header_large_packet = ProtoField.bool("fitmeterlink.large_packet", "Large packet", 8, nil, 0x40)
header_connection_setup_id = ProtoField.uint8("fitmeterlink.connection_setup.id", "Connection Setup ID")
header_data_length = ProtoField.uint8("fitmeterlink.length", "Data Length", base.DEC, nil, 0x3F)
header_data_length_large = ProtoField.uint8("fitmeterlink.length", "Data Length", base.DEC)
header_checksum = ProtoField.uint8("fitmeterlink.crc", "CRC-8")
header_data = ProtoField.bytes("fitmeterlink.data", "Data")
header_decrypted_data = ProtoField.bytes("fitmeterlink.decrypted_data", "Decrypted data")
	
wfmtrlnk_protocol.fields = {
	header_magic,
	header_connection,
	header_connection_setup,
	header_connection_setup_id,
	header_large_packet,
	header_data_length,
	header_data_length_large,
	header_checksum,
	header_data,
	header_decrypted_data
}

function wfmtrlnk_protocol.dissector(buffer, pinfo, tree)
	length = buffer:len()
	if length == 0 then return end
	
	pinfo.cols.protocol = wfmtrlnk_protocol.name
	local subtree = tree:add(wfmtrlnk_protocol, buffer(), "Wii Fit Meter Protocol Header")
	
	subtree:add_le(header_magic, buffer(0,1))
	subtree:add_le(header_connection, buffer(1,1))
	pinfo.cols.src = buffer(1,1):le_uint()
	pinfo.cols.dst = buffer(1,1):le_uint()
	
	connection_setup_flag = bit.band(buffer(2, 1):le_uint(), 0x80) == 0x80
	subtree:add_le(header_connection_setup, buffer(2,1))
	
	local data = nil
	-- If the length (3th byte), has the flag 0x40 set, the actual length is contained in the next byte
	subtree:add_le(header_large_packet, buffer(2,1))
	if bit.band(buffer(2, 1):le_uint(), 0x40) == 0x40 then
		data_length = buffer(3,1):le_uint()
		data = buffer:range(4, data_length)
		subtree:add_le(header_data_length_large, buffer(3,1), data_length)
		subtree:add_le(header_data, data, "Wii Fit Meter Data")
		if not connection_setup_flag then
			subtree:add_le(header_decrypted_data, buffer(4, data_length), decrypt_data(buffer(4, data_length):bytes()):raw())
		end
	else
		-- 0x3F = 0b00111111
		data_length = bit.band(buffer(2,1):le_uint(), 0x3F)
		data = buffer:range(3, data_length)
		--report_failure(length)
		subtree:add_le(header_data_length, buffer(2, 1), data_length)
		subtree:add_le(header_data, buffer(3, data_length))
		if not connection_setup_flag then
			subtree:add_le(header_decrypted_data, buffer(3, data_length), decrypt_data(buffer(3, data_length):bytes()):raw())
		end
	end
	
	-- Handle connection setup flag
	if connection_setup_flag then
		connection_command = data(0,1):le_uint()
		if connection_command == 0x01 then
			pinfo.cols.info = "Connection setup (proposed connectionId: "..data(3,1):le_uint()..")"
			subtree:add_le(header_connection_setup_id, data(3,1))
		elseif connection_command == 0x02 then
			pinfo.cols.info = "Connection setup ack"
		elseif connection_command == 0x0f then
			pinfo.cols.info = "Connection close"
		end
	else
		pinfo.cols.info = "Application Data"
	end
	
	subtree:add_le(header_checksum, buffer(length - 1, 1)):append_text(" (Expected: "..crc8_list(buffer(0, length - 1):bytes())..")")
	--crc8_list(buffer(0, length - 1):bytes())
	if data ~= nil and not connection_setup_flag then
		local appTvb = data:tvb("Fit Meter APP TVB")
		local fitmtrapp_dis = Dissector.get("data")
		fitmtrapp_dis:call(appTvb, pinfo, tree)
	end
end

function is_wfmtrlnk_protocol(buffer, pinfo, tree)
	return buffer(0,1):le_uint() == 0x53
	-- TODO: check if packet length matches protocol fields and if checksum matches
end

--wfmtrlnk_protocol.register_heuristic(wfmtrlnk_protocol, is_wfmtrlnk_protocol)

-------------------------------------------------------------------------------
-- Fit Meter Application Protocol                                            --
-------------------------------------------------------------------------------

function wfmtrapp_protocol.dissector(buffer, pinfo, tree)
	local foo = "bar"
end

local my_info = {
	version = "0.1.0",
	author = "Henk Kalkwater",
	repository = "https://github.com/henkkalkwater/fitmetersync"
}
set_plugin_info(my_info)
