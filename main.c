#include <xc.h>
#include "KF1.h"

unsigned int getADCValue(unsigned char channel);        //����ADת��
unsigned int getADS(void);                              //���ADת��
void __interrupt() ISR(void);                           //�жϴ�����

int t;
unsigned long Rt;                                   //VR ��ǰ��ѹֵ
unsigned int const TABLE[] = {9712, 9166, 8654, 8172, 7722, 7298, 6900, 6526, 6176, 5534, 5242, 4966, 4708, 4464, 4234, 4016, 3812, 3620, 3438, 3266, 
                              3104, 2950, 2806, 2668, 2540, 2418, 2302, 2192, 2088, 1990, 1897, 1809, 1726, 1646, 1571, 1500, 1432, 1368, 1307, 1249, 
                              1194, 1142, 1092, 1045, 1000, 957, 916, 877, 840, 805, 772, 740, 709, 680, 653, 626, 601, 577, 554, 532, 511, 491, 472, 
                              454, 436, 420, 404, 388, 374, 360, 346, 334, 321, 309, 298, 287, 277, 267, 258, 248
                             };  //   -20 ~ 60 �¶ȶ�Ӧ����ֵ��Ϊ���Ż��ռ䣬����ֵ��Сһλ�� 0 ~ 80

void main(void) 
{
    char add = 0;       //��λ���Ĳ���Ƶ��
    unsigned long ad1;  //ad1  NTC
    unsigned int ad2;   //ad2  ��λ��
    setup();            //ADת����ʼ���������ã�ԭ����λ��KF1.c
    PWMinit();          //PWM��ʼ���������ã�ԭ����λ��KF1.c
    __delay_ms(2000);   //������
    
    while (1)           //��ѭ��
    {
        if(add == 3)                        //����·ADת����Ƶ��
        {
            unsigned long VR;
            ad1 = getADCValue(0x00);        //AD�ӿڸ�Ӧ��ADֵ
            ad1 = 1024 - ad1;               //����NTCλ�ڲ��µ�·�¶ˣ������¶�����ʱ����ѹ�½���������������ˣ���ADֵ������
            VR = ad1 * 500 / 1024;          //ת���ɵ�ѹֵ
            Rt = (unsigned long)(500 - VR) * 1000 / VR;      //����NTC��ǰ�Ķ�̬����ֵ
            add = 0;                        //����ADʶ��ָʾ��
        }
        else
        {
            ad2 = getADS();                 //��ȡ��λ��ADֵ
            //t = (int)((float)(ad2) / 12.8);                   //1024 / 12 = 85
            t = (int)(ad2 / 12);
            add++;                          //ADʶ��ָʾ��
        }
    }
}

unsigned int getADCValue(unsigned char channel)            //ADת������
{
    ADCON0bits.CHS = channel;                               //ѡ��ADͨ��
    __delay_ms(5);                                          //�ı�ADͨ��������ʱ�ȶ�
    ADCON0bits.GO = 1;                                      //��ʼADת��
    while (ADCON0bits.GO);                                  //ת�����ָʾ
    return (unsigned int)((ADRESH << 2) | (ADRESL >> 6));   //���ظߵ�λ�ϲ����ADֵ
}

unsigned int getADS(void)               //���ADת������
{
    unsigned int ac1, ac2, ac3, acd;    //Ԥ���������������
    ac1 = getADCValue(0x03);
    __delay_us(5);
    ac2 = getADCValue(0x03);
    __delay_us(5);
    ac3 = getADCValue(0x03);
    __delay_us(5);
    return ((ac1 + ac2 + ac3) / 3);        //�ȶ�����
}

void __interrupt() ISR(void)            //�жϴ�����
{
    signed char v, i, p;                //p PWM����ָʾ��
    if(PIR1bits.TMR2IF == 1)			//���ʱ��2�Ƿ�����ж�
	{
		PIE1bits.TMR2IE = 0;			//��ֹʱ��2����ж�
        T2CONbits.TMR2ON = 0;			//ֹͣ����
        PIR1bits.TMR2IF = 0; 			//ʱ��2�����־����
		if(PWM1DCH >= PR2)              //��������Ƿ񳬹��������ֵ
        {
            p = -1;                     //�ǣ� ��ʼ�ݼ�
		}
        else if(PWM1DCH <= 0)          //��������Ƿ����������Сֵ
        {
			p = 1;                      //�ǣ� ��ʼ��������
		}
        if(p == 1)                      //�ж�PWM����ָʾ��״̬
        {
            PWM1DCH++;
        }
        else if(p == -1)                //PWM�ݼ�
        {
            PWM1DCH--;
        }
        TMR2 = 0x00;                    //����ʱ��2�ļ�ʱ
		T2CONbits.TMR2ON = 1;           //����ʱ��2
        PIE1bits.TMR2IE = 1;			//����ʱ��2����ж�
    }
    
    if(PIR1bits.ADIF == 1)              //���AD�Ƿ����ж�
    {
        PIE1bits.ADIE = 0;              //��ֹ�жϷ���
        PIR1bits.ADIF = 0;              //����жϱ�־
        
        if(ADCON0bits.CHS == 0x00)      //�жϷ����жϵ�ADͨ����������NTC
        {
            for(i = 0; i < 80; i++)     //��ת����Ķ�̬����ֵ��TABLE�����е�Ԫ�����αȽ�
            {
                if(Rt >= TABLE[i])      //�¶�Խ�ͣ�����Խ�ߣ���ˣ������ݱ��дӸߵ��ͽ��в���
                {
                    v = i;              //����������ֹ����
                    break;              //���ҵ��������ݺ�����ѭ��
                }
            }
        }
        else if(ADCON0bits.CHS == 0x03) //�жϷ����жϵ�ADͨ���������ǵ�λ��
        {
            if(v <= (t - 2))            //��ʵ���¶�С�ڵ����趨�¶� - 2�� �趨�ز�����¶��ٽ�����
            {
                B1 = 1;                 //����������
                PIE1bits.TMR2IE = 0;    //�ر�ʱ��2����ͣ�ж�
                PWM1DCH = PR2;          //ʹ�����Ƴ���
            }
            else if(v > t)              //��ʵ���¶ȴ����趨�¶�
            {
                B1 = 0;                 //�رձ�����
                PIE1bits.TMR2IE = 1;    //����ʱ��2�������ж�
            }
        }
        PIE1bits.ADIE = 1;              //һ���ж�ִ����ϣ������жϼ�������
    }
}
