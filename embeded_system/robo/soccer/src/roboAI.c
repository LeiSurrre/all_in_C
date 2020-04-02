/**************************************************************************
  CSC C85 - UTSC RoboSoccer AI core

  This file is where the actual planning is done and commands are sent
  to the robot.

  Please read all comments in this file, and add code where needed to
  implement your game playing logic. 

  Things to consider:

  - Plan - don't just react
  - Use the heading vectors!
  - Mind the noise (it's everywhere)
  - Try to predict what your oponent will do
  - Use feedback from the camera

  What your code should not do: 

  - Attack the opponent, or otherwise behave aggressively toward the
    oponent
  - Hog the ball (you can kick it, push it, or leave it alone)
  - Sit at the goal-line or inside the goal
  - Run completely out of bounds

  AI scaffold: Parker-Lee-Estrada, Summer 2013

  EV3 Version 1.1 - Updated Aug 2019 - F. Estrada
***************************************************************************/

#include "roboAI.h"			// <--- Look at this header file!

double dottie(double vx, double vy, double ux, double uy) {
  // Returns the dot product of the two vectors [vx,vy] and [ux,uy]
  return (vx*ux)+(vy*uy);
}

double crossie_sign(double vx, double vy, double ux, double uy) {
  // Returns the sign of the Z component of the cross product of 
  //   vectors [vx vy 0] and [ux uy 0]
  // MIND THE ORDER! v rotating onto u.     
  // i   j   k 
  // vx  vy  0
  // ux  uy  0
      
  if ((vx*uy)-(ux*vy)<0) return -1;
  else return 1;
}

/**************************************************************
 * Display List Management - EXPERIMENTAL (you don't need this
 * to solve your RoboSoccer project). The Display List is an
 * experimental feature to allow you to draw things on the
 * display window. However, it's not (yet) fully tested and 
 * you can easily get into a segfault.
 * ** USE AT YOUR OWN RISK **
 * ***********************************************************/
struct displayList *addPoint(struct displayList *head, int x, int y, double R, double G, double B) {
  struct displayList *newNode;
  newNode=(struct displayList *) calloc(1,sizeof(struct displayList));
  if (newNode==NULL) {
    fprintf(stderr,"addPoint(): Out of memory!\n");
    return head;
  }
  newNode->type=0;
  newNode->x1=x;
  newNode->y1=y;
  newNode->x2=-1;
  newNode->y2=-1;
  newNode->R=R;
  newNode->G=G;
  newNode->B=B;
  
  newNode->next=head;
  return(newNode);
}

struct displayList *addLine(struct displayList *head, int x1, int y1, int x2, int y2, double R, double G, double B) {
  struct displayList *newNode;
  newNode=(struct displayList *)calloc(1,sizeof(struct displayList));
  if (newNode==NULL) {
    fprintf(stderr,"addLine(): Out of memory!\n");
    return head;
  }
  newNode->type=1;
  newNode->x1=x1;
  newNode->y1=y1;
  newNode->x2=x2;
  newNode->y2=y2;
  newNode->R=R;
  newNode->G=G;
  newNode->B=B;
  newNode->next=head;
  return(newNode);  
}

struct displayList *addVector(struct displayList *head, int x1, int y1, double dx, double dy, int length, double R, double G, double B)
{
  struct displayList *newNode;
  double l;
  
  l=sqrt((dx*dx)+(dy*dy));
  dx=dx/l;
  dy=dy/l;
  
  newNode=(struct displayList *)calloc(1,sizeof(struct displayList));
  if (newNode==NULL) {
    fprintf(stderr,"addVector(): Out of memory!\n");
    return head;
  }
  newNode->type=1;
  newNode->x1=x1;
  newNode->y1=y1;
  newNode->x2=x1+(length*dx);
  newNode->y2=y1+(length*dy);
  newNode->R=R;
  newNode->G=G;
  newNode->B=B;
  newNode->next=head;
  return(newNode);
}

struct displayList *addCross(struct displayList *head, int x, int y, int length, double R, double G, double B) {
  struct displayList *newNode;
  newNode=(struct displayList *)calloc(1,sizeof(struct displayList));
  if (newNode==NULL) {
    fprintf(stderr,"addLine(): Out of memory!\n");
    return head;
  }
  newNode->type=1;
  newNode->x1=x-length;
  newNode->y1=y;
  newNode->x2=x+length;
  newNode->y2=y;
  newNode->R=R;
  newNode->G=G;
  newNode->B=B;
  newNode->next=head;
  head=newNode;

  newNode=(struct displayList *)calloc(1,sizeof(struct displayList));
  if (newNode==NULL) {
    fprintf(stderr,"addLine(): Out of memory!\n");
    return head;
  }
  newNode->type=1;
  newNode->x1=x;
  newNode->y1=y-length;
  newNode->x2=x;
  newNode->y2=y+length;
  newNode->R=R;
  newNode->G=G;
  newNode->B=B;
  newNode->next=head;
  return(newNode);
}

struct displayList *clearDP(struct displayList *head) {
  struct displayList *q;
  while(head) {
      q=head->next;
      free(head);
      head=q;
  }
  return(NULL);
}

/**************************************************************
 * End of Display List Management
 * ***********************************************************/

/*************************************************************
 * Blob identification and tracking
 * ***********************************************************/

struct blob *id_coloured_blob2(struct RoboAI *ai, struct blob *blobs, int col) {
  /////////////////////////////////////////////////////////////////////////////
  // This function looks for and identifies a blob with the specified colour.
  // It uses the hue and saturation values computed for each blob and tries to
  // select the blob that is most like the expected colour (red, green, or blue)
  //
  // If you find that tracking of blobs is not working as well as you'd like,
  // you can try to improve the matching criteria used in this function.
  // Remember you also have access to shape data and orientation axes for blobs.
  //
  // Inputs: The robot's AI data structure, a list of blobs, and a colour target:
  // Colour parameter: 0 -> Red
  //                   1 -> Yellow or Green (see below)
  //                   2 -> Blue
  // Returns: Pointer to the blob with the desired colour, or NULL if no such
  // 	     blob can be found.
  /////////////////////////////////////////////////////////////////////////////

  struct blob *p, *fnd;
  double vr_x,vr_y,maxfit,mincos,dp;
  double vb_x,vb_y,fit;
  double maxsize=0;
  double maxgray;
  int grayness;
  int i;
  
  maxfit=.025;                                             // Minimum fitness threshold
  mincos=.65;                                              // Threshold on colour angle similarity
  maxgray=.25;                                             // Maximum allowed difference in colour
                                                            // to be considered gray-ish (as a percentage
                                                            // of intensity)

