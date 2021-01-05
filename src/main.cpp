#include <mbed.h>
#include <LIS3DSH.h>
#include<math.h>
#include "USBSerial.h"

//accelerometer
LIS3DSH acc(PA_7, PA_6, PA_5, PE_3);   //mosi, miso, clk , cs

//LEDs

DigitalOut sq_Led(LED3);   //orange
DigitalOut jj_Led(LED4);   //green
DigitalOut su_Led(LED5);    //red
DigitalOut pu_Led(LED6);   //blue

//User input button
DigitalIn start_exercise(BUTTON1);

USBSerial serial;



//state variables to differentiate standing, lying down type of exercise
typedef enum state_t {
	standing,
	lying
} state_t;

//state variables to differentiate different type of exercises
typedef enum ex_Type {
	situp,
	pushup,
    squat,
    jumping,
    noexercise
} ex_Type;

//function to calculate mean and standard deviation
void SD(float data[], int n, float arr[])
{
    float sum = 0.0, mean, standardDeviation = 0.0;
    int i;
    for(i = 0; i < n; ++i) sum += data[i];
    mean = sum/n;
    for(i = 0; i < n; ++i) standardDeviation += pow(data[i] - mean, 2);
    arr[0] = mean;
    arr[1] = sqrt(standardDeviation / n);
}

//function to indicate end of a particular exercise
void counting_display(int count){

  //count 0, blink 2 LEDs at a time
  if((count==0)){
    sq_Led.write(1);
    jj_Led.write(1);
    wait_ms(250);
    sq_Led.write(0);
    jj_Led.write(0);
    su_Led.write(1);
    pu_Led.write(1);
    wait_ms(250);
    su_Led.write(0);
    pu_Led.write(0);

  }
  if((count==1)){
    sq_Led.write(1);
    jj_Led.write(0);
    su_Led.write(0);
    pu_Led.write(0);
  }
  if(count==2){
    sq_Led.write(0);
    jj_Led.write(1);
    su_Led.write(0);
    pu_Led.write(0);
  }
  if(count==3){
    sq_Led.write(1);
    jj_Led.write(1);
    su_Led.write(0);
    pu_Led.write(0);
  }
  if(count==4){
    sq_Led.write(0);
    jj_Led.write(0);
    su_Led.write(1);
    pu_Led.write(0);
  }
  if(count==5){
    sq_Led.write(1);
    jj_Led.write(0);
    su_Led.write(1);
    pu_Led.write(0);
  }
  if(count==6){
    sq_Led.write(0);
    jj_Led.write(1);
    su_Led.write(1);
    pu_Led.write(0);
  }
  if(count==7){
    sq_Led.write(1);
    jj_Led.write(1);
    su_Led.write(1);
    pu_Led.write(0);
  }
  if(count==8){
    sq_Led.write(0);
    jj_Led.write(0);
    su_Led.write(0);
    pu_Led.write(1);
  }
  if(count==9){
    sq_Led.write(1);
    jj_Led.write(0);
    su_Led.write(0);
    pu_Led.write(1);
  }
  if(count==10){
    sq_Led.write(0);
    jj_Led.write(1);
    su_Led.write(0);
    pu_Led.write(1);
  }
  if(count==11){
    sq_Led.write(1);
    jj_Led.write(1);
    su_Led.write(0);
    pu_Led.write(1);
  }
  if(count==12){
    sq_Led.write(0);
    jj_Led.write(0);
    su_Led.write(1);
    pu_Led.write(1);
  }
  if(count==13){
    sq_Led.write(1);
    jj_Led.write(0);
    su_Led.write(1);
    pu_Led.write(1);
  }
  if(count==14){
    sq_Led.write(0);
    jj_Led.write(1);
    su_Led.write(1);
    pu_Led.write(1);
  }
  if(count==15){
    sq_Led.write(1);
    jj_Led.write(1);
    su_Led.write(1);
    pu_Led.write(1);
  }
}



