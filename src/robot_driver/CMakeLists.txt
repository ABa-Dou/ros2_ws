cmake_minimum_required(VERSION 3.8)
project(robot_driver)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(robot_interfaces REQUIRED)

add_executable(robot_driver_node
  src/robot_driver_main.cpp
  src/robot_driver_node.cpp
  src/packet_parser.cpp
  src/tcp_service.cpp
  src/net_tool.cpp
)

target_include_directories(robot_driver_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_include_directories(robot_driver_node PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../common
)

ament_target_dependencies(robot_driver_node
  rclcpp
  sensor_msgs
  robot_interfaces
)

install(TARGETS robot_driver_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()