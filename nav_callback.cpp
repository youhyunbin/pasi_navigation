//Navigation
#include "moveToGoal.h"
#include <std_msgs/Int16.h>

//File read & write
#include <iostream>
#include <fstream>

using namespace std;

bool goalReached = false;

class NavCall
{
public:
  NavCall()
  {
    pubUltraSonic = nh.advertise<std_msgs::Int16>("/ultra", 1);
    pubMotor      = nh.advertise<std_msgs::Int16>("/motor", 1);

    subArrivalUltraSonicForward  = nh.subscribe("/ultra_forward_arrival", 1, &NavCall::CheckArrivalUltraSonicForward, this);
    subArrivalMotor              = nh.subscribe("/motor_arrival", 1, &NavCall::CheckArrivalMotor, this);
    subArrivalUltraSonicBackward = nh.subscribe("/ultra_backward_arrival", 1, &NavCall::ArrivalUltraSonicBackward, this);
  }

  void CheckArrivalUltraSonicForward(const std_msgs::Int16 rcvForwardResult)
  {
    ROS_INFO("Received : %d", rcvForwardResult.data);
    if(rcvForwardResult.data == 2)
    {
      Second.data = 102;
      pubMotor.publish(Second);
    }
  }

  void CheckArrivalMotor(const std_msgs::Int16 rcvMotorResult)
  {
    ROS_INFO("Received : %d", rcvMotorResult.data);
    if(rcvMotorResult.data == 3)
    {
      Backward.data = 2;
      pubUltraSonic2.publish(Backward);
    }
    else {
      ROS_INFO("Failed");
    }
  }

  void ArrivalUltraSonicBackward(const std_msgs::Int16 rcvBackwardResult)
  {
    ROS_INFO("Received : %d", rcvBackwardResult.data);
    if(rcvBackwardResult.data == 4)
    {
      ROS_INFO("Moving to Packing Place");
      ROS_INFO("Packing Place Reached");
    }
    else {
      ROS_INFO("Failed");
    }
  }

private:
  ros::NodeHandle nh;

  //Publisher
  ros::Publisher pubUltraSonic;
  ros::Publisher pubMotor;

  //Subscriber
  ros::Subscriber subArrivalUltraSonicForward;
  ros::Subscriber subArrivalMotor;
  ros::Subscriber subArrivalUltraSonicBackward;

  //msgs
  std_msgs::Int16 Forward;
  std_msgs::Int16 Backward;
  std_msgs::Int16 First;
  std_msgs::Int16 Second;
  std_msgs::Int16 Third;

  int list[100];
  double number[16];
};

int main(int argc, char** argv){
  //Initiate ROS
  ros::init(argc, argv, "nav_callback");
  ROS_INFO("Navigation_Callback connected");

  //Create an object of class NavCore
  NavCall navCall;

  ros::spin();
  return 0;
}