  // The reference colours here are in the HSV colourspace, we look at the hue component, which is a
  // defined within a colour-wheel that contains all possible colours. Hence, the hue component
  // is a value in [0 360] degrees, or [0 2*pi] radians, indicating the colour's location on the
  // colour wheel. If we want to detect a different colour, all we need to do is figure out its
  // location in the colour wheel and then set the angles below (in radians) to that colour's
  // angle within the wheel.
  // For reference: Red is at 0 degrees, Yellow is at 60 degrees, Green is at 120, and Blue at 240.

    // Agent IDs are as follows: 0 : Own bot,  1 : Opponent's bot, 2 : Ball
  if (col==0) { // RED  
    vr_x=cos(0); vr_y=sin(0);
  }                                                 
  else if (col==2) { // YELLOW
    vr_x=cos(2.0*PI*(60.0/360.0));
    vr_y=sin(2.0*PI*(60.0/360.0));
  } else if (col==1) { // BLUE
    vr_x=cos(2.0*PI*(240.0/360.0));
    vr_y=sin(2.0*PI*(240.0/360.0));
  }         

  // In what follows, colours are represented by a unit-length vector in the direction of the
  // hue for that colour. Similarity between two colours (e.g. a reference above, and a pixel's
  // or blob's colour) is measured as the dot-product between the corresponding colour vectors.
  // If the dot product is 1 the colours are identical (their vectors perfectly aligned), 
  // from there, the dot product decreases as the colour vectors start to point in different
  // directions. Two colours that are opposite will result in a dot product of -1.
  
  p=blobs;
  while (p!=NULL) { 
    if (p->size>maxsize) maxsize=p->size;
    p=p->next;
  }

  p=blobs;
  fnd=NULL;
  while (p!=NULL) {
    // Normalization and range extension
    vb_x=cos(p->H);
    vb_y=sin(p->H);

    dp=(vb_x*vr_x)+(vb_y*vr_y);                                       // Dot product between the reference color vector, and the
                                                                      // blob's color vector.

    fit=dp*p->S*p->S*(p->size/maxsize);                               // <<< --- This is the critical matching criterion.
                                                                      // * THe dot product with the reference direction,
                                                                      // * Saturation squared
                                                                      // * And blob size (in pixels, not from bounding box)
                                                                      // You can try to fine tune this if you feel you can
                                                                      // improve tracking stability by changing this fitness
                                                                      // computation

    // Check for a gray-ish blob - they tend to give trouble
    grayness=0;
    if (fabs(p->R-p->G)/p->R<maxgray&&fabs(p->R-p->G)/p->G<maxgray&&fabs(p->R-p->B)/p->R<maxgray&&fabs(p->R-p->B)/p->B<maxgray&&\
        fabs(p->G-p->B)/p->G<maxgray&&fabs(p->G-p->B)/p->B<maxgray) grayness=1;
    
    if (fit>maxfit&&dp>mincos&&grayness==0) {
      fnd=p;
      maxfit=fit;
    }
    p=p->next;
  }

  return(fnd);
}

void track_agents(struct RoboAI *ai, struct blob *blobs) {
  ////////////////////////////////////////////////////////////////////////
  // This function does the tracking of each agent in the field. It looks
  // for blobs that represent the bot, the ball, and our opponent (which
  // colour is assigned to each bot is determined by a command line
  // parameter).
  // It keeps track within the robot's AI data structure of multiple 
  // parameters related to each agent:
  // - Position
  // - Velocity vector. Not valid while rotating, but possibly valid
  //   while turning.
  // - Motion direction vector. Not valid
  //   while rotating - possibly valid while turning
  // - Heading direction - vector obtained from the blob shape, it is
  //   correct up to a factor of (-1) (i.e. it may point backward w.r.t.
  //   the direction your bot is facing). This vector remains valid
  //   under rotation.
  // - Pointers to the blob data structure for each agent
  //
  // This function will update the blob data structure with the velocity
  // and heading information from tracking. 
  //
  // NOTE: If a particular agent is not found, the corresponding field in
  //       the AI data structure (ai->st.ball, ai->st.self, ai->st.opp)
  //       will remain NULL. Make sure you check for this before you 
  //       try to access an agent's blob data! 
  //
  // In addition to this, if calibration data is available then this
  // function adjusts the Y location of the bot and the opponent to 
  // adjust for perspective projection error. See the handout on how
  // to perform the calibration process.
  //
  // This function receives a pointer to the robot's AI data structure,
  // and a list of blobs.
  //
  // You can change this function if you feel the tracking is not stable.
  // First, though, be sure to completely understand what it's doing.
  /////////////////////////////////////////////////////////////////////////

  struct blob *p;
  double mg,vx,vy,pink,doff,dmin,dmax,adj;

  // Reset ID flags
  ai->st.ballID=0;
  ai->st.selfID=0;
  ai->st.oppID=0;
  ai->st.ball=NULL;			// Be sure you check these are not NULL before
  ai->st.self=NULL;			// trying to access data for the ball/self/opponent!
  ai->st.opp=NULL;

  // Find the ball
  p=id_coloured_blob2(ai,blobs,2);       // With 2, Ball is set to be yellow
  if (p) {
    ai->st.ball=p;			// New pointer to ball
    ai->st.ballID=1;			// Set ID flag for ball (we found it!)
    ai->st.bvx=p->cx-ai->st.old_bcx;	// Update ball velocity in ai structure and blob structure
    ai->st.bvy=p->cy-ai->st.old_bcy;
    ai->st.ball->vx=ai->st.bvx;
    ai->st.ball->vy=ai->st.bvy;
    ai->st.bdx=p->dx;
    ai->st.bdy=p->dy;

    ai->st.old_bcx=p->cx; 		// Update old position for next frame's computation
    ai->st.old_bcy=p->cy;
    ai->st.ball->idtype=3;

    vx=ai->st.bvx;			// Compute motion direction (normalized motion vector)
    vy=ai->st.bvy;
    mg=sqrt((vx*vx)+(vy*vy));
    if (mg>NOISE_VAR) {			// Update heading vector if meaningful motion detected
      vx/=mg;
      vy/=mg;
      ai->st.bmx=vx;
      ai->st.bmy=vy;
    } else {
      ai->st.bmx=0;
      ai->st.bmy=0;
    }
    ai->st.ball->mx=ai->st.bmx;
    ai->st.ball->my=ai->st.bmy;
  } else {
    ai->st.ball=NULL;
  }
  
  // ID our bot
  if (ai->st.botCol==0) p=id_coloured_blob2(ai,blobs,1);     // If 0, our bot is BLUE, if 1 our bot is RED
  else p=id_coloured_blob2(ai,blobs,0);
  if (p!=NULL&&p!=ai->st.ball) {
    ai->st.self=p;			// Update pointer to self-blob

    // Adjust Y position if we have calibration data
    if (fabs(p->adj_Y[0][0])>.1) {
      dmax=384.0-p->adj_Y[0][0];
      dmin=767.0-p->adj_Y[1][0];
      pink=(dmax-dmin)/(768.0-384.0);
      adj=dmin+((p->adj_Y[1][0]-p->cy)*pink);
      p->cy=p->cy+adj;
      if (p->cy>767) p->cy=767;
      if (p->cy<1) p->cy=1;
    }

    ai->st.selfID=1;
    ai->st.svx=p->cx-ai->st.old_scx;
    ai->st.svy=p->cy-ai->st.old_scy;
    ai->st.self->vx=ai->st.svx;
    ai->st.self->vy=ai->st.svy;
    ai->st.sdx=p->dx;
    ai->st.sdy=p->dy;

    vx=ai->st.svx;
    vy=ai->st.svy;
    mg=sqrt((vx*vx)+(vy*vy));

    if (mg>NOISE_VAR) {
      vx/=mg;
      vy/=mg;
      ai->st.smx=vx;
      ai->st.smy=vy;
    } else {
      ai->st.smx=0;
      ai->st.smy=0;
    }
    ai->st.self->mx=ai->st.smx;
    ai->st.self->my=ai->st.smy;
    ai->st.old_scx=p->cx; 
    ai->st.old_scy=p->cy;
    ai->st.self->idtype=1;
  } else
    ai->st.self=NULL;

  // ID our opponent
  if (ai->st.botCol==0) p=id_coloured_blob2(ai,blobs,0);
  else p=id_coloured_blob2(ai,blobs,1);
  if (p!=NULL&&p!=ai->st.ball&&p!=ai->st.self) {
    ai->st.opp=p;	

    if (fabs(p->adj_Y[0][1])>.1) {
      dmax=384.0-p->adj_Y[0][1];
      dmin=767.0-p->adj_Y[1][1];
      pink=(dmax-dmin)/(768.0-384.0);
      adj=dmin+((p->adj_Y[1][1]-p->cy)*pink);
      p->cy=p->cy+adj;
      if (p->cy>767) p->cy=767;
      if (p->cy<1) p->cy=1;
    }

    ai->st.oppID=1;
    ai->st.ovx=p->cx-ai->st.old_ocx;
    ai->st.ovy=p->cy-ai->st.old_ocy;
    ai->st.opp->vx=ai->st.ovx;
    ai->st.opp->vy=ai->st.ovy;
    ai->st.odx=p->dx;
    ai->st.ody=p->dy;

    ai->st.old_ocx=p->cx; 
    ai->st.old_ocy=p->cy;
    ai->st.opp->idtype=2;

    vx=ai->st.ovx;
    vy=ai->st.ovy;
    mg=sqrt((vx*vx)+(vy*vy));
    if (mg>NOISE_VAR) {
      vx/=mg;
      vy/=mg;
      ai->st.omx=vx;
      ai->st.omy=vy;
    } else {
      ai->st.omx=0;
      ai->st.omy=0;
    }
    ai->st.opp->mx=ai->st.omx;
    ai->st.opp->my=ai->st.omy;
  } else
    ai->st.opp=NULL;

}

