/*

  CSC C85 - Embedded Systems - Project # 1 - EV3 Robot Localization
  
 This file provides the implementation of all the functionality required for the EV3
 robot localization project. Please read through this file carefully, and note the
 sections where you must implement functionality for your bot. 
 
 You are allowed to change *any part of this file*, not only the sections marked
 ** TO DO **. You are also allowed to add functions as needed (which must also
 be added to the header file). However, *you must clearly document* where you 
 made changes so your work can be properly evaluated by the TA.

 NOTES on your implementation:

 * It should be free of unreasonable compiler warnings - if you choose to ignore
   a compiler warning, you must have a good reason for doing so and be ready to
   defend your rationale with your TA.
 * It must be free of memory management errors and memory leaks - you are expected
   to develop high wuality, clean code. Test your code extensively with valgrind,
   and make sure its memory management is clean.
 
 In a nutshell, the starter code provides:
 
 * Reading a map from an input image (in .ppm format). The map is bordered with red, 
   must have black streets with yellow intersections, and buildings must be either
   blue, green, or be left white (no building).
   
 * Setting up an array with map information which contains, for each intersection,
   the colours of the buildings around it in ** CLOCKWISE ** order from the top-left.
   
 * Initialization of the EV3 robot (opening a socket and setting up the communication
   between your laptop and your bot)
   
 What you must implement:
 
 * All aspects of robot control:
   - Finding and then following a street
   - Recognizing intersections
   - Scanning building colours around intersections
   - Detecting the map boundary and turning around or going back - the robot must not
     wander outside the map (though of course it's possible parts of the robot will
     leave the map while turning at the boundary)

 * The histogram-based localization algorithm that the robot will use to determine its
   location in the map - this is as discussed in lecture.

 * Basic robot exploration strategy so the robot can scan different intersections in
   a sequence that allows it to achieve reliable localization
   
 * Basic path planning - once the robot has found its location, it must drive toward a 
   user-specified position somewhere in the map.

 --- OPTIONALLY but strongly recommended ---
 
  The starter code provides a skeleton for implementing a sensor calibration routine,
 it is called when the code receives -1  -1 as target coordinates. The goal of this
 function should be to gather informatin about what the sensor reads for different
 colours under the particular map/room illumination/battery level conditions you are
 working on - it's entirely up to you how you want to do this, but note that careful
 calibration would make your work much easier, by allowing your robot to more
 robustly (and with fewer mistakes) interpret the sensor data into colours. 
 
   --> The code will exit after calibration without running localization (no target!)
       SO - your calibration code must *save* the calibration information into a
            file, and you have to add code to main() to read and use this
            calibration data yourselves.
   
 What you need to understand thoroughly in order to complete this project:
 
 * The histogram localization method as discussed in lecture. The general steps of
   probabilistic robot localization.

 * Sensors and signal management - your colour readings will be noisy and unreliable,
   you have to handle this smartly
   
 * Robot control with feedback - your robot does not perform exact motions, you can
   assume there will be error and drift, your code has to handle this.
   
 * The robot control API you will use to get your robot to move, and to acquire 
   sensor data. Please see the API directory and read through the header files and
   attached documentation
   
 Starter code:
 F. Estrada, 2018 - for CSC C85 
 
*/

#include "EV3_Localization.h"

int map[400][4];            // This holds the representation of the map, up to 20x20
                            // intersections, raster ordered, 4 building colours per
                            // intersection.
int sx, sy;                 // Size of the map (number of intersections along x and y)
double beliefs[400][4];     // Beliefs for each location and motion direction

struct ColorInfo black, white, red, yellow, blue, green;

int main(int argc, char *argv[]) {
  char mapname[1024];
  int dest_x, dest_y, rx, ry;
  unsigned char *map_image;

  memset(&map[0][0],0,400*4*sizeof(int));
  sx=0;
  sy=0;

  if (argc<4) {
    fprintf(stderr,"Usage: EV3_Localization map_name dest_x dest_y\n");
    fprintf(stderr,"    map_name - should correspond to a properly formatted .ppm map image\n");
    fprintf(stderr,"    dest_x, dest_y - target location for the bot within the map, -1 -1 calls calibration routine\n");
    exit(1);
  }
  strcpy(&mapname[0],argv[1]);
  dest_x=atoi(argv[2]);
  dest_y=atoi(argv[3]);

  if (dest_x==-1&&dest_y==-1) {
    calibrate_sensor();
    exit(1);
  }

  read_calibration();

  /******************************************************************************************************************
   * OPTIONAL TO DO: If you added code for sensor calibration, add just below this comment block any code needed to
   *   read your calibration data for use in your localization code. Skip this if you are not using calibration
   * ****************************************************************************************************************/


  // Your code for reading any calibration information should not go below this line //
 
  map_image=readPPMimage(&mapname[0],&rx,&ry);
  if (map_image==NULL) {
    fprintf(stderr,"Unable to open specified map image\n");
    exit(1);
  }
  
  if (parse_map(map_image, rx, ry)==0) { 
    fprintf(stderr,"Unable to parse input image map. Make sure the image is properly formatted\n");
    free(map_image);
    exit(1);
  }

  if (dest_x<0||dest_x>=sx||dest_y<0||dest_y>=sy) {
    fprintf(stderr,"Destination location is outside of the map\n");
    free(map_image);
    exit(1);
  }

  // Initialize beliefs - uniform probability for each location and direction
  for (int j=0; j<sy; j++)
  for (int i=0; i<sx; i++) {
    beliefs[i+(j*sx)][0]=1.0/(double)(sx*sy*4);
    beliefs[i+(j*sx)][1]=1.0/(double)(sx*sy*4);
    beliefs[i+(j*sx)][2]=1.0/(double)(sx*sy*4);
    beliefs[i+(j*sx)][3]=1.0/(double)(sx*sy*4);
  }

  // Open a socket to the EV3 for remote controlling the bot.
  if (BT_open(HEXKEY)!=0) {
    fprintf(stderr,"Unable to open comm socket to the EV3, make sure the EV3 kit is powered on, and that the\n");
    fprintf(stderr," hex key for the EV3 matches the one in EV3_Localization.h\n");
    free(map_image);
    exit(1);
  }

  fprintf(stderr,"All set, ready to go!\n");
 
  /*******************************************************************************************************************************
  *
  *  TO DO - Implement the main localization loop, this loop will have the robot explore the map, scanning intersections and
  *          updating beliefs in the beliefs array until a single location/direction is determined to be the correct one.
  * 
  *          The beliefs array contains one row per intersection (recall that the number of intersections in the map_image
  *          is given by sx, sy, and that the map[][] array contains the colour indices of buildings around each intersection.
  *          Indexing into the map[][] and beliefs[][] arrays is by raster order, so for an intersection at i,j (with 0<=i<=sx-1
  *          and 0<=j<=sy-1), index=i+(j*sx)
  *  
  *          In the beliefs[][] array, you need to keep track of 4 values per intersection, these correspond to the belief the
  *          robot is at that specific intersection, moving in one of the 4 possible directions as follows:
  * 
  *          beliefs[i][0] <---- belief the robot is at intersection with index i, facing UP
  *          beliefs[i][1] <---- belief the robot is at intersection with index i, facing RIGHT
  *          beliefs[i][2] <---- belief the robot is at intersection with index i, facing DOWN
  *          beliefs[i][3] <---- belief the robot is at intersection with index i, facing LEFT
  * 
  *          Initially, all of these beliefs have uniform, equal probability. Your robot must scan intersections and update
  *          belief values based on agreement between what the robot sensed, and the colours in the map. 
  * 
  *          You have two main tasks these are organized into two major functions:
  * 
  *          robot_localization()    <---- Runs the localization loop until the robot's location is found
  *          go_to_target()          <---- After localization is achieved, takes the bot to the specified map location
  * 
  *          The target location, read from the command line, is left in dest_x, dest_y
  * 
  *          Here in main(), you have to call these two functions as appropriate. But keep in mind that it is always possible
  *          that even if your bot managed to find its location, it can become lost again while driving to the target
  *          location, or it may be the initial localization was wrong and the robot ends up in an unexpected place - 
  *          a very solid implementation should give your robot the ability to determine it's lost and needs to 
  *          run localization again.
  *
  *******************************************************************************************************************************/  

  // HERE - write code to call robot_localization() and go_to_target() as needed, any additional logic required to get the
  //        robot to complete its task should be here.

  //test_code();

  //find_street();

  //drive_along_street();

  //turn_at_intersection(0);
  //turn_at_intersection(1);

  // int tl, tr, bl, br;
  // scan_intersection(&tl, &tr, &br, &bl);
  // printf("scan intersection done, tl %d, tr %d, br %d, bl %d\n", tl, tr, br, bl);

  srand(time(0));


  int robot_x = -1;
  int robot_y = -1;
  int direction = -1;
  int succeed = 0;
  while (!succeed) {
      for (int j=0; j<sy; j++){
        for (int i=0; i<sx; i++) {
          beliefs[i+(j*sx)][0]=1.0/(double)(sx*sy*4);
          beliefs[i+(j*sx)][1]=1.0/(double)(sx*sy*4);
          beliefs[i+(j*sx)][2]=1.0/(double)(sx*sy*4);
          beliefs[i+(j*sx)][3]=1.0/(double)(sx*sy*4);
        }
      }
    //if (test_code(&robot_x, &robot_y, &direction) == 1) {
    if (robot_localization(&robot_x, &robot_y, &direction) == 1) {
      fprintf(stderr, "$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n\tmy location is %d %d\n\n$$$$$$$$$$$$$$$$$$$$$$$$$$\n", robot_x, robot_y);
      play_located_sound();
      if (go_to_target(robot_x, robot_y, direction, dest_x, dest_y) == 1) {
        play_succeed_sound();
        succeed = 1;
      } else {
        play_restarted_sound();
        fprintf(stderr, "failed in go to target :(\n");
      }
    } else {
      play_restarted_sound();
      fprintf(stderr, "failed in robot robot_localization :(\n");
    }
  }
  // if (go_to_target(0, 4, 3, dest_x, dest_y) == 1) {
  //   play_succeed_sound();
  // } else {
  //   fprintf(stderr, "failed in go to target :(\n");
  // }

  // play_succeed_sound();
  // Cleanup and exit - DO NOT WRITE ANY CODE BELOW THIS LINE
  BT_close();
  free(map_image);
  exit(0);
}

