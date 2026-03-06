#include "rclcpp/qos.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include <custom_interfaces/srv/get_direction.hpp>
#include <chrono>

using namespace std::chrono_literals; 

class TestService : public rclcpp::Node {
    public:
        TestService(const std::string &node_name = "test_service_node") 
            : Node(node_name), node_name_(node_name)
        {
            auto qos = rclcpp::QoS(10).reliability(rclcpp::ReliabilityPolicy::Reliable);

            laser_subscriber_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
                "/fastbot_1/scan", qos,
                std::bind(&TestService::laserscan_callback, this,
                        std::placeholders::_1));

            std::string test_service_name = "/direction_service";
            test_service_client_ = this->create_client<custom_interfaces::srv::GetDirection>(test_service_name);

            // Wait for the service to be available (checks every second)
            while (!test_service_client_->wait_for_service(1s)) {
            if (!rclcpp::ok()) {
                RCLCPP_ERROR(this->get_logger(), "Interrupted while waiting for the %s service. Exiting...", test_service_name.c_str());
                return;
            }
                RCLCPP_INFO(this->get_logger(), "%s not available, waiting again...", test_service_name.c_str());
            }

                RCLCPP_INFO(this->get_logger(), "%s client ready!", test_service_name.c_str());
        }

        void laserscan_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg) 
        {
            auto request = std::make_shared<custom_interfaces::srv::GetDirection::Request>();
            sensor_msgs::msg::LaserScan laser_data = *msg;
            request->laser_data;

            test_service_client_->async_send_request(
                request,
                [this](rclcpp::Client<custom_interfaces::srv::GetDirection>::SharedFuture future)
                {
                    RCLCPP_INFO(this->get_logger(), "/direction_service service requested...");
                    auto response = future.get();
                    RCLCPP_INFO(this->get_logger(), "safest direction found: %s", response->direction.c_str());
                }
            );
        }

    std::string node_name_;
    rclcpp::Client<custom_interfaces::srv::GetDirection>::SharedPtr test_service_client_;
    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr laser_subscriber_;
};

int main(int argc, char **argv) 
{
    rclcpp::init(argc, argv);
    auto test_service_node = std::make_shared<TestService>();
    
    rclcpp::executors::MultiThreadedExecutor executor;
    executor.add_node(test_service_node);

    executor.spin();

    rclcpp::shutdown();
    return 0;
}