#include <ros/ros.h>
#include "sensor_msgs/JointState.h"
#include "openarms/ArmActuators.h"
#include "openarms/SetJointTarget.h"
using std::string;

sensor_msgs::JointState g_target;
ros::Publisher *g_actuator_pub = NULL;
openarms::ArmActuators g_actuators;
//double g_stepper_vel[4];

void target_cb(const sensor_msgs::JointState &target_msg)
{
  g_target = target_msg;
}

bool set_joint_target_srv(openarms::SetJointTarget::Request &req,
                          openarms::SetJointTarget::Response &res)
{
  g_target = req.target;
  return true;
}

void state_cb(const sensor_msgs::JointState::ConstPtr &state_msg)
{
  const double MAX_VEL = 2000;
  const double MAX_ACCEL_PER_SEC = 3000;
  static ros::Time s_prev_time;
  static bool s_prev_time_init = false;

  if (!s_prev_time_init)
  {
    s_prev_time_init = true;
    s_prev_time = ros::Time::now();
    return;
  }
  else
  {
    ros::Time t = ros::Time::now();
    double dt = (t - s_prev_time).toSec();
    s_prev_time = t;
    const int32_t MAX_ACCEL = (int32_t)(MAX_ACCEL_PER_SEC * dt);

    if (g_target.position.size() > 0)
    {
      for (int i = 0; i < 4; i++)
      {
        double err = g_target.position[i] - state_msg->position[i];
        double desired_vel = 10000 * err;
        double accel = desired_vel - g_actuators.stepper_vel[i];
        // enforce acceleration limit
        if (accel > MAX_ACCEL)
          accel = MAX_ACCEL;
        else if (accel < -MAX_ACCEL)
          accel = -MAX_ACCEL;
        // enforce velocity limit
        g_actuators.stepper_vel[i] += accel;
        if (g_actuators.stepper_vel[i] > MAX_VEL)
          g_actuators.stepper_vel[i] = MAX_VEL;
        else if (g_actuators.stepper_vel[i] < -MAX_VEL)
          g_actuators.stepper_vel[i] = -MAX_VEL;
      }
      g_actuator_pub->publish(g_actuators);
    }
  }
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "control_jointspace");
  ros::NodeHandle n;
  ros::Subscriber target_sub = n.subscribe("target_joints", 1, target_cb);
  ros::Subscriber state_sub = n.subscribe("joint_states", 1, state_cb);
  ros::Publisher actuator_pub = n.advertise<openarms::ArmActuators>("arm_actuators", 1);
  ros::ServiceServer service = n.advertiseService("set_joint_target",
                                                  set_joint_target_srv);
  g_actuator_pub = &actuator_pub;
  g_actuators.stepper_vel.resize(4);
  g_actuators.servo_torque.resize(3);
  for (int i = 0; i < 4; i++)
    g_actuators.stepper_vel[i] = 0;

  ros::Rate loop_rate(100);
  while (ros::ok())
  {
    loop_rate.sleep();
    ros::spinOnce();
  }
}

