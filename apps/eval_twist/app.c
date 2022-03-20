#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <geometry_msgs/msg/twist.h>

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

rcl_publisher_t publisher;
rcl_subscription_t subscriber;

geometry_msgs__msg__Twist msg;

void subscription_callback(const void * msgin)
{
  printf("sub\r\n");

  const geometry_msgs__msg__Twist * msg = (const geometry_msgs__msg__Twist *)msgin;

  rcl_publish(&publisher, (const void*)msg, NULL);
}

void appMain(void *argument)
{
	printf("main start\r\n");

	rcl_allocator_t allocator = rcl_get_default_allocator();
	rclc_support_t support;

	// create init_options
	rclc_support_init(&support, 0, NULL, &allocator);

	// create node
	rcl_node_t node;
	rclc_node_init_default(&node, "uros_node", "", &support);

	// create publisher
	rclc_publisher_init_default(&publisher, &node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist), "to_linux");

	// create subscriber
	rclc_subscription_init_default(&subscriber, &node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist), "to_stm");

	// create executor
	rclc_executor_t executor;
	rclc_executor_init(&executor, &support.context, 3, &allocator);
	rclc_executor_add_subscription(&executor, &subscriber, &msg,
		&subscription_callback, ON_NEW_DATA);

	while(1){
		rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
		usleep(10000);
	}

	// Free resources
	rcl_publisher_fini(&publisher, &node);
	rcl_subscription_fini(&subscriber, &node);
	rcl_node_fini(&node);
}
