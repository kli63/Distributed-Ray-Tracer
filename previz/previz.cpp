//////////////////////////////////////////////////////////////////////////////////
// This is a front end for a set of viewer clases for the Carnegie Mellon
// Motion Capture Database: 
//    
//    http://mocap.cs.cmu.edu/
//
// The original viewer code was downloaded from:
//
//   http://graphics.cs.cmu.edu/software/mocapPlayer.zip
//
// where it is credited to James McCann (Adobe), Jernej Barbic (USC),
// and Yili Zhao (USC). There are also comments in it that suggest
// and Alla Safonova (UPenn) and Kiran Bhat (ILM) also had a hand in writing it.
//
//////////////////////////////////////////////////////////////////////////////////
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <float.h>
#include <chrono>
#include "SETTINGS.h"
#include "skeleton.h"
#include "displaySkeleton.h"
#include "motion.h"
#include "DEFINITIONS.h"
#include "WORLD.h"
#include "CAMERA.h"
#include "SPHERE.h"
#include "TRIANGLES.h"
#include "CYLINDER.h"
#include "BVH.h"
#include "TEXTURE_TRIANGLE.h"
#include "TEXTURE_TRIANGLE2.h"
#include "PERLIN.h"

using namespace std;

const bool antialiasing = true;
const bool softshadow = true;
const bool depth_of_view = true;

// Stick-man classes
DisplaySkeleton displayer;
Skeleton * skeleton;
Motion * motion;

int windowWidth = 640;
int windowHeight = 480;

VEC3 eye(-6, 0.5, 1);
// VEC3 eye(0, 0.5, 1);
VEC3 lookingAt(5, 0.5, 1);
VEC3 up(0, 1, 0);

// scene geometry
vector <VEC3> sphereCenters;
vector <float> sphereRadii;
vector <VEC3> sphereColors;
vector <Light> lights;

double a_air = 1.0;
double a_glass = 1.5;
Camera cam(640, 480, VEC3(0.0, 0.5, 1.0), VEC3(5.0, 0.5, 1.0), VEC3(0.0, 1.0, 0.0), 
1.0, 65.0, VEC3(0.35, 0.35, 0.35), a_air, 0.035, 2.0);

shared_ptr<Material>red_lambert = make_shared<Lambertian>(VEC3(1.0, 0.0, 0.0));
shared_ptr<Material>shirt_lambert = make_shared<Lambertian>(VEC3(0.92156862745, 0.26274509803, 0.34509803921));
shared_ptr<Material>skin_lambert = make_shared<Lambertian>(VEC3(0.82, 0.70, 0.56));
shared_ptr<Material>mirror = make_shared<Mirror>(VEC3(0.0, 0.0, 0.0));
shared_ptr<Material>glossy_mirror = make_shared<GlossyMirror>(VEC3(0.0, 0.0, 0.0), 0.05);
shared_ptr<Material>gray_lambert = make_shared<Lambertian>(VEC3(0.25, 0.25, 0.25));
shared_ptr<Material>green_lambert = make_shared<Lambertian>(VEC3(0.0, 1.0, 0.0));
shared_ptr<Material>pants_lambert = make_shared<Lambertian>(VEC3(0.04, 0.06, 0.082));
shared_ptr<Material> yellow_light = make_shared<Emissive>(VEC3(4.0, 4.0, 4.0));  
shared_ptr<Material>metal = make_shared<Mirror>(VEC3(0.5, 0.5, 0.0));
shared_ptr<Material>metal_red = make_shared<Mirror>(VEC3(0.7, 0.0, 0.0));
shared_ptr<Material>metal_yellow = make_shared<Mirror>(VEC3(0.7, 0.4, 0.0));
shared_ptr<Material>metal_blue = make_shared<Mirror>(VEC3(0.0, 0.0, 0.7));
shared_ptr<Material>glass = make_shared<fresnelDielectric>(VEC3(0.0, 0.0, 0.0), a_air / a_glass);

Primitives world;

int imageXRes, imageYRes;
float * imageValues = NULL;

