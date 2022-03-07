#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <array>

#include <optional>
#include <set>
#include <algorithm>
#include <fstream>

#include <glm/glm.hpp>

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
	glm::vec2 pos;
	glm::vec3 color;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;//在binding数组里的索引
		bindingDescription.stride = sizeof(Vertex);//从一个entry到另一个entry的byte数量，也就是步长
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;//逐顶点还是逐instance

		return bindingDescription;
	}


	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

		//attribute0 顶点坐标
		attributeDescriptions[0].binding = 0;//所属的binding
		attributeDescriptions[0].location = 0;//vertex shader里面写的location
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;//这个attribute的格式
		attributeDescriptions[0].offset = offsetof(Vertex, pos);//这个attribute的offset

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}
};


//todo:自己创建独立的transfer queue
class IndexBuffer {
public:
	void run();

private:
	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;
	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };//需要启用的validation layer的列表
	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };//需要声明的extension的列表

	//interleaving vertex attributes，因为vertex attributes是交错在一起的
	const std::vector<Vertex> vertices = {//顶点clip space坐标，顶点颜色
		{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
		{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
		{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
	};

	const std::vector<uint16_t> indices = {
		0, 1, 2, 2, 3, 0
	};

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

	VkPipelineLayout pipelineLayout;//功能类似于uniform变量，渲染时往shader传参
	VkRenderPass renderPass;//一个render pass
	VkPipeline graphicsPipeline;//一个完整的渲染流水线

	std::vector<VkFramebuffer> swapChainFramebuffers;//我们要为swap chain中每张图都创建一个FBO

	VkCommandPool commandPool;//用于管理cmdbuf的内存

	VkBuffer vertexBuffer;//顶点数据buffer
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;//ebo
	VkDeviceMemory indexBufferMemory;


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
};




