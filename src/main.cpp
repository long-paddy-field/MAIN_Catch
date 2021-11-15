asm(".global _printf_float");

#include"mbed.h"
#include"controller.h"


CAN can(PB_8,PB_9,500000);//CAN通信
DigitalOut S_valve(PA_7);//電磁弁
InterruptIn startUp(PB_13);//お父さんスイッチ1
InterruptIn Shift_Location(PB_14);//お父さんスイッチ3
InterruptIn Conveyor(PB_15);
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
void GuideUp();//ガイド上げる
void GuideDown();//ガイド下げる
void SetUp_Location();//開始位置を指定
void Denjiben(bool);//電磁弁の開閉
void con_awake();//コンベアの起動

int phaze_counter = 0;
int time_counter1 = 0;//お父さんスイッチ1用
int time_counter2 = 0;//お父さんスイッチ2用
int time_counter3 = 0;//お父さんスイッチ3用
int time_counter4 = 0;//手首サーボ用
int Own_Location = 0;//自分のエリアの初期位置
bool btnstopper = false;

unsigned char data1;
unsigned char data4;
unsigned char data5;
unsigned char data7;
unsigned char data8;

CANMessage msg1(0x1,&data1);
CANMessage msg2(0x2,CANStandard);
CANMessage msg3(0x3,CANStandard);
CANMessage msg4(0x4,&data4);
CANMessage msg5(0x5,&data5);
CANMessage msg6(0x6,CANStandard);
CANMessage msg7(0x7,&data7);
CANMessage msg8(0x8,&data8);
CANMessage rec_msg;



int main()
{
  printf("Program started\n");
  startUp.fall(callback(&phaze_admin));
  Shift_Location.fall(callback(&SetUp_Location));
  Conveyor.fall(callback(&con_awake));
  Conveyor.disable_irq();
  Shift_Location.disable_irq();
  C_hand.period_ms(20);
  C_wrist.period_ms(20);
  Guide1.period_ms(20);
  Guide2.period_ms(20);

  while(1)
  {
    switch(phaze_counter)
    {
      case 0:
      break;

      case 1:
      if(can.read(msg2))
      {
        printf("own task1 ended\n");
        startUp.enable_irq();
      }
      break;

      case 2:
      if(can.read(msg2)&&msg2.data[0] > 0)
      {
        Shift_Location.enable_irq();
      }
      break;

      case 3:
      if(can.read(msg6)&&msg6.data[0] == 0)
      {
        Denjiben(msg6.data[0]);
      }else if(can.read(msg6)&&msg6.data[0] == 1)
      {
        Denjiben(msg6.data[0]);
      }
      break;

      case 4:
      
      break;

      default:
      printf("error!\n");
    }
    time_counter1++;
    time_counter2++;
    time_counter3++;
    time_counter4++;
    wait_us(100000);
  }
  return 0;
}

void phaze_admin()
{
  if(time_counter1>5)
  {
    printf("phaze%d ended\n",phaze_counter);
    phaze_counter++;
    printf("phaze%d start\n",phaze_counter);
    time_counter1 = 0;
  }
  //一回性の動作はここでやる
  switch(phaze_counter)
  {
    case 1:
    phaze1();//スタートを通知、ガイドを下す、位置設定
    break;
    
    case 2:
    phaze2();//シフトボタン有効化
    break;
    
    case 3:
    phaze3();//稼働を通知
    break;
    
    case 4:
    phaze4();//終了通知、ガイドを上げる
    break;

    case 5:
    phaze_counter = 0;
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
    can.write(msg1);
    GuideDown();
    startUp.disable_irq();
}

void phaze2()
{
  Shift_Location.enable_irq();
}

void phaze3()
{
  Conveyor.enable_irq();
  can.write(msg5);
}

void phaze4()
{
  can.write(msg8);
  Conveyor.disable_irq();
  GuideUp();
}

void C_Hand_Grip()
{
  C_hand.pulsewidth_us(550);
}

void C_Hand_Release()
{
  C_hand.pulsewidth_us(1500);
}

void C_Wrist_CW()
{
  C_wrist.pulsewidth_us(time_counter4);
}

void C_Wrist_CCW()
{
  C_wrist.pulsewidth_us(time_counter4);
}

void GuideUp()
{
  Guide1.pulsewidth(1500);
  Guide2.pulsewidth(1500);
}

void GuideDown()
{
  Guide1.pulsewidth(550);
  Guide2.pulsewidth(2400);
}

void SetUp_Location()
{
  if(time_counter2 > 5)
  {
    Own_Location++;
    time_counter2 = 0;
    Shift_Location.disable_irq();
  }
}

void Denjiben(bool Denji_state)
{
  S_valve = Denji_state;
}

void con_awake()
{
  if(time_counter3 > 5)
  {
    data7++;
    data7 %= 2;
    can.write(msg7);
  }
}