/*
   utils.c - F.J. Estrada, Dec. 9, 2010

   Utilities for the ray tracer. You will need to complete
   some of the functions in this file. Look for the sections
   marked "TO DO". Be sure to read the rest of the file and
   understand how the entire code works.

   HOWEVER: Note that there are a lot of incomplete functions
            that will only be used for the advanced ray tracer!
	    So, read the handout carefully and implement only
	    the code you need for the corresponding assignment.
   
   Last updated: Aug. 2017  -  F.J.E.
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
* ____Yutong Chen_____             _____________________
* (sign with your name)            (sign with your name)
********************************************************************************/

#include "utils.h"

// A useful 4x4 identity matrix which can be used at any point to
// initialize or reset object transformations
double eye4x4[4][4]={{1.0, 0.0, 0.0, 0.0},
                    {0.0, 1.0, 0.0, 0.0},
                    {0.0, 0.0, 1.0, 0.0},
                    {0.0, 0.0, 0.0, 1.0}};

/////////////////////////////////////////////
// Primitive data structure section
/////////////////////////////////////////////
struct point3D *newPoint(double px, double py, double pz)
{
 // Allocate a new point structure, initialize it to
 // the specified coordinates, and return a pointer
 // to it.

 struct point3D *pt=(struct point3D *)calloc(1,sizeof(struct point3D));
 if (!pt) fprintf(stderr,"Out of memory allocating point structure!\n");
 else
 {
  pt->px=px;
  pt->py=py;
  pt->pz=pz;
  pt->pw=1.0;
 }
 return(pt);
}

struct pointLS *newPLS(struct point3D *p0, double r, double g, double b)
{
 // Allocate a new point light sourse structure. Initialize the light
 // source to the specified RGB colour
 // Note that this is a point light source in that it is a single point
 // in space, if you also want a uniform direction for light over the
 // scene (a so-called directional light) you need to place the
 // light source really far away.

 struct pointLS *ls=(struct pointLS *)calloc(1,sizeof(struct pointLS));
 if (!ls) fprintf(stderr,"Out of memory allocating light source!\n");
 else
 {
  memcpy(&ls->p0,p0,sizeof(struct point3D));	// Copy light source location

  ls->col.R=r;					// Store light source colour and
  ls->col.G=g;					// intensity
  ls->col.B=b;
 }
 return(ls);
}

/////////////////////////////////////////////
// Ray and normal transforms
/////////////////////////////////////////////
inline void rayTransform(struct ray3D *ray_orig, struct ray3D *ray_transformed, struct object3D *obj)
{
 // Transforms a ray using the inverse transform for the specified object. This is so that we can
 // use the intersection test for the canonical object. Note that this has to be done carefully!

  struct point3D newp0 = ray_orig->p0;
  struct point3D newd = ray_orig->d;
  newd.px = newd.px + obj->T[0][3];
  newd.py = newd.py + obj->T[1][3];
  newd.pz = newd.pz + obj->T[2][3];
  matVecMult(obj->Tinv, &newp0);
  matVecMult(obj->Tinv, &newd);

  ray_transformed->p0 = newp0;
  ray_transformed->d = newd;
  ray_transformed->rayPos = ray_orig->rayPos;
}

inline void normalTransform(struct point3D *n_orig, struct point3D *n_transformed, struct object3D *obj)
{
 // Computes the normal at an affinely transformed point given the original normal and the
 // object's inverse transformation. From the notes:
 // n_transformed=A^-T*n normalized.

  // inverse transpose
  double C[4][4];
  int i,j;

  for (i=0;i<3;i++)
    for (j=0;j<3;j++)
      C[i][j]=obj->Tinv[j][i];

  C[0][3] = 0;
  C[1][3] = 0;
  C[2][3] = 0;
  C[3][0] = 0;
  C[3][1] = 0;
  C[3][2] = 0;
  C[3][3] = 1;

  struct point3D newn = *n_orig;
  matVecMult(C, &newn);
  normalize(&newn);
  *n_transformed = newn;
}

/////////////////////////////////////////////
// Object management section
/////////////////////////////////////////////
void insertObject(struct object3D *o, struct object3D **list)
{
 if (o==NULL) return;
 // Inserts an object into the object list.
 if (*(list)==NULL)
 {
  *(list)=o;
  (*(list))->next=NULL;
 }
 else
 {
  o->next=(*(list))->next;
  (*(list))->next=o;
 }
}

struct object3D *newPlane(double ra, double rd, double rs, double rg, double r, double g, double b, double alpha, double r_index, double shiny)
{
 // Intialize a new plane with the specified parameters:
 // ra, rd, rs, rg - Albedos for the components of the Phong model
 // r, g, b, - Colour for this plane
 // alpha - Transparency, must be set to 1 unless you are doing refraction
 // r_index - Refraction index if you are doing refraction.
 // shiny - Exponent for the specular component of the Phong model
 //
 // The plane is defined by the following vertices (CCW)
 // (1,1,0), (-1,1,0), (-1,-1,0), (1,-1,0)
 // With normal vector (0,0,1) (i.e. parallel to the XY plane)

 struct object3D *plane=(struct object3D *)calloc(1,sizeof(struct object3D));

 if (!plane) fprintf(stderr,"Unable to allocate new plane, out of memory!\n");
 else
 {
  plane->alb.ra=ra;
  plane->alb.rd=rd;
  plane->alb.rs=rs;
  plane->alb.rg=rg;
  plane->col.R=r;
  plane->col.G=g;
  plane->col.B=b;
  plane->alpha=alpha;
  plane->r_index=r_index;
  plane->shinyness=shiny;
  plane->intersect=&planeIntersect;
  plane->surfaceCoords=&planeCoordinates;
  plane->randomPoint=&planeSample;
  plane->texImg=NULL;
  plane->photonMap=NULL;
  plane->normalMap=NULL;
  memcpy(&plane->T[0][0],&eye4x4[0][0],16*sizeof(double));
  memcpy(&plane->Tinv[0][0],&eye4x4[0][0],16*sizeof(double));
  plane->textureMap=&texMap;
  plane->frontAndBack=1;
  plane->photonMapped=0;
  plane->normalMapped=0;
  plane->isCSG=0;
  plane->isLightSource=0;
  plane->CSGnext=NULL;
  plane->next=NULL;
}
 return(plane);
}

