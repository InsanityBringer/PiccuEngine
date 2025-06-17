/*
 THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF OUTRAGE
 ENTERTAINMENT, INC. ("OUTRAGE").  OUTRAGE, IN DISTRIBUTING THE CODE TO
 END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
 ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
 IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
 SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
 FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
 CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
 AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
 COPYRIGHT 1996-2000 OUTRAGE ENTERTAINMENT, INC.  ALL RIGHTS RESERVED.
 */
 #ifndef NED_GEOMETRY_H
#define NED_GEOMETRY_H

#define X_AXIS	0
#define Y_AXIS	1
#define Z_AXIS	2

void GetFaceVertFromVert(int *face, int *vert, room *rp, int srcvert);
int InsertVertex(room *rp, vector pos);
int InsertVertex(room *rp,float x, float y, float z);
bool RotateVerts(room *rp, short *list, int num_verts, angle ang_x, angle ang_y, angle ang_z, vector offset);
bool MoveVerts(room *rp, short *list, int num_verts, vector vec);
bool MoveVerts(room *rp, short *list, int num_verts, float x, float y, float z);
int InsertFace(room *rp, short *vert_list, int num_verts);
bool ExtrudeFace(room *rp, int facenum, vector vec, float dist, BOOL delete_base_face, int inward);
bool LatheVerts(room *rp, short *list, int num_verts, int axis, int num_sides, BOOL bCaps, int inward, vector offset);
bool BendVerts(room *rp, short *list, int num_verts, int axis, float degrees, float dist, vector offset);
bool MergeVerts(room *rp, short *list, int num_verts);
int RemoveDuplicateAndUnusedPointsInRoom(room *rp);
// Record which faces contain verts in list
int GetFacesFromVerts(room *rp, short *face_list, short *vert_list, int num_verts);
void SnapVertsToFace(room *rp,short *list,int num_verts,vector *v0,vector *normal);
void SnapVertsToEdge(room *rp,short *list,int num_verts,vector *v0,vector *v1);
void SnapVertsToVert(room *rp,short *list,int num_verts,int vertnum);
bool SortVerts(room *rp,short *list,int num);
void MirrorRoom(room *rp,int axis);
bool ned_FindSharedEdge(face *fp0,face *fp1,int *vn0,int *vn1);
float ComputeFaceBoundingCircle(vector *center,room *rp,int facenum);
float SnapSingleVertToFace(room *rp,short *list,int num_verts,vector *v0,vector *normal,int snap_vert);
float SnapSingleVertToEdge(room *rp,short *list,int num_verts,vector *v0,vector *v1,int snap_vert);
float SnapSingleVertToVert(room *rp,short *list,int num_verts,int vertnum,int snap_vert);
int DeleteUnusedVertsInList(room *rp,short *list,int num_verts);

#endif 
