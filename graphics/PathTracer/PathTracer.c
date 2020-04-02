/*
  CSC D18 - Path Tracer code.

  Derived from the ray tracer starter code. Most function 
  names are identical, though in practice the implementation
  should be much simpler!

  You only need to modify or add code in sections
  clearly marked "TO DO" - remember to check what
  functionality is actually needed for the corresponding
  assignment!

  Last updated: Aug. 2017   - F.J.E.
*/

/*****************************************************************************
* COMPLETE THIS TEXT BOX:
*
* 1) Student Name:		
* 2) Student Name:		
*
* 1) Student number:
* 2) Student number:
* 
* 1) UtorID
* 2) UtorID
* 
* We hereby certify that the work contained here is our own
*
* ____Yutong Chen_____             _____________________
* (sign with your name)            (sign with your name)
********************************************************************************/

#include "utils_path.h"			// <-- This includes PathTracer.h
//#define __USE_IS			// Use importance sampling for diffuse materials
//#define __USE_ES			// Use explicit light sampling
//#define __DEBUG			// <-- Use this to turn on/off debugging output

// A couple of global structures and data: An object list, a light list, and the
// maximum recursion depth
struct object3D *object_list;
struct textureNode *texture_list;
unsigned long int NUM_RAYS;
int NUM_LS = 0;
int IS = 1;
int MAX_DEPTH;

#include "buildScene.c"			// Import scene definition

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
}