int test_code(int *robot_x, int *robot_y, int *direction) {
  char useless[3];

  if(!find_street()) {
    fprintf(stderr, "FAILED: cannot find street\n");
    return 0;
  }
  /*fprintf(stderr, "~~~~~~~~~~~~~~ Simulate find street ~~~~~~~~~~~~~~\n");
  fscanf(stdin, "%s", useless);*/

  while (1) {
    if(!drive_along_street()) {
      fprintf(stderr, "FAILED: cannot drive along street\n");
      return 0;
    }
    
    /*fprintf(stderr, "~~~~~~~~~~~~~~ Simulate drive along street ~~~~~~~~~~~~~~\n");
    fscanf(stdin, "%s", useless);*/
    int tl, tr, br, bl;
    //scan_intersection(&tl, &tr, &br, &bl);
    fprintf(stderr, "~~~~~~~~~~~~~~ Simulate scan intersection ~~~~~~~~~~~~~~\n");
    fprintf(stderr, "\tColor Code:\n\t\tblack: 1\n\t\tblue: 2\n\t\tgreen: 3\n\t\tyellow: 4\n\t\tred: 5\n\t\twhite: 6\n");
    fprintf(stderr, "\tPlease Enter Your color in order (tl, tr, br, bl): ");
    fscanf(stdin, "%d %d %d %d", &tl, &tr, &br, &bl);
    //int i, j;
    double sum = 0;
    double temp[400][4];
    //(with 0<=i<=sx-1 and 0<=j<=sy-1), index=i+(j*sx)
    for (int i = 0; i < sx; i++) {
      for (int j = 0; j < sy; j++) {
        int index = i+(j*sx);
        int mtl = map[index][0];
        int mtr = map[index][1];
        int mbr = map[index][2];
        int mbl = map[index][3];
        printf("\tat point i %d j %d index %d, map color %d %d %d %d \n", i, j, index, mtl, mtr, mbr, mbl);
        double pftop, pfright, pfbot, pfleft;
        pfleft = (i == 0) ? beliefs[index][3] : beliefs[index - 1][1];
        pftop = (j == 0) ? beliefs[index][0] : beliefs[index - sx][2];
        pfright = (i == (sx - 1)) ? beliefs[index][1] : beliefs[index + 1][3];
        pfbot = (j == (sy - 1)) ? beliefs[index][2] : beliefs[index + sx][0];
        printf("\tat point i %d j %d index %d, p from last %f %f %f %f\n", i, j, index, pfleft, pftop, pfright, pfbot);
        printf("\tscan intersection result, tl %d, tr %d, bl %d, br %d\n", tl, tr, br, bl);
        //printf("check: %d\n", (tl == mtl) && (tr == mtr) && (br == mbr) && (bl == mbl));
        if ((tl == mtl) && (tr == mtr) && (br == mbr) && (bl == mbl)) {
          // facing up
          temp[index][0] = pfbot;
          temp[index][1] = 0;
          temp[index][2] = 0;
          temp[index][3] = 0;
          sum = sum + pfbot;
        } else if (tl == mtr && tr == mbr && br == mbl && bl == mtl) {
          // facing right
          temp[index][0] = 0;
          temp[index][1] = pfleft;
          temp[index][2] = 0;
          temp[index][3] = 0;
          sum = sum + pfleft;
        } else if (tl == mbr && tr == mbl && br == mtl && bl == mtr) {
          // facing down
          temp[index][0] = 0;
          temp[index][1] = 0;
          temp[index][2] = pftop;
          temp[index][3] = 0;
          sum = sum + pftop;
        } else if (tl == mbl && tr == mtl && br == mtr && bl == mbr) {
          // facing left
          temp[index][0] = 0;
          temp[index][1] = 0;
          temp[index][2] = 0;
          temp[index][3] = pfright;
          sum = sum + pfright;
        } else {
          // not matching
          temp[index][0] = 0;
          temp[index][1] = 0;
          temp[index][2] = 0;
          temp[index][3] = 0;
        }
      }
    }

    if (sum == 0) {
      fprintf(stderr, "sum for beliefs is 0, check it !!!!!!!!!!!!!!!!!!!!!\n");
      return(0);
    }

    for(int i=0; i<sx*sy; i++) {
      beliefs[i][0] = temp[i][0], beliefs[i][1] = temp[i][1], beliefs[i][2] = temp[i][2], beliefs[i][3] = temp[i][3];
      printf("at index %d, %f %f %f %f\n", i, beliefs[i][0] , beliefs[i][1] , beliefs[i][2] , beliefs[i][3]);
    }


    //normalize
    //double normal_sum = 0;
    for (int i = 0; i < sx; i++) {
      for (int j = 0; j < sy; j++) {
        int index = i+(j*sx);
        beliefs[index][0] = beliefs[index][0]/sum;
        beliefs[index][1] = beliefs[index][1]/sum;
        beliefs[index][2] = beliefs[index][2]/sum;
        beliefs[index][3] = beliefs[index][3]/sum;
        // check if determined location
        if (beliefs[index][0] == 1) {
          *robot_x = i;
          *robot_y = j;
          *direction = 0;
          return(1);
        } else if (beliefs[index][1] == 1) {
          *robot_x = i;
          *robot_y = j;
          *direction = 1;
          return(1);
        } else if (beliefs[index][2] == 1) {
          *robot_x = i;
          *robot_y = j;
          *direction = 2;
          return(1);
        } else if (beliefs[index][3] == 1) {
          *robot_x = i;
          *robot_y = j;
          *direction = 3;
          return(1);
        }
      }
    }

    // need more navigation to make decision
    // randomly choose a direction to try
    int rd = rand() % 3;
    if (rd == 1) {
      // turn right
      turn_at_intersection(0);
      /*fprintf(stderr, "~~~~~~~~~~~~~~ Simulate turn right ~~~~~~~~~~~~~~");
      fscanf(stdin, "%s", useless);*/
      for (int i = 0; i < sx; i++) {
        for (int j = 0; j < sy; j++) {
          int index = i+(j*sx);
          double bup = beliefs[index][0];
          double bright = beliefs[index][1];
          double bdown = beliefs[index][2];
          double bleft = beliefs[index][3];
          beliefs[index][0] = bleft;
          beliefs[index][1] = bup;
          beliefs[index][2] = bright;
          beliefs[index][3] = bdown;
        }
      }
      fprintf(stderr, "\n\n\n~~~~~~~~~~~~~~ turned right ~~~~~~~~~~~~~~~~~~~~~~~\n\n\n");
    } else if (rd == 2) {
      // turn left
      turn_at_intersection(1);
      /*fprintf(stderr, "~~~~~~~~~~~~~~ Simulate turn left ~~~~~~~~~~~~~~");
      fscanf(stdin, "%s", useless);*/
      for (int i = 0; i < sx; i++) {
        for (int j = 0; j < sy; j++) {
          int index = i+(j*sx);
          double bup = beliefs[index][0];
          double bright = beliefs[index][1];
          double bdown = beliefs[index][2];
          double bleft = beliefs[index][3];
          beliefs[index][0] = bright;
          beliefs[index][1] = bdown;
          beliefs[index][2] = bleft;
          beliefs[index][3] = bup;
        }
      }
      fprintf(stderr, "\n\n\n~~~~~~~~~~~~~~ turned left ~~~~~~~~~~~~~~~~~~~~~~~\n\n\n");
    }
  }

  fprintf(stderr, "end of while loop, check it !!!!!!!!!!!!!!!!!!!!\n");
  *(robot_x)=-1;
  *(robot_y)=-1;
  *(direction)=-1;
  return(0);
}

