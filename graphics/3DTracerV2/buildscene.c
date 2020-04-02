 // Sets up all objects in the scene. This involves creating each object,
 // defining the transformations needed to shape and position it as
 // desired, specifying the reflectance properties (albedos and colours)
 // and setting up textures where needed.
 // Light sources must be defined, positioned, and their colour defined.
 // All objects must be inserted in the object_list. All light sources
 // must be inserted in the light_list.
 //
 // To create hierarchical objects:
 //    You must keep track of transformations carried out by parent objects
 //    as you move through the hierarchy. Declare and manipulate your own
 //    transformation matrices (use the provided functions in utils.c to
 //    compound transformations on these matrices). When declaring a new
 //    object within the hierarchy
 //    - Initialize the object
 //    - Apply any object-level transforms to shape/rotate/resize/move
 //      the object using regular object transformation functions
 //    - Apply the transformations passed on from the parent object
 //      by pre-multiplying the matrix containing the parent's transforms
 //      with the object's own transformation matrix.
 //    - Compute and store the object's inverse transform as usual.
 //
 // NOTE: After setting up the transformations for each object, don't
 //       forget to set up the inverse transform matrix!

 struct object3D *o;
 struct pointLS *l;
 struct point3D p;

 // Simple scene for Assignment 3:
 // Insert a couple of objects. A plane and two spheres
 // with some transformations.

 // Note the parameters: ra, rd, rs, rg, R, G, B, alpha, r_index, and shinyness)

 // o=newSphere(.1,.95,.35,.35,1,.25,.25,1,1,6);		// Initialize a sphere
 // Scale(o,1.5,.75,.75);					// Apply a few transforms (Translate * Rotate * Scale)
 // RotateZ(o,PI/4);					
 // Translate(o,2.0,2.5,1.5);
 // invert(&o->T[0][0],&o->Tinv[0][0]);			// Compute the inverse transform * DON'T FORGET TO DO THIS! *
 // loadTexture(o,"./textures/texture2.ppm",1,&texture_list);

 // // If needed, this is how you load a texture map
 // // loadTexture(o,"./Texture/mosaic2.ppm",1,&texture_list);	// This loads a texture called 'mosaic2.ppm'. The
	// 							// texture gets added to the texture list, and a
	// 							// pointer to it is stored within this object in the
	// 							// corresponding place. The '1' indicates this image
	// 							// will be used as a texture map. Use '2' to load
	// 							// an image as a normal map, and '3' to load an
	// 							// alpha map. Texture and normal maps are RGB .ppm
	// 							// files, alpha maps are grayscale .pgm files.
	// 							// * DO NOT * try to free image data loaded in this
	// 							// way, the cleanup function already provided will do
	// 							// this at the end.
 
 //  insertObject(o,&object_list);			// <-- If you don't insert the object into the object list,
						//     nothing happens! your object won't be rendered.

 // That's it for defining a single sphere... let's add a couple more objects
 // o=newSphere(.1,.95,.95,.75,.75,.95,.55,1,1,6);
 // Scale(o,.95,1.65,.65);
 // RotateZ(o,-PI/1.5);
 // Translate(o,-2.2,1.75,1.35);
 // invert(&o->T[0][0],&o->Tinv[0][0]);
 // loadTexture(o,"./textures/texture1.ppm",1,&texture_list);
 // insertObject(o,&object_list);

 // -------- sun ---------------
 o=newSphere(.75,.75,.95,.05,.9,.9,.9,0.8,1.1,5);
 Scale(o,1,1,1);
 Translate(o,-2,6,4);
 invert(&o->T[0][0],&o->Tinv[0][0]);
 loadTexture(o,"./textures/texture8.ppm",1,&texture_list);
 //loadTexture(o,"./textures/alpha1.pgm",3,&texture_list);
 insertObject(o,&object_list);
 // -------- sun ---------------

 // -------- background ---------------
 o=newPlane(.75,.75,.1,.05,.75,.75,.75,1,1,5);
 Scale(o,11,11,11);
 RotateZ(o,PI/4);
 RotateX(o,PI/2);
 Translate(o,0,-4,5);
 //Translate(o,0,0,5);
 invert(&o->T[0][0],&o->Tinv[0][0]);
 loadTexture(o,"./textures/texture6.ppm",1,&texture_list);
 loadTexture(o,"./textures/normal2.ppm",2,&texture_list);
 insertObject(o,&object_list);

 o=newPlane(.75,.75,.5,0,.75,.75,.75,1,1,3);
 Scale(o,20,20,15);
 RotateY(o,-PI/6);
 Translate(o,-11,15,6);
 invert(&o->T[0][0],&o->Tinv[0][0]);
 loadTexture(o,"./textures/texture7.ppm",1,&texture_list);
 loadTexture(o,"./textures/normal5.ppm",2,&texture_list);
 insertObject(o,&object_list);

 o=newPlane(.75,.75,.5,0,.75,.75,.75,1,1,3);
 Scale(o,20,20,15);
 RotateY(o,PI/6);
 Translate(o,11,15,6);
 invert(&o->T[0][0],&o->Tinv[0][0]);
 loadTexture(o,"./textures/texture7.ppm",1,&texture_list);
 loadTexture(o,"./textures/normal5.ppm",2,&texture_list);
 insertObject(o,&object_list);

 // -------- background ---------------

 // -------------- tree ---------------
 o=newCyl(.3,.85,.25,.05, 0.95, 0.95, 0.40, 1, 1, 3);
 Scale(o,.3,.3, 3);
 RotateZ(o,-PI/9);
 RotateX(o,-PI/2);
 Translate(o,3,-4,4.3);
 invert(&o->T[0][0],&o->Tinv[0][0]);
 loadTexture(o,"./textures/texture5.ppm",1,&texture_list);
 loadTexture(o,"./textures/normal3.ppm",2,&texture_list);
 insertObject(o,&object_list);

	 struct object3D oo = *o;
	 o=newCyl(.3,.85,.25,.05, 0.95, 0.95, 0.40, 1, 1, 3);
	 Scale(o,0.5,0.5,0.5);
	 //RotateY(o,PI/4);
	 //Translate(o,0.2,1.5,1.6);
	 Translate(o,1,1,1.2);
	 matMult(oo.T, o->T);
	 invert(&o->T[0][0],&o->Tinv[0][0]);
	 loadTexture(o,"./textures/texture5.ppm",1,&texture_list);
	 insertObject(o,&object_list);

  		 struct object3D ooo = *o;
		 o=newCyl(.3,.85,.25,.05, 0.95, 0.95, 0.40, 1, 1, 3);
		 Scale(o,0.8,0.8,0.8);
		 //RotateY(o,PI/4);
		 //Translate(o,0.2,1.5,1.6);
		 Translate(o,1,1,1.2);
		 matMult(ooo.T, o->T);
		 invert(&o->T[0][0],&o->Tinv[0][0]);
		 loadTexture(o,"./textures/texture5.ppm",1,&texture_list);
		 insertObject(o,&object_list);

		 o=newCyl(.3,.85,.25,.05, 0.95, 0.95, 0.40, 1, 1, 3);
		 Scale(o,0.8,0.8,0.8);
		 //RotateY(o,PI/4);
		 //Translate(o,0.2,1.5,1.6);
		 Translate(o,-1,1,1.4);
		 matMult(ooo.T, o->T);
		 invert(&o->T[0][0],&o->Tinv[0][0]);
		 loadTexture(o,"./textures/texture5.ppm",1,&texture_list);
		 insertObject(o,&object_list);

		 o=newCyl(.3,.85,.25,.05, 0.95, 0.95, 0.40, 1, 1, 3);
		 Scale(o,0.8,0.8,0.8);
		 //RotateY(o,PI/4);
		 //Translate(o,0.2,1.5,1.6);
		 Translate(o,1,-1,1.2);
		 matMult(ooo.T, o->T);
		 invert(&o->T[0][0],&o->Tinv[0][0]);
		 loadTexture(o,"./textures/texture5.ppm",1,&texture_list);
		 insertObject(o,&object_list);

	 o=newCyl(.3,.85,.25,.05, 0.95, 0.95, 0.40, 1, 1, 3);
	 Scale(o,0.5,0.5,0.5);
	 //RotateY(o,PI/4);
	 //Translate(o,0.2,1.5,1.6);
	 Translate(o,-1,1,1.2);
	 matMult(oo.T, o->T);
	 invert(&o->T[0][0],&o->Tinv[0][0]);
	 loadTexture(o,"./textures/texture5.ppm",1,&texture_list);
	 insertObject(o,&object_list);

		 ooo = *o;
		 o=newCyl(.3,.85,.25,.05, 0.95, 0.95, 0.40, 1, 1, 3);
		 Scale(o,0.8,0.8,0.8);
		 //RotateY(o,PI/4);
		 //Translate(o,0.2,1.5,1.6);
		 Translate(o,1,1,0.9);
		 matMult(ooo.T, o->T);
		 invert(&o->T[0][0],&o->Tinv[0][0]);
		 loadTexture(o,"./textures/texture5.ppm",1,&texture_list);
		 insertObject(o,&object_list);

		 o=newCyl(.3,.85,.25,.05, 0.95, 0.95, 0.40, 1, 1, 3);
		 Scale(o,0.8,0.8,0.8);
		 //RotateY(o,PI/4);
		 //Translate(o,0.2,1.5,1.6);
		 Translate(o,-1,1,1);
		 matMult(ooo.T, o->T);
		 invert(&o->T[0][0],&o->Tinv[0][0]);
		 loadTexture(o,"./textures/texture5.ppm",1,&texture_list);
		 insertObject(o,&object_list);

		 o=newCyl(.3,.85,.25,.05, 0.95, 0.95, 0.40, 1, 1, 3);
		 Scale(o,0.8,0.8,0.8);
		 //RotateY(o,PI/4);
		 //Translate(o,0.2,1.5,1.6);
		 Translate(o,-1,-1,1.1);
		 matMult(ooo.T, o->T);
		 invert(&o->T[0][0],&o->Tinv[0][0]);
		 loadTexture(o,"./textures/texture5.ppm",1,&texture_list);
		 insertObject(o,&object_list);

	 o=newCyl(.3,.85,.25,.05, 0.95, 0.95, 0.40, 1, 1, 3);
	 Scale(o,0.5,0.5,0.5);
	 //RotateY(o,PI/4);
	 //Translate(o,0.2,1.5,1.6);
	 Translate(o,-1,-1,1.2);
	 matMult(oo.T, o->T);
	 invert(&o->T[0][0],&o->Tinv[0][0]);
	 loadTexture(o,"./textures/texture5.ppm",1,&texture_list);
	 insertObject(o,&object_list);

		 ooo = *o;
		 o=newCyl(.3,.85,.25,.05, 0.95, 0.95, 0.40, 1, 1, 3);
		 Scale(o,0.8,0.8,0.8);
		 //RotateY(o,PI/4);
		 //Translate(o,0.2,1.5,1.6);
		 Translate(o,1,-1,0.8);
		 matMult(ooo.T, o->T);
		 invert(&o->T[0][0],&o->Tinv[0][0]);
		 loadTexture(o,"./textures/texture5.ppm",1,&texture_list);
		 insertObject(o,&object_list);

		 o=newCyl(.3,.85,.25,.05, 0.95, 0.95, 0.40, 1, 1, 3);
		 Scale(o,0.8,0.8,1.3);
		 //RotateY(o,PI/4);
		 //Translate(o,0.2,1.5,1.6);
		 Translate(o,-1,1,1);
		 matMult(ooo.T, o->T);
		 invert(&o->T[0][0],&o->Tinv[0][0]);
		 loadTexture(o,"./textures/texture5.ppm",1,&texture_list);
		 insertObject(o,&object_list);

		 o=newCyl(.3,.85,.25,.05, 0.95, 0.95, 0.40, 1, 1, 3);
		 Scale(o,0.8,0.8,0.9);
		 //RotateY(o,PI/4);
		 //Translate(o,0.2,1.5,1.6);
		 Translate(o,-1,-1,1);
		 matMult(ooo.T, o->T);
		 invert(&o->T[0][0],&o->Tinv[0][0]);
		 loadTexture(o,"./textures/texture5.ppm",1,&texture_list);
		 insertObject(o,&object_list);

	 o=newCyl(.3,.85,.25,.05, 0.95, 0.95, 0.40, 1, 1, 3);
	 Scale(o,0.5,0.5,0.5);
	 //RotateY(o,PI/4);
	 //Translate(o,0.2,1.5,1.6);
	 Translate(o,1,-1,1.2);
	 matMult(oo.T, o->T);
	 invert(&o->T[0][0],&o->Tinv[0][0]);
	 loadTexture(o,"./textures/texture5.ppm",1,&texture_list);
	 insertObject(o,&object_list);


 o=newCyl(.3,.85,.25,.05, 0.95, 0.95, 0.40, 1, 1, 3);
 Scale(o,.5,.5, 4);
 RotateX(o,-PI/2);
 Translate(o,1,-3,6.3);
 invert(&o->T[0][0],&o->Tinv[0][0]);
 loadTexture(o,"./textures/texture5.ppm",1,&texture_list);
 loadTexture(o,"./textures/normal3.ppm",2,&texture_list);
 insertObject(o,&object_list);

 o=newCyl(.3,.85,.25,.05, 0.95, 0.95, 0.40, 1, 1, 3);
 Scale(o,.5,.5, 4);
 RotateX(o,-PI/2);
 Translate(o,4.5,-3,3);
 invert(&o->T[0][0],&o->Tinv[0][0]);
 loadTexture(o,"./textures/texture5.ppm",1,&texture_list);
 loadTexture(o,"./textures/normal3.ppm",2,&texture_list);
 insertObject(o,&object_list);

 o=newCyl(.3,.85,.25,.05, 0.95, 0.95, 0.40, 1, 1, 3);
 Scale(o,.4,.4, 7);
 RotateX(o,-PI/2);
 Translate(o,5.5,-3,5);
 invert(&o->T[0][0],&o->Tinv[0][0]);
 loadTexture(o,"./textures/texture5.ppm",1,&texture_list);
 loadTexture(o,"./textures/normal3.ppm",2,&texture_list);
 insertObject(o,&object_list);


 // -------------- tree ---------------

 // -------------- snowman ---------------
 o=newSphere(.5,.75,.25,.05,.75,.95,1,1,1,3);
 Scale(o,1.5,1.5,1.5);
 Translate(o,-3,-3,4.5);
 invert(&o->T[0][0],&o->Tinv[0][0]);
 loadTexture(o,"./textures/texture10.ppm",1,&texture_list);
 loadTexture(o,"./textures/normal4.ppm",2,&texture_list);
 insertObject(o,&object_list);

 	 oo = *o;
 	 o=newSphere(.5,.75,.25,.05,.75,.95,1,1,1,3);
	 Scale(o,0.8,0.8,0.8);
	 Translate(o,0,1.5,0);
	 matMult(oo.T, o->T);
	 invert(&o->T[0][0],&o->Tinv[0][0]);
	 loadTexture(o,"./textures/texture10.ppm",1,&texture_list);
	 loadTexture(o,"./textures/normal4.ppm",2,&texture_list);
	 insertObject(o,&object_list);

	 	 ooo = *o;
	 	 o=newSphere(.5,.75,.25,.05,.75,.95,1,1,1,3);
		 Scale(o,0.9,0.9,0.9);
		 RotateY(o,-0.66*PI);
		 Translate(o,0,1.5,0);
		 matMult(ooo.T, o->T);
		 invert(&o->T[0][0],&o->Tinv[0][0]);
		 loadTexture(o,"./textures/texture9.ppm",1,&texture_list);
		 insertObject(o,&object_list);
 // -------------- snowman ---------------

 // -------------- ice ball -------------
 o=newSphere(.8,.75,.85,.1,.95,.95,.95,0.4,1.2,5);
 //o=newSphere(.75,.75,.95,.05,.9,.9,.9,0.8,1.1,5);
 Scale(o,0.8,0.8,0.8);
 Translate(o,-2,-3,3);
 invert(&o->T[0][0],&o->Tinv[0][0]);
 //loadTexture(o,"./textures/alpha1.pgm",3,&texture_list);
 insertObject(o,&object_list);

 o=newSphere(.8,.75,.85,.1,.95,.95,.95,0.4,1.2,5);
 Scale(o,1,1,1);
 Translate(o,-3.6,-3,2.7);
 invert(&o->T[0][0],&o->Tinv[0][0]);
 //loadTexture(o,"./textures/alpha1.pgm",3,&texture_list);
 insertObject(o,&object_list);

 o=newSphere(.8,.75,.85,.1,.95,.95,.95,0.4,1.2,5);
 Scale(o,0.6,0.6,0.6);
 Translate(o,-2.66,-3,2);
 invert(&o->T[0][0],&o->Tinv[0][0]);
 //loadTexture(o,"./textures/alpha1.pgm",3,&texture_list);
 insertObject(o,&object_list);
 // -------------- ice ball -------------

 // Insert a single point light source. We set up its position as a point structure, and specify its
 // colour in terms of RGB (in [0,1]).
 p.px=2;
 p.py=10.5;
 p.pz=-3.5;
 p.pw=1;
 l=newPLS(&p,.25,.25,.25);
 insertPLS(l,&light_list);

 // p.px=-1;
 // p.py=5;
 // p.pz=5;
 // p.pw=1;
 // l=newPLS(&p,.85,.85,.85);
 // insertPLS(l,&light_list);

 // p.px=0;
 // p.py=5.5;
 // p.pz=-3.5;
 // p.pw=1;
 // l=newPLS(&p,.75,.75,.75);
 // insertPLS(l,&light_list);

 //addAreaLight(3, 1, 0, 0.5, -0.5, -2, 25, -4, 50, .95, .95, .95, NULL, &light_list);
 addSphereAreaLight(1.5,1.5,1.5,0,0,0,-2,6,4,50,.95,.95,.95,NULL,&light_list);
 //addCylAreaLight(2,1,1,0,PI/2,0,-2,25,-4,50,.95,.95,.95,NULL,&light_list);

 // End of simple scene for Assignment 2
 // Keep in mind that you can define new types of objects such as cylinders and parametric surfaces,
 // or, you can create code to handle arbitrary triangles and then define objects as surface meshes.
 //
 // Remember: A lot of the quality of your scene will depend on how much care you have put into defining
 //           the relflectance properties of your objects, and the number and type of light sources
 //           in the scene.
 
 ///////////////////////////////////////////////////////////////////////////////////////////////////////////
 // TO DO: For Assignment 3 you *MUST* define your own cool scene.
 //	   We will be looking for the quality of your scene setup, the use of hierarchical or composite
 //	   objects that are more interesting than the simple primitives from A2, the use of textures
 //        and other maps, illumination and illumination effects such as soft shadows, reflections and
 //        transparency, and the overall visual quality of your result. Put some work into thinking
 //        about these elements when designing your scene.
 ///////////////////////////////////////////////////////////////////////////////////////////////////////////
