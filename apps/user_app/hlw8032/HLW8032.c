#include <stdio.h>
#include <stdlib.h>

/* run this program using the console pauser or add your own getch, system("pause") or input loop */
#define SAMPLE_RESISTANCE_MR	2//ʹ�õĲ�����ͭ����mRֵ

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

#define ARRAY_LEN   	20//ƽ���˲�buf����
#define COUNT_NUM   	1//��ʱ�������ݴ���

#define ARRAY_LEN1   	200//ƽ���˲�buf����
//8032���ܼ����������ʱ������
#define ENERGY_FLOW_NUM			65536   //�����ɼ����������ʱ���������ֵ

typedef struct RuningInf_s{
	unsigned short  voltage;//��ǰ��ѹֵ����λΪ0.1V
	unsigned short  electricity;//��ǰ����ֵ,��λΪ0.01A
	unsigned short  power;//��ǰ����,��λΪ0.1W
	
	unsigned long   energy;//��ǰ���ĵ���ֵ��Ӧ���������
	unsigned short  energyCurrent;//�������嵱ǰͳ��ֵ
	unsigned short  energyLast;//���������ϴ�ͳ��ֵ
	unsigned char   energyFlowFlag;//���������������
	
	unsigned long   energyUnit;//0.001�ȵ��Ӧ��������� 
}RuningInf_t;

RuningInf_t runingInf;

