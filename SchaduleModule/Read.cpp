#include "Read.h"
#include <stdio.h>
#include <iostream>

using namespace std;
//大于实际从txt中读入的车辆数 
#define NO_DATA 0

int car_order_min = INT_MAX;
int car_order_max = 0;
int car_txt_num = 0;
int car_total_num = 0;   //总车辆数
int road_order_min = INT_MAX;
int road_order_max = 0;
int road_txt_num = 0;
int road_total_num = 0;   //总道路数
int cross_order_min = INT_MAX;
int cross_order_max = 0;
int cross_txt_num = 0;
int cross_total_num = 0;   //总路口数目
int car_speed_min = INT_MAX;   //车辆可能的行驶速度
int car_speed_max = 0;
int car_speed_cnt = 0;    

int **GetCar(std::string address)
{
	char *add = (char*)address.data();
	char *ch = NULL;
	long int byte_num = 0;
	FILE *car_txt;
	if ((car_txt = fopen(add, "r")) == NULL)
	{
		fprintf(stderr, "Open Error!");
		exit(EXIT_FAILURE);
	}
	//获取文件字节数  EOF 算进去了 ??
	fseek(car_txt, 0L, SEEK_END);
	byte_num = ftell(car_txt);
	fseek(car_txt, 0L, SEEK_SET);  //要返回缓冲区开头

	if ((ch = (char *)malloc(sizeof(char) * byte_num)) == NULL)
	{
		fprintf(stderr, "Malloc Error!");
		exit(EXIT_FAILURE);
	}

	if (fread(ch, sizeof(char), byte_num, car_txt) == byte_num)
		fprintf(stdout, "Car Read Successfully!\n");

	fclose(car_txt);

	int MaxCarNum = 0;
	for (int byte_cnt = 0; byte_cnt < byte_num; byte_cnt++)
		if (ch[byte_cnt] == ')')MaxCarNum++; //每多一行回车就多一一辆车 保守估计
	
	//----开存储数组-----
	int **car;
	car = (int **)malloc(sizeof(int*) * MaxCarNum);  //这里车辆数可以更改为 动态数组  
	for (int i = 0; i < MaxCarNum; i++)
	{
		car[i] = (int *)malloc(sizeof(int) * 5);
		for (int j = 0; j < 5; j++)
			car[i][j] = NO_DATA;
	}
	//-------------------需要检查---------
	long int num = 0;
	unsigned char info = 0, cnt = 0;
	int car_cnt = 0;
	char char2num[15] = { 0 };

	while (num < byte_num)
	{
		if (ch[num] == '#')
			while (ch[num] != '\n')
				num++;
		if (ch[num - 2] == '\n' && ch[num - 1] == '(') //换行 + ( 开头
		{
			info = 0;
			//-----信息数组 0车编号  1起点  2终点  3速度  4计划出发时间-----
			while (ch[num] != '\n')
			{
				if (ch[num] >= '0' && ch[num] <= '9')
				{
					cnt = 0;
					while (ch[num] != ',' && ch[num] != ')')  //，或) 信息采集的结束
					{
						char2num[cnt++] = ch[num++];
						if (cnt == 14) break;
					}
					if (cnt == 14) break;         //越界保护
					if (num == byte_num) break;   //越界保护
					for (int i = 0; i < cnt; i++)
						car[car_cnt][info] = char2num[i] - '0' + car[car_cnt][info] * 10;
					//printf("%d   ", car[car_cnt][info]);
					info++;
				}
				else
					num++;
			}
			//更新车编号最小值 最大值
			if (car[car_cnt][0] < car_order_min) car_order_min = car[car_cnt][0];
			if (car[car_cnt][0] > car_order_max) car_order_max = car[car_cnt][0];
			if (car[car_cnt][3] > car_speed_max) car_speed_max = car[car_cnt][3];
			if (car[car_cnt][3] < car_speed_min) car_speed_min = car[car_cnt][3];
			//printf("\n");
			car_cnt++;
		}
		else
			num++;
	}
	car_speed_cnt = car_speed_max - car_speed_min + 1;
	car_total_num = car_order_max - car_order_min + 1;
	car_txt_num = car_cnt;
	free(ch);
	return car;
}