int find_street(void) {
  /*
    * This function gets your robot onto a street, wherever it is placed on the map. You can do this in many ways, but think
    * about what is the most effective and reliable way to detect a street and stop your robot once it's on it.
    * 
    * You can use the return value to indicate success or failure, or to inform the rest of your code of the state of your
    * bot after calling this function
    */   
  int rgb[3];
  while (1) {
    if (BT_read_colour_sensor_RGB(PORT_4, rgb) == 0) {
      // sensing black or yellow
      printf("In find_street, color: %d %d %d\n", rgb[0], rgb[1], rgb[2]);
      if (determine_color(rgb) == BLACKCOLOR || determine_color(rgb) == YELLOWCOLOR) {
        // drive to see if direction is good (stay on black or yellow)
        int count = 0;
        int error_count = 0;
        BT_drive(MOTOR_A,  MOTOR_D, 7.5);
        // count consective error or colour difference to tell colour changing
        while (error_count < 3) {
          if (BT_read_colour_sensor_RGB(PORT_4, rgb) == 0) {
            if (determine_color(rgb) == BLACKCOLOR || determine_color(rgb) == YELLOWCOLOR) {
              error_count = 0;
              // keep reading black or yellow without colour changing, on street
              if (count == 8) {
                fprintf(stderr,"In find_street, succeeded\n");
                BT_all_stop(0);
                return(1);
              } else {
                count++;
              }
            } else {
              error_count++;
            }
          } else {
            fprintf(stderr,"In find_street, BT_colour_sensor_RGB(): Command failed\n");
            BT_all_stop(0);
            return 0;
          }
        }
        //BT_all_stop(0);
        // reading is no more black nor yellow, need to turn
        int street_count = 0;
        BT_turn(MOTOR_A, 7.5, MOTOR_D, -7.5);
        // count number of black or yellow occurance
        while (street_count < 1) {
          if (BT_read_colour_sensor_RGB(PORT_4, rgb) == 0) {
            if (determine_color(rgb) == BLACKCOLOR || determine_color(rgb) == YELLOWCOLOR) {
              street_count++;
            }
          } else {
            fprintf(stderr,"In find_street, BT_colour_sensor_RGB(): Command failed\n");
            BT_all_stop(0);
            return 0;
          }
        }
        // stop when continue reading black or yellow, start from the top again
        //BT_all_stop(0);
      } else if (determine_color(rgb) == REDCOLOR) { // sensing red
        // need to turn until it is not red
        BT_turn(MOTOR_A, 10, MOTOR_D, -10);
        int count = 0;
        // count number of non red colour occurance
        while (count < 10) {
          if (BT_read_colour_sensor_RGB(PORT_4, rgb) == 0) {
            if (determine_color(rgb) != REDCOLOR) {
              count++;
            }
          } else {
            fprintf(stderr,"In find_street, BT_colour_sensor_RGB(): Command failed\n");
            BT_all_stop(0);
            return 0;
          }
        }
        // reading is no more red, start from the top again
        //BT_all_stop(0);
      } else { // initial is not black, yellow nor red
        // drive to until see black, yellow or red
        BT_drive(MOTOR_A, MOTOR_D, 7.5);
        int street_count = 0;
        int red_count = 0;
        // count number of occurance for black/yellow, and red separately
        while (street_count < 1 && red_count < 1) {
          if (BT_read_colour_sensor_RGB(PORT_4, rgb) == 0) {
            if (determine_color(rgb) == REDCOLOR) {
              red_count++;
            } else if (determine_color(rgb) == BLACKCOLOR || determine_color(rgb) == YELLOWCOLOR) {
              street_count++;
            }
          } else {
            fprintf(stderr,"In find_street, BT_colour_sensor_RGB(): Command failed\n");
            BT_all_stop(0);
            return 0;            
          }
        }
        // determined black/yellow or red, start from the top again
        //BT_all_stop(0);
      }
    } else {
      fprintf(stderr,"In find_street, BT_colour_sensor_RGB(): Command failed\n");
      BT_all_stop(0);
      return 0;
    }
  }
  return(0);
}

int drive_along_street(void) {
  /*
    * This function drives your bot along a street, making sure it stays on the street without straying to other pars of
    * the map. It stops at an intersection.
    * 
    * You can implement this in many ways, including a controlled (PID for example), a neural network trained to track and
    * follow streets, or a carefully coded process of scanning and moving. It's up to you, feel free to consult your TA
    * or the course instructor for help carrying out your plan.
    * 
    * You can use the return value to indicate success or failure, or to inform the rest of your code of the state of your
    * bot after calling this function.
    */   
  int rgb[3];
  while (1) {
    if (BT_read_colour_sensor_RGB(PORT_4, rgb) == 0) {
      // start driving
      BT_drive(MOTOR_A, MOTOR_D, 7.5);

      // move out if start on yellow
      if (determine_color(rgb) == YELLOWCOLOR) {
        int not_yellow_count = 0;
        while (not_yellow_count < 5) {
          if (BT_read_colour_sensor_RGB(PORT_4, rgb) == 0) {
            if (determine_color(rgb) != YELLOWCOLOR) {
              not_yellow_count++;
            }
          } else {
            fprintf(stderr,"In drive_along_street, BT_colour_sensor_RGB(): Command failed\n");
            BT_all_stop(0);
            return 0;
          }
        }
      }


      int error_count = 0;
      // keep driving and reading colour, should be driving on black
      // count error readings and stop driving if keep reading different colours
      while (error_count < 3) {
        if (BT_read_colour_sensor_RGB(PORT_4, rgb) == 0) {
          if (determine_color(rgb) == BLACKCOLOR) {
            error_count = 0;
          } else {
            error_count++;
          }
        } else {
          fprintf(stderr,"In drive_along_street, BT_colour_sensor_RGB(): Command failed\n");
          BT_all_stop(0);
          return 0;
        }
      }

      BT_all_stop(0);
      // driving on other colors detected, count to see which color
      int red_count = 0;
      int yellow_count = 0;
      int other_count = 0;
      while (red_count < 2 && yellow_count < 2 && other_count < 5) {
        if (BT_read_colour_sensor_RGB(PORT_4, rgb) == 0) {
          if (determine_color(rgb) == REDCOLOR) {
            red_count++;
          } else if (determine_color(rgb) == YELLOWCOLOR) {
            yellow_count++;
          } else {
            other_count++;
          }
        } else {
          fprintf(stderr,"In drive_along_street, BT_colour_sensor_RGB(): Command failed\n");
          BT_all_stop(0);
          return 0;
        }
      }

      // handle cases for color detected
      if (yellow_count == 2) { // yellow, succeeded
        fprintf(stderr,"In drive_along_street, succeeded\n");
        BT_all_stop(0);
        return 1;
      } else if (red_count == 2) { // red, turn around
        // need to turn until it is black
        BT_turn(MOTOR_A, 10, MOTOR_D, -10);
        int black_count = 0;
        // count number of black colour occurance
        while (black_count < 1) {
          if (BT_read_colour_sensor_RGB(PORT_4, rgb) == 0) {
            if (determine_color(rgb) == BLACKCOLOR) {
              black_count++;
            }
            // } else if (determine_color(rgb) == YELLOWCOLOR) {
            //   fprintf(stderr,"In drive_along_street, succeeded\n");
            //   BT_all_stop(0);
            //   return 1;
            // }
          } else {
            fprintf(stderr,"In drive_along_street, BT_colour_sensor_RGB(): Command failed\n");
            BT_all_stop(0);
            return 0;
          }
        }
        // turned around and sensing black, start from the top again
        //BT_all_stop(0);
      } else { // other color, need to center the car back to the street
        int right_count = 0;
        int right_black_count = 0;
        int right_yellow_count = 0;
        int left_count = 0;
        int left_black_count = 0;
        int left_yellow_count = 0;
        int right_fixed = 0;
        int left_fixed = 0;

        // turn right first to check fixing
        BT_turn(MOTOR_A, 7.5, MOTOR_D, -7.5);
        //while (right_count < 10 && right_black_count < 3 && !right_yellow) {
        while (right_count < 10 && !right_fixed) {
          if (BT_read_colour_sensor_RGB(PORT_4, rgb) == 0) {
            if (determine_color(rgb) == BLACKCOLOR) {
              // right_black_count++;
              // // turned back to track, no more turning action needed
              // if (right_black_count == 1) {
                right_fixed = 1;
              //}
            } else if (determine_color(rgb) == YELLOWCOLOR) {
              right_yellow_count++;
              if (right_yellow_count == 2) {
                BT_all_stop(0);
                fprintf(stderr,"In drive_along_street, succeeded\n");
                return 1;
              }
            }
            right_count++;
          } else {
            fprintf(stderr,"In drive_along_street, BT_colour_sensor_RGB(): Command failed\n");
            BT_all_stop(0);
            return 0;
          }
        }
        BT_all_stop(0);

        // if right turn did not fix the position
        if (!right_fixed) {
          // turn left to check fixing
          BT_turn(MOTOR_A, -7.5, MOTOR_D, 7.5);
          //while (left_count < 20 && left_black_count < 3 && !left_yellow) {
          while (left_count < 25 && !left_fixed) {
            if (BT_read_colour_sensor_RGB(PORT_4, rgb) == 0) {
              if (determine_color(rgb) == BLACKCOLOR) {
                // left_black_count++;
                // // turned back to track, no more turning action needed
                // if (left_black_count == 3) {
                  left_fixed = 1;
                //}
              } else if (determine_color(rgb) == YELLOWCOLOR) {
                left_yellow_count++;
                if (left_yellow_count == 2) {
                  BT_all_stop(0);
                  fprintf(stderr,"In drive_along_street, succeeded\n");
                  return 1;
                }
              }
              left_count++;
            } else {
              fprintf(stderr,"In drive_along_street, BT_colour_sensor_RGB(): Command failed\n");
              BT_all_stop(0);
              return 0;
            }
          }
        }
        BT_all_stop(0);

        // if slightly turn to both sides cannot fix the centering
        // may need to call find street again, or other way of fixing
        if (!right_fixed && !left_fixed) {
            // start driving back
            BT_drive(MOTOR_A, MOTOR_D, -7.5);

            int back_black_count = 0;
            while (back_black_count < 2) {
              if (BT_read_colour_sensor_RGB(PORT_4, rgb) == 0) {
                if (determine_color(rgb) == BLACKCOLOR || determine_color(rgb) == YELLOWCOLOR) {
                  back_black_count++;
                }
              } else {
                fprintf(stderr,"In drive_along_street, BT_colour_sensor_RGB(): Command failed\n");
                BT_all_stop(0);
                return 0;
              }
            }
            // fprintf(stderr,"In drive_along_street, cannot center back to street !!!!!!!!! check what to do\n");
            // BT_all_stop(0);
            // return 0;
        }
        BT_all_stop(0);
      }
    } else {
      fprintf(stderr,"In drive_along_street, BT_colour_sensor_RGB(): Command failed\n");
      BT_all_stop(0);
      return 0;
    }
  }
  return(0);
}