int imageXRes2, imageYRes2;
float * imageValues2 = NULL;

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void writePPM(const string & filename, int & xRes, int & yRes,
  const float * values) {
  int totalCells = xRes * yRes;
  unsigned char * pixels = new unsigned char[3 * totalCells];
  for (int i = 0; i<3 * totalCells; i++)
    pixels[i] = values[i];

  FILE * fp;
  fp = fopen(filename.c_str(), "wb");
  if (fp == NULL) {
    cout << " Could not open file \"" << filename.c_str() << "\" for writing." << endl;
    cout << " Make sure you're not trying to write from a weird location or with a " << endl;
    cout << " strange filename. Bailing ... " << endl;
    exit(0);
  }

  fprintf(fp, "P6\n%d %d\n255\n", xRes, yRes);
  fwrite(pixels, 1, totalCells * 3, fp);
  fclose(fp);
  delete[] pixels;
}

void readPPM(const string & filename, int & xRes, int & yRes, float * & values) {
  // try to open the file
  FILE * fp;
  fp = fopen(filename.c_str(), "rb");
  if (fp == NULL) {
    cout << " Could not open file \"" << filename.c_str() << "\" for reading." << endl;
    cout << " Make sure you're not trying to read from a weird location or with a " << endl;
    cout << " strange filename. Bailing ... " << endl;
    exit(0);
  }

  // get the dimensions
  unsigned char newline;
  fscanf(fp, "P6\n%d %d\n255%c", & xRes, & yRes, & newline);
  if (newline != '\n') {
    cout << " The header of " << filename.c_str() << " may be improperly formatted." << endl;
    cout << " The program will continue, but you may want to check your input. " << endl;
  }
  int totalCells = xRes * yRes;

  // grab the pixel values
  unsigned char * pixels = new unsigned char[3 * totalCells];
  fread(pixels, 1, totalCells * 3, fp);

  // copy to a nicer data type
  values = new float[3 * totalCells];
  for (int i = 0; i<3 * totalCells; i++)
    values[i] = pixels[i];

  // clean up
  delete[] pixels;
  fclose(fp);
}

//////////////////////////////////////////////////////////////////////////////////
// Load up a new motion captured frame
//////////////////////////////////////////////////////////////////////////////////
void setSkeletonsToSpecifiedFrame(int frameIndex) {
  if (frameIndex<0) {
    printf("Error in SetSkeletonsToSpecifiedFrame: frameIndex %d is illegal.\n", frameIndex);
    exit(0);
  }
  if (displayer.GetSkeletonMotion(0) != NULL) {
    int postureID;
    if (frameIndex >= displayer.GetSkeletonMotion(0) -> GetNumFrames()) {
      cout << " We hit the last frame! You might want to pick a different sequence. " << endl;
      postureID = displayer.GetSkeletonMotion(0) -> GetNumFrames() - 1;
    } else
      postureID = frameIndex;
    displayer.GetSkeleton(0) -> setPosture( * (displayer.GetSkeletonMotion(0) -> GetPosture(postureID)));
  }
}

//////////////////////////////////////////////////////////////////////////////////
// Build a list of spheres in the scene
//////////////////////////////////////////////////////////////////////////////////

typedef struct {
  VEC3 ring1History = VEC3(0, 0, 0);
  VEC3 ring2History = VEC3(0, 0, 0);
  float exponentialCoef;
}
ExpSphere;

ExpSphere exSph1;
ExpSphere exSph2;
ExpSphere exSph3;
ExpSphere exSph4;
ExpSphere exSph5;
ExpSphere exSph6;
ExpSphere exSphs[6] = {
  exSph1,
  exSph2,
  exSph3,
  exSph4,
  exSph5,
  exSph5
  };


