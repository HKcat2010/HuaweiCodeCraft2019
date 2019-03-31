#include <fstream>
#include <string>
#include "Mapping.h"
#include "Read.h"
#include <stdlib.h>
#include <time.h>
using namespace std;


void PrintTXT(struct Route ***MAP, int **car, std::string answerpath)
{
	ofstream fout(answerpath);
	fout <<"#(carId, StartTime, RoadId...)"<<endl;
	int start_time = 0;
	srand((unsigned)time(0));
	for (int i = 0; i < car_total_num; i++)
	{
		start_time = car[i][4] + rand() % 6;
		fout << "(" << car[i][0] << "," << start_time;
		struct RouteRoad* r = MAP[SPEED_REAL_SUB(car[i][3])][CROSS_ORD_SUB(car[i][1])][CROSS_ORD_SUB(car[i][2])].Road;
		while (r != NULL)
		{
			fout << "," << r->road_num;
			r = r->next;
		}
		fout << ")" << endl;
	}
}


