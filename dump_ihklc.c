#include <string.h>
#include <math.h>
#include <time.h>
#include "pil.h"
#include "isdc.h"
#include "dal3gen.h"
#include "dal3hk.h"
#include "dal3aux.h"
#include <dirent.h>
#include <time.h>
#include <ctype.h>
#include <stdarg.h>



#define  TOOLNAME "dump_ihkilc"
#define  TOOLVERSION "1.0"
#define  MAXFILE 40000      /* Maximum number of input files */
#define  MAXBINS 20000000      /* Maximum number of input files */
#define  MAXNAMELENGTH 256  /* Maximum character length of file names */
#define  MAXPATH 65535  /* Maximum character length of file names */
#define  BufLen_2      MAXNAMELENGTH /* Required for pfile.h */
#define  MAXSTRING 1024
#define  MAX_TARGETS 200


// flags to mark the bins with
#define FLAG_GOOD 0x0000
#define FLAG_BADPHASE 0x0001
#define FLAG_BADRATE 0x0010
#define FLAG_GAP 0x0100

#define TFLAG long

#define DEGRAD (180./M_PI)


typedef struct {
	char name[MAXNAMELENGTH];
	char hrw[MAXNAMELENGTH];
	char cnv[MAXNAMELENGTH];
	char col[MAXNAMELENGTH];
} dataspec;

