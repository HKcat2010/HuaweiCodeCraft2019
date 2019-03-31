#ifndef READ_H
#define READ_H 1

#include <string>
#include "limits.h"

extern int car_order_min;
extern int car_order_max;
extern int car_total_num;   //总车辆数
extern int road_order_min;
extern int road_order_max;
extern int road_total_num;   //总道路数
extern int cross_order_min;
extern int cross_order_max;
extern int cross_total_num;   //总路口数目
extern int car_speed_min;   //车辆可能的行驶速度
extern int car_speed_max;
extern int car_speed_cnt; 

int **GetCar(std::string address);
int **GetRoad(std::string address);
int **GetCross(std::string address);

#endif
