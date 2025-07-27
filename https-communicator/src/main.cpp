#include "headers.h"
#include "http_communicator.h"

#include <gtest/gtest.h>




int main(int argc, char **argv)
{
   ::testing::InitGoogleTest(&argc, argv);
   

   communicator::HTTPCommunicator http("http://localhost:8080", {}, 10);

   communicator::get("http://localhost:8080");

   http.get_string("/");
}