int scan_building(int lpower, int rpower) {
  int rgb[3], cnt = 0, currentColor = -1, ans=-1;
  if(BT_turn(MOTOR_A, lpower, MOTOR_D, rpower) != 0) {
    // Motor cannot start
    fprintf(stderr,"BT_turn(): Command failed\n");
    return 0;
  }
  while(1) {
    if(robust_color(rgb)==0) {
      fprintf(stderr,"Current color: %d %d %d\n", rgb[0], rgb[1], rgb[2]);
      currentColor = determine_color(rgb);
      if(currentColor > 0 && currentColor != YELLOWCOLOR && currentColor != BLACKCOLOR) { // Stop when reaching to a point that is good to turn
        cnt++;
        if(cnt == 3) break;
      }
    }
  }
  // then stop moving forward and ready to turn
  if(BT_all_stop(0) != 0) {
    // Motor cannot stop
    fprintf(stderr,"BT_all_stop(): Command failed\n");
    return 0;
  }
  fprintf(stderr,"======================\nThis Building color is : %d\n=========================\n", currentColor);
  ans = currentColor;

  if(BT_turn(MOTOR_A, -lpower, MOTOR_D, -rpower) != 0) {
    // Motor cannot start
    fprintf(stderr,"BT_turn(): Command failed\n");
    return 0;
  }
  cnt = 0;
  while(1) {
    if(robust_color(rgb)==0) {
      fprintf(stderr,"Current color: %d %d %d\n", rgb[0], rgb[1], rgb[2]);
      currentColor = determine_color(rgb);
      if(currentColor > 0 && (currentColor == YELLOWCOLOR || currentColor == BLACKCOLOR) ) { // Stop when reaching to a point that is good to turn
        cnt++;
        if(cnt == 2) break;
      }
    }
  }
  // then stop moving forward and ready to turn
  if(BT_all_stop(0) != 0) {
    // Motor cannot stop
    fprintf(stderr,"BT_all_stop(): Command failed\n");
    return 0;
  }
  return ans;
}

int scan_intersection(int *tl, int *tr, int *br, int *bl) {
  /*
  * This function carries out the intersection scan - the bot should (obviously) be placed at an intersection for this,
  * and the specific set of actions will depend on how you designed your bot and its sensor. Whatever the process, you
  * should make sure the intersection scan is reliable - i.e. the positioning of the sensor is reliably over the buildings
  * it needs to read, repeatably, and as the robot moves over the map.
  * 
  * Use the APIs sensor reading calls to poll the sensors. You need to remember that sensor readings are noisy and 
  * unreliable so * YOU HAVE TO IMPLEMENT SOME KIND OF SENSOR / SIGNAL MANAGEMENT * to obtain reliable measurements.
  * 
  * Recall your lectures on sensor and noise management, and implement a strategy that makes sense. Document your process
  * in the code below so your TA can quickly understand how it works.
  * 
  * Once your bot has read the colours at the intersection, it must return them using the provided pointers to 4 integer
  * variables:
  * 
  * tl - top left building colour
  * tr - top right building colour
  * br - bottom right building colour
  * bl - bottom left building colour
  * 
  * The function's return value can be used to indicate success or failure, or to notify your code of the bot's state
  * after this call.
  */

  /************************************************************************************************************************
  *   TO DO  -   Complete this function
  ***********************************************************************************************************************/

  int rgb[3];
  // first go forward to a point that is not intersection
  if(BT_drive(MOTOR_A,  MOTOR_D, 10) != 0) {
    // Motor cannot start
    fprintf(stderr,"BT_motor_port_start(): Command failed\n");
    return 0;
  }
  int cnt=0;
  while(1) {
    if(robust_color(rgb) == 0) {
      fprintf(stderr,"Current color: %d %d %d\n", rgb[0], rgb[1], rgb[2]);
      int currentColor = determine_color(rgb);
      if(currentColor > 0 && currentColor != YELLOWCOLOR) { // Stop when reaching to a point that is good to turn
        cnt++;
        if(cnt == 3) break;
      }
    }
  }

  // then stop moving forward and ready to turn
  if(BT_all_stop(0) != 0) {
    // Motor cannot stop
    fprintf(stderr,"BT_all_stop(): Command failed\n");
    return 0;
  }
  // scan top left and right
  *(tl) = scan_building(-10, 10);
  *(tr) = scan_building(10, -10);

  // then go backword to intersection
  if(BT_drive(MOTOR_A,  MOTOR_D, -8) != 0) {
    // Motor cannot start
    fprintf(stderr,"BT_motor_port_start(): Command failed\n");
    return 0;
  }
  cnt = 0;
  while(1) {
    if(robust_color(rgb) == 0) {
      fprintf(stderr,"Current color: %d %d %d\n", rgb[0], rgb[1], rgb[2]);
      int currentColor = determine_color(rgb);
      if(currentColor > 0 && currentColor == YELLOWCOLOR) { // Stop when reaching to a point that is good to turn
        cnt++;
        if(cnt == 2) break;
      }
    }
  }

  if(BT_all_stop(0) != 0) {
    // Motor cannot stop
    fprintf(stderr,"BT_all_stop(): Command failed\n");
    return 0;
  }

  // then go backward to a point that is not intersection
  if(BT_drive(MOTOR_A,  MOTOR_D, -10) != 0) {
    // Motor cannot start
    fprintf(stderr,"BT_motor_port_start(): Command failed\n");
    return 0;
  }
  cnt=0;
  while(1) {
    if(robust_color(rgb) == 0) {
      fprintf(stderr,"Current color: %d %d %d\n", rgb[0], rgb[1], rgb[2]);
      int currentColor = determine_color(rgb);
      if(currentColor > 0 && currentColor != YELLOWCOLOR) { // Stop when reaching to a point that is good to turn
        cnt++;
        if(cnt == 3) break;
      }
    }
  }

  // then stop moving and ready to turn
  if(BT_all_stop(0) != 0) {
    // Motor cannot stop
    fprintf(stderr,"BT_all_stop(): Command failed\n");
    return 0;
  }
  *(br) = scan_building(10, -10);
  *(bl) = scan_building(-10, 10);
  // Maybe drive along street
  
  // then go backword to intersection
  if(BT_drive(MOTOR_A,  MOTOR_D, 8) != 0) {
    // Motor cannot start
    fprintf(stderr,"BT_motor_port_start(): Command failed\n");
    return 0;
  }
  while(1) {
    if(robust_color(rgb) == 0) {
      fprintf(stderr,"Current color: %d %d %d\n", rgb[0], rgb[1], rgb[2]);
      int currentColor = determine_color(rgb);
      if(currentColor > 0 && currentColor == YELLOWCOLOR) { // Stop when reaching to a point that is good to turn
        break;
      }
    }
  }

  if(BT_all_stop(0) != 0) {
    // Motor cannot stop
    fprintf(stderr,"BT_all_stop(): Command failed\n");
    return 0;
  }

  // Return invalid colour values, and a zero to indicate failure (you will replace this with your code)
  /**(tl)=-1;
  *(tr)=-1;
  *(br)=-1;
  *(bl)=-1;*/
  return 1;
}

