// Author   : Alessio Trois
// Version  :   16-Mar-05
// Version  :   11-Jul-11
// Version  :   23-May-12

#include "CorUtilities.h"
double mjdrefi = 53005.;
// 57204.00000000

double mjdreff      = 0.000754444444444;
double mjdreff_2009 = 0.000766018518519;
double mjdreff_2012 = 0.000777592592593;
double mjdreff_2015 = 0.000789166666667;

double obtoi = 54160.;
double obtof      = 0.000754444444444;
double obtof_2009 = 0.000766018518519;
double obtof_2012 = 0.000777592592593;
double obtof_2015 = 0.000789166666667;

//    Il vecchio numero era (ed e' per dati prima di 1/1/2009)
//    0.0007544444444444444444444 = 65.184 / 86400 . = 0.000754444444444
//    Ai nuovi dati oltre il 1 Luglio 2008 si aggiungerà il leap second 
//    Il nuovo quindi e' 66.184 / 86400 = 0.000766018518519
//    Ai nuovi dati oltre il 1 Gennaio 2012 si aggiungerà il leap second 
//    Il nuovo quindi e' 67.184 / 86400 = 0.000777592592593
//    Ai nuovi dati oltre il 30 Giugno 2015 si aggiungerà il leap second 
//    Il nuovo quindi e' 68.184 / 86400 = 0.000789166666667

int diffobtsectt     = 756950400;
    
int diffobtsectt2015 = 756873344;

double diffobtmsect = 0.0;

int diffobtsecttEVT =         -99792000;

int diffobtsecttEVT2015 =    -368227456;

int diffobtmsectEVT2015 = 4000; //microsecondi

int diffobtmsectEVT = 0;

int CorrTimeEVT(fitsfile * output, FILE *fp) {
	
	int oldstatus = 0;
	int status = 0;
	int i = 0;
	int colnum = 0;	
	int ncols = 0;		
	long nrows = 0;
	char unit[FLEN_KEYWORD];
	char * s = "s";	
	double tstart = 0;
	double tstop = 0;
	float timezero = 0;	
	
	fits_movabs_hdu(output, 3, NULL, &status);	
	fits_get_num_rows(output, &nrows, &status);
	fits_get_num_cols(output, &ncols, &status);
	
	double * time = new double[nrows];
	int * micsec = new int[nrows];
	int * sec = new int[nrows];

	float * obt_corr = new float[nrows];

	
	tstart = 0;
	tstop = 0;
	char str[100];

/////////passo 1
	
	strcpy(str, "TT");
	fits_update_key(output, TSTRING, "TIMESYS",  str, NULL, &status);

/////////passo 2
	oldstatus = status;
	fits_delete_key(output, "MJDREF", &status);
	if (status == 202) status = oldstatus;

/////////passo 8 - 9 - 10
	fits_get_colnum(output, 1, "TIME", &colnum, &status);	
	fits_read_col(output, TDOUBLE, colnum, 1, 1, nrows, NULL, time, NULL, &status);		
		
	fits_get_colnum(output, 1, "MICSECTT", &colnum, &status);	
	fits_read_col(output, TUINT, colnum, 1, 1, nrows, NULL, micsec, NULL, &status);	
	
	fits_get_colnum(output, 1, "SECONDTT", &colnum, &status);	
	fits_read_col(output, TUINT, colnum, 1, 1, nrows, NULL, sec, NULL, &status);	
	
	fits_get_colnum(output, 1, "OBTCORR", &colnum, &status);	
	fits_read_col(output, TFLOAT, colnum, 1, 1, nrows, NULL, obt_corr, NULL, &status);		
		


	int orbit = 0;
	char orbitcomment[FLEN_KEYWORD];
  	fits_read_key(output, TINT, "OBS_ID", &orbit, orbitcomment,  &status);	     
// 	cout<<orbit<<endl;		
	
	for (i = 0; i < nrows; ++i) {




////////////////RICIRCOLI/////////////////////////////////////////////////
//timezero viene posto = a zero nei file COR
// il tempo in TT degli eventi sarà calcolato come (MJDREFI +MJDREFF)*86400. + TIME[i]

  
//PRIMO RICIRCOLO SOLO EVENTI 21176
//SECONDO RICIRCOLO EVENTI E PACCHETTI 43222
//NUOVO START MISSION		       43315

// int diffobtsecttEVT         =    -99792000;
// int diffobtsecttEVT2015 =    -368227456;

// int diffobtmsectEVT = 0;
// int diffobtmsectEVT2015 = 4000; //microsecondi


		if (orbit <= 21176) {
			sec[i] = sec[i] - diffobtsecttEVT;
		
			}
		else if (orbit > 21176 && orbit < 43222){
		 	sec[i]=sec[i]+(1<<27)- diffobtsecttEVT;;
			}
		else 	if (orbit >= 43222 && orbit < 43315){	
			sec[i]=sec[i]+(1<<28) - diffobtsecttEVT;			
			}
			
		else if (orbit >= 43315) {
			sec[i]=sec[i]- diffobtsecttEVT2015;
			}
/////////////////////////////////////////////////////////////////////////////////
			
//////////////////////////////////////////////////////////////////////////						
///////////////////obtcorr e microsecondi//////////////////////////////////////////
		micsec[i] = (int)(((double)micsec[i])*(1000000.0/(1000000.0+obt_corr[i])));
		
		if (orbit >= 43315 ) {
			micsec[i] = micsec[i] - diffobtmsectEVT2015;
			}		
		
		if ( micsec[i] >= 1000000) {
			micsec[i] = micsec[i] - 1000000;	
			sec[i] = sec[i] + 1;
			}		
		

		time[i] = sec[i] + (((double)micsec[i])/1000000);
		}


	fits_get_colnum(output, 1, "TIME", &colnum, &status);		
	fits_write_col(output, TDOUBLE, colnum, 1, 1, nrows,  time, &status);	

	
	fits_get_colnum(output, 1, "MICSECTT", &colnum, &status);
	fits_write_col(output, TUINT, colnum, 1, 1, nrows,  micsec, &status);	
		
	fits_get_colnum(output, 1, "SECONDTT", &colnum, &status);		
	fits_write_col(output, TUINT, colnum, 1, 1, nrows,  sec, &status);


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
/////////passo 3
	fits_update_key(output, TDOUBLE, "MJDREFI",  &mjdrefi, NULL, &status);

/////////passo 4

	if(time[0] < 157852800.00000) {
	// 	cout<<"vecchio leap second pkt "<<time[0]<<endl;
		fits_update_key(output, TDOUBLE, "MJDREFF",  &mjdreff, NULL, &status);
		}
	else if((time[0] >= 157852800.00000) && (time[0] < 268185600.)){
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		fits_update_key(output, TDOUBLE, "MJDREFF",  &mjdreff_2009, NULL, &status);
		}
	else if((time[0] >= 268185600) && (time[0] < 362793600)){
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		fits_update_key(output, TDOUBLE, "MJDREFF",  &mjdreff_2012, NULL, &status);
		}		
	else {
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		fits_update_key(output, TDOUBLE, "MJDREFF",  &mjdreff_2015, NULL, &status);
		}			
	
/////////passo 5
	fits_update_key(output, TFLOAT, "TIMEZERO",  &timezero, str, &status);	

/////////passo 6
	fits_update_key(output, TDOUBLE, "OBT0I",  &obtoi, NULL, &status);

/////////passo 7
	if(time[0] < 157852800.00000) {
		fits_update_key(output, TDOUBLE, "OBT0F",  &obtof, NULL, &status);
		}
	else if((time[0] >= 157852800.00000) && (time[0] < 268185600.)){
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		fits_update_key(output, TDOUBLE, "OBT0F",  &obtof_2009, NULL, &status);
		}
	else if((time[0] >= 268185600) && (time[0] < 362793600)){
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		fits_update_key(output, TDOUBLE, "OBT0F",  &obtof_2012, NULL, &status);
		}
	else {
		fits_update_key(output, TDOUBLE, "OBT0F",  &obtof_2015, NULL, &status);
		}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


		
////////passo 11
	fits_get_colnum(output, 1, "TIME", &colnum, &status);
	sprintf(unit, "TUNIT%d", colnum);
	fits_update_key(output, TSTRING, unit, s, NULL, &status);
	
	WriteTime(output, time[0], time[nrows-1]);
	fits_movabs_hdu(output, 1, NULL, &status);	
	fits_update_key(output, TFLOAT, "TIMEZERO",  &timezero, str, &status);
	WriteTime(output, time[0], time[nrows-1]);			


	delete[] time;
	delete[] micsec;
	delete[] sec;
	delete[] obt_corr;
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////	
	return status;
	}