void id_bot(struct RoboAI *ai, struct blob *blobs) {
  ///////////////////////////////////////////////////////////////////////////////
  // ** DO NOT CHANGE THIS FUNCTION **
  // This routine calls track_agents() to identify the blobs corresponding to the
  // robots and the ball. It commands the bot to move forward slowly so heading
  // can be established from blob-tracking.
  //
  // NOTE 1: All heading estimates, velocity vectors, position, and orientation
  //         are noisy. Remember what you have learned about noise management.
  //
  // NOTE 2: Heading and velocity estimates are not valid while the robot is
  //         rotating in place (and the final heading vector is not valid either).
  //         To re-establish heading, forward/backward motion is needed.
  //
  // NOTE 3: However, you do have a reliable orientation vector within the blob
  //         data structures derived from blob shape. It points along the long
  //         side of the rectangular 'uniform' of your bot. It is valid at all
  //         times (even when rotating), but may be pointing backward and the
  //         pointing direction can change over time.
  //
  // You should *NOT* call this function during the game. This is only for the
  // initialization step. Calling this function during the game will result in
  // unpredictable behaviour since it will update the AI state.
  ///////////////////////////////////////////////////////////////////////////////
 
  struct blob *p;
  static double stepID=0;
  double frame_inc=1.0/5.0;

  BT_drive(LEFT_MOTOR, RIGHT_MOTOR, 30);			// Start forward motion to establish heading
            // Will move for a few frames.

  track_agents(ai,blobs);		// Call the tracking function to find each agent

  if (ai->st.selfID==1&&ai->st.self!=NULL)
    fprintf(stderr,"Successfully identified self blob at (%f,%f)\n",ai->st.self->cx,ai->st.self->cy);
  if (ai->st.oppID==1&&ai->st.opp!=NULL)
    fprintf(stderr,"Successfully identified opponent blob at (%f,%f)\n",ai->st.opp->cx,ai->st.opp->cy);
  if (ai->st.ballID==1&&ai->st.ball!=NULL)
    fprintf(stderr,"Successfully identified ball blob at (%f,%f)\n",ai->st.ball->cx,ai->st.ball->cy);

  stepID+=frame_inc;
  if (stepID>=1&&ai->st.selfID==1) {	// Stop after a suitable number of frames.
    ai->st.state+=1;
    stepID=0;
    BT_all_stop(0);
  } else if
    (stepID>=1) stepID=0;

  // At each point, each agent currently in the field should have been identified.
  return;
}
/*********************************************************************************
 * End of blob ID and tracking code
 * ******************************************************************************/

/*********************************************************************************
 * Routine to initialize the AI 
 * *******************************************************************************/
