#include "ros/ros.h"
#include "std_msgs/Int16.h"
#include "std_msgs/Float32.h"
#include <nav_msgs/Odometry.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/TransformStamped.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_ros/transform_broadcaster.h>
#include <cmath>

ros::Publisher odom_pub;

double distanceLeft;
const double TICKS_PER_METER = 333;
int leftTicks = 0;

ros::Time prev_update_time_ros = ros::Time::now();
unsigned long prev_update_time = prev_update_time_ros.toSec();
ros::Time stamp_now;

#define TICK2RAD 0.001533981

nav_msgs::Odometry odom;

float odom_pose[3];
double odom_vel[3];

void Calc_distance(const std_msgs::Float32& leftCount) {

  static int lastCountL = 0;
  if(leftCount.data != 0 && lastCountL != 0) {

    leftTicks = (leftCount.data - lastCountL);

    if (leftTicks > 10000) {
      leftTicks = 0 - (65535 - leftTicks);
    }
    else if (leftTicks < -10000) {
      leftTicks = 65535-leftTicks;
    }
    else{}
    distanceLeft = leftTicks/TICKS_PER_METER;
  }
  lastCountL = leftCount.data;
}



void initOdom(void)
{
    for (int i=0; i < 3; i++)
    {
        odom_pose[i] = 0.0;
        odom_vel[i] = 0.0;
    }
    odom.header.frame_id = "odom";
    odom.pose.pose.position.x = 0.0;
    odom.pose.pose.position.y = 0.0;
    odom.pose.pose.position.z = 0.0;

    odom.pose.pose.orientation.x = 0.0;
    odom.pose.pose.orientation.y = 0.0;
    odom.pose.pose.orientation.z = 0.0;
    odom.pose.pose.orientation.w = 0.0;

    odom.twist.twist.linear.x = 0.0;
    odom.twist.twist.angular.z = 0.0;
}

bool calcOdometry(double diff_time)
{
    float orientation[4];
    double wheel_l, wheel_r;
    double delta_s, theta, delta_theta;
    static double last_theta = 0.0;
    double v, w;
    double step_time2;

    wheel_l = wheel_r = 0.0;
    delta_s = delta_theta = theta = 0.0;
    v = w = 0.0;
    step_time2 = diff_time;

    if (step_time2 == 0) return false;

    wheel_l = TICK2RAD * leftTicks;
    
    orientation[0] = 0.0;
    orientation[1] = 0.0;
    orientation[2] = 0.0;
    orientation[3] = 1.0;

    theta = atan2f(orientation[1]*orientation[2] + orientation[0]*orientation[3],
                   0.5f - orientation[2]*orientation[2] - orientation[3]*orientation[3]);

    delta_theta = theta - last_theta;
    
    odom_pose[0] += wheel_l * cos(odom_pose[2] + (delta_theta / 2.0));
    odom_pose[1] += wheel_l * sin(odom_pose[2] + (delta_theta / 2.0));
    odom_pose[2] += delta_theta;

    v = wheel_l / step_time2;
    w = delta_theta / step_time2;

    odom_vel[0] = v;
    odom_vel[1] = 0.0;
    odom_vel[2] = w;

    last_theta = theta;

    return true;
}





void publishDriveInformation(void)
{
    ros::Time time_now_ros = ros::Time::now();
    double time_now = time_now_ros.toSec();

    double step_time = time_now - prev_update_time;

    prev_update_time = time_now;

    stamp_now = time_now_ros;

    calcOdometry((double)(step_time * 0.001));
  
}



void updateOdometry(void)
{
    odom.header.frame_id = "odom";
    odom.child_frame_id = "base_footprint";

    odom.pose.pose.position.x = odom_pose[0];
    odom.pose.pose.position.y = odom_pose[1];
    odom.pose.pose.position.z = 0;
    
    tf2::Quaternion quaternion;
    quaternion.setRPY(0, 0, odom_pose[2]);
    odom.pose.pose.orientation.x = quaternion.x();
    odom.pose.pose.orientation.y = quaternion.y();
    odom.pose.pose.orientation.z = quaternion.z();
    odom.pose.pose.orientation.w = quaternion.w();

    odom.twist.twist.linear.x = odom_vel[0];
    odom.twist.twist.linear.y = 0;
    odom.twist.twist.linear.z = 0;
    odom.twist.twist.angular.x = 0;
    odom.twist.twist.angular.y = 0;
    odom.twist.twist.angular.z = odom_vel[2];
    odom_pub.publish(odom);
}


tf2_ros::TransformBroadcaster tf_broadcaster;
geometry_msgs::TransformStamped odom_tf;

void updateTF()
{
    odom_tf.header.frame_id = "odom";
    odom_tf.child_frame_id = "base_footprint";
    odom_tf.transform.translation.x = odom.pose.pose.position.x;
    odom_tf.transform.translation.y = odom.pose.pose.position.y;
    odom_tf.transform.translation.z = odom.pose.pose.position.z;
    odom_tf.transform.rotation = odom.pose.pose.orientation;


}


void set_initial_2d(const geometry_msgs::PoseStamped &rvizClick) {
 
  odom.pose.pose.position.x = rvizClick.pose.position.x;
  odom.pose.pose.position.y = rvizClick.pose.position.y;
  odom.pose.pose.orientation.z = rvizClick.pose.orientation.z;

}

int main(int argc, char **argv) {
    ros::init(argc, argv, "odom_pub_22_node");
    ros::NodeHandle node;

    odom_pub = node.advertise<nav_msgs::Odometry>("odom", 1);

    ros::Subscriber encoder_sub = node.subscribe("encoder", 1, Calc_distance);
    ros::Subscriber subInitialPose = node.subscribe("initial_2d", 1, set_initial_2d);

    initOdom();
   

    ros::Rate loop_rate(30); 

    while(ros::ok())
    {
        publishDriveInformation();


        updateOdometry();
        odom.header.stamp = ros::Time::now();


        updateTF();
        odom_tf.header.stamp = ros::Time::now();
        tf_broadcaster.sendTransform(odom_tf);

        loop_rate.sleep();
    }
    
    ros::spin();

    return 0;
}
