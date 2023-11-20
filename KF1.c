#include <xc.h>
#include <pic12f1501.h>
#include "KF1.h"

void setup(void) 
{
    OSCCON = 0b01110000;  // 设置振荡器为8MHz

    // 配置输出信号引脚
    B1 = 0;    // B1为输出
    T_B1 = 0;  // 设置B1为输出引脚

    // 配置模拟输入引脚
    ANSELAbits.ANSA0 = 1;   // 设置模拟输入引脚 A0
    ANSELAbits.ANSA4 = 1;   // 设置模拟输入引脚 A4

    // 配置 ADC
    ADCON1 = 0x50;          // 结果右对齐，FOSC/16，参考电压为VDD
    PIE1bits.ADIE = 1;      // 允许 ADC 中断
    PIR1bits.ADIF = 0;      // 清零 ADC 中断标志
    ADCON0bits.GO = 0;      // 初始化 AD 转换
    ADCON0bits.ADON = 1;    // 开启 AD 转换器
}

void PWMinit(void)
{
    TRISAbits.TRISA2 = 1;       //禁止输出
    PWM1CON = 0x00;             //清零
    PWM1CONbits.PWM1POL = 0;    //高电平有效
    PR2 = 200;                 //设定周期
    PWM1DCH = 0x00;             //清零PWM高位
    PWM1DCL = 0x00;             //清零PWM低位
    INTCONbits.GIE = 1;         //开总中断
    INTCONbits.PEIE = 1;        //开外设中断
    PIE1bits.TMR2IE = 1;        //允许Timer2溢出中断
    PIR1bits.TMR2IF = 0;        //Timer2溢出标志清零
    T2CONbits.T2CKPS = 3;       //时钟2预分频值为4
    PWM1CONbits.PWM1EN = 1;     //使能 PWM 模块
    TRISAbits.TRISA2 = 0;       //允许输出
    PWM1CONbits.PWM1OE = 1;     //使能到 PWM1 引脚的输出
    T2CONbits.TMR2ON = 1;       //开启时钟2
}