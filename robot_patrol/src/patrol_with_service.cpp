#include "geometry_msgs/msg/twist.hpp"
#include "rclcpp/callback_group.hpp"
#include "rclcpp/executor_options.hpp"
#include "rclcpp/executors/multi_threaded_executor.hpp"
#include "rclcpp/qos.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp/subscription_options.hpp"
#include "sensor_msgs/msg/detail/laser_scan__struct.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <custom_interfaces/srv/get_direction.hpp>

using namespace std::chrono_literals; 

class DirectionClientNode : public rclcpp::Node
{
    public:
        DirectionClientNode(const std::string &node_name = "direction_client_node")
        : Node(node_name), node_name_(node_name) {

            std::string service_name = "/direction_service";
            service_client_ = this->create_client<custom_interfaces::srv::GetDirection>(service_name);

            // Wait for the service to be available (checks every second)
            while (!service_client_->wait_for_service(1s)) {
            if (!rclcpp::ok()) {
                RCLCPP_ERROR(this->get_logger(), "Interrupted while waiting for the %s service. Exiting...", service_name.c_str());
                return;
            }
                RCLCPP_INFO(this->get_logger(), "%s not available, waiting again...", service_name.c_str());
            }

            RCLCPP_INFO(this->get_logger(), "%s client ready!", service_name.c_str());
        }

        std::string call_get_direction_service(sensor_msgs::msg::LaserScan laser_data) {
            auto request = std::make_shared<custom_interfaces::srv::GetDirection::Request>();
            request->laser_data = laser_data;

            auto future = service_client_->async_send_request(request);

            // Block until the response arrives (safe with Reentrant + MultiThreadedExecutor)
            if (future.wait_for(2s) == std::future_status::ready) {
                auto response = future.get();
                if (response) {
                    return response->direction;
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

            return "forward"; // safe fallback
        }
    std::string node_name_;
    rclcpp::Client<custom_interfaces::srv::GetDirection>::SharedPtr service_client_;
};

class PatrolNode : public rclcpp::Node {
public:
    PatrolNode(std::shared_ptr<DirectionClientNode> direction_client, const std::string &node_name = "robot_patrol_node")
      : Node(node_name), node_name_(node_name), direction_client_node_(direction_client) 
    {
        auto qos = rclcpp::QoS(10).reliability(rclcpp::ReliabilityPolicy::Reliable);
        reentrant_group_ = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);
        rclcpp::SubscriptionOptions sub_options;
        sub_options.callback_group = reentrant_group_;

        laser_subscriber_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
            "/scan", 
            qos,
            std::bind(&PatrolNode::laserscan_callback, this, std::placeholders::_1),
            sub_options
            );

        cmd_publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);

        auto timer_period = std::chrono::milliseconds(100); // Equivalent to 10Hz
        timer_ = this->create_wall_timer(
            timer_period, 
            std::bind(&PatrolNode::timer_control_callback, this),
            reentrant_group_
            );

        RCLCPP_INFO(this->get_logger(), "%s ready...", node_name_.c_str());
    }

    void stop_fastbot() {
        auto msg = geometry_msgs::msg::Twist();
        msg.linear.x = 0.0;
        msg.angular.z = 0.0;
        cmd_publisher_->publish(msg);
        RCLCPP_INFO(this->get_logger(), "Publishing: linear.x=%.2f, angular.z=%.2f",
                    msg.linear.x, msg.angular.z);
    }