int turn_at_intersection(int turn_direction) {
  /*
  * This function is used to have the robot turn either left or right at an intersection (obviously your bot can not just
  * drive forward!). 
  * 
  * If turn_direction=0, turn right, else if turn_direction=1, turn left.
  * 
  * You're free to implement this in any way you like, but it should reliably leave your bot facing the correct direction
  * and on a street it can follow. 
  * 
  * You can use the return value to indicate success or failure, or to inform your code of the state of the bot
  */
  int rgb[3];
  // first go forward to a point that is not intersection
  if(BT_drive(MOTOR_A,  MOTOR_D, 10) == 0) {
    int cnt=0;
    while(1) {
      if(BT_read_colour_sensor_RGB(PORT_4, rgb) == 0) {
        fprintf(stderr,"Current color: %d %d %d\n", rgb[0], rgb[1], rgb[2]);
        int currentColor = determine_color(rgb);
        if(currentColor > 0 && currentColor != YELLOWCOLOR) { // Stop when reaching to a point that is good to turn
          cnt++;
          if(cnt == 3) break;
        }
      } else {
        // Color sensor failed
        fprintf(stderr,"BT_colour_sensor_RGB(): Command failed\n");
        return 0;
      }
    }
  } else {
    // Motor cannot start
    fprintf(stderr,"BT_motor_port_start(): Command failed\n");
    return 0;
  }

  // then stop moving forward and ready to turn
  if(BT_all_stop(0) != 0) {
    // Motor cannot stop
    fprintf(stderr,"BT_all_stop(): Command failed\n");
    return 0;
  }

  int status = 0, currentColor = determine_color(rgb);
  if(currentColor != BLACKCOLOR) status = 1;
  //fprintf(stderr,"===============\nCurrent status: %d\n===============\n", status);

  if(turn_direction == 0) { // turn right
    if(BT_turn(MOTOR_A, 10,  MOTOR_D, -10) == 0) {
      int nbcnt = 0, bcnt = 0;
      while(1) {
        if(BT_read_colour_sensor_RGB(PORT_4, rgb) == 0) {
          //fprintf(stderr,"Current color: %d %d %d\n", rgb[0], rgb[1], rgb[2]);
          int currentColor = determine_color(rgb);
          //fprintf(stderr,"\t\tColor code: %d\n",currentColor);
          if(currentColor > 0 && currentColor != BLACKCOLOR && status == 0) {
            nbcnt ++;
            if(nbcnt == 3) status = 1;
          }
          else if(currentColor > 0 && currentColor == BLACKCOLOR && status == 1) {
            bcnt++;
            if(bcnt==2) {status = 2; break;}
          }
        } else {
          // Color sensor failed
          fprintf(stderr,"BT_colour_sensor_RGB(): Command failed\n");
          return 0;
        }
      }
    } else {
      // Turn failed
      fprintf(stderr,"BT_turn(): Command failed\n");
      return 0;
    }
  } else { // turn left
    if(BT_turn(MOTOR_A, -10,  MOTOR_D, 10) == 0) {
      int nbcnt = 0, bcnt = 0;
      while(1) {
        if(BT_read_colour_sensor_RGB(PORT_4, rgb) == 0) {
          //fprintf(stderr,"Current color: %d %d %d\n", rgb[0], rgb[1], rgb[2]);
          int currentColor = determine_color(rgb);
          //fprintf(stderr,"\t\tColor code: %d\n",currentColor);
          if(currentColor >0 && currentColor != BLACKCOLOR && status == 0) {
            nbcnt ++;
            if(nbcnt == 3) status = 1;
          }
          else if(currentColor >0 && currentColor == BLACKCOLOR && status == 1) {
            bcnt++;
            if(bcnt==2) {status = 2; break;}
          }
        } else {
          // Color sensor failed
          fprintf(stderr,"BT_colour_sensor_RGB(): Command failed\n");
          return 0;
        }
      }
    } else {
      // Turn failed
      fprintf(stderr,"BT_turn(): Command failed\n");
      return 0;
    }
  }

  if(BT_all_stop(0) != 0) {
    // Motor cannot stop
    fprintf(stderr,"BT_all_stop(): Command failed\n");
    return 0;
  }
  return 1;
}

int robot_localization(int *robot_x, int *robot_y, int *direction) {
 /*  This function implements the main robot localization process. You have to write all code that will control the robot
  *  and get it to carry out the actions required to achieve localization.
  *
  *  Localization process:
  *
  *  - Find the street, and drive along the street toward an intersection
  *  - Scan the colours of buildings around the intersection
  *  - Update the beliefs in the beliefs[][] array according to the sensor measurements and the map data
  *  - Repeat the process until a single intersection/facing direction is distintly more likely than all the rest
  * 
  *  * We have provided headers for the following functions:
  * 
  *  find_street()
  *  drive_along_street()
  *  scan_intersection()
  *  turn_at_intersection()
  * 
  *  You *do not* have to use them, and can write your own to organize your robot's work as you like, they are
  *  provided as a suggestion.
  * 
  *  Note that *your bot must explore* the map to achieve reliable localization, this means your intersection
  *  scanning strategy should not rely exclusively on moving forward, but should include turning and exploring
  *  other streets than the one your bot was initially placed on.
  * 
  *  For each of the control functions, however, you will need to use the EV3 API, so be sure to become familiar with
  *  it.
  * 
  *  In terms of sensor management - the API allows you to read colours either as indexed values or RGB, it's up to
  *  you which one to use, and how to interpret the noisy, unreliable data you're likely to get from the sensor
  *  in order to update beliefs.
  * 
  *  HOWEVER: *** YOU must document clearly both in comments within this function, and in your report, how the
  *               sensor is used to read colour data, and how the beliefs are updated based on the sensor readings.
  * 
  *  DO NOT FORGET - Beliefs should always remain normalized to be a probability distribution, that means the
  *                  sum of beliefs over all intersections and facing directions must be 1 at all times.
  * 
  *  The function receives as input pointers to three integer values, these will be used to store the estimated
  *   robot's location and facing direction. The direction is specified as:
  *   0 - UP
  *   1 - RIGHT
  *   2 - BOTTOM
  *   3 - LEFT
  * 
  *  The function's return value is 1 if localization was successful, and 0 otherwise.
  */
 
  /************************************************************************************************************************
   *   TO DO  -   Complete this function
   ***********************************************************************************************************************/

  // Return an invalid location/direction and notify that localization was unsuccessful (you will delete this and replace it
  // with your code).

  find_street();

  while (1) {
    drive_along_street();
    printf("~~~~~~~~~~~~~~ finish driving ~~~~~~~~~~~~~~~~~~~~~~~\n\n\n");
    int tl, tr, br, bl;
    scan_intersection(&tl, &tr, &br, &bl);
    //int i, j;
    double sum = 0;
    double temp[400][4];
    //(with 0<=i<=sx-1 and 0<=j<=sy-1), index=i+(j*sx)
    for (int i = 0; i < sx; i++) {
      for (int j = 0; j < sy; j++) {
        int index = i+(j*sx);
        int mtl = map[index][0];
        int mtr = map[index][1];
        int mbr = map[index][2];
        int mbl = map[index][3];
        printf("\tat point i %d j %d index %d, map color %d %d %d %d \n", i, j, index, mtl, mtr, mbr, mbl);
        double pftop, pfright, pfbot, pfleft;
        pfleft = (i == 0) ? beliefs[index][3] : beliefs[index - 1][1];
        pftop = (j == 0) ? beliefs[index][0] : beliefs[index - sx][2];
        pfright = (i == (sx - 1)) ? beliefs[index][1] : beliefs[index + 1][3];
        pfbot = (j == (sy - 1)) ? beliefs[index][2] : beliefs[index + sx][0];
        printf("\tat point i %d j %d index %d, p from last %f %f %f %f\n", i, j, index, pfleft, pftop, pfright, pfbot);
        printf("\tscan intersection result, tl %d, tr %d, bl %d, br %d\n", tl, tr, br, bl);
        //printf("check: %d\n", (tl == mtl) && (tr == mtr) && (br == mbr) && (bl == mbl));
        if ((tl == mtl) && (tr == mtr) && (br == mbr) && (bl == mbl)) {
          // facing up
          temp[index][0] = pfbot;
          temp[index][1] = 0;
          temp[index][2] = 0;
          temp[index][3] = 0;
          sum = sum + pfbot;
          printf("\tfacing top: sum = %lf\n\n\n", sum);
        } else if (tl == mtr && tr == mbr && br == mbl && bl == mtl) {
          // facing right
          temp[index][0] = 0;
          temp[index][1] = pfleft;
          temp[index][2] = 0;
          temp[index][3] = 0;
          sum = sum + pfleft;
          printf("\tfacing right: sum = %lf\n\n\n", sum);
        } else if (tl == mbr && tr == mbl && br == mtl && bl == mtr) {
          // facing down
          temp[index][0] = 0;
          temp[index][1] = 0;
          temp[index][2] = pftop;
          temp[index][3] = 0;
          sum = sum + pftop;
          printf("\tfacing down: sum = %lf\n\n\n", sum);
        } else if (tl == mbl && tr == mtl && br == mtr && bl == mbr) {
          // facing left
          temp[index][0] = 0;
          temp[index][1] = 0;
          temp[index][2] = 0;
          temp[index][3] = pfright;
          sum = sum + pfright;
          printf("\tfacing left: sum = %lf\n\n\n", sum);
        } else {
          // not matching
          temp[index][0] = 0;
          temp[index][1] = 0;
          temp[index][2] = 0;
          temp[index][3] = 0;
        }
      }
    }

    if (sum == 0) {
      fprintf(stderr, "sum for beliefs is 0, check it !!!!!!!!!!!!!!!!!!!!!\n");
      return(0);
    }

    for(int i=0; i<sx*sy; i++) {
      beliefs[i][0] = temp[i][0], beliefs[i][1] = temp[i][1], beliefs[i][2] = temp[i][2], beliefs[i][3] = temp[i][3];
      printf("at index %d, %f %f %f %f\n", i, beliefs[i][0] , beliefs[i][1] , beliefs[i][2] , beliefs[i][3]);
    }


    //normalize
    //double normal_sum = 0;
    for (int i = 0; i < sx; i++) {
      for (int j = 0; j < sy; j++) {
        int index = i+(j*sx);
        beliefs[index][0] = beliefs[index][0]/sum;
        beliefs[index][1] = beliefs[index][1]/sum;
        beliefs[index][2] = beliefs[index][2]/sum;
        beliefs[index][3] = beliefs[index][3]/sum;
        // if (i == (sx-1) && j == (sx-1)) {
        //   // make sure sum to 1, may not need this
        //   beliefs[index][3] = 1 - normal_sum - beliefs[index][0] - beliefs[index][1] - beliefs[index][2];
        // } else {
        //   normal_sum = normal_sum + beliefs[index][0] + beliefs[index][1] + beliefs[index][2] + beliefs[index][3];
        // }
        // check if determined location
        if (beliefs[index][0] == 1) {
          *robot_x = i;
          *robot_y = j;
          *direction = 0;
          return(1);
        } else if (beliefs[index][1] == 1) {
          *robot_x = i;
          *robot_y = j;
          *direction = 1;
          return(1);
        } else if (beliefs[index][2] == 1) {
          *robot_x = i;
          *robot_y = j;
          *direction = 2;
          return(1);
        } else if (beliefs[index][3] == 1) {
          *robot_x = i;
          *robot_y = j;
          *direction = 3;
          return(1);
        }
      }
    }

    // need more navigation to make decision
    // randomly choose a direction to try
    int rd = rand() % 3;
    if (rd == 1) {
      // turn right
      turn_at_intersection(0);
      for (int i = 0; i < sx; i++) {
        for (int j = 0; j < sy; j++) {
          int index = i+(j*sx);
          double bup = beliefs[index][0];
          double bright = beliefs[index][1];
          double bdown = beliefs[index][2];
          double bleft = beliefs[index][3];
          beliefs[index][0] = bleft;
          beliefs[index][1] = bup;
          beliefs[index][2] = bright;
          beliefs[index][3] = bdown;
        }
      }
      printf("\n\n\n~~~~~~~~~~~~~~ turned right ~~~~~~~~~~~~~~~~~~~~~~~\n\n\n");
    } else if (rd == 2) {
      // turn left
      turn_at_intersection(1);
      for (int i = 0; i < sx; i++) {
        for (int j = 0; j < sy; j++) {
          int index = i+(j*sx);
          double bup = beliefs[index][0];
          double bright = beliefs[index][1];
          double bdown = beliefs[index][2];
          double bleft = beliefs[index][3];
          beliefs[index][0] = bright;
          beliefs[index][1] = bdown;
          beliefs[index][2] = bleft;
          beliefs[index][3] = bup;
        }
      }
      printf("\n\n\n~~~~~~~~~~~~~~ turned left ~~~~~~~~~~~~~~~~~~~~~~~\n\n\n");
    }
  }

  fprintf(stderr, "end of while loop, check it !!!!!!!!!!!!!!!!!!!!\n");
  *(robot_x)=-1;
  *(robot_y)=-1;
  *(direction)=-1;
  return(0);
}

