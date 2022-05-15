//주행
#include "moveToGoal.h"
#include <std_msgs/Int16.h>

//파일 읽기
#include <iostream>
#include <fstream>

//소켓통신
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

//정의
#define PORT 5001

//namespace 설정
using namespace std;

char choose();

bool goalReached = false;

class NAV1{
private:
  //노드핸들 초기화
  ros::NodeHandle nh;
  //퍼블리셔
  ros::Publisher pub;
  ros::Publisher pub2;
  ros::Publisher pub3;
  ros::Publisher pub4;
  //서브스크라이버
  ros::Subscriber sub;
  ros::Subscriber sub2;
  ros::Subscriber sub3;
  //변수
  std::string name;
  std_msgs::Int16 forward;
  std_msgs::Int16 backward;
  std_msgs::Int16 motor1;
  std_msgs::Int16 motor2;
  std_msgs::Int16 motor3;
  //좌표 변수
  int i;

public:
  NAV1(const std::string &name_in = "nav1"): name(name_in){
    //초음파 센서
    pub = nh.advertise<std_msgs::Int16>("/Ultra", 100);
    //1층 모터
    pub2 = nh.advertise<std_msgs::Int16>("/motor1", 100);
    //2층 모터
    pub3 = nh.advertise<std_msgs::Int16>("/motor2", 100);
    //3층 모터
    pub4 = nh.advertise<std_msgs::Int16>("/motor3", 100);
    //초음파 명령 마치고 돌려받기
    sub = nh.subscribe<std_msgs::Int16>("/fin", 1000, boost::bind(&NAV1::ultra_to_motor, this, _1));
    sub2 = nh.subscribe<std_msgs::Int16>("/fin", 1000, boost::bind(&NAV1::ultra_to_pack, this, _1));
    //모터 구동 마치고 돌려받기
    sub3 = nh.subscribe<std_msgs::Int16>("/fin2", 1000, boost::bind(&NAV1::motor_to_ultra, this, _1));
    //이동
    move();
  }

  void move(){
    char choice = 'q';
    while(ros::ok()){
      //좌표 읽기
      ifstream fin;
      fin.open("floor");
      double number[16];
      for(i=0;i<16;i++){
        fin >> number[i];
      }

      choice = choose();
      //선반
      if(choice == '1'){
        ROS_INFO("Moving to Shelf...");
        goalReached = moveToGoal(number[0],number[1],number[2],number[3]);
        if(goalReached){
          ROS_INFO("Shelf reached!");
          forward.data = 1;
          pub.publish(forward);
          ros::spinOnce();
        }
      }
      //초기 위치
      if(choice == '9'){
        ROS_INFO("Moving to Station...");
        goalReached = moveToGoal(number[8],number[9],number[10],number[11]);
        if(goalReached){
          ROS_INFO("Station reached!");
          ros::spinOnce();
        }
      }
    }
  }

  //초음파 센서 전진 확인 후 모터 구동 명령
  void ultra_to_motor(const std_msgs::Int16::ConstPtr& msg){
    if(msg->data == 11){
      motor2.data = 102;
      pub2.publish(motor2);
    }
  }

  //초음파 센서 후진 확인 후 포장 장소 이동
  void ultra_to_pack(const std_msgs::Int16::ConstPtr& msg){
    while(ros::ok()){
      ifstream fin;
      fin.open("floor");
      double number[16];
      for(i=0;i<16;i++){
        fin >> number[i];
      }
      //포장 장소
      if(msg->data == 12){
        ROS_INFO("Moving to Packing Place");
        goalReached = moveToGoal(number[4],number[5],number[6],number[7]);
        if(goalReached){
          ROS_INFO("Packing Place Reached");
          ros::spinOnce();
        }
      }
    }
  }
  //모터 구동 마치고 초음파 센서 작동
  void motor_to_ultra(const std_msgs::Int16::ConstPtr& msg){
    if(msg->data == 11){
      backward.data = 2;
      pub.publish(backward);
      ros::spinOnce();
    }
  }
};

int main(int argc, char** argv){
  //노드 이름 설정
  ros::init(argc, argv, "move");
  ROS_INFO("Waiting...");
  //클래스 선언
  NAV1 nav1;
}

char choose(){
  char choice='q';
  std::cout<<"|-------------------------------|"<<std::endl;
  std::cout<<"|PRESSE A KEY:"<<std::endl;
  std::cout<<"|'0': Cafe "<<std::endl;
  std::cout<<"|'1': Office 1 "<<std::endl;
  std::cout<<"|'2': Office 2 "<<std::endl;
  std::cout<<"|'3': Office 3 "<<std::endl;
  std::cout<<"|'q': Quit "<<std::endl;
  std::cout<<"|-------------------------------|"<<std::endl;
  std::cout<<"|WHERE TO GO?";
  std::cin>>choice;
  return choice;
}