dataspec dataspecs[]={
			{.name="IBIS_VETO",.hrw="IBIS-DPE.-HRW",.cnv="IBIS-DPE.-CNV",.col="V1S_MBOT_MCOUNT"},
            {.name="IBIS_VETO_LAT",.hrw="IBIS-DPE.-HRW",.cnv="IBIS-DPE.-CNV",.col="V1S_MLAT_MCOUNT"},
            {.name="ISGRIRAW_MCE0",.hrw="IBIS-DPE.-HRW",.cnv="IBIS-DPE.-CNV",.col="I0S_MEVTCNT_MMCE0"},
            {.name="ISGRIRAW_MCE1",.hrw="IBIS-DPE.-HRW",.cnv="IBIS-DPE.-CNV",.col="I0S_MEVTCNT_MMCE1"},
            {.name="ISGRIRAW_MCE2",.hrw="IBIS-DPE.-HRW",.cnv="IBIS-DPE.-CNV",.col="I0S_MEVTCNT_MMCE2"},
            {.name="ISGRIRAW_MCE3",.hrw="IBIS-DPE.-HRW",.cnv="IBIS-DPE.-CNV",.col="I0S_MEVTCNT_MMCE3"},
            {.name="ISGRIRAW_MCE4",.hrw="IBIS-DPE.-HRW",.cnv="IBIS-DPE.-CNV",.col="I0S_MEVTCNT_MMCE4"},
            {.name="ISGRIRAW_MCE5",.hrw="IBIS-DPE.-HRW",.cnv="IBIS-DPE.-CNV",.col="I0S_MEVTCNT_MMCE5"},
            {.name="ISGRIRAW_MCE6",.hrw="IBIS-DPE.-HRW",.cnv="IBIS-DPE.-CNV",.col="I0S_MEVTCNT_MMCE6"},
            {.name="ISGRIRAW_MCE7",.hrw="IBIS-DPE.-HRW",.cnv="IBIS-DPE.-CNV",.col="I0S_MEVTCNT_MMCE7"},
            {.name="ACS",.hrw="SPI.-OACS-HRW",.cnv="SPI.-OACS-CNV",.col="ACS_RATE"},
            {.name="SPI_VETOGATE",.hrw="SPI.-SCHK-HRW",.cnv="SPI.-SCHK-CNV",.col="P__DF__NVTGT__L"},
			{.name="SPI_VETONONSAT",.hrw="SPI.-SCHK-HRW",.cnv="SPI.-SCHK-CNV",.col="P__DF__CNVT_MBW__L"},
			{.name="SPI_VETOSAT",.hrw="SPI.-SCHK-HRW",.cnv="SPI.-SCHK-CNV",.col="P__DF__CNVT_MAB__L"},
			{.name="SPI_GEDRATE0",.hrw="SPI.-SCHK-HRW",.cnv="SPI.-SCHK-CNV",.col="P__DF__CAFTT__L0"},
			{.name="SPI_GEDSAT0",.hrw="SPI.-SCHK-HRW",.cnv="SPI.-SCHK-CNV",.col="P__DF__CAFTS__L0"},
			{.name="SPTI1",.hrw="PICS-SPTI-RAW",.cnv="PICS-SPTI-CPR",.col="CELL_1"},
			{.name="SPTI2",.hrw="PICS-SPTI-RAW",.cnv="PICS-SPTI-CPR",.col="CELL_2"},
			{.name="SPTI3",.hrw="PICS-SPTI-RAW",.cnv="PICS-SPTI-CPR",.col="CELL_3"},
			{.name="SPTI4",.hrw="PICS-SPTI-RAW",.cnv="PICS-SPTI-CPR",.col="CELL_4"},
			{.name="SPTI5",.hrw="PICS-SPTI-RAW",.cnv="PICS-SPTI-CPR",.col="CELL_5"},
			{.name="SPTI6",.hrw="PICS-SPTI-RAW",.cnv="PICS-SPTI-CPR",.col="CELL_6"},
			{.name="SPTI7",.hrw="PICS-SPTI-RAW",.cnv="PICS-SPTI-CPR",.col="CELL_7"},
			{.name="SPTI8",.hrw="PICS-SPTI-RAW",.cnv="PICS-SPTI-CPR",.col="CELL_8"},
            {.name="FEE1",.hrw="SPI.-FEE0-HRW",.cnv="SPI.-FEE0-CNV",.col="FEE1"}, /// this should be filled automatically
            {.name="FEE2",.hrw="SPI.-FEE0-HRW",.cnv="SPI.-FEE0-CNV",.col="FEE2"},
            {.name="FEE3",.hrw="SPI.-FEE0-HRW",.cnv="SPI.-FEE0-CNV",.col="FEE3"},
            {.name="FEE4",.hrw="SPI.-FEE0-HRW",.cnv="SPI.-FEE0-CNV",.col="FEE4"},
            {.name="FEE5",.hrw="SPI.-FEE0-HRW",.cnv="SPI.-FEE0-CNV",.col="FEE5"},
            {.name="FEE6",.hrw="SPI.-FEE0-HRW",.cnv="SPI.-FEE0-CNV",.col="FEE6"},
            {.name="FEE7",.hrw="SPI.-FEE0-HRW",.cnv="SPI.-FEE0-CNV",.col="FEE7"},
            {.name="FEE8",.hrw="SPI.-FEE1-HRW",.cnv="SPI.-FEE1-CNV",.col="FEE8"},
            {.name="FEE9",.hrw="SPI.-FEE1-HRW",.cnv="SPI.-FEE1-CNV",.col="FEE9"},
            {.name="FEE10",.hrw="SPI.-FEE1-HRW",.cnv="SPI.-FEE1-CNV",.col="FEE10"},
            {.name="FEE11",.hrw="SPI.-FEE1-HRW",.cnv="SPI.-FEE1-CNV",.col="FEE11"},
            {.name="FEE12",.hrw="SPI.-FEE1-HRW",.cnv="SPI.-FEE1-CNV",.col="FEE12"},
            {.name="FEE13",.hrw="SPI.-FEE1-HRW",.cnv="SPI.-FEE1-CNV",.col="FEE13"},
            {.name="FEE14",.hrw="SPI.-FEE1-HRW",.cnv="SPI.-FEE1-CNV",.col="FEE14"},
            {.name="FEE15",.hrw="SPI.-FEE1-HRW",.cnv="SPI.-FEE1-CNV",.col="FEE15"},
            {.name="FEE16",.hrw="SPI.-FEE2-HRW",.cnv="SPI.-FEE2-CNV",.col="FEE16"},
            {.name="FEE17",.hrw="SPI.-FEE2-HRW",.cnv="SPI.-FEE2-CNV",.col="FEE17"},
            {.name="FEE18",.hrw="SPI.-FEE2-HRW",.cnv="SPI.-FEE2-CNV",.col="FEE18"},
            {.name="FEE19",.hrw="SPI.-FEE2-HRW",.cnv="SPI.-FEE2-CNV",.col="FEE19"},
            {.name="FEE20",.hrw="SPI.-FEE2-HRW",.cnv="SPI.-FEE2-CNV",.col="FEE20"},
            {.name="FEE21",.hrw="SPI.-FEE2-HRW",.cnv="SPI.-FEE2-CNV",.col="FEE21"},
            {.name="FEE22",.hrw="SPI.-FEE2-HRW",.cnv="SPI.-FEE2-CNV",.col="FEE22"},
            {.name="FEE23",.hrw="SPI.-FEE2-HRW",.cnv="SPI.-FEE2-CNV",.col="FEE23"},
            {.name="FEE24",.hrw="SPI.-FEE3-HRW",.cnv="SPI.-FEE3-CNV",.col="FEE24"},
            {.name="FEE25",.hrw="SPI.-FEE3-HRW",.cnv="SPI.-FEE3-CNV",.col="FEE25"},
            {.name="FEE26",.hrw="SPI.-FEE3-HRW",.cnv="SPI.-FEE3-CNV",.col="FEE26"},
            {.name="FEE27",.hrw="SPI.-FEE3-HRW",.cnv="SPI.-FEE3-CNV",.col="FEE27"},
            {.name="FEE28",.hrw="SPI.-FEE3-HRW",.cnv="SPI.-FEE3-CNV",.col="FEE28"},
            {.name="FEE29",.hrw="SPI.-FEE3-HRW",.cnv="SPI.-FEE3-CNV",.col="FEE29"},
            {.name="FEE30",.hrw="SPI.-FEE3-HRW",.cnv="SPI.-FEE3-CNV",.col="FEE30"},
            {.name="FEE31",.hrw="SPI.-FEE3-HRW",.cnv="SPI.-FEE3-CNV",.col="FEE31"},
            {.name="FEE32",.hrw="SPI.-FEE4-HRW",.cnv="SPI.-FEE4-CNV",.col="FEE32"},
            {.name="FEE33",.hrw="SPI.-FEE4-HRW",.cnv="SPI.-FEE4-CNV",.col="FEE33"},
            {.name="FEE34",.hrw="SPI.-FEE4-HRW",.cnv="SPI.-FEE4-CNV",.col="FEE34"},
            {.name="FEE35",.hrw="SPI.-FEE4-HRW",.cnv="SPI.-FEE4-CNV",.col="FEE35"},
            {.name="FEE36",.hrw="SPI.-FEE4-HRW",.cnv="SPI.-FEE4-CNV",.col="FEE36"},
            {.name="FEE37",.hrw="SPI.-FEE4-HRW",.cnv="SPI.-FEE4-CNV",.col="FEE37"},
            {.name="FEE38",.hrw="SPI.-FEE4-HRW",.cnv="SPI.-FEE4-CNV",.col="FEE38"},
            {.name="FEE39",.hrw="SPI.-FEE4-HRW",.cnv="SPI.-FEE4-CNV",.col="FEE39"},
            {.name="FEE40",.hrw="SPI.-FEE5-HRW",.cnv="SPI.-FEE5-CNV",.col="FEE40"},
            {.name="FEE41",.hrw="SPI.-FEE5-HRW",.cnv="SPI.-FEE5-CNV",.col="FEE41"},
            {.name="FEE42",.hrw="SPI.-FEE5-HRW",.cnv="SPI.-FEE5-CNV",.col="FEE42"},
            {.name="FEE43",.hrw="SPI.-FEE5-HRW",.cnv="SPI.-FEE5-CNV",.col="FEE43"},
            {.name="FEE44",.hrw="SPI.-FEE5-HRW",.cnv="SPI.-FEE5-CNV",.col="FEE44"},
            {.name="FEE45",.hrw="SPI.-FEE5-HRW",.cnv="SPI.-FEE5-CNV",.col="FEE45"},
            {.name="FEE46",.hrw="SPI.-FEE5-HRW",.cnv="SPI.-FEE5-CNV",.col="FEE46"},
            {.name="FEE47",.hrw="SPI.-FEE5-HRW",.cnv="SPI.-FEE5-CNV",.col="FEE47"},
            {.name="FEE48",.hrw="SPI.-FEE6-HRW",.cnv="SPI.-FEE6-CNV",.col="FEE48"},
            {.name="FEE49",.hrw="SPI.-FEE6-HRW",.cnv="SPI.-FEE6-CNV",.col="FEE49"},
            {.name="FEE50",.hrw="SPI.-FEE6-HRW",.cnv="SPI.-FEE6-CNV",.col="FEE50"},
            {.name="FEE51",.hrw="SPI.-FEE6-HRW",.cnv="SPI.-FEE6-CNV",.col="FEE51"},
            {.name="FEE52",.hrw="SPI.-FEE6-HRW",.cnv="SPI.-FEE6-CNV",.col="FEE52"},
            {.name="FEE53",.hrw="SPI.-FEE6-HRW",.cnv="SPI.-FEE6-CNV",.col="FEE53"},
            {.name="FEE54",.hrw="SPI.-FEE6-HRW",.cnv="SPI.-FEE6-CNV",.col="FEE54"},
            {.name="FEE55",.hrw="SPI.-FEE6-HRW",.cnv="SPI.-FEE6-CNV",.col="FEE55"},
            {.name="FEE56",.hrw="SPI.-FEE7-HRW",.cnv="SPI.-FEE7-CNV",.col="FEE56"},
            {.name="FEE57",.hrw="SPI.-FEE7-HRW",.cnv="SPI.-FEE7-CNV",.col="FEE57"},
            {.name="FEE58",.hrw="SPI.-FEE7-HRW",.cnv="SPI.-FEE7-CNV",.col="FEE58"},
            {.name="FEE59",.hrw="SPI.-FEE7-HRW",.cnv="SPI.-FEE7-CNV",.col="FEE59"},
            {.name="FEE60",.hrw="SPI.-FEE7-HRW",.cnv="SPI.-FEE7-CNV",.col="FEE60"},
            {.name="FEE61",.hrw="SPI.-FEE7-HRW",.cnv="SPI.-FEE7-CNV",.col="FEE61"},
            {.name="FEE62",.hrw="SPI.-FEE7-HRW",.cnv="SPI.-FEE7-CNV",.col="FEE62"},
            {.name="FEE63",.hrw="SPI.-FEE7-HRW",.cnv="SPI.-FEE7-CNV",.col="FEE63"},
            {.name="FEE64",.hrw="SPI.-FEE8-HRW",.cnv="SPI.-FEE8-CNV",.col="FEE64"},
            {.name="FEE65",.hrw="SPI.-FEE8-HRW",.cnv="SPI.-FEE8-CNV",.col="FEE65"},
            {.name="FEE66",.hrw="SPI.-FEE8-HRW",.cnv="SPI.-FEE8-CNV",.col="FEE66"},
            {.name="FEE67",.hrw="SPI.-FEE8-HRW",.cnv="SPI.-FEE8-CNV",.col="FEE67"},
            {.name="FEE68",.hrw="SPI.-FEE8-HRW",.cnv="SPI.-FEE8-CNV",.col="FEE68"},
            {.name="FEE69",.hrw="SPI.-FEE8-HRW",.cnv="SPI.-FEE8-CNV",.col="FEE69"},
            {.name="FEE70",.hrw="SPI.-FEE8-HRW",.cnv="SPI.-FEE8-CNV",.col="FEE70"},
            {.name="FEE71",.hrw="SPI.-FEE8-HRW",.cnv="SPI.-FEE8-CNV",.col="FEE71"},
            {.name="FEE72",.hrw="SPI.-FEE9-HRW",.cnv="SPI.-FEE9-CNV",.col="FEE72"},
            {.name="FEE73",.hrw="SPI.-FEE9-HRW",.cnv="SPI.-FEE9-CNV",.col="FEE73"},
            {.name="FEE74",.hrw="SPI.-FEE9-HRW",.cnv="SPI.-FEE9-CNV",.col="FEE74"},
            {.name="FEE75",.hrw="SPI.-FEE9-HRW",.cnv="SPI.-FEE9-CNV",.col="FEE75"},
            {.name="FEE76",.hrw="SPI.-FEE9-HRW",.cnv="SPI.-FEE9-CNV",.col="FEE76"},
            {.name="FEE77",.hrw="SPI.-FEE9-HRW",.cnv="SPI.-FEE9-CNV",.col="FEE77"},
            {.name="FEE78",.hrw="SPI.-FEE9-HRW",.cnv="SPI.-FEE9-CNV",.col="FEE78"},
            {.name="FEE79",.hrw="SPI.-FEE9-HRW",.cnv="SPI.-FEE9-CNV",.col="FEE79"},
            {.name="FEE80",.hrw="SPI.-FEEA-HRW",.cnv="SPI.-FEEA-CNV",.col="FEE80"},
            {.name="FEE81",.hrw="SPI.-FEEA-HRW",.cnv="SPI.-FEEA-CNV",.col="FEE81"},
            {.name="FEE82",.hrw="SPI.-FEEA-HRW",.cnv="SPI.-FEEA-CNV",.col="FEE82"},
            {.name="FEE83",.hrw="SPI.-FEEA-HRW",.cnv="SPI.-FEEA-CNV",.col="FEE83"},
            {.name="FEE84",.hrw="SPI.-FEEA-HRW",.cnv="SPI.-FEEA-CNV",.col="FEE84"},
            {.name="FEE85",.hrw="SPI.-FEEA-HRW",.cnv="SPI.-FEEA-CNV",.col="FEE85"},
            {.name="FEE86",.hrw="SPI.-FEEA-HRW",.cnv="SPI.-FEEA-CNV",.col="FEE86"},
            {.name="FEE87",.hrw="SPI.-FEEA-HRW",.cnv="SPI.-FEEA-CNV",.col="FEE87"},
            {.name="FEE88",.hrw="SPI.-FEEB-HRW",.cnv="SPI.-FEEB-CNV",.col="FEE88"},
            {.name="FEE89",.hrw="SPI.-FEEB-HRW",.cnv="SPI.-FEEB-CNV",.col="FEE89"},
            {.name="FEE90",.hrw="SPI.-FEEB-HRW",.cnv="SPI.-FEEB-CNV",.col="FEE90"},
            {.name="FEE91",.hrw="SPI.-FEEB-HRW",.cnv="SPI.-FEEB-CNV",.col="FEE91"},
			{.name="\0",.hrw="",.cnv="",.col=""}
		};



