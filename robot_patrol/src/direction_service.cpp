#include "custom_interfaces/srv/get_direction.hpp"
#include "rclcpp/node.hpp"
#include <cstddef>
#include <cstdint>
#include <rclcpp/rclcpp.hpp>

class DirectionService : public rclcpp::Node {
public:
  DirectionService() : Node("direction_service_node") {

    std::string service_name = "/direction_service";
    service_ = this->create_service<custom_interfaces::srv::GetDirection>(
        service_name,
        std::bind(&DirectionService::find_safe_direction_callback, this,
                  std::placeholders::_1, std::placeholders::_2)
    );

    RCLCPP_INFO(this->get_logger(), "/direction_service is ready...");
  }

private:
  void find_safe_direction_callback(const std::shared_ptr<custom_interfaces::srv::GetDirection::Request> request,
      std::shared_ptr<custom_interfaces::srv::GetDirection::Response> response) 
    {
        try 
        {
            RCLCPP_INFO(this->get_logger(), "/direction_service has been requested...");

            /*
            RCLCPP_INFO(this->get_logger(), "Request received - ranges size: %zu, angle_min: %.2f, angle_max: %.2f, angle_increment: %.4f",
                request->laser_data.ranges.size(),
                request->laser_data.angle_min,
                request->laser_data.angle_max,
                request->laser_data.angle_increment
            );
            */
            split_laser_data(request);
            std::string safest_direction = determine_safest_direction();
            response->direction = safest_direction;
            RCLCPP_INFO(this->get_logger(), "/direction_service has been completed...");
        }
        catch(const std::exception& e)
        {
            RCLCPP_ERROR(this->get_logger(), "find_safe Exception: %s", e.what());
        }
    }

  void split_laser_data(
      const std::shared_ptr<custom_interfaces::srv::GetDirection::Request> request) 
        {
            try 
            {
                total_dist_sec_right_.clear();
                total_dist_sec_front_.clear();
                total_dist_sec_left_.clear();

                RCLCPP_INFO(this->get_logger(), "angle min: %.2f", request->laser_data.angle_min);
                RCLCPP_INFO(this->get_logger(), "angle max: %.2f", request->laser_data.angle_max);

                for (size_t i = 0; i < request->laser_data.ranges.size(); i++) {
                    double angle = request->laser_data.angle_min + (i * request->laser_data.angle_increment);

                    if (angle >= (request->laser_data.angle_min) && angle <= -(M_PI / 6)) 
                    {
                        total_dist_sec_right_.push_back(request->laser_data.ranges[i]);
                        //RCLCPP_INFO(this->get_logger(), "right i = %li", i);
                    } 
                    else if (angle > -(M_PI / 6) && angle < (M_PI / 6)) 
                    {
                        total_dist_sec_front_.push_back(request->laser_data.ranges[i]);
                        //RCLCPP_INFO(this->get_logger(), "front i = %li", i);
                    } 
                    else if (angle >= (M_PI / 6) && angle <= request->laser_data.angle_max) 
                    {
                        total_dist_sec_left_.push_back(request->laser_data.ranges[i]);
                        //RCLCPP_INFO(this->get_logger(), "left i = %li", i);
                    }
                }
                
                RCLCPP_INFO(this->get_logger(), "Sec left ranges size: %zu", total_dist_sec_left_.size());
                RCLCPP_INFO(this->get_logger(), "Sec front ranges size: %zu", total_dist_sec_front_.size());
                RCLCPP_INFO(this->get_logger(), "Sec right ranges size: %zu", total_dist_sec_right_.size());
                
            }
            catch(const std::exception& e)
            {
                RCLCPP_ERROR(this->get_logger(), "split data Exception: %s", e.what());
            }
        }

  double sum_total_dist_sec_right_()
  {
    double sum = 0;
    for (size_t i = 0; i < total_dist_sec_right_.size(); i++) {
        //RCLCPP_INFO(this->get_logger(), "right i = %.2f", total_dist_sec_right_[i]);
      sum += total_dist_sec_right_[i];
    }
    return sum;
  }

  double sum_total_dist_sec_front_()
  {
    double sum = 0;
    for (size_t i = 0; i < total_dist_sec_front_.size(); i++) {
      sum += total_dist_sec_front_[i];
    }
    return sum;
  }

  double sum_total_dist_sec_left_()
  {
    double sum = 0;
    for (size_t i = 0; i < total_dist_sec_left_.size(); i++) {
    //RCLCPP_INFO(this->get_logger(), "left i = %.2f", total_dist_sec_left_[i]);
      sum += total_dist_sec_left_[i];
    }
    return sum;
  }

  std::string determine_safest_direction() {
    std::string safest_direction;

    double total_right = sum_total_dist_sec_right_();
    double total_front = sum_total_dist_sec_front_();
    double total_left = sum_total_dist_sec_left_();

    
    RCLCPP_INFO(this->get_logger(), "total right: %.2f", total_right);
    RCLCPP_INFO(this->get_logger(), "total front: %.2f", total_front);
    RCLCPP_INFO(this->get_logger(), "total left: %.2f", total_left);
    

    double max = std::max({total_right, total_front, total_left});

    if (max == total_front) {
      safest_direction = "forward";
    } 
    else if (max == total_right) {
      safest_direction = "right";
    }
    else if (max == total_left) {
      safest_direction = "left";
    } 
    else {
      safest_direction = "undetermined";
    }

    return safest_direction;
  }

  rclcpp::Service<custom_interfaces::srv::GetDirection>::SharedPtr service_;
  std::vector<double> total_dist_sec_right_;
  std::vector<double> total_dist_sec_front_;
  std::vector<double> total_dist_sec_left_;
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  auto node = std::make_shared<DirectionService>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}