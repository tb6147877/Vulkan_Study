#include "VulkanSetup.h"

#include <iostream>
#include <set>
#include <stdexcept>

#include "Utils.h"
#include "AppConstants.h"


void VulkanSetup::initSetup(GLFWwindow* window) {
	//keep a reference to the glfw window for now 
	_window = window;
	//start by creating a vulkan instance
	createInstance();

	//setup the debug messenger with the validation layers
	setupDebugMessenger();

	//create the surface to draw to
	createSurface();

	//pick the physical device we wat to use, making sure it is appropriate
	pickPhysicalDevice();

	//create the logical device for interfacing with the physical device
	createLogicalDevice();

	_setupComplete=true;
}


void VulkanSetup::cleanupSetup() {
	vkDestroyDevice(_device,nullptr);

	vkDestroySurfaceKHR(_instance,_surface,nullptr);
	
	if (enableValidationLayers)
	{
		destroyDebugUtilsMessengerEXT(_instance,nullptr,_debugMessenger);
	}

	//only called before program exits, destroys the vulkan instance
	vkDestroyInstance(_instance,nullptr);
}

void VulkanSetup::createInstance() {
	//if we have enabled validation layers and some requested layers aren't available, throw error
	if (enableValidationLayers&&!checkValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	//tells the driver how to optimise for our purpose
	VkApplicationInfo appInfo{};
	appInfo.sType=VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName=APP_NAME.data();
	appInfo.applicationVersion=VK_MAKE_VERSION(1,0,0);
	appInfo.pEngineName=ENGINE_NAME.data();
	appInfo.engineVersion=VK_MAKE_VERSION(1,0,0);
	appInfo.apiVersion=VK_API_VERSION_1_0;

	//create a VkInstanceCreateInfo struct, not optional!
	VkInstanceCreateInfo createInfo{};
	createInfo.sType=VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo=&appInfo;
	auto extensions=getRequiredExtensions();
	createInfo.enabledExtensionCount=static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames=extensions.data();

	//create a debug messenger before the instance is created to capture any errors in creation process
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount=static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames=validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext=(VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}else
	{
		createInfo.enabledLayerCount=0;
		createInfo.pNext=nullptr;
	}

	//we can now create the instance(pointer to struct, pointer to custom allocator callback,
	//pointer to handle that stores the new object)
	if (vkCreateInstance(&createInfo,nullptr,&_instance)!=VK_SUCCESS)
	{
		throw std::runtime_error("failed to create a vulkan instance!");
	}
}


std::vector<const char*> VulkanSetup::getRequiredExtensions() {
	//start by getting the glfw extensions, necessary for displaying something in a window.
	//vulkan is platform agnostic, so need an extension to interface with window system.
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions=glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	
	//glfwExtensions is an array of strings
	std::vector<const char*> result(glfwExtensions,glfwExtensions+glfwExtensionCount);

	//add the VK_EXT_debug_utils with macro on condition debug is activated
	if (enableValidationLayers)
	{
		result.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	
	return result;
}

void VulkanSetup::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	//the creation of a messenger create info is put in a separate function for use to debug the creation and
	//destruction of a VkInstance object
	createInfo={};
	createInfo.sType=VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	//types of callbacks to be called for
	createInfo.messageSeverity=VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT|VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	if (enableVerboseValidation)
	{
		createInfo.messageSeverity|=VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
	}
	//fliter which message type filtered by callback
	createInfo.messageType=VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT|VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT|VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	//pointer to callback function
	createInfo.pfnUserCallback=debugCallback;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanSetup::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	// some user data
	// message severity flags, values can be used to check how message compares to a certain level of severity
	// VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: Diagnostic message
	// VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT : Informational message like the creation of a resource
	// VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT : Message about behavior that is not necessarily an error, but very likely a bug in your application
	// VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT : Message about behavior that is invalid and may cause crashes
	// message type flags
	// VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: Some event has happened that is unrelated to the specification or performance
	// VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT : Something has happened that violates the specification or indicates a possible mistake
	// VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT : Potential non - optimal use of Vulkan
	// refers to a struct with the details of the message itself
	// pMessage : The debug message as a null - terminated string
	// pObjects : Array of Vulkan object handles related to the message
	// objectCount : Number of objects in array
	std::cerr<<"wdw validation layer:"<<pCallbackData->pMessage<<std::endl;
	return VK_FALSE;
}


bool VulkanSetup::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount,nullptr);
	std::vector<VkLayerProperties> availabelLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount,availabelLayers.data());

	for (const char* layerName:validationLayers)
	{
		bool layerFound=false;
		for (const auto& layerProperties:availabelLayers)
		{
			if (strcmp(layerName,layerProperties.layerName)==0)
			{
				layerFound=true;
				break;
			}
		}
		if (!layerFound)
		{
			return false;
		}
	}
	return true;
}

