#include <xc.h>
#include "KF1.h"

unsigned int getADCValue(unsigned char channel);        //单次AD转换
unsigned int getADS(void);                              //多次AD转换
void __interrupt() ISR(void);                           //中断处理函数
int t;
unsigned long Rt;                                   //VR 当前电压值
unsigned int const TABLE[] = {9712, 9166, 8654, 8172, 7722, 7298, 6900, 6526, 6176, 5534, 5242, 4966, 4708, 4464, 4234, 4016, 3812, 3620, 3438, 3266, 
                              3104, 2950, 2806, 2668, 2540, 2418, 2302, 2192, 2088, 1990, 1897, 1809, 1726, 1646, 1571, 1500, 1432, 1368, 1307, 1249, 
                              1194, 1142, 1092, 1045, 1000, 957, 916, 877, 840, 805, 772, 740, 709, 680, 653, 626, 601, 577, 554, 532, 511, 491, 472, 
                              454, 436, 420, 404, 388, 374, 360, 346, 334, 321, 309, 298, 287, 277, 267, 258, 248
                             };  //   -20 ~ 60 温度对应电阻值，为了优化空间，所有值缩小一位数 0 ~ 80

/*
unsigned long const TABLE2[] = {99102, 93450, 88156, 83195, 78544, 74183, 70091, 66250, 66643, 59255, 				//-20~-11
                                56071, 53078, 50263, 47614, 45121, 42774, 40563, 38480, 36517, 34665, 				//-10~-1
                                32919, 																				//0
                                31270, 29715, 28246, 26858, 25547, 24307, 23135, 22026, 20977, 19987, 				//1~10
	                            19044, 18154, 17310, 16510, 15752, 15034, 14352, 13705, 13090, 12507, 				//11~20
	                            11953, 11427, 10927, 10452, 10000, 9570, 9161, 8771, 8401, 8048, 					//21~30
	                            7712, 7391, 7086, 6795, 6518, 6254, 6001, 5761, 5531, 5311, 						//31~40
	                            5102, 4902, 4710, 4528, 4353, 4186, 4026, 3874, 3728, 3588, 						//41~50
	                            3454, 3326, 3203, 3085, 2973, 2865, 2761, 2662, 2567, 2476                          //51~60
                               };
*/

void main(void) 
{
    char add = 0;       //电位器的采样频率
    unsigned long ad1;  //ad1  NTC
    unsigned int ad2;   //ad2  电位器
    setup();            //AD转换初始化函数调用，原函数位于KF1.c
    PWMinit();          //PWM初始化函数调用，原函数位于KF1.c
    __delay_ms(2000);   //缓冲期
    
    while (1)           //主循环
    {
        if(add == 3)                        //错开两路AD转换的频率
        {
            unsigned long VR;
            ad1 = getADCValue(0x00);        //AD接口感应的AD值
            ad1 = 1024 - ad1;               //由于NTC位于测温电路下端，所以温度升高时，电压下降，电阻上升。因此，将AD值反向处理
            VR = ad1 * 500 / 1024;          //转换成电压值
            Rt = (unsigned long)(500 - VR) * 1000 / VR;      //计算NTC当前的动态电阻值
            add = 0;                        //重置AD识别指示器
        }
        else
        {
            ad2 = getADS();                 //获取电位器AD值
            t = (int)(ad2 * 10 / 128);      //1024 * 10 / 128 = 80
            add++;                          //AD识别指示器
        }
    }
}

unsigned int getADCValue(unsigned char channel)            //AD转换函数
{
    ADCON0bits.CHS = channel;                               //选择AD通道
    __delay_ms(5);                                          //改变AD通道后，需延时稳定
    ADCON0bits.GO = 1;                                      //开始AD转换
    while (ADCON0bits.GO);                                  //转换完成指示
    return (unsigned int)((ADRESH << 2) | (ADRESL >> 6));   //返回高低位合并后的AD值
}

unsigned int getADS(void)               //多次AD转换函数
{
    unsigned int ac[4], m;
    for(m = 0; m < 3; m++)
    {
        ac[m] = getADCValue(0x03);
        ac[3] += ac[m];
    }
    return ac[3] / 3;
}

void __interrupt() ISR(void)            //中断处理函数
{
    signed char v, i, p;                //p PWM增减指示器
    if(PIR1bits.TMR2IF == 1)			//检测时钟2是否溢出中断
	{
		PIE1bits.TMR2IE = 0;			//禁止时钟2溢出中断
        T2CONbits.TMR2ON = 0;			//停止计数
        PIR1bits.TMR2IF = 0; 			//时钟2溢出标志清零
		if(PWM1DCH >= PR2)              //检测脉宽是否超过周期最大值
        {   
            p = -1;
		}
        else if(PWM1DCH <= 0)
        {
			p = 1;
		}
        if(p == 1)                      //判断PWM增减指示器状态
        {
            PWM1DCH++;
        }
        else if(p == -1)
        {
            PWM1DCH--;
        }
        TMR2 = 0x00;                    //重置时钟2的计时
		T2CONbits.TMR2ON = 1;           //开启时钟2
        PIE1bits.TMR2IE = 1;			//允许时钟2溢出中断
    }
    
    if(PIR1bits.ADIF == 1)              //检查AD是否发生中断
    {
        PIE1bits.ADIE = 0;              //禁止中断发生
        PIR1bits.ADIF = 0;              //清除中断标志
        
        if(ADCON0bits.CHS == 0x00)      //判断发生中断的AD通道，这里是NTC
        {
            for(i = 0; i < 80; i++)     //将转换后的动态电阻值与TABLE数组中的元素依次比较
            {
                if(Rt >= TABLE[i])      //温度越低，电阻越高，因此，在数据表中从高到低进行查找
                {
                    v = i;              //另存变量，防止覆盖
                    break;              //查找到所需数据后跳出循环
                }
            }
        }
        else if(ADCON0bits.CHS == 0x03) //判断发生中断的AD通道，这里是电位器
        {
            if(v <= (t - 2))            //当实际温度小于等于设定温度 - 2， 设定回差，避免温度临界跳动
            {
                B1 = 1;                 //点亮报警灯
                PIE1bits.TMR2IE = 0;    //关闭时钟2，暂停中断
                PWM1DCH = PR2;          //使呼吸灯常亮
            }
            else if(v > t)              //当实际温度大于设定温度
            {
                B1 = 0;                 //关闭报警灯
                PIE1bits.TMR2IE = 1;    //开启时钟2，启用中断
            }
        }
        PIE1bits.ADIE = 1;              //一次中断执行完毕，允许中断继续发生
    }
}
