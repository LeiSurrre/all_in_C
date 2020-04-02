/*
  CSC D18 - RayTracer code.

  Written Dec. 9 2010 - Jan 20, 2011 by F. J. Estrada
  Freely distributable for adacemic purposes only.

  Uses Tom F. El-Maraghi's code for computing inverse
  matrices. You will need to compile together with
  svdDynamic.c

  You need to understand the code provided in
  this file, the corresponding header file, and the
  utils.c and utils.h files. Do not worry about
  svdDynamic.c, we need it only to compute
  inverse matrices.

  You only need to modify or add code in sections
  clearly marked "TO DO" - remember to check what
  functionality is actually needed for the corresponding
  assignment!

  Last updated: Aug. 2017   - F.J.E.
*/

/*****************************************************************************
* COMPLETE THIS TEXT BOX:
*
* 1) Student Name: Yutong Chen
* 2) Student Name:		
*
* 1) Student number: 1001351066
* 2) Student number:
* 
* 1) UtorID: chenyut7
* 2) UtorID
* 
* We hereby certify that the work contained here is our own
*
* ___Yutong Chen______             _____________________
* (sign with your name)            (sign with your name)
********************************************************************************/

#include "utils.h"	// <-- This includes RayTracer.h

// A couple of global structures and data: An object list, a light list, and the
// maximum recursion depth
struct object3D *object_list;
struct pointLS *light_list;
struct textureNode *texture_list;
int MAX_DEPTH;

void buildScene(void)
{
#include "buildscene.c"		// <-- Import the scene definition! 
}