int setupAI(int mode, int own_col, struct RoboAI *ai) {
  /////////////////////////////////////////////////////////////////////////////
  // ** DO NOT CHANGE THIS FUNCTION **
  // This sets up the initial AI for the robot. There are three different modes:
  //
  // SOCCER -> Complete AI, tries to win a soccer game against an opponent
  // PENALTY -> Score a goal (no goalie!)
  // CHASE -> Kick the ball and chase it around the field
  //
  // Each mode sets a different initial state (0, 100, 200). Hence, 
  // AI states for SOCCER will be 0 through 99
  // AI states for PENALTY will be 100 through 199
  // AI states for CHASE will be 200 through 299
  //
  // You will of course have to add code to the AI_main() routine to handle
  // each mode's states and do the right thing.
  //
  // Your bot should not become confused about what mode it started in!
  //////////////////////////////////////////////////////////////////////////////        

  switch (mode) {
    case AI_SOCCER:
      fprintf(stderr,"Standard Robo-Soccer mode requested\n");
      ai->st.state=0;		// <-- Set AI initial state to 0
      break;
    case AI_PENALTY:
      fprintf(stderr,"Penalty mode! let's kick it!\n");
      ai->st.state=100;	// <-- Set AI initial state to 100
      break;
    case AI_CHASE:
      fprintf(stderr,"Chasing the ball...\n");
      ai->st.state=200;	// <-- Set AI initial state to 200
      break;	
    default:
      fprintf(stderr, "AI mode %d is not implemented, setting mode to SOCCER\n", mode);
      ai->st.state=0;
  }

  BT_all_stop(0);			// Stop bot,
  ai->runAI = AI_main;		// and initialize all remaining AI data
  ai->calibrate = AI_calibrate;
  ai->st.ball=NULL;
  ai->st.self=NULL;
  ai->st.opp=NULL;
  ai->st.side=0;
  ai->st.botCol=own_col;
  ai->st.old_bcx=0;
  ai->st.old_bcy=0;
  ai->st.old_scx=0;
  ai->st.old_scy=0;
  ai->st.old_ocx=0;
  ai->st.old_ocy=0;
  ai->st.bvx=0;
  ai->st.bvy=0;
  ai->st.svx=0;
  ai->st.svy=0;
  ai->st.ovx=0;
  ai->st.ovy=0;
  ai->st.sdx=0;
  ai->st.sdy=0;
  ai->st.odx=0;
  ai->st.ody=0;
  ai->st.bdx=0;
  ai->st.bdy=0;
  ai->st.selfID=0;
  ai->st.oppID=0;
  ai->st.ballID=0;
  ai->DPhead=NULL;
  fprintf(stderr,"Initialized!\n");

  return(1);
}

void AI_calibrate(struct RoboAI *ai, struct blob *blobs) {
  // Basic colour blob tracking loop for calibration of vertical offset
  // See the handout for the sequence of steps needed to achieve calibration.
  // The code here just makes sure the image processing loop is constantly
  // tracking the bots while they're placed in the locations required
  // to do the calibration (i.e. you DON'T need to add anything more
  // in this function).
  track_agents(ai,blobs);
}


/**************************************************************************
 * AI state machine - this is where you will implement your soccer
 * playing logic
 * ************************************************************************/
