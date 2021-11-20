asm(".global _printf_float");

#include"mbed.h"
#include"controller.h"
#include"DENJIBEN.h"

CAN can(PA_11,PA_12,500000);//CAN通信
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
  Guide1.pulsewidth(Guide_s1);
  Guide2.pulsewidth(Guide_s2);
  Guide_s1 = Guide_s1 + time_counter4*30;
  Guide_s2 = Guide_s2 - time_counter4*30;
  }
}

void GuideDown()
{
  if(Guide_s1 >= start_s1 && Guide_s2 <= terminal_s2)
  {
  Guide1.pulsewidth(Guide_s1);
  Guide2.pulsewidth(Guide_s2);
  Guide_s1 = Guide_s1 - time_counter4*30; 
  Guide_s2 = Guide_s2 + time_counter4*30;
  }
}

/*
#include"mbed.h"
#include"controller.h"
#include"DENJIBEN.h"

//先に宣言
RawCAN can(PA_11,PA_12,500000);//CAN通信
Controller controller(can,0x334);//コントローラのコンストラクタ
InterruptIn MainSetting(PB_13);//お父さんスイッチ1
InterruptIn Shift_Location(PB_14);//お父さんスイッチ2
InterruptIn Conveyor(PB_15);//お父さんスイッチ3
DigitalOut S_valve(PA_7);//電磁弁
PwmOut C_hand(PB_5);//把持のサーボ
PwmOut C_wrist(PB_4);//手首のサーボ
PwmOut Guide1(PB_10);//ガイド1
PwmOut Guide2(PB_2);//ガイド2
Denjiben DJ(S_valve);

～phazeについて～
phaze 0:スタートボタンが押されるまで待機
phaze 1:PIの初期設定
phaze 2:スタート位置を決定
phaze 3:稼働開始
phaze 4:終了処理


void phaze0();//スタートボタン押されるまで待機
void phaze1();//起動後プロトコル→待機
void phaze2();//初期位置設定モード
void phaze3();//稼働開始
void phaze4();//終了処理

//以下は割り込みで呼び出す奴
void phaze_admin();//割り込みでphazecounterを上げてく
void SetUp_Location();//割り込みで開始位置を指定
void con_awake();//割り込みでコンベアの起動フラグを立てる

//以下は普通に呼び出す奴
void C_Hand_Grip();//掴む
void C_Hand_Release();//放す
void C_Wrist_CW();//時計回り
void C_Wrist_CCW();//反時計回り


int phaze_counter = 0;//phazeを記憶
int phaze_prev = 0;//0.1秒前のphazeを記憶

int time_counter4 = 0;//サーボ用
int time_counter5 = 0;//自陣の停止/再始動用
int Own_Location = 0;//自分のエリアの初期位置
int Guide_Location = 1500;//Guideのパルス幅を保持
int wrist_Location = 0;//手首の現在位置を保持
bool con_flag = false;//trueで動かす　falseで止める（コンベア）

//CAN送信用空データ
//CANMessageには、idとdataがある。

unsigned char data1 = 0;
unsigned char data4 = 0;
unsigned char data5 = 0;
unsigned char data7 = 0;
unsigned char data8 = 0;
unsigned char data9 = 0;//0が稼働　1が停止
//CANのメッセージ
CANMessage msg1(0x1,&data1);//作動開始を通知
CANMessage msg2(0x2,CANStandard);//PIのパラメタ設定の終了を通知
//CANMessage msg3(0x3,CANStandard);//共通エリアのPI用　不要になった
CANMessage msg4(0x4,&data4);//初期位置を指定
CANMessage msg5(0x5,&data5);//稼働開始
CANMessage msg6(0x6,CANStandard);//電磁弁の必要性を通知
CANMessage msg7(0x7,&data7);//コンベアの起動を通知
CANMessage msg8(0x8,&data8);//終了を通知
CANMessage msg9(0x9,&data9);//自動機の一時停止/再始動

int main()
{
  printf("Program started\n");
//初期設定
  //割り込み関数の設定
  MainSetting.rise(callback(&phaze_admin));
  Shift_Location.rise(callback(&SetUp_Location));
  Conveyor.rise(callback(&con_awake));
  //直つなぎのアクチュエータの設定
  C_hand.period_ms(20);
  C_wrist.period_ms(20);
  Guide1.period_ms(20);
  Guide2.period_ms(20);
  //はじめはMainSetting以外の割り込みを無効化
//  Conveyor.disable_irq();
//  Shift_Location.disable_irq();
  while(1)
  {
    if(controller.axes.x==100)
    {
      printf("UP ");
    }
    if(controller.axes.x==-100)
    {
      printf("DONW ");
    }
    if(controller.axes.y==100)
    {
      printf("right ");
    }
    if(controller.axes.y == -100)
    {
      printf("left ");
    }
    if(controller.buttons[0]==1)
    {
      printf("X ");
    }
    if(controller.buttons[1]==1)
    {
      printf("A ");
    }
    if(controller.buttons[2]==1)
    {
      printf("B ");
    }
    if(controller.buttons[3]==1)
    {
      printf("Y ");
    }
    if(controller.buttons[4]==1)
    {
      printf("LB ");
    }
    if(controller.buttons[5]==1)
    {
      printf("RB ");
    }
    if(controller.buttons[6]==1)
    {
      printf("LT ");
    }
    if(controller.buttons[7]==1)
    {
      printf("RT ");
    }
    printf("\b\n");
    wait_us(500000);
  }
//以下主要処理
  //0.1秒ごとに最適な処理を行う
  while(1)
  {
    
    switch(phaze_counter)
    {
      case 0:
      //何もしない
      break;

      case 1:
      //ガイドをn秒動かす->10nに帰る
      if(phaze_counter != phaze_prev)
      {
      //1回だけ:開始通知・MainSettingを無効に
        printf("phaze 1 start!\n");
        can.write(msg1);
        time_counter4 = 0;
      }else if(can.read(msg2) && time_counter4 > 50)
      {
      //ずっと:初期設定の終了を待機
        printf("phaze 1 ended!\n");
        MainSetting.enable_irq();
      }else if(time_counter4 <= 50)
      {
      //5秒かけてガイド下げる
        GuideDown();
      }
      break;

      case 2:
      if(phaze_counter != phaze_prev)
      {
      //１回だけ:shiftボタンの有効化
        printf("phaze 2: started!\n");
        Shift_Location.enable_irq();
      }else{
      //ずっと：Own_Locationを送信
        msg4.data[0] = Own_Location;
        can.write(msg4);
      }
      break;

      case 3:
      if(phaze_counter != phaze_prev)
      {
        printf("phaze 3: start!\n");
        can.write(msg5);
        Conveyor.enable_irq();
      }else{
        if(can.read(msg6))
        {
          printf("Denjiben activated!\n");
          if(msg6.data[0]==0)
          {
            DJ.close();
          }else if(msg6.data[0]==1)
          {
            DJ.open();
          }
        }
        if(con_flag)
        {
          printf("conveyor activated!\n");
          can.write(msg7);
        }
        if(controller.buttons[1] == 1)//A
        {
          printf("pushed A\n");
          C_Hand_Grip();
        }else if(controller.buttons[2] == 1)//B
        {
          printf("pushed B\n");
          C_Hand_Release();
        }
        if(controller.buttons[0] == 1)//X
        {
          printf("pushed X\n");
          C_Wrist_CW();
        }else if(controller.buttons[3] == 1)//Y
        {
          printf("pushed Y\n");
          C_Wrist_CCW();
        }
        if(controller.buttons[4] == 1)//LB ガイド上げる
        {
          GuideUp();
        }else if(controller.buttons[6] == 1)//LT ガイド下げる
        {
          GuideDown();
        }
        if(controller.buttons[5] == 1 && time_counter5 > 5)//RBが押されたら
        {
          time_counter5 = 0;
          can.write(msg9);
        }
      }
      break;

      case 4:
      //終了処理
      if(phaze_counter != phaze_prev)
      {
      //1回だけ：can送信
        can.write(msg8);
        printf("phaze 4: start");
        time_counter4 = 0;
        MainSetting.disable_irq();
        Shift_Location.disable_irq();
      }else if(time_counter4 <= 50)
      {
        GuideUp();
      }else{
        phaze_counter++;
        printf("all program ended!\n");
      }
      break;

      default:
      printf("error!\n");
    }
    time_counter1++;
    time_counter2++;
    time_counter3++;
    time_counter4++;
    time_counter5++;
    phaze_prev = phaze_counter;
    wait_us(100000);
  }
  return 0;
}

void phaze_admin()
{
  if(time_counter1 > 5)
  {
    phaze_counter++;
    time_counter1 = 0;
  }
}

void SetUp_Location()
{
  if(time_counter2 > 5)
  {
    Own_Location++;
    if(Own_Location > 5)
    {
      Own_Location -= 6;
    }
    time_counter2 = 0;
  }
}

void con_awake()
{
  if(time_counter3 > 5)
  {
    time_counter3 = 0;
    con_flag = !con_flag;
  }
}

void C_Hand_Grip()
{
  C_hand.pulsewidth_us(time_counter4 * 18);
}

void C_Hand_Release()
{
  C_hand.pulsewidth_us(2400 - time_counter4 * 18);
}

void C_Wrist_CW()
{
  wrist_Location += 18;
  C_wrist.pulsewidth_us(wrist_Location);
}

void C_Wrist_CCW()
{
  wrist_Location -= 18;
  C_wrist.pulsewidth_us(wrist_Location);
}

void GuideUp()
{
  Guide_Location -= 18;
  Guide1.pulsewidth(Guide_Location);
  Guide2.pulsewidth(3000 - Guide_Location);
}

void GuideDown()
{
  Guide_Location += 18;
  Guide1.pulsewidth(Guide_Location);
  Guide2.pulsewidth(3000 - Guide_Location);
}
*/