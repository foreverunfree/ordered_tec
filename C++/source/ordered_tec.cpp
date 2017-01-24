# include "ordered_tec.h"

# include <string>
# include <vector>
# include <map>
# include <cstring>
# include <stdexcept>
# include <sstream>
# include <stdio.h>

# define S_INT32 4
# define S_FLOAT32 4
# define S_FLOAT64 8

# define S_FLOAT 4
# define S_DOUBLE 8
# define S_LONGINT 4
# define S_SHORTINT 2
# define S_BYTE 1

TEC_FILE::TEC_FILE()
{
	FileName = "untitled_file";
	FileType = 0;
	Title = "untitled";
}

bool TEC_FILE::add_auxiliary_data(std::string name,std::string value)
{
	std::pair<std::map<std::string,std::string>::iterator,bool> ans;
	std::pair<std::string,std::string> temp(name,value);
	ans=Auxiliary.insert(temp);
	return ans.second;
}

bool TEC_FILE::add_auxiliary_data(std::string name,double value)
{
	std::stringstream ss;
	ss<<value;
	return add_auxiliary_data(name,ss.str());
}

DATA_P::DATA_P()
{
	DataP=NULL;
	type=TEC_NULL;
	size=0;
	buf=NULL;
}

DATA_P::DATA_P(void * iDataP,TEC_TYPE itype)
{
	DataP=iDataP;
	type=itype;
	switch(itype)
	{
	case TEC_FLOAT:
		{
			size=S_FLOAT;
			break;
		}
	case TEC_DOUBLE:
		{
			size=S_DOUBLE;
			break;
		}
	case TEC_LONGINT:
		{
			size=S_LONGINT;
			break;
		}
	case TEC_SHORTINT:
		{
			size=S_SHORTINT;
			break;
		}
	case TEC_BYTE:
		{
			size=S_BYTE;
			break;
		}
	}
	buf=NULL;
}

TEC_ZONE::TEC_ZONE()
{
	ZoneName = "untitled_zone";
	StrandId = -1;
	SolutionTime = 0.0f;
	IMax = 1;
	JMax = 1;
	KMax = 1;
	ISkip = 1;
	JSkip = 1;
	KSkip = 1;
	IBegin = 0;
	IEnd = 0;
	JBegin = 0;
	JEnd = 0;
	KBegin = 0;
	KEnd = 0;
	noskip = true;
	noexc = true;
	needreal = false;
}

INT32 TEC_ZONE::get_realijk(short o)
{
	switch(o)
	{
	case 1:
		return Real_IMax;
	case 2:
		return Real_JMax;
	case 3:
		return Real_KMax;
	default:
		return -1;
	}
}

void TEC_ZONE::real_ijk()
{
	if(IMax==1)
	{
		throw std::runtime_error("one of zone.IMax equal to 1");
	}
	if(JMax==1)
	{
		if(JSkip!=1||JBegin!=0||JEnd!=0)
		{
			throw std::runtime_error("zone.JSkip(or zone.JBegin or zone.JEnd) donnot need to set");
		}
	}
	if(IMax==0||JMax==0||KMax==0)
	{
		throw std::runtime_error("zone.IMax(or zone.JMax or zone.KMax) connot be zeor");
	}
	if(KMax==1)
	{
		if(KSkip!=1||KBegin!=0||KEnd!=0)
		{
			throw std::runtime_error("zone.KSkip(or zone.KBegin or zone.KEnd) donnot need to set");
		}
	}
	Real_IMax=(IMax-IBegin-IEnd)/ISkip;
	Real_JMax=(JMax-JBegin-JEnd)/JSkip;
	Real_KMax=(KMax-KBegin-KEnd)/KSkip;
	if((IMax-IBegin-IEnd)%ISkip)
	{
		++Real_IMax;
	}
	if((JMax-JBegin-JEnd)%JSkip)
	{
		++Real_JMax;
	}
	if((KMax-KBegin-KEnd)%KSkip)
	{
		++Real_KMax;
	}
	if(Real_IMax==0||Real_JMax==0||Real_KMax==0)
	{
		throw std::runtime_error("zone.Real_IMax(or zone.Real_JMax or zone.Real_KMax) is zeor due to unreasonable set of Skip, Being or End");
	}
	noskip = ISkip==1 && JSkip==1 && KSkip==1 ;
	noexc = IBegin==0 && IEnd==0 && JBegin==0 && JEnd==0 && KBegin==0 && KEnd==0 ;
}

