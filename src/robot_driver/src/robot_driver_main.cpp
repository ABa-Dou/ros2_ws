#include "rclcpp/rclcpp.hpp"
#include "tcp_service.hpp"
int main(int argc, char **argv)
{
    // 初始化 ROS 2
    rclcpp::init(argc, argv);

    // 创建一个节点
    auto node = rclcpp::Node::make_shared("robot_driver_node");

    RCLCPP_INFO(node->get_logger(), "Robot driver started!");

    // 运行节点，等待回调
    rclcpp::spin(node);

    // 关闭 ROS 2
    rclcpp::shutdown();

    return 0;
}