#include"Prereq.h"
#include "ShaderCompiler.h"
#include "Rendering/ShaderType.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Device.h"
#include  "Log.h"

#include <fstream>
#include <string_view>

#include "spirv_cross.hpp"
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

	static std::unordered_map<ShaderType, const char*> ShaderFolders =
	{
		{ShaderType::VERTEX,"Vertex"},
		{ShaderType::FRAGMENT,"Fragment"},
		{ShaderType::TESS_CONTROL,"Tesselation"},
		{ShaderType::TESS_EVALUATION,"Tesselation"},
		{ShaderType::COMPUTE,"Compute"}
	};
	static std::unordered_map<std::string_view, ShaderType> ShaderExtensionsToType =
	{
		{".vert",ShaderType::VERTEX},
		{".frag",ShaderType::FRAGMENT},
		{".tesc",ShaderType::TESS_CONTROL},
		{".tese",ShaderType::TESS_EVALUATION},
		{".comp",ShaderType::COMPUTE}
	};
	static std::unordered_map<ShaderType, const char*> ShaderBinaryExtensions =
	{
		{ShaderType::VERTEX,".spvV"},
		{ShaderType::FRAGMENT,".spvF"},
		{ShaderType::TESS_CONTROL,".spvC"},
		{ShaderType::TESS_EVALUATION,".spvE"},
		{ShaderType::COMPUTE,".spvCmp"}
	};

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
	void ShaderCompiler::Init()
	{
		InitFilePath();
	}



	std::string GetFileNameAndExtension(const std::string& filepath)
	{
		size_t extensionIndex = filepath.find_last_of('.');
		return filepath.substr(0, extensionIndex);
	}

	// returns exe invocations and binary path
	std::string CreateCommand(std::string& path, std::string& shaderOutput)
	{
		std::string command = SPIRV_COMPILER_PATH + " -V " + path + " -o " + shaderOutput;
		return { command };
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



	BindingDesc CreateBindingDesc(uint32_t set,
		uint32_t binding,
		ResourceType kind,
		ShaderType stage,
		uint32_t count, // how many descriptors are bound to the set, bounding [4]
		uint32_t stride = 0,
		uint32_t elemSize = 0,
		uint32_t format = 0)
	{
		BindingDesc d{};
		d.set = set;
		d.binding = binding;
		d.kind = kind;
		d.count = count;
		d.stride = stride;
		d.elemSize = elemSize;
		d.format = format;
		d.stage = stage;
		return d;
	}


	
	inline uint32_t array_size(const spirv_cross::SPIRType& t) {
		if (t.array.empty()) return 1;
		// SPIRV-Cross uses 0 as runtime-sized array
		return t.array.back() == 0 ? 0u : uint32_t(t.array.back());
	}

	std::pair<StageMeta,std::vector<uint32_t>> Reflect(std::string& path)
	{
		// reflection time, we need to figure out what resources shader needs to work
		auto spirv = LoadSpv(path.c_str());

		if (spirv.empty()) {
			Log::GetLog()->error("Empty SPIR-V: {}", path);
			return{};
		}
		if (spirv[0] != 0x07230203u) {
			Log::GetLog()->error("Not a SPIR-V binary (magic=0x{:08X}) at {}", spirv[0], path);
			return{};
		}
		spirv_cross::Compiler comp(spirv.data(), spirv.size());


		auto shaderType = GetShaderType(comp.get_execution_model());

		// we can encode everything the shader uses amd then later create it when it is needed

		auto res = comp.get_shader_resources();

		// UB, Textures, SB,

		StageMeta meta;
		// --- Uniform buffers ---
		for (auto& ub : res.uniform_buffers) {
			uint32_t set = comp.get_decoration(ub.id, spv::DecorationDescriptorSet);
			uint32_t binding = comp.get_decoration(ub.id, spv::DecorationBinding);
			const auto& t = comp.get_type(ub.base_type_id);
			uint32_t count = array_size(comp.get_type(ub.type_id)); // descriptor array on the variable
			// total struct size (std140/std430 layout is reflected as declared)
			uint32_t blockSize = uint32_t(comp.get_declared_struct_size(t));
			meta.bindings.push_back(CreateBindingDesc(set, binding, ResourceType::UniformBuffer,
				meta.stage, count, /*stride*/0, /*elemSize*/blockSize));
		}

		// --- Storage buffers (SSBO) ---
		for (auto& sb : res.storage_buffers) {
			uint32_t set = comp.get_decoration(sb.id, spv::DecorationDescriptorSet);
			uint32_t binding = comp.get_decoration(sb.id, spv::DecorationBinding);
			const auto& st = comp.get_type(sb.base_type_id);     // struct type
			uint32_t count = array_size(comp.get_type(sb.type_id)); // descriptor array size

			// Try to detect a trailing runtime array member and fetch its stride/elemSize.
			uint32_t stride = 0, elemSize = 0;
			if (!st.member_types.empty()) {
				uint32_t last = uint32_t(st.member_types.size() - 1);
				const auto& mt = comp.get_type(st.member_types[last]);
				if (!mt.array.empty() && mt.array.back() == 0) {
					// runtime array: array stride & element size
					stride = comp.type_struct_member_array_stride(st, last);
					// element size = declared size of one element type
					elemSize = uint32_t(comp.get_declared_struct_size(mt)); // for scalars/vectors this is fine too
				}
			}
			meta.bindings.push_back(CreateBindingDesc(set, binding, ResourceType::StorageBuffer,
				meta.stage, count, stride, elemSize));
		}

		// --- Sampled images (combined image-samplers) ---
		for (auto& si : res.sampled_images) {
			uint32_t set = comp.get_decoration(si.id, spv::DecorationDescriptorSet);
			uint32_t binding = comp.get_decoration(si.id, spv::DecorationBinding);
			uint32_t count = array_size(comp.get_type(si.type_id));

			// Optional: capture image format if specified (for storage images this matters more)
			const auto& ty = comp.get_type(si.type_id);
			uint32_t fmt = 0; // spv::ImageFormat (enum) if needed: ty.image.format

			meta.bindings.push_back(CreateBindingDesc(set, binding, ResourceType::SampledImage,
				meta.stage, count, 0, 0, fmt));
		}

		// --- Separate images (no sampler baked in) ---
		for (auto& img : res.separate_images) {
			uint32_t set = comp.get_decoration(img.id, spv::DecorationDescriptorSet);
			uint32_t binding = comp.get_decoration(img.id, spv::DecorationBinding);
			uint32_t count = array_size(comp.get_type(img.type_id));
			meta.bindings.push_back(CreateBindingDesc(set, binding, ResourceType::SampledImage,
				meta.stage, count));
		}

		// --- Separate samplers ---
		for (auto& smp : res.separate_samplers) {
			uint32_t set = comp.get_decoration(smp.id, spv::DecorationDescriptorSet);
			uint32_t binding = comp.get_decoration(smp.id, spv::DecorationBinding);
			uint32_t count = array_size(comp.get_type(smp.type_id));
			meta.bindings.push_back(CreateBindingDesc(set, binding, ResourceType::Sampler,
				meta.stage, count));
		}

		// --- Storage images (read/write) ---
		for (auto& si : res.storage_images) {
			uint32_t set = comp.get_decoration(si.id, spv::DecorationDescriptorSet);
			uint32_t binding = comp.get_decoration(si.id, spv::DecorationBinding);
			uint32_t count = array_size(comp.get_type(si.type_id));
			const auto& ty = comp.get_type(si.type_id);
			uint32_t fmt = uint32_t(ty.image.format); // spv::ImageFormat enum
			meta.bindings.push_back(CreateBindingDesc(set, binding, ResourceType::StorageImage,
				meta.stage, count, 0, 0, fmt));
		}

		// --- Push constants ---
		if (res.push_constant_buffers.size() == 1) {
			auto& pcb = res.push_constant_buffers[0];
			const auto& t = comp.get_type(pcb.base_type_id);
			uint32_t size = uint32_t(comp.get_declared_struct_size(t));
			// (SPIR-V push block has a single range; if you use multiple, split as you wish)
			meta.pushConsts.push_back(PushConstRange{ 0u, size, meta.stage });
		}

		// --- Compute local size (if compute) ---
		if (meta.stage == ShaderType::COMPUTE) {
			auto wg = comp.get_entry_points_and_stages()[0]; // assume one entry
			// Prefer specialization constants if present; fallback to execution mode
			if (comp.get_execution_mode_argument(spv::ExecutionModeLocalSizeId, 0) != 0) {
				meta.localSize[0] = comp.get_execution_mode_argument(spv::ExecutionModeLocalSizeId, 0);
				meta.localSize[1] = comp.get_execution_mode_argument(spv::ExecutionModeLocalSizeId, 1);
				meta.localSize[2] = comp.get_execution_mode_argument(spv::ExecutionModeLocalSizeId, 2);
			}
			else {
				meta.localSize[0] = comp.get_execution_mode_argument(spv::ExecutionModeLocalSize, 0);
				meta.localSize[1] = comp.get_execution_mode_argument(spv::ExecutionModeLocalSize, 1);
				meta.localSize[2] = comp.get_execution_mode_argument(spv::ExecutionModeLocalSize, 2);
			}
		}

		return {meta,spirv};

		
	}
	void ShaderCompiler::Link(ProgramHandle handle, uint8_t shaderAmount)
	{
		//std::vector<StageMeta> metas;
		ProgramMeta meta;
		for (int i = 0; i < shaderAmount; i++)
		{
			auto sMeta = m_StageMetas.top();
			meta.stages.push_back(sMeta);
			m_StageMetas.pop();
		}

		auto id = handle;
		m_Programs[id] = meta;
	}


	std::vector<char> ReadFile(std::string_view filename)
	{

		std::ifstream file(filename.data(), std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			Log::GetLog()->error("Failed to load {0}", filename);
		}

		size_t filesize{ static_cast<size_t>(file.tellg()) };

		std::vector<char> buffer(filesize);
		file.seekg(0);
		file.read(buffer.data(), filesize);

		file.close();
		return buffer;
	}

	static vk::ShaderModule CreateModule(std::vector<uint32_t> spirvCode, vk::Device device)
	{
		vk::ShaderModuleCreateInfo moduleInfo = {};
		moduleInfo.flags = vk::ShaderModuleCreateFlags();
		auto codeSize = spirvCode.size() * sizeof(uint32_t);
		assert((codeSize% 4) == 0);
		moduleInfo.codeSize = codeSize;
		moduleInfo.pCode = spirvCode.data();
		return device.createShaderModule(moduleInfo);	
	}



	void ShaderCompiler::Compile(std::filesystem::path path)
	{

		auto ext = path.extension().string();
		if (ShaderExtensionsToType.find(ext.c_str()) == ShaderExtensionsToType.end())
		{
			Log::GetLog()->error("unknown shader extension {0}", ext);
			return;
		}
		auto type = ShaderExtensionsToType.at(ext.c_str());

		auto folder = ShaderFolders[type];
		auto shaderName = path.filename().string();
		auto shaderPath = BASE_SHADER_PATH + folder + "/" + shaderName.data();
		auto isExist = std::filesystem::exists(shaderPath);
		if (!isExist)
		{
			Log::GetLog()->error("SHADER COMPILATOIN: Path {0} is not found ", shaderPath);
			return;
		}
		auto shaderOutput = BASE_SPIRV_OUTPUT + ShaderFolders.at(type) + shaderName  + ShaderBinaryExtensions[type];
		auto command= CreateCommand(shaderPath ,shaderOutput);
		int result = std::system(command.c_str());
		if (result != 0)
		{
			Log::GetLog()->error("shader {0} is not compiled! ", shaderName.data());
			return;
		}

	
#if 1
		auto [stageMeta,sprivCode]= Reflect(shaderOutput);
		
		try {
			stageMeta.module = CreateModule(sprivCode,RenderContext::GetDevice()->GetDevice());
		}
		catch (vk::SystemError err) {
			Log::GetLog()->error("Failed to create shader module for {0}", path.string());
		}
		
		m_StageMetas.push(stageMeta);
#endif
		

	}

}