int main() {
     int16_t X, Y, Z;    //signed integer variables for raw X,Y,Z values
     float x,y,z;        //acceleration reduced in terms of 'g'
     float x_orient,y_orient,z_orient;  //angles calculated from x,y,z
    
     //filter parameters
     const uint8_t N = 10;
     const float PI = 3.1415926;
     float buf_x[N];
     float buf_y[N];
     float buf_z[N];
     
    //index for arrays for filter calculation
     uint8_t index_x = 0;
     uint8_t index_y = 0;
     uint8_t index_z = 0;
     
     ex_Type ex_Type = noexercise;  //intially state is no exercise

     bool lying = false, standing = false; //later change this to structure variable


     float arr_y[2], arr_z[2];         //array to store mean and standard deviation values
     float y_mean, y_std, z_mean, z_std; //Variables to store mean and standard deviation

    //Array length for mean and std. calculation
     const uint8_t y_length = 20;
     const uint8_t z_length = 20;

     uint8_t y_ind = 0, z_ind = 0;

     float y_data[y_length], z_data[z_length];

     int begin;    // to record initial value of timer_1 for demo
     int begin_2;   // to record initial value of timer_2 for counting reps
     bool begin_detect = false; // to indicate beginning of detection
     bool demo = true;   //indicates operation is in demo mode


     //timer for detection
    Timer timer_1;
    timer_1.start();

    //timer for reps count
    Timer timer_2;
    timer_2.start();

    begin_2 = timer_2.read_ms();


    bool detected = false;  // to indicate completion of detection

    //to keep track of repetitions of exercise
    uint8_t sq_Count = 0, jj_Count = 0, su_Count = 0, pu_Count = 0;

     bool start_counting = false;

     //bool flag = false;

     bool ex_rep = false;

     bool hands_up= false, hands_down = true;

     if(acc.Detect() != 1) {
         printf("LIS3DSH Acceleromoter not detected!\n");
         while(1){ };
     }
     
     while(1) {
        acc.ReadData(&X, &Y, &Z);           //read X, Y, Z raw acceleration values
        x = (float) X/17694.0;
        y = (float) Y/17694.0;
        z = (float) Z/17694.0;
        
         
        //insert into circular buffer, normalized acceleration
        buf_x[index_x++] = x;
        buf_y[index_y++] = y;
        buf_z[index_z++] = z;
        
        //at the end of buffer, wrap around to the beginning
        if(index_x>N)
        {
            index_x = 0;        
            index_y = 0;        
            index_z = 0;
        }

        //-----------------------------------------------------------------------------
        //-----------------------------------------------------------------------------
        //filter

        float gz_filt_x = 0;
        float gz_filt_y = 0;
        float gz_filt_z = 0;

        //adding the samples
        for (uint8_t i = 0; i<N; i++)
        {
            gz_filt_x += buf_x[i];
            gz_filt_y += buf_y[i];
            gz_filt_z += buf_z[i]; 
        }

        
        //average of normalized acceleration values
        gz_filt_x /= float(N);
        gz_filt_y /= float(N);
        gz_filt_z /= float(N);

        //restrict to 1g
        if (gz_filt_x>1)
        {
            gz_filt_x = 1;
        }
        if (gz_filt_y>1)
        {
            gz_filt_y = 1;
        }
        if (gz_filt_z>1)
        {
            gz_filt_z = 1;
        }

        x_orient = 180*acos(gz_filt_x)/PI;  // angle wrt x
        y_orient = 180*acos(gz_filt_y)/PI;  // angle wrt y
        z_orient = 180*acos(gz_filt_z)/PI;  // angle wrt z

        
        //-------------------------------------------------------------------------------------
        //-------------------------------------------------------------------------------------
        //Start of demo mode

        //Button pressed indicating demo of exercise is started
        if(start_exercise && demo){
            demo = false;
            begin_detect = true;
            y_ind = 0;
            z_ind = 0;
            begin = timer_1.read_ms();    // timer for demo start
        }

        //Should run only during detection part => makes begin_detect = false when detection is done!!!
        //Collect sample only when detection is being done!
        if(begin_detect)
        {
            if (y_ind<20){
                y_data[y_ind] = float(Y);
                y_ind++;
            }

            if (z_ind<20){
                z_data[z_ind] = float(Z);
                z_ind++;
            }
        }


        //---------------------------------------------------------------------------------
        //---------------------------------------------------------------------------------
        //Exercise Detection start

        if(timer_1.read_ms()-begin > 4000 && begin_detect){   
            begin_detect = false;
            serial.printf("%s\n", "Detecing the Exercise");
               
            
            
            //calculate y_mean and y_std.
            SD(y_data, y_length, arr_y); //updates mean, sd. of 20 raw y acc. values
            y_mean = arr_y[0]; 
            y_std = arr_y[1];

            serial.printf("%f, %f\n\r", y_mean, y_std);
            


            //calculate z_mean and z_std
            SD(z_data, z_length, arr_z); //returns mean, std. of 20 raw y acc. values
            z_mean = arr_z[0];
            z_std = arr_z[1];

            
            
            
            //detecting if lying down or standing type exercise
            //y_mean and z_mean good differentiator
            if(y_mean >-10000){ 
                serial.printf("%s\n", "Its lying Exercise");
                lying = true;
                standing = false;
                detected = true;
            }else if(y_mean<-10000){  
                serial.printf("%s\n", "Its standing Exercise");
                lying = false;
                standing = true;
                detected = true;
            }
        }

            
        //detect squats or jumping jack, situps or pushups

        //angle z, z_mean good differentiator
        if(lying && detected){
            serial.printf("%s\n", "Inside lying");

            if(z_orient > 5 && z_orient < 90 && z_mean > 8000) {
                detected = false;
                serial.printf("%s\n", "Its a SITUP");
                su_Led.write(1);       // turn RED LED ON
                ex_Type = situp;
                start_counting = true;

           
            }
            else if(z_orient > 120 && z_mean < 0){
                detected = false;
                serial.printf("%s\n", "Its a PUSHUP");
                pu_Led.write(1);       // turn BLUE LED ON
                ex_Type = pushup;
                start_counting = true;
            }
        
        }else if(standing && detected){   
            serial.printf("%s\n", "Inside standing"); 
                
            if(y_std < 6000) {
                detected = false;
                serial.printf("%s\n", "Its a SQUAT");
                sq_Led.write(1);        // turn ORANGE LED ON
                ex_Type = squat;
                start_counting = true;
            
            
            } else if (y_std > 9000 ){
                detected = false;
                serial.printf("%s\n", "Its a JUMPING JACK");
                jj_Led.write(1);        // turn GREEN LED ON
                ex_Type = jumping;
                start_counting = true;
            }
            
        }
        /////COUNTING
        //Press button 2nd time to start counting actual exercise
        //Start counting the exercise repeatitions
        //---------------------------------------------------------------------------------
        //---------------------------------------------------------------------------------
        
        if(start_exercise && start_counting){
            ex_rep = true;
        }

        if(!demo && ex_rep){
            if(ex_Type == situp && (timer_2.read_ms() - begin_2) > 1500 ){
                //serial.printf("%d \n \r", Y);
                if (Y<-12500){
                    su_Count++;
                    serial.printf("Sitting up");
                    begin_2 = timer_2.read_ms();
                    
                }
                else if(Y>-1000){
                    serial.printf("lying down");
                    begin_2 = timer_2.read_ms();
                }
                //Call binary encoding LED combinations
                counting_display(su_Count);

                if(su_Count == 5){
                    su_Count = 0;     // to make sure initial count is zero when this exercise is started next time
                    serial.printf("End Exercise");
                    demo = true;
                    start_counting = false;
                    ex_rep = false;
                    counting_display(su_Count);
                }
            }
        
            if(ex_Type == pushup && (timer_2.read_ms() - begin_2) > 1000){
                //serial.printf("%d \n \r", Z);
                if (Z < -15000){
                    pu_Count++;
                    serial.printf("Push down");
                    begin_2 = timer_2.read_ms();
                    
                }
                //Call binary encoding LED combinations
                counting_display(pu_Count);

                if(pu_Count == 5){
                    pu_Count = 0;     // to make sure initial count is zero when this exercise is started next time
                    serial.printf("End Exercise");
                    demo = true;
                    start_counting = false;
                    ex_rep = false;
                    counting_display(pu_Count);
                }

            }
        
            
            // ideal time value found?! Yes, this value looks good
            if(ex_Type == squat && (timer_2.read_ms() - begin_2) > 1500){
                //serial.printf("%d \n \r", Y);
                if (Y<-17500){
                    serial.printf("%d \n \r", Y);
                    sq_Count++;
                    serial.printf("Squat Down \n");
                    serial.printf("%d \n \r", sq_Count);
                    begin_2 = timer_2.read_ms();
                }
                //Call binary encoding LED combinations
                counting_display(sq_Count);
                if(sq_Count == 5){
                    sq_Count = 0;     // to make sure initial count is zero when this exercise is started next time
                    serial.printf("End Exercise");
                    demo = true;
                    start_counting = false;
                    ex_rep = false;
                    counting_display(sq_Count);
                }
            }
        
            //not sure how to count, figure it out
            //750
            if(ex_Type == jumping && (timer_2.read_ms() - begin_2) > 1600){
                //serial.printf("%d \n \r", Y);
                if (Y > -10000){
                    jj_Count++;
                    serial.printf("%d \n \r", jj_Count);
                    begin_2 = timer_2.read_ms();
                }
                //Call binary encoding LED combinations
                counting_display(jj_Count);
                if(jj_Count == 5){
                    jj_Count = 0;     // to make sure initial count is zero when this exercise is started next time
                    serial.printf("End Exercise");
                    demo = true;
                    start_counting = false;
                    ex_rep = false;
                    counting_display(jj_Count);
                }

            }

        }

        wait_ms(100.0);  //delay before reading next values
    }
}

