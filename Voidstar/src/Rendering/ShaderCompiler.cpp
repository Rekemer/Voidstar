#include"Prereq.h"
#include "ShaderCompiler.h"
#include "Rendering/ShaderType.h"
#include "spirv_cross.hpp"
#include <filesystem>
#include <fstream>
#include  "Log.h"
namespace Voidstar
{


	inline std::string BASE_SHADER_PATH = "../Shaders/";
	inline std::string BASE_RES_PATH = "res";
	inline std::string BASE_VIRT_PATH = "E:/dev/Voidstar/mipMaps_virtualTex4.tiff/";
	const std::string SPIRV_COMPILER_PATH = std::string(std::string(std::getenv("VULKAN_SDK")) + std::string("/Bin/glslangvalidator.exe"));
	
	inline std::string BASE_SPIRV_OUTPUT = BASE_SHADER_PATH + "Binary/";


	ShaderType GetShaderType(spv::ExecutionModel m) {
		switch (m) {
		case spv::ExecutionModelVertex:                 return ShaderType::VERTEX;
		case spv::ExecutionModelFragment:               return ShaderType::FRAGMENT;
		case spv::ExecutionModelGLCompute:              return ShaderType::COMPUTE;
			//case spv::ExecutionModelTessellationControl:    return ShaderType::TESS_CONTROL;
			//case spv::ExecutionModelTessellationEvaluation: return ShaderType::TESS_EVALUATION;
		default: throw new std::exception("Shader format {} is not supported in reflection", m);
		}
	}

	static std::unordered_map<ShaderType, const char*> PipelineShaderFolders =
	{
		{ShaderType::VERTEX,"Vertex"},
		{ShaderType::FRAGMENT,"Fragment"},
		{ShaderType::TESS_CONTROL,"Tesselation"},
		{ShaderType::TESS_EVALUATION,"Tesselation"},
		{ShaderType::COMPUTE,"Compute"}
	};
	static std::unordered_map<ShaderType, const char*> PipelineShaderExtensions =
	{
		{ShaderType::VERTEX,".vert"},
		{ShaderType::FRAGMENT,".frag"},
		{ShaderType::TESS_CONTROL,".tesc"},
		{ShaderType::TESS_EVALUATION,".tese"},
		{ShaderType::COMPUTE,".comp"}
	};
	static std::unordered_map<ShaderType, const char*> PipelineShaderBinaryExtensions =
	{
		{ShaderType::VERTEX,".spvV"},
		{ShaderType::FRAGMENT,".spvF"},
		{ShaderType::TESS_CONTROL,".spvC"},
		{ShaderType::TESS_EVALUATION,".spvE"},
		{ShaderType::COMPUTE,".spvCmp"}
	};

	void ShaderCompiler::Init()
	{
		InitFilePath();
	}

	std::string InitFilePath()
	{
		std::string baseShaderPath = "";

		// Check if running within Visual Studio
		const char* visualStudioEnvVar = std::getenv("VSLANG");
		if (visualStudioEnvVar != nullptr)
		{
			// Set the base shader path relative to the project directory
			BASE_SHADER_PATH = "../Shaders/";
			BASE_RES_PATH = "../res/";

		}
		else
		{
			// Set the base shader path relative to the executable directory
			std::filesystem::path executablePath = std::filesystem::current_path();
			BASE_SHADER_PATH = executablePath.parent_path().string() + "../../../Shaders/";
			BASE_RES_PATH = executablePath.parent_path().string() + "../../../res/";
			BASE_SPIRV_OUTPUT = BASE_SHADER_PATH + "Binary/";
			BASE_VIRT_PATH = executablePath.parent_path().string() + "../../../../mipMaps_virtualTex4.tiff/";
		}

		return baseShaderPath;
	}


	std::string GetFileNameWithoutExtension(const std::string& filepath)
	{
		size_t extensionIndex = filepath.find_last_of('.');
		return filepath.substr(0, extensionIndex);
	}

