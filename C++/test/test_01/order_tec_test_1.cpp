# include <iostream>
using namespace std;
# include "ordered_tec.h"

# define DATATYPE double
int main(int argc,char **argv)
{
	size_t NI=1000,NJ=2000;
	DATATYPE *x=new DATATYPE[NI*NJ];
	DATATYPE *y=new DATATYPE[NI*NJ];
	DATATYPE *z=new DATATYPE[NI*NJ];
	for (int j = 0; j != NJ; ++j)
	{
		for (int i = 0; i != NI; ++i)
		{
			x[i + j*NI] = j;
			y[i + j*NI] = i;
			z[i + j*NI] = 1 + i / 2 + j;
		}
	}

	ORDERED_TEC::TEC_FILE tecfile;
	tecfile.Variables.push_back("x");
	tecfile.Variables.push_back("y");
	tecfile.Variables.push_back("z");
	tecfile.new_zone();
	tecfile.Zones[0].IMax=NI;
	tecfile.Zones[0].JMax=NJ;
	tecfile.Zones[0].Data.push_back(ORDERED_TEC::DATA_P(x, ORDERED_TEC::DATA_P::TEC_DOUBLE));
	tecfile.Zones[0].Data.push_back(ORDERED_TEC::DATA_P(y, ORDERED_TEC::DATA_P::TEC_DOUBLE));
	tecfile.Zones[0].Data.push_back(ORDERED_TEC::DATA_P(z, ORDERED_TEC::DATA_P::TEC_DOUBLE));
	try
	{
		tecfile.write_plt();
	}
	catch(std::runtime_error err)
	{
		cerr<<"runtime_error: "<<err.what()<<endl;
	}

	delete [] x;
	delete [] y;
	delete [] z;
	return 0;
}