struct object3D *newSphere(double ra, double rd, double rs, double rg, double r, double g, double b, double alpha, double r_index, double shiny)
{
 // Intialize a new sphere with the specified parameters:
 // ra, rd, rs, rg - Albedos for the components of the Phong model
 // r, g, b, - Colour for this plane
 // alpha - Transparency, must be set to 1 unless you are doing refraction
 // r_index - Refraction index if you are doing refraction.
 // shiny -Exponent for the specular component of the Phong model
 //
 // This is assumed to represent a unit sphere centered at the origin.
 //

 struct object3D *sphere=(struct object3D *)calloc(1,sizeof(struct object3D));

 if (!sphere) fprintf(stderr,"Unable to allocate new sphere, out of memory!\n");
 else
 {
  sphere->alb.ra=ra;
  sphere->alb.rd=rd;
  sphere->alb.rs=rs;
  sphere->alb.rg=rg;
  sphere->col.R=r;
  sphere->col.G=g;
  sphere->col.B=b;
  sphere->alpha=alpha;
  sphere->r_index=r_index;
  sphere->shinyness=shiny;
  sphere->intersect=&sphereIntersect;
  sphere->surfaceCoords=&sphereCoordinates;
  sphere->randomPoint=&sphereSample;
  sphere->texImg=NULL;
  sphere->photonMap=NULL;
  sphere->normalMap=NULL;
  memcpy(&sphere->T[0][0],&eye4x4[0][0],16*sizeof(double));
  memcpy(&sphere->Tinv[0][0],&eye4x4[0][0],16*sizeof(double));
  sphere->textureMap=&texMap;
  sphere->frontAndBack=0;
  sphere->photonMapped=0;
  sphere->normalMapped=0;
  sphere->isCSG=0;
  sphere->isLightSource=0;
  sphere->CSGnext=NULL;
  sphere->next=NULL; }
 return(sphere);
}

struct object3D *newCyl(double ra, double rd, double rs, double rg, double r, double g, double b, double alpha, double R_index, double shiny)
{
 ///////////////////////////////////////////////////////////////////////////////////////
 // TO DO:
 //	Complete the code to create and initialize a new cylinder object.
 ///////////////////////////////////////////////////////////////////////////////////////
  struct object3D *cylinder=(struct object3D *)calloc(1,sizeof(struct object3D));

  if (!cylinder) fprintf(stderr,"Unable to allocate new cylinder, out of memory!\n");
  else
  {
  cylinder->alb.ra=ra;
  cylinder->alb.rd=rd;
  cylinder->alb.rs=rs;
  cylinder->alb.rg=rg;
  cylinder->col.R=r;
  cylinder->col.G=g;
  cylinder->col.B=b;
  cylinder->alpha=alpha;
  cylinder->r_index=R_index;
  cylinder->shinyness=shiny;
  cylinder->intersect=&cylIntersect;
  cylinder->surfaceCoords=&cylCoordinates;
  cylinder->randomPoint=&cylSample;
  cylinder->texImg=NULL;
  cylinder->photonMap=NULL;
  cylinder->normalMap=NULL;
  memcpy(&cylinder->T[0][0],&eye4x4[0][0],16*sizeof(double));
  memcpy(&cylinder->Tinv[0][0],&eye4x4[0][0],16*sizeof(double));
  cylinder->textureMap=&texMap;
  cylinder->frontAndBack=0;
  cylinder->photonMapped=0;
  cylinder->normalMapped=0;
  cylinder->isCSG=0;
  cylinder->isLightSource=0;
  cylinder->CSGnext=NULL;
  cylinder->next=NULL; }
  return(cylinder);
}


