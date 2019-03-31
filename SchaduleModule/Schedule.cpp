#include <iostream>
#include "Mapping.h"
#include "Read.h"
#include "Schedule.h"

//-------------
#define DUPLEX 1
#define SINGLE 0
//-------------
#define NO_ROAD -1 
#define NO_CAR -1
#define IS_CAR 1
#define UP_ROAD 0
#define RIGHT_ROAD 1
#define DOWN_ROAD 2
#define LEFT_ROAD 3
//------------- 路口车辆的方向
#define GO_STRAIGHT 0
#define TURN_LEFT 1
#define TURN_RIGHT 2
//-------------
#define END_STATUS 0
#define WAIT_STATUS 1
//-------------  用于道路方向
#define FORWARD 0
#define BACKWARD 1
//-------------
#define FINISHED 1
#define UNFINISHED 0
//--------------
#define NO_CONFLICT 0
#define CONFLICT 1

//直接给出方向数组加快 路口冲突判定 但无法掉头设定为INT_MAX   也可下标相减 switch 得到方向
using namespace std;
const int DIRECTION_MAP[4][4] = {{INT_MAX,TURN_LEFT,GO_STRAIGHT,TURN_RIGHT},
                                 {TURN_RIGHT,INT_MAX,TURN_LEFT,GO_STRAIGHT},
                                 {GO_STRAIGHT,TURN_RIGHT,INT_MAX,TURN_LEFT},
                                 {TURN_LEFT,GO_STRAIGHT,TURN_RIGHT,INT_MAX} };


class CAR
{
public:
	int car_order = INT_MAX;
	int car_start_cross = INT_MAX;
	int car_end_cross = INT_MAX;
	int car_speed = INT_MAX;
	int car_start_time = INT_MAX;
	int car_status = WAIT_STATUS;
	int car_current_slot = INT_MAX;//车辆当前位置
	int car_next_slot = INT_MAX; //车辆下一时刻的位置
	int car_current_speed = 0;   //当前车道上的最大行驶速度
	int car_current_road = INT_MAX; //车辆所在道路编号
	int car_next_road = INT_MAX;  //下一路段编号
	int car_current_direction = GO_STRAIGHT; //存储当前的转向
	int car_schedule_finished = UNFINISHED;    //完成T时刻内调度标志
	int car_route_finish = UNFINISHED;//车辆到达目标标志
	Route *car_real_route = NULL; //车辆实际行驶的路径

	CAR(int *car);
	static CAR **BuildEmptyCar(int car_num);
};
CAR ** CAR::BuildEmptyCar(int car_num)
{
	CAR ** tmp = NULL;
	if ((tmp = (CAR **)malloc(sizeof(CAR *) * car_num)) == NULL)
		fprintf(stderr, "No Space!!\n");
	for (int i = 0; i < car_num; i++)
		tmp[i] = NULL;
	return tmp;
}
CAR::CAR(int *car)
{
	this->car_order = car[0];
	this->car_start_cross = car[1];
	this->car_end_cross = car[2];
	this->car_speed = car[3];
	this->car_start_time = car[4];
	this->car_status = WAIT_STATUS;
	this->car_schedule_finished = UNFINISHED;    //完成T时刻内调度标志
	this->car_current_slot = INT_MAX;
	this->car_route_finish = UNFINISHED;     //车辆到达目标标志
	this->car_current_direction = GO_STRAIGHT; //存储当前的转向
	if ((this->car_real_route = (Route *)malloc(sizeof(Route))) == NULL)
		fprintf(stderr, "No Space!!\n"); 
	if ((this->car_real_route->Road = (RouteRoad *)malloc(sizeof(RouteRoad))) == NULL)
		fprintf(stderr, "No Space!!\n");
	car_real_route->Road->next = NULL; //不赋NULL 无法拷贝 谨记！！！！
	CopyRouteRoad(car_real_route->Road, MAP[SPEED_REAL_SUB(car_speed)][CROSS_ORD_SUB(car_start_cross)][CROSS_ORD_SUB(car_end_cross)].Road);//将r2路径复制到r1上
	car_real_route->total_road_cost = MAP[SPEED_REAL_SUB(car_speed)][CROSS_ORD_SUB(car_start_cross)][CROSS_ORD_SUB(car_end_cross)].total_road_cost;
}
//--car-- 0车编号  1起点  2终点  3速度  4计划出发时间-----
class ROAD
{
public :
	int road_ord = INT_MAX;
	int road_len = INT_MAX;
	int road_speed = INT_MAX;
	int road_start_cross = INT_MAX;
	int road_lane_num = INT_MAX;
	int road_end_cross = INT_MAX;
	int road_duplex = DUPLEX;
	int frontest_car_direction[2] = { NO_CAR ,NO_CAR};
	CAR ****car_slot = NULL;  //道路上车辆槽位
	ROAD(int *road);
	static ROAD **BuildEmptyRoad(int road_num);
};
ROAD ** ROAD::BuildEmptyRoad(int road_num)
{
	ROAD ** tmp = NULL;
	if ((tmp = (ROAD **)malloc(sizeof(ROAD *) * road_num)) == NULL)
		fprintf(stderr, "No Space!!\n");
	for (int i = 0; i < road_num; i++)
		tmp[i] = NULL;
	return tmp;
}
ROAD::ROAD(int *road)
{
	this->road_ord = road[0];
	this->road_len = road[1];
	this->road_speed = road[2];
	this->road_lane_num = road[3];
	this->road_start_cross = road[4];
	this->road_end_cross = road[5];
	this->road_duplex = road[6];
	this->frontest_car_direction[0] = NO_CAR;
	this->frontest_car_direction[0] = NO_CAR;
	if (this->road_duplex == DUPLEX) //双向车道 
	{
		if ((this->car_slot = (CAR ****)malloc(sizeof(CAR ***) * 2)) == NULL)
			fprintf(stderr, "No Space!!\n");   //双向  0正 1负
		for (int i = 0; i < 2; i++)
		{
			if ((this->car_slot[i] = (CAR ***)malloc(sizeof(CAR **) * this->road_lane_num)) == NULL)
				fprintf(stderr, "No Space!!\n");   //车道
			for (int j = 0; j < road_lane_num; j++)
			{
				if ((this->car_slot[i][j] = (CAR **)malloc(sizeof(CAR *) * this->road_len)) == NULL)
					fprintf(stderr, "No Space!!\n");  //每条车道上的车辆槽位
				for (int k = 0; k < road_len; k++)
					car_slot[i][j][k] = NULL;
			}
		}
	}
	else    //单向车道
	{
		if ((this->car_slot = (CAR ****)malloc(sizeof(CAR ***))) == NULL)
			fprintf(stderr, "No Space!!\n");   //单向  0正
		if ((this->car_slot[0] = (CAR ***)malloc(sizeof(CAR **) * this->road_lane_num)) == NULL)
				fprintf(stderr, "No Space!!\n");   //车道
		for (int j = 0; j < road_lane_num; j++)
		{
			if ((this->car_slot[0][j] = (CAR **)malloc(sizeof(CAR *) * this->road_len)) == NULL)
				fprintf(stderr, "No Space!!\n");  //每条车道上的车辆槽位
			for (int k = 0; k < road_len; k++)
				car_slot[0][j][k] = NULL;
		}
	}
	//[len-1][len-2]...[0]  路口[-1]
}
//--road---0路编号  1长  2速度  3车道数  4起始节点  5终止节点  6双向标记-----

