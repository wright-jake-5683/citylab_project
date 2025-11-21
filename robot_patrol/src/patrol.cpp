#include "rclcpp/executor_options.hpp"
#include "rclcpp/executors/multi_threaded_executor.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include <nav_msgs/msg/odometry.hpp>
#include "rclcpp/qos.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Matrix3x3.h>

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

        odom_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
        "/odom", 10,
        std::bind(&PatrolNode::odomCallback, this, std::placeholders::_1));

        cmd_publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);

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
    double current_yaw_;
    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr laser_subscriber_;
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_subscriber_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_publisher_;
    rclcpp::TimerBase::SharedPtr timer_;

    void odomCallback(const nav_msgs::msg::Odometry::SharedPtr msg)
    {
        // Extract yaw from quaternion (range -π..π)
        tf2::Quaternion q(
            msg->pose.pose.orientation.x,
            msg->pose.pose.orientation.y,
            msg->pose.pose.orientation.z,
            msg->pose.pose.orientation.w);
        double roll, pitch, yaw;
        tf2::Matrix3x3(q).getRPY(roll, pitch, yaw);

        // Normalize to [-π, π]
        current_yaw_ = yaw;
        while (current_yaw_ > M_PI)  current_yaw_ -= 2*M_PI;
        while (current_yaw_ < -M_PI) current_yaw_ += 2*M_PI;
        //RCLCPP_INFO(this->get_logger(), "current_yaw_ = %.2f", current_yaw_);
    }


    void laserscan_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
    {
        //RCLCPP_INFO(this->get_logger(), "msg scan = %li", msg->ranges.size());
        
        
        sensor_msgs::msg::LaserScan front_scan = *msg;  // copy header, frame_id, etc.

        double angle_split = M_PI / 180.0 * (180.0 / 2.5); // 90 degrees -> pi/2


        std::vector<float> front_ranges;
        std::vector<float> front_intensities;

        front_ranges.reserve(msg->ranges.size() / 2 + 10);

        for (size_t i=338; i < msg->ranges.size(); i++)
        {
            double angle = msg->angle_min + (i * msg->angle_increment);

            while (angle >  M_PI) angle -= 2.0 * M_PI;
            while (angle <= -M_PI) angle += 2.0 * M_PI;

            // Keep only beams in front sector: [-angle_split, +angle_split]
            if (angle >= -angle_split && angle <= angle_split)
            {
                front_ranges.push_back(msg->ranges[i]);

                if (!msg->intensities.empty())
                    front_intensities.push_back(msg->intensities[i]);
            }
        }

        for (size_t i=0; i < 338; i++)
        {
            double angle = msg->angle_min + (i * msg->angle_increment);

            while (angle >  M_PI) angle -= 2.0 * M_PI;
            while (angle <= -M_PI) angle += 2.0 * M_PI;

            // Keep only beams in front sector: [-angle_split, +angle_split]
            if (angle >= -angle_split && angle <= angle_split)
            {
                front_ranges.push_back(msg->ranges[i]);

                if (!msg->intensities.empty())
                    front_intensities.push_back(msg->intensities[i]);
            }
        }

        front_scan.ranges = std::move(front_ranges);
        if (!front_intensities.empty())
            front_scan.intensities = std::move(front_intensities);
        else
            front_scan.intensities.clear();
        
        // update angle_min / angle_max / increment to reflect new data
        if (!front_scan.ranges.empty())
        {
            front_scan.angle_min = -angle_split;
            front_scan.angle_max = +angle_split;
            front_scan.angle_increment = msg->angle_increment;  // usually unchanged
        }
        
        RCLCPP_INFO(this->get_logger(), "angle min = %.2f", front_scan.angle_min);
        RCLCPP_INFO(this->get_logger(), "angle max = %.2f", front_scan.angle_max);

        auto min_distance = std::min_element(front_scan.ranges.begin(), front_scan.ranges.end());
        int min_distance_index = std::distance(front_scan.ranges.begin(), min_distance);
        float min_distance_angle = front_scan.angle_min + (min_distance_index * front_scan.angle_increment);
        RCLCPP_INFO(this->get_logger(), "min_distance = %.2f", *min_distance);
        RCLCPP_INFO(this->get_logger(), "min_distance_index = %i", min_distance_index);
        RCLCPP_INFO(this->get_logger(), "min_distance_angle = %f", min_distance_angle);

        //float angle = 1.57;
        //float index_float = (angle - front_scan.angle_min) / front_scan.angle_increment;
        //int index = static_cast<int>(std::round(index_float));
        //RCLCPP_INFO(this->get_logger(), "index at %.2f = %i", angle, index);
        
        if ((min_distance_index <= 230 && min_distance_index >= 0) && *min_distance < .35)
        {
            direction_ = (front_scan.angle_min + (min_distance_index * front_scan.angle_increment));
            RCLCPP_INFO(this->get_logger(), "direction_ = %.2f", direction_);
        }
        else
        {
            direction_ = 0.0;
        }
    }

    void timer_control_callback()
    {
        double ang_vel = -2 * direction_;
        ang_vel = std::max(-1.2, std::min(0.75, ang_vel));

        /*
        const double yaw_limit = M_PI_2;

        bool turning_left  = ang_vel > 0.0;
        bool turning_right = ang_vel < 0.0;

        bool would_exceed =
        (turning_left  && current_yaw_ >= yaw_limit) ||
        (turning_right && current_yaw_ <= -yaw_limit);

        if (would_exceed) {
            ang_vel = 0.0;
            RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
                           "Yaw limit reached (%.1f°), blocking rotation!",
                           current_yaw_ * 180.0 / M_PI);
        }
        */
        auto msg = geometry_msgs::msg::Twist();
        msg.linear.x = 0.1;
        msg.angular.z = ang_vel;
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