typedef struct {
	double ijd;
	unsigned int exposure;
} bint;

typedef struct {
	double rate;
	double error;
	int flag;
} bins;

void printerror(int status, int fatal) {
	if (status)
	{
		fits_report_error(stderr, status);

		if (fatal) {
			RILlogMessage(NULL,Error_0,"Fatal! Execution failed with exit code %i",status);
			exit( status );
		};
	}
	return;
};

double sec_in_day(char utc[]) {
	int g_y, g_m, g_d, g_H, g_M;
  	double g_S;

	sscanf(utc, "%04d-%02d-%02dT%02d:%02d:%lf", &g_y, &g_m, &g_d, &g_H, &g_M, &g_S);
        return g_H * 3600 + g_M * 60 + g_S;
};

int acsdump_xyz2radec(double x, double y, double z, double *ra, double *dec, double *dist)
 {
   *dist = sqrt(x * x + y * y + z * z);
   *dec = DEGRAD * asin(z / *dist);
   *ra = DEGRAD * atan2(y, x);
   if (*ra < 0.0) *ra += 360.0;
   return(0);
 }


inline int RILerror(int status,MessageType mt, const char* format, ... ) {
    if (status==ISDC_OK) return status;

    char str[MAXSTRING]="";

    va_list args;
    va_start( args, format );
    vsprintf(str, format, args );
    va_end( args );

    RILlogMessage(NULL,mt,str);

    if (mt==Error_0 || mt==Error_1 || mt==Error_2 || mt==Error_3) 
    	printerror(status,1);
    else
    	printerror(status,0);
    return status;
}


