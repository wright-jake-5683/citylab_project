#include "rclcpp/executor_options.hpp"
#include "rclcpp/executors/multi_threaded_executor.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "rclcpp/qos.hpp"
#include <algorithm>
#include <chrono>
#include <ranges>

class PatrolNode : public rclcpp::Node
{
public:
    PatrolNode(const std::string &node_name = "robot_patrol_node") :
        Node(node_name), node_name_(node_name)
    {
        auto qos = rclcpp::QoS(10).reliability(rclcpp::ReliabilityPolicy::Reliable);

        laser_subscriber_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
        "/scan", 
        qos, 
        std::bind(&PatrolNode::laserscan_callback, this, std::placeholders::_1)
        );

        cmd_publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);

        auto timer_period = std::chrono::milliseconds(100); //Equivalent to 10Hz
        timer_ = this->create_wall_timer(timer_period, std::bind(&PatrolNode::timer_control_callback, this));

        RCLCPP_INFO(this->get_logger(), "%s Ready...", node_name_.c_str());
    }

    void stop_fastbot()
    {
        auto msg = geometry_msgs::msg::Twist();
        msg.linear.x = 0.0;
        msg.angular.z = 0.0;
        cmd_publisher_->publish(msg);
        RCLCPP_INFO(this->get_logger(), "Publishing: linear.x=%.2f, angular.z=%.2f", msg.linear.x, msg.angular.z);

    }

private:
    std::string node_name_;
    float direction_;
    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr laser_subscriber_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_publisher_;
    rclcpp::TimerBase::SharedPtr timer_;

    void laserscan_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
    {
        /*
        float min_range = msg->ranges[50];
        int min_index = 50;

        // Find the index of the minimum range in the laser scan data
        for (int i = 49 i > -151; i--) {
            if (i < 0) {i=(-1) *}
            if (msg->ranges[i] < min_range) {
                min_range = msg->ranges[i];
                min_index = i;
            }
        }

        RCLCPP_INFO(this->get_logger(), "min_range = %.2f", min_range);
        RCLCPP_INFO(this->get_logger(), "min_index = %i", min_index);

         if (min_range < .35)
        {
            direction_ = msg->angle_min + (min_index * msg->angle_increment);
            RCLCPP_INFO(this->get_logger(), "direction_ = %.2f", direction_);

        }
        else
        {
            direction_ = 0.0;
        }
        */

        int end = 150;


        auto min_distance = std::min_element(msg->ranges.begin(), msg->ranges.begin() + end + 1);
        int min_distance_index = std::distance(msg->ranges.begin(), min_distance);
        RCLCPP_INFO(this->get_logger(), "min_distance = %.2f", *min_distance);
        RCLCPP_INFO(this->get_logger(), "min_distance_index = %i", min_distance_index);

        /*
        RCLCPP_INFO(this->get_logger(), "100th index = %.2f", msg->ranges[100]);
        RCLCPP_INFO(this->get_logger(), "0 index = %.2f", msg->ranges[0]);
        RCLCPP_INFO(this->get_logger(), "150 index = %.2f", msg->ranges[150]);
        RCLCPP_INFO(this->get_logger(), "50 index = %.2f", msg->ranges[50]);


        RCLCPP_INFO(this->get_logger(), "angle_max = %.2f", msg->angle_max);
        RCLCPP_INFO(this->get_logger(), "angle_min = %.2f", msg->angle_min);
*/
        if ((min_distance_index < 151) && *min_distance < .35)
        {
            direction_ = msg->angle_min + (min_distance_index * msg->angle_increment);
            RCLCPP_INFO(this->get_logger(), "direction_ = %.2f", direction_);

        }
        else
        {
            direction_ = 0.0;
        }
        
    }

    void timer_control_callback()
    {
        double angular = 3.5 * direction_;
        angular = std::max(-1.2, std::min(1.2, angular));

        auto msg = geometry_msgs::msg::Twist();
        msg.linear.x = 0.1;
        msg.angular.z = angular;
        cmd_publisher_->publish(msg);
        RCLCPP_INFO(this->get_logger(), "Publishing: linear.x=%.2f, angular.z=%.2f", msg.linear.x, msg.angular.z);

    }
};

std::shared_ptr<PatrolNode> patrol_node;

void signal_handler(int signum)
{
    patrol_node->stop_fastbot();
    rclcpp::shutdown();
}


int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    patrol_node = std::make_shared<PatrolNode>();

    // Register the signal handler for CTRL+C
    signal(SIGINT, signal_handler);

    rclcpp::spin(patrol_node);
    return 0;
}