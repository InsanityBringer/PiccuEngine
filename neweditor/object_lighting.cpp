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
 

#include "object_lighting.h"
#include "object.h"
#include "lighting.h"
#include "objinfo.h"
#include "weapon.h"
#include "descent.h"
#include "game.h"
#include "polymodel.h"
#include "renderobject.h"
#include <stdlib.h>
#include "lightmap_info.h"
#include "fireball.h"
#include "player.h"
#include "mem.h"
#include "config.h"
#include "FindIntersection.h"
#include "psrand.h"


// Frees all the memory associated with this objects lightmap
void ClearObjectLightmaps (object *obj)
{
	int t,k;

	if (obj->lm_object.used==1)
	{
		obj->lm_object.used=0;
		poly_model *pm=&Poly_models[obj->rtype.pobj_info.model_num];
		for (t=0;t<obj->lm_object.num_models;t++)
		{

			if (IsNonRenderableSubmodel (pm,t))
				continue;
			for (k=0;k<obj->lm_object.num_faces[t];k++)
			{
				lightmap_object_face *lof=&obj->lm_object.lightmap_faces[t][k];
				if (lof->lmi_handle!=BAD_LMI_INDEX)
					FreeLightmapInfo (lof->lmi_handle);
				mem_free (lof->u2);
				mem_free (lof->v2);
			}
			mem_free (obj->lm_object.lightmap_faces[t]);
		}
	}
}

// Frees all the memory associated with lightmap objects
void ClearAllObjectLightmaps (int terrain)
{
	int i;

	for (i=0;i<=Highest_object_index;i++)
	{
		object *obj=&Objects[i];

		if ((OBJECT_OUTSIDE(obj) != 0) != (terrain != 0))
			continue;

		if (obj->lm_object.used==1)
		{
			ClearObjectLightmaps (obj);
			
		}
	}
}


// Sets up the memory to be used by an object for lightmaps
void SetupObjectLightmapMemory (object *obj)
{
	poly_model *pm=&Poly_models[obj->rtype.pobj_info.model_num];
	ASSERT (pm->new_style);

	ASSERT (obj->lm_object.used==0);

	obj->lm_object.used=1;

	obj->lm_object.num_models=pm->n_models;

	int i,t;

	for (i=0;i<pm->n_models;i++)
	{
		
		if (IsNonRenderableSubmodel (pm,i))
		{
			obj->lm_object.num_faces[i]=0;
			obj->lm_object.lightmap_faces[i]=NULL;
			continue;

		}

		obj->lm_object.num_faces[i]=pm->submodel[i].num_faces;
		obj->lm_object.lightmap_faces[i]=(lightmap_object_face *)mem_malloc (obj->lm_object.num_faces[i]*sizeof(lightmap_object_face));

		ASSERT(obj->lm_object.lightmap_faces[i]);

		
		for (t=0;t<pm->submodel[i].num_faces;t++)
		{
			lightmap_object_face *lof=&obj->lm_object.lightmap_faces[i][t];
			lof->num_verts=pm->submodel[i].faces[t].nverts;
			lof->lmi_handle=BAD_LMI_INDEX;

			lof->u2=(float *)mem_malloc (lof->num_verts*sizeof(float));
			lof->v2=(float *)mem_malloc (lof->num_verts*sizeof(float));
			ASSERT (lof->u2);
			ASSERT (lof->v2);

		}
	}
}

