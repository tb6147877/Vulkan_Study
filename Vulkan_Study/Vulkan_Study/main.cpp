#include "HelloTriangleApplication.h"
#include "SwapChainRecreation.h"
#include "VertexBuffer.h"

int main() {
	//HelloTriangleApplication app;
	//SwapChainRecreation app;
	VertexBuffer app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;

}
