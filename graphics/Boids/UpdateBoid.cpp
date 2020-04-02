/************************************************************************
%
% Particle Systems
%
% A fundamental component of computer generated animation is
% what we call 'particle systems'.
%
% Loosely defined, a particle system consists of a set of 
% particles, each with their own appearance properties appropriate
% to the image being rendered. The particles in the system move
% and change appearance in response to a set of rules that
% automatically determine where they should be at each frame.
%
% Particle systems are typically used to generate water droplets,
% sand, smoke and airborne particles, animated swarms, falling
% leaves, snow, fireworks, and many more.
%
% For a few samples of what you can do with particle systems
% look here:
%
% https://www.youtube.com/watch?v=QbUPfMXXQIY
% https://www.youtube.com/watch?v=AcPbn4w5Vvw
% https://www.youtube.com/watch?v=Hw89cwVW_Lo
% https://www.youtube.com/watch?v=WZZEtolFzFI
%
% This is not too far from what happens in real life:
%
% https://www.youtube.com/watch?v=V4f_1_r80RY
%
% Here you will implement the simplest and best known example
% of a particle system - used to simulate flocks of birds, or
% schools of fish. These particles are called 'Boids' - there's
% a story behind that which you can look into if you're curious.
%
% So let's get on with it! - You will focus on implementing code
% within the UpdateBoid() function below. You can ignore the 
% sign() and HSV2RGB() functions, just there to help a bit.
% 
% Look for:  //  * You need to implement this *
% 
% DO NOT change MAX_BOIDS - or bad things will happen (e.g. segfault)
%
% F. Estrada - CSC D18 - Computer Graphics, Fall 2019
***************************************************************************/

#include<math.h>
#include<stdio.h>
#define MAX_BOIDS 50000

//*** A couple of useful functions you DON'T need to worry about ***/
float sign(float x){if (x>=0) return(1.0); else return(-1.0);}
void HSV2RGB(float H, float S, float V, float *R, float *G, float *B)
{
 // Convert an HSV triplet (all values in [0 1]) to RGB values in
 // [0 1].
 float c,x,hp,r1,g1,b1,m;

 hp=H*6;
 c=V*S;
 x=c*(1.0-fabs(fmod(hp,2)-1.0));
 if (hp<1){r1=c;g1=x;b1=0;}
 else if (hp<2){r1=x;g1=c;b1=0;}
 else if (hp<3){r1=0;g1=c;b1=x;}
 else if (hp<4){r1=0;g1=x;b1=c;}
 else if (hp<5){r1=x;g1=0;b1=c;}
 else{r1=c;g1=0;b1=x;}

 m=V-c;
 *R=r1+m;
 *G=g1+m;
 *B=b1+m;

 if (*R>1) *R=1;
 if (*R<0) *R=0;
 if (*G>1) *G=1;
 if (*G<0) *G=0;
 if (*B>1) *B=1;
 if (*B<0) *B=0;
}