int CorrTimeEVT1(fitsfile * output, FILE *fp) {
//	int * obt_corr;
	int oldstatus = 0;
	int status = 0;
	int  i = 0;
	int colnum = 0;	
	int ncols = 0;		
	long nrows = 0;
	double * time = NULL;
	char unit[FLEN_KEYWORD];
	char * s = "s";	
	double tstart = 0;
	double tstop = 0;
	float timezero = 0;	
	
	fits_movnam_hdu(output, BINARY_TBL, "EVENTS", 0, &status);	
	fits_get_num_rows(output, &nrows, &status);
	fits_get_num_cols(output, &ncols, &status);
	
	time = new double[nrows];
	int * micsec = new int[nrows];
	int * sec = new int[nrows];

// 	obt_corr = new int[nrows];
// 
// 	
	tstart = 0;
	tstop = 0;
	char str[100];


/////////passo 1
	
	strcpy(str, "TT");
	fits_update_key(output, TSTRING, "TIMESYS",  str, NULL, &status);

/////////passo 2
	oldstatus = status;
	fits_delete_key(output, "MJDREF", &status);
	if (status == 202) status = oldstatus;

/////////passo 3
	fits_update_key(output, TDOUBLE, "MJDREFI",  &mjdrefi, NULL, &status);

/////////passo 4

	if(time[0] < 157852800.00000) {
	// 	cout<<"vecchio leap second pkt "<<time[0]<<endl;
		fits_update_key(output, TDOUBLE, "MJDREFF",  &mjdreff, NULL, &status);
		}
	else if((time[0] >= 157852800.00000) && (time[0] < 268185600.)){
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		fits_update_key(output, TDOUBLE, "MJDREFF",  &mjdreff_2009, NULL, &status);
		}
	else if((time[0] >= 268185600) && (time[0] < 362793600)){
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		fits_update_key(output, TDOUBLE, "MJDREFF",  &mjdreff_2012, NULL, &status);
		}		
	else {
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		fits_update_key(output, TDOUBLE, "MJDREFF",  &mjdreff_2015, NULL, &status);
		}			
	
/////////passo 5
	fits_update_key(output, TFLOAT, "TIMEZERO",  &timezero, str, &status);	

/////////passo 6
	fits_update_key(output, TDOUBLE, "OBT0I",  &obtoi, NULL, &status);

/////////passo 7
	if(time[0] < 157852800.00000) {
		fits_update_key(output, TDOUBLE, "OBT0F",  &obtof, NULL, &status);
		}
	else if((time[0] >= 157852800.00000) && (time[0] < 268185600.)){
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		fits_update_key(output, TDOUBLE, "OBT0F",  &obtof_2009, NULL, &status);
		}
	else if((time[0] >= 268185600) && (time[0] < 362793600)){
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		fits_update_key(output, TDOUBLE, "OBT0F",  &obtof_2012, NULL, &status);
		}
	else {
		fits_update_key(output, TDOUBLE, "OBT0F",  &obtof_2015, NULL, &status);
		}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/////////passo 8 - 9 - 10
	fits_get_colnum(output, 1, "TIME", &colnum, &status);	
	fits_read_col(output, TDOUBLE, colnum, 1, 1, nrows, NULL, time, NULL, &status);		
		
	fits_get_colnum(output, 1, "MICSECTT", &colnum, &status);	
	fits_read_col(output, TUINT, colnum, 1, 1, nrows, NULL, micsec, NULL, &status);	
	
	fits_get_colnum(output, 1, "SECONDTT", &colnum, &status);	
	fits_read_col(output, TUINT, colnum, 1, 1, nrows, NULL, sec, NULL, &status);	
	
/*	fits_get_colnum(output, 1, "OBTCORR", &colnum, &status);	
	fits_read_col(output, TINT, colnum, 1, 1, nrows, NULL, obt_corr, NULL, &status);*/		
	int orbit = 0;
	char orbitcomment[FLEN_KEYWORD];
  	fits_read_key(output, TINT, "OBS_ID", &orbit, orbitcomment,  &status);	     
// 	cout<<orbit<<endl;		
	for (i = 0; i < nrows; ++i) {
// 		int diffobtsecttEVT = 99791934;
// 		double diffobtmsectEVT = 816000.0;
// 		double microsec=MicSecCol*(1E-6)*(1000000.0/(1000000.0+ObtCor));

	
////////////////RICIRCOLI/////////////////////////////////////////////////
//timezero viene posto = a zero nei file COR
// il tempo in TT degli eventi sarà calcolato come (MJDREFI +MJDREFF)*86400. + TIME[i]

  
//PRIMO RICIRCOLO SOLO EVENTI 21176
//SECONDO RICIRCOLO EVENTI E PACCHETTI 43222
//NUOVO START MISSION		       43315

// int diffobtsecttEVT         =    -99792000;
// int diffobtsecttEVT2015 =    -368227456;

// int diffobtmsectEVT = 0;
// int diffobtmsectEVT2015 = 4000; //microsecondi


		if (orbit <= 21176) {
			sec[i] = sec[i] - diffobtsecttEVT;
		
			}
		else if (orbit > 21176 && orbit < 43222){
		 	sec[i]=sec[i]+(1<<27)- diffobtsecttEVT;;
			}
		else 	if (orbit >= 43222 && orbit < 43315){	
			sec[i]=sec[i]+(1<<28) - diffobtsecttEVT;			
			}
			
		else if (orbit >= 43315) {
			sec[i]=sec[i]- diffobtsecttEVT2015;
			}
/////////////////////////////////////////////////////////////////////////////////
			
//////////////////////////////////////////////////////////////////////////						
///////////////////obtcorr e microsecondi//////////////////////////////////////////


		if (orbit >= 43315 ) {
			micsec[i] = micsec[i] - diffobtmsectEVT2015;
			}
			
		if ( micsec[i] >= 1000000) {
			micsec[i] = micsec[i] - 1000000;	
			sec[i] = sec[i] + 1;
			}
		time[i] = sec[i] + (((double)micsec[i])/1000000);
		

		}
	
	fits_get_colnum(output, 1, "TIME", &colnum, &status);		
	fits_write_col(output, TDOUBLE, colnum, 1, 1, nrows,  time, &status);	

	
	fits_get_colnum(output, 1, "MICSECTT", &colnum, &status);
	fits_write_col(output, TUINT, colnum, 1, 1, nrows,  micsec, &status);	
		
	fits_get_colnum(output, 1, "SECONDTT", &colnum, &status);		
	fits_write_col(output, TUINT, colnum, 1, 1, nrows,  sec, &status);

		
////////passo 11
	fits_get_colnum(output, 1, "TIME", &colnum, &status);
	sprintf(unit, "TUNIT%d", colnum);
	fits_update_key(output, TSTRING, unit, s, NULL, &status);
	

	WriteTime(output, time[0], time[nrows-1]);
	fits_movabs_hdu(output, 1, NULL, &status);	
	fits_update_key(output, TFLOAT, "TIMEZERO",  &timezero, str, &status);
	WriteTime(output, time[0], time[nrows-1]);			


	delete[] time;
	delete[] micsec;
	delete[] sec;
//	delete[] obt_corr;	
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////	
	return status;
	}


	
