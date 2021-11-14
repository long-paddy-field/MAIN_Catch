asm(".global _printf_float");

#include"mbed.h"
#include"controller.h"

#define SVALUE
#define USER_SW
#define PWM_CH
#define PWM_CW

//CAN通信
CAN can_con(PB_8,PB_9,500000);
CAN can_info(PB_5,PB_6,500000);

//電磁弁
DigitalOut S_valve(SVALUE);

//起動スイッチ（あれば）
DigitalIn SW(USER_SW);

//共通エリア用サーボモータ
PwmOut C_hand(PWM_CH);
PwmOut C_wrist(PWM_CW);

//コントローラのコンストラクタ
Controller controller(can_con,0x334);


int main()
{
  printf("Program started\n");
  while(SW);

  
  return 0;
}