void buildScene() {
  world.scene.clear();
  sphereCenters.clear();
  sphereRadii.clear();
  sphereColors.clear();
  displayer.ComputeBonePositions(DisplaySkeleton::BONES_AND_LOCAL_FRAMES);

  // retrieve all the bones of the skeleton
  vector<MATRIX4>& rotations = displayer.rotations();
  vector<MATRIX4>& scalings = displayer.scalings();
  vector<VEC4>& translations = displayer.translations();
  vector<float>& lengths = displayer.lengths();

  // build a sphere list, but skip the first bone, 
  // it's just the origin
  int totalBones = rotations.size();
  VEC3 floorColor = VEC3(0.5, 0.5, 0.5); // Color of the floor
  shared_ptr<Material>floorMaterial = gray_lambert; // Material of the floor

  for (int part = 1; part<totalBones; part++) {
    MATRIX4 & rotation = rotations[part];
    MATRIX4 & scaling = scalings[part];
    VEC4 & translation = translations[part];

    shared_ptr<Material>lambert;
    if (part == 26) {
      lambert = red_lambert;
    } else {
      lambert = red_lambert;
    }

    // get the endpoints of the cylinder
    VEC4 leftVertex(0.0, 0.0, 0.0, 1.0);
    VEC4 rightVertex(0.0, 0.0, lengths[part], 1.0);

    leftVertex = rotation * scaling * leftVertex + translation;
    rightVertex = rotation * scaling * rightVertex + translation;

    if (part == 19) {
      // POINT3 lightVertex = (rightVertex + (rightVertex - leftVertex) * (2.5 + 0.3)).head<3>();
      // Light sphereLight(lightVertex, VEC3(1.0, 0.0, 0.0));
      //   lights.push_back(sphereLight);
      for (int i = 1; i<6; i++) {
        POINT3 sphereVertex = (rightVertex + (rightVertex - leftVertex) * 2.5 * i).head<3>();
        
        
        


        ExpSphere exSph = exSphs[i];
        if (exSph.ring1History == VEC3(0, 0, 0)) {
          exSph.ring1History = sphereVertex;

        }
        exSph.exponentialCoef = 0.5 / pow(2, i);
        sphereVertex = exSph.ring1History * (1 - exSph.exponentialCoef) + sphereVertex * exSph.exponentialCoef;
        exSphs[i].ring1History = sphereVertex;
        const float sphereRadius = 0.05;
        Sphere sphere1(sphereVertex, sphereRadius, 10, VEC3(1, 0.7, 0), metal_yellow);
        world.addtoScene(make_shared<Sphere>(sphere1));


      }

    }
    if (part == 26) {
        // POINT3 lightVertex = (rightVertex + (rightVertex - leftVertex) * (2.5 + 0.3)).head<3>();
        // Light sphereLight(lightVertex, VEC3(0.0, 0.0, 1.0));
        // lights.push_back(sphereLight);
      for (int i = 1; i<6; i++) {
        POINT3 sphereVertex = (rightVertex + (rightVertex - leftVertex) * 2.5 * i).head<3>();

        

        ExpSphere exSph = exSphs[i];
        if (exSph.ring2History == VEC3(0, 0, 0)) {
          exSph.ring2History = sphereVertex;

        }
        exSph.exponentialCoef = 0.5 / pow(2, i);
        // cout << "Sphere vertex goes from " << sphereVertex << endl;
        sphereVertex = exSph.ring2History * (1 - exSph.exponentialCoef) + sphereVertex * exSph.exponentialCoef;
        // cout << "to " << sphereVertex << endl;
        exSphs[i].ring2History = sphereVertex;
        const float sphereRadius = 0.05;
        Sphere sphere1(sphereVertex, sphereRadius, 10, VEC3(0, 0, 1), metal_blue);
        world.addtoScene(make_shared<Sphere>(sphere1));

      }
    }
    // cout << "Left: " << leftVertex << "right: " << rightVertex << endl;

    // POINT3 center = ((leftVertex + rightVertex) / 2).head<3>();
    // VEC3 direction = (rightVertex - leftVertex).head<3>();
    // const float magnitude = direction.norm();
    // direction *= 1.0 / magnitude;

    // float radius = 0.5;
    // float length = (rightVertex - leftVertex).norm();
    // cout << totalBones;
    float radius = 0.025;
    if (part < 5)
    {
      // cout << "hello" << endl;
      Cylinder cylinder1(leftVertex.head<3>(), rightVertex.head<3>(), radius, 10, VEC3(1, 0, 0), pants_lambert);
      world.addtoScene(make_shared<Cylinder>(cylinder1));
    }
    else if (part == 5)
    {
      // cout << "hello" << endl;
      Cylinder cylinder1(leftVertex.head<3>(), rightVertex.head<3>(), radius, 10, VEC3(1, 0, 0), gray_lambert);
      world.addtoScene(make_shared<Cylinder>(cylinder1));
    }
    else if (part < 10)
    {
       // cout << "hello" << endl;
      Cylinder cylinder1(leftVertex.head<3>(), rightVertex.head<3>(), radius, 10, VEC3(1, 0, 0), pants_lambert);
      world.addtoScene(make_shared<Cylinder>(cylinder1));
    }
    else if (part == 10)
    {
      // cout << "hello" << endl;
      Cylinder cylinder1(leftVertex.head<3>(), rightVertex.head<3>(), radius, 10, VEC3(1, 0, 0), gray_lambert);
      world.addtoScene(make_shared<Cylinder>(cylinder1));
    }
    else if (part < 15)
    {
      Cylinder cylinder1(leftVertex.head<3>(), rightVertex.head<3>(), radius, 10, VEC3(1, 0, 0), shirt_lambert);
      world.addtoScene(make_shared<Cylinder>(cylinder1));
    }
    else if (part == 15)
    {
      Cylinder cylinder1(leftVertex.head<3>(), rightVertex.head<3>(), radius, 10, VEC3(1, 0, 0), skin_lambert);
      world.addtoScene(make_shared<Cylinder>(cylinder1));
    }
    else if (part == 16)
    {
      Cylinder cylinder1(leftVertex.head<3>(), rightVertex.head<3>(), radius, 10, VEC3(1, 0, 0), skin_lambert);
      world.addtoScene(make_shared<Cylinder>(cylinder1));
    }
    else if (part < 19)
    {
      Cylinder cylinder1(leftVertex.head<3>(), rightVertex.head<3>(), radius, 10, VEC3(1, 0, 0), shirt_lambert);
      world.addtoScene(make_shared<Cylinder>(cylinder1));
    }
    else if (part == 19)
    {
       Cylinder cylinder1(leftVertex.head<3>(), rightVertex.head<3>(), radius, 10, VEC3(1, 0, 0), pants_lambert);
      world.addtoScene(make_shared<Cylinder>(cylinder1));
    }
    else if (part < 24)
    {
      Cylinder cylinder1(leftVertex.head<3>(), rightVertex.head<3>(), radius, 10, VEC3(1, 0, 0), skin_lambert);
      world.addtoScene(make_shared<Cylinder>(cylinder1));
    }
    else if (part < 26)
    {
      Cylinder cylinder1(leftVertex.head<3>(), rightVertex.head<3>(), radius, 10, VEC3(1, 0, 0), shirt_lambert);
      world.addtoScene(make_shared<Cylinder>(cylinder1));
    }
    else if (part == 26)
    {
      Cylinder cylinder1(leftVertex.head<3>(), rightVertex.head<3>(), radius, 10, VEC3(1, 0, 0), pants_lambert);
      world.addtoScene(make_shared<Cylinder>(cylinder1));
    }
    else if (part < 31)
    {
      Cylinder cylinder1(leftVertex.head<3>(), rightVertex.head<3>(), radius, 10, VEC3(1, 0, 0), skin_lambert);
      world.addtoScene(make_shared<Cylinder>(cylinder1));
    }
    else
    {
      Cylinder cylinder1(leftVertex.head<3>(), rightVertex.head<3>(), radius, 10, VEC3(1, 0, 0), shirt_lambert);
      world.addtoScene(make_shared<Cylinder>(cylinder1));
    }

    // Cylinder cylinder1(leftVertex.head<3>(), rightVertex.head<3>(), radius, 10, VEC3(1, 0, 0), shirt_lambert);
    // world.addtoScene(make_shared<Cylinder>(cylinder1));

    // store the spheres
    sphereCenters.push_back(leftVertex.head<3>());
    sphereRadii.push_back(0.05);
    sphereColors.push_back(VEC3(1, 0, 0));

    Sphere s0(leftVertex.head<3>(), 0.05, 10.0, VEC3(1.0, 0.0, 0.0), red_lambert);
    // world.addtoScene(make_shared<Sphere>(s0));

    sphereCenters.push_back(rightVertex.head<3>());
    sphereRadii.push_back(0.05);
    sphereColors.push_back(VEC3(1, 0, 0));

    Sphere s1(rightVertex.head<3>(), 0.05, 10.0, VEC3(1.0, 0.0, 0.0), red_lambert);
    // world.addtoScene(make_shared<Sphere>(s1));

    // for (int y = 0; y<totalSpheres; y++)
    // {
    //   VEC3 center = ((float)y + 0.5) * rayIncrement * direction + leftVertex.head<3>();
    //   sphereCenters.push_back(center);
    //   sphereRadii.push_back(0.05);
    //   sphereColors.push_back(VEC3(1,0,0));
    //   Sphere temp(center, 0.05, 10.0, VEC3(1.0, 0.0, 0.0), red_lambert);
    //   world.addtoScene(make_shared<Sphere>(temp));
    // } 
  }
  float floorSize = 30.0f;
  VEC3 floorCenter = VEC3(0.0, 0.0, 0.0);
  VEC3 v1 = floorCenter + VEC3(-floorSize / 2, 0, -floorSize / 2);
  VEC3 v2 = floorCenter + VEC3(floorSize / 2, 0, -floorSize / 2);
  VEC3 v3 = floorCenter + VEC3(floorSize / 2, 0, floorSize / 2);
  VEC3 v4 = floorCenter + VEC3(-floorSize / 2, 0, floorSize / 2);

  
  TextureTriangle floorTriangle1Tex(v1, v2, v3, 10.0, red_lambert, imageValues, imageXRes, VEC2(0, 0), VEC2(0, imageXRes), VEC2(imageYRes, 0));
  world.addtoScene(make_shared<TextureTriangle>(floorTriangle1Tex));

  TextureTriangle floorTriangle2Tex(v1, v3, v4, 10.0, red_lambert, imageValues, imageXRes, VEC2(0, imageXRes), VEC2(imageYRes, imageYRes), VEC2(imageYRes, 0));
  world.addtoScene(make_shared<TextureTriangle>(floorTriangle2Tex));

  VEC3 vv1 = floorCenter + VEC3(-3, 0, 0);
  VEC3 vv2 = floorCenter + VEC3(-3, 0, 1);
  VEC3 vv3 = floorCenter + VEC3(-3, 1.3, 0);
  VEC3 vv4 = floorCenter + VEC3(-3, 1.3, 1);

  TextureTriangle2 shangchiPortrait1(vv2, vv3, vv4, 10.0, red_lambert, imageValues2, imageXRes2, VEC2(imageXRes2, 0), VEC2(0, 0), VEC2(0, imageYRes2));
  TextureTriangle2 shangchiPortrait2(vv1, vv2, vv3, 10.0, red_lambert, imageValues2, imageXRes2,VEC2(0, 0), VEC2(imageXRes2, imageYRes2), VEC2(imageXRes2, 0));
  world.addtoScene(make_shared<TextureTriangle2>(shangchiPortrait1));
  world.addtoScene(make_shared<TextureTriangle2>(shangchiPortrait2));

  Triangle floorTriangle1(v1, v2, v3, 10.0, floorColor, gray_lambert);
  Triangle floorTriangle2(v1, v3, v4, 10.0, floorColor, gray_lambert);
  // world.addtoScene(make_shared<Triangle>(floorTriangle1));
  // world.addtoScene(make_shared<Triangle>(floorTriangle2));

  Sphere temp1(lookingAt, 1.0, 10.0, VEC3(0.0, 0.0, 0.0), glossy_mirror);
  world.addtoScene(make_shared<Sphere>(temp1));
  Sphere temp2(VEC3(1.0, 0.5, -5.0), 1.0, 10.0, VEC3(0.0, 0.0, 0.0), glass);
  world.addtoScene(make_shared<Sphere>(temp2));
  // Sphere temp3(VEC3(-1.0, 0.5, 3.0), 1.0, 10.0, VEC3(0.0, 0.0, 0.0), mirror);
  // world.addtoScene(make_shared<Sphere>(temp3));
}

