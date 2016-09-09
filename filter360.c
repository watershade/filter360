#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

const uint8_t FilterAngle_Depth = 20;
const float share = 0.0;
// const uint32_t FilterSpeed_Depth = 5;
static int32_t filterAngleQueue[20];
// static int32_t filterSpeedQueue[FilterSpeed_Depth];
static float FQ_AverageAngle = 0;
// static int32_t FQ_AverageSpeed = 0;
static uint32_t FQ_P2NewAngle = 0;
// static int32_t FQ_P2NewSpeed = 0;
static uint8_t isQueueFilled = 0;
/* Functions Prototype -------------------------------------------------------*/
void Fill_AngleQueue(int32_t _val);
// void Fill_SpeedQueue(int32_t _val);
int32_t AddNewAngle2Queue(int32_t _val);
// int32_t AddNewSpeed2Queue(int32_t _val);



/* Functions -----------------------------------------------------------------*/
  /*fill a initial angle*/
  void Fill_AngleQueue(int32_t _val)
  {
    if(isQueueFilled == 0){
      isQueueFilled = 1;
      FQ_P2NewAngle = 0;
      FQ_AverageAngle = _val;
      for(uint8_t cir = 0;cir<FilterAngle_Depth;cir++){
        filterAngleQueue[cir] = _val;
      }
    }
    
  }


/*add a new angle to queue, and get a average angle*/
int32_t AddNewAngle2Queue(int32_t _val)
{
  int32_t temp = 0 ;
  int32_t lastAngle = filterAngleQueue[FQ_P2NewAngle];
  filterAngleQueue[FQ_P2NewAngle] = _val;
  FQ_P2NewAngle++;
  FQ_P2NewAngle = (FQ_P2NewAngle<FilterAngle_Depth)?FQ_P2NewAngle:0;
  FQ_AverageAngle = FQ_AverageAngle + (float)(_val - lastAngle)/20;
  temp = (int32_t)(share*_val + (1-share)*FQ_AverageAngle);
  return temp;
}

uint8_t getGlobalValue(uint32_t _raw,int32_t* p2result)
{
  int32_t gval;
  static uint8_t zone = 0;
  static uint8_t lastzone = 0;
  static uint8_t smode = 0;
  static int8_t base = 0;
  /*step 1: regions divide */
  if(_raw<1000) zone = 1;
  else if(_raw <18000) zone = 2;
  else if(_raw <=35000) zone = 3;
  else zone = 4;
  /*step 2: get special mode */
  if( (zone == 1)&&(lastzone == 2) ) smode = 1;
  else if( (zone ==4)&&(lastzone ==3) ) smode =2;

  /*step 3: get base level*/
  /*>start state<*/
  if(smode == 0){
    if((_raw >= 1000)&&(_raw<35000)) smode = 3; 
    /*in this zone don't filter*/
    gval = _raw;
  }
  else if(smode ==1){
    if( (zone==2)&&(lastzone==1) ){
      smode = 3;
    }
    else if( (zone==3)&&(lastzone==4) ){
      base -= 1;
      smode = 3;
    }

    /*step 4: get value*/
    if(_raw<1000) 
        gval = _raw + 36000*base;
    else if(_raw>35000) 
        gval = _raw + 36000*(base-1);
    else gval =  _raw + 36000*base;

  }
  else if(smode ==2){
    if( (zone==2)&&(lastzone==1) ){
      base += 1;
      smode = 3;
    }
    else if( (zone==3)&&(lastzone==4) ){      
      smode = 3;
    }

    /*step 4: get value*/
    if(_raw<1000) 
        gval = _raw + 36000*(base+1);
    else if(_raw>35000) 
        gval = _raw + 36000*base;
    else gval =  _raw + 36000*base;
  }
  else{
    /*step 4: get value*/
    gval = _raw + 36000*base;
  }

  /*step 5: prepare value*/
  lastzone = zone;
  (*p2result) = gval;

  return smode;  
}

int main(){
  static int k = 1;
  int bias = 0;
  uint32_t val = 0;
  uint32_t lastval = 0;
  isQueueFilled = 0;
  static int32_t result;
  int32_t temp = 0;
  while(k){
    printf("please give me a positive intial angle which is not bigger than 35999  \r\n");
    scanf("%d",&val);
    if(val<36000){      
      lastval = val;
      getGlobalValue(val,&temp);
      Fill_AngleQueue(temp);
      printf("Data in Queue is: \r\n");
      for(uint32_t cir=0;cir<20;cir++){
        printf("%d ",filterAngleQueue[cir]);
      }
      printf("\r\n adverage is %d \r\n",FQ_P2NewAngle);

      printf("I have initialize the queue and lastval\r\n");
      k = 0;
    }
  }

  k = 1;

  while(k){
    printf("please give me a val bias is not to high \r\n");
    scanf("%d",&val);
    if( ((abs(val - lastval)<500)||abs(val-lastval)>32000)&&(val<36000) ){
      lastval = val;
      if(getGlobalValue(val,&temp) != 0){
        result = AddNewAngle2Queue(temp);
      }
      printf("global angle is %d, after filter is %d\r\n",temp,result);
    }
    else if(val == 36000){
      k= 0;
    } 
    
  }
  return 1;
}
