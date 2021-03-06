wfmtrnet_protocol = Proto("FitMtrNet", "Wii Fit Meter Network Layer protocol") 
wfmtrtrn_protocol = Proto("FitMtrTrn", "Wii Fit Meter Transport Layer Protocol")
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
		--str = str.."(&: "..bit32.band(crc, 0x80)..")"
		if bit32.band(crc, 0x80) ~= 0 then
			xorval = di
		else
			xorval = 0
		end
		crc = bit32.bxor(bit32.lshift(crc, 1), xorval)
	end
	crc_table[i] = bit32.band(crc, 0xFF)
end

function crc8(prev_crc, value)
	return bit32.band(crc_table[bit32.bxor(prev_crc, value) + 1], 0xFF)
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
		list:set_index(i, bit32.bxor(list:get_index(i), 0x55))
	end
	return list
	--[[while size >= 8 do
		for j = 8, 16, 1 
		do
			list:set_index(i + j, bit32.bxor(list:get_index(i + j), 0x55))
		end
		i = i + 8
		size = size - 8
	end
	while size > 0 do
		list:set_index(i, bit32.bxor(list:get_index(i), 0x55))
		i = i + 1
		size = size - 1
	end
	return list]]
end

--message("test")
--HEADERS
header_magic = ProtoField.uint8("fitmtrnet.magic", "Magic Number", base.HEX)
header_connection = ProtoField.int8("fitmtrnet.connection", "Connection")
header_connection_setup = ProtoField.bool("fitmtrnet.connection_setup", "Connection Setup")
header_data_length = ProtoField.int8("fitmtrnet.length", "Data Length")
header_checksum = ProtoField.uint8("fitmtrnet.crc", "CRC-8")
header_data = ProtoField.bytes("fitmtrnet.data", "Data")
header_decrypted_data = ProtoField.bytes("fitmtrnet.decrypted_data", "Decrypted data")
	
wfmtrnet_protocol.fields = {header_magic, header_connection, header_connection_setup, header_data_length, header_checksum, header_data, header_decrypted_data}

function wfmtrnet_protocol.dissector(buffer, pinfo, tree)
	length = buffer:len()
	if length == 0 then return end
	
	pinfo.cols.protocol = wfmtrnet_protocol.name
	local subtree = tree:add(wfmtrnet_protocol, buffer(), "Wii Fit Meter Protocol Header")
	
	subtree:add_le(header_magic, buffer(0,1))
	subtree:add_le(header_connection, buffer(1,1))
	
	connection_setup_flag = bit32.band(buffer(2, 1):le_uint(), 0x80) == 0x80
	subtree:add_le(header_connection_setup, buffer(2,1), connection_setup_flag)
	if connection_setup_flag then
		pinfo.cols.info = "Connection Setup"
	end
	
	-- If the length (3th byte), has the flag 0x40 set, the actual length is contained in the next byte
	if bit32.band(buffer(2, 1):le_uint(), 0x40) == 0x40 then
		data_length = buffer(3,1):le_uint()
		subtree:add_le(header_data_length, data_length)
		subtree:add_le(header_data, buffer(4, buffer(3, 1):le_uint()), "Wii Fit Meter Data")
		if not connection_setup_flag then
			subtree:add_le(header_decrypted_data, buffer(4, data_length), decrypt_data(buffer(4, data_length)))
		end
	else
		-- 0x3F = 0b00111111
		data_length = bit32.band(buffer(2,1):le_uint(), 0x3F)
		--report_failure(length)
		subtree:add_le(header_data_length, buffer(2, 1), data_length)
		subtree:add_le(header_data, buffer(3, data_length))
		if not connection_setup_flag then
			subtree:add_le(header_decrypted_data, buffer(3, data_length), decrypt_data(buffer(3, data_length):bytes()):raw())
		end
	end
	subtree:add_le(header_checksum, buffer(length - 1, 1)):append_text(" (Expected: "..crc8_list(buffer(0, length - 1):bytes())..")")
	--crc8_list(buffer(0, length - 1):bytes())
end

function is_wfmtrnet_protocol(buffer, pinfo, tree)
	return buffer(0,1):le_uint() == 0x53
end

--wfmtrnet_protocol.register_heuristic(wfmtrnet_protocol, is_wfmtrnet_protocol)

local my_info = {
	version = "0.1.0",
	author = "Henk Kalkwater",
	repository = "https://github.com/henkkalkwater/fitmetersync"
}
set_plugin_info(my_info)