void rtShade(struct object3D *obj, struct point3D *p, struct point3D *n, struct ray3D *ray, int depth, double a, double b, struct colourRGB *col)
{
 // This function implements the shading model as described in lecture. It takes
 // - A pointer to the first object intersected by the ray (to get the colour properties)
 // - The coordinates of the intersection point (in world coordinates)
 // - The normal at the point
 // - The ray (needed to determine the reflection direction to use for the global component, as well as for
 //   the Phong specular component)
 // - The current racursion depth
 // - The (a,b) texture coordinates (meaningless unless texture is enabled)
 //
 // Returns:
 // - The colour for this ray (using the col pointer)
 //

 struct colourRGB tmp_col;	// Accumulator for colour components
 double R,G,B;			// Colour for the object in R G and B

 // This will hold the colour as we process all the components of
 // the Phong illumination model
 tmp_col.R=0;
 tmp_col.G=0;
 tmp_col.B=0;

 // use normal from normal map if exists
 if (obj->normalMap != NULL) {
  //fprintf(stderr, "normalmap\n");
  double nx, ny, nz;
  obj->textureMap(obj->normalMap,a,b,&nx,&ny,&nz);
  n = newPoint(nx, ny, nz);
  normalize(n);
 }


 // use alpha from alpha map if exists
 if (obj->alphaMap != NULL) {
  //fprintf(stderr, "alphamap\n");
  double tempA = 0;
  alphaMap(obj->alphaMap,a,b,&tempA);
  obj->alpha = tempA;
  //fprintf(stderr, "alphamap done\n");
 }


 if (obj->texImg==NULL)		// Not textured, use object colour
 {
    //fprintf(stderr, "no text map\n");
  R=obj->col.R;
  G=obj->col.G;
  B=obj->col.B;
 }
 else
 {
  // Get object colour from the texture given the texture coordinates (a,b), and the texturing function
  // for the object. Note that we will use textures also for Photon Mapping.
  if (a >= 0 && b >= 0) {
    //fprintf(stderr, "texture map\n");
    obj->textureMap(obj->texImg,a,b,&R,&G,&B);
  } else {
    //fprintf(stderr, "cyl top\n");
    R=obj->col.R;
    G=obj->col.G;
    B=obj->col.B;
  }
 }

 //////////////////////////////////////////////////////////////
 // TO DO: Implement this function. Refer to the notes for
 // details about the shading model.
 //////////////////////////////////////////////////////////////

  // ambient term
  double ambientR = obj->alb.ra * 1 * R;
  double ambientG = obj->alb.ra * 1 * G;
  double ambientB = obj->alb.ra * 1 * B;

  tmp_col.R = tmp_col.R + ambientR;
  tmp_col.G = tmp_col.G + ambientG;
  tmp_col.B = tmp_col.B + ambientB;

  //vector c
  struct point3D *c = newPoint(-ray->d.px, -ray->d.py, -ray->d.pz);
  normalize(c);

  double dotNC = dot(n, c);

  //vector ms
  struct point3D *ms = newPoint(ray->d.px + 2 * dotNC * n->px, ray->d.py + 2 * dotNC * n->py, ray->d.pz + 2 * dotNC * n->pz);
  normalize(ms);

  struct pointLS *curr_ls = light_list;
  while (curr_ls != NULL) {
    // vector s
    struct point3D s = curr_ls->p0;
    subVectors(p, &s);
    // ray with d not normalized for shadow check
    struct ray3D ps;
    initRay(&ps, p, &s);
    normalize(&s);

    // check shadow
    double lambda = -1;
    struct object3D *curr_obj = object_list;
    //int hitcount = 0;
    //int trans = 0;
    while (curr_obj!=NULL) {
      double curr_lambda;
      struct point3D curr_p;
      struct point3D curr_n;
      double curr_a;
      double curr_b; 
      curr_obj->intersect(curr_obj, &ps, &curr_lambda, &curr_p, &curr_n, &curr_a, &curr_b);
      if (curr_lambda > 0.00000001) {
        hitcount++;
        if ((lambda < 0) || (lambda > 0 && curr_lambda < lambda)) {
          lambda = curr_lambda;
          //trans = (curr_obj->alpha < 1) ? 1 : 0;
        }
      }
      curr_obj = curr_obj->next;
    }


    double dotNS = dot(n, &s);

    //vector m
    struct point3D *m = newPoint(-s.px + 2 * dotNS * n->px, -s.py + 2 * dotNS * n->py, -s.pz + 2 * dotNS * n->pz);
    normalize(m);

    double dotCM = dot(c, m);

    // diffuse term
    double diffuseR = obj->alb.rd * curr_ls->col.R * ((dotNS > 0) ? dotNS : 0) * R;
    double diffuseG = obj->alb.rd * curr_ls->col.G * ((dotNS > 0) ? dotNS : 0) * G;
    double diffuseB = obj->alb.rd * curr_ls->col.B * ((dotNS > 0) ? dotNS : 0) * B;

    // specular term
    double specularR = obj->alb.rs * curr_ls->col.R * pow(((dotCM > 0) ? dotCM : 0), obj->shinyness);
    double specularG = obj->alb.rs * curr_ls->col.G * pow(((dotCM > 0) ? dotCM : 0), obj->shinyness);
    double specularB = obj->alb.rs * curr_ls->col.B * pow(((dotCM > 0) ? dotCM : 0), obj->shinyness);


    if (lambda < 0.00000001 || lambda > 1) {
    //if (lambda > 0.00000001 && lambda < 1) {
      //fprintf(stderr, "producing shadow!!!!!!!!!!!!!!!!!!!!!! lambda %f\n", lambda);
      // tmp_col.R = 0;
      // tmp_col.G = 0;
      // tmp_col.B = 0;

      // hit transparent surface
      // if (hitcount == 1 && trans == 1) {
      //   tmp_col.R = tmp_col.R + diffuseR + specularR;
      //   tmp_col.G = tmp_col.G + diffuseG + specularG;
      //   tmp_col.B = tmp_col.B + diffuseB + specularB;
      // }
    //} else {
      //fprintf(stderr, "producing color??????????????????????? lambda %f\n", lambda);
      // tmp_col.R = diffuseR;
      // tmp_col.G = diffuseG;
      // tmp_col.B = diffuseB;
      // tmp_col.R = specularR;
      // tmp_col.G = specularG;
      // tmp_col.B = specularB;
      tmp_col.R = tmp_col.R + diffuseR + specularR;
      tmp_col.G = tmp_col.G + diffuseG + specularG;
      tmp_col.B = tmp_col.B + diffuseB + specularB;
    }

    // tmp_col.R = ms.px;
    // tmp_col.G = ms.py;
    // tmp_col.B = ms.pz;

    curr_ls = curr_ls->next;
  }


  // refraction
  if (obj->alpha < 0) obj->alpha = 0;
  if (obj->alpha < 1) {
    struct colourRGB rcol;
    rcol.R=0;
    rcol.G=0;
    rcol.B=0;

    // struct point3D *c = newPoint(-ray->d.px, -ray->d.py, -ray->d.pz);
    // normalize(c);
    // double dotNC = dot(&n, c);

    // double r = (dotNC > 0) ? 1/(obj->r_index) : (obj->r_index);
    // struct point3D *nn = newPoint(-n.px, -n.py, -n.pz);
    // double dotNNC = dot(c, nn);

    // double factor = r*dotNNC - sqrt(1-r*r*(1-dotNNC*dotNNC));
    // struct point3D *dt = newPoint(r*c->px + factor*n.px, r*c->py + factor*n.py, r*c->pz + factor*n.pz);

    struct point3D d = ray->d;
    //double r = (dot(n, &d) > 0) ? 1/(obj->r_index) : (obj->r_index);
    double r = 1/(obj->r_index);
    struct point3D *nn = newPoint(-n->px, -n->py, -n->pz);
    double dotNNB = abs(dot(&d, nn));
    //double r = (dot(n, c) < 0) ? 1/(obj->r_index) : (obj->r_index);
    //struct point3D *nn = newPoint(n->px, n->py, n->pz);
    //double dotNNB = dot(c, nn);

    // r*c - sqrt(1-r*r*(1-c*c))
    double factor = r*dotNNB - sqrt(1-r*r*(1-dotNNB*dotNNB));
    struct point3D *dt = newPoint(r*d.px + factor*n->px, r*d.py + factor*n->py, r*d.pz + factor*n->pz);
    normalize(dt);
    //struct point3D *dt = newPoint(r*c->px + factor*n->px, r*c->py + factor*n->py, r*c->pz + factor*n->pz);
    struct ray3D pdt;
    initRay(&pdt, p, dt);
    rayTrace(&pdt, depth + 1, &rcol, obj);

    if (rcol.R > 1) rcol.R = 1;
    if (rcol.G > 1) rcol.G = 1;
    if (rcol.B > 1) rcol.B = 1;
    if (rcol.R < 0) rcol.R = 0;
    if (rcol.G < 0) rcol.G = 0;
    if (rcol.B < 0) rcol.B = 0;

    tmp_col.R = obj->alpha*tmp_col.R + (1-obj->alpha)*rcol.R;
    tmp_col.G = obj->alpha*tmp_col.G + (1-obj->alpha)*rcol.G;
    tmp_col.B = obj->alpha*tmp_col.B + (1-obj->alpha)*rcol.B;
  }

  // global relfection
  struct colourRGB gcol;
  gcol.R=0;
  gcol.G=0;
  gcol.B=0;
  struct ray3D pms;
  initRay(&pms, p, ms);
  rayTrace(&pms, depth + 1, &gcol, obj);

  if (gcol.R > 1) gcol.R = 1;
  if (gcol.G > 1) gcol.G = 1;
  if (gcol.B > 1) gcol.B = 1;
  if (gcol.R < 0) gcol.R = 0;
  if (gcol.G < 0) gcol.G = 0;
  if (gcol.B < 0) gcol.B = 0;

  tmp_col.R = tmp_col.R + obj->alb.rg * gcol.R;
  tmp_col.G = tmp_col.G + obj->alb.rg * gcol.G;
  tmp_col.B = tmp_col.B + obj->alb.rg * gcol.B;

  if (tmp_col.R > 1) tmp_col.R = 1;
  if (tmp_col.G > 1) tmp_col.G = 1;
  if (tmp_col.B > 1) tmp_col.B = 1;
  if (tmp_col.R < 0) tmp_col.R = 0;
  if (tmp_col.G < 0) tmp_col.G = 0;
  if (tmp_col.B < 0) tmp_col.B = 0;
  *col = tmp_col;
  // Be sure to update 'col' with the final colour computed here!
  return;
}

