#include "HelloTriangleApplication.h"
#include "SwapChainRecreation.h"
#include "VertexBuffer.h"
#include "StagingBuffer.h"
#include "IndexBuffer.h"

int main() {
	//HelloTriangleApplication app;
	//SwapChainRecreation app;
	//VertexBuffer app;
	//StagingBuffer app;
	IndexBuffer app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;

}
