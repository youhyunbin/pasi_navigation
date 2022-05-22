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
#include <iconv.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

//정의
#define PORT 5001

//namespace 설정
using namespace std;

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
  //소켓 변수
  int serv_sock, clnt_sock;
  char message[1024];
  int i, n;
  size_t str_len;
  size_t InLen = strlen((char *)message);
  size_t OutLen = sizeof(message);
  char * Input = (char *)message;
  char * Output = message;

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
    //서버 및 이동
    server();
  }

  void server(){
    //EUC-KR 문자열을 UTF8 문자열로 변환할 수 있도록 호출
    iconv_t iConv = iconv_open("UTF-8", "EUC-KR");

    //구조체
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;

    //TCP연결지향형이고 ipv4 도메인을 위한 소켓을 생성
    serv_sock=socket(PF_INET, SOCK_STREAM, 0);

    //주소를 초기화한 후 IP주소와 포트 지정
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(PORT);

    //소켓과 서버 주소를 바인딩
    if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1)
      printf("bind error");

    //연결 대기열 5개 생성
    if(listen(serv_sock, 5) ==-1)
      printf("listen error");

    //클라이언트로부터 요청이 오면 연결 수락
    clnt_addr_size = sizeof(clnt_addr);
    while(ros::ok()){
      memset(message, 0x00, sizeof(message));
      //변환 변수 n
      n = iconv(iConv, &Input, &InLen, &Output, &OutLen);
      clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
      printf("server connected!!\n");

      //연결이 성공적으로 되었으면 데이터 받기
      while ((str_len = read(clnt_sock, message, 1024)) != 0)
        write(clnt_sock, message, str_len);
        printf("Message: %s", message);
        //이동
        while(ros::ok()){
          //좌표 읽기
          ifstream fin;
          fin.open("floor");
          double number[16];
          for(i=0;i<16;i++){
            fin >> number[i];
          }

          //네비게이션
          char a[] = "1"; //선반
          char b[] = "9"; //초기
          if(message[0] == a[0]){
            ROS_INFO("Moving to Shelf...");
            goalReached = moveToGoal(number[0],number[1],number[2],number[3]);
            if(goalReached){
              ROS_INFO("Shelf reached!");
              forward.data = 1;
              pub.publish(forward);
              ros::spinOnce();
            }
          else printf("fail");
          }
          //초기 위치
          else if(message[0] == b[0]){
            ROS_INFO("Moving to Station...");
            goalReached = moveToGoal(number[8],number[9],number[10],number[11]);
            if(goalReached){
              ROS_INFO("Station reached!");
              ros::spinOnce();
            }
          }
        }
      close(clnt_sock);
    }

    //소켓들 닫기
    close(serv_sock);
    iconv_close(iConv);
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
  ros::spinOnce();
}
