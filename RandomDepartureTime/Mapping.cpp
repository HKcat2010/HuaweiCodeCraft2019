#include "stdio.h"
#include "Mapping.h"
#include "Read.h"
#include <iostream> //可以注释！

using namespace std;

struct Route ***MAP; //地图3维 [车速][起点id][终点id]

void map_building(int **cross, int **road)
{
	if ((MAP = (struct Route***)malloc(car_speed_cnt * sizeof(struct Route**))) == NULL)
	{
		fprintf(stderr, "No Enough Space!");
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < car_speed_cnt; i++)
	{
		if ((MAP[i] = (struct Route**)malloc(cross_total_num * sizeof(struct Route*))) == NULL) //起点
		{
			fprintf(stderr, "No Enough Space!");
			exit(EXIT_FAILURE);
		}
		for (int j = 0; j < cross_total_num; j++)                                               //终点
			if ((MAP[i][j] = (struct Route*) malloc(cross_total_num * sizeof(struct Route))) == NULL)
			{
				fprintf(stderr, "No Enough Space!");
				exit(EXIT_FAILURE);
			}
	}

	//初始化
	for (int speed_cnt = 0; speed_cnt < car_speed_cnt; speed_cnt++) 
		for (int i = 0; i < cross_total_num; i++)
		{
			for (int j = 0; j < cross_total_num; j++)
			{
				if(i==j)MAP[speed_cnt][i][j].total_road_cost = 0; //节点到自身代价为0
				else MAP[speed_cnt][i][j].total_road_cost = INT_MAX;  //到任意结点代价无限大 等待刷新
				MAP[speed_cnt][i][j].finished = 0;      //未完成路径标志位
				if ((MAP[speed_cnt][i][j].Road = (struct RouteRoad *)malloc(sizeof(struct RouteRoad))) == NULL)
				{
					fprintf(stderr, "No Enough Space for New RouteRoad !");
					exit(EXIT_FAILURE);
				}
				MAP[speed_cnt][i][j].Road->road_num = INT_MAX;
				MAP[speed_cnt][i][j].Road->next = NULL;
			}
		}
	//初始化地图，得到每一个相邻结点间的通行代价
	for (int speed_cnt = 0; speed_cnt < car_speed_cnt; speed_cnt++) //不同速度 
		for (int i = 0; i < road_total_num; i++)  // 每条路径
		{
			//更新代价       //更改  映射函数 一定注意 [i] 的变化！！！！！！一定注意！！！！！！一定注意！！！！！！  //通行此路花费代价  长度 / 最高速 
			MAP[speed_cnt][CROSS_ORD_SUB(road[i][4])][CROSS_ORD_SUB(road[i][5])].total_road_cost = (road[i][1] * 10) / MIN(SPEED_SUB_REAL(speed_cnt), road[i][2]);
			MAP[speed_cnt][CROSS_ORD_SUB(road[i][4])][CROSS_ORD_SUB(road[i][5])].Road->road_num = road[i][0]; //记录道路名字
			
			/*cout << road[i][4] << " --> " << road[i][5] << ": " << MAP[speed_cnt][CROSS_ORD_SUB(road[i][4])][CROSS_ORD_SUB(road[i][5])].Road->road_num <<":"
				 << MAP[speed_cnt][CROSS_ORD_SUB(road[i][4])][CROSS_ORD_SUB(road[i][5])].total_road_cost <<endl*/
			if (road[i][6] == 1) //双向
			{
				MAP[speed_cnt][CROSS_ORD_SUB(road[i][5])][CROSS_ORD_SUB(road[i][4])].total_road_cost = MAP[speed_cnt][CROSS_ORD_SUB(road[i][4])][CROSS_ORD_SUB(road[i][5])].total_road_cost;
				MAP[speed_cnt][CROSS_ORD_SUB(road[i][5])][CROSS_ORD_SUB(road[i][4])].Road->road_num = road[i][0]; //记录道路名字

				/*cout << road[i][5] << " --> " << road[i][4] << ": " << MAP[speed_cnt][CROSS_ORD_SUB(road[i][5])][CROSS_ORD_SUB(road[i][4])].Road->road_num << ":"
					<< MAP[speed_cnt][CROSS_ORD_SUB(road[i][5])][CROSS_ORD_SUB(road[i][4])].total_road_cost << endl;*/
			}
		}
}