void PathTrace(struct ray3D *ray, int depth, struct colourRGB *col, struct object3D *Os, int CEL)
{
 // Trace one light path through the scene.
 //
 // Parameters:
 //   *ray   -  A pointer to the ray being traced
 //   depth  -  Current recursion depth for recursive raytracing
 //   *col   - Pointer to an RGB colour structure so you can return the object colour
 //            at the intersection point of this ray with the closest scene object.
 //   *Os    - 'Object source' is a pointer to the object from which the ray 
 //            originates so you can discard self-intersections due to numerical
 //            errors. NULL for rays originating from the center of projection. 
 
 double lambda;			// Lambda at intersection
 double a,b;			// Texture coordinates
 struct object3D *obj;		// Pointer to object at intersection
 struct point3D p;		// Intersection point
 struct point3D n;		// Normal at intersection
 double R,G,B;			// Handy in case you need to keep track of some RGB colour value
 double dice;			// Handy to keep a random value
 struct ray3D *next_ray;	// For the new ray to be used in recursive calls
 
 if (depth>MAX_DEPTH)	// Max recursion depth reached. Return black (no light coming into pixel from this path).
 {
  //fprintf(stderr, "max d ir %f ig %f ib %f\n", ray->Ir, ray->Ig, ray->Ib);
  col->R=ray->Ir;	// These are accumulators, initialized at 0. Whenever we find a source of light these
  col->G=ray->Ig;	// get incremented accordingly. At the end of the recursion, we return whatever light
  col->B=ray->Ib;	// we accumulated into these three values.
  return;
 }

  // double rr = drand48() * 0.5;
  // if (ray->R < rr && ray->G < rr && ray->B < rr) {
  //   col->R=ray->Ir;
  //   col->G=ray->Ig;
  //   col->B=ray->Ib;
  //   return;
  // }

 ///////////////////////////////////////////////////////
 // TO DO: Complete this function. Refer to the notes
 // if you are unsure what to do here.
 ///////////////////////////////////////////////////////

  findFirstHit(ray, &lambda, Os, &obj, &p, &n, &a, &b);
  if (lambda > 0.000001) {
    // //if (obj != Os) {
    // rtShade(obj, &p, &n, ray, depth, a, b, col);
    // // } else {
    // //   // refracted ray hit it self, only trace once
    // //   rtShade(obj, &p, &n, ray, MAX_DEPTH, a, b, col);
    // // }
    // // col->R = (n.px + 1)/2;
    // // col->G = (n.py + 1)/2;
    // // col->B = (n.pz + 1)/2;

    if (obj->isLightSource == 1) {
      struct point3D r = ray->d;
      // r.px = ray->d.px;
      // r.py = ray->d.py;
      // r.pz = ray->d.pz;
      // r.pz = 1;
      double dotNR = abs(dot(&n, &r));
      
      // return at light source
      col->R = ray->R * obj->col.R * dotNR + ray->Ir;
      col->G = ray->G * obj->col.G * dotNR + ray->Ig;
      col->B = ray->B * obj->col.B * dotNR + ray->Ib;
      //fprintf(stderr, "obj r %f obj g %f obj b %f\n", col->R, col->G, col->B);
      return;
    }

    if (obj->texImg==NULL)   // Not textured, use object colour
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

    double dice = drand48();
    if (dice < obj->diffPct) {
      // diffuse obj
      struct point3D d;

      if (IS > 0) {
        cosWeightedSample(&n, &d);
      } else {
        d.px = drand48() - 0.5;
        d.py = drand48() - 0.5;
        d.pz = drand48() - 0.5;
        d.pw = 1;
      }

      normalize(&d);

      if (dot(&n, &d) < 0) {
        d.px = -d.px;
        d.py = -d.py;
        d.pz = -d.pz;
      }

      struct ray3D rayDiff;
      initRay(&rayDiff,&p,&d);
      NUM_RAYS++;

      double dotND = dot(&n, &d);
      rayDiff.R = ray->R * R * dotND;
      rayDiff.G = ray->G * G * dotND;
      rayDiff.B = ray->B * B * dotND;

    //fprintf(stderr, "dot nd %f ray r %f ray g %f ray b %f res r %f res g %f res b %f\n", dotND, ray->R, ray->G, ray->B, rayDiff->R, rayDiff->G, rayDiff->B);
      int ignore = 0;
      if (CEL == 1 && NUM_LS > 0) { // add explicit light sampling if signaled
        struct object3D *es_ls;
        // random light source
        int es_dice = floor(drand48()*NUM_LS);
        struct object3D *curr_obj = object_list;
        int count = 0;
        while (curr_obj!=NULL) {
          if (curr_obj->isLightSource) {
            if (count == es_dice) {
              es_ls = curr_obj;
            }
            count++;
          }
          curr_obj = curr_obj->next;
        }

        double esx, esy, esz;
        es_ls->randomPoint(es_ls, &esx, &esy, &esz);

        // vector = point on es ls - p
        struct point3D *vpes = newPoint(esx, esy, esz);
        subVectors(&p, vpes);
        normalize(vpes);
        // ray with d not normalized for direct hit check
        struct ray3D pvpes;
        initRay(&pvpes, &p, vpes);
        NUM_RAYS++;

        double es_lambda = -1;
        struct object3D *es_obj;
        struct point3D es_p;
        struct point3D es_n;
        double es_a, es_b;
        findFirstHit(&pvpes, &es_lambda, NULL, &es_obj, &es_p, &es_n, &es_a, &es_b);

        if (es_ls == es_obj) {
          //double len = length(vpes);
          //normalize(vpes);
          double dotNVPES = abs(dot(&n, vpes));
          //double weight = fmin(1, 4*abs(dotNVPES)*abs(dot(&es_n, vpes))/(len*len)); //!!!!!!!!!!!!!!!!!!!!!!!!!!
          double weight = fmin(1, 2*PI*es_ls->LSweight*abs(dotNVPES)*abs(dot(&es_n, vpes))/pow(es_lambda, 2));
          rayDiff.Ir = ray->Ir + ray->R * R * es_ls->col.R * weight;
          rayDiff.Ig = ray->Ig + ray->G * G * es_ls->col.G * weight;
          rayDiff.Ib = ray->Ib + ray->B * B * es_ls->col.B * weight;
          //fprintf(stderr, "ir %f ig %f ib %f weight %f\n", rayDiff.Ir, rayDiff.Ig, rayDiff.Ib, weight);
          //fprintf(stderr, "ir %f ig %f ib %f weight %f\n", es_ls->col.R, es_ls->col.G, es_ls->col.B, weight);

          double nxt_lambda = -1;
          struct object3D *nxt_obj;
          struct point3D nxt_p;
          struct point3D nxt_n;
          double nxt_a, nxt_b;
          findFirstHit(&rayDiff, &nxt_lambda, NULL, &nxt_obj, &nxt_p, &nxt_n, &nxt_a, &nxt_b);
          if (es_ls == nxt_obj) {
            //fprintf(stderr, "ignore -------!!!!!!\n");
            ignore = 1;
          }
        }
      }

      double rr = drand48() * 0.5;
      if (rayDiff.R > rr || rayDiff.G > rr || rayDiff.B > rr) {
        if (ignore) {
          // col->R = rayDiff.R + rayDiff.Ir;
          // col->G = rayDiff.G + rayDiff.Ig;
          // col->B = rayDiff.B + rayDiff.Ib;
          col->R = rayDiff.Ir;
          col->G = rayDiff.Ig;
          col->B = rayDiff.Ib;
          //fprintf(stderr, "obj r %f obj g %f obj b %f\n", col->R, col->G, col->B);
        } else {
          PathTrace(&rayDiff, depth+1, col, obj, CEL);
          col->R = col->R/dotND;
          col->G = col->G/dotND;
          col->B = col->B/dotND;
        }
      } else {
        col->R=ray->Ir;
        col->G=ray->Ig;
        col->B=ray->Ib;
      }

    } else if (dice >= obj->diffPct && dice < obj->diffPct+obj->reflPct) {
      // reflective obj

      //vector c
      struct point3D *c = newPoint(-ray->d.px, -ray->d.py, -ray->d.pz);
      normalize(c);
      double dotNC = dot(&n, c);

      //vector ms
      struct point3D *ms = newPoint(ray->d.px + 2 * dotNC * n.px, ray->d.py + 2 * dotNC * n.py, ray->d.pz + 2 * dotNC * n.pz);
      normalize(ms);

      if (obj->refl_sig > 0 && obj->refl_sig < 1) {
        //double vari = obj->refl_sig * obj->refl_sig;
        //ms = newPoint(ms->px + (drand48()-0.5)*vari, ms->py + (drand48()-0.5)*vari, ms->pz + (drand48()-0.5)*vari);
        // ms = newPoint(ms->px + (drand48()-0.5)*obj->refl_sig,
        //           ms->py + (drand48()-0.5)*obj->refl_sig, ms->pz + (drand48()-0.5)*obj->refl_sig);
        // normalize(ms);
        // https://en.wikipedia.org/wiki/Normal_distribution
        double varix = exp(1)*drand48() * cos(2*PI*drand48())*obj->refl_sig;
        double variy = exp(1)*drand48() * cos(2*PI*drand48())*obj->refl_sig;
        double variz = exp(1)*drand48() * cos(2*PI*drand48())*obj->refl_sig;
        ms = newPoint(ms->px + varix, ms->py + variy, ms->pz + variz);
        normalize(ms);
      }

      struct ray3D rayRefl;
      initRay(&rayRefl,&p,ms);
      NUM_RAYS++;

      double dotNMS = dot(&n, ms);
      rayRefl.R = ray->R * R * dotNMS;
      rayRefl.G = ray->G * G * dotNMS;
      rayRefl.B = ray->B * B * dotNMS;
      rayRefl.Ir = ray->Ir;
      rayRefl.Ig = ray->Ig;
      rayRefl.Ib = ray->Ib;

      double rr = drand48() * 0.5;
      if (rayRefl.R > rr || rayRefl.G > rr || rayRefl.B > rr) {
        PathTrace(&rayRefl, depth+1, col, obj, CEL);
      } else {
        col->R=ray->Ir;
        col->G=ray->Ig;
        col->B=ray->Ib;
      }
    } else {
      // refractive obj

      struct point3D d = ray->d;
      //double r = (dot(&n, &d) > 0) ? 1/(obj->r_index) : (obj->r_index);
      double r = 1/(obj->r_index);
      struct point3D *nn = newPoint(-n.px, -n.py, -n.pz);
      double dotNND = abs(dot(&d, nn));

      double factor = r*dotNND - sqrt(1-r*r*(1-dotNND*dotNND));
      struct point3D *dt = newPoint(r*d.px + factor*n.px, r*d.py + factor*n.py, r*d.pz + factor*n.pz);
      normalize(dt);

      struct ray3D rayTran;
      initRay(&rayTran,&p,dt);
      NUM_RAYS++;

      double dotNDT = abs(dot(&n, dt));
      rayTran.R = ray->R * R * dotNDT;
      rayTran.G = ray->G * G * dotNDT;
      rayTran.B = ray->B * B * dotNDT;

      // rayTran.R = ray->R * R;
      // rayTran.G = ray->G * G;
      // rayTran.B = ray->B * B;
      rayTran.Ir = ray->Ir;
      rayTran.Ig = ray->Ig;
      rayTran.Ib = ray->Ib;

      double rr = drand48() * 0.5;
      if (rayTran.R > rr || rayTran.G > rr || rayTran.B > rr) {
        PathTrace(&rayTran, depth+1, col, obj, CEL);
      } else {
        col->R=ray->Ir;
        col->G=ray->Ig;
        col->B=ray->Ib;
      }
    }

    return;
  } else {
    // return at no obj hit
    // col->R=0;
    // col->G=0;
    // col->B=0;
    //fprintf(stderr, "back ir %f ig %f ib %f\n", ray->Ir, ray->Ig, ray->Ib);
    col->R=ray->Ir;
    col->G=ray->Ig;
    col->B=ray->Ib;
    return;
  }
  return;
}