void renderFrames(int start, int end, float* pixels)
{
  for (int x = 0; x< 2400; x += 8) {

    setSkeletonsToSpecifiedFrame(x);
    buildScene();

    double radius = distance(cam.eye, cam.lookat);
    double angle = (double(x) / 2400.0) * 2.0 * M_PI;
    cam.lookat = sphereCenters[0];
    cam.eye[0] = cam.lookat[0] - radius * cos(angle);
    cam.eye[2] = cam.lookat[2] + radius * sin(angle);
    cam.update();
    if (x / 8 < start || x / 8 >= end) {
      continue;
    }
    // cout << cam.focalLength << endl;
    if (x!= 0 && x < 840)
    {
      cam.focalLength = min(cam.focalLength + ((x / 8) * 0.0443), 5.0);
      // cout << cam.focalLength << endl;
      cam.aperture = max(cam.aperture - (x / 8) * 0.0005, 0.01);
    }
    if ( x > 840 && x < 1680)
    {
      cam.focalLength = 5.0;
      cam.aperture = 0.01;
    }
    if (x > 1680)
    {
      cam.focalLength = 5.0;
      cam.aperture = 0.0;
    }
    // cout << cam.focalLength << endl;
    // cout << cam.aperture << endl;
    bvh_node world_bvh(world);

    char buffer[256];
    snprintf(buffer, 256, "./frames/frame.%04i.ppm", x / 8);
    // renderImage(windowWidth, windowHeight, buffer);
    auto start = chrono::high_resolution_clock::now();
    cam.render(world_bvh, lights, pixels, antialiasing, softshadow, depth_of_view);
    writePPM(buffer, cam.xRes, cam.yRes, pixels);
    auto stop = chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    cout << "Rendered " + to_string(x / 8) + " frames using " << duration.count() / 1000000 << " seconds" << endl;
    // cout << cam.lookat << endl << endl;
    cam.clearInputs(pixels);

    // break;
  }

}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
int main(int argc, char ** argv) {
  int start = 0;
  int end = 300;
  if (argc == 2)
  {
    start = atoi(argv[1]);
    cout << "Rendering from frame " << start << endl;
  }
  else if (argc == 3)
  {
    start = atoi(argv[1]);
    end = atoi(argv[2]);
    cout << "Rendering frames [" << start << ", " << end << ")" << endl;
  }


  readPPM("marble3.ppm", imageXRes, imageYRes, imageValues);
  readPPM("Painting.ppm", imageXRes2, imageYRes2, imageValues2);



  Light l0(VEC3(10.0, 10.0, 5.0), VEC3(1.0, 1.0, 1.0) * 0.5);
  Light l1(VEC3(-10.0, 10.0, 7.5), VEC3(1.0, 1.0, 1.0) * 0.5);
  Light l2(VEC3(5.0, 10.0, 1.0), VEC3(1.0, 1.0, 1.0) * 0.5);
  Light l3(VEC3(5.0, 10.0, 5.0), VEC3(1.0, 1.0, 1.0) * 0.9);
  Light l4(VEC3(-5.0, 10.0, 5.0), VEC3(1.0, 1.0, 1.0) * 0.9);
  Light l5(VEC3(-5.0, 10.0, 1.0), VEC3(1.0, 1.0, 1.0) * 0.9);

  
  lights.push_back(l0);
  lights.push_back(l1);
  lights.push_back(l2);
  // lights.push_back(l3);
  // lights.push_back(l4);
  // lights.push_back(l5);

  exSph1.exponentialCoef = 0.1;
  exSph2.exponentialCoef = 0.05;
  exSph3.exponentialCoef = 0.01;
  exSph4.exponentialCoef = 0.001;
  exSph5.exponentialCoef = 0.0008;
  exSph6.exponentialCoef = 0.0004;

  string skeletonFilename("12.asf");
  string motionFilename("clipped.amc");
  //string skeletonFilename("02.asf");
  //string motionFilename("02_05.amc");

  // load up skeleton stuff
  skeleton = new Skeleton(skeletonFilename.c_str(), MOCAP_SCALE);
  skeleton -> setBasePosture();
  displayer.LoadSkeleton(skeleton);

  // load up the motion
  motion = new Motion(motionFilename.c_str(), MOCAP_SCALE, skeleton);
  displayer.LoadMotion(motion);
  skeleton -> setPosture( * (displayer.GetSkeletonMotion(0) -> GetPosture(0)));

  // Note we're going 8 frames at a time, otherwise the animation
  // is really slow.

  int totalPixels = (3 * cam.xRes * cam.yRes);
  float * pixels = new float[totalPixels];
  cam.clearInputs(pixels);
  renderFrames(start, end, pixels);

  cam.clearInputs(pixels);
  delete[] pixels;

  return 0;
}