int go_to_target(int robot_x, int robot_y, int direction, int target_x, int target_y)
{
 /*
  * This function is called once localization has been successful, it performs the actions required to take the robot
  * from its current location to the specified target location. 
  *
  * You have to write the code required to carry out this task - once again, you can use the function headers provided, or
  * write your own code to control the bot, but document your process carefully in the comments below so your TA can easily
  * understand how everything works.
  *
  * Your code should be able to determine if the robot has gotten lost (or if localization was incorrect), and your bot
  * should be able to recover.
  * 
  * Inputs - The robot's current location x,y (the intersection coordinates, not image pixel coordinates)
  *          The target's intersection location
  * 
  * Return values: 1 if successful (the bot reached its target destination), 0 otherwise
  */   

  /************************************************************************************************************************
   *   TO DO  -   Complete this function
   ***********************************************************************************************************************/
  double xdiff = target_x - robot_x;
  double ydiff = target_y - robot_y;

  // xdirection
  if (xdiff > 0) {
    if (direction == 0) {
      if (turn_at_intersection(0)) {
        direction = 1;
      }
    } else if (direction == 2) {
      if (turn_at_intersection(1)) {
        direction = 1;
      }
    } else if (direction == 3) {
      while (direction > 1) {
        // check return value -------------------------!
        if (turn_at_intersection(1)) {
          direction = direction - 1;
        }
      }
    }
    while (xdiff > 0) {
      if (drive_along_street() == 1) {
        xdiff = xdiff - 1;
      }
    }
  } else if (xdiff < 0) {
    if (direction == 0) {
      if (turn_at_intersection(1)) {
        direction = 3;
      }
    } else if (direction == 2) {
      if (turn_at_intersection(0)) {
        direction = 3;
      }
    } else if (direction == 1) {
      while (direction < 3) {
        // check return value -------------------------!
        if (turn_at_intersection(0)) {
          direction = direction + 1;
        }
      }
    }
    while (xdiff < 0) {
      if (drive_along_street() == 1) {
        xdiff = xdiff + 1;
      }
    }
  }

  // ydirection
  if (ydiff > 0) {
    if (direction == 1) {
      if (turn_at_intersection(0)) {
        direction = 2;
      }
    } else if (direction == 3) {
      if (turn_at_intersection(1)) {
        direction = 2;
      }
    } else if (direction == 0) {
      while (direction < 2) {
        // check return value -------------------------!
        if (turn_at_intersection(0)) {
          direction = direction + 1;
        }
      }
    }
    while (ydiff > 0) {
      if (drive_along_street() == 1) {
        ydiff = ydiff - 1;
      }
    }
  } else if (ydiff < 0) {
    if (direction == 1) {
      if (turn_at_intersection(1)) {
        direction = 0;
      }
    } else if (direction == 3) {
      if (turn_at_intersection(0)) {
        direction = 0;
      }
    } else if (direction == 2) {
      while (direction > 0) {
        // check return value -------------------------!
        if (turn_at_intersection(1)) {
          direction = direction - 1;
        }
      }
    }
    while (ydiff < 0) {
      if (drive_along_street() == 1) {
        ydiff = ydiff + 1;
      }
    }
  }

  // scan again to check final location
  int tl, tr, br, bl;
  scan_intersection(&tl, &tr, &br, &bl);
  int index = target_x+(target_y*sx);
  int mtl = map[index][0];
  int mtr = map[index][1];
  int mbr = map[index][2];
  int mbl = map[index][3];
  if (direction == 0 && tl == mtl && tr == mtr && br == mbr && bl == mbl) {
    fprintf(stderr, "In go_to_target, succeed!!!!!!\n");
    return(1);
  }
  if (direction == 1 && tl == mtr && tr == mbr && br == mbl && bl == mtl) {
    fprintf(stderr, "In go_to_target, succeed!!!!!!\n");
    return(1);
  }
  if (direction == 2 && tl == mbr && tr == mbl && br == mtl && bl == mtr) {
    fprintf(stderr, "In go_to_target, succeed!!!!!!\n");
    return(1);
  }
  if (direction == 3 && tl == mbl && tr == mtl && br == mtl && bl == mbr) {
    fprintf(stderr, "In go_to_target, succeed!!!!!!\n");
    return(1);
  }

  return(0);
}

void calibrate_sensor(void)
{
 /*
  * This function is called when the program is started with -1  -1 for the target location. 
  *
  * You DO NOT NEED TO IMPLEMENT ANYTHING HERE - but it is strongly recommended as good calibration will make sensor
  * readings more reliable and will make your code more resistent to changes in illumination, map quality, or battery
  * level.
  * 
  * The principle is - Your code should allow you to sample the different colours in the map, and store representative
  * values that will help you figure out what colours the sensor is reading given the current conditions.
  * 
  * Inputs - None
  * Return values - None - your code has to save the calibration information to a file, for later use (see in main())
  * 
  * How to do this part is up to you, but feel free to talk with your TA and instructor about it!
  */   

  /************************************************************************************************************************
   *   OIPTIONAL TO DO  -   Complete this function
   ***********************************************************************************************************************/
  fprintf(stderr,"Calibration function called!\n");

  // Open a socket to the EV3 for remote controlling the bot.
  if (BT_open(HEXKEY)!=0) {
    fprintf(stderr,"Unable to open comm socket to the EV3, make sure the EV3 kit is powered on, and that the\n");
    fprintf(stderr," hex key for the EV3 matches the one in EV3_Localization.h\n");
    exit(1);
  }

  // Open color value file
  FILE *fp;
  fp = fopen(COLOR_FILE, "w");
  fprintf(stderr,"Start calibration!\n");
  char colors[6][10] = {"black", "white", "red", "yellow", "blue", "green"};
  for(int i=0;i<6;i++) {
    fprintf(stderr,"Now read %s, move robot and press any key to start\n", colors[i]);
    char holder;
    fscanf(stdin, "%c", &holder);
    int rgb[CALI_N][3];
    for(int T=0;T<CALI_N;T++) {
      BT_read_colour_sensor_RGB(PORT_4, rgb[T]);
    }
    double average[3], diff[3];
    avg(rgb, average);
    //fprintf(stderr, "Average value for %s: %.10lf %.10lf %.10lf\n", colors[i], average[0], average[1], average[2]);
    calcuDiff(rgb, average, diff);
    // Print average to file
    fprintf(fp, "%.10lf %.10lf %.10lf\n", average[0], average[1], average[2]);
    // Print diff range to file
    fprintf(fp, "%.10lf %.10lf %.10lf\n", diff[0], diff[1], diff[2]);
  }
  fclose(fp);
  BT_close();
}

void read_calibration() {
  FILE *fp;
  fp = fopen(COLOR_FILE, "r");
  fscanf(fp, "%lf%lf%lf", &black.avg[0], &black.avg[1], &black.avg[2]);
  fscanf(fp, "%lf%lf%lf", &black.diffRange[0], &black.diffRange[1], &black.diffRange[2]);
  fscanf(fp, "%lf%lf%lf", &white.avg[0], &white.avg[1], &white.avg[2]);
  fscanf(fp, "%lf%lf%lf", &white.diffRange[0], &white.diffRange[1], &white.diffRange[2]);
  fscanf(fp, "%lf%lf%lf", &red.avg[0], &red.avg[1], &red.avg[2]);
  fscanf(fp, "%lf%lf%lf", &red.diffRange[0], &red.diffRange[1], &red.diffRange[2]);
  fscanf(fp, "%lf%lf%lf", &yellow.avg[0], &yellow.avg[1], &yellow.avg[2]);
  fscanf(fp, "%lf%lf%lf", &yellow.diffRange[0], &yellow.diffRange[1], &yellow.diffRange[2]);
  fscanf(fp, "%lf%lf%lf", &blue.avg[0], &blue.avg[1], &blue.avg[2]);
  fscanf(fp, "%lf%lf%lf", &blue.diffRange[0], &blue.diffRange[1], &blue.diffRange[2]);
  fscanf(fp, "%lf%lf%lf", &green.avg[0], &green.avg[1], &green.avg[2]);
  fscanf(fp, "%lf%lf%lf", &green.diffRange[0], &green.diffRange[1], &green.diffRange[2]);
  fprintf(stderr, "Finish reading calibration\n");
}

