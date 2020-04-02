/*
  CSC D18 - Assignment 1 - 2D light propagation

  This is the place where you will be doing most of your work for solving this
  assignment. Before you start working here, you shold have become familiar
  with the data structures and functions available to you from light2D.h, and
  with the way a scene is built in buildScene.c

  Go over each of the functions below, and implement the different components
  of the solution in the sections marked

  /************************
  / TO DO:
  ************************ /

  Do not add or modify code outside these sections.

  Details about what needs to be implemented are described in the comments, as
  well as in the assignment handout. You must read both carefully. 

  Starter by: F.J. Estrada, Aug. 2017
*/

/****************************************************************************
 * Uncomment the #define below to enable debug code, add whatever you need
 * to help you debug your program between #ifdef - #endif blocks
 * ************************************************************************/
#define __DEBUG_MODE

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

struct ray2D makeLightSourceRay(void)
{
 /*
   This function should return a light ray that has its origin at the light
   source, and whose direction depends on the type of light source.

   For point light sources (which emit light in all directions) the direction
    has to be chosen randomly and uniformly over a unit circle (i.e. any
    direction is equally likely)

   For a laser type light source, the direction of the ray is exactly the same
    as the direction of the lightsource.
    
   Set the colour of the ray to the same colour as the light source, and
    set the inside_outside flag to 0 (we assume light sources are 
    outside objects)

   In either case, the direction vector *must be unit length*.
*/
 
 /************************************************************************
 *  TO DO: Complete this function so that we can sample rays from the
 *         lightsource for the light propagation process.
 ************************************************************************/
 
 struct ray2D ray;

 // This creates a dummy ray (which won't go anywhere since the direction
 // vector d=[0 0]!. But it's here so you see which data values you have to
 // provide values for given the light source position, and type.  
 // ** REPLACE THE CODE BELOW with code that provides a valid ray that
 //    is consistent with the lightsource.
 
 ray.p.px=lightsource.l.p.px;			// Ray's origin
 ray.p.py=lightsource.l.p.py;
 ray.d.px=lightsource.l.d.px;			// Ray's direction
 ray.d.py=lightsource.l.d.py;
 ray.inside_out=0;		// Initially 0 since the ray starts outside an object
 ray.monochromatic=0;		// Initially 0 since the ray is white (from lightsource)
 ray.R=lightsource.R;			// Ray colour in RGB must be the same as the lightsource
 ray.G=lightsource.G;
 ray.B=lightsource.B;
 
 return(ray);
}