int CorrTimeEVT3904(fitsfile * output, FILE *fp) {
//	int * obt_corr;
	int oldstatus = 0;
	int status = 0;
	int  i = 0;
	int colnum = 0;	
	int ncols = 0;		
	long nrows = 0;
	double * time = NULL;
	char unit[FLEN_KEYWORD];
	char * s = "s";	
	double tstart = 0;
	double tstop = 0;
	float timezero = 0;	
	
	fits_movnam_hdu(output, BINARY_TBL, "EVENTS", 0, &status);	
	fits_get_num_rows(output, &nrows, &status);
	fits_get_num_cols(output, &ncols, &status);
	
	time = new double[nrows];
	int * micsec = new int[nrows];
	int * sec = new int[nrows];

// 	obt_corr = new int[nrows];
// 
// 	
	tstart = 0;
	tstop = 0;
	char str[100];


/////////passo 1
	
	strcpy(str, "TT");
	fits_update_key(output, TSTRING, "TIMESYS",  str, NULL, &status);

/////////passo 2
	oldstatus = status;
	fits_delete_key(output, "MJDREF", &status);
	if (status == 202) status = oldstatus;

/////////passo 3
	fits_update_key(output, TDOUBLE, "MJDREFI",  &mjdrefi, NULL, &status);

/////////passo 4

	if(time[0] < 157852800.00000) {
	// 	cout<<"vecchio leap second pkt "<<time[0]<<endl;
		fits_update_key(output, TDOUBLE, "MJDREFF",  &mjdreff, NULL, &status);
		}
	else if((time[0] >= 157852800.00000) && (time[0] < 268185600.)){
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		fits_update_key(output, TDOUBLE, "MJDREFF",  &mjdreff_2009, NULL, &status);
		}
	else if((time[0] >= 268185600) && (time[0] < 362793600)){
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		fits_update_key(output, TDOUBLE, "MJDREFF",  &mjdreff_2012, NULL, &status);
		}		
	else {
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		fits_update_key(output, TDOUBLE, "MJDREFF",  &mjdreff_2015, NULL, &status);
		}			
	
/////////passo 5
	fits_update_key(output, TFLOAT, "TIMEZERO",  &timezero, str, &status);	

/////////passo 6
	fits_update_key(output, TDOUBLE, "OBT0I",  &obtoi, NULL, &status);

/////////passo 7
	if(time[0] < 157852800.00000) {
		fits_update_key(output, TDOUBLE, "OBT0F",  &obtof, NULL, &status);
		}
	else if((time[0] >= 157852800.00000) && (time[0] < 268185600.)){
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		fits_update_key(output, TDOUBLE, "OBT0F",  &obtof_2009, NULL, &status);
		}
	else if((time[0] >= 268185600) && (time[0] < 362793600)){
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		fits_update_key(output, TDOUBLE, "OBT0F",  &obtof_2012, NULL, &status);
		}
	else {
		fits_update_key(output, TDOUBLE, "OBT0F",  &obtof_2015, NULL, &status);
		}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/////////passo 8 - 9 - 10
	fits_get_colnum(output, 1, "TIME", &colnum, &status);	
	fits_read_col(output, TDOUBLE, colnum, 1, 1, nrows, NULL, time, NULL, &status);		
	
/*	fits_get_colnum(output, 1, "OBTCORR", &colnum, &status);	
	fits_read_col(output, TINT, colnum, 1, 1, nrows, NULL, obt_corr, NULL, &status);*/		
		
	for (i = 0; i < nrows; ++i) {
		time[i] = time[i]/1000 -diffobtsecttEVT;
		}
	
	fits_get_colnum(output, 1, "TIME", &colnum, &status);		
	fits_write_col(output, TDOUBLE, colnum, 1, 1, nrows,  time, &status);	

	
	fits_get_colnum(output, 1, "MICSECTT", &colnum, &status);
	fits_write_col(output, TUINT, colnum, 1, 1, nrows,  micsec, &status);	
		
	fits_get_colnum(output, 1, "SECONDTT", &colnum, &status);		
	fits_write_col(output, TUINT, colnum, 1, 1, nrows,  sec, &status);

		
////////passo 11
	fits_get_colnum(output, 1, "TIME", &colnum, &status);
	sprintf(unit, "TUNIT%d", colnum);
	fits_update_key(output, TSTRING, unit, s, NULL, &status);
	

	WriteTime(output, time[0], time[nrows-1]);	
	fits_movabs_hdu(output, 1, NULL, &status);	
	fits_update_key(output, TFLOAT, "TIMEZERO",  &timezero, str, &status);
	WriteTime(output, time[0], time[nrows-1]);			


	delete[] time;
	delete[] micsec;
	delete[] sec;
//	delete[] obt_corr;	
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////	
	return status;
	}



int CorrTimePKT(fitsfile * output, FILE *fp) {
	int oldstatus = 0;
	int status = 0;
	int  i = 0;
	int colnum = 0;	
	int ncols = 0;		
	int * secpktobt;
	int * microsecpktobt;	
	long nrows = 0;
	double * time = NULL;
	char unit[FLEN_KEYWORD];
	char * s = "s";	
	int PKTTYPE = 0;
	int PKTSTYPE = 0;	
///////////////////////////////////////////////////////////////////////////////////////////////			
//////////////////////////////PACKETS//////////////////////////////////////////////////////////	
///////////////////////////////////////////////////////////////////////////////////////////////
	
	fits_movabs_hdu(output, 2, NULL, &status);	
	fits_get_num_rows(output, &nrows, &status);
	fits_get_num_cols(output, &ncols, &status);	
		
	time = new double[nrows];
	microsecpktobt = new int[nrows];	
	secpktobt = new int[nrows];

	char str[100];

	int orbit = 0;
	char orbitcomment[FLEN_KEYWORD];
  	fits_read_key(output, TINT, "OBS_ID", &orbit, orbitcomment,  &status);	    	

	fits_read_key(output, TINT, "PKTTYPE", &PKTTYPE, NULL, &status );
	fits_read_key(output, TINT, "PKTSTYPE", &PKTSTYPE, NULL, &status );	
		
	
/////////passo 1
	
	strcpy(str, "TT");
	fits_update_key(output, TSTRING, "TIMESYS",  str, NULL, &status);

/////////passo 2
	oldstatus = status;
	fits_delete_key(output, "MJDREF", &status);
	if (status == 202) status = oldstatus;
	
/////////passo 3
	fits_update_key(output, TDOUBLE, "MJDREFI",  &mjdrefi, NULL, &status);	
	
/////////passo 5
	float timezero = 0;
	fits_update_key(output, TFLOAT, "TIMEZERO",  &timezero, str, &status);	

/////////passo 6 - 7 - 8
	colnum = 20;
	fits_read_col(output, TDOUBLE, colnum, 1, 1, nrows, NULL, time, NULL, &status);
	
	fits_get_colnum(output, 1, "OBTMSETT", &colnum, &status);	
	fits_read_col(output, TINT, colnum, 1, 1, nrows, NULL, microsecpktobt, NULL, &status);
	
	fits_get_colnum(output, 1, "OBTSECTT", &colnum, &status);	
	fits_read_col(output, TINT, colnum, 1, 1, nrows, NULL, secpktobt, NULL, &status);	

	
	
	for (i = 0; i < nrows; ++i) {
		// int diffobtsectt = 756950334;
		// double diffobtmsect = 816000.0;
		microsecpktobt[i] = microsecpktobt[i] - (int)diffobtmsect;

		if (orbit >= 43222 && orbit < 43315){		
			if (PKTTYPE==1 && PKTSTYPE ==1) {
				secpktobt[i] = secpktobt[i] -diffobtsectt;
				}
			else {
				secpktobt[i] = secpktobt[i]  +(1<<28) -diffobtsectt;
				}
		
			
			}		
		else 	if (orbit >= 43315){		        
			
			if (PKTTYPE==1 && PKTSTYPE ==1) {
// 				cout<<"entrato qui"<<endl;
				secpktobt[i] = secpktobt[i] -diffobtsectt2015 -77056;
				}
			else {
				secpktobt[i] = secpktobt[i]   -diffobtsectt2015;
				}
			}
		else {	
			secpktobt[i] = secpktobt[i] - diffobtsectt;
			}
		if ( microsecpktobt[i] < 0) {
			microsecpktobt[i] = microsecpktobt[i] + 1000000;	
			secpktobt[i] = secpktobt[i] - 1;
			}
		time[i] = secpktobt[i] + (((double)microsecpktobt[i])/1000000);
		}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////cor_3908/////////////////////////
//////////////////////////////////////////////////////////////////////////////

	


/////////passo 4

	if(time[0] < 157852800.00000) {
	// 	cout<<"vecchio leap second pkt "<<time[0]<<endl;
		fits_update_key(output, TDOUBLE, "MJDREFF",  &mjdreff, NULL, &status);
		}
	else if((time[0] >= 157852800.00000) && (time[0] < 268185600.)){
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		fits_update_key(output, TDOUBLE, "MJDREFF",  &mjdreff_2009, NULL, &status);
		}
	else if((time[0] >= 268185600) && (time[0] < 362793600)){
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		fits_update_key(output, TDOUBLE, "MJDREFF",  &mjdreff_2012, NULL, &status);
		}		
	else {
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		fits_update_key(output, TDOUBLE, "MJDREFF",  &mjdreff_2015, NULL, &status);
		}			
	
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

	
	fits_get_colnum(output, 1, "OBTMSETT", &colnum, &status);
	fits_write_col(output, TINT, colnum, 1, 1, nrows,  microsecpktobt, &status);	
	
	fits_get_colnum(output, 1, "OBTSECTT", &colnum, &status);		
	fits_write_col(output, TINT, colnum, 1, 1, nrows,  secpktobt, &status);	

	colnum = 20;
	fits_write_col(output, TDOUBLE, colnum, 1, 1, nrows,  time, &status);

/////////passo 9
	sprintf(unit, "TUNIT%d", colnum);
	fits_update_key(output, TSTRING, unit, s, NULL, &status);

/////////passo 10 - 11
	WriteTime(output, time[0], time[nrows-1]);
	fits_movabs_hdu(output, 1, NULL, &status);	
	fits_update_key(output, TFLOAT, "TIMEZERO",  &timezero, str, &status);
	WriteTime(output, time[0], time[nrows-1]);	


	delete [] secpktobt;	
	delete [] time;	
	delete [] microsecpktobt;		
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////	
	return status;
}



