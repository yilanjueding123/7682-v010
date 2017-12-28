#include "drv_l1_i2c.h"
#include "drv_l2_sensor.h"
#include "drv_l1_cdsp.h"
#include "gp_aeawb.h"
//#include "cdsp_cfg.h"

#include "sensor_gc1004_iqj.h"


//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#if (USE_SENSOR_NAME == SENSOR_GC1004)
//================================================================//
//Set Gain



static sensor_exposure_t gc1004_seInfo;
static sensor_calibration_t gc1004_cdsp_calibration;
//static INT32U gc1004_digital_gain = 0x180; //SENSOR_GC1004
static INT32U gc1004_analog_gain = 0x100;
static int *p_expTime_table;
//static int sensor_max_ev_idx;
/**************************************************************************
 *                         SENSEOR FUNCTION                          *
 **************************************************************************/
	static int	pre_sensor_a_gain, pre_sensor_time;

sensor_exposure_t *gc1004_get_senInfo(void)
{
	return &gc1004_seInfo;
}

void gc1004_sensor_calibration_str(void)
{
	#if SENSOR_FLIP
    	gc1004_cdsp_calibration.r_b_gain = g_GC1004_r_b_gain_flip;
	#else
    	gc1004_cdsp_calibration.r_b_gain = g_GC1004_r_b_gain;
	#endif	

}

sensor_calibration_t *gc1004_get_calibration(void)
{
	return &gc1004_cdsp_calibration;
}

void gc1004_sensor_calibration(void)
{
	//OB
	gp_Cdsp_SetBadPixOb((INT16U *)g_GC1004_badpix_ob_table);
	//Lenscmp
	
#if SENSOR_FLIP
	hwIsp_luc_MaxTan8_Slop_CLP((INT16U *)g_GC1004_MaxTan8_flip ,(INT16U *)g_GC1004_Slope4_flip ,(INT16U *)g_GC1004_CLPoint_flip);
	hwIsp_RadusF0((INT16U *)g_GC1004_Radius_File_0_flip);
	hwIsp_RadusF1((INT16U *)g_GC1004_Radius_File_1_flip);	
	
	hwCdsp_InitGamma((INT32U *)g_GC1004_gamma_045_table);
	//Color Correction
	hwCdsp_SetColorMatrix_Str((INT16S *)g_GC1004_color_matrix4gamma045_flip);
	//AWB
	gp_Cdsp_SetAWBYUV((INT16S *)g_GC1004_awb_thr_flip);
#else
	hwIsp_luc_MaxTan8_Slop_CLP((INT16U *)g_GC1004_MaxTan8 ,(INT16U *)g_GC1004_Slope4 ,(INT16U *)g_GC1004_CLPoint);
	hwIsp_RadusF0((INT16U *)g_GC1004_Radius_File_0);
	hwIsp_RadusF1((INT16U *)g_GC1004_Radius_File_1);
		
	//Gamma
	hwCdsp_InitGamma((INT32U *)g_GC1004_gamma_045_table);
	//Color Correction
	hwCdsp_SetColorMatrix_Str((INT16S *)g_GC1004_color_matrix4gamma045);
	//AWB
	gp_Cdsp_SetAWBYUV((INT16S *)g_GC1004_awb_thr);
#endif		
}


static gpCdspWBGain_t gc1004_wbgain;

gpCdspWBGain_t *gc1004_awb_r_b_gain_boundary(void)
{

	int i;
	int max_r_gain, max_b_gain, min_r_gain, min_b_gain;
	
	max_r_gain = max_b_gain = 0;
	min_r_gain = min_b_gain = 255;
	
#if SENSOR_FLIP
	for(i = 10 ; i < 55 ; i++)
	{
		if(max_r_gain < g_GC1004_r_b_gain_flip[i][0]) max_r_gain = g_GC1004_r_b_gain_flip[i][0];
		if(max_b_gain < g_GC1004_r_b_gain_flip[i][1]) max_b_gain = g_GC1004_r_b_gain_flip[i][1];
		if(min_r_gain > g_GC1004_r_b_gain_flip[i][0]) min_r_gain = g_GC1004_r_b_gain_flip[i][0];
		if(min_b_gain > g_GC1004_r_b_gain_flip[i][1]) min_b_gain = g_GC1004_r_b_gain_flip[i][1];
	}
#else
	for(i = 10 ; i < 55 ; i++)
	{
		if(max_r_gain < g_GC1004_r_b_gain[i][0]) max_r_gain = g_GC1004_r_b_gain[i][0];
		if(max_b_gain < g_GC1004_r_b_gain[i][1]) max_b_gain = g_GC1004_r_b_gain[i][1];
		if(min_r_gain > g_GC1004_r_b_gain[i][0]) min_r_gain = g_GC1004_r_b_gain[i][0];
		if(min_b_gain > g_GC1004_r_b_gain[i][1]) min_b_gain = g_GC1004_r_b_gain[i][1];
	}
#endif
	gc1004_wbgain.max_rgain = max_r_gain;
	gc1004_wbgain.max_bgain = max_b_gain;
	gc1004_wbgain.min_rgain = min_r_gain;
	gc1004_wbgain.min_bgain = min_b_gain;
	
	return &gc1004_wbgain;
		
}


