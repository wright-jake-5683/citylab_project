#include <functional>
#include <memory>
#include <thread>
#include <chrono>
#include <cmath>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2/LinearMath/Matrix3x3.h"
#include "action_msgs/msg/goal_status.hpp"
#include "custom_interfaces/action/go_to_pose.hpp"
#include "geometry_msgs/msg/pose.h"
#include "geometry_msgs/msg/twist.hpp"
#include <nav_msgs/msg/odometry.hpp> 

class GoToPose : public rclcpp::Node
{
    public:
        using GoToPoseAction = custom_interfaces::action::GoToPose;
        using GoalHandleGoToPose = rclcpp_action::ServerGoalHandle<GoToPoseAction>;


        GoToPose(const rclcpp::NodeOptions &options = rclcpp::NodeOptions())
            : Node("go_to_pose", options)
        {
            odom_sub_ = this->create_subscription<nav_msgs::msg::Odometry>(
                "fastbot_1/odom",
                10,
                std::bind(&GoToPose::odom_callback, this, std::placeholders::_1)
            );

            cmd_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("/fastbot_1/cmd_vel", 10);

            this->action_server_ = rclcpp_action::create_server<GoToPoseAction>(
                this,
                "go_to_pose",
                std::bind(&GoToPose::handle_goal, this, std::placeholders::_1, std::placeholders::_2),
                std::bind(&GoToPose::handle_cancel, this, std::placeholders::_1),
                std::bind(&GoToPose::handle_accepted, this, std::placeholders::_1)
            );

            RCLCPP_INFO(this->get_logger(), "Action server /go_to_pose ready!");
            
        }

    private:
        rclcpp_action::Server<GoToPoseAction>::SharedPtr action_server_;
        rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_pub_;
        rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
        geometry_msgs::msg::Pose2D desired_pos_;
        geometry_msgs::msg::Pose2D current_pos_;

        void odom_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
        {
            current_pos_.x = msg->pose.pose.position.x;
            current_pos_.y = msg->pose.pose.position.y;

             // Calculate the yaw (orientation around the z-axis)
            tf2::Quaternion q(
                msg->pose.pose.orientation.x,
                msg->pose.pose.orientation.y,
                msg->pose.pose.orientation.z,
                msg->pose.pose.orientation.w);

            tf2::Matrix3x3 m(q);
            double roll, pitch, yaw;
            m.getRPY(roll, pitch, yaw);
            
            //theta usually just means the yaw angle --> the robot’s rotation around the z-axis.
            current_pos_.theta = yaw;
        }

        rclcpp_action::GoalResponse handle_goal(const rclcpp_action::GoalUUID &uuid, std::shared_ptr<const GoToPoseAction::Goal> goal)
        {
            RCLCPP_INFO(this->get_logger(), "Action server /go_to_pose called!");
            RCLCPP_INFO(this->get_logger(), "Recieved a goal request at x: %.2f, y: %.2f, theta: %.2f", goal->goal_pos.x, goal->goal_pos.y, goal->goal_pos.theta);
            (void)uuid;
            return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
        }


        rclcpp_action::CancelResponse handle_cancel(const std::shared_ptr<GoalHandleGoToPose> goal_handle)
        {
            RCLCPP_INFO(this->get_logger(), "Recieved request to cancel goal");
            (void)goal_handle;
            return rclcpp_action::CancelResponse::ACCEPT;
        }

        void handle_accepted(const std::shared_ptr<GoalHandleGoToPose> goal_handle)
        {
            const auto goal = goal_handle->get_goal();
            
            
            desired_pos_.x = goal->goal_pos.x;
            desired_pos_.y = goal->goal_pos.y;
            desired_pos_.theta = goal->goal_pos.theta * (M_PI/180);
            RCLCPP_ERROR(this->get_logger(), "Desired_theta: %.2f", desired_pos_.theta);

            RCLCPP_INFO(this->get_logger(), "Desired Position: x: %.2f, y: %.2f, theta: %.4f",
                desired_pos_.x,
                desired_pos_.y,
                desired_pos_.theta
            );
            

            std::thread{
                std::bind(&GoToPose::execute, this, std::placeholders::_1),
                goal_handle
            }.detach();
        }

