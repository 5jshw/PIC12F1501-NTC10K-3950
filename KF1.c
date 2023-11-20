#include <xc.h>
#include <pic12f1501.h>
#include "KF1.h"

void setup(void) 
{
    OSCCON = 0b01110000;  // ��������Ϊ8MHz

    // ��������ź�����
    B1 = 0;    // B1Ϊ���
    T_B1 = 0;  // ����B1Ϊ�������

    // ����ģ����������
    ANSELAbits.ANSA0 = 1;   // ����ģ���������� A0
    ANSELAbits.ANSA4 = 1;   // ����ģ���������� A4

    // ���� ADC
    ADCON1 = 0x50;          // ����Ҷ��룬FOSC/16���ο���ѹΪVDD
    PIE1bits.ADIE = 1;      // ���� ADC �ж�
    PIR1bits.ADIF = 0;      // ���� ADC �жϱ�־
    ADCON0bits.GO = 0;      // ��ʼ�� AD ת��
    ADCON0bits.ADON = 1;    // ���� AD ת����
}

void PWMinit(void)
{
    TRISAbits.TRISA2 = 1;       //��ֹ���
    PWM1CON = 0x00;             //����
    PWM1CONbits.PWM1POL = 0;    //�ߵ�ƽ��Ч
    PR2 = 200;                 //�趨����
    PWM1DCH = 0x00;             //����PWM��λ
    PWM1DCL = 0x00;             //����PWM��λ
    INTCONbits.GIE = 1;         //�����ж�
    INTCONbits.PEIE = 1;        //�������ж�
    PIE1bits.TMR2IE = 1;        //����Timer2����ж�
    PIR1bits.TMR2IF = 0;        //Timer2�����־����
    T2CONbits.T2CKPS = 3;       //ʱ��2Ԥ��ƵֵΪ4
    PWM1CONbits.PWM1EN = 1;     //ʹ�� PWM ģ��
    TRISAbits.TRISA2 = 0;       //�������
    PWM1CONbits.PWM1OE = 1;     //ʹ�ܵ� PWM1 ���ŵ����
    T2CONbits.TMR2ON = 1;       //����ʱ��2
}