int CorrTimePKT3911(fitsfile * output, FILE *fp) {

	int oldstatus = 0;
	int status = 0;
	int  i = 0;
	int colnum = 0;	
	int ncols = 0;		
	int * secpktobt;
	int * microsecpktobt;	
	long nrows = 0;
	double * time = NULL;
	char unit[FLEN_KEYWORD];
	char * s = "s";	

	char str[100];

	int orbit = 0;
	char orbitcomment[FLEN_KEYWORD];
  	fits_read_key(output, TINT, "OBS_ID", &orbit, orbitcomment,  &status);	   	
	
for (i = 3; i <= 4; ++i) {	
///////////////////////////////////////////////////////////////////////////////////////////////			
//////////////////////////////PACKETS//////////////////////////////////////////////////////////	
///////////////////////////////////////////////////////////////////////////////////////////////



	fits_movabs_hdu(output, i, NULL, &status);	
	fits_get_num_rows(output, &nrows, &status);
	fits_get_num_cols(output, &ncols, &status);	
	cout<<nrows<<endl;		
	time = new double[nrows];
	microsecpktobt = new int[nrows];	
	secpktobt = new int[nrows];

	char str[100];

/////////passo 1
	
	strcpy(str, "TT");
	fits_update_key(output, TSTRING, "TIMESYS",  str, NULL, &status);

/////////passo 2
	oldstatus = status;
	fits_delete_key(output, "MJDREF", &status);
	if (status == 202) status = oldstatus;
	
/////////passo 3
	fits_update_key(output, TDOUBLE, "MJDREFI",  &mjdrefi, NULL, &status);

/////////passo 4

	if(time[0] < 157852800.00000) {
	// 	cout<<"vecchio leap second pkt "<<time[0]<<endl;
		fits_update_key(output, TDOUBLE, "MJDREFF",  &mjdreff, NULL, &status);
		}
	else if((time[0] >= 157852800.00000) && (time[0] < 268185600.)){
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		fits_update_key(output, TDOUBLE, "MJDREFF",  &mjdreff_2009, NULL, &status);
		}
	else if((time[0] >= 268185600) && (time[0] < 362793600)){
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		fits_update_key(output, TDOUBLE, "MJDREFF",  &mjdreff_2012, NULL, &status);
		}		
	else {
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		fits_update_key(output, TDOUBLE, "MJDREFF",  &mjdreff_2015, NULL, &status);
		}			
	
/////////passo 5
	float timezero = 0;
	fits_update_key(output, TFLOAT, "TIMEZERO",  &timezero, str, &status);	

/////////passo 6 - 7 - 8
	fits_get_colnum(output, 1, "TIME", &colnum, &status);
	fits_read_col(output, TDOUBLE, colnum, 1, 1, nrows, NULL, time, NULL, &status);
	
	for (int k = 0; k < nrows; ++k) {
	
		time[k] = (time[k]/1000000.);
		
		if (orbit >= 43315){		        
			time[k]=time[k]+(1<<28)+77056;
			}
		time[k] = time[k] - diffobtsectt;	
		
		}

	fits_get_colnum(output, 1, "TIME", &colnum, &status);
	fits_write_col(output, TDOUBLE, colnum, 1, 1, nrows,  time, &status);

/////////passo 9
	sprintf(unit, "TUNIT%d", colnum);
	fits_update_key(output, TSTRING, unit, s, NULL, &status);

/////////passo 10 - 11
	WriteTime(output, time[0], time[nrows-1]);
	fits_movabs_hdu(output, 1, NULL, &status);	
	fits_update_key(output, TFLOAT, "TIMEZERO",  &timezero, str, &status);
	WriteTime(output, time[0], time[nrows-1]);	


	delete [] secpktobt;	
	delete [] time;	
	delete [] microsecpktobt;
}			
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////	



	return status;
}