//��ȡ��ѹ�����������ʵ���������
unsigned long getVIPvalue(unsigned long *arr)//���µ�ѹ�����������ʵ��б�
{
	int maxIndex = 0;
	int minIndex = 0;
	unsigned long sum = 0;
	int j = 0;
	//��һ���������棬�ҵ����ݵ����ֵ����Сֵ���±�
	for(j = 1; j<ARRAY_LEN; j++){
		if(arr[maxIndex] <= arr[j]){//������������һ��ʱminIndex����maxIndex
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

unsigned long getPRPvalue(unsigned long *arr)//���µ�ѹ�����������ʵ��б�
{
	int maxIndex = 0;
	int minIndex = 0;
	unsigned long sum = 0;
	int j = 0;
	//��һ���������棬�ҵ����ݵ����ֵ����Сֵ���±�  ����ͬ�����ֵ����ʹ���±����
	for(j = 1; j<ARRAY_LEN1; j++){
		if(arr[maxIndex] <= arr[j]){//������������һ��ʱminIndex����maxIndex
			maxIndex = j;
		}
		if(arr[minIndex] > arr[j]){
			minIndex = j;
		}
	}
	

	return arr[minIndex];
	//���ص��±��ǣ����������У������ֵ����Сֵ���±�ǰ�ĵ�һ��
	//���硣���ֵ�±���3.��Сֵ�±���4.��ô���ص���0.
	//������ֵ�±���5.��Сֵ�±���4.��ô���صĻ�����0.
	//������ֵ�±���0.��Сֵ�±���4.��ô���ص���1.
	// for(j = 0; j<ARRAY_LEN1; j++){
	// 	if((maxIndex == j)||(minIndex == j)){
	// 		continue;
	// 	}
	// 	else{
			
	// 		return arr[j];
	// 	}
	// }
}

unsigned long getPRPvalueK(unsigned long *arr)//���µ�ѹ�����������ʵ��б�
{
	int maxIndex = 0;
	int minIndex = 0;
	unsigned long sum = 0;
	int j = 0;
	//��һ���������棬�ҵ����ݵ����ֵ����Сֵ���±�
	for(j = 1; j<ARRAY_LEN1; j++){
		if(arr[maxIndex] <= arr[j]){//������������һ��ʱminIndex����maxIndex
			maxIndex = j;
		}
		if(arr[minIndex] > arr[j]){
			minIndex = j;
		}
	}
	
	return arr[maxIndex];
}

int isUpdataNewData(unsigned long *arr,unsigned long dat)//����ѹ���������Ƿ���Ҫ����
{
	if(arr[0] == dat){
		return 0;
	}
	else{
		return 1;
	}
}

void updataVIPvalue(unsigned long *arr,unsigned long dat)//���µ�ѹ�����������ʵ��б�
{
	int ii = ARRAY_LEN-1;
	for(ii = ARRAY_LEN-1; ii > 0; ii--){
		arr[ii] = arr[ii-1];
	}
	arr[0] = dat;
}

void updataPRPvalue(unsigned long *arr,unsigned long dat)//���µ�ѹ�����������ʵ��б�
{
	int ii = ARRAY_LEN1-1;
	for(ii = ARRAY_LEN1-1; ii > 0; ii--){
		arr[ii] = arr[ii-1];
	}
	arr[0] = dat;
}

void resetVIPvalue(unsigned long *arr,unsigned long dat)//�������е�ѹ�����������ʵ��б�
{	
	int ii = ARRAY_LEN-1;
	for(ii = ARRAY_LEN-1; ii >= 0; ii--){
		arr[ii] = dat;
	}
}


void resetPRPvalue(unsigned long *arr,unsigned long dat)//�������е�ѹ�����������ʵ��б�
{	
	int ii = ARRAY_LEN1-1;
	for(ii = ARRAY_LEN1-1; ii >= 0; ii--){
		arr[ii] = dat;
	}
}




// ҫ��ʱ���� 
extern unsigned char  voltage_array[3];
extern unsigned char  power_array[4];

void get_voltage_array(unsigned long p_v)
{
	voltage_array[0] = p_v / 100 % 10;  //��λ 
	voltage_array[1] = p_v / 10 % 10;  //ʮλ 
	voltage_array[2] = p_v / 1 % 10;  //��λ 

}

void get_power_array(unsigned long p_p)
{
	power_array[0] = p_p / 1000 % 10; // ǧ
	power_array[1] = p_p / 100 % 10; // ��
	power_array[2] = p_p / 10 % 10; // ʮ
	power_array[3] = p_p / 1 % 10; // ��

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

	static unsigned long power_a[ARRAY_LEN]= {0};  //���ʼĴ���
	static unsigned long power_a1[ARRAY_LEN1]= {0};  //���ʲ����Ĵ���

	static unsigned int  powerCnt	= 0;
	static unsigned int  powerCnt1	= 0;
	static unsigned long powerNewFlag = 1;
	






	unsigned int	  energy_cnt     = 0;
	unsigned char	  energyFlowFlag = 0;


	startFlag = inDataBuffer[UART_IND_HD];
	switch(startFlag)
	{
		case 0x55:
			// ------------------------- ��ѹ -----------------------------
			if((inDataBuffer[UART_IND_FG]&0x40) == 0x40){//��ȡ��ǰ��ѹ���£�Ϊ1ʱ˵����ѹ���OK
				//��ѹ�����Ĵ���
				voltage_k = ((inDataBuffer[UART_IND_VK] << 16)|(inDataBuffer[UART_IND_VK+1] << 8)|(inDataBuffer[UART_IND_VK+2]));//��ѹϵ��
				//��ѹ�Ĵ���
				voltage_t = ((inDataBuffer[UART_IND_VT] << 16)|(inDataBuffer[UART_IND_VT+1] << 8)|(inDataBuffer[UART_IND_VT+2]));//��ѹ����

// ƽ����������				
				if(isUpdataNewData(voltage_a,voltage_t)){  //����ѹ���������Ƿ���Ҫ����
					updataVIPvalue(voltage_a,voltage_t);
					voltageCnt = 0;
				}
				else{
					voltageCnt++;
					if(voltageCnt >= COUNT_NUM){
						voltageCnt = 0;
						updataVIPvalue(voltage_a,voltage_t); //���µ�ѹ�����������ʵ��б�
					}
				}



				printf("voltage:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",voltage_a[0],voltage_a[1],voltage_a[2],voltage_a[3],voltage_a[4],voltage_a[5],voltage_a[6],voltage_a[7],voltage_a[8],voltage_a[9]);
			

				voltage_t = getVIPvalue(voltage_a);  ////���µ�ѹ�����������ʵ��б�
				
				if(voltage_t == 0){
					voltage = 0;

				}
				else{
					// voltage = voltage_k * 100 / voltage_t;//��ѹ10mVֵ���������
					// voltage = voltage * 10;//��ѹmVֵ
					
					voltage = (voltage_k / voltage_t ) * 1.88;  //1.88�ǵ�ѹϵ��������Ӳ��4�����趼��470K����

				}
				
				get_voltage_array(voltage);
			
					
				printf("Vk = %d,Vt = %d,v = %d\r\n",voltage_k,voltage_t,voltage);
			}
			else{
				printf("%s(%d):V Flag Error\r\n",__func__,__LINE__);
			}
			// ------------------------- ���� ----------------------------------
			if((inDataBuffer[UART_IND_FG]&0x20) == 0x20){
				electricity_k = ((inDataBuffer[UART_IND_IK] << 16)|(inDataBuffer[UART_IND_IK+1] << 8)|(inDataBuffer[UART_IND_IK+2]));//����ϵ��
				electricity_t = ((inDataBuffer[UART_IND_IT] << 16)|(inDataBuffer[UART_IND_IT+1] << 8)|(inDataBuffer[UART_IND_IT+2]));//��������

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

					// electricity = electricity_k * 100 / electricity_t;//����10mAֵ���������
					// electricity = electricity * 10;//����mAֵ

					electricity = (electricity_k / electricity_t) * 1;


					#if(SAMPLE_RESISTANCE_MR == 1)
					//����ʹ��1mR�ĵ��裬�����͹�����Ҫ������2
					#elif(SAMPLE_RESISTANCE_MR == 2)
					//����ʹ��2mR�ĵ��裬�����͹�����Ҫ����2
					electricity >>= 1;
					#endif
				}
				printf("Ik = %d,It = %d,I = %d\r\n",electricity_k,electricity_t,electricity);
			}
			else{
				printf("%s(%d):I Flag Error\r\n",__func__,__LINE__);
			}

			// ---------------------------- ���� ----------------------------------
			if((inDataBuffer[UART_IND_FG]&0x10) == 0x10){
				powerNewFlag = 0;
				power_k = ((inDataBuffer[UART_IND_PK] << 16)|(inDataBuffer[UART_IND_PK+1] << 8)|(inDataBuffer[UART_IND_PK+2]));//����ϵ��
				power_t = ((inDataBuffer[UART_IND_PT] << 16)|(inDataBuffer[UART_IND_PT+1] << 8)|(inDataBuffer[UART_IND_PT+2]));//��������
				
				//���ʲ����Ĵ���
				//ƽ����������
				if(isUpdataNewData(power_a1,power_k)){
					updataPRPvalue(power_a1,power_k);   //���⴦��  50��
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


				//���ʼĴ���
				//ƽ����������
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
				

				// printf("power:%d,%d,%d\r\n",power_a[0],power_a[1],power_a[2]);  //���ʼĴ���
				power_t = getVIPvalue(power_a);
				power_k = getPRPvalueK(power_a1);  //�������

				if(power_t == 0){
					power = 0;
				}
				else{

					// power = power_k * 100 / power_t;//����10mwֵ���������
					// power = power * 10;//����mwֵ

					power = (power_k / power_t) * 1.88 * 1;

					#if(SAMPLE_RESISTANCE_MR == 1)
					//����ʹ��1mR�ĵ��裬�����͹�����Ҫ������2
					#elif(SAMPLE_RESISTANCE_MR == 2)
					//����ʹ��2mR�ĵ��裬�����͹�����Ҫ����2
					power >>= 1;
					#endif
				}


				get_power_array(power);
				printf("Pk = %d,Pt = %d,P = %d\r\n",power_k,power_t,power);
			}
			else if(powerNewFlag == 0){

				power_k = ((inDataBuffer[UART_IND_PK] << 16)|(inDataBuffer[UART_IND_PK+1] << 8)|(inDataBuffer[UART_IND_PK+2]));//����ϵ��
				power_t = ((inDataBuffer[UART_IND_PT] << 16)|(inDataBuffer[UART_IND_PT+1] << 8)|(inDataBuffer[UART_IND_PT+2]));//��������
				

				//���ʲ����Ĵ���
				//ƽ����������
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



				//���ʼĴ���
				if(isUpdataNewData(power_a,power_t)){  //��ȡ�������ݵĵ�0λ����ͬ
					unsigned long powerData = getVIPvalue(power_a);  
					if(power_t > powerData){  //��ǰ��ȡֵ������Сֵ
						if((power_t - powerData) > (powerData >> 2)){
							// printf("reset++++++++++++++++++++++++++++");
							resetVIPvalue(power_a,power_t);
						}
					}
				}



				// printf("power:%d,%d,%d\r\n",power_a[0],power_a[1],power_a[2]);

				power_t = getVIPvalue(power_a); // ������С
				power_k = getPRPvalueK(power_a1);//�������




				if(power_t == 0){
					power = 0;
				}
				else{
					// power = power_k * 100 / power_t;//����10mwֵ���������
					// power = power * 10;//����mwֵ

					power = (power_k / power_t) * 1.88 * 1;

					#if(SAMPLE_RESISTANCE_MR == 1)
					//����ʹ��1mR�ĵ��裬�����͹�����Ҫ������2
					#elif(SAMPLE_RESISTANCE_MR == 2)
					//����ʹ��2mR�ĵ��裬�����͹�����Ҫ����2
					power >>= 1;
					#endif
				}
				get_power_array(power);
				printf("Pk = %d,Pt = %d,P = %d\r\n",power_k,power_t,power);
			}
			
			// ------------------------------- ���� -------------------------------
			energyFlowFlag = (inDataBuffer[UART_IND_FG] >> 7);//��ȡ��ǰ���ܼ����������
			runingInf.energyCurrent = ((inDataBuffer[UART_IND_EN] << 8)|(inDataBuffer[UART_IND_EN+1]));//���µ�ǰ���������ֵ
			if(runingInf.energyFlowFlag != energyFlowFlag){//ÿ�μ������ʱ���µ�ǰ�������ֵ
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
			runingInf.energy += (energy_cnt * 10);//���ܸ����ۼ�ʱ����10������������ǳ�������10������֤���㾫��
			
			runingInf.energyUnit = 0xD693A400 >> 1;
			runingInf.energyUnit /= (power_k >> 1);//1mR��������0.001�ȵ��Ӧ���������
			#if(SAMPLE_RESISTANCE_MR == 1)
			//1mR��ͭ�����Ӧ���������
			#elif(SAMPLE_RESISTANCE_MR == 2)
			//2mR��ͭ�����Ӧ���������
			runingInf.energyUnit = (runingInf.energyUnit << 1);//2mR��������0.001�ȵ��Ӧ���������
			#endif
			runingInf.energyUnit =runingInf.energyUnit * 10;//0.001�ȵ��Ӧ���������(�������ʱ�Ŵ���10��������������ҲҪ�Ŵ�10��)
			
			//����ʹ����=runingInf.energy/runingInf.energyUnit;//��λ��0.001��
			break;
		
		case 0xAA:
			//оƬδУ׼
			printf("HLW8032 not check\r\n");
			break;

		default :
			if((startFlag & 0xF1) == 0xF1){//�洢���쳣��оƬ����
				//оƬ����������������
				printf("HLW8032 broken\r\n");
			}

			if((startFlag & 0xF2) == 0xF2){//�����쳣
				runingInf.power = 0;//��ȡ���Ĺ�������0.1WΪ��λ
				power = 0;
				//printf("Power Error\r\n");
			}
			else{
				if((inDataBuffer[UART_IND_FG]&0x10) == 0x10){
					powerNewFlag = 0;
					power_k = ((inDataBuffer[UART_IND_PK] << 16)|(inDataBuffer[UART_IND_PK+1] << 8)|(inDataBuffer[UART_IND_PK+2]));//����ϵ��
					power_t = ((inDataBuffer[UART_IND_PT] << 16)|(inDataBuffer[UART_IND_PT+1] << 8)|(inDataBuffer[UART_IND_PT+2]));//��������
					
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
						// power = power_k * 100 / power_t;//����10mwֵ���������
						// power = power * 10;//����mwֵ

						power = (power_k / power_t) * 1.88 * 1;

						#if(SAMPLE_RESISTANCE_MR == 1)
						//����ʹ��1mR�ĵ��裬�����͹�����Ҫ������2
						#elif(SAMPLE_RESISTANCE_MR == 2)
						//����ʹ��2mR�ĵ��裬�����͹�����Ҫ����2
						power >>= 1;
						#endif
					}

						// get_power_array(power);

					printf("Pk = %d,Pt = %d,P = %d\r\n",power_k,power_t,power);
				}
				else if(powerNewFlag == 0){
					power_k = ((inDataBuffer[UART_IND_PK] << 16)|(inDataBuffer[UART_IND_PK+1] << 8)|(inDataBuffer[UART_IND_PK+2]));//����ϵ��
					power_t = ((inDataBuffer[UART_IND_PT] << 16)|(inDataBuffer[UART_IND_PT+1] << 8)|(inDataBuffer[UART_IND_PT+2]));//��������
					
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
						// power = power_k * 100 / power_t;//����10mwֵ���������
						// power = power * 10;//����mwֵ

						power = (power_k / power_t) * 1.88 * 1;


						#if(SAMPLE_RESISTANCE_MR == 1)
						//����ʹ��1mR�ĵ��裬�����͹�����Ҫ������2
						#elif(SAMPLE_RESISTANCE_MR == 2)
						//����ʹ��2mR�ĵ��裬�����͹�����Ҫ����2
						power >>= 1;
						#endif
					}

						// get_power_array(power);
					printf("Pk = %d,Pt = %d,P = %d\r\n",power_k,power_t,power);
				}
			}

			if((startFlag & 0xF4) == 0xF4){//�����쳣
				runingInf.electricity = 0;//��ȡ���ĵ�����0.01AΪ��λ				
				electricity = 0;
			}
			else{
				if((inDataBuffer[UART_IND_FG]&0x20) == 0x20){
					electricity_k = ((inDataBuffer[UART_IND_IK] << 16)|(inDataBuffer[UART_IND_IK+1] << 8)|(inDataBuffer[UART_IND_IK+2]));//����ϵ��
					electricity_t = ((inDataBuffer[UART_IND_IT] << 16)|(inDataBuffer[UART_IND_IT+1] << 8)|(inDataBuffer[UART_IND_IT+2]));//��������

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
						// electricity = electricity_k * 100 / electricity_t;//����10mAֵ���������
						// electricity = electricity * 10;//����mAֵ

						electricity = (electricity_k /  electricity_t ) * 1;


						#if(SAMPLE_RESISTANCE_MR == 1)
						//����ʹ��1mR�ĵ��裬�����͹�����Ҫ������2
						#elif(SAMPLE_RESISTANCE_MR == 2)
						//����ʹ��2mR�ĵ��裬�����͹�����Ҫ����2
						electricity >>= 1;
						#endif
					}
					printf("Ik = %d,It = %d,I = %d\r\n",electricity_k,electricity_t,electricity);
				}
				else{
					printf("%s(%d):I Flag Error\r\n",__func__,__LINE__);
				}
			}
			
			if((startFlag & 0xF8) == 0xF8){//��ѹ�쳣
				runingInf.voltage = 0;//��ȡ���ĵ�ѹ����0.1VΪ��λ				
				voltage = 0;
			}
			else{
				if((inDataBuffer[UART_IND_FG]&0x40) == 0x40){//��ȡ��ǰ��ѹ���£�Ϊ1ʱ˵����ѹ���OK
					voltage_k = ((inDataBuffer[UART_IND_VK] << 16)|(inDataBuffer[UART_IND_VK+1] << 8)|(inDataBuffer[UART_IND_VK+2]));//��ѹϵ��
					voltage_t = ((inDataBuffer[UART_IND_VT] << 16)|(inDataBuffer[UART_IND_VT+1] << 8)|(inDataBuffer[UART_IND_VT+2]));//��ѹ����
					
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
					// voltage = voltage_k * 100 / voltage_t;//��ѹ10mVֵ���������
					// voltage = voltage * 10;//��ѹmVֵ
					
					voltage = (voltage_k / voltage_t ) * 1.88;  //1.88�ǵ�ѹϵ��������Ӳ��4�����趼��470K����

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
// 	//ģ��8032���͵�һ������ ��220V-5A-1100W 
// 	unsigned char dataTemp[24] = {0x55,0x5A,
// 						 0x02,0xDE,0xC4,0x00,0x03,0x57,
// 						 0x00,0x3F,0x66,0x00,0x0C,0xAE,
// 						 0x52,0x90,0x54,0x00,0x13,0x37,
// 						 0x71,0x00,0x13,0x61};//���ڽ��յ��� 8032���͵�һ����Ч���� 
// 	DealUartInf(dataTemp,24);//����8032���� 
// 	DealUartInf(dataTemp,24);//����8032���� 
// 	DealUartInf(dataTemp,24);//����8032���� 
// 	return 0;
// }



// 55 5A 02 0E C4 00 03 57 00 3F 66 00 0C AE 52 90 54 00 13 37 71 00 13 61
