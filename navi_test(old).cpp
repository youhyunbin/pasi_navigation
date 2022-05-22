//주행
#include "moveToGoal.h"
#include <std_msgs/Int16.h>
#include <std_msgs/String.h>

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

//포트 번호
#define PORT 5001

//namespace
using namespace std;

bool goalReached = false;

//소켓통신 변수
int serv_sock, clnt_sock;
char message[1024];
int i, n;
size_t str_len;
size_t InLen = strlen((char *)message);
size_t OutLen = sizeof(message);
char * Input = (char *)message;
char * Output = message;

class NAV1{
private:
  //노드핸들 초기화
  ros::NodeHandle nh;
  //퍼블리셔
  ros::Publisher pub;
  ros::Publisher pub2;
  //서브스크라이버
  ros::Subscriber sub;
  ros::Subscriber sub2;
  ros::Subscriber sub3;
  //변수
  std_msgs::Int16 motor1;
  std_msgs::Int16 motor2;
  std_msgs::Int16 motor3;
  std_msgs::Int16 forward;
  std_msgs::Int16 backward;

public:
  NAV1(){
    //초음파 센서
    pub = nh.advertise<std_msgs::Int16>("ultra", 5);
    //모터
    pub2 = nh.advertise<std_msgs::Int16>("motor", 5);
    //각 명령 마치고 돌려받기
    sub = nh.subscribe<std_msgs::Int16>("fin", 5,  boost::bind(&NAV1::msgCallback, this, _1));
    sub2 = nh.subscribe<std_msgs::Int16>("fin2", 5, boost::bind(&NAV1::msg2Callback, this, _1));
    sub3 = nh.subscribe<std_msgs::Int16>("fin3", 5, boost::bind(&NAV1::msg3Callback, this, _1));
  }

  void move(){
    while(ros::ok()){
      ifstream fin("/home/hyun/navigation/src/navigation/src/socket.txt");
      if(!fin.eof()){
        int a;
        fin >> a;
        fin.close();
        if(a == 1){
           ROS_INFO("Moving to Shelf...");
           ROS_INFO("Shelf reached!");
           forward.data = 1;
           pub.publish(forward);
           a = 0;
           remove("/home/hyun/navigation/src/navigation/src/socket.txt");
           ofstream fout("/home/hyun/navigation/src/navigation/src/socket.txt", ios::trunc);
           fout.close();
           break;
        }
        else if(a == 9){//초기 위치
          ROS_INFO("Moving to Station...");
          ROS_INFO("Station reached!");
          remove("/home/hyun/navigation/src/navigation/src/socket.txt");
          ofstream fout("/home/hyun/navigation/src/navigation/src/socket.txt", ios::trunc);
          fout.close();
          break;
        }
      }
    }
  }

  //초음파 -> 모터
  void msgCallback(const std_msgs::Int16::ConstPtr& msg){
    if(msg->data == 2){
      ROS_INFO("Received");
      motor2.data = 102;
      pub2.publish(motor2);
    }
  }
  //모터 -> 초음파
  void msg2Callback(const std_msgs::Int16::ConstPtr& msg){
    if(msg->data == 3){
      ROS_INFO("Received");
      backward.data = 2;
      pub.publish(backward);
    }
  }
  //초음파 -> 포장장소
  void msg3Callback(const std_msgs::Int16::ConstPtr& msg){
    if(msg->data == 4){
      ROS_INFO("Moving to Packing Place");
      ROS_INFO("Packing Place Reached");
    }
  }
};

int main(int argc, char** argv){
  //노드 이름 설정
  ros::init(argc, argv, "test");
  ROS_INFO("Waiting...");

  ofstream fout("/home/hyun/navigation/src/navigation/src/socket.txt", ios::trunc);
  fout.close();

  //클래스 선언
  NAV1 nav1;

  //서버
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
  if(listen(serv_sock, 5)==-1)
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
      ofstream fout("/home/hyun/navigation/src/navigation/src/socket.txt", ios::trunc);
      fout << message;
      fout.close();
      nav1.move();
      close(clnt_sock);
  }
  //소켓 닫기
  iconv_close(iConv);

  ros::spin();
  return 0;
}