int CorrTimePKT3908(fitsfile * output, FILE *fp) {

	int oldstatus = 0;
	int status = 0;
	int  i = 0;
	int colnum = 0;	
	int ncols = 0;		
	int * secpktobt;
	int * microsecpktobt;	
	long nrows = 0;
	double * time = NULL;
	char unit[FLEN_KEYWORD];
	char * s = "s";	


///////////////////////////////////////////////////////////////////////////






	fits_movabs_hdu(output, 2, NULL, &status);	
	fits_get_num_rows(output, &nrows, &status);

	int * bstarthi = new int[nrows];
	int * bstartlo = new int[nrows];
	int * bstophi = new int[nrows];
	int * bstoplo = new int[nrows];
	short * obt_corr = new short[nrows];
	double * bstart = new double[nrows];
	double * bstop  =  new double[nrows];

	fits_get_num_cols(output, &ncols, &status);

	fits_get_colnum(output, 1, "BSTARTHI", &colnum, &status);	
	fits_read_col(output, TINT, colnum, 1, 1, nrows, NULL, bstarthi, NULL, &status);
	
	fits_get_colnum(output, 1, "BSTARTLO", &colnum, &status);	
	fits_read_col(output, TINT, colnum, 1, 1, nrows, NULL, bstartlo, NULL, &status);

	fits_get_colnum(output, 1, "BSTOPHI", &colnum, &status);	
	fits_read_col(output, TINT, colnum, 1, 1, nrows, NULL, bstophi, NULL, &status);
	
	fits_get_colnum(output, 1, "BSTOPLO", &colnum, &status);	
	fits_read_col(output, TINT, colnum, 1, 1, nrows, NULL, bstoplo, NULL, &status);

	fits_get_colnum(output, 1, "OBTCORR", &colnum, &status);	
	fits_read_col(output, TSHORT, colnum, 1, 1, nrows, NULL, obt_corr, NULL, &status);	

	char str[100];

	int orbit = 0;
	char orbitcomment[FLEN_KEYWORD];
  	fits_read_key(output, TINT, "OBS_ID", &orbit, orbitcomment,  &status);	   
	
	for (i = 0; i < nrows; ++i) {
// 		int diffobtsecttEVT = 99791934;
// 		double diffobtmsectEVT = 816000.0;
// 		double microsec=MicSecCol*(1E-6)*(1000000.0/(1000000.0+ObtCor));


///////////////////obtcorr//////////////////////////////////////////
		bstartlo[i] = (int)(((double)bstartlo[i])*(1000000.0/(1000000.0+obt_corr[i])));
		if ( bstartlo[i] < 0) {
			bstartlo[i] = bstartlo[i] + 1000000;	
			bstarthi[i] = bstarthi[i] - 1;
			}

		bstoplo[i] = (int)(((double)bstoplo[i])*(1000000.0/(1000000.0+obt_corr[i])));
		if ( bstoplo[i] < 0) {
			bstoplo[i] = bstoplo[i] + 1000000;	
			bstophi[i] = bstophi[i] - 1;
			}
//////////////////////////////////////////////////////////////////////////
		if (orbit >= 43222 && orbit < 43315){		        
// 			secpktobt[i]=secpktobt[i]  +(1<<28)- diffobtsectt;
			bstarthi[i] = bstarthi[i] +(1<<28)- diffobtsecttEVT;
			bstophi[i]  = bstophi[i] +(1<<28)- diffobtsecttEVT;
			bstartlo[i] = bstartlo[i]- diffobtmsectEVT;
			bstoplo[i]  = bstoplo[i] - diffobtmsectEVT;			
			}

		else if (orbit >= 43315){
			bstarthi[i] = bstarthi[i] - diffobtsecttEVT2015;
			bstophi[i]  = bstophi[i] - diffobtsecttEVT2015;
			bstartlo[i] = bstartlo[i] - diffobtmsectEVT2015;
			bstoplo[i]  = bstoplo[i] - diffobtmsectEVT2015;				
			}
		else {	
			bstarthi[i] = bstarthi[i] - diffobtsecttEVT;
			bstophi[i]  = bstophi[i] - diffobtsecttEVT;
			bstartlo[i] = bstartlo[i] - (int)diffobtmsectEVT;
			bstoplo[i]  = bstoplo[i] - (int)diffobtmsectEVT;				
			}

		

		if ( bstophi[i] >= 1000000) {
			bstartlo[i] = bstophi[i] - 1000000;	
			bstarthi[i] = bstarthi[i] + 1;
			}
		if ( bstoplo[i] >= 1000000) {
			bstoplo[i] = bstoplo[i] - 1000000;	
			bstophi[i] = bstophi[i] + 1;
			}

		bstart[i] = bstarthi[i] + (((double)bstartlo[i])/1000000);

		bstop[i] = bstophi[i] + (((double)bstoplo[i])/1000000);

		}

		fits_insert_col(output, ncols+5, "BSTART", "1D", &status);
		fits_insert_col(output,  ncols+5, "BSTOP" , "1D", &status);


		fits_get_colnum(output, 1, "BSTART", &colnum, &status);
		fits_write_col(output, TDOUBLE, colnum, 1, 1, nrows,  bstart, &status);
	
		fits_get_colnum(output, 1, "BSTOP", &colnum, &status);
		fits_write_col(output, TDOUBLE, colnum, 1, 1, nrows,  bstop, &status);



///////////////////////////////////////////////////////////////////////////////////////////////			
//////////////////////////////PACKETS//////////////////////////////////////////////////////////	
///////////////////////////////////////////////////////////////////////////////////////////////



	fits_movabs_hdu(output, 3, NULL, &status);	
	fits_get_num_rows(output, &nrows, &status);
	fits_get_num_cols(output, &ncols, &status);	
	
	time = new double[nrows];
	microsecpktobt = new int[nrows];	
	secpktobt = new int[nrows];


/////////passo 1
	
	strcpy(str, "TT");
	fits_update_key(output, TSTRING, "TIMESYS",  str, NULL, &status);

/////////passo 2
	oldstatus = status;
	fits_delete_key(output, "MJDREF", &status);
	if (status == 202) status = oldstatus;
	
/////////passo 3
	fits_update_key(output, TDOUBLE, "MJDREFI",  &mjdrefi, NULL, &status);

/////////passo 4

	if(time[0] < 157852800.00000) {
	// 	cout<<"vecchio leap second pkt "<<time[0]<<endl;
		fits_update_key(output, TDOUBLE, "MJDREFF",  &mjdreff, NULL, &status);
		}
	else if((time[0] >= 157852800.00000) && (time[0] < 268185600.)){
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		fits_update_key(output, TDOUBLE, "MJDREFF",  &mjdreff_2009, NULL, &status);
		}
	else if((time[0] >= 268185600) && (time[0] < 362793600)){
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		fits_update_key(output, TDOUBLE, "MJDREFF",  &mjdreff_2012, NULL, &status);
		}		
	else {
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		fits_update_key(output, TDOUBLE, "MJDREFF",  &mjdreff_2015, NULL, &status);
		}			
	
/////////passo 5
	float timezero = 0;
	fits_update_key(output, TFLOAT, "TIMEZERO",  &timezero, str, &status);	

/////////passo 6 - 7 - 8
	fits_get_colnum(output, 1, "TIME", &colnum, &status);
	fits_read_col(output, TDOUBLE, colnum, 1, 1, nrows, NULL, time, NULL, &status);
	
	for (int k = 0; k < nrows; ++k) {
	
		time[k] = (time[k]/1000000);
		
		
		if (orbit >= 43222 && orbit < 43315){		        
			time[k] = time[k] +(1<<28) - diffobtsectt;			
			}		
		
		else if (orbit >= 43315){		        
			time[k] = time[k] - diffobtsectt2015;
			}
		else {	
			time[k] = time[k] - diffobtsectt;
			}	
		}

	fits_get_colnum(output, 1, "TIME", &colnum, &status);
	fits_write_col(output, TDOUBLE, colnum, 1, 1, nrows,  time, &status);

/////////passo 9
	sprintf(unit, "TUNIT%d", colnum);
	fits_update_key(output, TSTRING, unit, s, NULL, &status);

/////////passo 10 - 11
	WriteTime(output, time[0], time[nrows-1]);
	fits_movabs_hdu(output, 1, NULL, &status);	
	fits_update_key(output, TFLOAT, "TIMEZERO",  &timezero, str, &status);
	WriteTime(output, time[0], time[nrows-1]);	


	delete [] secpktobt;	
	delete [] time;	
	delete [] microsecpktobt;
		
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////	



	return status;
}




int euler(double ai, double bi, double * ao, double * bo, int select) {
	double twopi   =   2*PI;
	double fourpi  =   4*PI;
	double deg_to_rad = 360/twopi;
	
	double psi[6]   = { 0.57477043300, 4.9368292465, 0.00000000000, 0.0000000000, 0.11142137093, 4.71279419371};     
	double stheta[6] ={ 0.88998808748,-0.88998808748, 0.39777715593,-0.39777715593, 0.86766622025,-0.86766622025};   
	double ctheta[6] ={ 0.45598377618, 0.45598377618, 0.91748206207, 0.91748206207, 0.49714719172, 0.49714719172};   
	double phi[6]  = { 4.9368292465, 0.57477043300, 0.0000000000, 0.00000000000, 4.71279419371, 0.11142137093};

	int i  = select - 1;   
	double a  = ai/deg_to_rad - phi[i];
	double b = bi/deg_to_rad;
	double sb = sin(b);
	double cb = cos(b);
	double cbsa = cb * sin(a);
 	b  = -stheta[i] * cbsa + ctheta[i] * sb;
	*bo    = asin(b<1.0?b:1.0)*deg_to_rad;
 	a =  atan2( ctheta[i] * cbsa + stheta[i] * sb, cb * cos(a) );
	double x = fmod(a+psi[i]+fourpi,  twopi);
 	*ao = x*deg_to_rad;
	return 0;
	}


int min(double A[], int n){
	int i=0;
	double min = A[0];
	int jmin = 0;
	for (i=1; i<n; i++) {
		if (A[i] < min) {
			min = A[i];
			jmin = i;
			}
		}
	return(jmin);
	}

int max(double A[], int n){
	int i=0;
	double max = A[0];
	int jmax = 0;

	for (i=1; i<n; i++) {
		if (A[i] > max) {
			max = A[i];
			jmax = i;
			}
		}
	return(jmax);
	}