int gc1004_get_night_ev_idx(void)
{
	return gc1004_seInfo.night_ev_idx;
}

int gc1004_get_max_ev_idx(void)
{
	return gc1004_seInfo.max_ev_idx;
}




/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
static void gc1004_cvt_analog_gain(INT32U analog_gain)
{
 
	INT32U Analog_Multiple[11]={1000, 1420, 1990, 2850, 4030,5770,8060,11530,16120,23300,32750}; 

	INT32U Analog_Index;
	INT32U Digital_Gain;
	INT32U Decimal;

	gc1004_analog_gain = analog_gain*10;	
	
	Analog_Index=0;
	
	while(Analog_Index<11)
	{
	  if(gc1004_analog_gain<Analog_Multiple[Analog_Index]) 
	  {
		break;
	  }
	  else
	  {
		Analog_Index++; 
	  }
	}

	
	Digital_Gain = gc1004_analog_gain*1000/Analog_Multiple[Analog_Index-1];
	
	Decimal=(Digital_Gain*64)/1000;

    sccb_write(GC1004_SLAVE_ID, 0xfe,   0x01); 
	sccb_write(GC1004_SLAVE_ID, 0xb1,  Decimal>>6); 
	sccb_write(GC1004_SLAVE_ID, 0xb2,  (Decimal<<2)&0xfc);
	sccb_write(GC1004_SLAVE_ID, 0xb6,   Analog_Index-1);
	sccb_write(GC1004_SLAVE_ID, 0xfe,	0x00); 

}





int gc1004_set_exposure_time(sensor_exposure_t *si)
{
	//int ret=0;
	//unsigned short tmp;
	//int analog_gain;
	//unsigned char cvt_digital_gain;
	//int digital_gain;
	int lsb_time, msb_time;
	int idx;

	// From agoritham calc new data update to gc1004_seInfo.
	gc1004_seInfo.sensor_ev_idx += si->ae_ev_idx;
	if(gc1004_seInfo.sensor_ev_idx >= gc1004_seInfo.max_ev_idx) gc1004_seInfo.sensor_ev_idx = gc1004_seInfo.max_ev_idx;
	if(gc1004_seInfo.sensor_ev_idx < 0) gc1004_seInfo.sensor_ev_idx = 0;
	
	idx = gc1004_seInfo.sensor_ev_idx * 3;
	gc1004_seInfo.time = p_expTime_table[idx];
	gc1004_seInfo.analog_gain = p_expTime_table[idx+1];
	gc1004_seInfo.digital_gain = p_expTime_table[idx+2];
	
	gc1004_seInfo.userISO = si->userISO;

	//DBG_PRINT("T %d, ag %d, ev %d.\r\n", gc1004_seInfo.time, gc1004_seInfo.analog_gain, gc1004_seInfo.sensor_ev_idx );
	//DBG_PRINT("Time = %d, a gain = %d, d gain = %d, ev idx = %d [%d]\r\n", gc1004_seInfo.time, gc1004_seInfo.analog_gain, gc1004_seInfo.digital_gain, gc1004_seInfo.sensor_ev_idx, si->ae_ev_idx );
	//digital_gain = ((si->digital_gain >> 2) & 0xFF);	//0x40:1x, 0xff:4x
	// set exposure time
	if(gc1004_seInfo.time != pre_sensor_time)
	{
	
	pre_sensor_time =gc1004_seInfo.time;
		lsb_time = (gc1004_seInfo.time & 0xFF);
		msb_time = ((gc1004_seInfo.time >>8 )& 0xFF);

		#if 1
			sccb_write(GC1004_SLAVE_ID, 0x04 , lsb_time );//lsb_time
			sccb_write(GC1004_SLAVE_ID, 0x03 , msb_time );//msb_time
		#else
			sccb_write(GC1004_SLAVE_ID, 0x04 , 0xE2 );
			sccb_write(GC1004_SLAVE_ID, 0x03 , 0x00 );
		#endif
		}

	return 0;
}



void gc1004_set_exposure_gain(void)
{
	//int digital_gain_tmp;
	/*digital_gain_tmp =  0x60;((gc1004_seInfo.digital_gain  >> 2) & 0xFF);	//0x40:1x, 0xff:4x
	sccb_write(GC1004_SLAVE_ID, 0xb0, digital_gain_tmp);*/
	if(gc1004_seInfo.analog_gain != pre_sensor_a_gain)
	{
		// gain
		pre_sensor_a_gain = gc1004_seInfo.analog_gain;
		gc1004_cvt_analog_gain(gc1004_seInfo.analog_gain);	
	}
	//DBG_PRINT("G");
}


