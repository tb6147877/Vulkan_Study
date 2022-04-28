#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS//使用弧度作为参数，避免混淆
#define GLM_FORCE_DEPTH_ZERO_TO_ONE//vulkan里深度值是0-1
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <array>

#include <optional>
#include <set>
#include <algorithm>
#include <fstream>


//在debug模式开启ValidationLayers，在发布模式关闭
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif


struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;//Queue family要支持图形
	std::optional<uint32_t> presentFamily;//Queue family要支持窗口显示

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

//用来描述swapchain
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;//surface的基础功能（swap chain中的图片数量和大小）
	std::vector<VkSurfaceFormatKHR> formats;//surface中图片的格式和颜色空间
	std::vector<VkPresentModeKHR> presentModes;//可用的表现模式
};


struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	//实现hash map所需的
	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;//在binding数组里的索引
		bindingDescription.stride = sizeof(Vertex);//从一个entry到另一个entry的byte数量，也就是步长
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;//逐顶点还是逐instance

		return bindingDescription;
	}


	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

		//attribute0 顶点坐标
		attributeDescriptions[0].binding = 0;//所属的binding
		attributeDescriptions[0].location = 0;//vertex shader里面写的location
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;//这个attribute的格式
		attributeDescriptions[0].offset = offsetof(Vertex, pos);//这个attribute的offset

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}
};

//vulkan也有数据对齐的要求，使用C++11的特性alignas(16)，或者使用glm的宏GLM_FORCE_DEFAULT_ALIGNED_GENTYPES能解决
struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

//基本流程：
//1.依托device的内存创建一个image object的对象
//2.从一个图片文件往这个image objec填充像素
//3.创建一个image sampler
//4.增加一个combined image sampler descriptor来从texture中采样颜色

/*
image各种layout的特性
VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: Optimal for presentation
VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: Optimal as attachment for writing colors from the fragment shader
VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: Optimal as source in a transfer operation, like vkCmdCopyImageToBuffer
VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: Optimal as destination in a transfer operation, like vkCmdCopyBufferToImage
VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: Optimal for sampling from a shader
*/