int minint(int A[], int n){
	int i=0;
	int min = A[0];
	int jmin = 0;
	for (i=1; i<n; i++) {
		if (A[i] < min) {
			min = A[i];
			jmin = i;
			}
		}
	return(jmin);
	}

int maxint(int A[], int n){
	int i=0;
	int max = A[0];
	int jmax = 0;

	for (i=1; i<n; i++) {
		if (A[i] > max) {
			max = A[i];
			jmax = i;
			}
		}
	return(jmax);
	}

int binarySearch(double * vect, int from, int to, double v, float diff) {
        if (from > to)  
		return -1;
	
        int mid = (from + to) / 2;
	if (v < 0)
		v = 0;
	double value = fabs(vect[mid]-v);

        if (value <= diff) {     // a[mid] == v
                return mid;
			}
        else {
		value = vect[mid]-v;
		if (value < 0)     // a[mid] < v
			return binarySearch(vect, mid + 1, to, v, diff);
		else
			return binarySearch(vect, from, mid - 1, v, diff);
		}
	return 0;
	}
	
void carxradec(float * Xp, float * dec, float * ra){


	double r = sqrt( Xp[0]*Xp[0] + Xp[1]*Xp[1] + Xp[2]*Xp[2]);
	
	*dec = 90 - acosd(Xp[2]/r);

	if ( *dec == 90 || *dec == -90){
		*ra  = 0;
		}
	else {
		*ra  = atan2d(Xp[1],Xp[0]);
		}
/*






	if (Xp[0]==0 && Xp[1]==0) {
		*ra=0;
		}	
	else if (Xp[1]==0) {
		if(Xp[0]<0)
			*ra = 180;
		else
			*ra = 0;
		}
	else if (Xp[0]==0 && Xp[1]!=0 ) {
		if(Xp[1]<0)
			*ra = 270;
		else
			*ra = 90;
		}
	else if (Xp[0] < 0){
		*ra = atand( Xp[1]/Xp[0]) + 180;
//		*ra = (90/(asin(1))) * atan( Xp[1]/Xp[0]) + 180;		
		}
	else if (Xp[0] > 0 && Xp[1] < 0){
		*ra = (90/(asin(1)))*atan(Xp[1]/Xp[0]) + 360;
		}
	else if (Xp[0] > 0 && Xp[1] > 0){
		*ra = (90/(asin(1)))*atan(Xp[1]/Xp[0]);
		}			
	*dec=(90/(asin(1)))*asin(Xp[2]/sqrt(Xp[0]*Xp[0]+Xp[1]*Xp[1]+Xp[2]*Xp[2]));*/
	}	
    
float * ProdQuat(float *Q_1, float *Q_2) {
	float * temp=new float[4];

	temp[3] = Q_1[3]*Q_2[3] -  Q_1[0]*Q_2[0] - Q_1[1]*Q_2[1] - Q_1[2]*Q_2[2];
	temp[0] = Q_1[0]*Q_2[3] +  Q_1[3]*Q_2[0] + Q_1[1]*Q_2[2] - Q_1[2]*Q_2[1];
	temp[1] = Q_1[3]*Q_2[1] -  Q_1[0]*Q_2[2] + Q_1[1]*Q_2[3] + Q_1[2]*Q_2[0];
	temp[2] = Q_1[3]*Q_2[2] +  Q_1[0]*Q_2[1] - Q_1[1]*Q_2[0] + Q_1[2]*Q_2[3];

	return temp;
	}

float * ConQuat(float *Q_1) {
	float * temp=new float[4];
	temp[0] = -Q_1[0];
	temp[1] = -Q_1[1];
	temp[2] = -Q_1[2];
	temp[3] =  Q_1[3];
	return temp;
	}


int WriteTime(fitsfile * input, double t1, double t2) {

	int status = 0;
	int hdnum = 0;
	fits_get_num_hdus(input, &hdnum, &status);
	
	int i;
	fits_get_hdu_num(input, &i);

	char str[100];
	strcpy(str, "TT");
	float timezero = 0.0;
	char * secstr = "s";
	
	fits_update_key(input, TDOUBLE, "TSTART", &t1,  "[OBT]first event time", &status);
	fits_update_key(input, TDOUBLE, "TSTOP", &t2,  "[OBT]last event time", &status);
	fits_update_key(input, TSTRING, "TIMESYS",  str, NULL, &status);
	fits_update_key(input, TSTRING, "TIMEUNIT",  secstr, NULL, &status);
	int oldstatus = status;
	
	double mjdreff_loc =  0;
	
/////////passo 4

	if(t1 < 157852800.00000) {
	// 	cout<<"vecchio leap second pkt "<<time[0]<<endl;
		mjdreff =  mjdreff;
		}
	else if((t1 >= 157852800.00000) && (t1 < 268185600.)){
	      // 	cout<<"nuovo leap second pkt "<<mjdreff_2009<<endl;
		mjdreff =  mjdreff_2009;		}
	else if((t1 >= 268185600) && (t1 < 362793600)){
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2012<<endl;
		mjdreff =  mjdreff_2012;
		}		
	else {
	// 	cout<<"nuovo leap second pkt "<<mjdreff_2015<<endl;
		mjdreff =  mjdreff_2015;
		}			

	
	
	fits_delete_key(input, "MJDREF", &status);
	if (status == 202) status = oldstatus;
	fits_update_key(input, TDOUBLE, "MJDREFI",  &mjdrefi, NULL, &status);
	fits_update_key(input, TDOUBLE, "MJDREFF",  &mjdreff, NULL, &status);
	fits_update_key(input, TFLOAT, "TIMEZERO",  &timezero, str, &status);			
		
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////	
	
	fits_update_key(input, TFLOAT, "TIMEZERO",  &timezero, str, &status);	

	time_t tt = 0;
	tm * tstr = gmtime(&tt);
	tstr->tm_year = 107;
	tstr->tm_mon = 0;
	tstr->tm_mday = 1;
	tstr->tm_hour = 0;
	tstr->tm_min = 0;
	tstr->tm_sec = 0;
	tstr->tm_isdst = 0;
	tt = timegm(tstr);
	double tinc = difftime(tt+1,tt);

	tt += (time_t)(((mjdrefi - 54101 + mjdreff) * 86400 + timezero + t1) / tinc) ;
	tstr = gmtime(&tt);
	double inttstart;
	double sec = tstr->tm_sec + modf(t1, &inttstart);
	char datestr[FLEN_KEYWORD];
	fits_time2str( tstr->tm_year + 1900, tstr->tm_mon + 1, tstr->tm_mday, tstr->tm_hour, tstr->tm_min, sec, 0, datestr, &status);
	fits_update_key(input, TSTRING, "DATE-OBS",  datestr, "start date and time of the observation(TT)", &status);
	
	tt += (time_t)((t2 - inttstart) / tinc);
	tstr = gmtime(&tt);
	sec = tstr->tm_sec + modf(t2, &inttstart); 
	char datestr1[FLEN_KEYWORD];	
	fits_time2str( tstr->tm_year + 1900, tstr->tm_mon + 1, tstr->tm_mday, tstr->tm_hour, tstr->tm_min, sec, 0, datestr1, &status);
	fits_update_key(input, TSTRING, "DATE-END",  datestr1, "end date and time of the observation(TT)", &status);	

	if (i == hdnum) {
		fits_movabs_hdu(input, 1, NULL, &status);		
		fits_update_key(input, TDOUBLE, "TSTART", &t1,  "[OBT]first event time", &status);
		fits_update_key(input, TDOUBLE, "TSTOP", &t2,  "[OBT]last event time", &status);
		fits_update_key(input, TSTRING, "TIMESYS",  str, NULL, &status);
		fits_update_key(input, TSTRING, "TIMEUNIT",  secstr, NULL, &status);
		oldstatus = status;
		fits_delete_key(input, "MJDREF", &status);
		if (status == 202) status = oldstatus;
		fits_update_key(input, TDOUBLE, "MJDREFI",  &mjdrefi, NULL, &status);
		fits_update_key(input, TDOUBLE, "MJDREFF",  &mjdreff, NULL, &status);
		fits_update_key(input, TFLOAT, "TIMEZERO",  &timezero, str, &status);	
		fits_update_key(input, TSTRING, "DATE-OBS",  datestr, "start date and time of the observation(TT)", &status);
		fits_update_key(input, TSTRING, "DATE-END",  datestr1, "end date and time of the observation(TT)", &status);	
		fits_movabs_hdu(input, i, NULL, &status);		
		}
	return status;	
	}
	