void AI_main(struct RoboAI *ai, struct blob *blobs, void *state) {
  /*************************************************************************
    This is your robot's state machine.
    
    It is called by the imageCapture code *once* per frame. And it *must not*
    enter a loop or wait for visual events, since no visual refresh will happen
    until this call returns!
    
    Therefore. Everything you do in here must be based on the states in your
    AI and the actions the robot will perform must be started or stopped 
    depending on *state transitions*. 

    E.g. If your robot is currently standing still, with state = 03, and
    your AI determines it should start moving forward and transition to
    state 4. Then what you must do is 
    - send a command to start forward motion at the desired speed
    - update the robot's state
    - return
    
    I can not emphasize this enough. Unless this call returns, no image
    processing will occur, no new information will be processed, and your
    bot will be stuck on its last action/state.

    You will be working with a state-based AI. You are free to determine
    how many states there will be, what each state will represent, and
    what actions the robot will perform based on the state as well as the
    state transitions.

    You must *FULLY* document your state representation in the report

    The first two states for each more are already defined:
    State 0,100,200 - Before robot ID has taken place (this state is the initial
                    state, or is the result of pressing 'r' to reset the AI)
    State 1,101,201 - State after robot ID has taken place. At this point the AI
                    knows where the robot is, as well as where the opponent and
                    ball are (if visible on the playfield)

    Relevant UI keyboard commands:
    'r' - reset the AI. Will set AI state to zero and re-initialize the AI
    data structure.
    't' - Toggle the AI routine (i.e. start/stop calls to AI_main() ).
    'o' - Robot immediate all-stop! - do not allow your EV3 to get damaged!

    IMPORTANT NOTE: There are TWO sources of information about the 
                    location/parameters of each agent
                    1) The 'blob' data structures from the imageCapture module
                    2) The values in the 'ai' data structure.
                        The 'blob' data is incomplete and changes frame to frame
                        The 'ai' data should be more robust and stable
                        BUT in order for the 'ai' data to be updated, you
                        must call the function 'track_agents()' in your code
                        after eah frame!
                        
      DATA STRUCTURE ORGANIZATION:

      'RoboAI' data structure 'ai'
          \    \    \   \--- calibrate()  (pointer to AI_clibrate() )
            \    \    \--- runAI()  (pointer to the function AI_main() )
            \    \------ Display List head pointer 
              \_________ 'ai_data' data structure 'st'
                          \  \   \------- AI state variable and other flags
                            \  \---------- pointers to 3 'blob' data structures
                            \             (one per agent)
                              \------------ parameters for the 3 agents
                                
    ** Do not change the behaviour of the robot ID routine **
  **************************************************************************/

  static double ux,uy,len,mmx,mmy,px,py,lx,ly,mi;
  double angDif, lPow,rPow;
  char line[1024];
  static int count=0;
  static double old_dx=0, old_dy=0;
  struct displayList *q;
  
  // change to the ports representing the left and right motors in YOUR robot
  char lport=MOTOR_A;
  char rport=MOTOR_D;
      
  /************************************************************
   * Standard initialization routine for starter code,
   * from state **0 performs agent detection and initializes
   * directions, motion vectors, and locations
   * Triggered by toggling the AI on.
   * - Modified now (not in starter code!) to have local
   *   but STATIC data structures to keep track of robot
   *   parameters across frames (blob parameters change
   *   frame to frame, memoryless).
   ************************************************************/
  if (ai->st.state==0||ai->st.state==100||ai->st.state==200) { 	// Initial set up - find own, ball, and opponent blobs
    // Carry out self id process.
    fprintf(stderr,"Initial state, self-id in progress...\n"); 
    id_bot(ai,blobs);
    if ((ai->st.state%100)!=0) {	// The id_bot() routine will change the AI state to initial state + 1
  		// if robot identification is successful.
      if (ai->st.self->cx>=512) ai->st.side=1; else ai->st.side=0;
      BT_all_stop(0);
   
      fprintf(stderr,"Self-ID complete. Current position: (%f,%f), current heading: [%f, %f], blob direction=[%f, %f], AI state=%d side=%d\n",
        ai->st.self->cx,ai->st.self->cy,ai->st.smx,ai->st.smy,ai->st.sdx,ai->st.sdy,ai->st.state, ai->st.side);
   
      if (ai->st.self!=NULL) {
          // This checks that the motion vector and the blob direction vector
          // are pointing in the same direction. If they are not (the dot product
          // is less than 0) it inverts the blob direction vector so it points
          // in the same direction as the motion vector.
          if (((ai->st.smx*ai->st.sdx)+(ai->st.smy*ai->st.sdy))<0) {
            ai->st.self->dx*=-1.0;
            ai->st.self->dy*=-1.0;
            ai->st.sdx*=-1;
            ai->st.sdy*=-1;
          }
          old_dx=ai->st.sdx;
          old_dy=ai->st.sdy;
      }
  
      if (ai->st.opp!=NULL) {
        // Checks motion vector and blob direction for opponent. See above.
        if (((ai->st.omx*ai->st.odx)+(ai->st.omy*ai->st.ody))<0) {
          ai->st.opp->dx*=-1;
          ai->st.opp->dy*=-1;
          ai->st.odx*=-1;
          ai->st.ody*=-1;
        }       
      }

      // Notice that the ball's blob direction is not useful! only its
      // position and motion matter.
    }
  } else {

    /****************************************************************************
    TO DO:
    You will need to replace this 'catch-all' code with actual program logic to
    implement your bot's state-based AI.

    After id_bot() has successfully completed its work, the state should be
    1 - if the bot is in SOCCER mode
    101 - if the bot is in PENALTY mode
    201 - if the bot is in CHASE mode

    Your AI code needs to handle these states and their associated state
    transitions which will determine the robot's behaviour for each mode.

    Please note that in this function you should add appropriate functions below
    to handle each state's processing, and the code here should mostly deal with
    state transitions and with calling the appropriate function based on what
    the bot is supposed to be doing.
    *****************************************************************************/
    //  fprintf(stderr,"Just trackin'!\n"); // bot, opponent, and ball.
    //  track_agents(ai,blobs);   // Currently, does nothing but endlessly track
    double goalx = ai->st.side == 1 ? 0 : 1024;
    double selfGoalx = ai->st.side == 1 ? 1024 : 0;
    double goaly = 384;
    track_agents(ai,blobs);

    if (ai->st.self == NULL) {
      fprintf(stderr, "\tNull ai->self\n");
      BT_all_stop(0);
      return;
    }
    if (ai->st.ball == NULL) {
      fprintf(stderr, "\tNull ai->ball\n");
      BT_all_stop(0);
      return;
    }

    // flip self dx dy if needed
    if (dottie(ai->st.self->dx, ai->st.self->dy, old_dx, old_dy)<0) {
      ai->st.self->dx*=-1.0;
      ai->st.self->dy*=-1.0;
      ai->st.sdx*=-1;
      ai->st.sdy*=-1;
    }
    old_dx=ai->st.sdx;
    old_dy=ai->st.sdy;

    double selfw = ai->st.self->x2 - ai->st.self->x1;
    double selfh = ai->st.self->y2 - ai->st.self->y1;
    static double defend_ballx = 512;
    static double defend_bally = 384;
    // 0 -> offense, 1 -> defense, 2 -> safety
    static int robo_soccor_mode = 0;
    double spotx, spoty, vGoalToBallx, vGoalToBally;
    int indicator = 0;

    if (ai->st.state*0.01 > 0.0 && ai->st.state*0.01 < 1.0) { // 0xx robo soccer

      // consective 5 frames no opponent, attack
      if (ai->st.opp == NULL) {
        fprintf(stderr, "\tNull ai->opp\n");
        if (robo_soccor_mode != 0) {
          if (count < 5) {
            count++;
            BT_all_stop(0);
          } else {
            count = 0;
            robo_soccor_mode = 0;
            ai->st.state = 001;
          }
          return;
        }
      } else {
        count = 0;

        double selfOppDist = distance(ai->st.self->cx, ai->st.self->cy, ai->st.opp->cx, ai->st.opp->cy);
        double selfBallDist = distance(ai->st.self->cx, ai->st.self->cy, ai->st.ball->cx, ai->st.ball->cy);
        double oppBallDist = distance(ai->st.ball->cx, ai->st.ball->cy, ai->st.opp->cx, ai->st.opp->cy);
        // change to defense or safety state only when opp exists
        if (robo_soccor_mode == 0) {
          // safety first
          if (selfOppDist < selfw) {
            robo_soccor_mode = 2;
            ai->st.state = 003;
          } else if (abs(selfGoalx - ai->st.ball->cx) < selfw) {
            // ball close to our goal, defend
            robo_soccor_mode = 1;
            ai->st.state = 002;
          } else if (oppBallDist < 2*selfw && oppBallDist*2 < selfBallDist) {
            // opp closer than us, defend
            robo_soccor_mode = 1;
            ai->st.state = 002;
          }
        } else if (robo_soccor_mode == 1) {
          // safety first
          if (selfOppDist < selfw) {
            robo_soccor_mode = 2;
            ai->st.state = 003;
          } else if (oppBallDist > 2*selfw) {
            robo_soccor_mode = 0;
            ai->st.state = 001;
          }
        } else {
          if (selfOppDist > 2*selfw) {
            robo_soccor_mode = 0;
            ai->st.state = 001;
          }
        }
      }

      switch (ai->st.state) {
        // 00x deciding mode
        case 001:
          fprintf(stderr,"001 initialize offense mode\n");
          indicator = decide_offensive_behavior(ai,blobs,goalx,goaly);
          if (indicator == 0) {
            ai->st.state = 012;
          } else if (indicator == 1) {
            ai->st.state = 010;
          } else {
            ai->st.state = 014;
          }
          break;
        case 002:
          fprintf(stderr,"002 initialize defense mode\n");
          defend_ballx = ai->st.ball->cx;
          defend_bally = ai->st.ball->cy;
          indicator = decide_defensive_behavior(ai,blobs,selfGoalx,goaly);
          if (indicator == 0) {
            ai->st.state = 022;
          } else {
            ai->st.state = 020;
          }
          break;
        case 003:
          fprintf(stderr,"003 initialize safety mode\n");
          indicator = decide_safety_behavior(ai,blobs);
          if (indicator == 0) {
            ai->st.state = 030;
          } else {
            ai->st.state = 031;
          }
          break;

        // 01x offense mode
        case 010:
          fprintf(stderr,"010 turn to face the sopt on ball-goal line\n");
          vGoalToBallx = normalize(ai->st.ball->cx-goalx, ai->st.ball->cx-goalx, ai->st.ball->cy-goaly);
          vGoalToBally = normalize(ai->st.ball->cy-goaly, ai->st.ball->cx-goalx, ai->st.ball->cy-goaly);
          spotx = ai->st.ball->cx + 4*selfw*vGoalToBallx;
          spoty = ai->st.ball->cy + 4*selfw*vGoalToBally;
          if (spotx > 1024 - selfw) spotx = 1024 - selfw;
          if (spotx < 0 + selfw) spotx = 0 + selfw;
          if (spoty > 512 - selfw) spoty = 512 - selfw;
          if (spoty < 0 + selfw) spoty = 0 + selfw;

          if (turn_to_face(ai,blobs,spotx,spoty)) {
            ai->st.state = 011;
          }
          break;
        case 011:
          fprintf(stderr,"011 drive to the sopt on ball-goal line\n");
          vGoalToBallx = normalize(ai->st.ball->cx-goalx, ai->st.ball->cx-goalx, ai->st.ball->cy-goaly);
          vGoalToBally = normalize(ai->st.ball->cy-goaly, ai->st.ball->cx-goalx, ai->st.ball->cy-goaly);
          spotx = ai->st.ball->cx + 2*selfw*vGoalToBallx;
          spoty = ai->st.ball->cy + 2*selfw*vGoalToBally;
          spotx = ai->st.ball->cx + 4*selfw*vGoalToBallx;
          spoty = ai->st.ball->cy + 4*selfw*vGoalToBally;
          if (spotx > 1024 - selfw) spotx = 1024 - selfw;
          if (spotx < 0 + selfw) spotx = 0 + selfw;
          if (spoty > 512 - selfw) spoty = 512 - selfw;
          if (spoty < 0 + selfw) spoty = 0 + selfw;

          indicator = drive_to_spot(ai,blobs,spotx,spoty);
          if (indicator == 2) {
            ai->st.state = 001;
          } else if (indicator == 1) {
            ai->st.state = 012;
          }
          break;
        case 012:
          fprintf(stderr,"012 turn to ball\n");
          if (turn_to_face(ai,blobs,ai->st.ball->cx,ai->st.ball->cy)) {
            ai->st.state = 013;
          }
          break;
        case 013:
          fprintf(stderr,"013 drive and kick ball\n");
          if (drive_and_kick(ai,blobs)) {
            ai->st.state = 001;
          }
          break;
        case 014:
          fprintf(stderr,"014 turn away from the ball-goal line\n");
          spotx = ai->st.ball->cx;
          spoty = (ai->st.ball->cy > 384) ? ai->st.ball->cy * 0.5 : (ai->st.ball->cy + 1024) * 0.5;
          if (turn_to_face(ai,blobs,spotx,spoty)) {
            ai->st.state = 015;
          }
          break;
        case 015:
          fprintf(stderr,"015 clear from the ball-goal line\n");
          spotx = ai->st.ball->cx;
          spoty = (ai->st.ball->cy > 384) ? ai->st.ball->cy * 0.5 : (ai->st.ball->cy + 1024) * 0.5;
          indicator = drive_to_spot(ai,blobs,spotx,spoty);
          if (indicator == 2) {
            ai->st.state = 001;
          } else if (indicator == 1) {
            ai->st.state = 010;
          }
          break;

        // 02x defense mode
        case 020:
          fprintf(stderr,"020 turn to face the sopt on ball-selfgoal line\n");
          spotx = 0.5*(selfGoalx + defend_ballx);
          spoty = 0.5*(defend_bally + 384);

          if (turn_to_face(ai,blobs,spotx,spoty)) {
            ai->st.state = 021;
          }
          break;
        case 021:
          fprintf(stderr,"021 drive to the sopt on ball-selfgoal line\n");
          spotx = 0.5*(selfGoalx + defend_ballx);
          spoty = 0.5*(defend_bally + 384);

          if (drive_to_spot(ai,blobs,spotx,spoty) > 0) {
            ai->st.state = 002;
          }
          break;
        case 022:
          fprintf(stderr,"022 turn to face clear ball line\n");
          spotx = ai->st.ball->cx;
          spoty = (ai->st.ball->cy > 384) ? ai->st.ball->cy * 0.5 : (ai->st.ball->cy + 1024) * 0.5;

          if (turn_to_face(ai,blobs,spotx,spoty)) {
            ai->st.state = 023;
          }
          break;
        case 023:
          fprintf(stderr,"023 drive to the sopt on clear ball line\n");
          spotx = ai->st.ball->cx;
          spoty = (ai->st.ball->cy > 384) ? ai->st.ball->cy * 0.5 : (ai->st.ball->cy + 1024) * 0.5;

          if (drive_to_spot(ai,blobs,spotx,spoty) > 0) {
            ai->st.state = 024;
          }
          break;
        case 024:
          fprintf(stderr,"024 turn to ball\n");
          if (turn_to_face(ai,blobs,ai->st.ball->cx,ai->st.ball->cy)) {
            ai->st.state = 025;
          }
          break;
        case 025:
          fprintf(stderr,"025 drive and hit the ball\n");
          if (drive_and_hit(ai,blobs)) {
            ai->st.state = 002;
          }
          break;

        // 03x safety mode
        case 030:
          fprintf(stderr,"030 back off to create safety space\n");
          BT_drive(LEFT_MOTOR, RIGHT_MOTOR, -20);
          ai->st.state = 003;
          break;
        case 031:
          fprintf(stderr,"031 stop and draw a foul\n");
          BT_all_stop(0);
          ai->st.state = 003;
          break;

        default:
          fprintf(stderr,"0xx default\n");
          ai->st.state = 001;
          robo_soccor_mode = 0;
      }
    } else if (ai->st.state*0.01 > 1.0 && ai->st.state*0.01 < 2.0) { // 1xx penalty kicking
      switch (ai->st.state) {
        case 101:
          fprintf(stderr,"101 deciding \n");
          indicator = decide_offensive_behavior(ai,blobs,goalx,goaly);
          if (indicator == 0) {
            ai->st.state = 104;
          } else if (indicator == 1) {
            ai->st.state = 102;
          } else {
            ai->st.state = 106;
          }
          break;
        case 102:
          fprintf(stderr,"102 turn to face the sopt on ball-goal line\n");
          vGoalToBallx = normalize(ai->st.ball->cx-goalx, ai->st.ball->cx-goalx, ai->st.ball->cy-goaly);
          vGoalToBally = normalize(ai->st.ball->cy-goaly, ai->st.ball->cx-goalx, ai->st.ball->cy-goaly);
          spotx = ai->st.ball->cx + 4*selfw*vGoalToBallx;
          spoty = ai->st.ball->cy + 4*selfw*vGoalToBally;
          if (spotx > 1024 - selfw) spotx = 1024 - selfw;
          if (spotx < 0 + selfw) spotx = 0 + selfw;
          if (spoty > 512 - selfw) spoty = 512 - selfw;
          if (spoty < 0 + selfw) spoty = 0 + selfw;

          if (turn_to_face(ai,blobs,spotx,spoty)) {
            ai->st.state = 103;
          }
          break;
        case 103:
          fprintf(stderr,"103 drive to the sopt on ball-goal line\n");
          vGoalToBallx = normalize(ai->st.ball->cx-goalx, ai->st.ball->cx-goalx, ai->st.ball->cy-goaly);
          vGoalToBally = normalize(ai->st.ball->cy-goaly, ai->st.ball->cx-goalx, ai->st.ball->cy-goaly);
          spotx = ai->st.ball->cx + 4*selfw*vGoalToBallx;
          spoty = ai->st.ball->cy + 4*selfw*vGoalToBally;
          if (spotx > 1024 - selfw) spotx = 1024 - selfw;
          if (spotx < 0 + selfw) spotx = 0 + selfw;
          if (spoty > 512 - selfw) spoty = 512 - selfw;
          if (spoty < 0 + selfw) spoty = 0 + selfw;

          indicator = drive_to_spot(ai,blobs,spotx,spoty);
          if (indicator == 2) {
            ai->st.state = 101;
          } else if (indicator == 1) {
            ai->st.state = 104;
          }
          break;
        case 104:
          fprintf(stderr,"104 turn to ball\n");
          if (turn_to_face(ai,blobs,ai->st.ball->cx,ai->st.ball->cy)) {
            ai->st.state = 105;
          }
          break;
        case 105:
          fprintf(stderr,"105 drive and kick ball\n");
          if (drive_and_kick(ai,blobs)) {
            ai->st.state = 101;
          }
          break;
        case 106:
          fprintf(stderr,"106 turn away from the ball-goal line\n");
          spotx = ai->st.ball->cx;
          spoty = (ai->st.ball->cy > 384) ? ai->st.ball->cy * 0.5 : (ai->st.ball->cy + 1024) * 0.5;
          if (turn_to_face(ai,blobs,spotx,spoty)) {
            ai->st.state = 107;
          }
          break;
        case 107:
          fprintf(stderr,"107 clear from the ball-goal line\n");
          spotx = ai->st.ball->cx;
          spoty = (ai->st.ball->cy > 384) ? ai->st.ball->cy * 0.5 : (ai->st.ball->cy + 1024) * 0.5;
          indicator = drive_to_spot(ai,blobs,spotx,spoty);
          if (indicator == 2) {
            ai->st.state = 101;
          } else if (indicator == 1) {
            ai->st.state = 102;
          }
          break;
        default:
          fprintf(stderr,"1xx default deciding \n");
          ai->st.state = 101;
      }
    } else if (ai->st.state*0.01 > 2.0 && ai->st.state*0.01 < 3.0) { //2** chasing ball
      switch (ai->st.state) {
        case 201:
          fprintf(stderr,"201 after init\n");
          ai->st.state = 202;
          break;
        case 202:
          fprintf(stderr,"202 turn to ball\n");
          if (turn_to_face(ai,blobs,ai->st.ball->cx,ai->st.ball->cy)) {
            ai->st.state = 203;
          }
          break;
        case 203:
          fprintf(stderr,"203 drive and hit the ball\n");
          if (drive_and_hit(ai,blobs)) {
            ai->st.state = 202;
          }
          break;
        default:
          fprintf(stderr,"2xx default\n");
          ai->st.state = 202;
      }
    }
  }
}

