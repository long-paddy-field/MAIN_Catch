asm(".global _printf_float");

#include"mbed.h"
#include"controller.h"


CAN can(PB_8,PB_9,500000);//CAN通信
DigitalOut S_valve(PA_7);//電磁弁
InterruptIn startUp(PB_13);//お父さんスイッチ1
DigitalIn Shift_Location(PB_14);//お父さんスイッチ3
DigitalIn Conveyor(PB_15);
PwmOut C_hand(PB_5);//把持のサーボ
PwmOut C_wrist(PB_4);//手首のサーボ
PwmOut Guide1(PB_10);//ガイド1
PwmOut Guide2(PB_2);//ガイド2
Controller controller(can,0x334);//コントローラのコンストラクタ

void phaze_admin();//割り込みでphazecounterを上げてく
void phaze0();//スタートボタン押されるまで待機
void phaze1();//起動後プロトコル→待機
void phaze2();//初期位置設定モード
void phaze3();//稼働開始
void phaze4();//終了処理

void C_Hand_Grip();//掴む
void C_Hand_Release();//放す
void C_Wrist_CW();//時計回り
void C_Wrist_CCW();//反時計回り
void GuideUP();//ガイド上げる
void GuideDown();//ガイド下げる

int phaze_counter = 0;
int time_counter = 0;

int main()
{
  printf("Program started\n");
  startUp.fall(callback(&phaze_admin));
  while(1)
  {
    switch(phaze_counter)
    {
      case 0:
      phaze0();
      break;

      case 1:
      phaze1();
      break;

      case 2:
      phaze2();
      break;

      case 3:
      phaze3();
      break;

      case 4:
      phaze4();
      break;

      default:
      printf("error!\n");
    }
    wait_us(100000);
  }

  
  return 0;
}

void phaze_admin()
{

}

void phaze0()
{

}

void phaze1()
{

}

void phaze2()
{

}

void phaze3()
{

}

void phaze4()
{

}

void C_Hand_Grip()
{

}

void C_Hand_Release()
{

}

void C_Wrist_CW()
{

}

void C_Wrist_CCW()
{

}

void GuideUP()
{

}

void GuideDown()
{

}