void propagateRay(struct ray2D *ray, int depth)
{
 /*
   This function carries out the light propagation process. It is provided with access
   to a ray data structure, and must perform the following steps (in order!):
   
   - Check if maximum recursion depth has been reached (in which case, it just returns)
   - Find the *closest* intersection between the ray and objects in the scene. This
     means you have to check against the 4 walls, and any circles added in buildScene,
     determine which intersection is closest, and obtain the intersection point, the
     normal at the intersection, and the lambda value at which the intersection happens.
   - Renders the ray onto the image from its starting point all the way up to the 
     intersection point.
   - At the intersection, use the material properties to determine how the propagation
     process proceeds:
         * For mirror materials, compute the mirror reflection direction, create a ray
           along that direction whose origin is the intersection point, and propagate
           that ray
         * For scattering materials, choose a random direction within +- 90 degrees of
           the normal, create a ray with that direction with origin at the intersection
           point, and propagate that ray
         * For refracting materials you will need to propagate two rays - one in the
           mirror reflection direction (just like for reflecting materials), and
           another in the refraction direction. Propagate both rays one after the other!
           
   NOTE: You should only care about intersections for which lambda is POSITIVE (in front
         of the ray), and greater than zero (e.g. if the ray is reflected from some
         object, you do not care about the intersection with the object itself which will
         have a lambda of *very close* to zero)
    
   In every case, make sure the ray's direction vector has unit length. You will need to
   complete other functions as part of your work here.
*/
  
 /*********************************************************************************
  * TO DO: Complete this function to implement light propagation!
  ********************************************************************************/
 
 // Define your local variables here
 struct point2D intersection;
 double lambda = 10000;
 struct point2D normal;
 int type;
 double r_idx = 1;

 if (depth>=max_depth) return;	 	// Leave this be, it makes sure you don't
					// recurse forever
 

 // Step 1 - Find *closest* intersection with the 4 walls (the written part of A1
 //          should help you figure out how to do that.

 // How many walls can the ray intersect? how many walls can the ray intersect in the
 // forward direction?
 
  double px = ray->p.px;
  double py = ray->p.py;
  double dx = ray->d.px;
  double dy = ray->d.py;

  double lambda1 = (W_LEFT - px)/dx;
  double y1 = py + lambda1*dy;
  double lambda2 = (W_RIGHT - px)/dx;
  double y2 = py + lambda2*dy;
  double lambda3 = (W_TOP - py)/dy;
  double x3 = px + lambda3*dx;
  double lambda4 = (W_BOTTOM - py)/dy;
  double x4 = px + lambda4*dx;

  // hit left
  if (y1 <= W_BOTTOM && y1 >= W_TOP && lambda1 > 0) {
    intersection.px = W_LEFT;
    intersection.py = y1;
    if (y1 == W_BOTTOM) {
      normal.px = 1;
      normal.py = -1;
      normalize(&normal);
    } else if (y1 == W_TOP) {
      normal.px = 1;
      normal.py = 1;
      normalize(&normal);
    } else {
      normal.px = 1;
      normal.py = 0;
    }
    lambda = lambda1;
    type = walls[3].material_type;
  }
  // hit right
  else if (y2 <= W_BOTTOM && y2 >= W_TOP && lambda2 > 0) {
    intersection.px = W_RIGHT;
    intersection.py = y2;
    if (y2 == W_BOTTOM) {
      normal.px = -1;
      normal.py = -1;
      normalize(&normal);
    } else if (y2 == W_TOP) {
      normal.px = -1;
      normal.py = 1;
      normalize(&normal);
    } else {
      normal.px = -1;
      normal.py = 0;
    }
    lambda = lambda2;
    type = walls[1].material_type;
  }
  // hit top
  else if (x3 <= W_RIGHT && x3 >= W_LEFT && lambda3 > 0) {
    intersection.px = x3;
    intersection.py = W_TOP;
    if (x3 == W_RIGHT) {
      normal.px = -1;
      normal.py = 1;
      normalize(&normal);
    } else if (x3 == W_LEFT) {
      normal.px = 1;
      normal.py = 1;
      normalize(&normal);
    } else {
      normal.px = 0;
      normal.py = 1;
    }
    lambda = lambda3;
    type = walls[0].material_type;
  }
  // hit bottom
  else if (x4 <= W_RIGHT && x4 >= W_LEFT && lambda4 > 0) {
    intersection.px = x4;
    intersection.py = W_BOTTOM;
    if (x4 == W_RIGHT) {
      normal.px = -1;
      normal.py = -1;
      normalize(&normal);
    } else if (x4 == W_LEFT) {
      normal.px = 1;
      normal.py = -1;
      normalize(&normal);
    } else {
      normal.px = 0;
      normal.py = -1;
    }
    lambda = lambda4;
    type = walls[2].material_type;
  }
  else {
    fprintf(stderr,"Wall is broken!");
    return;
  }

 // Step 2 - Check for intersection against objects in the object array - you must
 //          complete the intersectRay() function, call it, and obtain the closest
 //          intersection (in the forward ray direction) with objects in the scene.
 //          Note that you must provide variables for intersectRay() to return
 //          the point of intersection, normal at intersection, lambda, material type,
 //          and refraction index for the closest object hit by the ray.
 
 // Step 3 - Check whether the closest intersection with objects is closer than the
 //          closest intersection with a wall. Choose whichever is closer.
  intersectRay(ray, &intersection, &normal, &lambda, &type, &r_idx);

 // Step 4 - Render the ray onto the image. Use renderRay(). Provide renderRay() with
 //          the origin of the ray, and the intersection point (it will then draw a
 //          ray from the origin to the intersection). You also need to provide the
 //          ray's colour.
  renderRay(&(ray->p), &intersection, ray->R, ray->G, ray->B);

 // Step 5 - Decide how to handle the ray's bounce at the intersection. You will have
 //          to provide code for 3 cases:
 //          If material type = 0, you have a mirror-reflecting object. 
 //                                Create a ray in the mirror reflection direction,
 //                                with the same colour as the incoming ray, and
 //                                with origin at the intersection point.
 //                                Then call propagateRay() recursively to trace it.
 //          if material type = 1, you have a scattering surface. 
 //                                Choose a random direction within +- 90 degrees 
 //                                from the normal at the intersection. Create a
 //                                ray in this direction, with the same colour as
 //                                the incoming ray, and origin at the intersection,
 //                                then call propagateRay() recursively to trace it.

 double theta1 = acos(dot(&(ray->d), &normal));
 double theta = PI - theta1;
 double nx = normal.px;
 double ny = normal.py;
 double cross = dx*ny-dy*nx;

  // reflecting for type 0
 if (type == 0) {
  double ndx;
  double ndy;
  double thetar = PI - 2*theta;
  if (cross == 0) {
    ndx = -dx;
    ndy = -dy;
  } else if (cross > 0) {
    ndx = cos(thetar) * dx - sin(thetar) * dy;
    ndy = sin(thetar) * dx + cos(thetar) * dy;
  } else {
    ndx = cos(-thetar) * dx - sin(-thetar) * dy;
    ndy = sin(-thetar) * dx + cos(-thetar) * dy;
  }

  struct ray2D reflect_ray;
  struct point2D np;
  struct point2D nd;
  nd.px = ndx;
  nd.py = ndy;
  np.px = intersection.px;
  np.py = intersection.py;
  reflect_ray.p = np;
  reflect_ray.d = nd;
  reflect_ray.monochromatic = ray->monochromatic;
  reflect_ray.R = ray->R;
  reflect_ray.G = ray->G;
  reflect_ray.B = ray->B;
  reflect_ray.inside_out = ray->inside_out;
  propagateRay(&reflect_ray, depth + 1);
 }

 // reflecting for type 1
 if (type == 1) {
  double ndx;
  double ndy;
  double randn = rand() % 100;
  double random = (randn/ 100) - 0.5;
  double thetar = random*PI;

  ndx = cos(thetar) * nx - sin(thetar) * ny;
  ndy = sin(thetar) * nx + cos(thetar) * ny;

  struct ray2D reflect_ray;
  struct point2D np;
  struct point2D nd;
  nd.px = ndx;
  nd.py = ndy;
  np.px = intersection.px;
  np.py = intersection.py;
  reflect_ray.p = np;
  reflect_ray.d = nd;
  reflect_ray.monochromatic = ray->monochromatic;
  reflect_ray.R = ray->R;
  reflect_ray.G = ray->G;
  reflect_ray.B = ray->B;
  reflect_ray.inside_out = ray->inside_out;
  propagateRay(&reflect_ray, depth + 1);
 }

//          if material type = 2, you have a refracting (transparent) material.
 // 				   Here you need to process two rays:
 //                                * First, determine how much of the incoming light is
 //                                  reflected and how much is transmitted, using 
 //				     Schlick's approximation:
 // 					 R0 = ((n1-n2)/(n1+n2))^2   
 // 					 R(theta)=R0+((1-R0)*(1-cos(theta))^5)
 //				     If the ray is travelling from air to the inside
 //                                  of an object, n1=1, n2=object's index of refraction.
 //                                  If the ray is travelling from inside an object
 //                                  back onto air, n1=object's index of refraction, n2=1
 //				     And 'theta' is the angle between the normal and the
 // 				     ray direction.
 //				     R(theta) gives the amount Rs of reflected light, 
 //				     1.0-R(theta) gives the amount Rt of transmitted light.
 //                                * Now, make a ray in the mirror-reflection direction
 //				     (same as for material type 0), with the same colour
 //				     as the incoming ray, but with intensity modulated
 //				     by Rs. (e.g. if the incoming's colour is R,G,B,
 //                                  the reflected ray's colour will be R*Rs, G*Rs, B*Rs)
 //				     trace this ray.
 //				   * Make a ray in the refracted-ray direction. The 
 //				     angle for the transmitted ray is given by Snell's law
 //				     n1*sin(theta1) = n2*sin(theta2). The colour of the
 //				     transmitted ray is the same as the incoming ray but
 //			             modulated by Rt. Trace this ray.

 // reflecting for type 2
 if (type == 2) {
  double ndx;
  double ndy;
  double n1;
  double n2;

  if (ray->inside_out == 0) {
    n1 = 1;
    n2 = r_idx;
  } else {
    n1 = r_idx;
    n2 = 1;
  }
  double r0 = ((n1-n2)/(n1+n2))*((n1-n2)/(n1+n2));
  double rs = r0+(1-r0)*(1-cos(theta))*(1-cos(theta))*(1-cos(theta))*(1-cos(theta))*(1-cos(theta));
  if (rs > 1) rs = 1;
  if (rs < 0) rs = 0;

  double thetar = PI - 2*theta;
  if (cross == 0) {
    ndx = -dx;
    ndy = -dy;
  } else if (cross > 0) {
    ndx = cos(thetar) * dx - sin(thetar) * dy;
    ndy = sin(thetar) * dx + cos(thetar) * dy;
  } else {
    ndx = cos(-thetar) * dx - sin(-thetar) * dy;
    ndy = sin(-thetar) * dx + cos(-thetar) * dy;
  }

  struct ray2D reflect_ray;
  struct point2D np;
  struct point2D nd;
  nd.px = ndx;
  nd.py = ndy;
  np.px = intersection.px;
  np.py = intersection.py;
  reflect_ray.p = np;
  reflect_ray.d = nd;
  reflect_ray.monochromatic = ray->monochromatic;
  reflect_ray.R = (ray->R) * rs;
  reflect_ray.G = (ray->G) * rs;
  reflect_ray.B = (ray->B) * rs;
  reflect_ray.inside_out = ray->inside_out;
  propagateRay(&reflect_ray, depth + 1);
 }

 // refraction for type 2
 if (type == 2) {
  double ndx;
  double ndy;
  double n1;
  double n2;
  if (ray->inside_out == 0) {
    n1 = 1;
    n2 = r_idx;
  } else {
    n1 = r_idx;
    n2 = 1;
  }
  double r0 = ((n1-n2)/(n1+n2))*((n1-n2)/(n1+n2));
  double rs = r0+(1-r0)*(1-cos(theta))*(1-cos(theta))*(1-cos(theta))*(1-cos(theta))*(1-cos(theta));
  double rt = 1 - rs;
  if (rt > 1) rt = 1;
  if (rt < 0) rt = 0;

  // refraction angle exists
  double sintheta2 = sin(theta)*n2/n1;
  if (sintheta2 < 1 && sintheta2 > -1) {
    double theta2 = asin(sintheta2);
    double thetar = theta2 - theta;
    if (cross == 0) {
      ndx = dx;
      ndy = dy;
    } else if (cross > 0) {
      ndx = cos(thetar) * dx - sin(thetar) * dy;
      ndy = sin(thetar) * dx + cos(thetar) * dy;
    } else {
      ndx = cos(-thetar) * dx - sin(-thetar) * dy;
      ndy = sin(-thetar) * dx + cos(-thetar) * dy;
    }

    struct ray2D refract_ray;
    struct point2D np;
    struct point2D nd;
    nd.px = ndx;
    nd.py = ndy;
    np.px = intersection.px;
    np.py = intersection.py;
    refract_ray.p = np;
    refract_ray.d = nd;
    refract_ray.monochromatic = ray->monochromatic;
    refract_ray.R = (ray->R) * rt;
    refract_ray.G = (ray->G) * rt;
    refract_ray.B = (ray->B) * rt;
    refract_ray.inside_out = 1-(ray->inside_out);
    propagateRay(&refract_ray, depth + 1);
  }
 }

 //	That's it! you're done!
   
}