int **GetRoad(std::string address)
{
	char *add = (char*)address.data();
	char *ch = NULL;
	long int byte_num = 0;
	FILE *road_txt;
	if ((road_txt = fopen(add, "r")) == NULL)
	{
		fprintf(stderr, "Open Error!");
		exit(EXIT_FAILURE);
	}
	//获取文件字节数  EOF 算进去了 ??
	fseek(road_txt, 0L, SEEK_END);
	byte_num = ftell(road_txt);
	fseek(road_txt, 0L, SEEK_SET);  //要返回缓冲区开头

	if ((ch = (char *)malloc(sizeof(char) * byte_num)) == NULL)
	{
		fprintf(stderr, "Malloc Error!");
		exit(EXIT_FAILURE);
	}

	if (fread(ch, sizeof(char), byte_num, road_txt) == byte_num)
		fprintf(stdout, "Road Read Successfully!\n");
	
	fclose(road_txt);

	int MaxRoadNum = 0;
	for (int byte_cnt = 0; byte_cnt < byte_num; byte_cnt++)
		if (ch[byte_cnt] == '\n')MaxRoadNum++; //每多一行回车就多一条路 保守估计

	//----开存储数组-----
	int **road;
	road = (int **)malloc(sizeof(int*) * MaxRoadNum);  //这里车辆数可以更改为 动态数组  
	for (int i = 0; i < MaxRoadNum; i++)
	{
		road[i] = (int *)malloc(sizeof(int) * 7);
		for (int j = 0; j < 7; j++)
			road[i][j] = NO_DATA;
	}
	//-------------------需要检查---------
	long int num = 0;
	unsigned char info = 0, cnt = 0;
	int road_cnt = 0;
	char char2num[15] = { 0 };

	while (num < byte_num - 5)
	{
		if (ch[num] == '#')
			while (ch[num] != '\n')
				num++;
		if (ch[num - 2] == '\n' && ch[num - 1] == '(') //换行 + ( 开头
		{
			info = 0;
			//-----信息数组 0路编号  1长  2速度  3车道数  4起始节点  5终止节点  6双向标记-----
			while (ch[num] != '\n')
			{
				if (ch[num] >= '0' && ch[num] <= '9')
				{
					cnt = 0;
					while (ch[num] != ',' && ch[num] != ')')  //，或) 信息采集的结束
					{
						char2num[cnt++] = ch[num++];
						if (cnt == 14) break;
					}
					if (cnt == 14) break;         //越界保护
					if (num == byte_num) break;   //越界保护
					for (int i = 0; i < cnt; i++)
						road[road_cnt][info] = char2num[i] - '0' + road[road_cnt][info] * 10;
					//printf("%d   ", road[road_cnt][info]);
					info++;
				}
				else
					num++;
			}
			//更编号最小值 最大值
			if (road[road_cnt][0] < road_order_min) road_order_min = road[road_cnt][0];
			if (road[road_cnt][0] > road_order_max) road_order_max = road[road_cnt][0];
			road_cnt++;
			//printf("\n");
		}
		else
			num++;
	}
	road_total_num = road_order_max - road_order_min + 1;
	road_txt_num = road_cnt;
	free(ch);
	return road;
}

int **GetCross(std::string address)
{
	char *add = (char*)address.data();
	char *ch = NULL;
	long int byte_num = 0;
	FILE *cross_txt;
	if ((cross_txt = fopen(add, "r")) == NULL)
	{
		fprintf(stderr, "Open Error!");
		exit(EXIT_FAILURE);
	}
	//获取文件字节数  EOF 算进去了 ??
	fseek(cross_txt, 0L, SEEK_END);
	byte_num = ftell(cross_txt);
	fseek(cross_txt, 0L, SEEK_SET);  //要返回缓冲区开头

	if ((ch = (char *)malloc(sizeof(char) * byte_num)) == NULL)
	{
		fprintf(stderr, "Malloc Error!");
		exit(EXIT_FAILURE);
	}

	if (fread(ch, sizeof(char), byte_num, cross_txt) == byte_num)
		fprintf(stdout, "Cross Read Successfully!\n");

	fclose(cross_txt);
	int MaxCrossNum = 0;
	for (int byte_cnt = 0; byte_cnt < byte_num; byte_cnt++)
		if (ch[byte_cnt] == '\n')MaxCrossNum++; //每多一行回车就多一条路 保守估计
	//----开存储数组-----
	int **cross;
	cross = (int **)malloc(sizeof(int*) * MaxCrossNum);  //这里车辆数可以更改为 动态数组  
	for (int i = 0; i < MaxCrossNum; i++)
	{
		cross[i] = (int *)malloc(sizeof(int) * 5);
		for (int j = 0; j < 5; j++)
			cross[i][j] = NO_DATA;
	}
	//-------------------需要检查---------
	long int num = 0;
	unsigned char info = 0, cnt = 0;
	int cross_cnt = 0;
	char char2num[15] = { 0 };

	while (num < byte_num - 5)
	{
		if (ch[num] == '#')
			while (ch[num] != '\n')
				num++;
		if (ch[num - 2] == '\n' && ch[num - 1] == '(') //换行 + ( 开头
		{
			info = 0;
			//-----信息数组 0路口编号  1上  2右  3下  4左-----
			while (ch[num] != '\n')
			{
				if ((ch[num] >= '0' && ch[num] <= '9') || ch[num] == '-')
				{
					cnt = 0;
					while (ch[num] != ',' && ch[num] != ')')  //，或) 信息采集的结束
					{
						char2num[cnt++] = ch[num++];
						if (cnt == 14) break;
					}
					if (cnt == 14) break;         //越界保护
					if (num == byte_num) break;   //越界保护
					if (char2num[0] == '-') //"-1"道路标记
						cross[cross_cnt][info] = -1;
					else
						for (int i = 0; i < cnt; i++)
							cross[cross_cnt][info] = char2num[i] - '0' + cross[cross_cnt][info] * 10;
					//printf("%d   ", cross[cross_cnt][info]);
					info++;
				}
				else
					num++;
			}
			//更编号最小值 最大值
			if (cross[cross_cnt][0] < cross_order_min) cross_order_min = cross[cross_cnt][0];
			if (cross[cross_cnt][0] > cross_order_max) cross_order_max = cross[cross_cnt][0];
			cross_cnt++;
			//printf("\n");
		}
		else
			num++;
	}
	cross_total_num = cross_order_max - cross_order_min + 1;
	cross_txt_num = cross_cnt;
	free(ch);
	return cross;
}

