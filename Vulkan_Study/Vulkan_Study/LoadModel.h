#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS//ʹ�û�����Ϊ�������������
#define GLM_FORCE_DEPTH_ZERO_TO_ONE//vulkan�����ֵ��0-1
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
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	//ʵ��hash map�����
	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;//��binding�����������
		bindingDescription.stride = sizeof(Vertex);//��һ��entry����һ��entry��byte������Ҳ���ǲ���
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;//�𶥵㻹����instance

		return bindingDescription;
	}


	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

		//attribute0 ��������
		attributeDescriptions[0].binding = 0;//������binding
		attributeDescriptions[0].location = 0;//vertex shader����д��location
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;//���attribute�ĸ�ʽ
		attributeDescriptions[0].offset = offsetof(Vertex, pos);//���attribute��offset

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

//vulkanҲ�����ݶ����Ҫ��ʹ��C++11������alignas(16)������ʹ��glm�ĺ�GLM_FORCE_DEFAULT_ALIGNED_GENTYPES�ܽ��
struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

//�������̣�
//1.����device���ڴ洴��һ��image object�Ķ���
//2.��һ��ͼƬ�ļ������image objec�������
//3.����һ��image sampler
//4.����һ��combined image sampler descriptor����texture�в�����ɫ

/*
image����layout������
VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: Optimal for presentation
VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: Optimal as attachment for writing colors from the fragment shader
VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: Optimal as source in a transfer operation, like vkCmdCopyImageToBuffer
VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: Optimal as destination in a transfer operation, like vkCmdCopyBufferToImage
VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: Optimal for sampling from a shader
*/


//todo:�Լ�����������transfer queue
//todo:layout����˼û�㶮
//todo����ˮ�ߵ�barrierû�㶮
class LoadModel {
public:
	void run();

private:
	const std::string MODEL_PATH = "D:/Project/Vulkan_Study/Vulkan_Study/Models/viking_room.obj";
	const std::string TEXTURE_PATH = "D:/Project/Vulkan_Study/Vulkan_Study/Textures/viking_room.png";

	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;
	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };//��Ҫ���õ�validation layer���б�
	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };//��Ҫ������extension���б�

	

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

	VkDescriptorSetLayout descriptorSetLayout;//ָ��pipelineҪ���ʵ���Դ����
	VkPipelineLayout pipelineLayout;//����������uniform��������Ⱦʱ��shader����
	VkRenderPass renderPass;//һ��render pass
	VkPipeline graphicsPipeline;//һ����������Ⱦ��ˮ��

	std::vector<VkFramebuffer> swapChainFramebuffers;//����ҪΪswap chain��ÿ��ͼ������һ��FBO

	VkCommandPool commandPool;//���ڹ���cmdbuf���ڴ�

	//todo����������buffer��һ��
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	VkBuffer vertexBuffer;//��������buffer
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;//ebo
	VkDeviceMemory indexBufferMemory;
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;


	VkImage textureImage;//��buffer�з�������Ҳ�У��������ܲ��ã����Ǵ�image�з������ܸ���
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;//������1D����2D����3D��ͼ�����Բ���

	//vulkan�����Զ�����depth buffer����Ҫ�Լ�����
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;


	VkDescriptorPool descriptorPool;//���ڷ���descriptor set
	VkDescriptorSet descriptorSet;//descriptor��ʵ�ϰ󶨵�buffer

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

	//��ʼ�༭cmd buf
	VkCommandBuffer beginSingleTimeCommands();

	//�����༭cmd buf
	void endSingleTimeCommands(VkCommandBuffer cmdbuf);

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

	//����UBO����
	void createDescriptorSetLayout();

	//����UBO
	void createUniformBuffer();

	//ÿ֡����UBO
	void updateUniformBuffer(uint32_t currentImage);

	//����descriptor pool�����Ǵ���descriptor set�Ĺ���
	void createDescriptorPool();

	//�������ڻ���ʱ����Ⱦָ�����
	void createDescriptorSets();

	//����ͼƬ�ļ�
	void createTextureImage();

	//����һ��ͼƬ
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

	//����һ��helper������ʹimage������ȷ��layout������������ڴ���layoutת��
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

	//����һ��helper������ָ����һ���ֵ�buffer��������һ���ֵ�image��
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	//����ͼƬ�õ�image view
	void createTextureImageView();

	//����һ��helper����������image view
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	//����image sampler���ƹ� filtering��nearest��bilinear���������Եȣ���addressing mode��repeat��clamp�ȣ�
	void createTextureSampler();

	//����depth buffer
	void createDepthResources();

	//�ҵ�֧�ֵ�depth buffer��ʽ
	VkFormat findDepthFormat();

	//�����ʽ����û��stencil buffer֧��
	bool hasStencilComponent(VkFormat format);

	//�ҵ�֧�ֵ�ͼƬ��ʽ
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	//����ģ��
	void loadModel();
};