class CROSS
{
public:
	int cross_ord = INT_MAX;
	int start_car_num = 0; //记录从此路口出发的车辆数目
	int *start_car = NULL; //出发车辆数组
	int start_car_cnt = 0; //发车计数
	int cross_locked_time = 0;
	ROAD **road_in_cross = NULL; // 0上  1右  2下  3左
	int travel_order[4] = { INT_MAX }; //道路遍历升序 存储的是方向序号 
	CROSS(int *cross, ROAD **road);
	static CROSS **BuildEmptyCross(int cross_num);
};
//----------------------
CROSS ** CROSS::BuildEmptyCross(int cross_num)
{
	CROSS ** tmp = NULL;
	if ((tmp = (CROSS **)malloc(sizeof(CROSS *) * cross_num)) == NULL)
		fprintf(stderr, "No Space!!\n");
	for (int i = 0; i < cross_num; i++)
		tmp[i] = NULL;
	return tmp;
}
//-----------

CROSS::CROSS(int *cross, ROAD **road)
{
	this->start_car_num = 0;
	this->cross_ord = cross[0];
	this->start_car_cnt = 0;
	if ((this->road_in_cross = (ROAD **)malloc(sizeof(ROAD *) * 4)) == NULL)//每个路口有四个指针
		fprintf(stderr, "No Space!!\n");
	//--------序号表初始化----------
	int ordertable[4][2] = {0};
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 2; j++)
			ordertable[i][j] = INT_MAX;
	//--------读入相邻道路信息----------
	for (int cross_road_cnt = 0; cross_road_cnt < 4; cross_road_cnt++)
	{
		if (cross[cross_road_cnt + 1] == NO_ROAD) //此方向没有Road
		{
			this->road_in_cross[cross_road_cnt] = NULL;                       //道路不存在 -> 指针=NULL -> 序号=INT_MAX
			ordertable[cross_road_cnt][0] = INT_MAX;
		}
		else
		{
			this->road_in_cross[cross_road_cnt] =
				road[ROAD_ORD_SUB(cross[cross_road_cnt + 1])]; //指针指向相应的Road
			ordertable[cross_road_cnt][0] = road_in_cross[cross_road_cnt]->road_ord;
		}
		ordertable[cross_road_cnt][1] = cross_road_cnt;
	}
	//冒泡排序确定遍历顺序 不存在的道路放在最后
	int tmp = 0; 
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3 - i; j++)
		{
			if (ordertable[j][0] > ordertable[j + 1][0])
			{
				tmp = ordertable[j + 1][0];  //交换序号
				ordertable[j + 1][0] = ordertable[j][0];
				ordertable[j][0] = tmp;
				tmp = ordertable[j + 1][1];  //交换编号
				ordertable[j + 1][1] = ordertable[j][1];
				ordertable[j][1] = tmp;
			}
		}
	for (int i = 0; i < 4; i++) //升序 路口方向 // 上  右  下  左  无道路
		this->travel_order[i] = ordertable[i][1]; 
}
void StartCarSort(CAR **car, CROSS *cross);//升序时间>序号冒泡
int RoadDirectionInCross(int road_ord,CROSS *cross); //道路在路口中的方位 0上 1右 2下 3坐
int TravelDirection(ROAD *road, CROSS *cross); //遍历的方向
int IntoRoadTravelDiraction(ROAD *road, CROSS *cross);//进入车道的搜索方向
int DirectionConflict(ROAD *road, CROSS *cross); //从road经过cross的车辆 转向冲突判定
int FindCarNextRoad(int road, RouteRoad *r); //输入当前道路得到下一条道路

