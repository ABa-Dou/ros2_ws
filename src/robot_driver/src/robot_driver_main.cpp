#include "rclcpp/rclcpp.hpp"
#include "robot_driver/robot_driver_node.hpp"

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<RobotDriverNode>();
    RCLCPP_INFO(node->get_logger(), "Robot driver started!");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}