//todo:自己创建独立的transfer queue
//todo:layout的意思没搞懂
//todo：流水线的barrier没搞懂
class LoadModel {
public:
	void run();

private:
	const std::string MODEL_PATH = "D:/Project/Vulkan_Study/Vulkan_Study/Models/viking_room.obj";
	const std::string TEXTURE_PATH = "D:/Project/Vulkan_Study/Vulkan_Study/Textures/viking_room.png";

	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;
	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };//需要启用的validation layer的列表
	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };//需要声明的extension的列表

	

	GLFWwindow* window;
	VkInstance instance;//应用程序和Vulkan库之间的连接
	VkDebugUtilsMessengerEXT debugMessenger;//专门处理用于vulkan debug的回调
	VkSurfaceKHR surface;//Vulkan窗口接口

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;//物理显卡访问，instance被删掉时这个device也会被删掉
	VkDevice device;//逻辑接口，一个应用程序里可以有多个


	VkQueue graphicsQueue;//图形queue，跟随device自动创建
	VkQueue presentQueue;//显示queue

	VkSwapchainKHR swapChain;//swap chain对象
	std::vector<VkImage> swapChainImages;//swap chain中的图片
	VkFormat swapChainImageFormat;//swap chain中的图片格式
	VkExtent2D swapChainExtent;//swap chain中图片的分辨率
	std::vector<VkImageView> swapChainImageViews;//VkImageView是VkImage在渲染流水线中的代理

	VkDescriptorSetLayout descriptorSetLayout;//指定pipeline要访问的资源类型
	VkPipelineLayout pipelineLayout;//功能类似于uniform变量，渲染时往shader传参
	VkRenderPass renderPass;//一个render pass
	VkPipeline graphicsPipeline;//一个完整的渲染流水线

	std::vector<VkFramebuffer> swapChainFramebuffers;//我们要为swap chain中每张图都创建一个FBO

	VkCommandPool commandPool;//用于管理cmdbuf的内存

	//todo，把这两个buffer放一起
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	VkBuffer vertexBuffer;//顶点数据buffer
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;//ebo
	VkDeviceMemory indexBufferMemory;
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;


	VkImage textureImage;//从buffer中访问像素也行，但是性能不好；还是从image中访问性能更高
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;//不管是1D还是2D还是3D的图都可以采样

	//vulkan不会自动创建depth buffer，需要自己创建
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;


	VkDescriptorPool descriptorPool;//用于分配descriptor set
	VkDescriptorSet descriptorSet;//descriptor事实上绑定的buffer

	VkCommandBuffer commandBuffer;//cmdbuf

	//用于线程同步的工具
	VkSemaphore imageAvailableSemaphores;//表明这个图可以准备好被渲染了
	VkSemaphore renderFinishedSemaphores;//表明这个图渲染好了可以被显示了
	VkFence inFlightFence;//用于渲染帧之间同步，这里只需要一个fence就行了，因为只有一个queue；如果有多个queue，那可能有必要多个fence

	bool framebufferResized = false;//窗口尺寸是否变化

	//glfw初始化窗口
	void initWindow();

	//初始化vulkan的各种对象
	void initVulkan();

	//主循环
	void mainLoop();

	//程序结束清理各种资源
	void cleanup();

	//创建Vulkan的各种实例
	void createInstance();

	bool checkValidationLayerSupport();

	std::vector<const char*> getRequiredExtensions();

	//vulkan debug回调，第一个参数是debug类型的枚举，第二个是msg的类型，第三个参数是消息本身的一些信息，第四个参数是用户自己传的一些信息
	//返回值是本消息是否会报错
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	void setupDebugMessenger();


	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	static std::vector<char> readFile(const std::string& filename);

	//选择物理显卡硬件
	void pickPhysicalDevice();

	//检查device是否适配
	bool isDeviceSuitable(VkPhysicalDevice device);

	//通过物理device找到合适的Queue Family
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	//创建逻辑device和queue
	void createLogicalDevice();

	//创建窗口系统
	void createSurface();

	//检查PhysicalDevice是否支持extension
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	//获取swap chain支持
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	//选择swap chain的属性，像素格式和颜色空间
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	//选择swap chain的执行模式，就是swap chain中几张图片的交换方式
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	//选择swap chain的分辨率
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	//正式创建一个swap chain
	void createSwapChain();

	//创建swap chain中要使用的VkImageView
	void createImageViews();

	//创建一个渲染流水线
	void createGraphicsPipeline();

	//创建一个shader
	VkShaderModule createShaderModule(const std::vector<char>& code);

	//创建render pass
	void createRenderPass();

	//创建fbo
	void createFramebuffers();

	//创建cmd pool，用于管理cmdbuf的内存
	void createCommandPool();

	//创建cmdbuf，cmdbuf随cmd pool释放，不用显式clean
	void createCommandBuffers();

	//记录cmd buf
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	//开始编辑cmd buf
	VkCommandBuffer beginSingleTimeCommands();

	//结束编辑cmd buf
	void endSingleTimeCommands(VkCommandBuffer cmdbuf);

	//画这一帧
	void drawFrame();

	//创建信号量
	void createSyncObjects();

	//在需要时，比如窗口尺寸改变时重建swap chain
	void recreateSwapChain();

	//清理旧的swap chain
	void cleanupSwapChain();

	//创建顶点数据，vbo/vao
	void createVertexBuffer();

	//找到合适的内存类型，显卡可以提供不同的内存分配类型，每种都有用途和性能的特点，我们需要找到合适的那个
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	//创建buffer的通用函数
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	//拷贝buffer
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	//创建EBO
	void createIndexBuffer();

	//创建UBO容器
	void createDescriptorSetLayout();

	//创建UBO
	void createUniformBuffer();

	//每帧更新UBO
	void updateUniformBuffer(uint32_t currentImage);

	//创建descriptor pool，它是创建descriptor set的工具
	void createDescriptorPool();

	//它才能在绘制时与渲染指令相绑定
	void createDescriptorSets();

	//创建图片文件
	void createTextureImage();

	//创建一张图片
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

	//这是一个helper函数，使image处于正确的layout，这个函数用于处理layout转变
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

	//这是一个helper函数，指定哪一部分的buffer拷贝到哪一部分的image中
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	//创建图片用的image view
	void createTextureImageView();

	//这是一个helper函数，创建image view
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	//创建image sampler，掌管 filtering（nearest，bilinear，各向异性等），addressing mode（repeat，clamp等）
	void createTextureSampler();

	//创建depth buffer
	void createDepthResources();

	//找到支持的depth buffer格式
	VkFormat findDepthFormat();

	//这个格式中有没有stencil buffer支持
	bool hasStencilComponent(VkFormat format);

	//找到支持的图片格式
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	//加载模型
	void loadModel();
};




