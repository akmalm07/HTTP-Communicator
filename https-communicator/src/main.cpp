#include "headers.h"
#include "http_communicator.h"

#include <gtest/gtest.h>




int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
   

	//communicator::HTTPCommunicator http("http://localhost:8080", {}, 10);
	//auto res = http.post_string("/test", communicator::HTTPContent::ApplicationJSON, R"({"key": "value"})", { {"Custom-Header", "Value"} });
	//std::cerr << "Error: " << static_cast<int>(res) << std::endl;
	//http.get_string("/");

	auto res = communicator::post("http://localhost:8080/test", communicator::HTTPContent::ApplicationJSON, R"({"key": "value"})", { {"Custom-Header", "Value"} });

	if (res.has_value())
	{
		std::cout << "Response: " << std::get<std::string>(res->body) << std::endl;
	}
	else
	{
		std::cerr << "Error: " << static_cast<int>(res.error()) << std::endl;
	}

}