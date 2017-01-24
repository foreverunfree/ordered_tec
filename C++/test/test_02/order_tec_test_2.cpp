# include <iostream>
using namespace std;
# include "ordered_tec.h"

# define DATATYPE double
int main(int argc,char **argv)
{
	TEC_FILE tecfile;
	TEC_ZONE teczone;
	size_t NI=1000,NJ=2000;
	DATATYPE *x=new DATATYPE[NI*NJ];
	DATATYPE *y=new DATATYPE[NI*NJ];
	DATATYPE *z=new DATATYPE[NI*NJ];
	DATATYPE *w=new DATATYPE[NI*NJ];
	for (int j = 0; j != NJ; ++j)
	{
		for (int i = 0; i != NI; ++i)
		{
			x[i + j*NI] = j;
			y[i + j*NI] = i;
			z[i + j*NI] = 1 + i / 2 + j;
			w[i + j*NI] = i + j;
		}
	}
	unsigned int echo = 7;

	tecfile.FileName="test_02";
	tecfile.Title="test_02";
	tecfile.Variables.push_back("x");
	tecfile.Variables.push_back("y");
	tecfile.Variables.push_back("z");
	tecfile.add_auxiliary_data("Auxiliary1","Auxiliary_test_1_ds");
	tecfile.add_auxiliary_data("Auxiliary2",3.14);

	vector<TEC_ZONE> tecz;

	teczone.ZoneName="A";
	teczone.IMax=NI;
	teczone.JMax=NJ;
	teczone.Data.push_back(DATA_P(x,DATA_P::TEC_DOUBLE));
	teczone.Data.push_back(DATA_P(y,DATA_P::TEC_DOUBLE));
	teczone.Data.push_back(DATA_P(z,DATA_P::TEC_DOUBLE));
	teczone.ISkip=2;
	teczone.JSkip=3;
	teczone.IBegin=50;
	teczone.IEnd=50;
	teczone.JBegin=10;
	teczone.JEnd=10;
	teczone.add_auxiliary_data("Auxiliary1","Auxiliary_test_1");
	teczone.add_auxiliary_data("Auxiliary2",3.14);
	tecz.push_back(teczone);

	teczone.ZoneName="B";
	teczone.Data[2]=DATA_P(w,DATA_P::TEC_DOUBLE);
	teczone.Auxiliary.clear();
	teczone.add_auxiliary_data("Auxiliary3","Auxiliary_test_1_2");
	teczone.add_auxiliary_data("Auxiliary4",3.1415);
	tecz.push_back(teczone);

	try
	{
		ORDERED_TEC(tecfile,tecz, echo);
	}
	catch(std::runtime_error err)
	{
		cerr<<"runtime_error: "<<err.what()<<endl;
	}
	delete [] x;
	delete [] y;
	delete [] z;
	delete [] w;
	return 0;
}