///////////////////////////////////////////////////////////////////////////////////////
// TO DO:
//	Complete the functions that compute intersections for the canonical plane
//      and canonical sphere with a given ray. This is the most fundamental component
//      of the raytracer.
///////////////////////////////////////////////////////////////////////////////////////
void planeIntersect(struct object3D *plane, struct ray3D *ray, double *lambda, struct point3D *p, struct point3D *n, double *a, double *b)
{
 // Computes and returns the value of 'lambda' at the intersection
 // between the specified ray and the specified canonical plane.

  struct point3D normal;
  normal.px = 0;
  normal.py = 0;
  normal.pz = -1;
  normal.pw = 1;

  struct point3D p0;
  p0.px = 0;
  p0.py = 0;
  p0.pz = 0;
  p0.pw = 1;

  struct point3D unit_intersection;

  struct ray3D trans_ray;
  rayTransform(ray, &trans_ray, plane);
  struct point3D pa = trans_ray.p0;
  struct point3D d = trans_ray.d;

  if (dot(&d, &normal) == 0) {
    // fprintf(stderr, "In planeIntersect, ray parallel to plane.\n");
    *lambda = -1;
    return;
  }

  // p0 = (p0 - a)
  subVectors(&pa, &p0);
  // (p0 - a)*n/(d*n)
  double local_lambda = dot(&p0, &normal)/dot(&d, &normal);
  if (local_lambda > 0.000001) {
    trans_ray.rayPos(&trans_ray, local_lambda, &unit_intersection);
    if (unit_intersection.px >= -1 && unit_intersection.px <= 1 && unit_intersection.py >= -1 && unit_intersection.py <= 1) {
      *lambda = local_lambda;
      *a = 0.5 * (unit_intersection.px + 1);
      *b = -0.5 * (unit_intersection.py - 1);
      ray->rayPos(ray, *lambda, p);
      normalTransform(&normal, n, plane);
      normalize(n);
    } else {
      // fprintf(stderr, "In planeIntersect, no intersection found.\n");
      *lambda = -1;
      return;
    }
  } else {
    *lambda = -1;
    // fprintf(stderr, "In planeIntersect, no positive lambda.\n");
  }
  // missing the use of a, b here ---------------------!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

void sphereIntersect(struct object3D *sphere, struct ray3D *ray, double *lambda, struct point3D *p, struct point3D *n, double *a, double *b)
{
 // Computes and returns the value of 'lambda' at the intersection
 // between the specified ray and the specified canonical sphere.

  struct ray3D trans_ray;
  rayTransform(ray, &trans_ray, sphere);
  struct point3D pa = trans_ray.p0;
  struct point3D d = trans_ray.d;

  double A = dot(&d, &d);
  double B = dot(&pa, &d);
  double C = dot(&pa, &pa) - 1;
  double D = B*B - A*C;
  if (D < 0) {
    // fprintf(stderr, "In sphereIntersect, no lambda solution.\n");
    *lambda = -1;
    return;
  } else {
    double lambda1 = -B/A + sqrt(D)/A;
    double lambda2 = -B/A - sqrt(D)/A;
    if (lambda1 > 0.000001 || lambda2 > 0.000001) {
      if (lambda1 > 0 && lambda2 <= 0) {
        *lambda = lambda1;
      } else if (lambda2 > 0 && lambda1 <= 0) {
        *lambda = lambda2;
      } else {
        *lambda = lambda1 < lambda2 ? lambda1 : lambda2;
      }
      ray->rayPos(ray, *lambda, p);
      trans_ray.rayPos(&trans_ray, *lambda, n);
      // x = sqrt(r*r - u*u)cos(a)
      // z = u = cos(b)
      *a = (n->pz >= 0) ? acos(n->px/sqrt(1 - n->py*n->py)) : 2*PI - acos(n->px/sqrt(1 - n->py*n->py));
      *a = *a/(2*PI);
      *b = acos(n->py)/PI;
      normalTransform(n, n, sphere);
      //fprintf(stderr, "In sphereIntersect, normal point after %f %f %F.\n", n->px, n->py, n->pz);
    } else {
      // fprintf(stderr, "In sphereIntersect, no positive lambda.\n");
      *lambda = -1;
      return;
    }
  }
  // missing the use of a, b here ---------------------!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

void cylIntersect(struct object3D *cylinder, struct ray3D *ray, double *lambda, struct point3D *p, struct point3D *n, double *a, double *b)
{
 // Computes and returns the value of 'lambda' at the intersection
 // between the specified ray and the specified canonical cylinder.

  struct ray3D trans_ray;
  rayTransform(ray, &trans_ray, cylinder);
  struct point3D pa = trans_ray.p0;
  struct point3D d = trans_ray.d;;

  struct point3D normal_top;
  normal_top.px = 0;
  normal_top.py = 0;
  normal_top.pz = -1;
  normal_top.pw = 1;

  struct point3D normal_bot;
  normal_bot.px = 0;
  normal_bot.py = 0;
  normal_bot.pz = 1;
  normal_bot.pw = 1;

  struct point3D p0_top;
  p0_top.px = 0;
  p0_top.py = 0;
  p0_top.pz = 1;
  p0_top.pw = 1;

  struct point3D p0_bot;
  p0_bot.px = 0;
  p0_bot.py = 0;
  p0_bot.pz = 0;
  p0_bot.pw = 1;

  *lambda = -1;

  // top intersection
  if (dot(&d, &normal_top) != 0) {
    // p0 = (p0 - a)
    subVectors(&pa, &p0_top);
    // (p0 - a)*n/(d*n)
    double lambda_top = dot(&p0_top, &normal_top)/dot(&d, &normal_top);
    if (lambda_top > 0) {
      struct point3D top_intersect;
      trans_ray.rayPos(&trans_ray, lambda_top, &top_intersect);
      top_intersect.pz = 0;
      if (length(&top_intersect) <= 1) {
        if (*lambda < 0) {
          *lambda = lambda_top;
          *n = normal_top;
        } else {
          if (lambda_top < *lambda) {
            *lambda = lambda_top;
            *n = normal_top;
            // top no texture
            *a = -1;
            *b = -1;
          }
        }
      }
    }
  }

  // bot intersection
  if (dot(&d, &normal_bot) != 0) {
    // a = (p0 - a)
    subVectors(&pa, &p0_bot);
    // (p0 - a)*n/(d*n)
    double lambda_bot = dot(&p0_bot, &normal_bot)/dot(&d, &normal_bot);
    if (lambda_bot > 0) {
      struct point3D bot_intersect;
      trans_ray.rayPos(&trans_ray, lambda_bot, &bot_intersect);
      bot_intersect.pz = 0;
      if (length(&bot_intersect) <= 1) {
        if (*lambda < 0) {
          *lambda = lambda_bot;
          *n = normal_bot;
        } else {
          if (lambda_bot < *lambda) {
            *lambda = lambda_bot;
            *n = normal_bot;
            // bot no texture
            *a = -1;
            *b = -1;
          }
        }
      }
    }
  }

  // side surface intersection
  pa = trans_ray.p0;
  double ax = pa.px;
  double ay = pa.py;
  double dx = d.px;
  double dy = d.py;
  double A = dx*dx + dy*dy;
  double B = 2*ax*dx + 2*ay*dy;
  double C = ax*ax + ay*ay - 1;
  double quadratic = B*B - 4*A*C;
  if (quadratic >= 0) {
    // calculate lambda with the unit circle
    double lambda_side1 = (-B + sqrt(quadratic))/(2*A);
    double lambda_side2 = (-B - sqrt(quadratic))/(2*A);
    // calculate hight to check if point is on cylinder;
    if (lambda_side1 > 0.000001) {
      struct point3D side_intersect1;
      trans_ray.rayPos(&trans_ray, lambda_side1, &side_intersect1);
      if (side_intersect1.pz >= -1 && side_intersect1.pz <=1) {
        if (*lambda < 0) {
          *lambda = lambda_side1;
          *a = (side_intersect1.py >= 0) ? acos(n->px) : 2*PI - acos(n->px);
          *a = *a/(2*PI);
          *b = side_intersect1.pz;
          side_intersect1.pz = 0;
          *n = side_intersect1;
        } else {
          if (lambda_side1 < *lambda) {
            *lambda = lambda_side1;
            *a = (side_intersect1.py >= 0) ? acos(n->px) : 2*PI - acos(n->px);
            *a = *a/(2*PI);
            *b = side_intersect1.pz;
            side_intersect1.pz = 0;
            *n = side_intersect1;
          }
        }
      }
    }

    if (lambda_side2 > 0.000001) {
      struct point3D side_intersect2;
      trans_ray.rayPos(&trans_ray, lambda_side2, &side_intersect2);
      if (side_intersect2.pz >= -1 && side_intersect2.pz <= 1) {
        if (*lambda < 0) {
          *lambda = lambda_side2;
          *a = (side_intersect2.py >= 0) ? acos(n->px) : 2*PI - acos(n->px);
          *a = *a/(2*PI);
          *b = (side_intersect2.pz + 1) * 0.5;
          side_intersect2.pz = 0;
          *n = side_intersect2;
        } else {
          if (lambda_side2 < *lambda) {
            *lambda = lambda_side2;
            *a = (side_intersect2.py >= 0) ? acos(n->px) : 2*PI - acos(n->px);
            *a = *a/(2*PI);
            *b = (side_intersect2.pz + 1) * 0.5;
            side_intersect2.pz = 0;
            *n = side_intersect2;
          }
        }
      }
    }
  }

  if (*lambda > 0.000001) {
    ray->rayPos(ray, *lambda, p);
    normalTransform(n, n, cylinder);
  } else {
    // fprintf(stderr, "In cylIntersect, no lambda solution.\n");
    return;
  }

  // missing the use of a, b here ---------------------!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

/////////////////////////////////////////////////////////////////
// Surface coordinates & random sampling on object surfaces
/////////////////////////////////////////////////////////////////
void planeCoordinates(struct object3D *plane, double a, double b, double *x, double *y, double *z)
{
 // Return in (x,y,z) the coordinates of a point on the plane given by the 2 parameters a,b in [0,1].
 // 'a' controls displacement from the left side of the plane, 'b' controls displacement from the
 // bottom of the plane.
 
  struct point3D p0;
  p0.px = -1 + 2*a;
  p0.py = -1 + 2*b;
  p0.pz = 0;
  p0.pw = 1;
  matVecMult(plane->T, &p0);
  *x = p0.px;
  *y = p0.py;
  *z = p0.pz;
}

void sphereCoordinates(struct object3D *sphere, double a, double b, double *x, double *y, double *z)
{
 // Return in (x,y,z) the coordinates of a point on the plane given by the 2 parameters a,b.
 // 'a' in [0, 2*PI] corresponds to the spherical coordinate theta
 // 'b' in [-PI/2, PI/2] corresponds to the spherical coordinate phi

  struct point3D p0;
  p0.px = cos(a)*sin(b);
  p0.py = sin(a)*sin(b);
  p0.pz = cos(b);
  p0.pw = 1;
  matVecMult(sphere->T, &p0);
  *x = p0.px;
  *y = p0.py;
  *z = p0.pz;
}

void cylCoordinates(struct object3D *cyl, double a, double b, double *x, double *y, double *z)
{
 // Return in (x,y,z) the coordinates of a point on the plane given by the 2 parameters a,b.
 // 'a' in [0, 2*PI] corresponds to angle theta around the cylinder
 // 'b' in [0, 1] corresponds to height from the bottom
 
  struct point3D p0;
  p0.px = cos(a);
  p0.py = sin(a);
  p0.pz = b;
  p0.pw = 1;
  matVecMult(cyl->T, &p0);
  *x = p0.px;
  *y = p0.py;
  *z = p0.pz; 
}

void planeSample(struct object3D *plane, double *x, double *y, double *z)
{
 // Returns the 3D coordinates (x,y,z) of a randomly sampled point on the plane
 // Sapling should be uniform, meaning there should be an equal change of gedtting
 // any spot on the plane

  double random1 = drand48();
  double random2 = drand48();
  planeCoordinates(plane, random1, random2, x, y, z);
}

void sphereSample(struct object3D *sphere, double *x, double *y, double *z)
{
 // Returns the 3D coordinates (x,y,z) of a randomly sampled point on the sphere
 // Sampling should be uniform - note that this is tricky for a sphere, do some
 // research and document in your report what method is used to do this, along
 // with a reference to your source.

  //http://mathworld.wolfram.com/SpherePointPicking.html
 
  double random1 = drand48() * 2 * PI;
  double random2 = acos(drand48() * 2 - 1);
  sphereCoordinates(sphere, random1, random2, x, y, z);
}

void cylSample(struct object3D *cyl, double *x, double *y, double *z)
{
 // Returns the 3D coordinates (x,y,z) of a randomly sampled point on the cylinder
 // Sampling should be uniform over the cylinder.

  double random1 = drand48() * 2 * PI;
  double random2 = drand48();
  cylCoordinates(cyl, random1, random2, x, y, z);
}


/////////////////////////////////
// Texture mapping functions
/////////////////////////////////
void loadTexture(struct object3D *o, const char *filename, int type, struct textureNode **t_list)
{
 // Load a texture or normal map image from file and assign it to the
 // specified object. 
 // type:   1  ->  Texture map  (RGB, .ppm)
 //         2  ->  Normal map   (RGB, .ppm)
 //         3  ->  Alpha map    (grayscale, .pgm)
 // Stores loaded images in a linked list to avoid replication
 struct image *im;
 struct textureNode *p;
 
 if (o!=NULL)
 {
  // Check current linked list
  p=*(t_list);
  while (p!=NULL)
  {
   if (strcmp(&p->name[0],filename)==0)
   {
    // Found image already on the list
    if (type==1) o->texImg=p->im;
    else if (type==2) o->normalMap=p->im;
    else o->alphaMap=p->im;
    return;
   }
   p=p->next;
  }    

  // Load this texture image 
  if (type==1||type==2)
   im=readPPMimage(filename);
  else if (type==3)
   im=readPGMimage(filename);

  // Insert it into the texture list
  if (im!=NULL)
  {
   p=(struct textureNode *)calloc(1,sizeof(struct textureNode));
   strcpy(&p->name[0],filename);
   p->type=type;
   p->im=im;
   p->next=NULL; 
   // Insert into linked list
   if ((*(t_list))==NULL)
    *(t_list)=p;
   else
   {
    p->next=(*(t_list))->next;
    (*(t_list))->next=p;
   }
   // Assign to object
   if (type==1) o->texImg=im;
   else if (type==2) o->normalMap=im;
   else o->alphaMap=im;
  }
 
 }  // end if (o != NULL)
}


void texMap(struct image *img, double a, double b, double *R, double *G, double *B)
{
 /*
  Function to determine the colour of a textured object at
  the normalized texture coordinates (a,b).

  a and b are texture coordinates in [0 1].
  img is a pointer to the image structure holding the texture for
   a given object.

  The colour is returned in R, G, B. Uses bi-linear interpolation
  to determine texture colour.
 */

 //////////////////////////////////////////////////
 // TO DO (Assignment 4 only):
 //
 //  Complete this function to return the colour
 // of the texture image at the specified texture
 // coordinates. Your code should use bi-linear
 // interpolation to obtain the texture colour.
 //////////////////////////////////////////////////

 double tempR, tempG, tempB;
 int imgx = a * img->sx;
 int imgx1 = (imgx == img->sx-1) ? imgx : imgx + 1;
 int imgy = b * img->sy;
 int imgy1 = (imgy == img->sy-1) ? imgy : imgy + 1;
 //fprintf(stderr, "imgx %d imgy %d sx %f sy %f\n",imgx, imgy, a*img->sx, a*img->sy);
 double dx = a*img->sx - imgx;
 double dy = b*img->sy - imgy;
 double *rgbIm = (double *)img->rgbdata;

 double rxy = rgbIm[(imgy*img->sx + imgx)*3 + 0];
 double rx1y = rgbIm[(imgy*img->sx + imgx1)*3 + 0];
 double rxy1 = rgbIm[((imgy1)*img->sx + imgx)*3 + 0];
 double rx1y1 = rgbIm[((imgy1)*img->sx + imgx1)*3 + 0];
 double rx = (dx) * rx1y + (1 - dx) * rxy;
 double rx1 = (dx) * rx1y1 + (1 - dx) * rxy1;
 tempR = (dy) * rx1 + (1 - dy) * rx;
 //tempR = rgbIm[(imgy*img->sx + imgx)*3 + 0];
 *R = tempR;

 double gxy = rgbIm[(imgy*img->sx + imgx)*3 + 1];
 double gx1y = rgbIm[(imgy*img->sx + imgx1)*3 + 1];
 double gxy1 = rgbIm[((imgy1)*img->sx + imgx)*3 + 1];
 double gx1y1 = rgbIm[((imgy1)*img->sx + imgx1)*3 + 1];
 double gx = (dx) * gx1y + (1 - dx) * gxy;
 double gx1 = (dx) * gx1y1 + (1 - dx) * gxy1;
 tempG = (dy) * gx1 + (1 - dy) * gx;
 //tempG = rgbIm[(imgy*img->sx + imgx)*3 + 1];
 *G = tempG;

 double bxy = rgbIm[(imgy*img->sx + imgx)*3 + 2];
 double bx1y = rgbIm[(imgy*img->sx + imgx1)*3 + 2];
 double bxy1 = rgbIm[((imgy1)*img->sx + imgx)*3 + 2];
 double bx1y1 = rgbIm[((imgy1)*img->sx + imgx1)*3 + 2];
 double bx = (dx) * bx1y + (1 - dx) * bxy;
 double bx1 = (dx) * bx1y1 + (1 - dx) * bxy1;
 tempB = (dy) * bx1 + (1 - dy) * bx;
 //tempB = rgbIm[(imgy*img->sx + imgx)*3 + 2];
 *B = tempB;
 //fprintf(stderr, "R %f G %f B %f\n", tempR, tempG, tempB);
 //fprintf(stderr, "R %f G %f B %f\n", (*R)*255, (*G)*255, (*B)*255);
 return;
}

void alphaMap(struct image *img, double a, double b, double *alpha)
{
 // Just like texture map but returns the alpha value at a,b,
 // notice that alpha maps are single layer grayscale images, hence
 // the separate function.

 //////////////////////////////////////////////////
 // TO DO (Assignment 4 only):
 //
 //  Complete this function to return the alpha
 // value from the image at the specified texture
 // coordinates. Your code should use bi-linear
 // interpolation to obtain the texture colour.
 //////////////////////////////////////////////////

 double tempA;
 int imgx = a * img->sx;
 int imgx1 = (imgx == img->sx-1) ? imgx : imgx + 1;
 int imgy = b * img->sy;
 int imgy1 = (imgy == img->sy-1) ? imgy : imgy + 1;
 //fprintf(stderr, "imgx %d imgy %d sx %f sy %f\n",imgx, imgy, a*img->sx, a*img->sy);
 double dx = a*img->sx - imgx;
 double dy = b*img->sy - imgy;
 double *rgbIm = (double *)img->rgbdata;

 double axy = rgbIm[(imgy*img->sx + imgx)*3 + 0];
 double ax1y = rgbIm[(imgy*img->sx + imgx1)*3 + 0];
 double axy1 = rgbIm[((imgy1)*img->sx + imgx)*3 + 0];
 double ax1y1 = rgbIm[((imgy1)*img->sx + imgx1)*3 + 0];
 double ax = (dx) * ax1y + (1 - dx) * axy;
 double ax1 = (dx) * ax1y1 + (1 - dx) * axy1;
 tempA = (dy) * ax1 + (1 - dy) * ax;
 //tempA = rgbIm[(imgy*img->sx + imgx)*3];
 *alpha = tempA;
 
 //*(alpha)=1;	// Returns 1 which means fully opaque. Replace
 return;	// with your code if implementing alpha maps.
}


/////////////////////////////
// Light sources
/////////////////////////////
void insertPLS(struct pointLS *l, struct pointLS **list)
{
 if (l==NULL) return;
 // Inserts a light source into the list of light sources
 if (*(list)==NULL)
 {
  *(list)=l;
  (*(list))->next=NULL;
 }
 else
 {
  l->next=(*(list))->next;
  (*(list))->next=l;
 }

}

void addAreaLight(double sx, double sy, double nx, double ny, double nz,\
                  double tx, double ty, double tz, int N,\
                  double r, double g, double b, struct object3D **o_list, struct pointLS **l_list)
{
 /*
   This function sets up and inserts a rectangular area light source
   with size (sx, sy)
   orientation given by the normal vector (nx, ny, nz)
   centered at (tx, ty, tz)
   consisting of (N) point light sources (uniformly sampled)
   and with colour/intensity (r,g,b) 
   
   Note that the light source must be visible as a uniformly colored rectangle which
   casts no shadows. If you require a lightsource to shade another, you must
   make it into a proper solid box with a back and sides of non-light-emitting
   material
 */

  /////////////////////////////////////////////////////
  // TO DO: (Assignment 4!)
  // Implement this function to enable area light sources
  /////////////////////////////////////////////////////

  // NOTE: The best way to implement area light sources is to random sample from the
  //       light source's object surface within rtShade(). This is a bit more tricky
  //       but reduces artifacts significantly. If you do that, then there is no need
  //       to insert a series of point lightsources in this function.

  struct object3D *o;
  o=newPlane(0,0,0,0,1,1,1,1,1,1);
  Scale(o,sx,sy,1);
  struct point3D *n = newPoint(nx, nx, nz);
  normalize(n);
  RotateX(o, acos(n->py));
  RotateY(o, acos(n->px/(sqrt(1-n->py*n->py))));      
  Translate(o,tx,ty,tz);

  for (int i = 0; i < N; i++) {
    double x,y,z;
    planeSample(o, &x, &y, &z);
    //fprintf(stderr, "x %f y %f z %f\n", x, y, z);
    struct point3D *p0 = newPoint(x, y, z);
    struct pointLS *l = newPLS(p0, r/N, g/N, b/N);
    insertPLS(l, l_list);
  }
}

void addSphereAreaLight(double sx, double sy, double sz, double rx, double ry, double rz,\
                  double tx, double ty, double tz, int N,\
                  double r, double g, double b, struct object3D **o_list, struct pointLS **l_list)
{
  struct object3D *o;
  o=newSphere(0,0,0,0,1,1,1,1,1,1);
  Scale(o,sx,sy,sz);
  RotateX(o, rx);
  RotateY(o, ry);
  RotateZ(o, rz);      
  Translate(o,tx,ty,tz);

  for (int i = 0; i < N; i++) {
    double x,y,z;
    sphereSample(o, &x, &y, &z);
    //fprintf(stderr, "x %f y %f z %f\n", x, y, z);
    struct point3D *p0 = newPoint(x, y, z);
    struct pointLS *l = newPLS(p0, r/N, g/N, b/N);
    insertPLS(l, l_list);
  }
}

void addCylAreaLight(double sx, double sy, double sz, double rx, double ry, double rz,\
                  double tx, double ty, double tz, int N,\
                  double r, double g, double b, struct object3D **o_list, struct pointLS **l_list)
{
  struct object3D *o;
  o=newCyl(0,0,0,0,1,1,1,1,1,1);
  Scale(o,sx,sy,sz);
  RotateX(o, rx);
  RotateY(o, ry);
  RotateZ(o, rz);      
  Translate(o,tx,ty,tz);

  for (int i = 0; i < N; i++) {
    double x,y,z;
    cylSample(o, &x, &y, &z);
    //fprintf(stderr, "x %f y %f z %f\n", x, y, z);
    struct point3D *p0 = newPoint(x, y, z);
    struct pointLS *l = newPLS(p0, r/N, g/N, b/N);
    insertPLS(l, l_list);
  }
}

///////////////////////////////////
// Geometric transformation section
///////////////////////////////////

void invert(double *T, double *Tinv)
{
 // Computes the inverse of transformation matrix T.
 // the result is returned in Tinv.

 double *U, *s, *V, *rv1;
 int singFlag, i;

 // Invert the affine transform
 U=NULL;
 s=NULL;
 V=NULL;
 rv1=NULL;
 singFlag=0;

 SVD(T,4,4,&U,&s,&V,&rv1);
 if (U==NULL||s==NULL||V==NULL)
 {
  fprintf(stderr,"Error: Matrix not invertible for this object, returning identity\n");
  memcpy(Tinv,eye4x4,16*sizeof(double));
  return;
 }

 // Check for singular matrices...
 for (i=0;i<4;i++) if (*(s+i)<1e-9) singFlag=1;
 if (singFlag)
 {
  fprintf(stderr,"Error: Transformation matrix is singular, returning identity\n");
  memcpy(Tinv,eye4x4,16*sizeof(double));
  return;
 }

 // Compute and store inverse matrix
 InvertMatrix(U,s,V,4,Tinv);

 free(U);
 free(s);
 free(V);
}

void RotateXMat(double T[4][4], double theta)
{
 // Multiply the current object transformation matrix T in object o
 // by a matrix that rotates the object theta *RADIANS* around the
 // X axis.

 double R[4][4];
 memset(&R[0][0],0,16*sizeof(double));

 R[0][0]=1.0;
 R[1][1]=cos(theta);
 R[1][2]=-sin(theta);
 R[2][1]=sin(theta);
 R[2][2]=cos(theta);
 R[3][3]=1.0;

 matMult(R,T);
}

void RotateX(struct object3D *o, double theta)
{
 // Multiply the current object transformation matrix T in object o
 // by a matrix that rotates the object theta *RADIANS* around the
 // X axis.

 double R[4][4];
 memset(&R[0][0],0,16*sizeof(double));

 R[0][0]=1.0;
 R[1][1]=cos(theta);
 R[1][2]=-sin(theta);
 R[2][1]=sin(theta);
 R[2][2]=cos(theta);
 R[3][3]=1.0;

 matMult(R,o->T);
}

void RotateYMat(double T[4][4], double theta)
{
 // Multiply the current object transformation matrix T in object o
 // by a matrix that rotates the object theta *RADIANS* around the
 // Y axis.

 double R[4][4];
 memset(&R[0][0],0,16*sizeof(double));

 R[0][0]=cos(theta);
 R[0][2]=sin(theta);
 R[1][1]=1.0;
 R[2][0]=-sin(theta);
 R[2][2]=cos(theta);
 R[3][3]=1.0;

 matMult(R,T);
}

void RotateY(struct object3D *o, double theta)
{
 // Multiply the current object transformation matrix T in object o
 // by a matrix that rotates the object theta *RADIANS* around the
 // Y axis.

 double R[4][4];
 memset(&R[0][0],0,16*sizeof(double));

 R[0][0]=cos(theta);
 R[0][2]=sin(theta);
 R[1][1]=1.0;
 R[2][0]=-sin(theta);
 R[2][2]=cos(theta);
 R[3][3]=1.0;

 matMult(R,o->T);
}

void RotateZMat(double T[4][4], double theta)
{
 // Multiply the current object transformation matrix T in object o
 // by a matrix that rotates the object theta *RADIANS* around the
 // Z axis.

 double R[4][4];
 memset(&R[0][0],0,16*sizeof(double));

 R[0][0]=cos(theta);
 R[0][1]=-sin(theta);
 R[1][0]=sin(theta);
 R[1][1]=cos(theta);
 R[2][2]=1.0;
 R[3][3]=1.0;

 matMult(R,T);
}

void RotateZ(struct object3D *o, double theta)
{
 // Multiply the current object transformation matrix T in object o
 // by a matrix that rotates the object theta *RADIANS* around the
 // Z axis.

 double R[4][4];
 memset(&R[0][0],0,16*sizeof(double));

 R[0][0]=cos(theta);
 R[0][1]=-sin(theta);
 R[1][0]=sin(theta);
 R[1][1]=cos(theta);
 R[2][2]=1.0;
 R[3][3]=1.0;

 matMult(R,o->T);
}

void TranslateMat(double T[4][4], double tx, double ty, double tz)
{
 // Multiply the current object transformation matrix T in object o
 // by a matrix that translates the object by the specified amounts.

 double tr[4][4];
 memset(&tr[0][0],0,16*sizeof(double));

 tr[0][0]=1.0;
 tr[1][1]=1.0;
 tr[2][2]=1.0;
 tr[0][3]=tx;
 tr[1][3]=ty;
 tr[2][3]=tz;
 tr[3][3]=1.0;

 matMult(tr,T);
}

void Translate(struct object3D *o, double tx, double ty, double tz)
{
 // Multiply the current object transformation matrix T in object o
 // by a matrix that translates the object by the specified amounts.

 double tr[4][4];
 memset(&tr[0][0],0,16*sizeof(double));

 tr[0][0]=1.0;
 tr[1][1]=1.0;
 tr[2][2]=1.0;
 tr[0][3]=tx;
 tr[1][3]=ty;
 tr[2][3]=tz;
 tr[3][3]=1.0;

 matMult(tr,o->T);
}

void ScaleMat(double T[4][4], double sx, double sy, double sz)
{
 // Multiply the current object transformation matrix T in object o
 // by a matrix that scales the object as indicated.

 double S[4][4];
 memset(&S[0][0],0,16*sizeof(double));

 S[0][0]=sx;
 S[1][1]=sy;
 S[2][2]=sz;
 S[3][3]=1.0;

 matMult(S,T);
}

void Scale(struct object3D *o, double sx, double sy, double sz)
{
 // Multiply the current object transformation matrix T in object o
 // by a matrix that scales the object as indicated.

 double S[4][4];
 memset(&S[0][0],0,16*sizeof(double));

 S[0][0]=sx;
 S[1][1]=sy;
 S[2][2]=sz;
 S[3][3]=1.0;

 matMult(S,o->T);
}

void printmatrix(double mat[4][4])
{
 fprintf(stderr,"Matrix contains:\n");
 fprintf(stderr,"%f %f %f %f\n",mat[0][0],mat[0][1],mat[0][2],mat[0][3]);
 fprintf(stderr,"%f %f %f %f\n",mat[1][0],mat[1][1],mat[1][2],mat[1][3]);
 fprintf(stderr,"%f %f %f %f\n",mat[2][0],mat[2][1],mat[2][2],mat[2][3]);
 fprintf(stderr,"%f %f %f %f\n",mat[3][0],mat[3][1],mat[3][2],mat[3][3]);
}

/////////////////////////////////////////
// Camera and view setup
/////////////////////////////////////////
struct view *setupView(struct point3D *e, struct point3D *g, struct point3D *up, double f, double wl, double wt, double wsize)
{
 /*
   This function sets up the camera axes and viewing direction as discussed in the
   lecture notes.
   e - Camera center
   g - Gaze direction
   up - Up vector
   fov - Fild of view in degrees
   f - focal length
 */
 struct view *c;
 struct point3D *u, *v;

 u=v=NULL;

 // Allocate space for the camera structure
 c=(struct view *)calloc(1,sizeof(struct view));
 if (c==NULL)
 {
  fprintf(stderr,"Out of memory setting up camera model!\n");
  return(NULL);
 }

 // Set up camera center and axes
 c->e.px=e->px;		// Copy camera center location, note we must make sure
 c->e.py=e->py;		// the camera center provided to this function has pw=1
 c->e.pz=e->pz;
 c->e.pw=1;

 // Set up w vector (camera's Z axis). w=-g/||g||
 c->w.px=-g->px;
 c->w.py=-g->py;
 c->w.pz=-g->pz;
 c->w.pw=1;
 normalize(&c->w);

 // Set up the horizontal direction, which must be perpenticular to w and up
 u=cross(&c->w, up);
 normalize(u);
 c->u.px=u->px;
 c->u.py=u->py;
 c->u.pz=u->pz;
 c->u.pw=1;

 // Set up the remaining direction, v=(u x w)  - Mind the signs
 v=cross(&c->u, &c->w);
 normalize(v);
 c->v.px=v->px;
 c->v.py=v->py;
 c->v.pz=v->pz;
 c->v.pw=1;

 // Copy focal length and window size parameters
 c->f=f;
 c->wl=wl;
 c->wt=wt;
 c->wsize=wsize;

 // Set up coordinate conversion matrices
 // Camera2World matrix (M_cw in the notes)
 // Mind the indexing convention [row][col]
 c->C2W[0][0]=c->u.px;
 c->C2W[1][0]=c->u.py;
 c->C2W[2][0]=c->u.pz;
 c->C2W[3][0]=0;

 c->C2W[0][1]=c->v.px;
 c->C2W[1][1]=c->v.py;
 c->C2W[2][1]=c->v.pz;
 c->C2W[3][1]=0;

 c->C2W[0][2]=c->w.px;
 c->C2W[1][2]=c->w.py;
 c->C2W[2][2]=c->w.pz;
 c->C2W[3][2]=0;

 c->C2W[0][3]=c->e.px;
 c->C2W[1][3]=c->e.py;
 c->C2W[2][3]=c->e.pz;
 c->C2W[3][3]=1;

 // World2Camera matrix (M_wc in the notes)
 // Mind the indexing convention [row][col]
 c->W2C[0][0]=c->u.px;
 c->W2C[1][0]=c->v.px;
 c->W2C[2][0]=c->w.px;
 c->W2C[3][0]=0;

 c->W2C[0][1]=c->u.py;
 c->W2C[1][1]=c->v.py;
 c->W2C[2][1]=c->w.py;
 c->W2C[3][1]=0;

 c->W2C[0][2]=c->u.pz;
 c->W2C[1][2]=c->v.pz;
 c->W2C[2][2]=c->w.pz;
 c->W2C[3][2]=0;

 c->W2C[0][3]=-dot(&c->u,&c->e);
 c->W2C[1][3]=-dot(&c->v,&c->e);
 c->W2C[2][3]=-dot(&c->w,&c->e);
 c->W2C[3][3]=1;

 free(u);
 free(v);
 return(c);
}

/////////////////////////////////////////
// Image I/O section
/////////////////////////////////////////
struct image *readPPMimage(const char *filename)
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
 // readPPMdata converts the image colour information to floating point. This is so that
 // the texture mapping function doesn't have to do the conversion every time
 // it is asked to return the colour at a specific location.
 //

 FILE *f;
 struct image *im;
 char line[1024];
 int sizx,sizy;
 int i;
 unsigned char *tmp;
 double *fRGB;
 int tmpi;
 char *tmpc;

 im=(struct image *)calloc(1,sizeof(struct image));
 if (im!=NULL)
 {
  im->rgbdata=NULL;
  f=fopen(filename,"rb+");
  if (f==NULL)
  {
   fprintf(stderr,"Unable to open file %s for reading, please check name and path\n",filename);
   free(im);
   return(NULL);
  }
  tmpc=fgets(&line[0],1000,f);
  if (strcmp(&line[0],"P6\n")!=0)
  {
   fprintf(stderr,"Wrong file format, not a .ppm file or header end-of-line characters missing\n");
   free(im);
   fclose(f);
   return(NULL);
  }
  fprintf(stderr,"%s\n",line);
  // Skip over comments
  tmpc=fgets(&line[0],511,f);
  while (line[0]=='#')
  {
   fprintf(stderr,"%s",line);
   tmpc=fgets(&line[0],511,f);
  }
  sscanf(&line[0],"%d %d\n",&sizx,&sizy);           // Read file size
  fprintf(stderr,"nx=%d, ny=%d\n\n",sizx,sizy);
  im->sx=sizx;
  im->sy=sizy;

  tmpc=fgets(&line[0],9,f);  	                // Read the remaining header line
  fprintf(stderr,"%s\n",line);
  tmp=(unsigned char *)calloc(sizx*sizy*3,sizeof(unsigned char));
  fRGB=(double *)calloc(sizx*sizy*3,sizeof(double));
  if (tmp==NULL||fRGB==NULL)
  {
   fprintf(stderr,"Out of memory allocating space for image\n");
   free(im);
   fclose(f);
   return(NULL);
  }

  tmpi=fread(tmp,sizx*sizy*3*sizeof(unsigned char),1,f);
  fclose(f);

  // Conversion to floating point
  for (i=0; i<sizx*sizy*3; i++) *(fRGB+i)=((double)*(tmp+i))/255.0;
  free(tmp);
  im->rgbdata=(void *)fRGB;

  return(im);
 }

 fprintf(stderr,"Unable to allocate memory for image structure\n");
 return(NULL);
}

struct image *readPGMimage(const char *filename)
{
 // Just like readPPMimage() except it is used to load grayscale alpha maps. In
 // alpha maps, a value of 255 corresponds to alpha=1 (fully opaque) and 0 
 // correspondst to alpha=0 (fully transparent).
 // A .pgm header of the following form is expected:
 //
 // P5
 // # One or more comment lines preceded by '#'
 // 340 200
 // 255
 //
 // readPGMdata converts the image grayscale data to double floating point in [0,1]. 

 FILE *f;
 struct image *im;
 char line[1024];
 int sizx,sizy;
 int i;
 unsigned char *tmp;
 double *fRGB;
 int tmpi;
 char *tmpc;

 im=(struct image *)calloc(1,sizeof(struct image));
 if (im!=NULL)
 {
  im->rgbdata=NULL;
  f=fopen(filename,"rb+");
  if (f==NULL)
  {
   fprintf(stderr,"Unable to open file %s for reading, please check name and path\n",filename);
   free(im);
   return(NULL);
  }
  tmpc=fgets(&line[0],1000,f);
  if (strcmp(&line[0],"P5\n")!=0)
  {
   fprintf(stderr,"Wrong file format, not a .pgm file or header end-of-line characters missing\n");
   free(im);
   fclose(f);
   return(NULL);
  }
  // Skip over comments
  tmpc=fgets(&line[0],511,f);
  while (line[0]=='#')
   tmpc=fgets(&line[0],511,f);
  sscanf(&line[0],"%d %d\n",&sizx,&sizy);           // Read file size
  im->sx=sizx;
  im->sy=sizy;

  tmpc=fgets(&line[0],9,f);  	                // Read the remaining header line
  tmp=(unsigned char *)calloc(sizx*sizy,sizeof(unsigned char));
  fRGB=(double *)calloc(sizx*sizy,sizeof(double));
  if (tmp==NULL||fRGB==NULL)
  {
   fprintf(stderr,"Out of memory allocating space for image\n");
   free(im);
   fclose(f);
   return(NULL);
  }

  tmpi=fread(tmp,sizx*sizy*sizeof(unsigned char),1,f);
  fclose(f);

  // Conversion to double floating point
  for (i=0; i<sizx*sizy; i++) *(fRGB+i)=((double)*(tmp+i))/255.0;
  free(tmp);
  im->rgbdata=(void *)fRGB;

  return(im);
 }

 fprintf(stderr,"Unable to allocate memory for image structure\n");
 return(NULL);
}

struct image *newImage(int size_x, int size_y)
{
 // Allocates and returns a new image with all zeros. Assumes 24 bit per pixel,
 // unsigned char array.
 struct image *im;

 im=(struct image *)calloc(1,sizeof(struct image));
 if (im!=NULL)
 {
  im->rgbdata=NULL;
  im->sx=size_x;
  im->sy=size_y;
  im->rgbdata=(void *)calloc(size_x*size_y*3,sizeof(unsigned char));
  if (im->rgbdata!=NULL) return(im);
 }
 fprintf(stderr,"Unable to allocate memory for new image\n");
 return(NULL);
}

void imageOutput(struct image *im, const char *filename)
{
 // Writes out a .ppm file from the image data contained in 'im'.
 // Note that Windows typically doesn't know how to open .ppm
 // images. Use Gimp or any other seious image processing
 // software to display .ppm images.
 // Also, note that because of Windows file format management,
 // you may have to modify this file to get image output on
 // Windows machines to work properly.
 //
 // Assumes a 24 bit per pixel image stored as unsigned chars
 //

 FILE *f;

 if (im!=NULL)
  if (im->rgbdata!=NULL)
  {
   f=fopen(filename,"wb+");
   if (f==NULL)
   {
    fprintf(stderr,"Unable to open file %s for output! No image written\n",filename);
    return;
   }
   fprintf(f,"P6\n");
   fprintf(f,"# Output from RayTracer.c\n");
   fprintf(f,"%d %d\n",im->sx,im->sy);
   fprintf(f,"255\n");
   fwrite((unsigned char *)im->rgbdata,im->sx*im->sy*3*sizeof(unsigned char),1,f);
   fclose(f);
   return;
  }
 fprintf(stderr,"imageOutput(): Specified image is empty. Nothing output\n");
}

void deleteImage(struct image *im)
{
 // De-allocates memory reserved for the image stored in 'im'
 if (im!=NULL)
 {
  if (im->rgbdata!=NULL) free(im->rgbdata);
  free(im);
 }
}

void cleanup(struct object3D *o_list, struct pointLS *l_list, struct textureNode *t_list)
{
 // De-allocates memory reserved for the object list and the point light source
 // list. Note that *YOU* must de-allocate any memory reserved for images
 // rendered by the raytracer.
 struct object3D *p, *q;
 struct pointLS *r, *s;
 struct textureNode *t, *u;

 p=o_list;		// De-allocate all memory from objects in the list
 while(p!=NULL)
 {
  q=p->next;
  if (p->photonMap!=NULL)	// If object is photon mapped, free photon map memory
  {
   if (p->photonMap->rgbdata!=NULL) free(p->photonMap->rgbdata);
   free(p->photonMap);
  }
  free(p);
  p=q;
 }

 r=l_list;		// Delete light source list
 while(r!=NULL)
 {
  s=r->next;
  free(r);
  r=s;
 }

 t=t_list;		// Delete texture Images
 while(t!=NULL)
 {
  u=t->next;
  if (t->im->rgbdata!=NULL) free(t->im->rgbdata);
  free(t->im);
  free(t);
  t=u;
 }
}