void findFirstHit(struct ray3D *ray, double *lambda, struct object3D *Os, struct object3D **obj, struct point3D *p, struct point3D *n, double *a, double *b)
{
 // Find the closest intersection between the ray and any objects in the scene.
 // Inputs:
 //   *ray    -  A pointer to the ray being traced
 //   *Os     -  'Object source' is a pointer toward the object from which the ray originates. It is used for reflected or refracted rays
 //              so that you can check for and ignore self-intersections as needed. It is NULL for rays originating at the center of
 //              projection
 // Outputs:
 //   *lambda -  A pointer toward a double variable 'lambda' used to return the lambda at the intersection point
 //   **obj   -  A pointer toward an (object3D *) variable so you can return a pointer to the object that has the closest intersection with
 //              this ray (this is required so you can do the shading)
 //   *p      -  A pointer to a 3D point structure so you can store the coordinates of the intersection point
 //   *n      -  A pointer to a 3D point structure so you can return the normal at the intersection point
 //   *a, *b  -  Pointers toward double variables so you can return the texture coordinates a,b at the intersection point

 /////////////////////////////////////////////////////////////
 // TO DO: Implement this function. See the notes for
 // reference of what to do in here
 /////////////////////////////////////////////////////////////
  struct object3D *curr_obj = object_list;
  *lambda = -1;
  // if (Os != NULL) {
  //   // if Os is not NULL, need to see what happens ------------------!!!!!!!!!!!!!
  //   return;
  // } else {
    while (curr_obj!=NULL) {
      double curr_lambda;
      struct point3D curr_p;
      struct point3D curr_n;
      double curr_a;
      double curr_b; 
      curr_obj->intersect(curr_obj, ray, &curr_lambda, &curr_p, &curr_n, &curr_a, &curr_b);
      if (curr_lambda > 0.000001) {
        if ((*lambda < 0) || (*lambda > 0 && curr_lambda < *lambda)) {
          *lambda = curr_lambda;
          *obj = curr_obj;
          *p = curr_p;
          *n = curr_n;
          *a = curr_a;
          *b = curr_b;
        }
      }
      curr_obj = curr_obj->next;
    }
  //}
}

