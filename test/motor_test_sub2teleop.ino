#include <ros.h>
#include <std_msgs/Int32.h>
#include <geometry_msgs/Twist.h>

ros::NodeHandle nh;

geometry_msgs::Twist msg;

int PWM1 = 2;
int Direction1 = 52;
int Brake1 = 53;

int PWM2 = 5;
int Direction2 = 50;
int Brake2 = 51;

float velocity;
float steer_angle;

void cmd_vel_callback(const geometry_msgs::Twist& cmd_vel)
{
  velocity = cmd_vel.linear.x;
  steer_angle = cmd_vel.angular.z;

  if(velocity >= 100) velocity  = 100;  // pwm
  if(velocity <= -100) velocity = -100;  // pwm
}

ros::Subscriber<geometry_msgs::Twist> teleop_sub("cmd_vel", &cmd_vel_callback);

void setup()
{
  nh.initNode();
  nh.subscribe(teleop_sub);
  
  pinMode(PWM1, OUTPUT);
  pinMode(PWM2, OUTPUT);
  
  pinMode(Direction1, OUTPUT);
  pinMode(Direction2, OUTPUT);
  
  pinMode(Brake1, OUTPUT);
  pinMode(Brake2, OUTPUT);

}

void loop()
{
  analogWrite(PWM1, velocity);
  analogWrite(PWM2, velocity);
  
  digitalWrite(Direction1, LOW);
  digitalWrite(Direction2, LOW);
  
  
  nh.spinOnce();
  
  delay(1000);
  
}