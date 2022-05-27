/*
If an MPU6050 
    * is an ideal member of its tribe, 
    * is properly warmed up, 
    * is at rest in a neutral position, 
    * is in a location where the pull of gravity is exactly 1g, and 
    * has been loaded with the best possible offsets, 
then it will report 0 for all accelerations and displacements, except for 
Z acceleration, for which it will report 16384 (that is, 2^14).  Your device 
probably won't do quite this well, but good offsets will all get the baseline 
outputs close to these target values.

  Put the MPU6050 on a flat and horizontal surface, and leave it operating for 
5-10 minutes so its temperature gets stabilized.

  Run this program.  A "----- done -----" line will indicate that it has done its best.
With the current accuracy-related constants (NFast = 1000, NSlow = 10000), it will take 
a few minutes to get there.

  Along the way, it will generate a dozen or so lines of output, showing that for each 
of the 6 desired offsets, it is 
      * first, trying to find two estimates, one too low and one too high, and
      * then, closing in until the bracket can't be made smaller.

  The line just above the "done" line will look something like
    [567,567] --> [-1,2]  [-2223,-2223] --> [0,1] [1131,1132] --> [16374,16404] [155,156] --> [-1,1]  [-25,-24] --> [0,3] [5,6] --> [0,4]
As will have been shown in interspersed header lines, the six groups making up this
line describe the optimum offsets for the X acceleration, Y acceleration, Z acceleration,
X gyro, Y gyro, and Z gyro, respectively.  In the sample shown just above, the trial showed
that +567 was the best offset for the X acceleration, -2223 was best for Y acceleration, 
and so on.

  The need for the delay between readings (usDelay) was brought to my attention by Nikolaus Doppelhammer.
===============================================
*/

#include "I2Cdev.h"
#include "MPU6050.h"
#include "esp_log.h"
#include "config.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "mpu_calibration.h"

static const char *TAG = "mpu calibration";

MPU6050 accelgyro;

const int iAx = 0;
const int iAy = 1;
const int iAz = 2;
const int iGx = 3;
const int iGy = 4;
const int iGz = 5;

const wchar_t* labels[] {L"Accel X", L"Accel Y", L"Accel Z", L"Gyro X", L"Gyro Y", L"Gyro Z"};

const int usDelay = 3150;   // empirical, to hold sampling to 200 Hz
const int NFast =  1000;    // the bigger, the better (but slower)
const int NSlow = 10000;    // ..
const int LinesBetweenHeaders = 5;
      int LowValue[6];
      int HighValue[6];
      int Smoothed[6];
      int LowOffset[6];
      int HighOffset[6];
      int Target[6];
      int N;
    
void GetSmoothed()
  { int16_t RawValue[6];
    int i;
    long Sums[6];
    for (i = iAx; i <= iGz; i++)
      { Sums[i] = 0; }
    ulong start = esp_timer_get_time();

    for (i = 1; i <= N; i++) { // get sums
        accelgyro.getMotion6(&RawValue[iAx], &RawValue[iAy], &RawValue[iAz], 
                             &RawValue[iGx], &RawValue[iGy], &RawValue[iGz]);
        if ((i % 500) == 0) {
            ESP_LOGI(TAG, ".");
        }
        for (int j = iAx; j <= iGz; j++) {
            Sums[j] = Sums[j] + RawValue[j];
        }
    } // get sums
    unsigned long usForN = esp_timer_get_time() - start;
    ESP_LOGI(TAG, "reading at %d Hz", (int)(1000000/((usForN+N/2)/N)));
    for (i = iAx; i <= iGz; i++)
      { Smoothed[i] = (Sums[i] + N/2) / N ; }
  } // GetSmoothed