void rayTrace(struct ray3D *ray, int depth, struct colourRGB *col, struct object3D *Os)
{
 // Trace one ray through the scene.
 //
 // Parameters:
 //   *ray   -  A pointer to the ray being traced
 //   depth  -  Current recursion depth for recursive raytracing
 //   *col   - Pointer to an RGB colour structure so you can return the object colour
 //            at the intersection point of this ray with the closest scene object.
 //   *Os    - 'Object source' is a pointer to the object from which the ray 
 //            originates so you can discard self-intersections due to numerical
 //            errors. NULL for rays originating from the center of projection. 
 
 double lambda;		// Lambda at intersection
 double a,b;		// Texture coordinates
 struct object3D *obj;	// Pointer to object at intersection
 struct point3D p;	// Intersection point
 struct point3D n;	// Normal at intersection
 struct colourRGB I;	// Colour returned by shading function

 if (depth>MAX_DEPTH)	// Max recursion depth reached. Return invalid colour.
 {
  col->R=-1;
  col->G=-1;
  col->B=-1;
  return;
 }

 ///////////////////////////////////////////////////////
 // TO DO: Complete this function. Refer to the notes
 // if you are unsure what to do here.
 ///////////////////////////////////////////////////////
  findFirstHit(ray, &lambda, Os, &obj, &p, &n, &a, &b);
  if (lambda > 0.000001) {
    //if (obj != Os) {
    rtShade(obj, &p, &n, ray, depth, a, b, col);
    // } else {
    //   // refracted ray hit it self, only trace once
    //   rtShade(obj, &p, &n, ray, MAX_DEPTH, a, b, col);
    // }
    // col->R = (n.px + 1)/2;
    // col->G = (n.py + 1)/2;
    // col->B = (n.pz + 1)/2;
  } else {
    col->R=0;
    col->G=0;
    col->B=0;
  }
}

