/*
motion.h

Motion class  

1. read an AMC file and store it in a sequence of state bonevector 
2. write an AMC file
3. export to a mrdplot format for plotting the trajectories

You can add more motion data processing functions in this class. 

Revision 1 - Steve Lin, Jan. 14, 2002
Revision 2 - Alla and Kiran, Jan 18, 2002
Revision 3 - Jernej Barbic and Yili Zhao, Feb 2012
Revision 4 - Kenny Li, Nov, 2023

*/

#ifndef _MOTION_H_
#define _MOTION_H_

#include "bonevector.h"
#include "types.h"
#include "posture.h"
#include "skeleton.h"

class Motion 
{
  //function members
public:

  // parse AMC file (default scale=0.06)
  Motion(const char *amc_filename, double scale, Skeleton * pSkeleton);

  //Use to create default motion with specified number of frames
  Motion(int numFrames, Skeleton * pSkeleton);

  ~Motion();

  // scale is a parameter to adjust the translationalal scaling
  // the value of scale should be consistent with the scale parameter used in Skeleton()
  // forceAllJointsBe3DOF should be set to 0; use 1 to signal that the file contains three Euler
  // angles for all the joints, even those that are 1-dimensional or 2-dimensional (advanced usage)
  int writeAMCfile(char* filename, double scale, int forceAllJointsBe3DOF=0);

  //Set all postures to default posture
  //Root position at (0,0,0), orientation of each bone to (0,0,0)
  void SetPosturesToDefault();

  //Set the entire posture at specified frame (posture = root position and all bone rotations)
  void SetPosture(int frameIndex, Posture InPosture);

  //Set root position at specified frame
  void SetRootPos(int frameIndex, bonevector vPos);
  //Set specified bone rotation at specified frame
  void SetBoneRotation(int frameIndex, int boneIndex, bonevector vRot);

  int GetNumFrames() { return m_NumFrames; }
  Posture * GetPosture(int frameIndex);

  Skeleton * GetSkeleton() { return pSkeleton; }

protected:
  int m_NumFrames; //number of frames in the motion 
  Skeleton * pSkeleton;
  //Root position and all bone rotation angles for each frame (as read from AMC file)
  Posture * m_pPostures; 

  // The default value is 0.06
  int readAMCfile(const char* name, double scale);
};

#endif