bool TEC_ZONE::add_auxiliary_data(std::string name,std::string value)
{
	std::pair<std::map<std::string,std::string>::iterator,bool> ans;
	std::pair<std::string,std::string> temp(name,value);
	ans=Auxiliary.insert(temp);
	return ans.second;
}

bool TEC_ZONE::add_auxiliary_data(std::string name,double value)
{
	std::stringstream ss;
	ss<<value;
	return add_auxiliary_data(name,ss.str());
}

void TEC_ZONE::make_buf()
{
	if(noskip && noexc)
	{
		for(std::vector<DATA_P>::iterator i=Data.begin();i!=Data.end();++i)
		{
			i->buf=(byte *)i->DataP;
		}
		needreal = false;
	}
	else if(noskip)
	{
		for(std::vector<DATA_P>::iterator i=Data.begin();i!=Data.end();++i)
		{
			size_t size=i->size;
			byte * datap=(byte *)i->DataP;
			i->buf=new byte[Real_IMax*Real_JMax*Real_KMax*size];
			if(i->buf==NULL)
			{
				throw std::runtime_error("out of memory");
			}
			for(size_t sk=KBegin;sk<KMax-KEnd;sk+=KSkip)
			{
				for(size_t sj=JBegin;sj<JMax-JEnd;sj+=JSkip)
				{
					std::memcpy(i->buf+(Real_IMax*(sj-JBegin)/JSkip+Real_IMax*Real_JMax*(sk-KBegin)/KSkip)*size,
						        datap+(IBegin+IMax*sj+IMax*JMax*sk)*size, Real_IMax*size);
				}
			}
		}
		needreal = true;
	}
	else
	{
		for(std::vector<DATA_P>::iterator i=Data.begin();i!=Data.end();++i)
		{
			size_t size=i->size;
			byte * datap=(byte *)i->DataP;
			i->buf=new byte[Real_IMax*Real_JMax*Real_KMax*size];
			if(i->buf==NULL)
			{
				throw std::runtime_error("out of memory");
			}
			for(size_t sk=KBegin;sk<KMax-KEnd;sk+=KSkip)
			{
				for(size_t sj=JBegin;sj<JMax-JEnd;sj+=JSkip)
				{
					for(size_t si=IBegin;si<IMax-IEnd;si+=ISkip)
					{
						std::memcpy(i->buf+((si-IBegin)/ISkip+Real_IMax*(sj-JBegin)/JSkip+Real_IMax*Real_JMax*(sk-KBegin)/KSkip)*size,
							        datap+(si+IMax*sj+IMax*JMax*sk)*size, size);
					}
				}
			}
		}
		needreal = true;
	}
}

void TEC_ZONE::realise_buf()
{
	if(needreal)
	{
		for(std::vector<DATA_P>::iterator i=Data.begin();i!=Data.end();++i)
		{
			delete [] i->buf;
		}
		needreal = false;
	}
	else
	{
		for(std::vector<DATA_P>::iterator i=Data.begin();i!=Data.end();++i)
		{
			i->buf = NULL;
		}
	}
}

void TEC_ZONE::write_data(const DATA_P &data,FILE *of) const
{
	fwrite(data.buf, data.size, Real_IMax*Real_JMax*Real_KMax, of);
}

