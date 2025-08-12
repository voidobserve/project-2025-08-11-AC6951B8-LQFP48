#include <stdio.h>
#include <stdlib.h>

/* run this program using the console pauser or add your own getch, system("pause") or input loop */
#define SAMPLE_RESISTANCE_MR	2//使用的采样锰铜电阻mR值

#define UART_IND_HD			0
#define UART_IND_5A			1
#define UART_IND_VK			2
#define UART_IND_VT			5
#define UART_IND_IK			8
#define UART_IND_IT			11
#define UART_IND_PK			14
#define UART_IND_PT			17
#define UART_IND_FG			20
#define UART_IND_EN			21
#define UART_IND_SM			23

#define ARRAY_LEN   	20//平滑滤波buf长度
#define COUNT_NUM   	1//超时更新数据次数

#define ARRAY_LEN1   	200//平滑滤波buf长度
//8032电能计数脉冲溢出时的数据
#define ENERGY_FLOW_NUM			65536   //电量采集，电能溢出时的脉冲计数值

typedef struct RuningInf_s{
	unsigned short  voltage;//当前电压值，单位为0.1V
	unsigned short  electricity;//当前电流值,单位为0.01A
	unsigned short  power;//当前功率,单位为0.1W
	
	unsigned long   energy;//当前消耗电能值对应的脉冲个数
	unsigned short  energyCurrent;//电能脉冲当前统计值
	unsigned short  energyLast;//电能脉冲上次统计值
	unsigned char   energyFlowFlag;//电能脉冲溢出标致
	
	unsigned long   energyUnit;//0.001度点对应的脉冲个数 
}RuningInf_t;

RuningInf_t runingInf;

//获取电压、电流、功率的有限数据
unsigned long getVIPvalue(unsigned long *arr)//更新电压、电流、功率的列表
{
	int maxIndex = 0;
	int minIndex = 0;
	unsigned long sum = 0;
	int j = 0;
	//在一个数组里面，找到数据的最大值和最小值的下标
	for(j = 1; j<ARRAY_LEN; j++){
		if(arr[maxIndex] <= arr[j]){//避免所有数据一样时minIndex等于maxIndex
			maxIndex = j;
		}
		if(arr[minIndex] > arr[j]){
			minIndex = j;
		}
	}
	
	// printf("minIndex = %d", minIndex);
	// printf("maxIndex = %d", maxIndex);

		return arr[minIndex];

	// for(j = 0; j<ARRAY_LEN; j++){
	// 	if((maxIndex == j)||(minIndex == j)){
	// 		continue;
	// 	}
	// 	else{
	// 		printf("j  = %d", j);
	// 		return arr[j];
	// 	}
	// }
}

unsigned long getPRPvalue(unsigned long *arr)//更新电压、电流、功率的列表
{
	int maxIndex = 0;
	int minIndex = 0;
	unsigned long sum = 0;
	int j = 0;
	//在一个数组里面，找到数据的最大值和最小值的下标  有相同的最大值，会使用下标最大
	for(j = 1; j<ARRAY_LEN1; j++){
		if(arr[maxIndex] <= arr[j]){//避免所有数据一样时minIndex等于maxIndex
			maxIndex = j;
		}
		if(arr[minIndex] > arr[j]){
			minIndex = j;
		}
	}
	

	return arr[minIndex];
	//返回的下标是，整个数组中，在最大值，最小值的下标前的第一个
	//例如。最大值下标是3.最小值下标是4.那么返回的是0.
	//如果最大值下标是5.最小值下标是4.那么返回的还是是0.
	//如果最大值下标是0.最小值下标是4.那么返回的是1.
	// for(j = 0; j<ARRAY_LEN1; j++){
	// 	if((maxIndex == j)||(minIndex == j)){
	// 		continue;
	// 	}
	// 	else{
			
	// 		return arr[j];
	// 	}
	// }
}