int main(int argc, char *argv[])
{
 // Main function for the path tracer. Parses input parameters,
 // sets up the initial blank image, and calls the functions
 // that set up the scene and do the raytracing.
 struct image *im;		// Will hold the final image
 struct view *cam;		// Camera and view for this scene
 int sx;			// Size of the  image
 int num_samples;		// Number of samples to use per pixel
 char output_name[1024];	// Name of the output file for the .ppm image file
 struct point3D e;		// Camera view parameters 'e', 'g', and 'up'
 struct point3D g;
 struct point3D up;
 double du, dv;			// Increase along u and v directions for pixel coordinates
 struct point3D pc,d;		// Point structures to keep the coordinates of a pixel and
				// the direction or a ray
 struct ray3D ray;		// Structure to keep the ray from e to a pixel
 struct colourRGB col;		// Return colour for pixels
 int i,j,k;			// Counters for pixel coordinates and samples
 double *rgbIm;			// Image is now double precision floating point since we
				// will be accumulating brightness differences with a 
				// wide dynamic range
 struct object3D *obj;		// Will need this to process lightsource weights
 double *wght;			// Holds weights for each pixel - to provide log response
 double pct,wt;
 
 time_t t1,t2;
 FILE *f;
				
 if (argc<5)
 {
  fprintf(stderr,"PathTracer: Can not parse input parameters\n");
  fprintf(stderr,"USAGE: PathTracer size rec_depth num_samples output_name\n");
  fprintf(stderr,"   size = Image size (both along x and y)\n");
  fprintf(stderr,"   rec_depth = Recursion depth\n");
  fprintf(stderr,"   num_samples = Number of samples per pixel\n");
  fprintf(stderr,"   output_name = Name of the output file, e.g. MyRender.ppm\n");
  exit(0);
 }
 sx=atoi(argv[1]);
 MAX_DEPTH=atoi(argv[2]);
 num_samples=atoi(argv[3]);
 strcpy(&output_name[0],argv[4]);

 fprintf(stderr,"Rendering image at %d x %d\n",sx,sx);
 fprintf(stderr,"Recursion depth = %d\n",MAX_DEPTH);
 fprintf(stderr,"NUmber of samples = %d\n",num_samples);
 fprintf(stderr,"Output file name: %s\n",output_name);

 object_list=NULL;
 texture_list=NULL;

 // Allocate memory for the new image
 im=newImage(sx, sx);
 wght=(double *)calloc(sx*sx,sizeof(double));
 if (!im||!wght)
 {
  fprintf(stderr,"Unable to allocate memory for image\n");
  exit(0);
 }
 else rgbIm=(double *)im->rgbdata;
 for (i=0;i<sx*sx;i++) *(wght+i)=1.0;
 
 buildScene();		// Create a scene. 
 
 // Mind the homogeneous coordinate w of all vectors below. DO NOT
 // forget to set it to 1, or you'll get junk out of the
 // geometric transformations later on.

 // Camera center
 e.px=0;
 e.py=0;
 e.pz=-15;
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
 cam=setupView(&e, &g, &up, -3, -2, 2, 4);

 if (cam==NULL)
 {
  fprintf(stderr,"Unable to set up the view and camera parameters. Our of memory!\n");
  cleanup(object_list, texture_list);
  deleteImage(im);
  exit(0);
 }

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

 // Update light source weights - will give you weights for each light source that add up to 1
 obj=object_list;
 pct=0;
 while (obj!=NULL)
 {
  if (obj->isLightSource)
   pct+=obj->LSweight;
  obj=obj->next;
 }
 obj=object_list;
 while (obj!=NULL)
 {
  if (obj->isLightSource)
  {
   NUM_LS++;
   obj->LSweight/=pct;
  }
  obj=obj->next;
 }
 fprintf(stderr,"\n");

 NUM_RAYS=0;

 t1=time(NULL);

 fprintf(stderr,"Rendering pass... ");
 for (k=0; k<num_samples; k++)
 {
  fprintf(stderr,"%d/%d, ",k,num_samples);
#pragma omp parallel for schedule(dynamic,1) private(i,j,pc,wt,ray,col,d)
  for (j=0;j<sx;j++)		// For each of the pixels in the image
  {
   for (i=0;i<sx;i++)
   {
    // Random sample within the pixel's area
    pc.px=(cam->wl+((i+(drand48()-.5))*du));
    pc.py=(cam->wt+((j+(drand48()-.5))*dv));
    pc.pz=cam->f;
    pc.pw=1;

    // Convert image plane sample coordinates to world coordinates
    matVecMult(cam->C2W,&pc);

    // Now compute the ray direction
    memcpy(&d,&pc,sizeof(struct point3D));
    subVectors(&cam->e,&d);		// Direction is d=pc-e
    normalize(&d);

    // Create a ray and do the raytracing for this pixel.
    initRay(&ray, &pc,&d);
    NUM_RAYS++;

    wt=*(wght+i+(j*sx));
    PathTrace(&ray,1, &col,NULL,1);
    //PathTrace(&ray,1, &col,NULL,0);
    //fprintf(stderr, "obj r %f obj g %f obj b %f\n", col.R, col.G, col.B);
    (*(rgbIm+((i+(j*sx))*3)+0))+=col.R*pow(2,-log(wt));
    (*(rgbIm+((i+(j*sx))*3)+1))+=col.G*pow(2,-log(wt));
    (*(rgbIm+((i+(j*sx))*3)+2))+=col.B*pow(2,-log(wt));
    wt+=col.R;
    wt+=col.G;
    wt+=col.B;
    *(wght+i+(j*sx))=wt;
   } // end for i
  } // end for j  
  if (k%25==0)  dataOutput(rgbIm,sx,&output_name[0]);  		// Update output image every 25 passes
 } // End for k 
 t2=time(NULL);

 fprintf(stderr,"\nDone!\n");

 dataOutput(rgbIm,sx,&output_name[0]);
 
 fprintf(stderr,"Total number of rays created: %ld\n",NUM_RAYS);
 fprintf(stderr,"Rays per second: %f\n",(double)NUM_RAYS/(double)difftime(t2,t1));

 // Exit section. Clean up and return.
 cleanup(object_list,texture_list);			// Object and texture lists
 deleteImage(im);					// Rendered image
 free(cam);						// camera view
 free(wght);
 exit(0);
}

