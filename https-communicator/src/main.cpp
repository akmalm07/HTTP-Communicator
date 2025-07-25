#include "headers.h"
#include "http_communicator.h"

#include <gtest/gtest.h>

TEST(HTTPCommunicatorTest, Get_ValidURL_ReturnsResponse)
{
    communicator::HTTPCommunicator http({}, 10);
    auto result = http.post("http://localhost:8080/upload", communicator::HTTPContent::TextPlain, "hello form C++");

	ASSERT_EQ(result.error(), communicator::HTTPErr::None);
}


int main(int argc, char **argv)
{
   ::testing::InitGoogleTest(&argc, argv);
   

 /*  communicator::HTTPCommunicator http({}, 10);
   auto result = http.get("http://localhost:8080");
   if (result.has_value())
   {
	   std::cout << "Response: " << result.value() << std::endl;
   }
   else
   {
	   std::cerr << "Error: " << static_cast<int>(result.error()) << std::endl;
   }*/

   // THere was an errors becoase I gave the wrong info to the resolver
   RUN_ALL_TESTS();
}