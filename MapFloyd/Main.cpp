#include <iostream>
#include <string>
#include "time.h"  //注意注释掉
#include "Mapping.h"
#include "Read.h"
#include "Output.h"
using namespace std;

int main()
{
	int **car;           //车辆数据
	int **road;          //道路数据
	int **cross;         //路口数据
	long int start_time = clock();
	car = GetCar("D:/competition/Maps/Map1/car.txt");
	road = GetRoad("D:/competition/Maps/Map1/road.txt");
	cross = GetCross("D:/competition/Maps/Map1/cross.txt");
	string answerPath("D:/competition/Maps/Map1/answer.txt"); //注意更改

	map_building(cross, road); //建立邻接矩阵
	MAP_Floyd();  //最小代价距离

	PrintTXT(MAP, car, answerPath); //输出answer.txt
	cout << (double)(clock() - start_time) / CLOCKS_PER_SEC << endl;
	system("pause");
	return 0;
}


