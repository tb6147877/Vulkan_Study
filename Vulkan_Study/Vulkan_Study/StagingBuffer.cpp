//#include "StagingBuffer.h"
//
////����һ�����������������ز�ִ��vkCreateDebugUtilsMessengerEXT()
//VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
//	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
//	if (func != nullptr) {
//		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
//	}
//	else {
//		return VK_ERROR_EXTENSION_NOT_PRESENT;
//	}
//}
//
////����һ�����������������ز�ִ��vkDestroyDebugUtilsMessengerEXT()
//void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
//	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
//	if (func != nullptr) {
//		func(instance, debugMessenger, pAllocator);
//	}
//}
//
//
//void StagingBuffer::run() {
//	initWindow();
//	initVulkan();
//	mainLoop();
//	cleanup();
//}
//
////ʹ��glfw����ʼ������
//void StagingBuffer::initWindow() {
//	glfwInit();
//
//	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//
//	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
//
//	glfwSetWindowUserPointer(window, this);//�������ʵ����window��
//	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
//}
//
//void StagingBuffer::initVulkan() {
//	createInstance();
//	setupDebugMessenger();
//	createSurface();
//	pickPhysicalDevice();
//	createLogicalDevice();
//	createSwapChain();
//	createImageViews();
//	createRenderPass();
//	createGraphicsPipeline();
//	createFramebuffers();
//	createCommandPool();
//	createVertexBuffer();
//	createCommandBuffers();
//	createSyncObjects();
//}
//
//void StagingBuffer::pickPhysicalDevice() {
//	uint32_t deviceCount = 0;
//	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
//
//	if (deviceCount == 0) {
//		throw std::runtime_error("failed to find GPUs with Vulkan support!");
//	}
//
//	std::vector<VkPhysicalDevice> devices(deviceCount);
//	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
//
//	for (const auto& device : devices) {
//		if (isDeviceSuitable(device)) {
//			physicalDevice = device;
//			break;
//		}
//	}
//
//	if (physicalDevice == VK_NULL_HANDLE) {
//		throw std::runtime_error("failed to find a suitable GPU!");
//	}
//
//}
//
//
//void StagingBuffer::createLogicalDevice() {
//	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
//
//	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
//	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
//
//	float queuePriority = 1.0f;
//	for (uint32_t queueFamily : uniqueQueueFamilies) {
//		VkDeviceQueueCreateInfo queueCreateInfo{};
//		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
//		queueCreateInfo.queueFamilyIndex = queueFamily;
//		queueCreateInfo.queueCount = 1;
//		queueCreateInfo.pQueuePriorities = &queuePriority;
//		queueCreateInfos.push_back(queueCreateInfo);
//	}
//
//	VkPhysicalDeviceFeatures deviceFeatures{};
//
//	VkDeviceCreateInfo createInfo{};
//	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
//
//	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
//	createInfo.pQueueCreateInfos = queueCreateInfos.data();
//
//	//����logical device
//	createInfo.pEnabledFeatures = &deviceFeatures;
//	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
//	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
//
//	if (enableValidationLayers) {
//		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
//		createInfo.ppEnabledLayerNames = validationLayers.data();
//	}
//	else {
//		createInfo.enabledLayerCount = 0;
//	}
//
//	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
//		throw std::runtime_error("failed to create logical device!");
//	}
//
//	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
//	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
//}
//
//void StagingBuffer::createSurface() {
//	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
//		throw std::runtime_error("failed to create window surface!");
//	}
//}
//
//bool StagingBuffer::checkDeviceExtensionSupport(VkPhysicalDevice device) {
//	uint32_t extensionCount;
//	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
//
//	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
//	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
//
//	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
//
//	for (const auto& extension : availableExtensions) {
//		requiredExtensions.erase(extension.extensionName);
//	}
//
//	return requiredExtensions.empty();
//}
//
//SwapChainSupportDetails StagingBuffer::querySwapChainSupport(VkPhysicalDevice device) {
//	SwapChainSupportDetails details;
//	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
//
//	uint32_t formatCount;
//	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
//
//	if (formatCount != 0) {
//		details.formats.resize(formatCount);
//		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
//	}
//
//	uint32_t presentModeCount;
//	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
//
//	if (presentModeCount != 0) {
//		details.presentModes.resize(presentModeCount);
//		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
//	}
//
//	return details;
//}
//
//
//VkSurfaceFormatKHR StagingBuffer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
//	for (const auto& availableFormat : availableFormats) {
//		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
//			return availableFormat;
//		}
//	}
//
//	return availableFormats[0];
//}
//
//
//VkPresentModeKHR StagingBuffer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
//	for (const auto& availablePresentMode : availablePresentModes) {
//		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
//			return availablePresentMode;
//		}
//	}
//
//	return VK_PRESENT_MODE_FIFO_KHR;
//}
//
//
//
//VkExtent2D StagingBuffer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
//	if (capabilities.currentExtent.width != UINT32_MAX) {
//		return capabilities.currentExtent;
//	}
//	else {//���������һЩwindow manager�������������Լ��ķֱ���
//		int width, height;
//		glfwGetFramebufferSize(window, &width, &height);
//
//		VkExtent2D actualExtent = {
//			static_cast<uint32_t>(width),
//			static_cast<uint32_t>(height)
//		};
//
//		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
//		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
//
//		return actualExtent;
//	}
//}
//
//
//void StagingBuffer::createSwapChain() {
//	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
//
//	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
//	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
//	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
//
//	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;//�Ƽ���swap chain�е�ͼƬ����+1����ֹ�Կ������ȴ�
//
//	//��ֹ����swap chain�����ͼƬ����
//	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
//		imageCount = swapChainSupport.capabilities.maxImageCount;
//	}
//	std::cout << "\nswap chain image number:" << imageCount << "\n";
//
//	//����swap chain�Ľṹ
//	VkSwapchainCreateInfoKHR createInfo{};
//	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
//	createInfo.surface = surface;
//
//	createInfo.minImageCount = imageCount;
//	createInfo.imageFormat = surfaceFormat.format;
//	createInfo.imageColorSpace = surfaceFormat.colorSpace;
//	createInfo.imageExtent = extent;
//	createInfo.imageArrayLayers = 1;//һ�㶼��1�����ǿ��������۲�ͬ��3D��Ϸ
//	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;//ͼƬֱ����Ⱦ�������Ļ������ǴӺ����Ǳ����ľ���VK_IMAGE_USAGE_TRANSFER_DST_BIT
//
//	//����������Ҫָ��swap chain���������family queueʹ��
//	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
//	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
//
//	if (indices.graphicsFamily != indices.presentFamily) {//���graphics queue family��ͬ��presentation queue family
//		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;//����ģʽ��ͼƬ���Ա����queue family���ã�����Ҫ��ʽ��ת������Ȩ
//		createInfo.queueFamilyIndexCount = 2;//������queue family����
//		createInfo.pQueueFamilyIndices = queueFamilyIndices;//����Щqueue family����
//	}
//	else {
//		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;//ר��ģʽ��һ��ͼƬ��һ��ʱ��ֻ�ܱ�һ��queue familyӵ�У�������ʽ��ת������Ȩ���������
//	}
//
//	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;//Ӧ�õ�swap chain��ͼƬ�ı任������˵ˮƽ��ת����ת90�ȣ�����ѡ���ޱ任
//	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;//�ں��������ڻ��ʱAlphaͨ���Ƿ񷢻����ã�����ѡ��
//	createInfo.presentMode = presentMode;
//	createInfo.clipped = VK_TRUE;//true������ע�����ڱ��ͼƬ���������
//
//	createInfo.oldSwapchain = VK_NULL_HANDLE;//���ڶ��swap chain�л������
//
//	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
//		throw std::runtime_error("failed to create swap chain!");
//	}
//
//	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
//	swapChainImages.resize(imageCount);
//	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
//
//	swapChainImageFormat = surfaceFormat.format;
//	swapChainExtent = extent;
//}
//
//
//
//void StagingBuffer::createImageViews() {
//	swapChainImageViews.resize(swapChainImages.size());
//
//	for (size_t i = 0; i < swapChainImages.size(); i++) {
//		//����VkImageView�Ľṹ��
//		VkImageViewCreateInfo createInfo{};
//		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//		createInfo.image = swapChainImages[i];
//		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
//		createInfo.format = swapChainImageFormat;
//		//ͼƬ��ÿ��ͨ���Ƿ�����һ��
//		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
//		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
//		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
//		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
//		//����ͼƬ����;
//		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//		createInfo.subresourceRange.baseMipLevel = 0;
//		createInfo.subresourceRange.levelCount = 1;
//		createInfo.subresourceRange.baseArrayLayer = 0;
//		createInfo.subresourceRange.layerCount = 1;
//
//		if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
//			throw std::runtime_error("failed to create image views!");
//		}
//	}
//
//}
//
//
//void StagingBuffer::createGraphicsPipeline() {
//	auto vertShaderCode = readFile("D:/Project/Vulkan_Study/Vulkan_Study/Shaders/vert02.spv");
//	auto fragShaderCode = readFile("D:/Project/Vulkan_Study/Vulkan_Study/Shaders/frag01.spv");
//
//	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
//	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
//
//	//������ˮ���ж�Ӧshader
//	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
//	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
//	vertShaderStageInfo.module = vertShaderModule;
//	vertShaderStageInfo.pName = "main";
//
//	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
//	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
//	fragShaderStageInfo.module = fragShaderModule;
//	fragShaderStageInfo.pName = "main";
//	//pSpecializationInfo����ֶκ��б�Ҫ���ۣ�������ָ��shader�еĳ���������һ��shader�ڲ�ͬʱ����ֵ��ͬ���������ڴ�����ˮ��ʱ��ָ���ã���������Ⱦʱ�򴫱���ֵ��ȥ���ܸ���
//
//	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
//
//	//�����������붥��shader�����ݸ�ʽ����������VBO
//	//binding:�����м�Ŀ�϶�Լ��������𶥵�Ļ�����instance��
//	//Attribute descriptions:Attribute�ĸ�ʽ�Լ�offset
//	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
//	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//	auto bindingDescription = Vertex::getBindingDescription();
//	auto attributeDescriptions = Vertex::getAttributeDescriptions();
//	vertexInputInfo.vertexBindingDescriptionCount = 1;
//	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
//	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
//	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
//
//	//��������primitive�������Լ��Ƿ�����ͼԪ������0xFFFF or 0xFFFFFFFF��
//	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
//	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
//	inputAssembly.primitiveRestartEnable = VK_FALSE;
//
//	//������ȾҪ�������FBO������
//	VkViewport viewport{};
//	viewport.x = 0.0f;
//	viewport.y = 0.0f;
//	viewport.width = (float)swapChainExtent.width;//��Ⱦʱ��̬�ɱ�
//	viewport.height = (float)swapChainExtent.height;
//	viewport.minDepth = 0.0f;//depth�ķ�Χ��������0.0f��1.0f֮�䣬minDepth���Ա�maxDepth���������γ�������������ϵ���л�
//	viewport.maxDepth = 1.0f;
//
//	//��������
//	VkRect2D scissor{};
//	scissor.offset = { 0, 0 };
//	scissor.extent = swapChainExtent;
//
//	VkPipelineViewportStateCreateInfo viewportState{};
//	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//	viewportState.viewportCount = 1;
//	viewportState.pViewports = &viewport;
//	viewportState.scissorCount = 1;
//	viewportState.pScissors = &scissor;
//
//	//��դ���׶�ִ����Ȳ��ԣ����޳������������Լ������ȫ����������߿����
//	VkPipelineRasterizationStateCreateInfo rasterizer{};
//	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//	rasterizer.depthClampEnable = VK_FALSE;//�����true�����ڽ������Զ����֮���fragment�ᱻclamp������discard����������ڴ�����Ӱʱ��������
//	rasterizer.rasterizerDiscardEnable = VK_FALSE;//�����true�ͻ��ֹ�κ������FBO
//	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;//�����ʾ���߿���ʾ������ʾ
//	rasterizer.lineWidth = 1.0f;//�ߵĴ�ϸ����Ⱦʱ��̬�ɱ�
//	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;//�޳�����
//	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;//����������
//	rasterizer.depthBiasEnable = VK_FALSE;//�Ƿ�����depth bias������Ӱ��������
//
//	//�����������������������Ϣ
//	VkPipelineMultisampleStateCreateInfo multisampling{};
//	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//	multisampling.sampleShadingEnable = VK_FALSE;
//	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
//
//	//��ɫ��ϣ������ַ�ʽ��һ���ǻ�Ͼ�ֵ����ֵ�ó�һ��������ɫ����һ����λ�������ϲ���ֵ����ֵ
//	//����ṹ������������FBO��blend����
//	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
//	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
//	colorBlendAttachment.blendEnable = VK_FALSE;
//	//alpha blend������
//	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
//	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
//	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
//	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
//	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
//	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
//
//	//����ṹ��������������FBO��blend
//	VkPipelineColorBlendStateCreateInfo colorBlending{};
//	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//	colorBlending.logicOpEnable = VK_FALSE;
//	colorBlending.logicOp = VK_LOGIC_OP_COPY;//����������ó�λ����
//	colorBlending.attachmentCount = 1;
//	colorBlending.pAttachments = &colorBlendAttachment;
//	colorBlending.blendConstants[0] = 0.0f;//��Ⱦʱ��̬�ɱ�
//	colorBlending.blendConstants[1] = 0.0f;
//	colorBlending.blendConstants[2] = 0.0f;
//	colorBlending.blendConstants[3] = 0.0f;
//
//	//����Ⱦʱ�������
//	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
//	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//	pipelineLayoutInfo.setLayoutCount = 0;
//	pipelineLayoutInfo.pushConstantRangeCount = 0;
//
//	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
//		throw std::runtime_error("failed to create pipeline layout!");
//	}
//
//	//����pipeline
//	VkGraphicsPipelineCreateInfo pipelineInfo{};
//	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//	pipelineInfo.stageCount = 2;//vs��fs����stage
//	pipelineInfo.pStages = shaderStages;
//	pipelineInfo.pVertexInputState = &vertexInputInfo;
//	pipelineInfo.pInputAssemblyState = &inputAssembly;
//	pipelineInfo.pViewportState = &viewportState;
//	pipelineInfo.pRasterizationState = &rasterizer;
//	pipelineInfo.pMultisampleState = &multisampling;
//	pipelineInfo.pColorBlendState = &colorBlending;
//	pipelineInfo.layout = pipelineLayout;
//	pipelineInfo.renderPass = renderPass;
//	pipelineInfo.subpass = 0;//subpass��index
//
//	//�������ֶ����ڴ�һ�����е�pipeline�п��ٴ���һ����pipeline�����ڸĶ���������
//	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
//	pipelineInfo.basePipelineIndex = -1;
//
//	//�����������һ���Դ������pipeline
//	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
//		throw std::runtime_error("failed to create graphics pipeline!");
//	}
//
//	vkDestroyShaderModule(device, fragShaderModule, nullptr);
//	vkDestroyShaderModule(device, vertShaderModule, nullptr);
//}
//
//
//VkShaderModule StagingBuffer::createShaderModule(const std::vector<char>& code) {
//	VkShaderModuleCreateInfo createInfo{};
//	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
//	createInfo.codeSize = code.size();
//	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
//
//	VkShaderModule shaderModule;
//	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
//		throw std::runtime_error("failed to create shader module!");
//	}
//
//	return shaderModule;
//}
//
//
//void StagingBuffer::createRenderPass() {
//	VkAttachmentDescription colorAttachment{};
//	colorAttachment.format = swapChainImageFormat;//��������swap chainһ��
//	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;//����е���������Ⱦǰ��ô����Ӧ������ɫ��depth���
//	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;//����е���������Ⱦ����ô����Ӧ������ɫ��depth���
//	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;//����е���������Ⱦǰ��ô����Ӧ����stencil���
//	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;//����е���������Ⱦ����ô����Ӧ����stencil���
//	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;//��ʾ���ͼ����Ⱦǰ�����������ﷴ�����Ƕ�Ҫ������ݣ����Բ��ں���������
//	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;//��ʾ���ͼ��Ⱦ��Ҫȥ�ģ�������Ҫȥswap chain
//
//	//ÿ��subpass��������һ���������
//	VkAttachmentReference colorAttachmentRef{};
//	colorAttachmentRef.attachment = 0;//�ڲ��������������0
//	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;//��۵���;����ɫ
//
//	//��������subpass����������ֻ��һ��subpass
//	VkSubpassDescription subpass{};
//	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;//��������һ��ͼ��subpass
//	subpass.colorAttachmentCount = 1;
//	subpass.pColorAttachments = &colorAttachmentRef;//fragment shader��layout(location = 0) out vec4 outColor
//
//	//subpass���ڴ���render pass��fbo�Ĳ��ͼƬ�Ĵ�������
//	VkSubpassDependency dependency{};
//	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;//������srcָ��render passǰ��һ����ʽsubpass
//	dependency.dstSubpass = 0;//dstSubpassҪ��srcSubpass���Է�����ѭ�����ã���Ȼ����һ�������VK_SUBPASS_EXTERNAL��û�£�������dstָ��render pass���һ����ʽsubpass
//	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;//�ȵ�swap chain��ȡ���ٽ���subpass fbo����
//	dependency.srcAccessMask = 0;
//	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;//�ȵ�swap chainд���ٽ���subpass fbo����
//	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//
//
//	//����render pass
//	VkRenderPassCreateInfo renderPassInfo{};
//	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//	renderPassInfo.attachmentCount = 1;
//	renderPassInfo.pAttachments = &colorAttachment;
//	renderPassInfo.subpassCount = 1;
//	renderPassInfo.pSubpasses = &subpass;
//	renderPassInfo.dependencyCount = 1;
//	renderPassInfo.pDependencies = &dependency;
//
//	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
//		throw std::runtime_error("failed to create render pass!");
//	}
//}
//
//
//void StagingBuffer::createFramebuffers() {
//	swapChainFramebuffers.resize(swapChainImageViews.size());
//
//	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
//		VkImageView attachments[] = {
//			swapChainImageViews[i]
//		};
//
//		VkFramebufferCreateInfo framebufferInfo{};
//		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//		framebufferInfo.renderPass = renderPass;
//		framebufferInfo.attachmentCount = 1;
//		framebufferInfo.pAttachments = attachments;
//		framebufferInfo.width = swapChainExtent.width;
//		framebufferInfo.height = swapChainExtent.height;
//		framebufferInfo.layers = 1;
//
//		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
//			throw std::runtime_error("failed to create framebuffer!");
//		}
//	}
//}
//
//
//void StagingBuffer::createCommandPool() {
//	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
//
//	VkCommandPoolCreateInfo poolInfo{};
//	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
//	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;//ֻ��¼command bufferһ�Σ�Ȼ��ÿִ֡�����ǣ�VK_COMMAND_POOL_CREATE_TRANSIENT_BIT��ÿ�ζ������µģ������ڴ���������
//	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
//
//	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
//		throw std::runtime_error("failed to create command pool!");
//	}
//}
//
//
//void StagingBuffer::createCommandBuffers() {
//	VkCommandBufferAllocateInfo allocInfo{};//cmd buf�����������Ϣ
//	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//	allocInfo.commandPool = commandPool;
//	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;//PRIMARY�ǿ��Ա����Ա��ύ��queue�����ǲ��ܱ�����cmd buf���ã�SECONDARY�����Ա��ύ��queue�����ǿ��Ա�PRIMARY���ã���ͨ�ò�������ʱ�Ƚ�����
//	allocInfo.commandBufferCount = 1;
//
//	if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
//		throw std::runtime_error("failed to allocate command buffers!");
//	}
//
//}
//
//void StagingBuffer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
//	VkCommandBufferBeginInfo beginInfo{};//����һ��cmd buf��ʽ��ʼ�Ľṹ
//	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//	beginInfo.flags = 0;//�������ָ������Ҫ��ôʹ�����cmd buf
//	beginInfo.pInheritanceInfo = nullptr;//�������secondary cmdָ����primary cmd�̳е�״̬
//
//	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {//vkBeginCommandBuffer()��������ĵ��ý����������cmd buf
//		throw std::runtime_error("failed to begin recording command buffer!");
//	}
//
//	VkRenderPassBeginInfo renderPassInfo{};//����һ��render pass��ʽ��ʼ�Ľṹ
//	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//	renderPassInfo.renderPass = renderPass;//���render pass
//	renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];//��󶨵�FBO
//	renderPassInfo.renderArea.offset = { 0, 0 };//��Ⱦ����
//	renderPassInfo.renderArea.extent = swapChainExtent;
//
//	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };//clear value
//	renderPassInfo.clearValueCount = 1;
//	renderPassInfo.pClearValues = &clearColor;
//
//	//��ʽ��ʼrender pass�����������ָ��
//	//INLINE����ִֻ��primary cmd��û��secondary cmd
//	//SECONDARY����ִ��secondary cmd
//	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
//
//	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);//GRAPHICS����ͼ����ˮ��
//
//	//��vao
//	VkBuffer vertexBuffers[] = { vertexBuffer };
//	VkDeviceSize offsets[] = { 0 };
//	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
//
//
//	vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);//����������instance��������һ�������ƫ�ƣ���һ��instance��ƫ��
//
//	vkCmdEndRenderPass(commandBuffer);//render pass����
//
//	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {//����cmd buf
//		throw std::runtime_error("failed to record command buffer!");
//	}
//}
//
////ִ��������������swap chain��һ��ͼ����FBO�󶨵Ĳ��ͼƬ��ִ��cmd buf����ͼƬ����swap chain������ʾ
////��Щ������ʵ��ȫ�����첽ִ�еģ�����Ϊ�˻����ȷ�Ľ��������Ҫ��ͬ����������fences��semaphoresʵ��
////fences��Ҫ����ͬ����Ⱦ���к�CPU֮���ͬ������������״̬����signaled��unsignaled��CPU-GPU֮��ͬ����semaphores����GPU-GPUͬ��
//void StagingBuffer::drawFrame() {
//	vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);//����CPU�ڵ�ǰλ�ñ���������Ȼ��һֱ�ȴ��������ܵ�Fence��Ϊsignaled��״̬�������Ϳ���ʵ����ĳ����Ⱦ�����ڵ�����������ɺ�CPU��ִ��ĳЩ������ͬ���龰��
//	vkResetFences(device, 1, &inFlightFence);//��һ��Fence�ָ���unsignaled��״̬
//
//	uint32_t imageIndex;
//	VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores, VK_NULL_HANDLE, &imageIndex);
//
//	if (result == VK_ERROR_OUT_OF_DATE_KHR) {//���result����swap chain��������
//		recreateSwapChain();
//		return;
//	}
//	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {//VK_SUBOPTIMAL_KHR����swap chain�����ã������Ѿ���׼ȷ�ˣ���Ϊ�˴��Ž�
//		throw std::runtime_error("failed to acquire swap chain image!");
//	}
//
//	vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);//�������������Ҳ���ԣ�recordCommandBuffer()��Ҳ���������cmdbuf
//	recordCommandBuffer(commandBuffer, imageIndex);
//
//
//	VkSubmitInfo submitInfo{};//��������queue���ύ��ͬ��
//	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//
//	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores };
//	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };//�ź����ȵ�����Ⱦ��ˮ�߽׶�
//	submitInfo.waitSemaphoreCount = 1;
//	submitInfo.pWaitSemaphores = waitSemaphores;//ָ��ִ��ǰ�ĵȴ�
//	submitInfo.pWaitDstStageMask = waitStages;
//
//	submitInfo.commandBufferCount = 1;
//	submitInfo.pCommandBuffers = &commandBuffer;
//
//	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores };
//	submitInfo.signalSemaphoreCount = 1;
//	submitInfo.pSignalSemaphores = signalSemaphores;//ָ��ִ�к�ĵȴ�
//
//	//��cmd buf�ύ��graphics queue������һ��Fence��������Queue�е��������������Ժ�Fence�ͻᱻ���ó�signaled��״̬
//	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
//		throw std::runtime_error("failed to submit draw command buffer!");
//	}
//
//	VkPresentInfoKHR presentInfo{};
//	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//
//	presentInfo.waitSemaphoreCount = 1;
//	presentInfo.pWaitSemaphores = signalSemaphores;
//
//	VkSwapchainKHR swapChains[] = { swapChain };
//	presentInfo.swapchainCount = 1;
//	presentInfo.pSwapchains = swapChains;
//
//	presentInfo.pImageIndices = &imageIndex;
//	presentInfo.pResults = nullptr;//����ָ��N��VkResult�������swap chain�Ľ��
//
//	result = vkQueuePresentKHR(presentQueue, &presentInfo);//��swap chain�ύһ����ʾ����
//
//	//��vkQueuePresentKHR()֮���ж�framebufferResizedԭ����ȷ�����򲻻ᱻsemaphore��ס��Ϊ��ѭ��
//
//	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
//		framebufferResized = false;
//		recreateSwapChain();
//	}
//	else if (result != VK_SUCCESS) {
//		throw std::runtime_error("failed to present swap chain image!");
//	}
//}
//
//
//void StagingBuffer::createSyncObjects() {
//	VkSemaphoreCreateInfo semaphoreInfo{};
//	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
//
//	VkFenceCreateInfo fenceInfo{};
//	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
//
//	if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores) != VK_SUCCESS ||
//		vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores) != VK_SUCCESS ||
//		vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
//		throw std::runtime_error("failed to create synchronization objects for a frame!");
//	}
//}
//
//
//void StagingBuffer::recreateSwapChain() {
//	//����Ĵ�����Ϊ�˴����ڵ���С��
//	int width = 0, height = 0;
//	glfwGetFramebufferSize(window, &width, &height);
//	while (width == 0 || height == 0) {
//		glfwGetFramebufferSize(window, &width, &height);
//		glfwWaitEvents();//������ڱ���С����һֱ��������
//	}
//
//	vkDeviceWaitIdle(device);//��ֹ����Ⱦ���е�ʱ���ؽ�swap chain
//
//	cleanupSwapChain();
//
//	createSwapChain();
//	createImageViews();
//	createRenderPass();
//	createGraphicsPipeline();
//	createFramebuffers();
//}
//
//void StagingBuffer::cleanupSwapChain() {
//	for (auto framebuffer : swapChainFramebuffers) {
//		vkDestroyFramebuffer(device, framebuffer, nullptr);
//	}
//
//	vkDestroyPipeline(device, graphicsPipeline, nullptr);
//	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
//	vkDestroyRenderPass(device, renderPass, nullptr);
//
//	for (auto imageView : swapChainImageViews) {
//		vkDestroyImageView(device, imageView, nullptr);
//	}
//
//	vkDestroySwapchainKHR(device, swapChain, nullptr);
//}
//
//
//void StagingBuffer::createVertexBuffer() {
//	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
//
//	//staging buffer��һ����תվ���Ȱ�buffer��CPU��ߴ��������ã����洫��gpu��
//	VkBuffer stagingBuffer;
//	VkDeviceMemory stagingBufferMemory;
//	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
//
//	void* data;
//	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
//	memcpy(data, vertices.data(), (size_t)bufferSize);
//	vkUnmapMemory(device, stagingBufferMemory);
//
//	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
//		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, //�ڴ�������д�ٶȿ�
//		vertexBuffer,
//		vertexBufferMemory);
//
//	copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
//
//	vkDestroyBuffer(device, stagingBuffer, nullptr);
//	vkFreeMemory(device, stagingBufferMemory, nullptr);
//}
//
//
//void StagingBuffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
//	VkBufferCreateInfo bufferInfo{};
//	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//	bufferInfo.size = size;
//	bufferInfo.usage = usage;
//	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//
//	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
//		throw std::runtime_error("failed to create buffer!");
//	}
//
//	VkMemoryRequirements memRequirements;
//	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
//
//	VkMemoryAllocateInfo allocInfo{};
//	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//	allocInfo.allocationSize = memRequirements.size;
//	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
//
//	//����ʵ��Ӧ�ó����У����ǻ�һ��������һ����ڴ�
//	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
//		throw std::runtime_error("failed to allocate buffer memory!");
//	}
//
//	vkBindBufferMemory(device, buffer, bufferMemory, 0);
//}
//
//
//void StagingBuffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
//	VkCommandBufferAllocateInfo allocInfo{};
//	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//	allocInfo.commandPool = commandPool;//todo:������Դ���һ����ʱ��command pool�����������VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
//	allocInfo.commandBufferCount = 1;
//
//	VkCommandBuffer temp_cmdbuf;
//	vkAllocateCommandBuffers(device, &allocInfo, &temp_cmdbuf);
//
//	VkCommandBufferBeginInfo beginInfo{};
//	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;//ֻ�������cmdһ�Σ����ҵ������������ִ����
//
//	vkBeginCommandBuffer(temp_cmdbuf, &beginInfo);
//
//	VkBufferCopy copyRegion{};
//	copyRegion.srcOffset = 0; // Optional
//	copyRegion.dstOffset = 0; // Optional
//	copyRegion.size = size;
//	vkCmdCopyBuffer(temp_cmdbuf, srcBuffer, dstBuffer, 1, &copyRegion);
//
//	vkEndCommandBuffer(temp_cmdbuf);
//
//	VkSubmitInfo submitInfo{};
//	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//	submitInfo.commandBufferCount = 1;
//	submitInfo.pCommandBuffers = &temp_cmdbuf;
//
//	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);//��������ø�fence��������bufferͬʱ�������ҵȴ�����ȫ��������ɣ��Ż�����
//	vkQueueWaitIdle(graphicsQueue);
//
//	vkFreeCommandBuffers(device, commandPool, 1, &temp_cmdbuf);
//
//}
//
//
//uint32_t StagingBuffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
//	//����������һ������ͬ���ڴ������Լ���Ӧ���ڴ�ѣ���vram��ram�У�
//	VkPhysicalDeviceMemoryProperties memProperties;
//	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
//
//	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
//		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
//			return i;
//		}
//	}
//
//	throw std::runtime_error("failed to find suitable memory type!");
//}
//
//bool StagingBuffer::isDeviceSuitable(VkPhysicalDevice device) {
//	QueueFamilyIndices indices = findQueueFamilies(device);
//
//	bool extensionsSupported = checkDeviceExtensionSupport(device);//�ж�physical device�Ƿ�֧�����������extension
//
//	bool swapChainAdequate = false;//�ж�swap chain�Ƿ��������ǵ�����
//	if (extensionsSupported) {
//		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
//		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
//	}
//
//
//	return indices.isComplete() && extensionsSupported && swapChainAdequate;
//}
//
//QueueFamilyIndices StagingBuffer::findQueueFamilies(VkPhysicalDevice device) {
//	QueueFamilyIndices indices;
//
//	uint32_t queueFamilyCount = 0;
//	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
//
//	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
//	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
//
//	int i = 0;
//	for (const auto& queueFamily : queueFamilies) {
//		//ѡ��֧��ͼ��Graphics��Queue family��֧��GRAPHICS��COMPUTE��queue familyҲ��ʽ֧��VK_QUEUE_TRANSFER_BIT
//		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
//			indices.graphicsFamily = i;
//		}
//
//		VkBool32 presentSupport = false;
//		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
//
//		if (presentSupport) {
//			indices.presentFamily = i;
//		}
//
//		if (indices.isComplete()) {
//			break;
//		}
//
//		i++;
//	}
//
//	return indices;
//}
//
//void StagingBuffer::mainLoop() {
//	while (!glfwWindowShouldClose(window)) {
//		glfwPollEvents();
//		drawFrame();
//	}
//
//	//�������첽ִ�У����ͷ���Դʱ��������Ⱦ�����ڽ���
//	vkDeviceWaitIdle(device);
//}
//
//void StagingBuffer::cleanup() {
//	vkDestroySemaphore(device, renderFinishedSemaphores, nullptr);
//	vkDestroySemaphore(device, imageAvailableSemaphores, nullptr);
//	vkDestroyFence(device, inFlightFence, nullptr);
//
//	vkDestroyCommandPool(device, commandPool, nullptr);
//
//	cleanupSwapChain();
//	vkDestroyBuffer(device, vertexBuffer, nullptr);
//	vkFreeMemory(device, vertexBufferMemory, nullptr);
//
//
//	vkDestroyDevice(device, nullptr);
//
//	if (enableValidationLayers) {
//		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
//	}
//
//	vkDestroySurfaceKHR(instance, surface, nullptr);
//	vkDestroyInstance(instance, nullptr);
//	glfwDestroyWindow(window);
//	glfwTerminate();
//}
//
////���������Ŀ���Ǽ������Ҫʹ�õ�validation layer��vulkan�Ƿ�֧��
//bool StagingBuffer::checkValidationLayerSupport() {
//	uint32_t layerCount;
//	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
//
//	std::vector<VkLayerProperties> availableLayers(layerCount);
//	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
//
//	for (const char* layerName : validationLayers) {
//		bool layerFound = false;
//
//		for (const auto& layerProperties : availableLayers) {
//			if (strcmp(layerName, layerProperties.layerName) == 0) {
//				layerFound = true;
//				break;
//			}
//		}
//
//		if (!layerFound) {
//			return false;
//		}
//	}
//	return true;
//}
//
////�õ�Vulkan���������������չ
//std::vector<const char*> StagingBuffer::getRequiredExtensions() {
//	uint32_t glfwExtensionCount = 0;
//	const char** glfwExtensions;
//	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
//
//	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
//
//	//��glfw��extension����vulkan����extension
//	if (enableValidationLayers) {
//		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
//	}
//
//	//��glfw֧�ֵ�extension��ӡ����
//	std::cout << "glfw extensions:\n";
//	for (int i = 0; i < extensions.size(); i++)
//	{
//		std::cout << '\t' << (extensions[i]) << '\n';
//	}
//
//	return extensions;
//}
//
//void StagingBuffer::createInstance() {
//	if (enableValidationLayers && !checkValidationLayerSupport()) {
//		throw std::runtime_error("validation layers requested, but not available!");
//	}
//
//	//����ṹ���ǿ�ѡ�ģ�����еĻ����Ը��������ṩһЩ�Ż�
//	VkApplicationInfo appInfo{};
//	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;//������ṹ�������
//	appInfo.pApplicationName = "Hello Triangle";
//	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
//	appInfo.pEngineName = "No Engine";
//	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
//	appInfo.apiVersion = VK_API_VERSION_1_0;
//
//	//����ṹ��ͱ������ˣ�����������������Ҫʹ��ʲôglobal extensions��validation layers
//	VkInstanceCreateInfo createInfo{};
//	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
//	createInfo.pApplicationInfo = &appInfo;
//
//	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
//
//	//ʹ��glfw��Vulkan�ṩһ������ϵͳ�Ľӿ�
//	auto extensions = getRequiredExtensions();
//	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
//	createInfo.ppEnabledExtensionNames = extensions.data();//����������Щ֧�ֵ�extension
//
//	if (enableValidationLayers) {
//		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
//		createInfo.ppEnabledLayerNames = validationLayers.data();
//
//		populateDebugMessengerCreateInfo(debugCreateInfo);
//		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
//	}
//	else {
//		createInfo.enabledLayerCount = 0;
//	}
//
//
//	//����һ��vulkan instance
//	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
//	if (result != VK_SUCCESS)
//	{
//		throw std::runtime_error("failed to create instance!");
//	}
//
//	//�õ�vulkan֧�ֵ�����extension
//	uint32_t extensionCount = 0;
//	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
//	std::vector<VkExtensionProperties> extensions_vk(extensionCount);
//	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions_vk.data());
//	std::cout << "vulkan available extensions:\n";
//	for (const auto& extension_vk : extensions_vk) {
//		std::cout << '\t' << extension_vk.extensionName << '\n';
//	}
//}
//
//VkBool32 StagingBuffer::debugCallback(
//	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
//	VkDebugUtilsMessageTypeFlagsEXT messageType,
//	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
//	void* pUserData)
//{
//	// Message is important enough to show���ж���Ϣ��Ҫ��
//	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
//		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
//	}
//	return VK_FALSE;//����һ�㶼�Ƿ���false
//}
//
//void StagingBuffer::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
//	auto app = reinterpret_cast<StagingBuffer*>(glfwGetWindowUserPointer(window));//ͨ��window��ȡ�Ѿ��󶨺õı���ʵ��
//	app->framebufferResized = true;
//}
//
//void StagingBuffer::setupDebugMessenger() {
//	if (!enableValidationLayers) return;
//
//	VkDebugUtilsMessengerCreateInfoEXT createInfo;
//	populateDebugMessengerCreateInfo(createInfo);
//
//	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
//		throw std::runtime_error("failed to set up debug messenger!");
//	}
//}
//
//void StagingBuffer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
//	createInfo = {};
//	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
//	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
//	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
//	createInfo.pfnUserCallback = debugCallback;
//	createInfo.pUserData = nullptr; // Optional
//}
//
//std::vector<char> StagingBuffer::readFile(const std::string& filename) {
//	std::ifstream file(filename, std::ios::ate | std::ios::binary);
//
//	if (!file.is_open()) {
//		throw std::runtime_error("failed to open file!");
//	}
//
//	size_t fileSize = (size_t)file.tellg();
//	std::vector<char> buffer(fileSize);
//
//	file.seekg(0);
//	file.read(buffer.data(), fileSize);
//
//	file.close();
//
//	return buffer;
//}