void Initialize()
  {
    // initialize device
    ESP_LOGI(TAG, "Initializing I2C devices...");
    i2c_config_t conf {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = PIN_SDA,
        .scl_io_num = PIN_CLK,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {.clk_speed = 400000,},
        .clk_flags = 0,
    };
    ESP_LOGI(TAG, "Configuring I2C");
	ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
    ESP_LOGI(TAG, "Installing I2C driver");
	ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
    accelgyro.initialize();
    accelgyro.setFullScaleGyroRange(MPU6050_GYRO_FS_2000);

    // verify connection
    ESP_LOGI(TAG, "Testing device connections...");
    if (accelgyro.testConnection()) {
        ESP_LOGI(TAG, "MPU6050 connection successful");
    } else {
        ESP_LOGI(TAG, "MPU6050 connection failed");
    }
    
    ESP_LOGI(TAG, "PID tuning Each Dot = 100 readings");
  /*A tidbit on how PID (PI actually) tuning works. 
    When we change the offset in the MPU6050 we can get instant results. This allows us to use Proportional and 
    integral of the PID to discover the ideal offsets. Integral is the key to discovering these offsets, Integral 
    uses the error from set-point (set-point is zero), it takes a fraction of this error (error * ki) and adds it 
    to the integral value. Each reading narrows the error down to the desired offset. The greater the error from 
    set-point, the more we adjust the integral value. The proportional does its part by hiding the noise from the 
    integral math. The Derivative is not used because of the noise and because the sensor is stationary. With the 
    noise removed the integral value lands on a solid offset after just 600 readings. At the end of each set of 100 
    readings, the integral value is used for the actual offsets and the last proportional reading is ignored due to 
    the fact it reacts to any noise.
  */
    accelgyro.CalibrateAccel(6);
    accelgyro.CalibrateGyro(6);
    ESP_LOGI(TAG, "\nat 600 Readings");
    ESP_LOGI(TAG, "AX: %d, AY: %d, AZ: %d, GX: %d, GY: %d, GZ: %d", accelgyro.getXAccelOffset(), accelgyro.getYAccelOffset(), accelgyro.getZAccelOffset(), accelgyro.getXGyroOffset(), accelgyro.getYGyroOffset(), accelgyro.getZGyroOffset());

    ESP_LOGI(TAG, "");
    accelgyro.CalibrateAccel(1);
    accelgyro.CalibrateGyro(1);
    ESP_LOGI(TAG, "700 Total Readings");
    ESP_LOGI(TAG, "AX: %d, AY: %d, AZ: %d, GX: %d, GY: %d, GZ: %d", accelgyro.getXAccelOffset(), accelgyro.getYAccelOffset(), accelgyro.getZAccelOffset(), accelgyro.getXGyroOffset(), accelgyro.getYGyroOffset(), accelgyro.getZGyroOffset());

    ESP_LOGI(TAG, "");
    accelgyro.CalibrateAccel(1);
    accelgyro.CalibrateGyro(1);
    ESP_LOGI(TAG, "800 Total Readings");
    ESP_LOGI(TAG, "AX: %d, AY: %d, AZ: %d, GX: %d, GY: %d, GZ: %d", accelgyro.getXAccelOffset(), accelgyro.getYAccelOffset(), accelgyro.getZAccelOffset(), accelgyro.getXGyroOffset(), accelgyro.getYGyroOffset(), accelgyro.getZGyroOffset());

    ESP_LOGI(TAG, "");
    accelgyro.CalibrateAccel(1);
    accelgyro.CalibrateGyro(1);
    ESP_LOGI(TAG, "900 Total Readings");
    ESP_LOGI(TAG, "AX: %d, AY: %d, AZ: %d, GX: %d, GY: %d, GZ: %d", accelgyro.getXAccelOffset(), accelgyro.getYAccelOffset(), accelgyro.getZAccelOffset(), accelgyro.getXGyroOffset(), accelgyro.getYGyroOffset(), accelgyro.getZGyroOffset());

    ESP_LOGI(TAG, "");    
    accelgyro.CalibrateAccel(1);
    accelgyro.CalibrateGyro(1);
    ESP_LOGI(TAG, "1000 Total Readings");
    ESP_LOGI(TAG, "AX: %d, AY: %d, AZ: %d, GX: %d, GY: %d, GZ: %d", accelgyro.getXAccelOffset(), accelgyro.getYAccelOffset(), accelgyro.getZAccelOffset(), accelgyro.getXGyroOffset(), accelgyro.getYGyroOffset(), accelgyro.getZGyroOffset());

     ESP_LOGI(TAG, "\n\n Any of the above offsets will work nice \n\n Lets proof the PID tuning using another method:"); 
  } // Initialize

