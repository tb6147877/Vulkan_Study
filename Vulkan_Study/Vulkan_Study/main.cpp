#include "HelloTriangleApplication.h"
#include "SwapChainRecreation.h"
#include "VertexBuffer.h"
#include "StagingBuffer.h"
#include "IndexBuffer.h"
#include "Descriptor.h"
#include "Vulkan_Image.h"
#include "DepthBuffer.h"
#include "LoadModel.h"
#include "Mipmap.h"
#include "MSAA.h"

int main() {
	//HelloTriangleApplication app;
	//SwapChainRecreation app;
	//VertexBuffer app;
	//StagingBuffer app;
	//IndexBuffer app;
	//Descriptor app;
	//Vulkan_Image app;
	//DepthBuffer app;
	//LoadModel app;
	//Mipmap app;
	MSAA app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;

}



