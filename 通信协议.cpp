//Э�飺֡ͷ(0xff) - ���� - ֡id(0x01) - data_1[0] - data_1[1] - data_1[2] - data_1[3] -...- data_n[3] - CRC 
#include<iostream>
#include <stdlib.h>
#include <time.h> 
#include <iomanip>
using namespace std;

typedef unsigned char u8;
typedef float f32;
typedef unsigned long u32; 

const int NUM = 10;			//�������ݸ��� 
u8 DT[3+4*NUM+1]; 			//����������� 

typedef struct
{
	float rawData[NUM];		//�����͵�ԭʼfloat���� 
	u8 num;					//���͵�float�����ݸ��� 
	
	u8 head;				//֡ͷ 0xff 
	u8 length;				//֡���� 
	u8 id;					//֡id 0x01 
	u8 data[4*NUM];			//���ݶΣ�һ��float���ֽ� 
	u8 crc;					//crcУ��λ 
}frame; 


//����-------------------------------------------- 
//����У��λ 
u8 Add_CRC(u8 *buf,u8 num)
{
	u8 byte_crc=0;
	for(int i=0;i<4*num;i++)
    	byte_crc+=buf[i]; 
	
  	return byte_crc;
}

//������� 
void dataDisintegrate(f32 data,u8 *buf)
{	
	for(int i=0;i<4;i++)
		buf[i]=(*((u8 *)(&data)+i));
}
void prepareData(frame *f)
{
	for(int i=0;i<f->num;i++)
		dataDisintegrate(f->rawData[i],&(f->data[4*i]));
}

//���� 
void dataCode(frame *f,f32 *d)
{
	f->num=NUM; 
	for(int i=0;i<f->num;i++)
		f->rawData[i]=d[i];
		
	f->head=0xff;
	f->length=3+4*f->num+1; 
	f->id=0x01;

	prepareData(f);
	f->crc=Add_CRC(f->data,f->num);
}

//����-------------------------------------------- 
//ƴ������
f32 dataAssmeble(u8 *buf)
{
	u32 temp=buf[3];
	for(int i=2;i>=0;i--)
		temp=(temp<<8)|buf[i];
	
	f32 *data=(f32*)(&temp); 
	return *data;
}

int dataDecode(u8 *recBuf,f32 *res)
{		
	int status;					//0x01 -> У��ʧ��
								//0x02 -> ��ʧ֡ͷ
								//0x03 -> id����
		
	u8 data_length = recBuf[1];
  	u8 data_id     = recBuf[2];
  	u8 data_crc    = Add_CRC(recBuf+3,NUM);
	
	//data lost the frame header.
  	if(recBuf[0]!=0xFF)
  	{
    	status=0x02;
    	return status;
  	}
  	//data verify error
  	if(data_crc!=recBuf[data_length-1])
  	{
    	status=0x01;
    	return status;
  	}
  	//data x_y property error
  	if(data_id!=0x01) 
  	{
  		status=0x03;
  		return status;
  	};
  	//receive non error
	status=0;
		
	for(int i=0;i<NUM;i++)
		res[i]=dataAssmeble(&recBuf[3+4*i]);

	return status;
} 

//����ģ��------------------------
void sendData(frame *f)
{
	DT[0]=f->head;
	DT[1]=f->length;
	DT[2]=f->id;
	for(int i=0;i<4*f->num;i++)
		DT[3+i]=f->data[i];
	DT[f->length-1]=f->crc;
}

//main---------------------------------- 
int main()
{
	srand((unsigned)time(NULL)); 
	
	frame f; 
	f32 send[NUM];			//������������ 
	f32 rec[NUM];			//�ӻ��������� 
	
	//���ɲ��Ը����� 
	for(int i=0;i<NUM;i++)
		send[i]=((rand()%360)-180),send[i]/=10,cout<<send[i]<<"      ";
	
	dataCode(&f,send);		//���� 
	sendData(&f);			//���� & ���� 
	dataDecode(DT,rec);		//���� 

	//�����ܵĸ����� 
	cout<<endl;
	for(int i=0;i<NUM;i++)
		cout<<rec[i]<<"      ";	
	
	return 0;
}

