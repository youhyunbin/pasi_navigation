//Navigation
#include "moveToGoal.h"
#include <std_msgs/Int16.h>

//File read & write
#include <iostream>
#include <fstream>

using namespace std;

bool goalReached = false;

class Navsock
{
public:
  Navsock()
  {
    pubUltraSonic = nh.advertise<std_msgs::Int16>("/ultra", 1);

    Move();
  }

  void Move()
  {
    while(ros::ok()){
      ifstream floors("/home/hyun/navigation/src/navigation/src/floor.txt");
      for(i=0;i<16;i++){
        floors >> number[i];
      }

      ifstream socket("/home/hyun/navigation/src/navigation/src/socket.txt");
      if(!socket.eof())
      {
        socket >> check;
        socket.close();
        if(check == 1)
        {
          ROS_INFO("Moving to Shelf...");
          goalReached = moveToGoal(number[0],number[1],number[2],number[3]);
          if(goalReached){
            ROS_INFO("Shelf reached!");
            Forward.data = 1;
            pubUltraSonic.publish(Forward);
            check = 0;
            remove("/home/hyun/navigation/src/navigation/src/socket.txt");
            ofstream fout("/home/hyun/navigation/src/navigation/src/socket.txt", ios::trunc);
            fout.close();
          }
        }

        else if(check == 9)
        {
          ROS_INFO("Moving to Station...");
          goalReached = moveToGoal(number[8],number[9],number[10],number[11]);
          if(goalReached){
            ROS_INFO("Station reached!");
            check = 0;
            remove("/home/hyun/navigation/src/navigation/src/socket.txt");
            ofstream fout("/home/hyun/navigation/src/navigation/src/socket.txt", ios::trunc);
            fout.close();
          }
        }
      }
    }
  }

private:
  ros::NodeHandle nh;

  //Publisher
  ros::Publisher pubUltraSonic;

  std_msgs::Int16 Forward;

  int i, j;
  double number[16];
  int check;
};


int main(int argc, char **argv)
{
  //Initiate ROS
  ros::init(argc, argv, "nav_socket");

  ROS_INFO("Navigation_Socket connected");

  ofstream fout("/home/hyun/navigation/src/navigation/src/socket.txt", ios::trunc);
  fout.close();

  //Create an object of class Navsock
  Navsock navsock;

  ros::spin();

  return 0;
}
