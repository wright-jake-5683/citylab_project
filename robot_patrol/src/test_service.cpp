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
            reentrant_group_ = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);

            rclcpp::SubscriptionOptions sub_options;
            sub_options.callback_group = reentrant_group_;

            laser_subscriber_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
                "/fastbot_1/scan", qos,
                std::bind(&TestService::laserscan_callback, this,
                        std::placeholders::_1),
                        sub_options
            );
            
            timer_ = this->create_wall_timer(
                1s,
                std::bind(&TestService::send_test_request, this),
                reentrant_group_
            );

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
            laser_data_ = *msg;
        }

        void send_test_request()
        {
            auto request = std::make_shared<custom_interfaces::srv::GetDirection::Request>();
            request->laser_data = laser_data_;

            auto future = test_service_client_->async_send_request(request);
            RCLCPP_INFO(this->get_logger(), "/direction_service has been requested from client...");

            // Block until the response arrives (safe with Reentrant + MultiThreadedExecutor)
            if (future.wait_for(5s) == std::future_status::ready) {
                auto response = future.get();
                if (response) {
                    RCLCPP_INFO(this->get_logger(), "/direction_service has responded to client \n Response: %s", response->direction.c_str());
                } 
                else 
                {
                    RCLCPP_ERROR(this->get_logger(), "Null response from /direction_service");
                }
            } 
            else 
            {
                RCLCPP_ERROR(this->get_logger(), "/direction_service timed out");
            }
        }

    std::string node_name_;
    rclcpp::Client<custom_interfaces::srv::GetDirection>::SharedPtr test_service_client_;
    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr laser_subscriber_;
    sensor_msgs::msg::LaserScan laser_data_;
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::CallbackGroup::SharedPtr reentrant_group_;
    
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