void W_INT32(const INT32 &a, FILE *f)
{
	INT32 t = a;
	fwrite(&t, S_INT32, 1, f);
}
void W_FLOAT32(const FLOAT32 &a, FILE *f)
{
	FLOAT32 t = a;
	fwrite(&t, S_FLOAT32, 1, f);
}
void W_FLOAT64(const FLOAT64 &a, FILE *f)
{
	FLOAT64 t = a;
	fwrite(&t, S_FLOAT64, 1, f);
}

void W_STRING(const std::string &a,FILE *f)
{
	for(std::string::const_iterator i=a.begin();i!=a.end();++i)
	{
		W_INT32(*i, f);
	}
	W_INT32(0, f);
}

template<typename T>
void get_minmax(T *data,size_t N,FLOAT64 &re_min,FLOAT64 &re_max)
{
	T min,max;
	min=data[0];
	max=data[0];
	T loc_min,loc_max;
	size_t n;
	if(N%2)
	{
		n=(N-1)/2;
	}
	else
	{
		n=N/2;
	}
	for(size_t i=0;i!=n;++i)
	{
		if(data[2*i]>data[2*i+1])
		{
			loc_min=data[2*i+1];
			loc_max=data[2*i];
		}
		else
		{
			loc_min=data[2*i];
			loc_max=data[2*i+1];
		}
		if(loc_min<min)
		{
			min=loc_min;
		}
		if(loc_max>max)
		{
			max=loc_max;
		}
	}
	if(N%2)
	{
		if(data[N-1]<min)
		{
			min=data[N-1];
		}
		else if(data[N-1]>max)
		{
			max=data[N-1];
		}
	}
	re_min=min;
	re_max=max;
}

std::pair<FLOAT64,FLOAT64> minmax(DATA_P datap,size_t N)
{
	FLOAT64 min,max;
	switch(datap.type)
	{
	case DATA_P::TEC_FLOAT:
		{
			float *data=(float *)datap.buf;
			get_minmax<float>(data,N,min,max);
			break;
		}
	case DATA_P::TEC_DOUBLE:
		{
			double *data=(double *)datap.buf;
			get_minmax<double>(data,N,min,max);
			break;
		}
	case DATA_P::TEC_LONGINT:
		{
			longint *data=(longint *)datap.buf;
			get_minmax<longint>(data,N,min,max);
			break;
		}
	case DATA_P::TEC_SHORTINT:
		{
			shortint *data=(shortint *)datap.buf;
			get_minmax<shortint>(data,N,min,max);
			break;
		}
	case DATA_P::TEC_BYTE:
		{
			byte *data=(byte *)datap.buf;
			get_minmax<byte>(data,N,min,max);
			break;
		}
	default:
		{
			throw std::runtime_error("data type error");
		}
	}
	std::pair<FLOAT64,FLOAT64> ans(min,max);
	return ans;
}