void ShowRouteRoad(struct RouteRoad *r) //显示路线
{
	while(r != NULL)
	{
		cout << r->road_num << " -> ";
		r = r->next;
	} 
}

void CopyRouteRoad(struct RouteRoad *p1, struct RouteRoad *p2) //将r2路径复制到r1上
{
	struct RouteRoad *tmp = NULL, *lastp1 = NULL;
	while (p2 != NULL) //顺着r2复制路径
	{
		p1->road_num = p2->road_num;
		if (p1->next == NULL && p2->next != NULL)
		{
			if ((p1->next = (struct RouteRoad *)malloc(sizeof(struct RouteRoad))) == NULL)
			{
				fprintf(stderr, "No Enough Space for New Road !");
				exit(EXIT_FAILURE);
			}
			p1->next->next = NULL;
		}
		lastp1 = p1; //记录p1方便后续清空
		p1 = p1->next;
		p2 = p2->next;
	}
   //复制结束时r1后还有未清空的路径
	while (p1 != NULL )
	{
		tmp = p1->next;
		p1-> next = NULL;
		free(p1);
		p1 = tmp;
	}
	lastp1->next = NULL;//最后一格注意指针置NULL
}
void AddRouteRoad(struct RouteRoad *p1, struct RouteRoad *p2) //将r2路径 复制 加载到r1末尾
{
	while (p1->next != NULL)p1 = p1->next;  //直到r1路径的最后
	while (p2 != NULL) //顺着r2复制路径
	{
		if ((p1->next = (struct RouteRoad *)malloc(sizeof(struct RouteRoad))) == NULL)
			{
				fprintf(stderr, "No Enough Space for New Road !");
				exit(EXIT_FAILURE);
			}
		p1 = p1->next;
		p1->road_num = p2->road_num;
		p1->next = NULL;
		p2 = p2->next;
	}
}

void MAP_Floyd()
{
	for (int speed_cnt = 0; speed_cnt < car_speed_cnt; speed_cnt++)    //不同速度  
		for (int mid = 0; mid < cross_total_num; mid++)
			for (int i = 0; i < cross_total_num; i++)                  // 每个起点
				for (int j = 0; j < cross_total_num; j++)              // 每个终点
					if (INT_MAX - MAP[speed_cnt][i][mid].total_road_cost > MAP[speed_cnt][mid][j].total_road_cost)//注意溢出检测 
						if (MAP[speed_cnt][i][j].total_road_cost > MAP[speed_cnt][i][mid].total_road_cost + MAP[speed_cnt][mid][j].total_road_cost)
						{   
							//新路线代价更小   修改更短路径
							MAP[speed_cnt][i][j].total_road_cost = MAP[speed_cnt][i][mid].total_road_cost + MAP[speed_cnt][mid][j].total_road_cost;
							CopyRouteRoad(MAP[speed_cnt][i][j].Road, MAP[speed_cnt][i][mid].Road);
							AddRouteRoad(MAP[speed_cnt][i][j].Road, MAP[speed_cnt][mid][j].Road);
							//if (i == 13 && j == 11)
							//{
							//	cout << CROSS_SUB_ORD(i) << " -> " << CROSS_SUB_ORD(j) << "  at Speed " << SPEED_SUB_REAL(speed_cnt) << " : " << endl;
							//	ShowRouteRoad(MAP[speed_cnt][i][j].Road);
							//	cout << " TimeCost : " << MAP[speed_cnt][i][j].total_road_cost << endl;
							//}
						}
}

