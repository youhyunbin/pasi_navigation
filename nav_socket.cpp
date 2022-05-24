#include <fstream>
#include <iostream>

#include "moveToGoal.h"
#include <std_msgs/Int16.h>

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
      ifstream order("/home/hyun/navigation/src/navigation/src/order.txt");

      int j;
      int list[16];
      int count = 0; int count1 = 0; int count2 = 0;
      int k=0; int n=0; int t=0;

      fill_n(list,16,0); // Initiate list to "0"

      for (j=0;j<16;j++)
      {
        order >> list[j];
        if(list[j] == 2) count++;
        if(list[j] == 3) count1++;
        if(list[j] == 4) count2++;
      }

      if(count != 0){
        while(k < count){
          First.data = 101;
          pubMotor.publish(First);
          sleep(5);
          k++;
        }
      }

      if(count1 != 0){
        while(n < count1){
          Second.data = 102;
          pubMotor.publish(Second);
          sleep(5);
          n++;
        }
      }

      if(count2 != 0){
        while(t < count2){
          Third.data = 103;
          pubMotor.publish(Third);
          sleep(5);
          t++;
        }
      }
      order.close();
    }
  }

  void CheckArrivalMotor(const std_msgs::Int16 rcvMotorResult)
  {
    ROS_INFO("Received : %d", rcvMotorResult.data);
    if(rcvMotorResult.data == 3)
    {
      Backward.data = 2;
      pubUltraSonic.publish(Backward);
    }
  }

  void ArrivalUltraSonicBackward(const std_msgs::Int16 rcvBackwardResult)
  {
    ROS_INFO("Received : %d", rcvBackwardResult.data);
    if(rcvBackwardResult.data == 4)
    {
      ifstream floors("/home/hyun/navigation/src/navigation/src/floor.txt");

      int i;
      double number[16];

      for(i=0;i<16;i++)
      {
        floors >> number[i];
      }

      ROS_INFO("Moving to Packing Place");
      goalReached = moveToGoal(number[4],number[5],number[6],number[7]);
      if(goalReached){
        ROS_INFO("Packing Place Reached");
      }
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
};


int main(int argc, char** argv){
  //Initiate ROS
  ros::init(argc, argv, "nav_callback");

  ROS_INFO("Navigation_Callback connected");

  //Create an object of class Navcall
  NavCall navCall;

  ros::spin();

  return 0;
}
