#include "Prereq.h"
#include "ResourceCommandBuffer.h"


namespace Voidstar 
{
	
	void ResourceCommandBuffer::WriteByte(uint8_t command)
	{
		commands.push_back(command);
	}
	void WriteBytes(std::vector<uint8_t>& buf, const uint8_t* data,  const size_t len) 
	{
		buf.insert(buf.end(), data, data + len);
	}
	void ResourceCommandBuffer::Write(std::string_view str)
	{
		auto len = static_cast<uint32_t>(str.size());

		auto p = reinterpret_cast<const uint8_t*>(&len);
		
		commands.insert(commands.end(), p, p + sizeof(uint32_t));

		WriteBytes(commands, reinterpret_cast<const uint8_t*>(str.data()), len);
	}

	uint8_t ResourceCommandBuffer::ReadByte()
	{
		return commands[current++];
	}

	std::string_view ResourceCommandBuffer::ReadString()
	{
		uint32_t len;
		std::memcpy(&len, commands.data() + current, sizeof(uint32_t));
		current += sizeof(uint32_t);

		auto data = reinterpret_cast<const char*>(commands.data() + current);
		return { data, len };
	}

}