const double acsbin=0.05/24/3600;

double orbit_accy;

int timecorr125ms=0;

double *times, *tdels;
float *rates, *errors, *frexps;
TFLAG *flags;
double *r_rates, *r_times;

double sid_ref;

/*const double rev_days=2.990997;about 3 sideral days use perigee200 to perigee300
  const double rev_T0= 1046.3121-10*rev_days; rev 10 started 2002-10-17T-5:47:25*/

const double rev_days=2.990997;
const double rev_T0=1016.4021300000001;

// IJD to revolution
int revol(double tstart){
	double t=(tstart-rev_T0)/rev_days;
	int i=(int)t;
	return i;
}

// IJD to satellite orbit phase
double phase(double tstart){
	double t;
	t=fmod(tstart-rev_T0,rev_days);
	t/=rev_days;
	return t;
}

// read SPI-ACS lightcurve from the ScW
int readscw(char *swgfile, dataspec * dss[],char ntargets,FILE * outf,double rstart,double rstop,int nrev,long *n,int *pstatus) {
	dal_element* g=NULL;
	dal_element* x=NULL;
	dal_element* x1=NULL;

	double tstart,tstop;

	int status=*pstatus;
	

	status=DALobjectOpen(swgfile,&g,0);
	if (RILerror(status,Warning_2,"unable to open %s: skipping",swgfile)!=ISDC_OK) return 0;
	
	status=DALobjectFindElement(g,"GNRL-SCWG-GRP",&x,0);
	if (RILerror(status,Warning_2,"unable to read GNRL-SCWG-GRP: skipping")!=ISDC_OK) return 0;

	status=DALattributeGetReal(x,"TSTART",&tstart,NULL,NULL,status);
	if (RILerror(status,Warning_2,"unable to read TSTART: skipping")!=ISDC_OK) return 0;

	status=DALattributeGetReal(x,"TSTOP",&tstop,NULL,NULL,status);
	if (RILerror(status,Warning_2,"unable to read TSTOP: skipping")!=ISDC_OK) return 0;

	RILlogMessage(NULL,Log_2,"from %.10lg to %.10lg (while %.10lg - %.10lg)\n",tstart,tstop,rstart,rstop);

	if (tstop<rstart) {
		RILlogMessage(NULL,Log_2,"not yet");
		return 0;
	};
	
	if (tstart>rstop) {
		RILlogMessage(NULL,Log_2,"no more");
		return -1;
	};

    int itarget;
    long int total_rows=0;

    for (itarget=0;itarget<ntargets;itarget++) {
        dataspec *ds=dss[itarget];

        status=DALobjectFindElement(g,ds->hrw,&x,0);
        if (RILerror(status,Warning_2,"unable to read %s: skipping",ds->hrw)!=ISDC_OK) continue;

        status=DALobjectFindElement(g,ds->cnv,&x1,0);
        if (RILerror(status,Warning_2,"unable to read %s: skipping",ds->cnv)!=ISDC_OK) continue;

        long rows;
        status=DALtableGetNumRows(x,&rows,0);
        if (RILerror(status,Warning_2,"unable to read: skipping")!=ISDC_OK) return 0;

        long rows1;
        status=DALtableGetNumRows(x1,&rows1,0);
        if (RILerror(status,Warning_2,"unable to read: skipping")!=ISDC_OK) return 0;

        if (rows!=rows1) {
            RILlogMessage(NULL,Warning_2,"inconsistent number of rows: skipping\n");
            return 0;
        };

        RILlogMessage(NULL,Log_1,"%li bins",rows);


        double * rate=malloc(sizeof(double)*rows);
        double * ijd=malloc(sizeof(double)*rows);
        OBTime * obt=malloc(sizeof(OBTime)*rows);

        dal_dataType typ=DAL_DOUBLE;
        status=DALtableGetCol(x,ds->col,0,&typ,&rows,rate,0);
        if (RILerror(status,Warning_2,"unable to read the rate")!=ISDC_OK) return 0;

        status=DAL3GENtableGetOBT(x1,"OB_TIME",0,&rows,obt,0);
        if (RILerror(status,Warning_2,"unable to read the time")!=ISDC_OK) return 0;

        status=DAL3AUXconvertOBT2IJDRev(nrev,TCOR_ANY,rows,obt,ijd,0);
        if (RILerror(status,Warning_2,"unable to convert the time")!=ISDC_OK) return 0;

        int i;
        int nbadtimes=0;

        for (i=0;i<rows;i++) {
            if (ijd[i]<tstart || ijd[i]>tstop) {
                //RILlogMessage(NULL,Log_2,"bad time: %.15lg",ijd[i]);
                nbadtimes++;
            //	continue;
            };
            
            if (ijd[i]<rstart) {
                continue;
            };
            
            if (ijd[i]>rstop) {
                break;
            };

            double _ijd=ijd[i];
            
            if (ntargets==1)
                fprintf(outf,"%.20lg %.10lg %.5lg %.15lg\n",_ijd,(_ijd-rstart)*24.*3600.,rate[i],sid_ref+(_ijd-rstart)*24.*3600.);
            else
                fprintf(outf,"%.20lg %.10lg %.5lg %.15lg %i %s\n",_ijd,(_ijd-rstart)*24.*3600.,rate[i],sid_ref+(_ijd-rstart)*24.*3600.,itarget,ds->name);

            (*n)++;
        };

        RILlogMessage(NULL,Log_2,"filled to %i, %i bad times excluded\n",*n,nbadtimes);

        free(rate);
        free(ijd);
        free(obt);


        *pstatus=0;
        //*pstatus=status;
        total_rows+=rows-nbadtimes;
    };
    status=DALobjectClose(g,DAL_SAVE,0);
    if (RILerror(status,Warning_2,"unable to close the ScW")!=ISDC_OK) return 0;

	return total_rows;
};

