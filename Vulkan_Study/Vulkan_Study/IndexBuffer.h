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

//��debugģʽ����ValidationLayers���ڷ���ģʽ�ر�
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif


struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;//Queue familyҪ֧��ͼ��
	std::optional<uint32_t> presentFamily;//Queue familyҪ֧�ִ�����ʾ

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

//��������swapchain
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;//surface�Ļ������ܣ�swap chain�е�ͼƬ�����ʹ�С��
	std::vector<VkSurfaceFormatKHR> formats;//surface��ͼƬ�ĸ�ʽ����ɫ�ռ�
	std::vector<VkPresentModeKHR> presentModes;//���õı���ģʽ
};


struct Vertex {
	glm::vec2 pos;
	glm::vec3 color;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;//��binding�����������
		bindingDescription.stride = sizeof(Vertex);//��һ��entry����һ��entry��byte������Ҳ���ǲ���
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;//�𶥵㻹����instance

		return bindingDescription;
	}


	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

		//attribute0 ��������
		attributeDescriptions[0].binding = 0;//������binding
		attributeDescriptions[0].location = 0;//vertex shader����д��location
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;//���attribute�ĸ�ʽ
		attributeDescriptions[0].offset = offsetof(Vertex, pos);//���attribute��offset

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}
};


//todo:�Լ�����������transfer queue
class IndexBuffer {
public:
	void run();

private:
	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;
	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };//��Ҫ���õ�validation layer���б�
	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };//��Ҫ������extension���б�

	//interleaving vertex attributes����Ϊvertex attributes�ǽ�����һ���
	const std::vector<Vertex> vertices = {//����clip space���꣬������ɫ
		{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
		{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
		{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
	};

	const std::vector<uint16_t> indices = {
		0, 1, 2, 2, 3, 0
	};

	GLFWwindow* window;
	VkInstance instance;//Ӧ�ó����Vulkan��֮�������
	VkDebugUtilsMessengerEXT debugMessenger;//ר�Ŵ�������vulkan debug�Ļص�
	VkSurfaceKHR surface;//Vulkan���ڽӿ�

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;//�����Կ����ʣ�instance��ɾ��ʱ���deviceҲ�ᱻɾ��
	VkDevice device;//�߼��ӿڣ�һ��Ӧ�ó���������ж��


	VkQueue graphicsQueue;//ͼ��queue������device�Զ�����
	VkQueue presentQueue;//��ʾqueue

	VkSwapchainKHR swapChain;//swap chain����
	std::vector<VkImage> swapChainImages;//swap chain�е�ͼƬ
	VkFormat swapChainImageFormat;//swap chain�е�ͼƬ��ʽ
	VkExtent2D swapChainExtent;//swap chain��ͼƬ�ķֱ���
	std::vector<VkImageView> swapChainImageViews;//VkImageView��VkImage����Ⱦ��ˮ���еĴ���

	VkPipelineLayout pipelineLayout;//����������uniform��������Ⱦʱ��shader����
	VkRenderPass renderPass;//һ��render pass
	VkPipeline graphicsPipeline;//һ����������Ⱦ��ˮ��

	std::vector<VkFramebuffer> swapChainFramebuffers;//����ҪΪswap chain��ÿ��ͼ������һ��FBO

	VkCommandPool commandPool;//���ڹ���cmdbuf���ڴ�

	VkBuffer vertexBuffer;//��������buffer
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;//ebo
	VkDeviceMemory indexBufferMemory;


	VkCommandBuffer commandBuffer;//cmdbuf

	//�����߳�ͬ���Ĺ���
	VkSemaphore imageAvailableSemaphores;//�������ͼ����׼���ñ���Ⱦ��
	VkSemaphore renderFinishedSemaphores;//�������ͼ��Ⱦ���˿��Ա���ʾ��
	VkFence inFlightFence;//������Ⱦ֮֡��ͬ��������ֻ��Ҫһ��fence�����ˣ���Ϊֻ��һ��queue������ж��queue���ǿ����б�Ҫ���fence

	bool framebufferResized = false;//���ڳߴ��Ƿ�仯

	//glfw��ʼ������
	void initWindow();

	//��ʼ��vulkan�ĸ��ֶ���
	void initVulkan();

	//��ѭ��
	void mainLoop();

	//����������������Դ
	void cleanup();

	//����Vulkan�ĸ���ʵ��
	void createInstance();

	bool checkValidationLayerSupport();

	std::vector<const char*> getRequiredExtensions();

	//vulkan debug�ص�����һ��������debug���͵�ö�٣��ڶ�����msg�����ͣ���������������Ϣ�����һЩ��Ϣ�����ĸ��������û��Լ�����һЩ��Ϣ
	//����ֵ�Ǳ���Ϣ�Ƿ�ᱨ��
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	void setupDebugMessenger();


	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	static std::vector<char> readFile(const std::string& filename);

	//ѡ�������Կ�Ӳ��
	void pickPhysicalDevice();

	//���device�Ƿ�����
	bool isDeviceSuitable(VkPhysicalDevice device);

	//ͨ������device�ҵ����ʵ�Queue Family
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	//�����߼�device��queue
	void createLogicalDevice();

	//��������ϵͳ
	void createSurface();

	//���PhysicalDevice�Ƿ�֧��extension
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	//��ȡswap chain֧��
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	//ѡ��swap chain�����ԣ����ظ�ʽ����ɫ�ռ�
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	//ѡ��swap chain��ִ��ģʽ������swap chain�м���ͼƬ�Ľ�����ʽ
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	//ѡ��swap chain�ķֱ���
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	//��ʽ����һ��swap chain
	void createSwapChain();

	//����swap chain��Ҫʹ�õ�VkImageView
	void createImageViews();

	//����һ����Ⱦ��ˮ��
	void createGraphicsPipeline();

	//����һ��shader
	VkShaderModule createShaderModule(const std::vector<char>& code);

	//����render pass
	void createRenderPass();

	//����fbo
	void createFramebuffers();

	//����cmd pool�����ڹ���cmdbuf���ڴ�
	void createCommandPool();

	//����cmdbuf��cmdbuf��cmd pool�ͷţ�������ʽclean
	void createCommandBuffers();

	//��¼cmd buf
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	//����һ֡
	void drawFrame();

	//�����ź���
	void createSyncObjects();

	//����Ҫʱ�����細�ڳߴ�ı�ʱ�ؽ�swap chain
	void recreateSwapChain();

	//����ɵ�swap chain
	void cleanupSwapChain();

	//�����������ݣ�vbo/vao
	void createVertexBuffer();

	//�ҵ����ʵ��ڴ����ͣ��Կ������ṩ��ͬ���ڴ�������ͣ�ÿ�ֶ�����;�����ܵ��ص㣬������Ҫ�ҵ����ʵ��Ǹ�
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	//����buffer��ͨ�ú���
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	//����buffer
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	//����EBO
	void createIndexBuffer();
};