//**** You will be working on the code in the function below! read the description and instructions carefully!
void updateBoid(int i, int nBoids, float Boid_Location[MAX_BOIDS][3], float Boid_Velocity[MAX_BOIDS][3], float Boid_Color[MAX_BOIDS][3], float r_rule1, float r_rule2, float r_rule3, float k_rule0, float k_rule1, float k_rule2, float k_rule3)
{
 /*
  * Your job is to implement the rules that govern Boid behaviour. To that end, you have access to
  * each Boid's location and velocity, as well as the values of all constants and
  * radii for the update rules - notes on how to apply these rules follow. Notice that this
  * function updates the position and velocity of *a single* Boid. It will be called by the 
  * driver code for each of the Boids in the simulation. 
  * 
 */

 //////////////////////////////////////////////////////////////////////////
 // Boids follow a set of very simple rules. Each rule is explained below, and must be
 // implemented for the code to work properly. Be sure to pay close attention to your
 // computations - it's easy to have quantities explore and send your Boids to infinity
 // and beyond!
 ///////////////////////////////////////////////////////////////////////////

 float r2_x,r2_y,r2_z,dist,H;	// Variables for Boid colour update. Don't change them.
				// Add any variables you need just below this line.

 ///////////////////////////////////////////////////////////////////////////
 //
 // Boid update Rule 1:  Boids of a feather flock together!
 //
 //    This boid will try to move the center of mass of the group it's in
 //
 // In the original reference, the center of mass is computed for ALL boids. 
 // Here we will do it a little differently:
 //
 //    * Compute the center of mass for all boids that are within a radius 
 //      'r_rule1' from the current boid. r_rule1 is a parameter passed to
 //      your function, it is set by the command line, but can be updated
 //      via keyboard. Its value is in [5, 150]
 //
 //    * Get a vector from the current Boid to the center of mass. Say it's 'v1'
 //      The updated velocity  v' = v + v1*k_rule1
 //
 //    k_rule1 determines the strength of Rule 1, it's set by command line
 //    and can be changed via keyboard.
 //
 //  * You need to implement this *
 ///////////////////////////////////////////////////////////////////////////

  float vxl = Boid_Velocity[i][0];
  float vyl = Boid_Velocity[i][1];
  float vzl = Boid_Velocity[i][2];

  int n;
  float cmassx = Boid_Location[i][0];
  float cmassy = Boid_Location[i][1];
  float cmassz = Boid_Location[i][2];
  float count = 0;
  for (n = 0; n < nBoids; n++) {
    float bx = Boid_Location[n][0] - Boid_Location[i][0];
    float by = Boid_Location[n][1] - Boid_Location[i][1];
    float bz = Boid_Location[n][2] - Boid_Location[i][2];
    if (sqrt(bx*bx + by*by + bz*bz) <= r_rule1) {
      cmassx += bx;
      cmassy += by;
      cmassz += bz;
      count += 1;
    }
  }

  cmassx = cmassx/count;
  cmassy = cmassy/count;
  cmassz = cmassz/count;

  float v1x = cmassx - Boid_Location[i][0];
  float v1y = cmassy - Boid_Location[i][1];
  float v1z = cmassz - Boid_Location[i][2];

  Boid_Velocity[i][0] = Boid_Velocity[i][0] + v1x*k_rule1;
  Boid_Velocity[i][1] = Boid_Velocity[i][1] + v1y*k_rule1;
  Boid_Velocity[i][2] = Boid_Velocity[i][2] + v1z*k_rule1;

 ///////////////////////////////////////////////////////////////////////////
 // Crunchy:
 //
 //    This rule results in an N^2 computation since we have to call
 //  UpdateBoid() for each of N Boids, and each time we compute a center
 //  of mass from the remaining Boids.
 //
 //    Think of a way to achieve what Rule 1 needs to do but more
 //  efficiently! - this applies to Rules 2, 3, and 0 as well!
 //
 ///////////////////////////////////////////////////////////////////////////

 ///////////////////////////////////////////////////////////////////////////////
 // Boid update Rule 2:
 //  Boids avoid (he he he!) colliding with each other!
 //
 //  * For *each* Boid 'j' within distance r_rule2 from the current Boid 'i'
 //    - Compute a vector from Boid 'i' to Boid 'j'. Call this v2
 //    - Update Boid the velocity for Boid 'i' so that v' -= k_rule2*V2
 //      (Boid 'i' will try to move away from Boid 'j')
 //
 //  Just like for Rule 1, the parameters r_rule2 and k_rule2 are set by
 //  the command line and can be changed via keyboard.
 //
 //  * You need to implement this *
 //////////////////////////////////////////////////////////////////////////////
  int j;
  for (j = 0; j < nBoids; j++) {
    float v2x = Boid_Location[j][0] - Boid_Location[i][0];
    float v2y = Boid_Location[j][1] - Boid_Location[i][1];
    float v2z = Boid_Location[j][2] - Boid_Location[i][2];
    if (sqrt(v2x*v2x + v2y*v2y + v2z*v2z) <= r_rule2) {
      Boid_Velocity[i][0] -= k_rule2*v2x;
      Boid_Velocity[i][1] -= k_rule2*v2y;
      Boid_Velocity[i][2] -= k_rule2*v2z;
    }
  }
 ///////////////////////////////////////////////////////////////////////////////
 // Boid update Rule 3:
 //  Boids fly together!!
 //
 //  In effect, Boids will try to match the velovity of all other Boids within
 // their group.
 //
 //  * Compute the average velocity for all Boids within a radius r_rule3 from
 //    the current Boid, call the average velocity v3.
 //  * Update the velocity for Boid 'i' so that v'+= k_rule3*v3
 //
 //  r_rule3 and k_rule3 are set by command line and can be changed via keyboard
 //
 //  * You need to implement this *
 ///////////////////////////////////////////////////////////////////////////////
  int m;
  float tolvx = 0;
  float tolvy = 0;
  float tolvz = 0;
  float k3count = 0;
  for (m = 0; m < nBoids; m++) {
    float dx = Boid_Location[m][0] - Boid_Location[i][0];
    float dy = Boid_Location[m][1] - Boid_Location[i][1];
    float dz = Boid_Location[m][2] - Boid_Location[i][2];
    if (sqrt(dx*dx + dy*dy + dz*dz) <= r_rule3) {
      tolvx += Boid_Velocity[m][0];
      tolvy += Boid_Velocity[m][1];
      tolvz += Boid_Velocity[m][2];
      k3count += 1;
    }
  }

  float v3x = tolvx/k3count;
  float v3y = tolvy/k3count;
  float v3z = tolvz/k3count;

  Boid_Velocity[i][0] += k_rule3*v3x;
  Boid_Velocity[i][1] += k_rule3*v3y;
  Boid_Velocity[i][2] += k_rule3*v3z;
 
 ///////////////////////////////////////////////////////////////////////////////
 // The view box is -50 to +50 along each direction, try to keep Boids within
 // the view box - DO NOT CHANGE THIS
 //////////////////////////////////////////////////////////////////////////////
 if (Boid_Location[i][0]<-50) Boid_Velocity[i][0]+=1.0;
 if (Boid_Location[i][0]>50) Boid_Velocity[i][0]-=1.0;
 if (Boid_Location[i][1]<-50) Boid_Velocity[i][1]+=1.0;
 if (Boid_Location[i][1]>50) Boid_Velocity[i][1]-=1.0;
 if (Boid_Location[i][2]<-50) Boid_Velocity[i][2]+=1.0;
 if (Boid_Location[i][2]>50) Boid_Velocity[i][2]-=1.0;

 //////////////////////////////////////////////////////////////////////////////
 // Velocity Limit: Try to keep the Boids from flying off
 // The speed clamping used here was determined 'experimentally', 
 // i.e. I tweaked it by hand! Don't expect miracles!
 //
 // CHANGE THIS ONLY AT YOUR OWN RISK! I recommend leaving it alone.
 //////////////////////////////////////////////////////////////////////////////
 Boid_Velocity[i][0]=sign(Boid_Velocity[i][0])*sqrt(fabs(Boid_Velocity[i][0]));
 Boid_Velocity[i][1]=sign(Boid_Velocity[i][1])*sqrt(fabs(Boid_Velocity[i][1]));
 Boid_Velocity[i][2]=sign(Boid_Velocity[i][2])*sqrt(fabs(Boid_Velocity[i][2]));

 //////////////////////////////////////////////////////////////////////////////
 // Paco's Rule Zero (not part of the standard Boids definitions)
 //
 //   Boids have inertia - they will keep going in their previous direction
 //    unless strong forces pull them differently.
 //
 //    * Update the velocity for this Boid so that v' += previous_v * k_rule0 
 //
 //  k_rule0 is set by the command line and can be updated via keyboard.
 //
 //  * You need to implement this *
 /////////////////////////////////////////////////////////////////////////////
  Boid_Velocity[i][0] += k_rule0*vxl;
  Boid_Velocity[i][1] += k_rule0*vyl;
  Boid_Velocity[i][2] += k_rule0*vzl;
 /////////////////////////////////////////////////////////////////////////////
 // QUESTION: Why add inertia at the end and
 //  not at the beginning?
 /////////////////////////////////////////////////////////////////////////////

 //////////////////////////////////////////////////////////////////////////////
 // Finally (phew!) update the position and colour for this Boid using your
 // updated Boid_Velocity.
 //
 // ****  DO NOT CHANGE THIS PART ****
 //////////////////////////////////////////////////////////////////////////////
 Boid_Location[i][0]+=Boid_Velocity[i][0];
 Boid_Location[i][1]+=Boid_Velocity[i][1];
 Boid_Location[i][2]+=Boid_Velocity[i][2];

 r2_x=Boid_Location[i][0];
 r2_y=Boid_Location[i][1];
 r2_z=Boid_Location[i][2];
 dist=(r2_x*r2_x)+(r2_y*r2_y)+(r2_z*r2_z);
 dist=sqrt(dist);
 if (dist>0)
 {
  float H;
  r2_x/=dist;
  r2_y/=dist;
  r2_z/=dist;
  H=(r2_z+1.0)/2;
  H*=(cos(atan2(r2_y,r2_x))+1.0)/2;
  HSV2RGB(H,.95,.95,&Boid_Color[i][0],&Boid_Color[i][1],&Boid_Color[i][2]);
 }

 ////////////////////////////////////////////////////////////////////////////
 // CRUNCHY:
 //
 //  Things you can add here to make the behaviour more interesting.
 //
 // - Follow the leader: Select a handful (1 to 5) boids randomly. 
 //   Add code so that nearby boids tend to move toward these
 //   'leaders'
 //
 // - Make the updates smoother: Idea, instead of having hard thresholds 
 //   on distances for the update computations (r_rule1, r_rule2,
 //   r_rule3), use a weighted computation where contributions are weighted by
 //   distance and the weight decays as a function of the corresponding r_rule
 //   parameter.
 //
 // - Add a few 'predatory boids'. Select a couple of boids randomly. 
 //   These become predators and the rest of the boids should have a strong 
 //   tendency to avoid them. The predatory boids should follow the standard 
 //   rules. 
 //
 // - Cleverly introduce some randomness - Boids are more interesting if
 //   there's carefully introduced randomness in the update process!
 //
 // 
 // - Make this fast and see how many Boids you can handle at a reasonable
 //   frame rate!
 //
 //////////////////////////////////////////////////////////////////////////

}