unsigned long getPRPvalueK(unsigned long *arr)//更新电压、电流、功率的列表
{
	int maxIndex = 0;
	int minIndex = 0;
	unsigned long sum = 0;
	int j = 0;
	//在一个数组里面，找到数据的最大值和最小值的下标
	for(j = 1; j<ARRAY_LEN1; j++){
		if(arr[maxIndex] <= arr[j]){//避免所有数据一样时minIndex等于maxIndex
			maxIndex = j;
		}
		if(arr[minIndex] > arr[j]){
			minIndex = j;
		}
	}
	
	return arr[maxIndex];
}

int isUpdataNewData(unsigned long *arr,unsigned long dat)//检测电压电流功率是否需要更新
{
	if(arr[0] == dat){
		return 0;
	}
	else{
		return 1;
	}
}

void updataVIPvalue(unsigned long *arr,unsigned long dat)//更新电压、电流、功率的列表
{
	int ii = ARRAY_LEN-1;
	for(ii = ARRAY_LEN-1; ii > 0; ii--){
		arr[ii] = arr[ii-1];
	}
	arr[0] = dat;
}

void updataPRPvalue(unsigned long *arr,unsigned long dat)//更新电压、电流、功率的列表
{
	int ii = ARRAY_LEN1-1;
	for(ii = ARRAY_LEN1-1; ii > 0; ii--){
		arr[ii] = arr[ii-1];
	}
	arr[0] = dat;
}

void resetVIPvalue(unsigned long *arr,unsigned long dat)//更新所有电压、电流、功率的列表
{	
	int ii = ARRAY_LEN-1;
	for(ii = ARRAY_LEN-1; ii >= 0; ii--){
		arr[ii] = dat;
	}
}


void resetPRPvalue(unsigned long *arr,unsigned long dat)//更新所有电压、电流、功率的列表
{	
	int ii = ARRAY_LEN1-1;
	for(ii = ARRAY_LEN1-1; ii >= 0; ii--){
		arr[ii] = dat;
	}
}




// 耀祥时序器 
extern unsigned char  voltage_array[3];
extern unsigned char  power_array[4];

void get_voltage_array(unsigned long p_v)
{
	voltage_array[0] = p_v / 100 % 10;  //百位 
	voltage_array[1] = p_v / 10 % 10;  //十位 
	voltage_array[2] = p_v / 1 % 10;  //个位 

}

void get_power_array(unsigned long p_p)
{
	power_array[0] = p_p / 1000 % 10; // 千
	power_array[1] = p_p / 100 % 10; // 百
	power_array[2] = p_p / 10 % 10; // 十
	power_array[3] = p_p / 1 % 10; // 个

}

