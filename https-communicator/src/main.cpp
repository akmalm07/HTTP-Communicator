#include "headers.h"
#include "http_communicator.h"

//#include <gtest/gtest.h>




int main(int argc, char **argv)
{
	//::testing::InitGoogleTest(&argc, argv);

	auto res = communicator::post("http://localhost:8080/test", communicator::HTTPContent::ApplicationJSON, R"({"key": "value"})", { {"Custom-Header", "Value"} });

	if (res.has_value())
	{
		std::cout << "Response: " << std::get<std::string>(res->body) << std::endl;
	}
	else
	{
		std::cerr << "Error: " << static_cast<int>(res.error()) << std::endl;
	}

	auto res1 = communicator::get("http://localhost:8080", { {"Custom-Header", "Value"} });

	if (res1.has_value())
	{
		std::cout << "response1: " << std::get<std::string>(res1->body) << std::endl;
	}
	else
	{
		std::cerr << "Error1: " << static_cast<int>(res1.error()) << std::endl;
	}

}