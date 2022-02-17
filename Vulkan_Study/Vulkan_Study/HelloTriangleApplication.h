#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>

//在debug模式开启ValidationLayers，在发布模式关闭
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif


struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;

	bool isComplete() {
		return graphicsFamily.has_value();
	}
};



class HelloTriangleApplication {
public:
	void run();

private:
	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;
	const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};




	GLFWwindow* window;
	VkInstance instance;//应用程序和Vulkan库之间的连接
	VkDebugUtilsMessengerEXT debugMessenger;//专门处理用于vulkan debug的回调
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;//物理显卡访问，instance被删掉时这个device也会被删掉
	VkDevice device;//逻辑接口，一个应用程序里可以有多个
	VkQueue graphicsQueue;//跟随device自动创建

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

	void setupDebugMessenger();


	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	//选择物理显卡硬件
	void pickPhysicalDevice();

	//检查device是否适配
	bool isDeviceSuitable(VkPhysicalDevice device);

	//找到合适的Queue Family
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	void createLogicalDevice();
};