void calcuDiff(int a[CALI_N][3], double average[3], double ans[3]) {
  double r=0, g=0, b=0;
  //int cnt = 0;
  for(int i=0;i<CALI_N;i++) {
    if(a[i][0]>=0 && a[i][0]< 256 && a[i][1] >= 0 && a[i][1] < 256 && a[i][2] >= 0 && a[i][2] < 256) {
      /*cnt++;
      r+=fabs(a[i][0]-average[0]);
      g+=fabs(a[i][1]-average[1]);
      b+=fabs(a[i][2]-average[2]);*/
      r = r > fabs(a[i][0]-average[0]) ? r : fabs(a[i][0]-average[0]);
      g = g > fabs(a[i][1]-average[1]) ? g : fabs(a[i][1]-average[1]);
      b = b > fabs(a[i][2]-average[2]) ? b : fabs(a[i][2]-average[2]);
    }
  }
  /*if(cnt == 0) {
    fprintf(stderr, "No valid data from color sensor\n");
    exit(1);
  }*/
  // ans[0] = r/cnt;
  // ans[1] = g/cnt;
  // ans[2] = b/cnt;
  ans[0] = r;
  ans[1] = g;
  ans[2] = b;
  return;
}

void avg(int a[CALI_N][3], double ans[3]) {
  double r=0, g=0, b=0;
  int cnt = 0;
  for(int i=0;i<CALI_N;i++) {
    if(a[i][0]>=0 && a[i][0]< 256 && a[i][1] >= 0 && a[i][1] < 256 && a[i][2] >= 0 && a[i][2] < 256) {
      cnt++;
      r+=a[i][0];
      g+=a[i][1];
      b+=a[i][2];
    }
  }
  if(cnt == 0) {
    fprintf(stderr, "No valid data from color sensor\n");
    exit(1);
  }
  ans[0] = r/cnt;
  ans[1] = g/cnt;
  ans[2] = b/cnt;
  return;
}

int isColor(struct ColorInfo *c, int rgb[3]) {
  if(rgb[0]>=0 && rgb[0]< 256 && rgb[1] >= 0 && rgb[1] < 256 && rgb[2] >= 0 && rgb[2] < 256) {
    double r = c->avg[0] / rgb[0], g = c->avg[1] / rgb[1], b = c->avg[2] / rgb[2];
    double h = 3 / (1/r + 1/g + 1/b), a = (r+g+b)/3;
    double similation = h/a;
    //fprintf(stderr, "\t\tSimilarity: %.10lf\n", similation);
    return (similation >= 0.9) ? 1 : 0;
  } else {
    return 0;
  }
}

int determine_color(int rgb[3]) {
  if(rgb[0]>=0 && rgb[0]< 256 && rgb[1] >= 0 && rgb[1] < 256 && rgb[2] >= 0 && rgb[2] < 256) {
    if(isColor(&black, rgb) && fabs(rgb[0]-black.avg[0]) <= 10 && fabs(rgb[1]- black.avg[1]) <= 10 && fabs(rgb[2]-black.avg[2]) <= 10) {fprintf(stderr, "\t\tIs Black\n"); return BLACKCOLOR;}
    else if(isColor(&white, rgb) && fabs(rgb[0]-white.avg[0]) <= 10 && fabs(rgb[1]- white.avg[1]) <= 10 && fabs(rgb[2]-white.avg[2]) <= 10) {fprintf(stderr, "\t\tIs White\n"); return WHITECOLOR;}
    else if(isColor(&red, rgb)) {fprintf(stderr, "\t\tIs Red\n"); return REDCOLOR;}
    else if(isColor(&yellow, rgb)) {fprintf(stderr, "\t\tIs Yellow\n"); return YELLOWCOLOR;}
    else if(isColor(&green, rgb)) {fprintf(stderr, "\t\tIs Green\n"); return GREENCOLOR;}
    else if(isColor(&blue, rgb)) {fprintf(stderr, "\t\tIs Blue\n"); return BLUECOLOR;}
    else if (isColor(&black, rgb) && fabs(rgb[0]-black.avg[0]) > 10 && fabs(rgb[1]- black.avg[1]) > 10 && fabs(rgb[2]-black.avg[2]) > 10) {fprintf(stderr, "\t\tIs White\n"); return WHITECOLOR;}
    else return 0;
  } else {
    return -1;
  }
  
}

int robust_color(int rgb[3]) {
  if(BT_read_colour_sensor_RGB(PORT_4, rgb) == 0) {
    if(rgb[0]>=0 && rgb[0]< 256 && rgb[1] >= 0 && rgb[1] < 256 && rgb[2] >= 0 && rgb[2] < 256) {
      return 0;
    } else {
      return -1;
    }
  } else {
    // Color sensor failed
    fprintf(stderr,"BT_colour_sensor_RGB(): Command failed\n");
    BT_all_stop(0);
    exit(1);
  }
}

void play_succeed_sound() {
  int tone_data[50][3];
  // Reset tone data information
  for (int i=0;i<50; i++) 
  {
    tone_data[i][0]=-1;
    tone_data[i][1]=-1;
    tone_data[i][2]=-1;
  }

  //G-F#-Eb-A-G#-E-G#-C
  tone_data[0][0]=784;
  tone_data[0][1]=200;
  tone_data[0][2]=50;
  tone_data[1][0]=740;
  tone_data[1][1]=200;
  tone_data[1][2]=25;
  tone_data[2][0]=622.25;
  tone_data[2][1]=200;
  tone_data[2][2]=25;
  tone_data[3][0]=440;
  tone_data[3][1]=200;
  tone_data[3][2]=50;
  tone_data[4][0]=415.3;
  tone_data[4][1]=200;
  tone_data[4][2]=25;
  tone_data[5][0]=659.26;
  tone_data[5][1]=200;
  tone_data[5][2]=50;
  tone_data[6][0]=830.61;
  tone_data[6][1]=200;
  tone_data[6][2]=25;
  tone_data[7][0]=1046.5;
  tone_data[7][1]=200;
  tone_data[7][2]=50;

  BT_play_tone_sequence(tone_data);
}

void play_located_sound() {
  int tone_data[50][3];
  // Reset tone data information
  for (int i=0;i<50; i++) 
  {
    tone_data[i][0]=-1;
    tone_data[i][1]=-1;
    tone_data[i][2]=-1;
  }

  //D-E-F-G-C-F#-G
  tone_data[0][0]=1109;
  tone_data[0][1]=150;
  tone_data[0][2]=50;
  tone_data[1][0]=1318.5;
  tone_data[1][1]=150;
  tone_data[1][2]=50;
  tone_data[2][0]=1397;
  tone_data[2][1]=150;
  tone_data[2][2]=50;
  tone_data[3][0]=1568;
  tone_data[3][1]=150;
  tone_data[3][2]=50;
  tone_data[4][0]=2093;
  tone_data[4][1]=150;
  tone_data[4][2]=50;
  tone_data[5][0]=1480;
  tone_data[5][1]=25;
  tone_data[5][2]=50;
  tone_data[6][0]=1568;
  tone_data[6][1]=150;
  tone_data[6][2]=50;

  BT_play_tone_sequence(tone_data);
}

void play_restarted_sound() {
  int tone_data[50][3];
  // Reset tone data information
  for (int i=0;i<50; i++) 
  {
    tone_data[i][0]=-1;
    tone_data[i][1]=-1;
    tone_data[i][2]=-1;
  }

  //E-G-E-C-D-G
  tone_data[0][0]=1318.5;
  tone_data[0][1]=150;
  tone_data[0][2]=50;
  tone_data[1][0]=1568;
  tone_data[1][1]=150;
  tone_data[1][2]=50;
  tone_data[2][0]=2637;
  tone_data[2][1]=150;
  tone_data[2][2]=50;
  tone_data[3][0]=2093;
  tone_data[3][1]=150;
  tone_data[3][2]=50;
  tone_data[4][0]=2349.3;
  tone_data[4][1]=150;
  tone_data[4][2]=50;
  tone_data[5][0]=3136;
  tone_data[5][1]=150;
  tone_data[5][2]=50;

  BT_play_tone_sequence(tone_data);
}

