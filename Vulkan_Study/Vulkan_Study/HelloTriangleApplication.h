#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>

//��debugģʽ����ValidationLayers���ڷ���ģʽ�ر�
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
	VkInstance instance;//Ӧ�ó����Vulkan��֮�������
	VkDebugUtilsMessengerEXT debugMessenger;//ר�Ŵ�������vulkan debug�Ļص�
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;//�����Կ����ʣ�instance��ɾ��ʱ���deviceҲ�ᱻɾ��
	VkDevice device;//�߼��ӿڣ�һ��Ӧ�ó���������ж��
	VkQueue graphicsQueue;//����device�Զ�����

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

	void setupDebugMessenger();


	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	//ѡ�������Կ�Ӳ��
	void pickPhysicalDevice();

	//���device�Ƿ�����
	bool isDeviceSuitable(VkPhysicalDevice device);

	//�ҵ����ʵ�Queue Family
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	void createLogicalDevice();
};