	// returns exe invocations and binary path
	std::pair<std::string, std::string> CreateCommand(std::string_view shader, const char* binaryExtension, std::string& shaderPath)
	{
		auto name = GetFileNameWithoutExtension(shader.data());
		std::string shaderOutput = BASE_SPIRV_OUTPUT + name.c_str() + binaryExtension;
		std::string command = SPIRV_COMPILER_PATH + " -V " + shaderPath + " -o " + shaderOutput;
		return { command, shaderOutput };
	}

	std::vector<uint32_t> LoadSpv(const char* path) {
		std::ifstream f(path, std::ios::binary);
		if (!f) throw std::runtime_error("can't open spv");
		f.seekg(0, std::ios::end);
		size_t bytes = size_t(f.tellg());
		if (bytes % 4 != 0) throw std::runtime_error("spv size not multiple of 4");
		f.seekg(0, std::ios::beg);

		std::vector<uint32_t> words(bytes / 4);
		f.read(reinterpret_cast<char*>(words.data()), bytes);
		return words;
	}

	void ShaderCompiler::Compile(std::string_view binaryShaderName, ShaderType type)
	{
		auto folder = PipelineShaderFolders[type];
		auto shaderName = GetFileNameWithoutExtension(binaryShaderName.data());
		shaderName += PipelineShaderExtensions[type];
		auto path = BASE_SHADER_PATH + folder + "/" + shaderName.data();
		auto isExist = std::filesystem::exists(path);
		if (!isExist)
		{
			Log::GetLog()->error("SHADER COMPILATOIN: Path {0} is not found ", path);
			return;
		}
		auto binaryExtension = PipelineShaderBinaryExtensions[type];
		auto [command, output] = CreateCommand(shaderName, binaryExtension, path);
		int result = std::system(command.c_str());
		if (result != 0)
		{
			Log::GetLog()->error("shader {0} is not compiled! ", shaderName.data());
			return;
		}
		// reflection time, we need to figure out what resources shader needs to work
		auto spirv = LoadSpv(output.c_str());

		if (spirv.empty()) {
			Log::GetLog()->error("Empty SPIR-V: {}", output);
			return;
		}
		if (spirv[0] != 0x07230203u) {
			Log::GetLog()->error("Not a SPIR-V binary (magic=0x{:08X}) at {}", spirv[0], output);
			return;
		}

		spirv_cross::Compiler comp(spirv.data(), spirv.size());


		auto shaderType = GetShaderType(comp.get_execution_model());

		// we can encode everything the shader uses amd then later create it when it is needed

		auto res = comp.get_shader_resources();

		// UB, Textures, SB,


		for (auto& buf : res.storage_buffers) {
			uint32_t set = comp.get_decoration(buf.id, spv::DecorationDescriptorSet);
			uint32_t binding = comp.get_decoration(buf.id, spv::DecorationBinding);
			size_t   blockSize = comp.get_declared_struct_size(comp.get_type(buf.base_type_id));
			const auto& block = comp.get_type(buf.base_type_id);      // struct StorageBuffer
			uint32_t arr_index = 0;                                   // tiles

			// Stride of tiles[] (bytes between elements)
			uint32_t stride = comp.type_struct_member_array_stride(block, arr_index);

			std::cout << "SSBO: name=\"" << buf.name
				<< "\" set=" << set << " binding=" << binding
				<< " blockSize=" << blockSize << " bytes\n";

			// Inspect members of the SSBO struct
			const auto& st = comp.get_type(buf.base_type_id);
			for (uint32_t m = 0; m < st.member_types.size(); ++m) {
				auto memberTypeId = st.member_types[m];
				auto memberName = comp.get_member_name(buf.base_type_id, m);
				size_t offset = comp.type_struct_member_offset(st, m);
				size_t size = comp.get_declared_struct_member_size(st, m);

				std::cout << "  member[" << m << "] name=\"" << memberName
					<< "\" offset=" << offset << " size=" << size << "\n";
			}
		}



		for (auto& ub : res.uniform_buffers) {
			uint32_t set = comp.get_decoration(ub.id, spv::DecorationDescriptorSet);
			uint32_t binding = comp.get_decoration(ub.id, spv::DecorationBinding);
			std::cout << "UBO: name=\"" << ub.name << "\" set=" << set << " binding=" << binding << "\n";
		}

	}

}