int DealUartInf(unsigned char *inDataBuffer,int recvlen)
{
	unsigned char     startFlag     = 0;
	
	unsigned long	  voltage_k	    = 0;
	unsigned long	  voltage_t     = 0;
	unsigned long	  voltage       = 0;
	static unsigned long voltage_a[ARRAY_LEN]  = {0};
	static unsigned int  voltageCnt	= 0;

	unsigned long	  electricity_k = 0;
	unsigned long	  electricity_t = 0;
	unsigned long	  electricity   = 0;
	static unsigned long electricity_a[ARRAY_LEN]  = {0};
	static unsigned int  electricityCnt	= 0;

	
	unsigned long	  power_k = 0;
	unsigned long	  power_t = 0;
	unsigned long	  power	  = 0;

	static unsigned long power_a[ARRAY_LEN]= {0};  //功率寄存器
	static unsigned long power_a1[ARRAY_LEN1]= {0};  //功率参数寄存器

	static unsigned int  powerCnt	= 0;
	static unsigned int  powerCnt1	= 0;
	static unsigned long powerNewFlag = 1;
	






	unsigned int	  energy_cnt     = 0;
	unsigned char	  energyFlowFlag = 0;


	startFlag = inDataBuffer[UART_IND_HD];
	switch(startFlag)
	{
		case 0x55:
			// ------------------------- 电压 -----------------------------
			if((inDataBuffer[UART_IND_FG]&0x40) == 0x40){//获取当前电压标致，为1时说明电压检测OK
				//电压参数寄存器
				voltage_k = ((inDataBuffer[UART_IND_VK] << 16)|(inDataBuffer[UART_IND_VK+1] << 8)|(inDataBuffer[UART_IND_VK+2]));//电压系数
				//电压寄存器
				voltage_t = ((inDataBuffer[UART_IND_VT] << 16)|(inDataBuffer[UART_IND_VT+1] << 8)|(inDataBuffer[UART_IND_VT+2]));//电压周期

// 平滑载入数据				
				if(isUpdataNewData(voltage_a,voltage_t)){  //检测电压电流功率是否需要更新
					updataVIPvalue(voltage_a,voltage_t);
					voltageCnt = 0;
				}
				else{
					voltageCnt++;
					if(voltageCnt >= COUNT_NUM){
						voltageCnt = 0;
						updataVIPvalue(voltage_a,voltage_t); //更新电压、电流、功率的列表
					}
				}



				printf("voltage:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",voltage_a[0],voltage_a[1],voltage_a[2],voltage_a[3],voltage_a[4],voltage_a[5],voltage_a[6],voltage_a[7],voltage_a[8],voltage_a[9]);
			

				voltage_t = getVIPvalue(voltage_a);  ////更新电压、电流、功率的列表
				
				if(voltage_t == 0){
					voltage = 0;

				}
				else{
					// voltage = voltage_k * 100 / voltage_t;//电压10mV值，避免溢出
					// voltage = voltage * 10;//电压mV值
					
					voltage = (voltage_k / voltage_t ) * 1.88;  //1.88是电压系数，根据硬件4个电阻都是470K决定

				}
				
				get_voltage_array(voltage);
			
					
				printf("Vk = %d,Vt = %d,v = %d\r\n",voltage_k,voltage_t,voltage);
			}
			else{
				printf("%s(%d):V Flag Error\r\n",__func__,__LINE__);
			}
			// ------------------------- 电流 ----------------------------------
			if((inDataBuffer[UART_IND_FG]&0x20) == 0x20){
				electricity_k = ((inDataBuffer[UART_IND_IK] << 16)|(inDataBuffer[UART_IND_IK+1] << 8)|(inDataBuffer[UART_IND_IK+2]));//电流系数
				electricity_t = ((inDataBuffer[UART_IND_IT] << 16)|(inDataBuffer[UART_IND_IT+1] << 8)|(inDataBuffer[UART_IND_IT+2]));//电流周期

				if(isUpdataNewData(electricity_a,electricity_t)){
					updataVIPvalue(electricity_a,electricity_t);
					electricityCnt = 0;
				}
				else{
					electricityCnt++;
					if(electricityCnt >= COUNT_NUM){
						electricityCnt = 0;
						updataVIPvalue(electricity_a,electricity_t);
					}
				}
				printf("electricity:%d,%d,%d\r\n",electricity_a[0],electricity_a[1],electricity_a[2]);
				electricity_t = getVIPvalue(electricity_a);
				
				if(electricity_t == 0){
					electricity = 0;
				}
				else{

					// electricity = electricity_k * 100 / electricity_t;//电流10mA值，避免溢出
					// electricity = electricity * 10;//电流mA值

					electricity = (electricity_k / electricity_t) * 1;


					#if(SAMPLE_RESISTANCE_MR == 1)
					//由于使用1mR的电阻，电流和功率需要不除以2
					#elif(SAMPLE_RESISTANCE_MR == 2)
					//由于使用2mR的电阻，电流和功率需要除以2
					electricity >>= 1;
					#endif
				}
				printf("Ik = %d,It = %d,I = %d\r\n",electricity_k,electricity_t,electricity);
			}
			else{
				printf("%s(%d):I Flag Error\r\n",__func__,__LINE__);
			}

			// ---------------------------- 功率 ----------------------------------
			if((inDataBuffer[UART_IND_FG]&0x10) == 0x10){
				powerNewFlag = 0;
				power_k = ((inDataBuffer[UART_IND_PK] << 16)|(inDataBuffer[UART_IND_PK+1] << 8)|(inDataBuffer[UART_IND_PK+2]));//功率系数
				power_t = ((inDataBuffer[UART_IND_PT] << 16)|(inDataBuffer[UART_IND_PT+1] << 8)|(inDataBuffer[UART_IND_PT+2]));//功率周期
				
				//功率参数寄存器
				//平滑载入数据
				if(isUpdataNewData(power_a1,power_k)){
					updataPRPvalue(power_a1,power_k);   //特殊处理  50个
					powerCnt1 = 0;
				}
				else{
					powerCnt1++;
					if(powerCnt1 >= COUNT_NUM){
						powerCnt1 = 0;
						updataPRPvalue(power_a1,power_k);
					}
				}

				// printf("power_a1:%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",power_a1[10],power_a1[11],power_a1[12],power_a1[13],power_a1[14],power_a1[15],power_a1[16],power_a1[17],power_a1[8],power_a1[9]);


				//功率寄存器
				//平滑载入数据
				if(isUpdataNewData(power_a,power_t)){
					updataVIPvalue(power_a,power_t);
					powerCnt = 0;
				}
				else{
					powerCnt++;
					if(powerCnt >= COUNT_NUM){
						powerCnt = 0;
						updataVIPvalue(power_a,power_t);
					}
				}
				

				// printf("power:%d,%d,%d\r\n",power_a[0],power_a[1],power_a[2]);  //功率寄存器
				power_t = getVIPvalue(power_a);
				power_k = getPRPvalueK(power_a1);  //返回最大

				if(power_t == 0){
					power = 0;
				}
				else{

					// power = power_k * 100 / power_t;//功率10mw值，避免溢出
					// power = power * 10;//功率mw值

					power = (power_k / power_t) * 1.88 * 1;

					#if(SAMPLE_RESISTANCE_MR == 1)
					//由于使用1mR的电阻，电流和功率需要不除以2
					#elif(SAMPLE_RESISTANCE_MR == 2)
					//由于使用2mR的电阻，电流和功率需要除以2
					power >>= 1;
					#endif
				}


				get_power_array(power);
				printf("Pk = %d,Pt = %d,P = %d\r\n",power_k,power_t,power);
			}
			else if(powerNewFlag == 0){

				power_k = ((inDataBuffer[UART_IND_PK] << 16)|(inDataBuffer[UART_IND_PK+1] << 8)|(inDataBuffer[UART_IND_PK+2]));//功率系数
				power_t = ((inDataBuffer[UART_IND_PT] << 16)|(inDataBuffer[UART_IND_PT+1] << 8)|(inDataBuffer[UART_IND_PT+2]));//功率周期
				

				//功率参数寄存器
				//平滑载入数据
				if(isUpdataNewData(power_a1,power_k)){
					updataPRPvalue(power_a1,power_k);
					powerCnt1 = 0;
				}
				else{
					powerCnt1++;
					if(powerCnt1 >= COUNT_NUM){
						powerCnt1 = 0;
						updataPRPvalue(power_a1,power_k);
					}
				}

				// printf("power_a1:%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",power_a1[10],power_a1[11],power_a1[12],power_a1[13],power_a1[14],power_a1[15],power_a1[16],power_a1[17],power_a1[8],power_a1[9]);



				//功率寄存器
				if(isUpdataNewData(power_a,power_t)){  //获取到与数据的第0位不相同
					unsigned long powerData = getVIPvalue(power_a);  
					if(power_t > powerData){  //当前获取值大于最小值
						if((power_t - powerData) > (powerData >> 2)){
							// printf("reset++++++++++++++++++++++++++++");
							resetVIPvalue(power_a,power_t);
						}
					}
				}



				// printf("power:%d,%d,%d\r\n",power_a[0],power_a[1],power_a[2]);

				power_t = getVIPvalue(power_a); // 返回最小
				power_k = getPRPvalueK(power_a1);//返回最大




				if(power_t == 0){
					power = 0;
				}
				else{
					// power = power_k * 100 / power_t;//功率10mw值，避免溢出
					// power = power * 10;//功率mw值

					power = (power_k / power_t) * 1.88 * 1;

					#if(SAMPLE_RESISTANCE_MR == 1)
					//由于使用1mR的电阻，电流和功率需要不除以2
					#elif(SAMPLE_RESISTANCE_MR == 2)
					//由于使用2mR的电阻，电流和功率需要除以2
					power >>= 1;
					#endif
				}
				get_power_array(power);
				printf("Pk = %d,Pt = %d,P = %d\r\n",power_k,power_t,power);
			}
			
			// ------------------------------- 电量 -------------------------------
			energyFlowFlag = (inDataBuffer[UART_IND_FG] >> 7);//获取当前电能计数溢出标致
			runingInf.energyCurrent = ((inDataBuffer[UART_IND_EN] << 8)|(inDataBuffer[UART_IND_EN+1]));//更新当前的脉冲计数值
			if(runingInf.energyFlowFlag != energyFlowFlag){//每次计数溢出时更新当前脉冲计数值
				runingInf.energyFlowFlag = energyFlowFlag;
				if(runingInf.energyCurrent > runingInf.energyLast){
					runingInf.energyCurrent = 0;
				}
				energy_cnt = runingInf.energyCurrent + ENERGY_FLOW_NUM - runingInf.energyLast;
			}
			else{
				energy_cnt = runingInf.energyCurrent - runingInf.energyLast;
			}
			runingInf.energyLast = runingInf.energyCurrent;
			runingInf.energy += (energy_cnt * 10);//电能个数累加时扩大10倍，计算电能是除数扩大10倍，保证计算精度
			
			runingInf.energyUnit = 0xD693A400 >> 1;
			runingInf.energyUnit /= (power_k >> 1);//1mR采样电阻0.001度电对应的脉冲个数
			#if(SAMPLE_RESISTANCE_MR == 1)
			//1mR锰铜电阻对应的脉冲个数
			#elif(SAMPLE_RESISTANCE_MR == 2)
			//2mR锰铜电阻对应的脉冲个数
			runingInf.energyUnit = (runingInf.energyUnit << 1);//2mR采样电阻0.001度电对应的脉冲个数
			#endif
			runingInf.energyUnit =runingInf.energyUnit * 10;//0.001度电对应的脉冲个数(计算个数时放大了10倍，所以在这里也要放大10倍)
			
			//电能使用量=runingInf.energy/runingInf.energyUnit;//单位是0.001度
			break;
		
		case 0xAA:
			//芯片未校准
			printf("HLW8032 not check\r\n");
			break;

		default :
			if((startFlag & 0xF1) == 0xF1){//存储区异常，芯片坏了
				//芯片坏掉，反馈服务器
				printf("HLW8032 broken\r\n");
			}

			if((startFlag & 0xF2) == 0xF2){//功率异常
				runingInf.power = 0;//获取到的功率是以0.1W为单位
				power = 0;
				//printf("Power Error\r\n");
			}
			else{
				if((inDataBuffer[UART_IND_FG]&0x10) == 0x10){
					powerNewFlag = 0;
					power_k = ((inDataBuffer[UART_IND_PK] << 16)|(inDataBuffer[UART_IND_PK+1] << 8)|(inDataBuffer[UART_IND_PK+2]));//功率系数
					power_t = ((inDataBuffer[UART_IND_PT] << 16)|(inDataBuffer[UART_IND_PT+1] << 8)|(inDataBuffer[UART_IND_PT+2]));//功率周期
					
					if(isUpdataNewData(power_a,power_t)){
						updataVIPvalue(power_a,power_t);
						powerCnt = 0;
					}
					else{
						powerCnt++;
						if(powerCnt >= COUNT_NUM){
							powerCnt = 0;
							updataVIPvalue(power_a,power_t);
						}
					}
					//printf("power:%d,%d,%d\r\n",power_a[0],power_a[1],power_a[2]);
					power_t = getVIPvalue(power_a);
					
					if(power_t == 0){
						power = 0;
					}
					else{
						// power = power_k * 100 / power_t;//功率10mw值，避免溢出
						// power = power * 10;//功率mw值

						power = (power_k / power_t) * 1.88 * 1;

						#if(SAMPLE_RESISTANCE_MR == 1)
						//由于使用1mR的电阻，电流和功率需要不除以2
						#elif(SAMPLE_RESISTANCE_MR == 2)
						//由于使用2mR的电阻，电流和功率需要除以2
						power >>= 1;
						#endif
					}

						// get_power_array(power);

					printf("Pk = %d,Pt = %d,P = %d\r\n",power_k,power_t,power);
				}
				else if(powerNewFlag == 0){
					power_k = ((inDataBuffer[UART_IND_PK] << 16)|(inDataBuffer[UART_IND_PK+1] << 8)|(inDataBuffer[UART_IND_PK+2]));//功率系数
					power_t = ((inDataBuffer[UART_IND_PT] << 16)|(inDataBuffer[UART_IND_PT+1] << 8)|(inDataBuffer[UART_IND_PT+2]));//功率周期
					
					if(isUpdataNewData(power_a,power_t)){
						unsigned long powerData = getVIPvalue(power_a);
						if(power_t > powerData){
							if((power_t - powerData) > (powerData >> 2)){
								resetVIPvalue(power_a,power_t);
							}
						}
					}
					//printf("power:%d,%d,%d\r\n",power_a[0],power_a[1],power_a[2]);
					power_t = getVIPvalue(power_a);
					
					if(power_t == 0){
						power = 0;
					}
					else{
						// power = power_k * 100 / power_t;//功率10mw值，避免溢出
						// power = power * 10;//功率mw值

						power = (power_k / power_t) * 1.88 * 1;


						#if(SAMPLE_RESISTANCE_MR == 1)
						//由于使用1mR的电阻，电流和功率需要不除以2
						#elif(SAMPLE_RESISTANCE_MR == 2)
						//由于使用2mR的电阻，电流和功率需要除以2
						power >>= 1;
						#endif
					}

						// get_power_array(power);
					printf("Pk = %d,Pt = %d,P = %d\r\n",power_k,power_t,power);
				}
			}

			if((startFlag & 0xF4) == 0xF4){//电流异常
				runingInf.electricity = 0;//获取到的电流以0.01A为单位				
				electricity = 0;
			}
			else{
				if((inDataBuffer[UART_IND_FG]&0x20) == 0x20){
					electricity_k = ((inDataBuffer[UART_IND_IK] << 16)|(inDataBuffer[UART_IND_IK+1] << 8)|(inDataBuffer[UART_IND_IK+2]));//电流系数
					electricity_t = ((inDataBuffer[UART_IND_IT] << 16)|(inDataBuffer[UART_IND_IT+1] << 8)|(inDataBuffer[UART_IND_IT+2]));//电流周期

					if(isUpdataNewData(electricity_a,electricity_t)){
						updataVIPvalue(electricity_a,electricity_t);
						electricityCnt = 0;
					}
					else{
						electricityCnt++;
						if(electricityCnt >= COUNT_NUM){
							electricityCnt = 0;
							updataVIPvalue(electricity_a,electricity_t);
						}
					}
					//printf("electricity:%d,%d,%d\r\n",electricity_a[0],electricity_a[1],electricity_a[2]);
					electricity_t = getVIPvalue(electricity_a);
					
					if(electricity_t == 0){
						electricity = 0;
					}
					else{
						// electricity = electricity_k * 100 / electricity_t;//电流10mA值，避免溢出
						// electricity = electricity * 10;//电流mA值

						electricity = (electricity_k /  electricity_t ) * 1;


						#if(SAMPLE_RESISTANCE_MR == 1)
						//由于使用1mR的电阻，电流和功率需要不除以2
						#elif(SAMPLE_RESISTANCE_MR == 2)
						//由于使用2mR的电阻，电流和功率需要除以2
						electricity >>= 1;
						#endif
					}
					printf("Ik = %d,It = %d,I = %d\r\n",electricity_k,electricity_t,electricity);
				}
				else{
					printf("%s(%d):I Flag Error\r\n",__func__,__LINE__);
				}
			}
			
			if((startFlag & 0xF8) == 0xF8){//电压异常
				runingInf.voltage = 0;//获取到的电压是以0.1V为单位				
				voltage = 0;
			}
			else{
				if((inDataBuffer[UART_IND_FG]&0x40) == 0x40){//获取当前电压标致，为1时说明电压检测OK
					voltage_k = ((inDataBuffer[UART_IND_VK] << 16)|(inDataBuffer[UART_IND_VK+1] << 8)|(inDataBuffer[UART_IND_VK+2]));//电压系数
					voltage_t = ((inDataBuffer[UART_IND_VT] << 16)|(inDataBuffer[UART_IND_VT+1] << 8)|(inDataBuffer[UART_IND_VT+2]));//电压周期
					
					if(isUpdataNewData(voltage_a,voltage_t)){
						updataVIPvalue(voltage_a,voltage_t);
						voltageCnt = 0;
					}
					else{
						voltageCnt++;
						if(voltageCnt >= COUNT_NUM){
							voltageCnt = 0;
							updataVIPvalue(voltage_a,voltage_t);
						}
					}
					//printf("voltage:%d,%d,%d\r\n",voltage_a[0],voltage_a[1],voltage_a[2]);
					voltage_t = getVIPvalue(voltage_a);
					
					if(voltage_t == 0){
						voltage = 0;
					}
					else{
					// voltage = voltage_k * 100 / voltage_t;//电压10mV值，避免溢出
					// voltage = voltage * 10;//电压mV值
					
					voltage = (voltage_k / voltage_t ) * 1.88;  //1.88是电压系数，根据硬件4个电阻都是470K决定

				}
				
				// get_voltage_array(voltage);
					printf("Vk = %d,Vt = %d,v = %d\r\n",voltage_k,voltage_t,voltage);
				}
				else{
					printf("%s(%d):V Flag Error\r\n",__func__,__LINE__);
				} 
			}
			printf("0x%x:V = %d;I = %d;P = %d;\r\n",startFlag,voltage,electricity,power);
			break;
	}
	return 1;
}

// int main(int argc, char *argv[]) {
// 	//模拟8032发送的一包数据 ，220V-5A-1100W 
// 	unsigned char dataTemp[24] = {0x55,0x5A,
// 						 0x02,0xDE,0xC4,0x00,0x03,0x57,
// 						 0x00,0x3F,0x66,0x00,0x0C,0xAE,
// 						 0x52,0x90,0x54,0x00,0x13,0x37,
// 						 0x71,0x00,0x13,0x61};//串口接收到的 8032发送的一包有效数据 
// 	DealUartInf(dataTemp,24);//处理8032数据 
// 	DealUartInf(dataTemp,24);//处理8032数据 
// 	DealUartInf(dataTemp,24);//处理8032数据 
// 	return 0;
// }



// 55 5A 02 0E C4 00 03 57 00 3F 66 00 0C AE 52 90 54 00 13 37 71 00 13 61