void gc1004_get_exposure_time(sensor_exposure_t *se)
{
	//int ret=0;
	gp_memcpy((INT8S *)se, (INT8S *)&gc1004_seInfo, sizeof(sensor_exposure_t));

}

void gc1004_set_exp_freq(int freq)
{
	if(freq == 50)
	{
		gc1004_seInfo.sensor_ev_idx = GC1004_50HZ_INIT_EV_IDX;
		gc1004_seInfo.ae_ev_idx = 0;
		gc1004_seInfo.daylight_ev_idx= GC1004_50HZ_DAY_EV_IDX;
		gc1004_seInfo.night_ev_idx= GC1004_50HZ_NIGHT_EV_IDX;
		
		gc1004_seInfo.max_ev_idx = GC1004_50HZ_MAX_EXP_IDX - 1;
		p_expTime_table = (int *)g_GC1004_exp_time_gain_50Hz;
	}
	else if(freq == 60)
	{
		gc1004_seInfo.sensor_ev_idx = GC1004_60HZ_INIT_EV_IDX;
		gc1004_seInfo.ae_ev_idx = 0;
		gc1004_seInfo.daylight_ev_idx= GC1004_60HZ_DAY_EV_IDX;
		gc1004_seInfo.night_ev_idx= GC1004_60HZ_NIGHT_EV_IDX;
		gc1004_seInfo.max_ev_idx = GC1004_60HZ_MAX_EXP_IDX - 1;
		p_expTime_table = (int *)g_GC1004_exp_time_gain_60Hz;
	}
}

static int gc1004_init(void)
{
	gc1004_seInfo.max_time = GC1004_MAX_EXPOSURE_TIME;
	gc1004_seInfo.min_time = GC1004_MIN_EXPOSURE_TIME;

	gc1004_seInfo.max_digital_gain = GC1004_MAX_DIGITAL_GAIN ;
	gc1004_seInfo.min_digital_gain = GC1004_MIN_DIGITAL_GAIN ;

	gc1004_seInfo.max_analog_gain = GC1004_MAX_ANALOG_GAIN;
	gc1004_seInfo.min_analog_gain = GC1004_MIN_ANALOG_GAIN;

	gc1004_seInfo.analog_gain = gc1004_seInfo.min_analog_gain;
	gc1004_seInfo.digital_gain = gc1004_seInfo.min_digital_gain;
	gc1004_seInfo.time = gc1004_seInfo.max_time;// >> 1;
	gc1004_seInfo.userISO = ISO_AUTO;

	gc1004_set_exp_freq(50);
	pre_sensor_time = 1;
	pre_sensor_a_gain = 0x00;
	
	DBG_PRINT("gc1004_init\r\n");
	return 0;
}

void sensor_gc1004_init(INT32U WIDTH, INT32U HEIGHT)
{
	//i2c_bus_handle_t i2c_handle; 
	INT32U i;
	INT8U reg_tmp, data_tmp;
	/*
	  °Ñ¼Æ³]­È
	*/
	//i2c_handle.slaveAddr = 0x60;
	//i2c_handle.clkRate = 100;
	
	gc1004_init();
	gc1004_sensor_calibration_str();
	DBG_PRINT("init_1034\r\n");
	if (0)
		{
		 INT32U t;
		INT32U t1;
		
		t = sccb_read(GC1004_SLAVE_ID, 0xF0);
		t1 = sccb_read(GC1004_SLAVE_ID, 0xF1);
		DBG_PRINT("%x%x\r\n",t,t1);
			if ((t==0x10)&&(t1==0x24))
		{
			DBG_PRINT("1064\r\n");
		}
		else
		{
			sccb_write(GC1004_SLAVE_ID, 0xFE, 0x00);
			sccb_write(GC1004_SLAVE_ID, 0xF9, 0x0F);
			t =sccb_read(GC1004_SLAVE_ID, 0x26);
			//DBG_PRINT("Sencer_id=%d\r\n",t);
			if (t == 0x00)
			{
				DBG_PRINT("1004\r\n");
			}
			else
			{
				DBG_PRINT("1024\r\n");
			}
		}
	
		}
     if(sensor_format == GC1004_MIPI){
	   if	(WIDTH == 1280 && HEIGHT == 720)
		{
	
				for (i=0; i<sizeof(GC1004_MIPI_720P)/2; i++) 
				{
					reg_tmp = GC1004_MIPI_720P[i][0];
					data_tmp =  GC1004_MIPI_720P[i][1];
					sccb_write(GC1004_SLAVE_ID,GC1004_MIPI_720P[i][0], GC1004_MIPI_720P[i][1]);
					
					#if 1
					if (reg_tmp == 0x10 && data_tmp == 0x80)
					{
						 drv_msec_wait(200);//90); //wait 1 is 1ms for CLKO stable
					}
					#endif				
				}    		
		}
     
		
		else 
		{
			while(1);
		}
	}
}

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#endif //(USE_SENSOR_NAME == SENSOR_GC1004)     //
//================================================================//
