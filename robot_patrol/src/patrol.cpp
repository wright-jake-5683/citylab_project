#include "rclcpp/executor_options.hpp"
#include "rclcpp/executors/multi_threaded_executor.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "rclcpp/qos.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>

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

        cmd_publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);

        auto timer_period = std::chrono::milliseconds(100); //Equivalent to 10Hz
        //timer_ = this->create_wall_timer(timer_period, std::bind(&PatrolNode::timer_control_callback, this));

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

    struct Quadrants {
        std::vector<float> front;
        std::vector<float> left;
        std::vector<float> back;
        std::vector<float> right;
    };

    Quadrants splitLaserData(const sensor_msgs::msg::LaserScan &msg)
    {
         Quadrants q;

        // Check if there is any laser scan data coming in
        if (msg->ranges.empty())
        {
            return q;
        }

        // Reserve memory to improve speed
        size_t laser_vector_size = msg->ranges.size();
        q.front.reserve(laser_vector_size / 4);
        q.left.reserve(laser_vector_size / 4);
        q.back.reserve(laser_vector_size / 4);
        q.right.reserve(laser_vector_size / 4);


        for (size_t i=0; i < laser_vector_size; i++)
        {
            // Find angle based on angle min plus the angle increment times the index
            float angle = msg->angle_min + (i * msg->angle_increment);

            // Normalize angle between pi & -pi
            while (angle > M_PI) { angle -= 2.0 * M_PI; }
            while (angle < -M_PI) { angle += 2.0 * M_PI; }

            // Capture distance reading for laser at index i
            float distance = msg->range[i];

            // Ignore invalid readings (optional)
            if (std::isinf(distance) || std::isnan(distance) || distance < msg->range_min || distance > msg->range_max) 
            {
                continue;  // skip inf/nan/out-of-range
            }

            // Split into 4 quadrants: ±45° and ±135°
            if (angle >= -M_PI_4 && angle <= M_PI_4) {
                q.front.push_back(distance);           // -45° to +45°  → Front
            }
            else if (angle > M_PI_4 && angle < 3*M_PI_4) {
                q.left.push_back(distance);            // +45° to +135° → Left
            }
            else if (angle >= 3*M_PI_4 || angle <= -3*M_PI_4) {
                q.rear.push_back(distance);            // +135° to +180° and -180° to -135° → Rear
            }
            else {
                q.right.push_back(distance);           // -135° to -45° → Right
            }
        }
    }

    void laserscan_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
    {
        Quadrants quadrants = splitLaserData(*msg);

        for (int i=0; i < quadrants.front.size(); i++)
        {
            RCLCPP_INFO(this->get_logger(), "front i: %.2f", quadrants.front[i]);
        }

        auto min_distance_index = [](const std::vector<float> &v) -> float {
            auto distance = std::min_element(v.begin(), v.end())
            return v.empty() ? INFINITY : std::distance(v.begin(), distance);
        };

        float front_index  = min_distance(quadrants.front);
        float right_index  = min_distance(quadrants.right);
        float left_index  = min_distance(quadrants.left);

    }

    void timer_control_callback()
    {
        double angular = -2 * direction_;
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