private:
    double normalizeAngle(double angle) {
        return std::atan2(std::sin(angle), std::cos(angle));
    }

    void laserscan_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg) {
         //RCLCPP_INFO(this->get_logger(), "laserscan fired...");

        sensor_msgs::msg::LaserScan front_scan = *msg; // copy header, frame_id, etc.
        sensor_msgs::msg::LaserScan danger_zone_scan = *msg;

        double front_angle_split = M_PI * 0.5; // 90 degrees -> pi/2
        double danger_zone_angle_split = M_PI * .2; // ~30 degrees 

        // clear existing values in the copied ranges and intensities array. This
        // removes all copied values while keeping the structure of the msg the same
        front_scan.ranges.clear();
        front_scan.intensities.clear();
        danger_zone_scan.ranges.clear();
        danger_zone_scan.intensities.clear();

        for (size_t i = 0; i < msg->ranges.size(); i++) {
            double angle = msg->angle_min + (i * msg->angle_increment);
            angle = normalizeAngle(angle);

            // Keep only beams in front sector: [-angle_split, +angle_split]
            if (angle >= -front_angle_split && angle <= front_angle_split) 
            {
                RCLCPP_INFO(this->get_logger(), "front scan angle: %.2f", angle);
                front_scan.ranges.push_back(msg->ranges[i]);

                if (!msg->intensities.empty())
                front_scan.intensities.push_back(msg->intensities[i]);
            }
            if (angle >=-danger_zone_angle_split && angle <= danger_zone_angle_split)
            {
                danger_zone_scan.ranges.push_back(msg->ranges[i]);

                if (!msg->intensities.empty())
                danger_zone_scan.intensities.push_back(msg->intensities[i]);
            }
        }


        front_scan.angle_min = -front_angle_split;
        front_scan.angle_max = front_angle_split;
        danger_zone_scan.angle_min = -danger_zone_angle_split;
        danger_zone_scan.angle_max = danger_zone_angle_split;


        RCLCPP_INFO(this->get_logger(), "front scan angle min: %.2f", front_scan.angle_min );
        RCLCPP_INFO(this->get_logger(), "front scan angle max: %.2f", front_scan.angle_max);
        RCLCPP_INFO(this->get_logger(), "danger scan angle min: %.2f", danger_zone_scan.angle_min );
        RCLCPP_INFO(this->get_logger(), "danger scan angle max: %.2f", danger_zone_scan.angle_max);
    

        front_laser_data_ = front_scan;
        danger_laser_data_ = danger_zone_scan;

    }

    void timer_control_callback() 
    {
        //RCLCPP_INFO(this->get_logger(), "timer fired...");
        if (!danger_laser_data_.ranges.empty()) 
        {
            handle_obstacles();
        }
        else {
            RCLCPP_WARN(this->get_logger(), "danger laserscan data is empty...");
        }
    }

    void handle_obstacles() {
        bool obstacle_detected = check_laser_data();
        if (obstacle_detected) {
            std::string safest_direction = direction_client_node_->call_get_direction_service(front_laser_data_);
            RCLCPP_INFO(this->get_logger(), "safe direction: %s", safest_direction.c_str());
            move_fastbot(safest_direction);
        } 
        else 
        {
            move_fastbot("forward");
        }
    }

    bool check_laser_data() {
        auto min_distance = std::min_element(danger_laser_data_.ranges.begin(), danger_laser_data_.ranges.end());
        int min_distance_index = std::distance(danger_laser_data_.ranges.begin(), min_distance);
        double min_distance_angle = danger_laser_data_.angle_min + (min_distance_index * danger_laser_data_.angle_increment);
        RCLCPP_INFO(this->get_logger(), "min distance: %.2f", *min_distance);
        RCLCPP_INFO(this->get_logger(), "min distance angle: %.2f", min_distance_angle);

        if (*min_distance < .35) 
        {
            RCLCPP_INFO(this->get_logger(), "check_laser_data returned true");
            return true;
        } 
        else 
        {
            RCLCPP_INFO(this->get_logger(), "check_laser_data returned false");
            return false;
        }
    }

    void move_fastbot(std::string safest_direction) {
        //RCLCPP_INFO(this->get_logger(), "move fired...");
        //RCLCPP_INFO(this->get_logger(), "safe direction: %s", safest_direction.c_str());
        float ang_vel;

        if (safest_direction == "forward") 
        {
            ang_vel = 0.0;
        } 
        else if (safest_direction == "left") 
        {
            ang_vel = 0.5;
        } 
        else if (safest_direction == "right") 
        {
            ang_vel = -0.5;
        } 
        else 
        {
            RCLCPP_INFO(this->get_logger(), "no safe direction found");
            return;
        }

        publish_velocities(ang_vel);
    }

    void publish_velocities(float ang_vel) {
        auto msg = geometry_msgs::msg::Twist();
        msg.linear.x = 0.1;
        msg.angular.z = ang_vel;
        cmd_publisher_->publish(msg);

        RCLCPP_INFO(this->get_logger(), "Publishing: linear.x=%.2f, angular.z=%.2f",
                    msg.linear.x, msg.angular.z);
    }

    std::string node_name_;
    rclcpp::CallbackGroup::SharedPtr reentrant_group_;
    std::shared_ptr<DirectionClientNode> direction_client_node_;
    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr laser_subscriber_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    sensor_msgs::msg::LaserScan front_laser_data_;
    sensor_msgs::msg::LaserScan danger_laser_data_;
};

std::shared_ptr<PatrolNode> patrol_node;

void signal_handler(int signum) {
  patrol_node->stop_fastbot();
  rclcpp::shutdown();
}

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  auto direction_client_node = std::make_shared<DirectionClientNode>();
  patrol_node = std::make_shared<PatrolNode>(direction_client_node);

  rclcpp::executors::MultiThreadedExecutor executor(rclcpp::ExecutorOptions(), 2);
  executor.add_node(patrol_node);
  executor.add_node(direction_client_node);

  // Register the signal handler for CTRL+C
  signal(SIGINT, signal_handler);

  try
  {
    executor.spin();
  }
  catch(const std::exception& e)
  {
    RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Exception: %s", e.what());
  }

  //rclcpp::spin(patrol_node);
  return 0;
}