void ScheduleModule(int **car, int **road, int **cross)
{
	CAR **Car = CAR::BuildEmptyCar(car_total_num);
	ROAD **Road = ROAD::BuildEmptyRoad(road_total_num); //建造路的指针数组空间
	CROSS **Cross = CROSS::BuildEmptyCross(cross_total_num);  //建造路口指针数组空间
	//初始化路
	for (int road_cnt = 0; road_cnt < road_txt_num; road_cnt++)
		Road[ROAD_ORD_SUB(road[road_cnt][0])] = new ROAD(road[road_cnt]);
	//初始化路口
	for (int cross_cnt = 0; cross_cnt < cross_txt_num; cross_cnt++)
		Cross[CROSS_ORD_SUB(cross[cross_cnt][0])] = new CROSS(cross[cross_cnt], Road);
	//初始化车辆

	for (int car_cnt = 0; car_cnt < car_txt_num; car_cnt++)
	{//车辆出发地点统计
		Car[CAR_ORD_SUB(car[car_cnt][0])] = new CAR(car[car_cnt]);
		Cross[CROSS_ORD_SUB(car[car_cnt][1])]->start_car_num++;
	}
	for (int cross_cnt = 0; cross_cnt < cross_txt_num; cross_cnt++)
	{//开辟车库
		if ((Cross[CROSS_ORD_SUB(cross[cross_cnt][0])]->start_car = (int *)malloc(sizeof(int) * Cross[CROSS_ORD_SUB(cross[cross_cnt][0])]->start_car_num)) == NULL)
			fprintf(stderr, "No Space!!\n");
		for (int i = 0; i < Cross[CROSS_ORD_SUB(cross[cross_cnt][0])]->start_car_num; i++)
			Cross[CROSS_ORD_SUB(cross[cross_cnt][0])]->start_car[i] = NO_CAR; //默认没有车
		Cross[CROSS_ORD_SUB(cross[cross_cnt][0])]->start_car_num = 0; //车辆计数归零
	}
	for (int car_cnt = 0; car_cnt < car_txt_num; car_cnt++)
	{   //车辆入库 初始化车辆信息
		Cross[CROSS_ORD_SUB(car[car_cnt][1])]->start_car[Cross[CROSS_ORD_SUB(car[car_cnt][1])]->start_car_num] = car[car_cnt][0];
		Car[CAR_ORD_SUB(car[car_cnt][0])]->car_current_road = Car[CAR_ORD_SUB(car[car_cnt][0])]->car_real_route->Road->road_num;
		Car[CAR_ORD_SUB(car[car_cnt][0])]->car_schedule_finished = UNFINISHED;
		Car[CAR_ORD_SUB(car[car_cnt][0])]->car_current_speed = MIN(car[car_cnt][3], Road[ROAD_ORD_SUB(Car[CAR_ORD_SUB(car[car_cnt][0])]->car_current_road)]->road_speed);
		Car[CAR_ORD_SUB(car[car_cnt][0])]->car_next_slot = Road[ROAD_ORD_SUB(Car[CAR_ORD_SUB(car[car_cnt][0])]->car_current_road)]->road_len - Car[CAR_ORD_SUB(car[car_cnt][0])]->car_current_speed;
		Cross[CROSS_ORD_SUB(car[car_cnt][1])]->start_car_num++;
	}

	for (int cross_cnt = 0; cross_cnt < cross_total_num; cross_cnt++)
	{
		StartCarSort(Car, Cross[cross_cnt]); //整理每条道路发车优先级
		Cross[cross_cnt]->start_car_cnt = 0; //从0开始按优先级发车  计数归0
	}
	
	int FinishedFlag = UNFINISHED;
	while (FinishedFlag == UNFINISHED)
	{
		FinishedFlag = FINISHED;
		int ScheduleTime = 0; //全局调度总计用时初始化
		int ScheduleFinishedFlag = UNFINISHED; //全局调度结束
		int FinishedCar = 0;
		while (ScheduleFinishedFlag == UNFINISHED)
		{
			ScheduleFinishedFlag = FINISHED;
			//STEP1  标记所有车辆的状态 
			for (int cross_cnt = 0; cross_cnt < cross_total_num; cross_cnt++) //对于每条
			{
				for (int road_cnt = 0; road_cnt < 4; road_cnt++) //每个路口最多4条道
				{
					if (TravelDirection(Cross[cross_cnt]->road_in_cross[road_cnt], Cross[cross_cnt]) != NO_ROAD)//路口可以遍历
					{
						int DrectionConfirmFlag = UNFINISHED;
						int travel_direction = TravelDirection(Cross[cross_cnt]->road_in_cross[road_cnt], Cross[cross_cnt]); //遍历的方向
						CAR **CarForward;
						if ((CarForward = (CAR **)malloc(sizeof(CAR *) * Cross[cross_cnt]->road_in_cross[road_cnt]->road_lane_num)) == NULL)//前车存储
							fprintf(stderr, "No Space!!\n");
						for (int lane_cnt = 0; lane_cnt < Cross[cross_cnt]->road_in_cross[road_cnt]->road_lane_num; lane_cnt++)
							CarForward[lane_cnt] = NULL;

						Cross[cross_cnt]->road_in_cross[road_cnt]->frontest_car_direction[travel_direction] = NO_CAR;

						for (int car_slot_cnt = 0; car_slot_cnt < Cross[cross_cnt]->road_in_cross[road_cnt]->road_len; car_slot_cnt++)
						{ //每条道上的每个槽位    [len-1][len-2]...[0]  路口[-1]
							for (int lane_cnt = 0; lane_cnt < Cross[cross_cnt]->road_in_cross[road_cnt]->road_lane_num; lane_cnt++)
							{  //每条道路
								if (Cross[cross_cnt]->road_in_cross[road_cnt]->car_slot[travel_direction][lane_cnt][car_slot_cnt] != NULL)//某方向某车道上某槽位上有车
								{
									CAR *temp_car = Cross[cross_cnt]->road_in_cross[road_cnt]->car_slot[travel_direction][lane_cnt][car_slot_cnt];

									ScheduleFinishedFlag = UNFINISHED;

									temp_car->car_schedule_finished = UNFINISHED;  //刷新调度标志 所有的车辆均未调度

									//当前道路上 该车位车辆可行驶的最大速度
									temp_car->car_current_speed = MIN(temp_car->car_speed, Cross[cross_cnt]->road_in_cross[road_cnt]->road_speed);

									//当前车位
									temp_car->car_current_slot = car_slot_cnt;

									//下一时刻车辆 在没有阻挡的情况下  能够行驶到的位置
									temp_car->car_next_slot = car_slot_cnt - temp_car->car_current_speed;

									if (FindCarNextRoad(temp_car->car_current_road, temp_car->car_real_route->Road) == NO_ROAD)//输入当前道路得到下一条道路
										temp_car->car_route_finish = FINISHED; //抵达终点标志


									if (CarForward[lane_cnt] == NULL)
									{
										if (temp_car->car_next_slot < 0)//车自身速度能够出路口
										{
											if (temp_car->car_route_finish == FINISHED)
											{
												temp_car->car_status = WAIT_STATUS;//车辆标记为等待
												temp_car->car_current_direction = GO_STRAIGHT;
												if (DrectionConfirmFlag == UNFINISHED)
												{
													Cross[cross_cnt]->road_in_cross[road_cnt]->frontest_car_direction[travel_direction] = temp_car->car_current_direction;
													DrectionConfirmFlag = FINISHED;
												}
											}
											else
												if (Road[ROAD_ORD_SUB(temp_car->car_next_road)]->road_speed > temp_car->car_current_slot)
												{
													temp_car->car_status = WAIT_STATUS;//车辆标记为等待
													temp_car->car_current_direction =
														DIRECTION_MAP[RoadDirectionInCross(temp_car->car_current_road, Cross[cross_cnt])][RoadDirectionInCross(temp_car->car_next_road, Cross[cross_cnt])];
													if (DrectionConfirmFlag == UNFINISHED)
													{
														Cross[cross_cnt]->road_in_cross[road_cnt]->frontest_car_direction[travel_direction] = temp_car->car_current_direction;
														DrectionConfirmFlag = FINISHED;
													}
												}
												else
												{
													temp_car->car_status = END_STATUS;
													temp_car->car_next_slot = 0;
												}
										}
										else   //速度不够出路口 前方没车
											temp_car->car_status = END_STATUS;
									}
									else //前方有车
									{
										if (temp_car->car_next_slot <= CarForward[lane_cnt]->car_current_slot) //被阻挡
										{
											if (CarForward[lane_cnt]->car_status == WAIT_STATUS)
												temp_car->car_status = WAIT_STATUS;
											else
											{
												temp_car->car_status = END_STATUS;
												temp_car->car_next_slot = CarForward[lane_cnt]->car_current_slot + 1;
											}
										}
										else
											temp_car->car_status = END_STATUS;
									}
									//---------前车刷新一定在最后
									CarForward[lane_cnt] = temp_car;//刷新前车
								}
							}
						}
						free(CarForward); //记得释放内存!
					}

				}

			}
			//STEP2  调度等待车辆  
			int Schedule_In_T_Finishied = UNFINISHED;
			while (Schedule_In_T_Finishied == UNFINISHED)
			{
				Schedule_In_T_Finishied = FINISHED;
				for (int cross_cnt = 0; cross_cnt < cross_total_num; cross_cnt++)
				{
					int DirectionConflictFlag = CONFLICT;
					int NextRoadWait = NO_CONFLICT;
					while (DirectionConflictFlag == CONFLICT)
					{
						for (int road_cnt = 0; road_cnt < 4; road_cnt++) //每个路口最多4条道
						{
							DirectionConflictFlag = NO_CONFLICT;
							if (TravelDirection(Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]], Cross[cross_cnt]) != NO_ROAD)
							{
								int travel_direction = TravelDirection(Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]], Cross[cross_cnt]); //遍历的方向
								CAR **CarForward;
								if ((CarForward = (CAR **)malloc(sizeof(CAR *) * Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->road_lane_num)) == NULL)//前车  状态判定
									fprintf(stderr, "No Space!!\n");
								for (int lane_cnt = 0; lane_cnt < Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->road_lane_num; lane_cnt++)
									CarForward[lane_cnt] = NULL;

								for (int car_slot_cnt = 0; car_slot_cnt < Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->road_len; car_slot_cnt++)
								{
									for (int lane_cnt = 0; lane_cnt < Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->road_lane_num; lane_cnt++)
									{
										if (Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->car_slot[travel_direction][lane_cnt][car_slot_cnt] != NULL)//某方向某车道上某槽位上有车
										{ //某车位上有车
											CAR *temp_car = Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->car_slot[travel_direction][lane_cnt][car_slot_cnt];

											if (temp_car->car_schedule_finished == UNFINISHED)
											{
												Schedule_In_T_Finishied = UNFINISHED;

												if (temp_car->car_status == WAIT_STATUS)
												{
													if (car_slot_cnt < temp_car->car_current_speed)     //S1 < V1 能过路口 
													{
														if (CarForward[lane_cnt] == NULL) //前方没有车  
														{
															if (temp_car->car_route_finish == FINISHED) //到达终点
															{   //刷新道路状态
																Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->frontest_car_direction[travel_direction] = NO_CAR;
																Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->car_slot[travel_direction][lane_cnt][car_slot_cnt] = NULL;
																//刷新车辆状态 
																cout << temp_car->car_order << endl;
																temp_car->car_schedule_finished = FINISHED;
																temp_car->car_status = END_STATUS;
																cout << FinishedCar++ << "" << endl;
																break; //下一辆车
															}
															else
															{   //未到达终点 V2限速  可以出路口
																if (Road[ROAD_ORD_SUB(temp_car->car_next_road)]->road_speed > car_slot_cnt)
																{
																	if (DirectionConflict(Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]], Cross[cross_cnt]) == CONFLICT)
																	{
																		DirectionConflictFlag = CONFLICT;
																		break;  //有冲突 判断另一条道路
																	}

																	int IntoDrection = IntoRoadTravelDiraction(Road[ROAD_ORD_SUB(temp_car->car_next_road)], Cross[cross_cnt]);

																	if (IntoDrection != NO_ROAD) //双向 或单向起点
																	{
																		int temp_speed = MIN(temp_car->car_speed, Road[ROAD_ORD_SUB(temp_car->car_next_road)]->road_speed); //可行驶的最大速度
																		temp_car->car_next_slot = Road[ROAD_ORD_SUB(temp_car->car_next_road)]->road_len - (temp_speed - car_slot_cnt); //在下一条路上的位置

																		for (int next_lane_cnt = 0; next_lane_cnt < Road[ROAD_ORD_SUB(temp_car->car_next_road)]->road_lane_num; next_lane_cnt++)
																		{
																			for (int next_slot_cnt = Road[ROAD_ORD_SUB(temp_car->car_next_road)]->road_len - 1; next_slot_cnt >= temp_car->car_next_slot; next_slot_cnt--)
																			{
																				//要从后往前搜，以免出现路口超车
																				if (Road[ROAD_ORD_SUB(temp_car->car_next_road)]->car_slot[IntoDrection][next_lane_cnt][next_slot_cnt] != NULL)//若车位有车 
																				{
																					if (Road[ROAD_ORD_SUB(temp_car->car_next_road)]->car_slot[IntoDrection][next_lane_cnt][next_slot_cnt]->car_status == END_STATUS)
																					{
																						//该车终止状态才可以紧贴
																						if (next_slot_cnt < Road[ROAD_ORD_SUB(temp_car->car_next_road)]->road_len - 1) //该车道没有满
																						{
																							//刷新道路状态    没有冲突 方向可以刷新
																							Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->frontest_car_direction[travel_direction] = NO_CAR;
																							Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->car_slot[travel_direction][lane_cnt][car_slot_cnt] = NULL;
																							Road[ROAD_ORD_SUB(temp_car->car_next_road)]->car_slot[IntoDrection][next_lane_cnt][next_slot_cnt + 1] = temp_car;

																							//刷新车辆状态 
																							temp_car->car_status = END_STATUS;
																							temp_car->car_current_road = temp_car->car_next_road;
																							temp_car->car_current_speed = MIN(temp_car->car_speed, Road[ROAD_ORD_SUB(temp_car->car_next_road)]->road_speed);
																							temp_car->car_current_slot = next_slot_cnt + 1;
																							temp_car->car_next_road = FindCarNextRoad(temp_car->car_current_road, temp_car->car_real_route->Road);  //下一路段编号
																							temp_car->car_schedule_finished = FINISHED;
																							break;
																						}
																						else
																							break; //该车道已经满搜索下一个车道
																					}
																					else //下一车道车辆也处于等待状态  
																					{
																						Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->frontest_car_direction[travel_direction] = temp_car->car_current_direction;
																						CarForward[lane_cnt] = temp_car;  //刷新前车
																						NextRoadWait = CONFLICT;
																						LockedTime++; 
																						//cout << "下个车道有等待" << endl;

																						break;
																						//该车状态不动，等待下次调度 由于前车等待使得该车等待!!可能出现死锁 !!!!
																					}
																				}
																				else
																					if (next_slot_cnt == temp_car->car_next_slot) //该条车道到顶没有其他车辆
																					{
																						//刷新道路状态
																						Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->frontest_car_direction[travel_direction] = NO_CAR;
																						Road[ROAD_ORD_SUB(temp_car->car_next_road)]->car_slot[IntoDrection][next_lane_cnt][next_slot_cnt] = temp_car;
																						Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->car_slot[travel_direction][lane_cnt][car_slot_cnt] = NULL;

																						//刷新车辆状态 
																						temp_car->car_status = END_STATUS;
																						temp_car->car_current_road = temp_car->car_next_road;
																						temp_car->car_current_speed = MIN(temp_car->car_speed, Road[ROAD_ORD_SUB(temp_car->car_next_road)]->road_speed);
																						temp_car->car_current_slot = next_slot_cnt;
																						temp_car->car_next_road = FindCarNextRoad(temp_car->car_current_road, temp_car->car_real_route->Road);  //下一路段编号
																						temp_car->car_schedule_finished = FINISHED;
																						break;
																					}
																			}
																			if (temp_car->car_schedule_finished == FINISHED) break;  //该车调度结束 下一辆车
																			if (NextRoadWait == CONFLICT)break; //等待冲突 不做处理 直接调度下一个路口 再返回调度该路口
																		}
																		if (DirectionConflictFlag == CONFLICT)break; //若有冲突跳出这条道路直接遍历下一条道路
																		if (NextRoadWait == CONFLICT)break;     //等待冲突 不做处理 直接调度下一个路口 再返回调度该路口
																		if (temp_car->car_schedule_finished == UNFINISHED) //车道遍历 结束 下一道路车道满 没有车位 
																		{
																			//刷新道路状态
																			Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->frontest_car_direction[travel_direction] = NO_CAR;
																			Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->car_slot[travel_direction][lane_cnt][car_slot_cnt] = NULL;
																			Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->car_slot[travel_direction][lane_cnt][0] = temp_car;

																			//刷新车辆状态 
																			temp_car->car_status = END_STATUS;
																			temp_car->car_schedule_finished = FINISHED;
																			temp_car->car_current_slot = 0;
																			CarForward[lane_cnt] = temp_car;  //刷新前车
																			continue; //下一辆车 调度
																		}
																	}
																	else
																		printf("车辆单向车道逆行!!");
																}
																else //V2限速 不出路口 前方没车
																{
																	//刷新道路状态
																	Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->frontest_car_direction[travel_direction] = NO_CAR;
																	Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->car_slot[travel_direction][lane_cnt][car_slot_cnt] = NULL;
																	Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->car_slot[travel_direction][lane_cnt][0] = temp_car;
																	//刷新车辆状态 
																	temp_car->car_status = END_STATUS;
																	temp_car->car_schedule_finished = FINISHED;
																	temp_car->car_current_slot = 0;
																	CarForward[lane_cnt] = temp_car;  //刷新前车  终止
																	break;
																}
															}
														}
														else
															if (CarForward[lane_cnt]->car_status == END_STATUS)//终止态 被挡住出不去
															{
																//刷新道路状态
																Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->frontest_car_direction[travel_direction] = NO_CAR;
																Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->car_slot[travel_direction][lane_cnt][car_slot_cnt] = NULL;
																Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->car_slot[travel_direction][lane_cnt][CarForward[lane_cnt]->car_current_slot + 1] = temp_car;
																//刷新车辆状态 
																temp_car->car_schedule_finished = FINISHED;
																temp_car->car_status = END_STATUS;
																temp_car->car_next_slot = CarForward[lane_cnt]->car_current_slot + 1;
																CarForward[lane_cnt] = temp_car;  //刷新前车
																break; //下一辆车
															}
															else  //等待状态
															{
																Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->frontest_car_direction[travel_direction] = temp_car->car_current_direction;
																CarForward[lane_cnt] = temp_car;  //刷新前车
																NextRoadWait = CONFLICT;
																//cout << "前车有等待" << endl;
																break;
																//该车状态不动，等待下次调度 由于前车等待使得该车等待!!可能出现死锁 !!!!
															}
													}
													else // 不能出路口 后方等待
													{
														temp_car->car_next_slot = temp_car->car_current_slot - temp_car->car_current_speed;
														if ((CarForward[lane_cnt] != NULL) && (temp_car->car_next_slot <= CarForward[lane_cnt]->car_current_slot))
														{
															if (CarForward[lane_cnt]->car_status == END_STATUS)//终止态 被挡住
															{
																//刷新道路状态
																Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->car_slot[travel_direction][lane_cnt][car_slot_cnt] = NULL;
																Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->car_slot[travel_direction][lane_cnt][CarForward[lane_cnt]->car_current_slot + 1] = temp_car;
																//刷新车辆状态 
																temp_car->car_schedule_finished = FINISHED;
																temp_car->car_status = END_STATUS;
																temp_car->car_next_slot = CarForward[lane_cnt]->car_current_slot + 1;
																CarForward[lane_cnt] = temp_car;  //刷新前车
																break; //下一辆车
															}
															else  //等待状态
															{
																CarForward[lane_cnt] = temp_car;  //刷新前车
																NextRoadWait = CONFLICT;
																//cout << "前车有等待" << endl;
																break;
																//该车状态不动，等待下次调度 由于前车等待使得该车等待!!可能出现死锁 !!!!
															}
														}
														else //无阻挡  或没有车
														{
															//刷新道路状态
															Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->car_slot[travel_direction][lane_cnt][car_slot_cnt] = NULL;
															Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->car_slot[travel_direction][lane_cnt][temp_car->car_next_slot] = temp_car;
															//刷新车辆状态 
															temp_car->car_status = END_STATUS;
															temp_car->car_current_slot = temp_car->car_next_slot;
															temp_car->car_schedule_finished = FINISHED;
															CarForward[lane_cnt] = temp_car;  //刷新前车
															break; //该车调度结束，调度下一辆车
														}
													}
												}
												else //END状态的车辆 temp_car->car_next_slot 已经被计算过 行驶后仍然在当前车道
												{
													//刷新道路状态
													Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->car_slot[travel_direction][lane_cnt][car_slot_cnt] = NULL;
													Cross[cross_cnt]->road_in_cross[Cross[cross_cnt]->travel_order[road_cnt]]->car_slot[travel_direction][lane_cnt][temp_car->car_next_slot] = temp_car;
													//刷新车辆状态 
													temp_car->car_status = END_STATUS;
													temp_car->car_current_slot = temp_car->car_next_slot;
													temp_car->car_schedule_finished = FINISHED;
													CarForward[lane_cnt] = temp_car;  //刷新前车
													break; //该车调度结束，调度下一辆车)
												}
											}
											if (DirectionConflictFlag == CONFLICT)break; //若有冲突跳出这条道路直接遍历下一条道路
											if (NextRoadWait == CONFLICT)break; //当前道路锁死 等待下次调度
										}
									}
									if (DirectionConflictFlag == CONFLICT)break; //若有冲突跳出这条道路直接遍历下一条道路
									if (NextRoadWait == CONFLICT)break; //当前道路锁死 等待下次调度
								}
								free(CarForward); //记得释放内存！！！！
								CarForward = NULL;
								if (NextRoadWait == CONFLICT)break; //当前道路锁死 等待下次调度
							}
						}
					}
				}
			}
			//STEP3 新车上路
			for (int cross_cnt = 0; cross_cnt < cross_total_num; cross_cnt++)
			{
				while (Cross[cross_cnt]->start_car_cnt < Cross[cross_cnt]->start_car_num)//该车道车辆没有发车完毕
				{
					ScheduleFinishedFlag = UNFINISHED;
					if (Car[CAR_ORD_SUB(Cross[cross_cnt]->start_car[Cross[cross_cnt]->start_car_cnt])]->car_start_time <= ScheduleTime)//到点发车
					{
						CAR *Start_Car_temp = Car[CAR_ORD_SUB(Cross[cross_cnt]->start_car[Cross[cross_cnt]->start_car_cnt])];

						int IntoDrection = IntoRoadTravelDiraction(Road[ROAD_ORD_SUB(Start_Car_temp->car_current_road)], Cross[cross_cnt]);

						if (IntoDrection != NO_ROAD)
						{
							for (int next_lane_cnt = 0; next_lane_cnt < Road[ROAD_ORD_SUB(Start_Car_temp->car_current_road)]->road_lane_num; next_lane_cnt++)
							{
								for (int next_slot_cnt = Road[ROAD_ORD_SUB(Start_Car_temp->car_current_road)]->road_len - 1; next_slot_cnt >= Start_Car_temp->car_next_slot; next_slot_cnt--)
								{
									//要从后往前搜，以免出现路口超车
									if (Road[ROAD_ORD_SUB(Start_Car_temp->car_current_road)]->car_slot[IntoDrection][next_lane_cnt][next_slot_cnt] != NULL)//若车位有车 
									{
										if (next_slot_cnt < Road[ROAD_ORD_SUB(Start_Car_temp->car_current_road)]->road_len - 1) //该车道没有满
										{
											//刷新道路状态
											Road[ROAD_ORD_SUB(Start_Car_temp->car_current_road)]->car_slot[IntoDrection][next_lane_cnt][next_slot_cnt + 1] = Start_Car_temp;
											//刷新车辆状态 
											Cross[cross_cnt]->start_car_cnt++; //成功发车计数
											Start_Car_temp->car_status = END_STATUS;
											Start_Car_temp->car_current_slot = next_slot_cnt + 1;
											Start_Car_temp->car_next_road = FindCarNextRoad(Start_Car_temp->car_current_road, Start_Car_temp->car_real_route->Road);  //下一路段编号
											Start_Car_temp->car_schedule_finished = FINISHED;
											break;
										}
										else //该车道满 下一车道
											break;
									}
									else
										if (next_slot_cnt == Start_Car_temp->car_next_slot) //该条车道到顶没有其他车辆
										{
											Road[ROAD_ORD_SUB(Start_Car_temp->car_current_road)]->car_slot[IntoDrection][next_lane_cnt][Start_Car_temp->car_next_slot] = Start_Car_temp;
											//刷新车辆状态 
											Cross[cross_cnt]->start_car_cnt++; //成功发车计数
											Start_Car_temp->car_status = END_STATUS;
											Start_Car_temp->car_current_slot = next_slot_cnt;
											Start_Car_temp->car_next_road = FindCarNextRoad(Start_Car_temp->car_current_road, Start_Car_temp->car_real_route->Road);  //下一路段编号
											Start_Car_temp->car_schedule_finished = FINISHED;
											break;
										}
								}
								if (Start_Car_temp->car_schedule_finished == FINISHED) break;//该车调度结束 下一辆车
							}
						}
						else
							printf("发车方向不对!");
						//全部搜索完毕没有车位 则不能发车 
						if (Start_Car_temp->car_schedule_finished == UNFINISHED)break; //跳出发车循环
					}
					else
						break;//没到点
				}
			}
			ScheduleTime++;
			//cout<< ScheduleTime <<endl;
			/*for (int i = 0; i < car_txt_num; i++)
			if (Car[i]->car_current_road==5051)
			{
				cout << Car[i]->car_order << ":" << Car[i] ->car_current_road << ":" <<  " --- " << Car[i]->car_current_slot << endl;
			}*/
			//cout << endl;
			//cout << Car[83]->car_order << ":" << Car[83]->car_current_road << ":" << " --- " << Car[83]->car_current_slot << endl;
			//cout << Car[87]->car_order << ":" << Car[87]->car_current_road << ":" << " --- " << Car[87]->car_current_slot << endl;
		}
		for (int i = 0; i < car_total_num; i++)
			if (Car[i] != NULL && Car[i]->car_route_finish != FINISHED)
			{
				cout << Car[i]->car_order << endl;
				ShowRouteRoad(Car[i]->car_real_route->Road);
				cout << " --- " << Car[i]->car_next_road << endl;
			}
		cout << endl;
		//ShowRouteRoad(Car[10]->car_real_route->Road);
		//ShowRouteRoad(Car[87]->car_real_route->Road);
	}
}
int RoadDirectionInCross(int road_ord,CROSS *cross) //道路在路口中的方位 0上 1右 2下 3坐
{
	for (int cnt = 0; cnt < 4; cnt++)
		if (cross->road_in_cross[cnt] != NULL) //道路存在
			if (cross->road_in_cross[cnt]->road_ord == road_ord)
				return cnt;  
	return NO_ROAD;
}
int TravelDirection(ROAD *road, CROSS *cross)
{
	if (road != NULL)
	{
		if (road->road_duplex == DUPLEX)//双向
		{
			if (road->road_end_cross == cross->cross_ord) //该路口为终点
				return FORWARD;   //遍历前向道路  正方向驶入路口 [0]
			else                              //该路口为起点
				return BACKWARD;   //遍历反向道路 反方向驶入路口 [1]
		}
		else
		{
			if (road->road_end_cross == cross->cross_ord) //该路口为终点
				return  SINGLE;   //遍历正向道路
			else
				return NO_ROAD;   //单向为起点则不遍历
		}
	}
	else
		return NO_ROAD;
}
int DirectionConflict(ROAD *road, CROSS *cross) //从road经过cross的车辆 转向冲突判定
{
	switch (road->frontest_car_direction[TravelDirection(road, cross)])
	{
	    case TURN_LEFT:
		{
			switch (RoadDirectionInCross(road->road_ord, cross))
			{  //左转只与直行冲突
				case UP_ROAD: 
				{
					int JudgeDirection = TravelDirection(cross->road_in_cross[3], cross);
					if ((JudgeDirection!=NO_ROAD) && (cross->road_in_cross[3]->frontest_car_direction[JudgeDirection] == GO_STRAIGHT))
						return CONFLICT;
					break;
				}
				case 1: 
				{
					int JudgeDirection = TravelDirection(cross->road_in_cross[0], cross);
					if ((JudgeDirection != NO_ROAD) && (cross->road_in_cross[0]->frontest_car_direction[JudgeDirection] == GO_STRAIGHT))
						return CONFLICT;
					break;
				}
				case 2: 
				{
					int JudgeDirection = TravelDirection(cross->road_in_cross[1], cross);
					if ((JudgeDirection != NO_ROAD) && (cross->road_in_cross[1]->frontest_car_direction[JudgeDirection] == GO_STRAIGHT))
						return CONFLICT;
					break;
				}
				case 3: 
				{
					int JudgeDirection = TravelDirection(cross->road_in_cross[2], cross);
					if ((JudgeDirection != NO_ROAD) && (cross->road_in_cross[2]->frontest_car_direction[JudgeDirection] == GO_STRAIGHT))
						return CONFLICT;
					break;
				}
			}
			break;
		}
		case TURN_RIGHT:
		{
			switch (RoadDirectionInCross(road->road_ord, cross))
			{ //右转与直行左转均冲突
				case 0: 
				{
					int JudgeDirection = TravelDirection(cross->road_in_cross[1], cross);
					if ((JudgeDirection != NO_ROAD) && (cross->road_in_cross[1]->frontest_car_direction[JudgeDirection] == GO_STRAIGHT))
						return CONFLICT;
					    JudgeDirection = TravelDirection(cross->road_in_cross[2], cross);
					if ((JudgeDirection != NO_ROAD) && (cross->road_in_cross[2]->frontest_car_direction[JudgeDirection] == TURN_LEFT))
						return CONFLICT;
					break;
				}
				case 1: 
				{
					int JudgeDirection = TravelDirection(cross->road_in_cross[2], cross);
					if ((JudgeDirection != NO_ROAD) && (cross->road_in_cross[2]->frontest_car_direction[JudgeDirection] == GO_STRAIGHT))
						return CONFLICT;
					JudgeDirection = TravelDirection(cross->road_in_cross[3], cross);
					if ((JudgeDirection != NO_ROAD) && (cross->road_in_cross[3]->frontest_car_direction[JudgeDirection] == TURN_LEFT))
						return CONFLICT;
					break;
				}
				case 2: 
				{
					int JudgeDirection = TravelDirection(cross->road_in_cross[3], cross);
					if ((JudgeDirection != NO_ROAD) && (cross->road_in_cross[3]->frontest_car_direction[JudgeDirection] == GO_STRAIGHT))
						return CONFLICT;
					JudgeDirection = TravelDirection(cross->road_in_cross[0], cross);
					if ((JudgeDirection != NO_ROAD) && (cross->road_in_cross[0]->frontest_car_direction[JudgeDirection] == TURN_LEFT))
						return CONFLICT;
					break;
				}
				case 3: 
				{
					int JudgeDirection = TravelDirection(cross->road_in_cross[0], cross);
					if ((JudgeDirection != NO_ROAD) && (cross->road_in_cross[0]->frontest_car_direction[JudgeDirection] == GO_STRAIGHT))
						return CONFLICT;
					JudgeDirection = TravelDirection(cross->road_in_cross[1], cross);
					if ((JudgeDirection != NO_ROAD) && (cross->road_in_cross[1]->frontest_car_direction[JudgeDirection] == TURN_LEFT))
						return CONFLICT;
					break;
				}
			}
			break;
		}
	}
	return NO_CONFLICT;
}
int FindCarNextRoad(int road, RouteRoad *r) //输入当前道路得到下一条道路
{
	while (r != NULL)
	{
		if (r->road_num == road && r->next != NULL) //找到当前路  下调路不空
			return r->next->road_num; //结束寻找
		r = r->next;
	}
	return NO_ROAD; //未找道路
}
void StartCarSort(CAR **car, CROSS *cross) //将从某点出发的各车辆进行优先级排序
{
	int temp_ord = 0;
	if (cross->start_car_num <= 1)return;  //少于一辆车 无需排队
	for (int i = 0; i < cross->start_car_num - 1; i++)
	{
		for (int j = 0; j < cross->start_car_num - 1 - i; j++)
		{
			//升序冒泡
			if (car[CAR_ORD_SUB(cross->start_car[j])]->car_start_time > car[CAR_ORD_SUB(cross->start_car[j + 1])]->car_start_time)//先看时间
			{
				temp_ord = cross->start_car[j];
				cross->start_car[j] = cross->start_car[j + 1];
				cross->start_car[j + 1] = temp_ord;
			}
			else
			{
				if ((car[CAR_ORD_SUB(cross->start_car[j])]->car_start_time == car[CAR_ORD_SUB(cross->start_car[j + 1])]->car_start_time)
					&& (car[CAR_ORD_SUB(cross->start_car[j])]->car_order > car[CAR_ORD_SUB(cross->start_car[j + 1])]->car_order))//时间相同 但序号不同
				{
					temp_ord = cross->start_car[j];
					cross->start_car[j] = cross->start_car[j + 1];
					cross->start_car[j + 1] = temp_ord;
				}
			}
		}
	}
}
int IntoRoadTravelDiraction(ROAD *road, CROSS *cross)
{
	if (road != NULL)
	{
		if (road->road_duplex == DUPLEX)//双向
		{
			if (road->road_end_cross == cross->cross_ord) //该路口为终点
				return BACKWARD;   //遍历前向道路  正方向驶入路口 [0]
			else                              //该路口为起点
				return FORWARD;   //遍历反向道路 反方向驶入路口 [1]
		}
		else
		{
			if (road->road_end_cross == cross->cross_ord) //该路口为终点
				return  NO_ROAD;   //单向为终点则不遍历
			else
				return SINGLE;   //遍历正向道路
		}
	}
	else
		return NO_ROAD;
}