void intersectRay(struct ray2D *ray, struct point2D *p, struct point2D *n, double *lambda, int *type, double *r_idx)
{
 /*
  This function checks for intersection between the ray and any objects in the objects 
  array. The objects are circles, so we are in fact solving for the intersection
  between a ray and a circle.
  
  For a unit circle centered at the origin, we would have the equation
  
  x^2 + y^2 = 1
  
  Using vector notation, with C=[x y]', we get
  
  ||C||^2 = 1
  
  A point on the ray is given by p + lambda*d
  
  Substituting in the equation for the circle we have 
  
  (p + lambda*d)(p + lambda*d) - 1 = 0
  
  If we expand the product above (here the product of two vectors is a DOT product), 
  we can form a quadratic equation
  
  A*lambda^2 + B*lambda + C = 0
  
  Which as you know, has a very simple solution. 
  
  Your task is to 
  * Figure out A, B, and C, considering that your circles don't necessarily have r=1
  * Figure out how to deal with the fact that circles in the scene are likely
    *not* centered at the origin
    
  Then implement the code that will find the value(s) of lambda at the intersection(s).
  
  Note that you can have no intersections, 1 intersection, or 2 intersections
  
  This function *must* find the closest intersection (if any) and update the value
  of lambda, the intersection point p, the normal n at the intersection, 
  the corresponding object's material type (needed outside here to figure out how
  to handle the light's bouncing off this object), and the index of refraction for
  the object (needed if this is a transparent object). 
  
  You must make sure that n is a unit-length vector.
 */
 
 /**********************************************************************************
  * TO DO: Complete this function to find the closest intersection between the
  *        ray and any objects in the scene, as well as the values at the
  *        intersection that will be needed to determine how to bounce/refract the
  *	   ray.
  * *******************************************************************************/
  for (int i = 0; i < MAX_OBJECTS; i++) {
    struct circ2D cir = objects[i];
    if (cir.r > 0) {
      double cx = cir.c.px;
      double cy = cir.c.py;
      double px = ray->p.px;
      double py = ray->p.py;
      double dx = ray->d.px;
      double dy = ray->d.py;
      double nx;
      double ny;
      double r = cir.r;
      double A = dx*dx + dy*dy;
      double B = 2*(px-cx)*dx + 2*(py-cy)*dy;
      double C = (px-cx)*(px-cx) + (py-cy)*(py-cy) - r*r;
      double quadric = B*B-4*A*C;
      if (quadric > 0) {
        double lambda1 = (-B + sqrt(quadric)) / (2*A);
        double lambda2 = (-B - sqrt(quadric)) / (2*A);
        if ((lambda1 > 0.000001) && (lambda1 <= *lambda)) {
          *lambda = lambda1;
          p->px = px + *lambda*dx;
          p->py = py + *lambda*dy;
          struct point2D normal;
          if (ray->inside_out == 0) {
            nx = p->px - cx;
            ny = p->py - cy;
          } else {
            nx = cx - p->px;
            ny = cy - p->py;
          }
          normal.px = nx;
          normal.py = ny;
          normalize(&normal);
          *n = normal;
          *type = cir.material_type;
          *r_idx = cir.r_idx;
        }
        if ((lambda2 > 0.000001) && (lambda2 <= *lambda)) {
          *lambda = lambda2;
          p->px = px + *lambda*dx;
          p->py = py + *lambda*dy;
          struct point2D normal;
          if (ray->inside_out == 0) {
            nx = p->px - cx;
            ny = p->py - cy;
          } else {
            nx = cx - p->px;
            ny = cy - p->py;
          }
          normal.px = nx;
          normal.py = ny;
          normalize(&normal);
          *n = normal;
          *type = cir.material_type;
          *r_idx = cir.r_idx;
        }
      }
    }
  }
}