/**********************************************************************************
 TO DO:

 Add the rest of your game playing logic below. Create appropriate functions toTo
 handle different states (be sure to name the states/functions in a meaningful
 way), and do any processing required in the space below.

 AI_main() should *NOT* do any heavy lifting. It should only call appropriate
 functions based on the current AI state.

 You will lose marks if AI_main() is cluttered with code that doesn't belong
 there.
**********************************************************************************/
double normalize(double v, double vx, double vy) {
  return v/sqrt(vx*vx + vy*vy);
}

double distance(double x1, double y1, double x2, double y2) {
  return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}

int decide_offensive_behavior(struct RoboAI *ai, struct blob *blobs, double goalx, double goaly) {
  double vSelfToBallx = -1 * normalize(ai->st.self->cx-ai->st.ball->cx, ai->st.self->cx-ai->st.ball->cx, ai->st.self->cy-ai->st.ball->cy);
  double vSelfToBally = -1 * normalize(ai->st.self->cy-ai->st.ball->cy, ai->st.self->cx-ai->st.ball->cx, ai->st.self->cy-ai->st.ball->cy);
  double vBallToGoalx = -1 * normalize(ai->st.ball->cx-goalx, ai->st.ball->cx-goalx, ai->st.ball->cy-goaly);
  double vBallToGoaly = -1 * normalize(ai->st.ball->cy-goaly, ai->st.ball->cx-goalx, ai->st.ball->cy-goaly);

  if (dottie(vSelfToBallx, vSelfToBally, vBallToGoalx, vBallToGoaly) > 0.9) {
    // right direction, go kick
    return 0;
  } else if (dottie(vSelfToBallx, vSelfToBally, vBallToGoalx, vBallToGoaly) < -0.8) {
    // in between ball and goal, adjust
    return 2;
  } else {
    // other locations, go to a good kicking spot
    return 1;
  }
}