int parse_map(unsigned char *map_img, int rx, int ry)
{
 /*
   This function takes an input image map array, and two integers that specify the image size.
   It attempts to parse this image into a representation of the map in the image. The size
   and resolution of the map image should not affect the parsing (i.e. you can make your own
   maps without worrying about the exact position of intersections, roads, buildings, etc.).

   However, this function requires:
   
   * White background for the image  [255 255 255]
   * Red borders around the map  [255 0 0]
   * Black roads  [0 0 0]
   * Yellow intersections  [255 255 0]
   * Buildings that are pure green [0 255 0], pure blue [0 0 255], or white [255 255 255]
   (any other colour values are ignored - so you can add markings if you like, those 
    will not affect parsing)

   The image must be a properly formated .ppm image, see readPPMimage below for details of
   the format. The GIMP image editor saves properly formatted .ppm images, as does the
   imagemagick image processing suite.
   
   The map representation is read into the map array, with each row in the array corrsponding
   to one intersection, in raster order, that is, for a map with k intersections along its width:
   
    (row index for the intersection)
    
    0     1     2    3 ......   k-1
    
    k    k+1   k+2  ........    
    
    Each row will then contain the colour values for buildings around the intersection 
    clockwise from top-left, that is
    
    
    top-left               top-right
            
            intersection
    
    bottom-left           bottom-right
    
    So, for the first intersection (at row 0 in the map array)
    map[0][0] <---- colour for the top-left building
    map[0][1] <---- colour for the top-right building
    map[0][2] <---- colour for the bottom-right building
    map[0][3] <---- colour for the bottom-left building
    
    Color values for map locations are defined as follows (this agrees with what the
    EV3 sensor returns in indexed-colour-reading mode):
    
    1 -  Black
    2 -  Blue
    3 -  Green
    4 -  Yellow
    5 -  Red
    6 -  White
    
    If you find a 0, that means you're trying to access an intersection that is not on the
    map! Also note that in practice, because of how the map is defined, you should find
    only Green, Blue, or White around a given intersection.
    
    The map size (the number of intersections along the horizontal and vertical directions) is
    updated and left in the global variables sx and sy.

    Feel free to create your own maps for testing (you'll have to print them to a reasonable
    size to use with your bot).
    
 */    
 
  int last3[3];
  int x,y;
  unsigned char R,G,B;
  int ix,iy;
  int bx,by,dx,dy,wx,wy;         // Intersection geometry parameters
  int tgl;
  int idx;
  
  ix=iy=0;       // Index to identify the current intersection
  
  // Determine the spacing and size of intersections in the map
  tgl=0;
  for (int i=0; i<rx; i++)
  {
    for (int j=0; j<ry; j++)
    {
    R=*(map_img+((i+(j*rx))*3));
    G=*(map_img+((i+(j*rx))*3)+1);
    B=*(map_img+((i+(j*rx))*3)+2);
    if (R==255&&G==255&&B==0)
    {
      // First intersection, top-left pixel. Scan right to find width and spacing
      bx=i;           // Anchor for intersection locations
      by=j;
      for (int k=i; k<rx; k++)        // Find width and horizontal distance to next intersection
      {
      R=*(map_img+((k+(by*rx))*3));
      G=*(map_img+((k+(by*rx))*3)+1);
      B=*(map_img+((k+(by*rx))*3)+2);
      if (tgl==0&&(R!=255||G!=255||B!=0))
      {
        tgl=1;
        wx=k-i;
      }
      if (tgl==1&&R==255&&G==255&&B==0)
      {
        tgl=2;
        dx=k-i;
      }
      }
      for (int k=j; k<ry; k++)        // Find height and vertical distance to next intersection
      {
      R=*(map_img+((bx+(k*rx))*3));
      G=*(map_img+((bx+(k*rx))*3)+1);
      B=*(map_img+((bx+(k*rx))*3)+2);
      if (tgl==2&&(R!=255||G!=255||B!=0))
      {
        tgl=3;
        wy=k-j;
      }
      if (tgl==3&&R==255&&G==255&&B==0)
      {
        tgl=4;
        dy=k-j;
      }
      }
      
      if (tgl!=4)
      {
      fprintf(stderr,"Unable to determine intersection geometry!\n");
      return(0);
      }
      else break;
    }
    }
    if (tgl==4) break;
  }
    fprintf(stderr,"Intersection parameters: base_x=%d, base_y=%d, width=%d, height=%d, horiz_distance=%d, vertical_distance=%d\n",bx,by,wx,wy,dx,dy);

    sx=0;
    for (int i=bx+(wx/2);i<rx;i+=dx)
    {
    R=*(map_img+((i+(by*rx))*3));
    G=*(map_img+((i+(by*rx))*3)+1);
    B=*(map_img+((i+(by*rx))*3)+2);
    if (R==255&&G==255&&B==0) sx++;
    }

    sy=0;
    for (int j=by+(wy/2);j<ry;j+=dy)
    {
    R=*(map_img+((bx+(j*rx))*3));
    G=*(map_img+((bx+(j*rx))*3)+1);
    B=*(map_img+((bx+(j*rx))*3)+2);
    if (R==255&&G==255&&B==0) sy++;
    }
    
    fprintf(stderr,"Map size: Number of horizontal intersections=%d, number of vertical intersections=%d\n",sx,sy);

    // Scan for building colours around each intersection
    idx=0;
    for (int j=0; j<sy; j++)
    for (int i=0; i<sx; i++)
    {
      x=bx+(i*dx)+(wx/2);
      y=by+(j*dy)+(wy/2);
      
      fprintf(stderr,"Intersection location: %d, %d\n",x,y);
      // Top-left
      x-=wx;
      y-=wy;
      R=*(map_img+((x+(y*rx))*3));
      G=*(map_img+((x+(y*rx))*3)+1);
      B=*(map_img+((x+(y*rx))*3)+2);
      if (R==0&&G==255&&B==0) map[idx][0]=3;
      else if (R==0&&G==0&&B==255) map[idx][0]=2;
      else if (R==255&&G==255&&B==255) map[idx][0]=6;
      else fprintf(stderr,"Colour is not valid for intersection %d,%d, Top-Left RGB=%d,%d,%d\n",i,j,R,G,B);

      // Top-right
      x+=2*wx;
      R=*(map_img+((x+(y*rx))*3));
      G=*(map_img+((x+(y*rx))*3)+1);
      B=*(map_img+((x+(y*rx))*3)+2);
      if (R==0&&G==255&&B==0) map[idx][1]=3;
      else if (R==0&&G==0&&B==255) map[idx][1]=2;
      else if (R==255&&G==255&&B==255) map[idx][1]=6;
      else fprintf(stderr,"Colour is not valid for intersection %d,%d, Top-Right RGB=%d,%d,%d\n",i,j,R,G,B);

      // Bottom-right
      y+=2*wy;
      R=*(map_img+((x+(y*rx))*3));
      G=*(map_img+((x+(y*rx))*3)+1);
      B=*(map_img+((x+(y*rx))*3)+2);
      if (R==0&&G==255&&B==0) map[idx][2]=3;
      else if (R==0&&G==0&&B==255) map[idx][2]=2;
      else if (R==255&&G==255&&B==255) map[idx][2]=6;
      else fprintf(stderr,"Colour is not valid for intersection %d,%d, Bottom-Right RGB=%d,%d,%d\n",i,j,R,G,B);
      
      // Bottom-left
      x-=2*wx;
      R=*(map_img+((x+(y*rx))*3));
      G=*(map_img+((x+(y*rx))*3)+1);
      B=*(map_img+((x+(y*rx))*3)+2);
      if (R==0&&G==255&&B==0) map[idx][3]=3;
      else if (R==0&&G==0&&B==255) map[idx][3]=2;
      else if (R==255&&G==255&&B==255) map[idx][3]=6;
      else fprintf(stderr,"Colour is not valid for intersection %d,%d, Bottom-Left RGB=%d,%d,%d\n",i,j,R,G,B);
      
      fprintf(stderr,"Colours for this intersection: %d, %d, %d, %d\n",map[idx][0],map[idx][1],map[idx][2],map[idx][3]);
      
      idx++;
    }

  return(1);  
}

unsigned char *readPPMimage(const char *filename, int *rx, int *ry)
{
 // Reads an image from a .ppm file. A .ppm file is a very simple image representation
 // format with a text header followed by the binary RGB data at 24bits per pixel.
 // The header has the following form:
 //
 // P6
 // # One or more comment lines preceded by '#'
 // 340 200
 // 255
 //
 // The first line 'P6' is the .ppm format identifier, this is followed by one or more
 // lines with comments, typically used to inidicate which program generated the
 // .ppm file.
 // After the comments, a line with two integer values specifies the image resolution
 // as number of pixels in x and number of pixels in y.
 // The final line of the header stores the maximum value for pixels in the image,
 // usually 255.
 // After this last header line, binary data stores the RGB values for each pixel
 // in row-major order. Each pixel requires 3 bytes ordered R, G, and B.
 //
 // NOTE: Windows file handling is rather crotchetty. You may have to change the
 //       way this file is accessed if the images are being corrupted on read
 //       on Windows.
 //

  FILE *f;
  unsigned char *im;
  char line[1024];
  int i;
  unsigned char *tmp;
  double *fRGB;

  im=NULL;
  f=fopen(filename,"rb+");
  if (f==NULL)
  {
    fprintf(stderr,"Unable to open file %s for reading, please check name and path\n",filename);
    return(NULL);
  }
  fgets(&line[0],1000,f);
  if (strcmp(&line[0],"P6\n")!=0)
  {
    fprintf(stderr,"Wrong file format, not a .ppm file or header end-of-line characters missing\n");
    fclose(f);
    return(NULL);
  }
  fprintf(stderr,"%s\n",line);
  // Skip over comments
  fgets(&line[0],511,f);
  while (line[0]=='#')
  {
    fprintf(stderr,"%s",line);
    fgets(&line[0],511,f);
  }
  sscanf(&line[0],"%d %d\n",rx,ry);                  // Read image size
  fprintf(stderr,"nx=%d, ny=%d\n\n",*rx,*ry);

  fgets(&line[0],9,f);  	                // Read the remaining header line
  fprintf(stderr,"%s\n",line);
  im=(unsigned char *)calloc((*rx)*(*ry)*3,sizeof(unsigned char));
  if (im==NULL)
  {
    fprintf(stderr,"Out of memory allocating space for image\n");
    fclose(f);
    return(NULL);
  }
  fread(im,(*rx)*(*ry)*3*sizeof(unsigned char),1,f);
  fclose(f);

  return(im);    
}