#include "robot_driver/robot_driver_node.hpp"

RobotDriverNode::RobotDriverNode() : Node("robot_driver_node")
{
    speed_pub_ = this->create_publisher<robot_interfaces::msg::WheelSpeeds>(
        "~/wheel_speeds", 10);
    pulse_pub_ = this->create_publisher<robot_interfaces::msg::WheelTicks>(
        "~/wheel_ticks", 10);
    imu_pub_ = this->create_publisher<sensor_msgs::msg::Imu>(
        "~/imu", 10);
    lidar_pub_ = this->create_publisher<sensor_msgs::msg::LaserScan>(
        "~/lidar", 10);
    pid_pub_ = this->create_publisher<robot_interfaces::msg::PidGains>(
        "~/pid_gains", 10);

    parser_.on_speed = [this](const protocol::SpeedData& d, uint8_t seq, uint32_t ts) {
        publish_speed(d, seq, ts);
    };
    parser_.on_pulse = [this](const protocol::PulseData& d, uint8_t seq, uint32_t ts) {
        publish_pulse(d, seq, ts);
    };
    parser_.on_imu = [this](const protocol::ImuData& d, uint8_t seq, uint32_t ts) {
        publish_imu(d, seq, ts);
    };
    parser_.on_lidar = [this](const protocol::LidarData& d, uint8_t seq, uint32_t ts) {
        publish_lidar(d, seq, ts);
    };
    parser_.on_pid = [this](const protocol::PidData& d, uint8_t seq, uint32_t ts) {
        publish_pid(d, seq, ts);
    };

    tcp_service_.on_data = [this](const uint8_t* data, size_t len) {
        on_raw_data(data, len);
    };
    if (!tcp_service_.start(common_cfg::DEFAULT_PORT)) {
        RCLCPP_ERROR(this->get_logger(), "Failed to start TCP service on port %d", common_cfg::DEFAULT_PORT);
    } else {
        RCLCPP_INFO(this->get_logger(), "TCP service started on port %d", common_cfg::DEFAULT_PORT);
    }
}

void RobotDriverNode::on_raw_data(const uint8_t* data, size_t len)
{
    parser_.feed(data, len);
}

void RobotDriverNode::publish_speed(const protocol::SpeedData& d, uint8_t seq, uint32_t ts)
{
    auto msg = robot_interfaces::msg::WheelSpeeds();
    msg.lf = d.lf;
    msg.rf = d.rf;
    msg.lr = d.lr;
    msg.rr = d.rr;
    speed_pub_->publish(msg);
}

void RobotDriverNode::publish_pulse(const protocol::PulseData& d, uint8_t seq, uint32_t ts)
{
    auto msg = robot_interfaces::msg::WheelTicks();
    msg.lf = d.lf;
    msg.rf = d.rf;
    msg.lr = d.lr;
    msg.rr = d.rr;
    pulse_pub_->publish(msg);
}

void RobotDriverNode::publish_imu(const protocol::ImuData& d, uint8_t seq, uint32_t ts)
{
    auto msg = sensor_msgs::msg::Imu();
    msg.header.stamp = ms_to_stamp(ts);
    msg.linear_acceleration.x = d.ax;
    msg.linear_acceleration.y = d.ay;
    msg.linear_acceleration.z = d.az;
    msg.angular_velocity.x = d.gx;
    msg.angular_velocity.y = d.gy;
    msg.angular_velocity.z = d.gz;
    imu_pub_->publish(msg);
}

void RobotDriverNode::publish_lidar(const protocol::LidarData& d, uint8_t seq, uint32_t ts)
{
    auto msg = sensor_msgs::msg::LaserScan();
    msg.header.stamp = ms_to_stamp(ts);
    msg.header.frame_id = "laser";
    msg.angle_min = d.points.empty() ? 0.0f : d.points.front().angle;
    msg.angle_max = d.points.empty() ? 0.0f : d.points.back().angle;
    msg.angle_increment = (d.point_count > 1)
        ? (msg.angle_max - msg.angle_min) / static_cast<float>(d.point_count - 1)
        : 0.0f;
    msg.time_increment = 0.0f;
    msg.scan_time = 0.0f;
    msg.range_min = 0.0f;
    msg.range_max = 100.0f;
    msg.ranges.resize(d.point_count);
    for (uint16_t i = 0; i < d.point_count; ++i) {
        msg.ranges[i] = d.points[i].distance;
    }
    lidar_pub_->publish(msg);
}

void RobotDriverNode::publish_pid(const protocol::PidData& d, uint8_t seq, uint32_t ts)
{
    auto msg = robot_interfaces::msg::PidGains();
    for (int i = 0; i < 4; ++i) {
        msg.kp[i] = d.kp[i];
        msg.ki[i] = d.ki[i];
        msg.kd[i] = d.kd[i];
        msg.ff_gain[i] = d.ff_gain[i];
        msg.integral_limit[i] = d.integral_limit[i];
        msg.integral_separation_err[i] = d.integral_separation_err[i];
        msg.filter_alpha[i] = d.filter_alpha[i];
    }
    pid_pub_->publish(msg);
}