int addfile(fitsfile *iFile, char * fileList, char * expr, double tmin, double tmax){
	char buffer[1024];
	int status = 0;
	FILE *fp = fopen(fileList, "r");
	if(!fp) {
		printf("Errore in apertura file '%s'\n", fileList);
		return 104;	
		}
		
	fitsfile *tempFits;	
	int hdutype = 0;
	int  find = 0;
	char *name=new char[FLEN_FILENAME];

	double t1 = 0, t2 = 0;


			
	while (fgets(buffer , 40960, fp)) {
		sscanf(buffer, "%s %lf %lf", name, &t1, &t2);
		
		if ( (tmin <= t1 &&  tmax >= t1) || (tmin <= t2 && tmin >= t1)) {

			
			if (find == 0) {
				if ( fits_open_file(&tempFits, name, READONLY, &status) != 0 ) {
					printf("Errore in apertura file '%s'\n", name);
					return status;
					}	
				fits_copy_file(tempFits, iFile, 1, 1, 1, &status);
				fits_select_rows(iFile, iFile, expr, &status);
				fits_close_file(tempFits, &status);							
				find++;		
				}
			else {
				if ( fits_open_file(&tempFits, name, READONLY, &status) != 0 ) {
					printf("Errore in apertura file '%s'\n", name);
					return status;
					}
				fits_movabs_hdu(tempFits, 2, &hdutype, &status);					
				fits_select_rows(tempFits, iFile, expr, &status);					
				fits_close_file(tempFits, &status);			
				find++;									
				}
			}
		}
	if (find == 0) {
		return 1005;
		}	
	fclose(fp);
	return status;
	}
	
	
	
int addfile3914(fitsfile *iFile, char * fileList, char * expr, double tmin, double tmax, char * pkt){
	char buffer[1024];
	int status = 0;
	FILE *fp = fopen(fileList, "r");
	if(!fp) {
		printf("Errore in apertura file '%s'\n", fileList);
		return 104;	
		}
		
	fitsfile *tempFits;	
	int hdutype = 0;
	int  find = 0;
	char *name=new char[FLEN_FILENAME];
	char *type=new char[FLEN_FILENAME];
	double t1,t2;

	while (fgets(buffer , 40960, fp)) {
		sscanf(buffer, "%s %lf %lf %s", name, &t1, &t2, type);

		if ( ((tmin <= t1 &&  tmax >= t1) || (tmin <= t2 && tmin >= t1)) && (strcmp(type,"3914")==0)) {
		
			if (find == 0) {
				if ( fits_open_file(&tempFits, name, READONLY, &status) != 0 ) {
					printf("Errore in apertura file '%s'\n", name);
					return status;
					}
				fits_movabs_hdu(tempFits, 3, &hdutype, &status);	
				fits_copy_file(tempFits, iFile, 0, 1, 0, &status);
				fits_select_rows(iFile, iFile, expr, &status);
				fits_close_file(tempFits, &status);							
				find++;		
				}
			else {
				if ( fits_open_file(&tempFits, name, READONLY, &status) != 0 ) {
					printf("Errore in apertura file '%s'\n", name);
					return status;
					}
				fits_movabs_hdu(tempFits, 3, &hdutype, &status);					
				fits_select_rows(tempFits, iFile, expr, &status);					
				fits_close_file(tempFits, &status);			
				find++;									
				}
			}
		}
	delete [] name;
	delete [] type;
	if (find == 0) {
		return 1005;
		}	
	fclose(fp);
	return status;
	}
	
int addfile3201(fitsfile *iFile, char * fileList, char * expr, double tmin, double tmax, char * pkt){
	char buffer[1024];
	int status = 0;
	FILE *fp = fopen(fileList, "r");
	if(!fp) {
		printf("Errore in apertura file '%s'\n", fileList);
		return 104;	
		}
		
	fitsfile *tempFits;	
	int hdutype = 0;
	int  find = 0;
	char *name=new char[FLEN_FILENAME];
	char *type=new char[FLEN_FILENAME];
	double t1,t2;

	while (fgets(buffer , 40960, fp)) {
		sscanf(buffer, "%s %lf %lf %s", name, &t1, &t2, type);

		if ( ((tmin <= t1 &&  tmax >= t1) || (tmin <= t2 && tmin >= t1)) && (strcmp(type,"3201")==0)) {
		
			if (find == 0) {
				if ( fits_open_file(&tempFits, name, READONLY, &status) != 0 ) {
					printf("Errore in apertura file '%s'\n", name);
					return status;
					}
				fits_movabs_hdu(tempFits, 2, &hdutype, &status);	
				fits_copy_file(tempFits, iFile, 0, 1, 0, &status);
				fits_select_rows(iFile, iFile, expr, &status);
				fits_close_file(tempFits, &status);							
				find++;		
				}
			else {
				if ( fits_open_file(&tempFits, name, READONLY, &status) != 0 ) {
					printf("Errore in apertura file '%s'\n", name);
					return status;
					}
				fits_movabs_hdu(tempFits, 2, &hdutype, &status);					
				fits_select_rows(tempFits, iFile, expr, &status);					
				fits_close_file(tempFits, &status);			
				find++;									
				}
			}
		}
	delete [] name;
	delete [] type;
	if (find == 0) {
		return 1005;
		}	
	fclose(fp);
	return status;
	}
	
int addfile3916(fitsfile *iFile, char * fileList, char * expr, double tmin, double tmax, char * pkt){
	char buffer[1024];
	int status = 0;
	FILE *fp = fopen(fileList, "r");
	if(!fp) {
		printf("Errore in apertura file '%s'\n", fileList);
		return 104;	
		}
		
	fitsfile *tempFits;	
	int hdutype = 0;
	int  find = 0;
	char *name=new char[FLEN_FILENAME];
	char *type=new char[FLEN_FILENAME];
	double t1,t2;

	while (fgets(buffer , 40960, fp)) {
		sscanf(buffer, "%s %lf %lf %s", name, &t1, &t2, type);

		if ( ((tmin <= t1 &&  tmax >= t1) || (tmin <= t2 && tmin >= t1)) && (strcmp(type,"3916")==0)) {
		
			if (find == 0) {
				if ( fits_open_file(&tempFits, name, READONLY, &status) != 0 ) {
					printf("Errore in apertura file '%s'\n", name);
					return status;
					}
				fits_movabs_hdu(tempFits, 3, &hdutype, &status);	
				fits_copy_file(tempFits, iFile, 0, 1, 0, &status);
				fits_select_rows(iFile, iFile, expr, &status);
				fits_close_file(tempFits, &status);							
				find++;		
				}
			else {
				if ( fits_open_file(&tempFits, name, READONLY, &status) != 0 ) {
					printf("Errore in apertura file '%s'\n", name);
					return status;
					}
				fits_movabs_hdu(tempFits, 3, &hdutype, &status);					
				fits_select_rows(tempFits, iFile, expr, &status);					
				fits_close_file(tempFits, &status);			
				find++;									
				}
			}
		}
	delete [] name;
	delete [] type;
	if (find == 0) {
		return 1005;
		}	
	fclose(fp);
	return status;
	}
	
	
	
