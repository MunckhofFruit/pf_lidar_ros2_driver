#include <string>
#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>

#include "pf_driver/pf/handle_info.h"
#include "pf_driver/pf/pfsdp_base.h"
#include "pf_driver/pf/scan_config.h"
#include "pf_driver/pf/scan_parameters.h"

namespace
{
using boost::asio::ip::tcp;

class HttpTestServer
{
public:
  explicit HttpTestServer(std::vector<std::string> responses) : responses_(std::move(responses))
  {
    acceptor_.open(tcp::v4());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(tcp::endpoint(tcp::v4(), 0));
    acceptor_.listen();
    port_ = acceptor_.local_endpoint().port();
    thread_ = std::thread([this] { serve(); });
  }

  ~HttpTestServer()
  {
    if (thread_.joinable())
    {
      thread_.join();
    }
  }

  unsigned short port() const
  {
    return port_;
  }

private:
  void serve()
  {
    for (const auto& response : responses_)
    {
      tcp::socket socket(io_context_);
      acceptor_.accept(socket);

      boost::asio::streambuf request;
      boost::asio::read_until(socket, request, "\r\n\r\n");

      const std::string http_response =
          "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " +
          std::to_string(response.size()) + "\r\nConnection: close\r\n\r\n" + response;
      boost::asio::write(socket, boost::asio::buffer(http_response));
    }
  }

  boost::asio::io_context io_context_;
  tcp::acceptor acceptor_ {io_context_};
  std::vector<std::string> responses_;
  std::thread thread_;
  unsigned short port_ {0};
};

std::shared_ptr<PFSDPBase> make_protocol_interface(unsigned short port, std::shared_ptr<ScanParameters> params)
{
  auto node = std::make_shared<rclcpp::Node>("pfsdp_scan_parameters_test");
  auto info = std::make_shared<HandleInfo>();
  info->hostname = "127.0.0.1:" + std::to_string(port);
  auto config = std::make_shared<ScanConfig>();
  return std::make_shared<PFSDPBase>(node, info, config, params);
}

const std::string common_parameters_response =
    R"({"error_code":0,"error_text":"success","radial_range_min":"1.5","radial_range_max":"30.0","sampling_rate_max":"252000"})";
}

TEST(PFSDPScanParametersTest, ReadsLayerCountAndInclination)
{
  rclcpp::init(0, nullptr);
  HttpTestServer server({common_parameters_response,
                         R"({"error_code":0,"error_text":"success","layer_count":4})",
                         R"({"error_code":0,"error_text":"success","layer_inclination":[1,2,3,4]})"});
  auto params = std::make_shared<ScanParameters>();
  auto protocol = make_protocol_interface(server.port(), params);

  protocol->get_scan_parameters();

  EXPECT_FLOAT_EQ(params->radial_range_min, 1.5F);
  EXPECT_FLOAT_EQ(params->radial_range_max, 30.0F);
  EXPECT_EQ(params->sampling_rate_max, 252000);
  EXPECT_TRUE(params->layer_count_received);
  EXPECT_EQ(params->layer_count, 4);
  EXPECT_TRUE(params->inclination_count_received);
  EXPECT_EQ(params->inclination_count, 4);

  protocol.reset();
  rclcpp::shutdown();
}

TEST(PFSDPScanParametersTest, LeavesLayerCountUnavailableWhenMissing)
{
  rclcpp::init(0, nullptr);
  HttpTestServer server({common_parameters_response,
                         R"({"error_code":-5,"error_text":"parameter not found"})"});
  auto params = std::make_shared<ScanParameters>();
  params->layer_count = 99;
  params->inclination_count = 88;
  auto protocol = make_protocol_interface(server.port(), params);

  protocol->get_scan_parameters();

  EXPECT_FALSE(params->layer_count_received);
  EXPECT_EQ(params->layer_count, 99);
  EXPECT_FALSE(params->inclination_count_received);
  EXPECT_EQ(params->inclination_count, 88);

  protocol.reset();
  rclcpp::shutdown();
}