#pragma once

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <robot_interfaces/msg/wheel_speeds.hpp>
#include <robot_interfaces/msg/wheel_ticks.hpp>
#include <robot_interfaces/msg/pid_gains.hpp>

#include "packet_parser.hpp"
#include "tcp_service.hpp"

class RobotDriverNode : public rclcpp::Node {
public:
    RobotDriverNode();

    void on_raw_data(const uint8_t* data, size_t len);

private:
    protocol::FrameParser parser_;
    TcpService tcp_service_;

    rclcpp::Publisher<robot_interfaces::msg::WheelSpeeds>::SharedPtr speed_pub_;
    rclcpp::Publisher<robot_interfaces::msg::WheelTicks>::SharedPtr   pulse_pub_;
    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr               imu_pub_;
    rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr         lidar_pub_;
    rclcpp::Publisher<robot_interfaces::msg::PidGains>::SharedPtr     pid_pub_;

    void publish_speed(const protocol::SpeedData& d, uint8_t seq, uint32_t ts);
    void publish_pulse(const protocol::PulseData& d, uint8_t seq, uint32_t ts);
    void publish_imu(const protocol::ImuData& d,   uint8_t seq, uint32_t ts);
    void publish_lidar(const protocol::LidarData& d, uint8_t seq, uint32_t ts);
    void publish_pid(const protocol::PidData& d,   uint8_t seq, uint32_t ts);

    builtin_interfaces::msg::Time ms_to_stamp(uint32_t ms) const;
};