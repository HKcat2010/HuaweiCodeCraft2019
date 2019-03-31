#ifndef MAPPING_H 
#define MAPPING_H 1

#include "limits.h"

#define MIN(x,y) ((x)<(y)?(x):(y))  //括号保证运算优先级

//hash 函数 后期可用于压缩空间
//MAP 和 读入txt 数组下标一直，但和实际编号[0]不一致
#define CAR_ORD_SUB(x)  (x-car_order_min)  //车牌号    ->  数组下标
#define CAR_SUB_ORD(x)  (x+car_order_min)  //数组下标  ->  车牌号
#define ROAD_ORD_SUB(x) (x-road_order_min)  //路牌   ->  数组下标
#define ROAD_SUB_ORD(x) (x+road_order_min)  //数组下标   ->   路牌
#define CROSS_ORD_SUB(x) (x-cross_order_min)  //路口编号   ->   数组下标
#define CROSS_SUB_ORD(x) (x+cross_order_min)  //数组下标   ->   路口编号
#define SPEED_REAL_SUB(x) (x-car_speed_min)  //真实速度   ->   速度下标
#define SPEED_SUB_REAL(x) (x+car_speed_min)  //速度下标   ->   真实速度

//现阶段牺牲空间 换取时间，空间不够 改用hash

struct RouteRoad  //途径路径
{
	int road_num = 0;
	struct RouteRoad *next = NULL;
};

struct Route   //线路
{
	int total_road_cost = INT_MAX;   //长度/速度 = 可能总代价 
	struct RouteRoad *Road;
};

extern struct Route ***MAP;

void Mapping(int **cross, int **road);
void ShowRouteRoad(struct RouteRoad *r); //显示路线
void CopyRouteRoad(struct RouteRoad *p1, struct RouteRoad *p2); //将r2路径复制到r1上
void AddRouteRoad(struct RouteRoad *p1, struct RouteRoad *p2); //将r2路径 复制 加载到r1末尾
void MAP_Floyd();

#endif