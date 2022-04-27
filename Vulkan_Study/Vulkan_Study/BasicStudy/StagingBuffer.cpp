//#include "StagingBuffer.h"
//
////这是一个代理函数，用来加载并执行vkCreateDebugUtilsMessengerEXT()
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
////这是一个代理函数，用来加载并执行vkDestroyDebugUtilsMessengerEXT()
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
////使用glfw来初始化窗口
//void StagingBuffer::initWindow() {
//	glfwInit();
//
//	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//
//	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
//
//	glfwSetWindowUserPointer(window, this);//将本类的实体与window绑定
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
//	//开启logical device
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
//	else {//这种情况是一些window manager允许我们设置自己的分辨率
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
//	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;//推荐给swap chain中的图片数量+1来防止显卡驱动等待
//
//	//防止超过swap chain中最大图片数量
//	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
//		imageCount = swapChainSupport.capabilities.maxImageCount;
//	}
//	std::cout << "\nswap chain image number:" << imageCount << "\n";
//
//	//创建swap chain的结构
//	VkSwapchainCreateInfoKHR createInfo{};
//	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
//	createInfo.surface = surface;
//
//	createInfo.minImageCount = imageCount;
//	createInfo.imageFormat = surfaceFormat.format;
//	createInfo.imageColorSpace = surfaceFormat.colorSpace;
//	createInfo.imageExtent = extent;
//	createInfo.imageArrayLayers = 1;//一般都是1，除非开发左右眼不同的3D游戏
//	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;//图片直接渲染输出到屏幕，如果是从后处理那边来的就用VK_IMAGE_USAGE_TRANSFER_DST_BIT
//
//	//这里我们需要指定swap chain怎样被多个family queue使用
//	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
//	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
//
//	if (indices.graphicsFamily != indices.presentFamily) {//如果graphics queue family不同于presentation queue family
//		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;//并存模式，图片可以被多个queue family共用，不需要显式的转换所有权
//		createInfo.queueFamilyIndexCount = 2;//被几个queue family共有
//		createInfo.pQueueFamilyIndices = queueFamilyIndices;//被哪些queue family共有
//	}
//	else {
//		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;//专有模式，一张图片在一个时刻只能被一个queue family拥有，除非显式的转移所有权，性能最好
//	}
//
//	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;//应用到swap chain中图片的变换，比如说水平翻转或旋转90度，这里选择无变换
//	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;//在和其他窗口混合时Alpha通道是否发挥作用，这里选否
//	createInfo.presentMode = presentMode;
//	createInfo.clipped = VK_TRUE;//true代表不关注隐藏在别的图片后面的像素
//
//	createInfo.oldSwapchain = VK_NULL_HANDLE;//关于多个swap chain切换的情况
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
//		//创建VkImageView的结构体
//		VkImageViewCreateInfo createInfo{};
//		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//		createInfo.image = swapChainImages[i];
//		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
//		createInfo.format = swapChainImageFormat;
//		//图片的每个通道是否混合在一起
//		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
//		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
//		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
//		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
//		//这张图片的用途
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
//	//创建流水线中对应shader
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
//	//pSpecializationInfo这个字段很有必要讨论，它可以指定shader中的常量，比如一个shader在不同时候常量值不同，那我们在创建流水线时候指定好，这样比渲染时候传变量值进去性能更好
//
//	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
//
//	//用于描述传入顶点shader的数据格式，概念类似VBO
//	//binding:数据中间的空隙以及数据是逐顶点的还是逐instance的
//	//Attribute descriptions:Attribute的格式以及offset
//	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
//	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//	auto bindingDescription = Vertex::getBindingDescription();
//	auto attributeDescriptions = Vertex::getAttributeDescriptions();
//	vertexInputInfo.vertexBindingDescriptionCount = 1;
//	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
//	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
//	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
//
//	//用于描述primitive的类型以及是否启用图元重启（0xFFFF or 0xFFFFFFFF）
//	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
//	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
//	inputAssembly.primitiveRestartEnable = VK_FALSE;
//
//	//描述渲染要输出到的FBO的区域
//	VkViewport viewport{};
//	viewport.x = 0.0f;
//	viewport.y = 0.0f;
//	viewport.width = (float)swapChainExtent.width;//渲染时动态可变
//	viewport.height = (float)swapChainExtent.height;
//	viewport.minDepth = 0.0f;//depth的范围，必须在0.0f和1.0f之间，minDepth可以比maxDepth大，这样就形成了左右手坐标系的切换
//	viewport.maxDepth = 1.0f;
//
//	//剪刀测试
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
//	//光栅化阶段执行深度测试，面剔除，剪刀测试以及输出是全部输出还是线框输出
//	VkPipelineRasterizationStateCreateInfo rasterizer{};
//	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//	rasterizer.depthClampEnable = VK_FALSE;//如果是true，那在近截面和远截面之外的fragment会被clamp而不是discard，这种情况在处理阴影时可能有用
//	rasterizer.rasterizerDiscardEnable = VK_FALSE;//如果是true就会禁止任何输出到FBO
//	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;//填充显示，线框显示，点显示
//	rasterizer.lineWidth = 1.0f;//线的粗细，渲染时动态可变
//	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;//剔除哪面
//	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;//哪面是正面
//	rasterizer.depthBiasEnable = VK_FALSE;//是否启动depth bias，在阴影那里启用
//
//	//这里描述多采样抗锯齿相关信息
//	VkPipelineMultisampleStateCreateInfo multisampling{};
//	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//	multisampling.sampleShadingEnable = VK_FALSE;
//	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
//
//	//颜色混合，有两种方式，一种是混合旧值和新值得出一个最终颜色；另一种是位运算来合并旧值和新值
//	//这个结构体描述的是逐FBO的blend配置
//	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
//	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
//	colorBlendAttachment.blendEnable = VK_FALSE;
//	//alpha blend的配置
//	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
//	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
//	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
//	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
//	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
//	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
//
//	//这个结构体描述的是所有FBO的blend
//	VkPipelineColorBlendStateCreateInfo colorBlending{};
//	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//	colorBlending.logicOpEnable = VK_FALSE;
//	colorBlending.logicOp = VK_LOGIC_OP_COPY;//这里可以设置成位操作
//	colorBlending.attachmentCount = 1;
//	colorBlending.pAttachments = &colorBlendAttachment;
//	colorBlending.blendConstants[0] = 0.0f;//渲染时动态可变
//	colorBlending.blendConstants[1] = 0.0f;
//	colorBlending.blendConstants[2] = 0.0f;
//	colorBlending.blendConstants[3] = 0.0f;
//
//	//与渲染时传参相关
//	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
//	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//	pipelineLayoutInfo.setLayoutCount = 0;
//	pipelineLayoutInfo.pushConstantRangeCount = 0;
//
//	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
//		throw std::runtime_error("failed to create pipeline layout!");
//	}
//
//	//创建pipeline
//	VkGraphicsPipelineCreateInfo pipelineInfo{};
//	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//	pipelineInfo.stageCount = 2;//vs和fs两个stage
//	pipelineInfo.pStages = shaderStages;
//	pipelineInfo.pVertexInputState = &vertexInputInfo;
//	pipelineInfo.pInputAssemblyState = &inputAssembly;
//	pipelineInfo.pViewportState = &viewportState;
//	pipelineInfo.pRasterizationState = &rasterizer;
//	pipelineInfo.pMultisampleState = &multisampling;
//	pipelineInfo.pColorBlendState = &colorBlending;
//	pipelineInfo.layout = pipelineLayout;
//	pipelineInfo.renderPass = renderPass;
//	pipelineInfo.subpass = 0;//subpass的index
//
//	//这两个字段用于从一个现有的pipeline中快速创建一个新pipeline，用于改动不大的情况
//	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
//	pipelineInfo.basePipelineIndex = -1;
//
//	//这个函数可以一次性创建多个pipeline
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
//	colorAttachment.format = swapChainImageFormat;//这个必须和swap chain一致
//	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;//插槽中的数据在渲染前怎么处理，应用于颜色和depth插槽
//	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;//插槽中的数据在渲染后怎么处理，应用于颜色和depth插槽
//	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;//插槽中的数据在渲染前怎么处理，应用于stencil插槽
//	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;//插槽中的数据在渲染后怎么处理，应用于stencil插槽
//	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;//表示这个图在渲染前从哪来，这里反正我们都要清掉数据，所以不在乎从哪里来
//	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;//表示这个图渲染后要去哪，这里是要去swap chain
//
//	//每个subpass都引用着一个或多个插槽
//	VkAttachmentReference colorAttachmentRef{};
//	colorAttachmentRef.attachment = 0;//在插槽数组中索引是0
//	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;//插槽的用途是颜色
//
//	//用于描述subpass，这里我们只有一个subpass
//	VkSubpassDescription subpass{};
//	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;//表明这是一个图形subpass
//	subpass.colorAttachmentCount = 1;
//	subpass.pColorAttachments = &colorAttachmentRef;//fragment shader中layout(location = 0) out vec4 outColor
//
//	//subpass用于处理render pass中fbo的插槽图片的传输问题
//	VkSubpassDependency dependency{};
//	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;//出现在src指在render pass前的一个隐式subpass
//	dependency.dstSubpass = 0;//dstSubpass要比srcSubpass大，以防出现循环引用，当然其中一个如果是VK_SUBPASS_EXTERNAL就没事；出现在dst指在render pass后的一个隐式subpass
//	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;//等到swap chain读取完再进入subpass fbo传输
//	dependency.srcAccessMask = 0;
//	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;//等到swap chain写完再进入subpass fbo传输
//	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//
//
//	//创建render pass
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
//	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;//只记录command buffer一次，然后每帧执行它们；VK_COMMAND_POOL_CREATE_TRANSIENT_BIT是每次都创建新的（包含内存分配操作）
//	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
//
//	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
//		throw std::runtime_error("failed to create command pool!");
//	}
//}
//
//
//void StagingBuffer::createCommandBuffers() {
//	VkCommandBufferAllocateInfo allocInfo{};//cmd buf创建的相关信息
//	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//	allocInfo.commandPool = commandPool;
//	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;//PRIMARY是可以被可以被提交到queue，但是不能被其他cmd buf调用；SECONDARY不可以被提交到queue，但是可以被PRIMARY调用，在通用操作复用时比较有用
//	allocInfo.commandBufferCount = 1;
//
//	if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
//		throw std::runtime_error("failed to allocate command buffers!");
//	}
//
//}
//
//void StagingBuffer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
//	VkCommandBufferBeginInfo beginInfo{};//用于一个cmd buf正式开始的结构
//	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//	beginInfo.flags = 0;//这个用于指定我们要怎么使用这个cmd buf
//	beginInfo.pInheritanceInfo = nullptr;//这个用于secondary cmd指定从primary cmd继承的状态
//
//	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {//vkBeginCommandBuffer()这个函数的调用将会重置这个cmd buf
//		throw std::runtime_error("failed to begin recording command buffer!");
//	}
//
//	VkRenderPassBeginInfo renderPassInfo{};//用于一个render pass正式开始的结构
//	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//	renderPassInfo.renderPass = renderPass;//这个render pass
//	renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];//其绑定的FBO
//	renderPassInfo.renderArea.offset = { 0, 0 };//渲染区域
//	renderPassInfo.renderArea.extent = swapChainExtent;
//
//	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };//clear value
//	renderPassInfo.clearValueCount = 1;
//	renderPassInfo.pClearValues = &clearColor;
//
//	//正式开始render pass，往里面添加指令
//	//INLINE代表只执行primary cmd，没有secondary cmd
//	//SECONDARY代表执行secondary cmd
//	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
//
//	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);//GRAPHICS代表图形流水线
//
//	//绑定vao
//	VkBuffer vertexBuffers[] = { vertexBuffer };
//	VkDeviceSize offsets[] = { 0 };
//	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
//
//
//	vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);//顶点数量，instance数量，第一个顶点的偏移，第一个instance的偏移
//
//	vkCmdEndRenderPass(commandBuffer);//render pass结束
//
//	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {//创建cmd buf
//		throw std::runtime_error("failed to record command buffer!");
//	}
//}
//
////执行三个操作：从swap chain拿一张图；在FBO绑定的插槽图片上执行cmd buf；将图片返回swap chain用于显示
////这些操作事实上全部是异步执行的，但是为了获得正确的结果它们需要被同步，可以用fences和semaphores实现
////fences主要用于同步渲染队列和CPU之间的同步，它有两种状态——signaled和unsignaled，CPU-GPU之间同步；semaphores用于GPU-GPU同步
//void StagingBuffer::drawFrame() {
//	vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);//会让CPU在当前位置被阻塞掉，然后一直等待到它接受的Fence变为signaled的状态，这样就可以实现在某个渲染队列内的所有任务被完成后，CPU再执行某些操作的同步情景。
//	vkResetFences(device, 1, &inFlightFence);//让一个Fence恢复成unsignaled的状态
//
//	uint32_t imageIndex;
//	VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores, VK_NULL_HANDLE, &imageIndex);
//
//	if (result == VK_ERROR_OUT_OF_DATE_KHR) {//这个result代表swap chain不能用了
//		recreateSwapChain();
//		return;
//	}
//	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {//VK_SUBOPTIMAL_KHR代表swap chain还能用，但是已经不准确了，成为了次优解
//		throw std::runtime_error("failed to acquire swap chain image!");
//	}
//
//	vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);//这个函数不调用也可以，recordCommandBuffer()中也会重置这个cmdbuf
//	recordCommandBuffer(commandBuffer, imageIndex);
//
//
//	VkSubmitInfo submitInfo{};//用于描述queue的提交和同步
//	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//
//	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores };
//	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };//信号量等到的渲染流水线阶段
//	submitInfo.waitSemaphoreCount = 1;
//	submitInfo.pWaitSemaphores = waitSemaphores;//指定执行前的等待
//	submitInfo.pWaitDstStageMask = waitStages;
//
//	submitInfo.commandBufferCount = 1;
//	submitInfo.pCommandBuffers = &commandBuffer;
//
//	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores };
//	submitInfo.signalSemaphoreCount = 1;
//	submitInfo.pSignalSemaphores = signalSemaphores;//指定执行后的等待
//
//	//将cmd buf提交到graphics queue，传入一个Fence，这样当Queue中的所有命令都被完成以后，Fence就会被设置成signaled的状态
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
//	presentInfo.pResults = nullptr;//可以指定N个VkResult来检查多个swap chain的结果
//
//	result = vkQueuePresentKHR(presentQueue, &presentInfo);//向swap chain提交一个显示请求
//
//	//在vkQueuePresentKHR()之后判断framebufferResized原因是确保程序不会被semaphore卡住成为死循环
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
//	//这里的代码是为了处理窗口的最小化
//	int width = 0, height = 0;
//	glfwGetFramebufferSize(window, &width, &height);
//	while (width == 0 || height == 0) {
//		glfwGetFramebufferSize(window, &width, &height);
//		glfwWaitEvents();//如果窗口被最小化就一直等在这里
//	}
//
//	vkDeviceWaitIdle(device);//防止在渲染进行的时候重建swap chain
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
//	//staging buffer是一个中转站，先把buffer在CPU这边创建并填充好，后面传到gpu端
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
//		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, //内存放这里读写速度快
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
//	//在真实的应用程序中，我们会一次性请求一大块内存
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
//	allocInfo.commandPool = commandPool;//todo:这里可以创建一个临时的command pool，可以用这个VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
//	allocInfo.commandBufferCount = 1;
//
//	VkCommandBuffer temp_cmdbuf;
//	vkAllocateCommandBuffers(device, &allocInfo, &temp_cmdbuf);
//
//	VkCommandBufferBeginInfo beginInfo{};
//	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;//只是用这个cmd一次，并且等这个拷贝操作执行完
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
//	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);//这里可以用个fence来处理多个buffer同时拷贝并且等待它们全部拷贝完成，优化性能
//	vkQueueWaitIdle(graphicsQueue);
//
//	vkFreeCommandBuffers(device, commandPool, 1, &temp_cmdbuf);
//
//}
//
//
//uint32_t StagingBuffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
//	//这里面寸的是一个个不同的内存类型以及对应的内存堆（在vram或ram中）
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
//	bool extensionsSupported = checkDeviceExtensionSupport(device);//判断physical device是否支持我们所需的extension
//
//	bool swapChainAdequate = false;//判断swap chain是否满足我们的需求
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
//		//选出支持图形Graphics的Queue family，支持GRAPHICS和COMPUTE的queue family也隐式支持VK_QUEUE_TRANSFER_BIT
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
//	//由于是异步执行，怕释放资源时候还是有渲染任务在进行
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
////这个函数的目的是检测我们要使用的validation layer在vulkan是否支持
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
////得到Vulkan调试所需的所有扩展
//std::vector<const char*> StagingBuffer::getRequiredExtensions() {
//	uint32_t glfwExtensionCount = 0;
//	const char** glfwExtensions;
//	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
//
//	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
//
//	//给glfw的extension增加vulkan调试extension
//	if (enableValidationLayers) {
//		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
//	}
//
//	//把glfw支持的extension打印出来
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
//	//这个结构体是可选的，如果有的话可以给驱动器提供一些优化
//	VkApplicationInfo appInfo{};
//	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;//是这个结构体的类型
//	appInfo.pApplicationName = "Hello Triangle";
//	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
//	appInfo.pEngineName = "No Engine";
//	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
//	appInfo.apiVersion = VK_API_VERSION_1_0;
//
//	//这个结构体就必须有了，它告诉驱动器我们要使用什么global extensions和validation layers
//	VkInstanceCreateInfo createInfo{};
//	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
//	createInfo.pApplicationInfo = &appInfo;
//
//	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
//
//	//使用glfw给Vulkan提供一个窗口系统的接口
//	auto extensions = getRequiredExtensions();
//	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
//	createInfo.ppEnabledExtensionNames = extensions.data();//开启所有这些支持的extension
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
//	//创建一个vulkan instance
//	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
//	if (result != VK_SUCCESS)
//	{
//		throw std::runtime_error("failed to create instance!");
//	}
//
//	//得到vulkan支持的所有extension
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
//	// Message is important enough to show，判断消息重要性
//	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
//		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
//	}
//	return VK_FALSE;//这里一般都是返回false
//}
//
//void StagingBuffer::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
//	auto app = reinterpret_cast<StagingBuffer*>(glfwGetWindowUserPointer(window));//通过window获取已经绑定好的本类实体
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