void VulkanSetup::setupDebugMessenger()
{
	if (!enableValidationLayers)
	{
		return;
	}

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	populateDebugMessengerCreateInfo(createInfo);
	if (createDebugUtilsMessengerEXT(_instance,&createInfo,nullptr,&_debugMessenger)!=VK_SUCCESS)
	{
		throw std::runtime_error("failed to setup debug messenger!");
	}
}

//proxy function handles finding the extension function vkCreateDebugUtilsMessengerEXT
VkResult VulkanSetup::createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
		const VkAllocationCallbacks* pAllocator,VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	//vkGetInstanceProcAddr returns nullptr if the debug messenger creator function couldn't be loaded
	auto func=(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,"vkCreateDebugUtilsMessengerEXT");

	if (func!=nullptr)
	{
		return func(instance,pCreateInfo,pAllocator,pDebugMessenger);
	}else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

//similar to above function but for destroying a debug messenger
void VulkanSetup::destroyDebugUtilsMessengerEXT(VkInstance instance,const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT debugMessenger)
{
	auto func=(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,"vkDestroyDebugUtilsMessengerEXT");
	if (func!=nullptr)
	{
		func(instance,debugMessenger,pAllocator);
	}
}

void VulkanSetup::createSurface()
{
	if (glfwCreateWindowSurface(_instance,_window,nullptr,&_surface)!=VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}
}

void VulkanSetup::pickPhysicalDevice()
{
	//get the physical devices available
	uint32_t deviceCount=0;
	vkEnumeratePhysicalDevices(_instance,&deviceCount,nullptr);

	if (deviceCount==0)
	{
		throw std::runtime_error("failed to find GPUs with vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(_instance,&deviceCount,devices.data());


	for (const auto& device:devices)
	{
		if (isDeviceSuitable(device))
		{
			_physicalDevice=device;
			break;
		}
	}

	if (_physicalDevice==VK_NULL_HANDLE)
	{
		throw std::runtime_error("failed to find a suitable GPU!");
		return;
	}

	vkGetPhysicalDeviceProperties(_physicalDevice,&_deviceProperties);
}

bool VulkanSetup::isDeviceSuitable(VkPhysicalDevice device)
{
	utils::QueuefamilyIndices indices=utils::QueuefamilyIndices::findQueueFamilies(device,_surface);
	bool extensionsSupported=checkDeviceExtensionSupport(device);

	bool swapChainAdequate=false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport=querySwapchainSupport(device);
		//at least one supported image format and presentation mode is sufficient for now
		swapChainAdequate=!swapChainSupport.formats.empty()&&!swapChainSupport.presentModes.empty();
	}

	//get the device's supported features
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device,&supportedFeatures);

	//samplerAnisotropy is not necessary
	return indices.isComplete()&&extensionsSupported&&swapChainAdequate&&supportedFeatures.samplerAnisotropy;
}

bool VulkanSetup::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(),deviceExtensions.end());

	for (const auto& extension:availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	//if the required extensions vector is empty, then they were erased because they are available
	return requiredExtensions.empty();
}

VulkanSetup::SwapChainSupportDetails VulkanSetup::querySwapchainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device,_surface,&details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device,_surface,&formatCount,nullptr);
	if (formatCount!=0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device,_surface,&formatCount,details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device,_surface,&presentModeCount,nullptr);
	if (presentModeCount!=0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device,_surface,&presentModeCount,details.presentModes.data());
	}

	return details;
}

void VulkanSetup::createLogicalDevice()
{
	//query the queue families available on the device
	utils::QueuefamilyIndices indices=utils::QueuefamilyIndices::findQueueFamilies(_physicalDevice,_surface);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	//using a set makes sure that there are no duplicate references to a same queue
	std::set<uint32_t> uniqueQueueFamilies={indices.graphicsFamily.value(),indices.presentFamily.value()};

	float queuePriority=1.0;
	for (uint32_t queueFamily:uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCraeteInfo{};
		queueCraeteInfo.sType=VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCraeteInfo.queueFamilyIndex=queueFamily;
		queueCraeteInfo.queueCount=1;
		queueCraeteInfo.pQueuePriorities=&queuePriority;
		queueCreateInfos.push_back(queueCraeteInfo);
	}

	//queries support certain features(like geometry shader, other things in the vulkan pipeline)
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy=VK_TRUE;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType=VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount=static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos=queueCreateInfos.data();//raw underlying array
	createInfo.pEnabledFeatures=&deviceFeatures;
	createInfo.ppEnabledExtensionNames=deviceExtensions.data();

	if (enableValidationLayers)
	{
		//these fields are ignored by newer vulkan implementations
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames=validationLayers.data();
	}else
	{
		createInfo.enabledLayerCount =0;
	}

	if (vkCreateDevice(_physicalDevice,&createInfo,nullptr,&_device)!=VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	//set the graphics queue and the presentation queue handle, only want a single queue so use index 0
	vkGetDeviceQueue(_device,indices.graphicsFamily.value(),0,&_graphicsQueue);
	vkGetDeviceQueue(_device,indices.presentFamily.value(),0,&_presentQueue);
}