int addfileFLG(fitsfile *iFile, char * fileList, char * expr, char * pkt){
	char buffer[1024];
	int status = 0;
	FILE *fp = fopen(fileList, "r");
	if(!fp) {
		printf("COULD NOT OPEN THE NAMED FILE:  %s\n", fileList);
		return 1003;
		}
		
	fitsfile *tempFits;	
	int hdutype = 0;
	int  find = 0;
	char *name=new char[FLEN_FILENAME];
	char *type=new char[FLEN_FILENAME];
	double t1,t2;
	int  pkttype = 0;
	int  pktstype = 0;	

	while (fgets(buffer , 40960, fp)) {
		sscanf(buffer, "%s %lf %lf %s", name, &t1, &t2, type);

		if ( strcmp(type,"FLG")==0) {
//			printf("..........opening input file: %s\n",name);	
			if (find == 0) {
				if ( fits_open_file(&tempFits, name, READONLY, &status) != 0 ) {
					printf("Errore in apertura file '%s'\n", name);
					return status;
					}
				
					
				fits_read_key(tempFits, TINT, "PKTTYPE", &pkttype, NULL, &status );
				fits_read_key(tempFits, TINT, "PKTSTYPE", &pktstype, NULL, &status );
				if (pkttype!=39 || ((pktstype!=1) && (pktstype!=2))) {
					printf("error packet type [39]   %d!!\n",pkttype );
					printf("error packet subtype [1,2] %d!!\n",pktstype );
					fits_close_file(tempFits, &status);
					return 1001;
					}					
					
						
				fits_copy_file(tempFits, iFile, 1, 1, 1, &status);
				fits_select_rows(iFile, iFile, expr, &status);
				fits_close_file(tempFits, &status);							
				find++;		
				}
			else {
				if ( fits_open_file(&tempFits, name, READONLY, &status) != 0 ) {
					printf("Errore in apertura file '%s'\n", name);
					return status;
					}
				fits_read_key(tempFits, TINT, "PKTTYPE", &pkttype, NULL, &status );
				fits_read_key(tempFits, TINT, "PKTSTYPE", &pktstype, NULL, &status );
				if (pkttype!=39 || pktstype!=1) {
					printf("error packet type [39]   %d!!\n",pkttype );
					printf("error packet subtype [1] %d!!\n",pktstype );
					fits_close_file(tempFits, &status);
					return 1001;
					}					
				fits_movabs_hdu(tempFits, 2, &hdutype, &status);					
				fits_select_rows(tempFits, iFile, expr, &status);					
				fits_close_file(tempFits, &status);			
				find++;									
				}
			}
		}
	delete [] name;
	delete [] type;
	if (find == 0) {
		return 1005;
		}	
	fclose(fp);
	return status;
	}
	
	
int orderFileIndex(char * fileList){
	char buffer[4096];
	int status = 0;
	FILE *fp = fopen(fileList, "r");
	if(!fp) {
		printf("COULD NOT OPEN THE NAMED FILE:  %s\n", fileList);
		return 1003;
		}

	double t1 = 0, t2 = 0;

	int ccounter = 0;	
	while (fgets(buffer , 4096, fp)) {
		ccounter++;	
		}
	fseek(fp,0,SEEK_SET);		
	lineLineStruct A[ccounter];
	int i = 0;
	
	while (fgets(buffer , 4096, fp)) {
		A[i].name = new char[FLEN_FILENAME];
		A[i].type = new char[8];		
		sscanf(buffer, "%s %lf %lf %s", A[i].name, &t1, &t2, A[i].type);
		A[i].tempo1 = t1;
		A[i].tempo2 = t2;
		i++;	
		}
			
	int n =1;
	int p =0;
	while (p < i-1)	{
		n = p +1;
		
		while (n < i) {
			if ((A[n].tempo1 < A[p].tempo1)) {
							
				lineLineStruct temp;
				temp.name = new char[FLEN_FILENAME];
				temp.type = new char[8];					
				strcpy(temp.name,A[p].name);
				temp.tempo1 = A[p].tempo1;
				temp.tempo2 = A[p].tempo2;
							
				strcpy(temp.type,A[p].type);	
							
				strcpy(A[p].name,A[n].name);
				A[p].tempo1 = A[n].tempo1;
				A[p].tempo2 = A[n].tempo2;				
				strcpy(A[p].type,A[n].type);
				
				strcpy(A[n].name,temp.name);
				A[n].tempo1 = temp.tempo1;
				A[n].tempo2 = temp.tempo2;
				strcpy(A[n].type,temp.type);
				
				delete [] temp.name;
				delete [] temp.type;	
											
				}
			n++;
			}
		p++;		
		}

	fclose(fp);
	remove(fileList);
	
	fp = fopen(fileList, "w");
	if(!fp) {
		printf("COULD NOT OPEN THE NAMED FILE:  %s\n", fileList);
		return 1003;
		}
	n = 0;	
	while (n < i) {
		fprintf(fp, "%s %f %f %s\n", A[n].name,  A[n].tempo1 ,A[n].tempo2, A[n].type);
		delete [] A[n].name;
		delete [] A[n].type;			
		n++;	
		}	
	
	fclose(fp);		
	return status;
	}
	
	
	
int checkfile(char * fileList, char * filename, double tmin, double tmax, char * packet){
	char *buffer = new char[4096];	
	char *name=new char[FLEN_FILENAME];
	char *type=new char[10];	
	float t1 = 0, t2 = 0;
		
	FILE *fp = fopen(fileList, "r");
	if(!fp) {
		return 0;	
		}
	
	while (fgets(buffer , 4096, fp)) {
		sscanf(buffer, "%s %f %f %s", name, &t1, &t2, type);
		if ( strcmp(name,filename)==0 || (t1 < tmin && t2 > tmax && strcmp(packet,type)==0)) {
			delete [] buffer;
			delete [] name;	
			delete [] type;					
			return 1;					
			}
		}	
	fclose(fp);
	
	delete [] buffer;
	delete [] name;	
	delete [] type;		
	return 0;
	}	
	
void getError(int status) {
	char error_array[100][200];
	sprintf(error_array[0], "FILE TYPE WRONG FOR THIS TASK");
	sprintf(error_array[1], "ERROR IN PARAMETER FILE");
	sprintf(error_array[2], ".");	
	sprintf(error_array[3], "WRONG INDEX FILE..list corrected is: filename time_init time_stop type_packet");	
	sprintf(error_array[4], "FILE(S) NOT FOUND BETWEEN TMIN AND TMAX");		
	sprintf(error_array[5], "INPUT FILE SHOULD BE A LIST");	
	sprintf(error_array[6], "MAXIMUM TIME BEFORE MINIMUM TIME");
	sprintf(error_array[7], "NOT MATCHING TMIN AND TMAX");
	printf("%s.....STATUS: %d\n",error_array[status-1001], status);
	}


void fpgetError(FILE * fp, int status)  {
	char error_array[100][200];
	sprintf(error_array[0], "FILE TYPE WRONG FOR THIS TASK");
	sprintf(error_array[1], "ERROR IN PARAMETER FILE");
	sprintf(error_array[2], ".");	
	sprintf(error_array[3], "WRONG INDEX FILE..list corrected is: filename time_init time_stop type_packet");	
	sprintf(error_array[4], "FILE(S) NOT FOUND BETWEEN TMIN AND TMAX");		
	sprintf(error_array[5], "INPUT FILE SHOULD BE A LIST");	
	sprintf(error_array[6], "MAXIMUM TIME BEFORE MINIMUM TIME");
	sprintf(error_array[7], "NOT MATCHING TMIN AND TMAX");			
	fprintf(fp, "%s.....STATUS: %d\n",error_array[status-1001], status);
	}




int isfile (const char *filename)
{
  /* Returns a true value if filename exists as a file, or false if not. */

  FILE *stream;

  /* Open the file */
// cout<<"filename = "<<filename<<endl;
  stream = fopen (filename, "r");
  if (stream == NULL)
    {
//     cout<<"filename non c'è"<<endl;
      /* Failed */
      return (0);
    }
    else {
//      cout<<"filename c'è"<<endl;   
    }

  /* Close the file, and return success */

  fclose (stream);
  return (1);
}
 
	
	
char * filetemp(char * name){  

	time_t seconds;
	seconds = time(&seconds);
	srand(seconds);
 	int first = rand() % 32767;
	int second = rand() % 32767;
 	int third = rand() % 32767;		
	sprintf(name,"/tmp/filetemp%i_%i_%i",first,second,third);
	int c = 1;
	while (isfile(name) == 1){
		srand(c);
		first = rand() % 32767;
		second = rand() % 32767;
		third = rand() % 32767;		
		sprintf(name,"/tmp/filetemp%i_%i_%i",first,second,third); 
		c++;
		}
			
// 	cout<<name<<endl;
	return name;
}
	











