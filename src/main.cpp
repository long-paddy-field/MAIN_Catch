asm(".global _printf_float");

#include"mbed.h"
#include"controller.h"
#include"DENJIBEN.h"

RawCAN can(PA_11,PA_12,500000);//CAN通信
Controller controller(can,0x334);

InterruptIn Btn_start(PB_13);//起動スイッチ
InterruptIn Btn_pause(PB_14);//中断スイッチ
InterruptIn Btn_owari(PB_15);//終了スイッチ

DigitalOut valve(PA_7);//電磁弁
PwmOut Guide1(PB_10);//ガイド1
PwmOut Guide2(PB_2);//ガイド2
Denjiben DJ(valve);

void start_protocol();
void pause_protocol();
void owari_protocol();
int phaze = 0;//0:待機 1:稼働 2:終了
int prev_phaze = 0;
bool admin = true;//pauseが押されたらfalse

void GuideUp();//ガイド上げる
void GuideDown();//ガイド下げる

int time_counter1 = 0;//お父さんスイッチ1用
int time_counter2 = 0;//お父さんスイッチ2用
int time_counter3 = 0;//お父さんスイッチ3用
int time_counter4 = 0;//ガイドのサーボ用
int prev_z = 0;//ちょっと前のz軸値を保持

const int start_s1 = 0;//0度 
const int terminal_s1 = 0;//180度
int Guide_s1 = (start_s1 + terminal_s1)/2;//90度
const int start_s2 = 0;//0度
const int terminal_s2 = 0;//180度
int Guide_s2 = (start_s2 + terminal_s2)/2;//90度

unsigned char data1 = 0;
unsigned char data3 = 0;
unsigned char data4 = 0;

CANMessage msg1(0x1,&data1);
CANMessage msg2(0x2,CANStandard);
CANMessage msg3(0x3,&data3);
CANMessage msg4(0x4,&data4);

int main()
{
  printf("program start\n");
  Guide1.period_ms(20);
  Guide2.period_ms(20);
  Btn_start.rise(callback(&start_protocol));
  Btn_pause.rise(callback(&pause_protocol));
  Btn_owari.rise(callback(&owari_protocol));
/*
  while(1)
  {
    printf("%d%d%d%d%d%d%d%d\n",controller.buttons[0],controller.buttons[1],controller.buttons[2],controller.buttons[3],controller.buttons[4],controller.buttons[5],controller.buttons[6],controller.buttons[7]);
    wait_us(100000);
  }
  */
  while(1)
  {
    switch(phaze){
      case 0:
      //待機
      printf("now waiting\n");
      break;
      case 1:
      //本稼働
      if(prev_phaze == 0)
      {
        printf("now moving\n");
        can.write(msg1);
      }
      if(can.read(msg2))
      {
        if(msg2.data[0]==0)
        {
          printf("grab BISCO\n");
          DJ.open();        
        }else if(msg2.data[0]==1)
        {
          printf("release BISCO\n");
          DJ.close();
        }
      }
      if(admin)
      {
        data3 = admin ? 1 : 0;
        can.write(msg3);
      }
      if(controller.axes.z == 100)
      {

        if(controller.axes.z != prev_z)
        {
          time_counter4 = 0;
        }
        GuideUp();
      }else if(controller.axes.z == -100)
      {
        if(controller.axes.z != prev_z)
        {
          time_counter4 = 0;
        }
        GuideDown();
      }
      break;
      case 2:
      if(prev_phaze == 1)
      {
        printf("end program\n");
        can.write(msg4);
        Guide1.pulsewidth((start_s1+terminal_s1)/2);
        Guide2.pulsewidth((start_s2+terminal_s2)/2);
      }
      //終了
      break;
    }
    time_counter1++;
    time_counter2++;
    time_counter3++;
    time_counter4++;
    prev_z = controller.axes.z;
    prev_phaze = phaze;
    wait_us(100000);
  }
  return 0;
}

void start_protocol()
{
  phaze = 1;
}

void pause_protocol()
{
  if(time_counter2 > 5)
  {
    admin = !admin;
    time_counter2 = 0;
  }
}
void owari_protocol()
{
  phaze = 2;
}

void GuideUp()
{
  if(Guide_s1 <= terminal_s1 && Guide_s2 >= start_s2)
  {
  Guide_s1 = Guide_s1 + time_counter4*30;
  Guide_s2 = Guide_s2 - time_counter4*30;
  Guide1.pulsewidth(Guide_s1);
  Guide2.pulsewidth(Guide_s2);
  }
}

void GuideDown()
{
  if(Guide_s1 >= start_s1 && Guide_s2 <= terminal_s2)
  {
  Guide_s1 = Guide_s1 - time_counter4*30; 
  Guide_s2 = Guide_s2 + time_counter4*30;
  Guide1.pulsewidth(Guide_s1);
  Guide2.pulsewidth(Guide_s2);
  }
}