void SetOffsets(int TheOffsets[6])
  { accelgyro.setXAccelOffset(TheOffsets [iAx]);
    accelgyro.setYAccelOffset(TheOffsets [iAy]);
    accelgyro.setZAccelOffset(TheOffsets [iAz]);
    accelgyro.setXGyroOffset (TheOffsets [iGx]);
    accelgyro.setYGyroOffset (TheOffsets [iGy]);
    accelgyro.setZGyroOffset (TheOffsets [iGz]);
  } // SetOffsets

void ShowProgress()
  {
    for (int i = iAx; i <= iGz; i++) {
        ESP_LOGI(TAG, "%S: [%d,%d] --> [%d,%d] ", labels[i], LowOffset[i], HighOffset[i], LowValue[i], HighValue[i]);
    }
  } // ShowProgress

void SetAveraging(int NewN) {
    N = NewN;
    ESP_LOGI(TAG, "averaging %d readings each time", N);
   } // SetAveraging
  
void PullBracketsIn()
  { bool AllBracketsNarrow;
    bool StillWorking;
    int NewOffset[6];
  
    ESP_LOGI(TAG, "\nclosing in:");
    AllBracketsNarrow = false;
    StillWorking = true;
    while (StillWorking) 
      { StillWorking = false;
        if (AllBracketsNarrow && (N == NFast))
          { SetAveraging(NSlow); }
        else
          { AllBracketsNarrow = true; }// tentative
        for (int i = iAx; i <= iGz; i++)
          { if (HighOffset[i] <= (LowOffset[i]+1))
              { NewOffset[i] = LowOffset[i]; }
            else
              { // binary search
                StillWorking = true;
                NewOffset[i] = (LowOffset[i] + HighOffset[i]) / 2;
                if (HighOffset[i] > (LowOffset[i] + 10))
                  { AllBracketsNarrow = false; }
              } // binary search
          }
        SetOffsets(NewOffset);
        GetSmoothed();
        for (int i = iAx; i <= iGz; i++)
          { // closing in
            if (Smoothed[i] > Target[i])
              { // use lower half
                HighOffset[i] = NewOffset[i];
                HighValue[i] = Smoothed[i];
              } // use lower half
            else
              { // use upper half
                LowOffset[i] = NewOffset[i];
                LowValue[i] = Smoothed[i];
              } // use upper half
          } // closing in
        ShowProgress();
      } // still working
   
  } // PullBracketsIn

void PullBracketsOut() { 
    bool Done = false;
    int NextLowOffset[6];
    int NextHighOffset[6];

    ESP_LOGI(TAG, "expanding:");
 
    while (!Done)
      { Done = true;
        SetOffsets(LowOffset);
        GetSmoothed();
        for (int i = iAx; i <= iGz; i++)
          { // got low values
            LowValue[i] = Smoothed[i];
            if (LowValue[i] >= Target[i])
              { Done = false;
                NextLowOffset[i] = LowOffset[i] - 1000;
              }
            else
              { NextLowOffset[i] = LowOffset[i]; }
          } // got low values
      
        SetOffsets(HighOffset);
        GetSmoothed();
        for (int i = iAx; i <= iGz; i++)
          { // got high values
            HighValue[i] = Smoothed[i];
            if (HighValue[i] <= Target[i])
              { Done = false;
                NextHighOffset[i] = HighOffset[i] + 1000;
              }
            else
              { NextHighOffset[i] = HighOffset[i]; }
          } // got high values
        ShowProgress();
        for (int i = iAx; i <= iGz; i++)
          { LowOffset[i] = NextLowOffset[i];   // had to wait until ShowProgress done
            HighOffset[i] = NextHighOffset[i]; // ..
          }
     } // keep going
  } // PullBracketsOut

void CalibrateMPU() {
    Initialize();
    for (int i = iAx; i <= iGz; i++)
      { // set targets and initial guesses
        Target[i] = 0; // must fix for ZAccel 
        HighOffset[i] = 0;
        LowOffset[i] = 0;
      } // set targets and initial guesses
    Target[iAz] = 16384;
    SetAveraging(NFast);
    
    PullBracketsOut();
    PullBracketsIn();
    
    ESP_LOGI(TAG, "-------------- done --------------");
  }
 