        void execute(const std::shared_ptr<GoalHandleGoToPose> goal_handle)
        {
            try 
            {
                RCLCPP_INFO(this->get_logger(), "Executing goal");

                auto feedback = std::make_shared<GoToPoseAction::Feedback>();
                auto result = std::make_shared<GoToPoseAction::Result>();

                rclcpp::Rate rate(10);
                while (rclcpp::ok())
                {
                    feedback->current_pos.x = current_pos_.x; 
                    feedback->current_pos.y = current_pos_.y;
                    feedback->current_pos.theta = (current_pos_.theta * (M_PI/180));
                    goal_handle->publish_feedback(feedback);

                    if (move_to_goal())
                    {
                        stop_fastbot();
                        RCLCPP_INFO(this->get_logger(), "Goal reached successfully");
                        result->status = true;
                        goal_handle->succeed(result);
                        RCLCPP_INFO(this->get_logger(), "Action has been completed!!");
                        return;
                    }

                    rate.sleep();
                }
            }
            catch (const std::exception & e)
            {
                RCLCPP_ERROR(this->get_logger(), "Exception executing goal: %s", e.what());
            }
        }


        bool move_to_goal()
        {
            // find the vector that represented the difference between where you want to go and where you currently are
            auto direction_vector = std::make_tuple((desired_pos_.x - current_pos_.x), (desired_pos_.y - current_pos_.y));

            //taget_theta is the direction you want to go in
            double target_theta = atan2(std::get<1>(direction_vector), std::get<0>(direction_vector));

            // find the difference between the target direction and the direction you are currently facing to figure out the angle in which you need to turn to face the target
            double yaw_error = target_theta - current_pos_.theta;
            
            //Normalize angle get sign to be able to tell direction as yaw-error can be outside pi & -pi
            yaw_error = normalize_angle(yaw_error);

            // set a tolerancae for facing the correct direction to avoid constant adjustments
            double tolerance = 0.1; // --> in radians, about 5.7 degrees

            if ((std::get<0>(direction_vector) > -.03 && std::get<0>(direction_vector) < .03) && (std::get<1>(direction_vector) > -.03 && std::get<1>(direction_vector) < .03))
            {
                double target_yaw_error = desired_pos_.theta - current_pos_.theta;
                if (std::abs(target_yaw_error) > tolerance)
                {
                    RCLCPP_INFO(this->get_logger(), "Adjusting theta");
                    rotate_to_goal_theta(target_yaw_error, tolerance);
                    return false;
                }
                else 
                {
                    RCLCPP_INFO(this->get_logger(), "Goal Reached");
                    return true;
                }
            }

            float ang_vel = determine_theta(yaw_error, tolerance);
            
            if (yaw_error < tolerance && yaw_error > -tolerance)
            {
                publish_velocities(0.2, 0.0);
                return false;
            }
            else 
            {
                publish_velocities(0.0, ang_vel);
                return false;
            }

        }

        float determine_theta(double yaw_error, double tolerance)
        {
            if (std::abs(yaw_error) < tolerance)
            {
                return 0.0;
            }
            else if (yaw_error < 0 && std::abs(yaw_error) > tolerance)
            {
                return -0.5;
            }
            else if (yaw_error > 0  && std::abs(yaw_error) > tolerance)
            {
                return 0.5;
            }
            else
            {
                RCLCPP_ERROR(this->get_logger(), "Could not determine direction to reach goal...");
                return 0.0;
            }
        }

        void rotate_to_goal_theta(double target_yaw_error, double tolerance)
        {
            float ang_vel = determine_theta(target_yaw_error, tolerance);

            if (std::abs(target_yaw_error) > tolerance)
            {
                publish_velocities(0.0, ang_vel);
            }

        }


        double normalize_angle(double angle) {
            return std::atan2(std::sin(angle), std::cos(angle));
        }


        void stop_fastbot() {
            publish_velocities(0.0, 0.0);
            RCLCPP_INFO(this->get_logger(), "Stopping Fastbot");
        }

        void publish_velocities(float linear_vel, float ang_vel) {
            auto msg = geometry_msgs::msg::Twist();
            msg.linear.x = linear_vel;
            msg.angular.z = ang_vel;
            cmd_pub_->publish(msg);

            RCLCPP_INFO(this->get_logger(), "Publishing velocities: linear.x=%.2f, angular.z=%.2f",
                        msg.linear.x, msg.angular.z);
        }
       
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  
  auto node = std::make_shared<GoToPose>();
  
  rclcpp::spin(node);
  
  rclcpp::shutdown();
  return 0;
}