char split_targets(char target_list_str[],char *targets[]) {
    char *token;
    int i=0;
    while ((token = strsep(&target_list_str, ",")) && (i<MAX_TARGETS))  {
        if (strcmp(token,"FEEALL")==0) {
            int ifee;
            for (ifee=1;ifee<=91;ifee++) {
                asprintf(&targets[i++],"FEE%i",ifee);
                printf("adding %s",targets[i-1]);
            };
        } else if (strcmp(token,"ISGRIRAW")==0) {
            int ifee;
            for (ifee=0;ifee<=7;ifee++) {
                asprintf(&targets[i++],"ISGRIRAW_MCE%i",ifee);
                printf("adding %s",targets[i-1]);
            };
        } else {
            targets[i++]=token;
            printf("adding %s",targets[i-1]);
        };
    };
    return i;
};

int dump_spiacs(){
	RILlogMessage(NULL,Log_0,"Running %s %s",TOOLNAME,TOOLVERSION);

	int rev=-1;
	int status=0;

	int n=0;

	char *arcbase;

	int i=0;

	struct dirent **filelist;

	char rdir[MAXPATH]="";

	char scwdir[MAXPATH];
	char swgfile[MAXPATH];
	char outputfile[MAXPATH];
	char start_utc[MAXPATH];
	char stop_utc[MAXPATH];
	
    char target_list_str[MAXPATH];
    char *targets[MAX_TARGETS];
    char ntargets=0;

	double tstart,tstop;

	long cn=0;

	int maxfiles=-1;
	int nread=0;

    int mode=0;


	r_rates=malloc(sizeof(double)*MAXBINS);
	r_times=malloc(sizeof(double)*MAXBINS);

	times=malloc(sizeof(double)*MAXBINS);
	tdels=malloc(sizeof(double)*MAXBINS);
	rates=malloc(sizeof(float)*MAXBINS);
	errors=malloc(sizeof(float)*MAXBINS);
	frexps=malloc(sizeof(float)*MAXBINS);
	flags=malloc(sizeof(TFLAG)*MAXBINS);

	RILerror(PILGetString("output",outputfile),Error_0,"Problem getting \"output\" parameter");
	RILerror(PILGetString("target",target_list_str),Error_0,"");
	RILerror(PILGetString("start_time_utc",start_utc),Error_0,"");
	RILerror(PILGetString("stop_time_utc",stop_utc),Error_0,"");
	RILerror(PILGetReal("orbit_accy",&orbit_accy),Error_0,"");
	RILerror(PILGetInt("maxfiles",&maxfiles),Error_0,"Problem getting maxfiles parameter");

	RILerror(PILGetInt("mode",&mode),Error_0,"Problem getting mode parameter");

    ntargets=split_targets(target_list_str,targets);


	double start_ijd,stop_ijd;

    dataspec * ds[MAX_TARGETS];

    i=0;
    for (i=0;i<ntargets;i++) {
        printf("target: %i %s\n",i,targets[i]);
        dataspec * ids;
        for (ids=dataspecs;ids->name[0]!=0;++ids) {
            ds[i]=ids;
            if (strcmp(ds[i]->name,targets[i])==0) {
                RILlogMessage(NULL,Log_2,"target %s selected",ds[i]->name);
                break;
            };
        };
        if (ds[i]->name[0]==0) {
            RILerror(-1,Error_0,"unknown target %s",targets[i]);
            return -1;
        };
    };



	int astatus=ISDC_OK;
	astatus=DAL3GENconvertUTC2IJD(start_utc,&start_ijd,astatus);
	astatus=DAL3GENconvertUTC2IJD(stop_utc,&stop_ijd,astatus);
	

	int arev=rev;
	astatus = DAL3AUXgetRevolution( start_ijd, &rev, astatus);
	astatus = DAL3AUXgetRevolution( stop_ijd, &arev, astatus);

	if (arev!=rev) {
		RILerror(-1,Error_0,"overrevolution!");
		return -1;
	};

	RILlogMessage(NULL,Log_2,"requested %.10lg - %.10lg\n",start_ijd,stop_ijd);
	RILlogMessage(NULL,Log_2,"revolution %i\n",rev);
	
	// dry run - no reason to prodce if can not write

	FILE * outfile=fopen(outputfile,"w");
	if (outfile==NULL) {
		RILerror(-1,Error_0,"cannot open output file!");
		return -1;
	};
    
    if (mode!=0 && mode!=1 && mode!=2) {
	    RILlogMessage(NULL,Log_2,"Mode %i is undefined!",mode);
    };
    
    if (mode==2) {
        double ra_scx, dec_scx, ra_scz, dec_scz;
        RILlogMessage(NULL,Log_2,"Mode 2 is for extraction of the orbit parameters");
        RILerror(DAL3AUXgetAttitude(NULL,  DAL3AUX_ANY, 1, &start_ijd, &ra_scx, &dec_scx,&ra_scz, &dec_scz, status),Error_0,"Error in DAL3AUXgetAttitude");
                                               
        RILlogMessage(NULL,Log_2,"ijd: %.20lg scx: %.15lg,%.15lg scz: %.15lg,%.15lg, status: %i\n",start_ijd,ra_scx,dec_scx,ra_scz,dec_scz,status); 

        fprintf(outfile, "#spacecraft X(RA,DEC) Z(RA,DEC)\n");
        fprintf(outfile, "%.3lf %.3lf %.3lf %.3lf\n", ra_scx, dec_scx, ra_scz, dec_scz);
        fclose(outfile);
        return 0;
    };

    
    if (mode==1) {
        double x,y,z,ra,dec,dist;
        RILlogMessage(NULL,Log_2,"Mode 1 is for extraction of the orbit parameters");
        RILerror(DAL3AUXgetOrbitPos(NULL, DAL3AUX_HISTORIC, 1, &start_ijd, &x, &y, &z, astatus),Error_0,"Error in DAL3AUXgetOrbitPos");

        acsdump_xyz2radec(x, y, z, &ra, &dec, &dist);
        fprintf(outfile, "%.3lf %.3lf %.1lf\n", ra, dec, dist);
        fprintf(outfile, "%.3lf %.3lf %.1lf", atan(orbit_accy / dist) * 180. / 3.1415926535,
                atan(orbit_accy / dist) * 180. / 3.1415926535, orbit_accy);
        fclose(outfile);
        return 0;
    };

    if (mode==0) {
	RILlogMessage(NULL,Log_2,"Mode 0 means extracting the light curve");

	arcbase=getenv("REP_BASE_PROD");

	snprintf(rdir,MAXPATH,"%s/scw/%.4i",arcbase,rev);
	RILlogMessage(NULL,Log_2,"in %s\n",rdir);
	n = scandir(rdir, &filelist, 0, alphasort);
	RILlogMessage(NULL,Log_2,"Found %d ScW files\n",n-2);

	if (n<=2) {
		RILlogMessage(NULL,Warning_2,"No ScW files in here! Nothing created\n",n);
		return 0; 
	};

	sid_ref=sec_in_day(start_utc);

	fprintf(outfile,"# please take care! the UTC reference times are computed assuming no leap seconds in the requested time interval\n");
	fprintf(outfile,"# (which is a very infrequent occasion, of course)\n");
	fprintf(outfile,"# reference (start) UTC is: %s %.15lg\n",start_utc,sid_ref);
	fprintf(outfile,"# columns are:\n");
	fprintf(outfile,"# [IJD] [seconds since reference] [counts in bin] [seconds since midnight]\n");

	if (maxfiles<0) maxfiles=n;
	else {
		RILlogMessage(NULL,Warning_2,"Following the request only %i of %i files used\n",maxfiles,n);
	};

	for (i=0;i<n && maxfiles>0;i++){  
		if (!strncmp(filelist[i]->d_name,".",1)) continue;
		if (!strncmp(filelist[i]->d_name,"rev",3)) continue;

		strcpy(scwdir,"");
		strncat(scwdir,rdir,MAXPATH);
		strncat(scwdir,"/",MAXPATH);
		strncat(scwdir,filelist[i]->d_name,MAXPATH);

		RILlogMessage(NULL,Log_2,"%d/%d %s\n",i+1,n,scwdir);

		if (i<n-1 && !strncmp(filelist[i]->d_name,filelist[i+1]->d_name,12)) {
			RILlogMessage(NULL,Log_2,"next one is a better version of the same pointing\n",i+1,n,scwdir);
			continue;
		};

		strncpy(swgfile,scwdir,MAXPATH);
		strncat(swgfile,"/swg.fits",MAXPATH);

		nread=readscw(swgfile,ds,ntargets,outfile,start_ijd,stop_ijd,rev,&cn,&status);
		fflush(outfile);

		if (nread>0) maxfiles--;
		if (nread<0) break;
	};
	free(filelist);

	fclose(outfile);

	RILlogMessage(NULL,Log_2,"Read %d ScW files with %i bins\n",n,cn);

	RILlogMessage(NULL,Log_2,"Successfully finished.\n");

	return status;
    };
};


int main(int argc,char **argv) {
	CommonInit(TOOLNAME,TOOLVERSION,argc,argv);
	return dump_spiacs();;
};
