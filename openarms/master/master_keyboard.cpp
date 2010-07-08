// copyright 2010 morgan quigley bsd license blah blah
// lots of code lifted from the teleop_head_keyboard program by Kevin Watts
#include <cstdlib>
#include <cstdio>
#include <termios.h>

#include <ros/ros.h>
#include "geometry_msgs/Transform.h"
#include "LinearMath/btTransform.h"
#include "tf/transform_datatypes.h"

struct termios cooked, raw; // mmm makes me hungry

int main(int argc, char **argv)
{
  ros::init(argc, argv, "master_keyboard"); // remember he-man?
  ros::NodeHandle n;
  ros::Publisher tf_pub = n.advertise<geometry_msgs::Transform>("target_frame", 1);
  char c;
  tcgetattr(0, &cooked);
  memcpy(&raw, &cooked, sizeof(struct termios));
  raw.c_lflag &= ~(ICANON | ECHO);
  raw.c_cc[VTIME] = 1;
  raw.c_cc[VMIN] = 0;
  tcsetattr(0, TCSANOW, &raw);
  puts("greetings. ctrl-c to exit.");
  //tf::Transform t(btQuaternion::getIdentity(), btVector3(0, 0, 0));
  btVector3 target_vec(0, 0, 0);
  geometry_msgs::Transform tf_msg;

  const double DELTA = 0.01;
  
  while (ros::ok())
  {
    ssize_t nread = read(0, &c, 1);
    if (nread < 0)
    {
      perror("read():");
      exit(-1);
    }
    else if (nread == 0)
      c = 0;
    switch (c)
    {
      case 'a':
        //t.pose.setX(1);
        target_vec.setX(target_vec.x() - DELTA);
        break;
      case 'x':
        target_vec.setY(target_vec.y() - DELTA);
        break;
      case 'w':
        target_vec.setY(target_vec.y() + DELTA);
        break;
      case 'd':
        target_vec.setX(target_vec.x() + DELTA);
        break;
      case 'q':
        target_vec.setZ(target_vec.z() + DELTA);
        break;
      case 'z':
        target_vec.setZ(target_vec.z() - DELTA);
        break;
      default:
        break;
    }
    tf::transformTFToMsg(tf::Transform(btQuaternion::getIdentity(), target_vec),
                         tf_msg);
    tf_pub.publish(tf_msg);
  }
  tcsetattr(0, TCSADRAIN, &cooked);
  printf("bai\n");
  fflush(0);
  return 0;
}