int decide_defensive_behavior(struct RoboAI *ai, struct blob *blobs, double selfGoalx, double selfGoaly) {

  double selfBallDist = distance(ai->st.self->cx, ai->st.self->cy, ai->st.ball->cx, ai->st.ball->cy);
  double oppBallDist = distance(ai->st.ball->cx, ai->st.ball->cy, ai->st.opp->cx, ai->st.opp->cy);

  if (abs(selfGoalx - ai->st.ball->cx) < (ai->st.self->x2 - ai->st.self->x1)) {
    // ball close to our goal, defend
    return 0;
  } else if (oppBallDist < 2*(ai->st.self->x2 - ai->st.self->x1) && oppBallDist*2 < selfBallDist) {
    // opp closer than us, defend
    return 1;
  }
}

int decide_safety_behavior(struct RoboAI *ai, struct blob *blobs) {
  //double selfOppDist = distance(ai->st.self->cx, ai->st.self->cy, ai->st.opp->cx, ai->st.opp->cy);
  double vOppToSelfx = -1 * normalize(ai->st.opp->cx-ai->st.self->cx, ai->st.opp->cx-ai->st.self->cx, ai->st.opp->cy-ai->st.self->cy);
  double vOppToSelfy = -1 * normalize(ai->st.opp->cy-ai->st.self->cy, ai->st.opp->cx-ai->st.self->cx, ai->st.opp->cy-ai->st.self->cy);

  if (dottie(vOppToSelfx, vOppToSelfy, ai->st.opp->mx, ai->st.opp->my) > 0.8) {
    // opp is charging me
    return 1;
  } else  {
    // back off
    return 0;
  }
}

