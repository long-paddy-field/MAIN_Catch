asm(".global _printf_float");

#include"mbed.h"
#include"controller.h"


CAN can(PB_8,PB_9,500000);//CAN通信
DigitalOut S_valve(PA_7);//電磁弁
InterruptIn startUp(PB_13);//お父さんスイッチ1
InterruptIn Shift_Location(PB_14);//お父さんスイッチ3
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
bool btnstopper = false;

int main()
{
  printf("Program started\n");
  startUp.fall(callback(&phaze_admin));
  CANMessage rec_msg1(0x2,CANStandard);
  Shift_Location.disable_irq();

  while(1)
  {
    switch(phaze_counter)
    {
      case 0:
      break;

      case 1:
      if(can.read(rec_msg1))
      {
        printf("own task1 ended\n");
        btnstopper = false;
      }
      break;

      case 2:
      
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
    time_counter;
    wait_us(100000);
  }

  
  return 0;
}

void phaze_admin()
{
  if(time_counter>5&& !btnstopper)
  {
    printf("phaze%d ended\n",phaze_counter);
    phaze_counter++;
    printf("phaze%d start\n",phaze_counter);
    time_counter = 0;
  }
  switch(phaze_counter)
  {
    case 1:
    phaze1();
    break;
    
    case 2:
    phaze2();
    break;
    
    case 3:
    break;
    
    case 4:
    break;

    default:
    printf("error!\n");
    break;
  }
}

void phaze0()
{

}

void phaze1()
{
    unsigned char data = 0;
    CANMessage msg1(0x1,&data);
    can.write(msg1);
    GuideDown();
    btnstopper=true;
}

void phaze2()
{
  Shift_Location.enable_irq();
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