int main(int argc, char *argv[])
{
 // Main function for the raytracer. Parses input parameters,
 // sets up the initial blank image, and calls the functions
 // that set up the scene and do the raytracing.
 struct image *im;	// Will hold the raytraced image
 struct view *cam;	// Camera and view for this scene
 int sx;		// Size of the raytraced image
 int antialiasing;	// Flag to determine whether antialiaing is enabled or disabled
 char output_name[1024];	// Name of the output file for the raytraced .ppm image
 struct point3D e;		// Camera view parameters 'e', 'g', and 'up'
 struct point3D g;
 struct point3D up;
 double du, dv;			// Increase along u and v directions for pixel coordinates
 struct point3D pc,d;		// Point structures to keep the coordinates of a pixel and
				// the direction or a ray
 struct ray3D ray;		// Structure to keep the ray from e to a pixel
 struct colourRGB col;		// Return colour for raytraced pixels
 struct colourRGB background;   // Background colour
 int i,j;			// Counters for pixel coordinates
 unsigned char *rgbIm;

 if (argc<5)
 {
  fprintf(stderr,"RayTracer: Can not parse input parameters\n");
  fprintf(stderr,"USAGE: RayTracer size rec_depth antialias output_name\n");
  fprintf(stderr,"   size = Image size (both along x and y)\n");
  fprintf(stderr,"   rec_depth = Recursion depth\n");
  fprintf(stderr,"   antialias = A single digit, 0 disables antialiasing. Anything else enables antialiasing\n");
  fprintf(stderr,"   output_name = Name of the output file, e.g. MyRender.ppm\n");
  exit(0);
 }
 sx=atoi(argv[1]);
 MAX_DEPTH=atoi(argv[2]);
 if (atoi(argv[3])==0) antialiasing=0; else antialiasing=1;
 strcpy(&output_name[0],argv[4]);

 fprintf(stderr,"Rendering image at %d x %d\n",sx,sx);
 fprintf(stderr,"Recursion depth = %d\n",MAX_DEPTH);
 if (!antialiasing) fprintf(stderr,"Antialising is off\n");
 else fprintf(stderr,"Antialising is on\n");
 fprintf(stderr,"Output file name: %s\n",output_name);

 object_list=NULL;
 light_list=NULL;
 texture_list=NULL;

 // Allocate memory for the new image
 im=newImage(sx, sx);
 if (!im)
 {
  fprintf(stderr,"Unable to allocate memory for raytraced image\n");
  exit(0);
 }
 else rgbIm=(unsigned char *)im->rgbdata;

 ///////////////////////////////////////////////////
 // TO DO: You will need to implement several of the
 //        functions below. For Assignment 2, you can use
 //        the simple scene already provided. But
 //        for Assignment 3 you need to create your own
 //        *interesting* scene.
 ///////////////////////////////////////////////////
 buildScene();		// Create a scene. This defines all the
			// objects in the world of the raytracer

 //////////////////////////////////////////
 // TO DO: For Assignment 2 you can use the setup
 //        already provided here. For Assignment 3
 //        you may want to move the camera
 //        and change the view parameters
 //        to suit your scene.
 //////////////////////////////////////////

 // Mind the homogeneous coordinate w of all vectors below. DO NOT
 // forget to set it to 1, or you'll get junk out of the
 // geometric transformations later on.

 // Camera center is at (0,0,-1)
 e.px=0;
 e.py=0;
 e.pz=-1;
 e.pw=1;

 // To define the gaze vector, we choose a point 'pc' in the scene that
 // the camera is looking at, and do the vector subtraction pc-e.
 // Here we set up the camera to be looking at the origin.
 g.px=0-e.px;
 g.py=0-e.py;
 g.pz=0-e.pz;
 g.pw=1;
 // In this case, the camera is looking along the world Z axis, so
 // vector w should end up being [0, 0, -1]

 // Define the 'up' vector to be the Y axis
 up.px=0;
 up.py=1;
 up.pz=0;
 up.pw=1;

 // Set up view with given the above vectors, a 4x4 window,
 // and a focal length of -1 (why? where is the image plane?)
 // Note that the top-left corner of the window is at (-2, 2)
 // in camera coordinates.
 cam=setupView(&e, &g, &up, -1, -2, 2, 4);

 if (cam==NULL)
 {
  fprintf(stderr,"Unable to set up the view and camera parameters. Our of memory!\n");
  cleanup(object_list,light_list, texture_list);
  deleteImage(im);
  exit(0);
 }

 // Set up background colour here
 background.R=0;
 background.G=0;
 background.B=0;

 // Do the raytracing
 //////////////////////////////////////////////////////
 // TO DO: You will need code here to do the raytracing
 //        for each pixel in the image. Refer to the
 //        lecture notes, in particular, to the
 //        raytracing pseudocode, for details on what
 //        to do here. Make sure you undersand the
 //        overall procedure of raytracing for a single
 //        pixel.
 //////////////////////////////////////////////////////
 du=cam->wsize/(sx-1);		// du and dv. In the notes in terms of wl and wr, wt and wb,
 dv=-cam->wsize/(sx-1);		// here we use wl, wt, and wsize. du=dv since the image is
				// and dv is negative since y increases downward in pixel
				// coordinates and upward in camera coordinates.

 fprintf(stderr,"View parameters:\n");
 fprintf(stderr,"Left=%f, Top=%f, Width=%f, f=%f\n",cam->wl,cam->wt,cam->wsize,cam->f);
 fprintf(stderr,"Camera to world conversion matrix (make sure it makes sense!):\n");
 printmatrix(cam->C2W);
 fprintf(stderr,"World to camera conversion matrix:\n");
 printmatrix(cam->W2C);
 fprintf(stderr,"\n");

 // flag for DOP, change to 1 to enable
 int dop = 0;
 if (dop) {
   // DOP variables
   double x1 = 5;
   double x0 = -(abs(cam->f))/(1-(abs(cam->f)/x1));
   fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! %f\n", x0);
   cam->f = x0;
 }

 fprintf(stderr,"Rendering row: ");
 for (j=0;j<sx;j++)		// For each of the pixels in the image
 {
  fprintf(stderr,"%d/%d, ",j,sx);
  for (i=0;i<sx;i++)
  {
    // fprintf(stderr,"  %d-%d/%d, ", j,i,sx);
    ///////////////////////////////////////////////////////////////////
    // TO DO - complete the code that should be in this loop to do the
    //         raytracing!
    ///////////////////////////////////////////////////////////////////

    // pijc
    struct point3D *pij = newPoint(cam->wl + i * du, cam->wt + j * dv, cam->f);

    //pijw
    matVecMult(cam->C2W, pij);

    // d = p - e normalized
    subVectors(&e, pij);
    normalize(pij);
    initRay(&ray, &e, pij);
    rayTrace(&ray, 0, &col, NULL);
    if (dop) {
      double lensr = 0.01;
      struct object3D *o = newPlane(.75,.75,.1,.05,.75,.75,.75,1,1,5);
      Scale(o,50,50,50);
      Translate(o,0,0,5);
      invert(&o->T[0][0],&o->Tinv[0][0]);

      double lambda;   // Lambda at intersection
      double a,b;    // Texture coordinates
      struct point3D px1;  // Intersection point
      struct point3D n;
      o->intersect(o, &ray, &lambda, &px1, &n, &a, &b);
      if (lambda > 0.0000001) {
        for (int nop = 0; nop < 9; nop++) {
          double rand1 = drand48() * lensr;
          double rand2 = drand48() * 2 * PI;
          struct point3D *pop = newPoint(e.px + rand1 * cos(rand2), e.py + rand1 * sin(rand2), e.pz);

          matVecMult(cam->C2W, pop);

          struct colourRGB coldop;
          struct ray3D raydop;
          subVectors(pop, &px1);
          normalize(&px1);
          initRay(&raydop, pop, &px1);
          rayTrace(&raydop, 0, &coldop, NULL);
          col.R += coldop.R;
          col.G += coldop.G;
          col.B += coldop.B;
        }
        col.R = 0.1 * col.R;
        col.G = 0.1 * col.G;
        col.B = 0.1 * col.B;
      }
    } else {
      //fprintf(stderr, "before r %f g %f b %f\n", col.R, col.G, col.B);
      // antialiasing for 10 samples
      if (antialiasing) {
        for (int aa = 0; aa < 9; aa++) {
          double rand1 = drand48() - 0.5;
          double rand2 = drand48() - 0.5;
          struct point3D *paa = newPoint(cam->wl + i * du + rand1 * du, cam->wt + j * dv + rand2 * dv, cam->f);
          //fprintf(stderr, "rand1 %f rand2 %f du %f dv %f pij %f %f paa %f %f \n", rand1*du, rand2*dv, du, dv, pij.px, pij.py, paa.px, paa.py);

          matVecMult(cam->C2W, paa);

          struct colourRGB colaa;
          struct ray3D rayaa;
          subVectors(&e, paa);
          normalize(paa);
          initRay(&rayaa, &e, paa);
          rayTrace(&rayaa, 0, &colaa, NULL);
          col.R += colaa.R;
          col.G += colaa.G;
          col.B += colaa.B;
        }
        col.R = 0.1 * col.R;
        col.G = 0.1 * col.G;
        col.B = 0.1 * col.B;
        //fprintf(stderr, "after r %f g %f b %f\n", col.R, col.G, col.B);
      }
    }
 
    rgbIm[(j*sx + i)*3 + 0] = col.R * 255;
    rgbIm[(j*sx + i)*3 + 1] = col.G * 255;
    rgbIm[(j*sx + i)*3 + 2] = col.B * 255;
    //fprintf(stderr, "color r %f g %f b %f\n", (double)rgbIm[(j*sx + i)*3 + 0], (double)rgbIm[(j*sx + i)*3 + 1], (double)rgbIm[(j*sx + i)*3 + 2]);
  } // end for i
 } // end for j

 fprintf(stderr,"\nDone!\n");

 // Output rendered image
 imageOutput(im,output_name);

 // Exit section. Clean up and return.
 cleanup(object_list,light_list,texture_list);		// Object, light, and texture lists
 deleteImage(im);					// Rendered image
 free(cam);						// camera view
 exit(0);
}

