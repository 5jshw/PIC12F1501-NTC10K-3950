#include <xc.h>
#include "KF1.h"

unsigned int getADCValue(unsigned char channel);
void __interrupt() ISR(void);
unsigned int ad1, ad2;      // ad1, ad2��ADת���Ĵ���
int t, v, i;                //t ��λ��ת���趨�¶�ֵ�� v NTCת��ʵ���¶�ֵ�� i �¶ȱ���ָʾ��
unsigned long VR;           //VR ��ǰ��ѹֵ
unsigned int Rt;            //Rt ��ǰ��̬����ֵ
unsigned int const TABLE[] = {9712, 9166, 8654, 8172, 7722, 7298, 6900, 6526, 6176, 5534, 5242, 4966, 4708, 4464, 4234, 4016, 3812, 3620, 3438, 3266, 
                              3104, 2950, 2806, 2668, 2540, 2418, 2302, 2192, 2088, 1990, 1897, 1809, 1726, 1646, 1571, 1500, 1432, 1368, 1307, 1249, 
                              1194, 1142, 1092, 1045, 1000, 957, 916, 877, 840, 805, 772, 740, 709, 680, 653, 626, 601, 577, 554, 532, 511, 491, 472, 
                              454, 436, 420, 404, 388, 374, 360, 346, 334, 321, 309, 298, 287, 277, 267, 258, 248
                             };  //   -20 ~ 60 �¶ȶ�Ӧ����ֵ��Ϊ���Ż��ռ䣬����ֵ��Сһλ��

void main(void) 
{
    setup();        //ADת����ʼ���������ã�ԭ����λ��KF1.c
    PWMinit();      //PWM��ʼ���������ã�ԭ����λ��KF1.c
    while (1)
    {
        ADRESH = 0;                     //ADת�������λ����
        ADRESL = 0;                     //ADת�������λ����
        ad1 = getADCValue(0x00);        //AD�ӿڸ�Ӧ��ADֵ
            ad1 = 1024 - ad1;           //����NTCλ�ڲ��µ�·�¶ˣ������¶�����ʱ����ѹ�½���������������ˣ���ADֵ������
            VR = ad1 * 500 / 1024;      //ת���ɵ�ѹֵ
            Rt = (unsigned int)(500 - VR) * 1000 / VR;      //����NTC��ǰ�Ķ�̬����ֵ
            
        ADRESH = 0;                     //ADת�������λ����
        ADRESL = 0;                     //ADת�������λ����

        ad2 = getADCValue(0x03);        //��λ���趨�¶�
            t = ad2 / 12;               //1024 / 12 = 85
    }
}

unsigned int getADCValue(unsigned char channel)     //ADת������
{
    ADCON0bits.CHS = channel;       //ѡ��ADͨ��
    __delay_ms(5);                  //�ı�ADͨ��������ʱ�ȶ�
    ADCON0bits.GO = 1;              //��ʼADת��
    while (ADCON0bits.GO);          //ת�����ָʾ
    return (unsigned int)((ADRESH << 2) | (ADRESL >> 6));    //���ظߵ�λ�ϲ����ADֵ
}

void __interrupt() ISR(void)            //�жϴ�����
{
    short p;                            //p PWM����ָʾ��
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
        else if(p == -1)
        {
            PWM1DCH--;
        }
        TMR2 = 0x00;                    //����ʱ��2�ļ�ʱ
		PIE1bits.TMR2IE = 1;			//����ʱ��2����ж�
		T2CONbits.TMR2ON = 1;           //����ʱ��2
    }
    else if(PIR1bits.ADIF == 1)         //���AD�Ƿ����ж�
    {
        PIE1bits.ADIE = 0;              //��ֹ�жϷ���������жϱ�־
        PIR1bits.ADIF = 0;
        
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
            if(v >= t)                  //��ʵ���¶ȴ��ڵ����趨�¶�
            {
                B1 = 1;                 //����������
                T2CONbits.TMR2ON = 0;
                PWM1DCH = 0;
            }
            else if(v <= t - 10)        //��ʵ���¶�С�ڵ����趨�¶� - 2 �ȣ� �趨�ز�����¶��ٽ�����
            {
                B1 = 0;                 //�رձ�����
                T2CONbits.TMR2ON = 1;

            }
        }
        PIE1bits.ADIE = 1;              //һ���ж�ִ����ϣ������жϼ�������
    }
}
