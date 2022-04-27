//#pragma once
//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>
//
//#include <iostream>
//#include <stdexcept>
//#include <cstdlib>
//#include <vector>
//#include <optional>
//#include <set>
//#include <algorithm>
//#include <fstream>
//
////��debugģʽ����ValidationLayers���ڷ���ģʽ�ر�
//#ifdef NDEBUG
//const bool enableValidationLayers = false;
//#else
//const bool enableValidationLayers = true;
//#endif
//
//
//struct QueueFamilyIndices {
//	std::optional<uint32_t> graphicsFamily;//Queue familyҪ֧��ͼ��
//	std::optional<uint32_t> presentFamily;//Queue familyҪ֧�ִ�����ʾ
//
//	bool isComplete() {
//		return graphicsFamily.has_value()&&presentFamily.has_value();
//	}
//};
//
////��������swapchain
//struct SwapChainSupportDetails {
//	VkSurfaceCapabilitiesKHR capabilities;//surface�Ļ������ܣ�swap chain�е�ͼƬ�����ʹ�С��
//	std::vector<VkSurfaceFormatKHR> formats;//surface��ͼƬ�ĸ�ʽ����ɫ�ռ�
//	std::vector<VkPresentModeKHR> presentModes;//���õı���ģʽ
//};
//
//
//
//class HelloTriangleApplication {
//public:
//	void run();
//
//private:
//	const uint32_t WIDTH = 800;
//	const uint32_t HEIGHT = 600;
//	const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};//��Ҫ���õ�validation layer���б�
//	const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};//��Ҫ������extension���б�
//
//	GLFWwindow* window;
//	VkInstance instance;//Ӧ�ó����Vulkan��֮�������
//	VkDebugUtilsMessengerEXT debugMessenger;//ר�Ŵ�������vulkan debug�Ļص�
//	VkSurfaceKHR surface;//Vulkan���ڽӿ�
//
//	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;//�����Կ����ʣ�instance��ɾ��ʱ���deviceҲ�ᱻɾ��
//	VkDevice device;//�߼��ӿڣ�һ��Ӧ�ó���������ж��
//
//
//	VkQueue graphicsQueue;//ͼ��queue������device�Զ�����
//	VkQueue presentQueue;//��ʾqueue
//
//	VkSwapchainKHR swapChain;//swap chain����
//	std::vector<VkImage> swapChainImages;//swap chain�е�ͼƬ
//	VkFormat swapChainImageFormat;//swap chain�е�ͼƬ��ʽ
//	VkExtent2D swapChainExtent;//swap chain��ͼƬ�ķֱ���
//	std::vector<VkImageView> swapChainImageViews;//VkImageView��VkImage����Ⱦ��ˮ���еĴ���
//
//	VkPipelineLayout pipelineLayout;//����������uniform��������Ⱦʱ��shader����
//	VkRenderPass renderPass;//һ��render pass
//	VkPipeline graphicsPipeline;//һ����������Ⱦ��ˮ��
//
//	std::vector<VkFramebuffer> swapChainFramebuffers;//����ҪΪswap chain��ÿ��ͼ������һ��FBO
//
//	VkCommandPool commandPool;//���ڹ���cmdbuf���ڴ�
//	VkCommandBuffer commandBuffer;//cmdbuf
//
//	//�����߳�ͬ���Ĺ���
//	VkSemaphore imageAvailableSemaphores;//�������ͼ����׼���ñ���Ⱦ��
//	VkSemaphore renderFinishedSemaphores;//�������ͼ��Ⱦ���˿��Ա���ʾ��
//	VkFence inFlightFence;//������Ⱦ֮֡��ͬ��������ֻ��Ҫһ��fence�����ˣ���Ϊֻ��һ��queue������ж��queue���ǿ����б�Ҫ���fence
//
//
//
//	//glfw��ʼ������
//	void initWindow();
//
//	//��ʼ��vulkan�ĸ��ֶ���
//	void initVulkan();
//
//	//��ѭ��
//	void mainLoop();
//
//	//����������������Դ
//	void cleanup();
//
//	//����Vulkan�ĸ���ʵ��
//	void createInstance();
//
//	bool checkValidationLayerSupport();
//
//	std::vector<const char*> getRequiredExtensions();
//
//	//vulkan debug�ص�����һ��������debug���͵�ö�٣��ڶ�����msg�����ͣ���������������Ϣ�����һЩ��Ϣ�����ĸ��������û��Լ�����һЩ��Ϣ
//	//����ֵ�Ǳ���Ϣ�Ƿ�ᱨ��
//	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
//		VkDebugUtilsMessageTypeFlagsEXT messageType,
//		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
//		void* pUserData);
//
//	void setupDebugMessenger();
//
//
//	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
//
//	static std::vector<char> readFile(const std::string& filename);
//
//	//ѡ�������Կ�Ӳ��
//	void pickPhysicalDevice();
//
//	//���device�Ƿ�����
//	bool isDeviceSuitable(VkPhysicalDevice device);
//
//	//ͨ������device�ҵ����ʵ�Queue Family
//	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
//
//	//�����߼�device��queue
//	void createLogicalDevice();
//
//	//��������ϵͳ
//	void createSurface();
//
//	//���PhysicalDevice�Ƿ�֧��extension
//	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
//
//	//��ȡswap chain֧��
//	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
//
//	//ѡ��swap chain�����ԣ����ظ�ʽ����ɫ�ռ�
//	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
//
//	//ѡ��swap chain��ִ��ģʽ������swap chain�м���ͼƬ�Ľ�����ʽ
//	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
//
//	//ѡ��swap chain�ķֱ���
//	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
//
//	//��ʽ����һ��swap chain
//	void createSwapChain();
//
//	//����swap chain��Ҫʹ�õ�VkImageView
//	void createImageViews();
//
//	//����һ����Ⱦ��ˮ��
//	void createGraphicsPipeline();
//
//	//����һ��shader
//	VkShaderModule createShaderModule(const std::vector<char>& code);
//
//	//����render pass
//	void createRenderPass();
//
//	//����fbo
//	void createFramebuffers();
//
//	//����cmd pool�����ڹ���cmdbuf���ڴ�
//	void createCommandPool();
//
//	//����cmdbuf��cmdbuf��cmd pool�ͷţ�������ʽclean
//	void createCommandBuffers();
//
//	//��¼cmd buf
//	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
//
//	//����һ֡
//	void drawFrame();
//
//	//�����ź���
//	void createSyncObjects();
//};