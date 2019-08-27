//协议：帧头(0xff) - 长度 - 帧id(0x01) - data_1[0] - data_1[1] - data_1[2] - data_1[3] -...- data_n[3] - CRC 
#include<iostream>
#include <stdlib.h>
#include <time.h> 
#include <iomanip>
using namespace std;

typedef unsigned char u8;
typedef float f32;
typedef unsigned long u32; 

const int NUM = 10;			//传输数据个数 
u8 DT[3+4*NUM+1]; 			//被传输的数据 

typedef struct
{
	float rawData[NUM];		//被发送的原始float数据 
	u8 num;					//发送的float型数据个数 
	
	u8 head;				//帧头 0xff 
	u8 length;				//帧长度 
	u8 id;					//帧id 0x01 
	u8 data[4*NUM];			//数据段，一个float四字节 
	u8 crc;					//crc校验位 
}frame; 


//编码-------------------------------------------- 
//计算校验位 
u8 Add_CRC(u8 *buf,u8 num)
{
	u8 byte_crc=0;
	for(int i=0;i<4*num;i++)
    	byte_crc+=buf[i]; 
	
  	return byte_crc;
}

//拆分数据 
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

//编码 
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

//解码-------------------------------------------- 
//拼接数据
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
	int status;					//0x01 -> 校验失败
								//0x02 -> 丢失帧头
								//0x03 -> id错误
		
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

//发送模拟------------------------
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
	f32 send[NUM];			//主机发送数据 
	f32 rec[NUM];			//从机接收数据 
	
	//生成测试浮点数 
	for(int i=0;i<NUM;i++)
		send[i]=((rand()%360)-180),send[i]/=10,cout<<send[i]<<"      ";
	
	dataCode(&f,send);		//编码 
	sendData(&f);			//发送 & 接受 
	dataDecode(DT,rec);		//解码 

	//检查接受的浮点数 
	cout<<endl;
	for(int i=0;i<NUM;i++)
		cout<<rec[i]<<"      ";	
	
	return 0;
}