int turn_to_face(struct RoboAI *ai, struct blob *blobs, double spotx, double spoty) {
  double vSelfToSpotx = normalize(spotx - ai->st.self->cx, spotx - ai->st.self->cx, spoty - ai->st.self->cy);
  double vSelfToSpoty = normalize(spoty - ai->st.self->cy, spotx - ai->st.self->cx, spoty - ai->st.self->cy);

  double dot = dottie(ai->st.self->dx, ai->st.self->dy, vSelfToSpotx, vSelfToSpoty);
  // fprintf(stderr, "\t turn dot product %lf, stsx %lf, stsy %lf, ndx %lf, ndy %lf\n", dot, vSelfToSpotx, vSelfToSpoty, ai->st.self->dx, ai->st.self->dy);
  // fprintf(stderr, "\t turn dot product %lf, bx %lf, by %lf, sx %lf, sy %lf\n", dot, spotx, spoty, ai->st.self->cx, ai->st.self->cy);
  if (dot > 0.95) {
    BT_all_stop(0);
    fprintf(stderr, "\t turn face to spot\n");
    return 1;
  } else {
    double propertion = (dot < 0) ? 1 : 1-(dot*dot);
    if (crossie_sign(ai->st.self->dx, ai->st.self->dy, vSelfToSpotx, vSelfToSpoty) > 0) {
      BT_turn(LEFT_MOTOR, 25*propertion,  RIGHT_MOTOR, -25*propertion);
    } else {
      BT_turn(LEFT_MOTOR, -25*propertion, RIGHT_MOTOR, 25*propertion);

    }
  }
  return 0;
}

int drive_to_spot(struct RoboAI *ai, struct blob *blobs, double spotx, double spoty) {
  double selfSpotDist = distance(ai->st.self->cx, ai->st.self->cy, spotx, spoty);
  double propertion = fmax((selfSpotDist/384), 0.66);
  BT_drive(LEFT_MOTOR, RIGHT_MOTOR, 30*propertion);
  if (selfSpotDist < (ai->st.self->y2 - ai->st.self->y1)) {
  //if (abs(ai->st.self->cx - spotx) < 1 || abs(ai->st.self->cy - spoty) < 1) {
    fprintf(stderr, "\t drive to spot Already at spot\n");
    BT_all_stop(0);
    return 1;
  }

  double vSelfToSpotx = normalize(spotx - ai->st.self->cx, spotx - ai->st.self->cx, spoty - ai->st.self->cy);
  double vSelfToSpoty = normalize(spoty - ai->st.self->cy, spotx - ai->st.self->cx, spoty - ai->st.self->cy);
  double normSelfmx = 0;
  double normSelfmy = 0;
  
  //fprintf(stderr, "\tmx: %lf;    my: %lf\n", ai->st.self->mx, ai->st.self->my);
  if (ai->st.self->mx != 0 || ai->st.self->my != 0) {
    normSelfmx = normalize(ai->st.self->mx, ai->st.self->mx, ai->st.self->my);
    normSelfmy = normalize(ai->st.self->my, ai->st.self->mx, ai->st.self->my);
  }

  // when motion vector valid, use it
  static int angCount = 0;
  if (normSelfmx != 0 || normSelfmy != 0) {
    if (dottie(normSelfmx, normSelfmy, vSelfToSpotx, vSelfToSpoty) < 0.8) {
      if (angCount < 2) {
        angCount++;
      } else {
        angCount = 0;
        BT_all_stop(0);
        fprintf(stderr, "\t drive to spot angle very different\n");
        return 2;
      }
    }
  }

  static double lastSSDist = 10000;
  static int disCount = 0;
  if (selfSpotDist > lastSSDist) {
    if (disCount < 2) {
      disCount++;
    } else {
    // moving away from the ball, redo decision
      fprintf(stderr, "\t drive to spot, moving away\n");
      BT_all_stop(0);
      disCount = 0;
      lastSSDist = 10000;
      return 2;
    }
  } else {
    disCount = 0;BT_drive(LEFT_MOTOR, RIGHT_MOTOR, 30*propertion);
  }

  lastSSDist = selfSpotDist;
  return 0;
}

int drive_and_kick(struct RoboAI *ai, struct blob *blobs) {
  static double lastSBDist = 10000;
  static int count = 0;
  double selfBallDist = distance(ai->st.self->cx, ai->st.self->cy, ai->st.ball->cx, ai->st.ball->cy);
  if (selfBallDist > lastSBDist) {
    if (count < 2) {
      count ++;
    } else {
    // moving away from the ball, redo decision
      fprintf(stderr, "\t drive and kick, ball is moving away\n");
      BT_all_stop(0);
      count = 0;
      lastSBDist = 10000;
      return 1;
    }
  } else {
    count = 0;
  }
  lastSBDist = selfBallDist;

  drive_to_spot(ai, blobs, ai->st.ball->cx, ai->st.ball->cy);
  if (selfBallDist < 2*(ai->st.self->x2 - ai->st.self->x1)) {
    // ball close, start kicking
    //BT_drive(LEFT_MOTOR, RIGHT_MOTOR, 10);
    BT_turn(KICK_MOTOR, 100, MOTOR_B, 0);
  }
  return 0;
}

int drive_and_hit(struct RoboAI *ai, struct blob *blobs) {
  // if (ai->st.self == NULL) {
  //   fprintf(stderr, "\tNull ai->self\n");
  //   BT_all_stop(0);
  //   return 0;
  // }
  // if (ai->st.ball == NULL) {
  //   fprintf(stderr, "\tNull ai->ball\n");
  //   BT_all_stop(0);
  //   return 0;
  // }

  static double lastSBDist = 10000;
  static int count = 0;
  double selfBallDist = distance(ai->st.self->cx, ai->st.self->cy, ai->st.ball->cx, ai->st.ball->cy);
  if (selfBallDist > lastSBDist) {
    if (count < 3) {
      count ++;
    } else {
    // moving away from the ball, redo decision
      fprintf(stderr, "\t drive and hit, ball is moving away\n");
      BT_all_stop(0);
      count = 0;
      lastSBDist = 10000;
      return 1;
    }
  } else {
    count = 0;
  }
  lastSBDist = selfBallDist;

  if (selfBallDist < (ai->st.self->x2 - ai->st.self->x1) && ball_close_to_wall(ai->st.self, ai->st.ball)) {
    // if ball close to wall, hook it out
    fprintf(stderr, "\t drive and hit, hooking\n");
    if (ai->st.ball->cy < 384) {
      if (ai->st.side == 0) {
        BT_turn(LEFT_MOTOR, -25,  RIGHT_MOTOR, 25);
      } else {
        BT_turn(LEFT_MOTOR, 25,  RIGHT_MOTOR, -25);
      }
    } else {
      if (ai->st.side == 0) {
        BT_turn(LEFT_MOTOR, 25,  RIGHT_MOTOR, -25);
      } else {
        BT_turn(LEFT_MOTOR, -25,  RIGHT_MOTOR, 25);
      }
    }
  } else {
    drive_to_spot(ai, blobs, ai->st.ball->cx, ai->st.ball->cy);
  }
  return 0;
}

int ball_close_to_wall(struct blob *self, struct blob *ball) {
  if (ball->cx > 1024 - (self->x2 - self->x1)) return 1;
  if (ball->cx < 0 + (self->x2 - self->x1)) return 1;
  if (ball->cy > 768 - (self->x2 - self->x1)) return 1;
  if (ball->cy < 0 + (self->x2 - self->x1)) return 1;
  return 0;
}