void ORDERED_TEC(TEC_FILE tec_file,std::vector<TEC_ZONE> tec_zone,unsigned int echo)
{
	if(tec_file.Variables.size()==0)
	{
		throw std::runtime_error("tec_file.Variables is empty");
	}
	if(tec_zone.size()==0)
	{
		throw std::runtime_error("tec_zone(vector<TEC_ZONE>) is empty");
	}
	for(std::vector<TEC_ZONE>::iterator i=tec_zone.begin();i!=tec_zone.end();++i)
	{
		i->real_ijk();
		if(i->Data.size()==0)
		{
			throw std::runtime_error("one of zone.Data is empty");
		}
		if(i->Data.size()!=tec_file.Variables.size())
		{
			throw std::runtime_error("the size of zone.Data is not equal to the size of tec_file.Variables");
		}
		for(std::vector<DATA_P>::const_iterator j=i->Data.begin();j!=i->Data.end();++j)
		{
			if(j->DataP==NULL||j->type==0||j->size==0)
			{
				throw std::runtime_error("one of Data is unset");
			}
		}
	}

	if(echo>1)
	{
		printf("#### creat file %s.plt ####\n",tec_file.FileName.c_str());
	}
	std::ios::sync_with_stdio(false);
	FILE *of = fopen((tec_file.FileName+".plt").c_str(), "wb");
	if(of==NULL)
	{
		throw std::runtime_error(std::string("cannot open file ")+(tec_file.FileName+".plt"));
	}
	//I    HEADER SECTION
	//i    Magic number, Version number
	fwrite("#!TDV112", sizeof(char), 8, of);//8 Bytes, exact characters "#!TDV112". Version number follows the "V" and consumes the next 3 characters (for example: "V75", "V101")
	//ii   Integer value of 1
	W_INT32(1, of);//This is used to determine the byte order of the reader, relative to the writer
	//iii  Title and variable names
	W_INT32(tec_file.FileType, of);//FileType 0 = FULL, 1 = GRID, 2 = SOLUTION
	W_STRING(tec_file.Title,of);//The TITLE
	W_INT32(tec_file.Variables.size(), of);//Number of variables (NumVar) in the datafile
	for(std::vector<std::string>::const_iterator i=tec_file.Variables.begin();i!=tec_file.Variables.end();++i)
	{
		W_STRING(*i,of);//Variable names
	}
	//iv   Zones
	for(std::vector<TEC_ZONE>::const_iterator i=tec_zone.begin();i!=tec_zone.end();++i)
	{
		W_FLOAT32(299.0f, of);//Zone marker. Value = 299.0
		W_STRING(i->ZoneName,of);//Zone name
		W_INT32(-1, of);//ParentZone
		W_INT32(i->StrandId, of);//StrandID
		W_FLOAT64(i->SolutionTime, of);//Solution time
		W_INT32(-1, of);//Not used. Set to -1
		W_INT32(0, of);//ZoneType 0=ORDERED
		W_INT32(0, of);//Specify Var Location. 0 = Don��t specify, all data is located at the nodes
		W_INT32(0, of);//Are raw local 1-to-1 face neighbors supplied? (0=FALSE 1=TRUE) ORDERED and FELINESEG zones must specify 0
		W_INT32(0, of);//Number of miscellaneous user-defined face neighbor connections (value >= 0)
		W_INT32(i->Real_IMax, of);//IMax
		W_INT32(i->Real_JMax, of);//JMax
		W_INT32(i->Real_KMax, of);//KMax
		for(std::map<std::string,std::string>::const_iterator j=i->Auxiliary.begin();j!=i->Auxiliary.end();++j)
		{
			W_INT32(1, of);//Auxiliary name/value pair to follow
			W_STRING(j->first,of);//name string
			W_INT32(0, of);//Auxiliary Value Format (Currently only allow 0=AuxDataType_String)
			W_STRING(j->second,of);//Value string
		}
		W_INT32(0, of);//No more Auxiliary name/value pairs
	}
	//ix Dataset Auxiliary data
	for(std::map<std::string,std::string>::const_iterator i=tec_file.Auxiliary.begin();i!=tec_file.Auxiliary.end();++i)
	{
		W_FLOAT32(799.0f, of);//DataSetAux Marker
		W_STRING(i->first,of);//Text for Auxiliary "Name"
		W_INT32(0, of);//Auxiliary Value Format (Currently only allow 0=AuxDataType_String)
		W_STRING(i->second,of);//Text for Auxiliary "Value"
	}
	//EOHMARKER, value=357.0
	W_FLOAT32(357.0f, of);
	if(echo>1)
	{
		printf("--   write head section   --\n");
	}
	//II   DATA SECTION
	//i    For both ordered and fe zones
	double s_f = 0;
	for(std::vector<TEC_ZONE>::iterator i=tec_zone.begin();i!=tec_zone.end();++i)
	{
		W_FLOAT32(299.0f, of);//Zone marker Value = 299.0
		if(echo>2)
		{
			printf("--   write zone %i: %s   --\n",int(i-tec_zone.begin()+1),tec_zone[i-tec_zone.begin()].ZoneName.c_str());
		}
		double s_z = 0;
		for(std::vector<DATA_P>::const_iterator j=i->Data.begin();j!=i->Data.end();++j)
		{
			W_INT32(j->type, of);//Variable data format
			s_z+=j->size;
		}
		s_z*=i->Real_IMax*i->Real_JMax*i->Real_KMax;
		s_z/=1024*1024;
		s_f+=s_z;
		W_INT32(0, of);//Has passive variables: 0 = no
		W_INT32(0, of);//Has variable sharing 0 = no
		W_INT32(-1, of);//Zero based zone number to share connectivity list with (-1 = no sharing)
		i->make_buf();
		for(std::vector<DATA_P>::const_iterator j=i->Data.begin();j!=i->Data.end();++j)
		{
			std::pair<FLOAT64,FLOAT64> mm=minmax(*j,(i->Real_IMax)*(i->Real_JMax)*(i->Real_KMax));
			W_FLOAT64(mm.first, of);//Min value
			W_FLOAT64(mm.second, of);//Max value
		}
		if(echo>2)
		{
			printf("..   write variables: ");
		}
		for(std::vector<DATA_P>::const_iterator j=i->Data.begin();j!=i->Data.end();++j)
		{
			if(echo>2)
			{
				printf("%s ",tec_file.Variables[j-i->Data.begin()].c_str());
			}
			i->write_data(*j,of);//Zone Data. Each variable is in data format as specified above
		}
		if(echo>2)
		{
			printf("  ..\n");
		}
		i->realise_buf();
		if(echo>4)
		{
			printf("     zone size: %.2fMB\n",s_z);
		}
		if(echo>3)
		{
			printf("     IMax=%i",i->Real_IMax);
			if(i->Real_IMax!=1)
			{
				printf(" JMax=%i",i->Real_JMax);
				if(i->Real_KMax!=1)
				{
					printf(" KMax=%i",i->Real_KMax);
				}
			}
			printf("\n");
		}
		if(echo>5)
		{
			printf("     ISkip=%i",int(i->ISkip));
			if(i->Real_IMax!=1)
			{
				printf(" JSkip=%i",int(i->JSkip));
				if(i->Real_KMax!=1)
				{
					printf(" KSkip=%i",int(i->KSkip));
				}
			}
			printf("\n");
		}
		if(echo>6)
		{
			printf("     IBegin=%i IEnd=%i\n",int(i->IBegin),int(i->IEnd));
			if(i->Real_IMax!=1)
			{
				printf("     JBegin=%i JEnd=%i\n",int(i->JBegin),int(i->JEnd));
				if(i->Real_KMax!=1)
				{
					printf("     KBegin=%i KEnd=%i\n",int(i->KBegin),int(i->KEnd));
				}
			}
		}
		if(echo>3 && i->StrandId!=-1)
		{
			printf("     StrandId=%i SolutionTime=%f\n",i->StrandId,i->SolutionTime);
		}
		if(echo>1)
		{
			printf("--   write zone %i: %s   --\n",int(i-tec_zone.begin()+1),tec_zone[i-tec_zone.begin()].ZoneName.c_str());
		}
	}
	fclose(of);
	std::ios::sync_with_stdio(true);
	if(echo>4)
	{
		printf("     file size: %.2fMB\n",s_f);
	}
	if(echo>0)
	{
		printf("#### save file %s.plt ####\n",tec_file.FileName.c_str());
	}
}

void ORDERED_TEC(TEC_FILE tec_file,TEC_ZONE tec_zone,unsigned int echo)
{
	std::vector<TEC_ZONE> tec_zone_vector;
	tec_zone_vector.push_back(tec_zone);
	ORDERED_TEC(tec_file,tec_zone_vector,echo);
}