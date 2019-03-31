#include <fstream>
#include <string>
#include "Mapping.h"
#include "Read.h"

using namespace std;


void PrintTXT(struct Route ***MAP, int **car, std::string answerpath)
{
	ofstream fout(answerpath);
	fout <<"#(carId, StartTime, RoadId...)"<<endl;
	for (int i = 0; i < car_total_num; i++)
	{
		fout << "(" << car[i][0] << "," << car[i][4];
		struct RouteRoad* r = MAP[SPEED_REAL_SUB(car[i][3])][CROSS_ORD_SUB(car[i][1])][CROSS_ORD_SUB(car[i][2])].Road;
		while (r != NULL)
		{
			fout << "," << r->road_num;
			r = r->next;
		}
		fout << ")" << endl;
	}
}


