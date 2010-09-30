// vim:ts=4:sw=4:cindent
/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4071 $
 * $Date: 2010-07-18 15:57:08 +0200 (Sun, 18 Jul 2010) $
 * $Author: tels $
 *
 ***************************************************************************/

// Copyright (C) 2010 Tels (Donated to The Dark Mod Team)

/*
   MegaModel

   Contains a rendermodel combined from different LOD stages of the same model

	TODO: Put the definition of the changes into ModelGenerator.h.

*/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: MegaModel.cpp 4071 2010-07-18 13:57:08Z tels $", init_version);

#include "MegaModel.h"

/*
===============
CMegaModel::CMegaModel
===============
*/
CMegaModel::CMegaModel( idList<const idRenderModel*>* LODs, idList<model_ofs_t>* offsets, const idVec3 *origin,
		const idMaterial* material, const int iUpdateTime ) {

	m_hModel = NULL;
	m_iUpdateTime = iUpdateTime;
	m_iNextUpdate = gameLocal.time + iUpdateTime;
	m_iMaxChanges = 25;

	m_bActive = true;

	m_Offsets.Clear();
	m_Offsets.Append( *offsets );

	m_Changes.Clear();
	// avoid frequent resizes
	m_Changes.SetGranularity(32);

	m_Origin = *origin;
	m_LODs = LODs;

	m_Material = material;

	m_hModel = gameLocal.m_ModelGenerator->DuplicateLODModels( LODs, "megamodel", offsets, origin, material );
}

/*
===============
CMegaModel::Save
===============
*/
void CMegaModel::Save( idSaveGame *savefile ) const {

	savefile->WriteBool( m_bActive );
	savefile->WriteInt( m_iNextUpdate );
	savefile->WriteInt( m_iUpdateTime );
	savefile->WriteVec3( m_Origin );
	savefile->WriteInt( m_iMaxChanges );

	// TODO: how to save a pointer? probably write out the underlying object, then restore it,
	// 		 but wouldn't leavethis us with a lot of copies after Restore()?
//	savefile->WritePtr( m_Material );

	// TODO: Wouldn't the existing entity also save the model?
	// TODO: Could we simply regenerate the model on Restore()?
	savefile->WriteModel( m_hModel );

	savefile->WriteInt( m_Offsets.Num() );
	for (int i = 0; i < m_Offsets.Num(); i++ )
	{
		savefile->WriteVec3( m_Offsets[i].offset );
		savefile->WriteVec3( m_Offsets[i].scale );
		savefile->WriteAngles( m_Offsets[i].angles );
		savefile->WriteInt( m_Offsets[i].lod );
		savefile->WriteInt( m_Offsets[i].flags );
		savefile->WriteUnsignedInt( m_Offsets[i].color );
	}

	savefile->WriteInt( m_Changes.Num() );
	for (int i = 0; i < m_Changes.Num(); i++ )
	{
		savefile->WriteInt( m_Changes[i].entity );
		savefile->WriteInt( m_Changes[i].oldLOD );
		savefile->WriteInt( m_Changes[i].newLOD );
	}
}

/*
===============
CMegaModel::Restore
===============
*/
void CMegaModel::Restore( idRestoreGame *savefile ) {

	savefile->ReadBool( m_bActive );
	savefile->ReadInt( m_iNextUpdate );
	savefile->ReadInt( m_iUpdateTime );
	savefile->ReadVec3( m_Origin );
	savefile->ReadInt( m_iMaxChanges );

	savefile->ReadModel( m_hModel );

	m_Offsets.Clear();
	int n;

	savefile->ReadInt( n );
	m_Offsets.SetNum(n);
	for (int i = 0; i < n; i ++)
	{
		savefile->ReadVec3( m_Offsets[i].offset );
		savefile->ReadVec3( m_Offsets[i].scale );
		savefile->ReadAngles( m_Offsets[i].angles );
		savefile->ReadInt( m_Offsets[i].lod );
		savefile->ReadInt( m_Offsets[i].flags );
		savefile->ReadUnsignedInt( m_Offsets[i].color );
	}

	m_Changes.Clear();
	savefile->ReadInt( n );
	m_Changes.SetNum(n);
	for (int i = 0; i < n; i ++)
	{
		savefile->ReadInt( m_Changes[i].entity );
		savefile->ReadInt( m_Changes[i].oldLOD );
		savefile->ReadInt( m_Changes[i].newLOD );
	}
}

/*
===============
CMegaModel::GetRenderModel
===============
*/
idRenderModel* CMegaModel::GetRenderModel() const
{
	return m_hModel;
}

/*
===============
CMegaModel::UpdateRenderModel

Updates the rendermodel if nec. and returns true if it was updated.
===============
*/
bool CMegaModel::UpdateRenderModel()
{
	if (m_bActive == false ||
		m_Changes.Num() == 0 ||
		(m_Changes.Num() < m_iMaxChanges) )
	{
		return false;
	}

	int n = m_Changes.Num();
	// apply all our changes to the offsets list
	for (int i = 0; i < n; i++)
	{
		m_Offsets[ m_Changes[i].entity ].lod = m_Changes[i].newLOD;
	}

	// now clear the changes
	m_Changes.Clear();

	// TODO: this might not work?

	// free our old model
	delete m_hModel;

	// and create a new one
	m_hModel = gameLocal.m_ModelGenerator->DuplicateLODModels( m_LODs, "megamodel", &m_Offsets, &m_Origin, m_Material );

	return true;
}

/*
===============
CMegaModel::Update

Checks for each offset what LOD stage to use and queues all updates, then
calls UpdateRenderModel(). deltaSq is the distance from the model origin to
the player origin.
===============
*/
bool CMegaModel::Update( const idVec3 *playerOrigin, const float lod_bias, idVec3 *gravity, const bool xycheck )
{
	if (m_bActive == false ||
		gameLocal.time < m_iNextUpdate)
	{
		// not time yet to update
		return false;
	}

	int n = m_Offsets.Num();
	float dist;

	for (int i = 0; i < n; i++)
	{
		// TODO: make xycheck work
		idVec3 delta = m_Origin + m_Offsets[i].offset - *playerOrigin;

		if ( xycheck )
		{
			delta -= (*gravity * delta) * *gravity;
		}

		// multiply with the user LOD bias setting
		// floor the value to avoid inaccurancies leading to toggling when the player stands still:
		float deltaSq = idMath::Floor( delta.LengthSqr() / lod_bias );

		// TODO: check LOD stage and add into m_Changes

		bool bWithinDist = false;

		// by default fully visible
		float fAlpha = 1.0f;

//	gameLocal.Warning("ThinkAboutLOD called with m_LOD %p deltaSq %0.2f", m_LOD, deltaSq);

		// Tels: check in which LOD level we are 
		for (int i = 0; i < LOD_LEVELS; i++)
		{

			// TODO: we need more information than just the rendermodels for each LOD stage to
			// 		 get this to work:

/*			// skip this level
			if (m_LOD->DistLODSq[i] <= 0)
			{
	//			gameLocal.Printf ("%s skipping LOD %i (distance %f)\n", GetName(), i, m_LOD->DistLODSq[i] );
				continue;
			}

			// find the next usable level
			int nextLevel = i + 1;
			while (nextLevel < LOD_LEVELS && m_LOD->DistLODSq[nextLevel] <= 0 )
			{
				nextLevel++;
			}

			if (nextLevel < LOD_LEVELS)
			{
				bWithinDist = (deltaSq > m_LOD->DistLODSq[i]) && (deltaSq <= m_LOD->DistLODSq[nextLevel]);
			}
			else
			{
				if (i < LOD_LEVELS - 1)
				{
					bWithinDist = (deltaSq < m_LOD->DistLODSq[ LOD_LEVELS - 1]);
				}
				else
				{
					// only hide if hiding isn't disabled
					// last usable level goes to infinity
					bWithinDist = m_LOD->DistLODSq[i] > 0 && (deltaSq > m_LOD->DistLODSq[i]);
				}

				// compute the alpha value of still inside the fade range
				if (bWithinDist)
				{
					if (deltaSq > (m_LOD->DistLODSq[i] + m_LOD->fLODFadeOutRange))
					{
						fAlpha = 0.0f;
					}
					else
					{
						fAlpha = 1.0f - ( (deltaSq - m_LOD->DistLODSq[i]) / m_LOD->fLODFadeOutRange );
					}
					// set the timestamp so we think the next frame again to get a smooth blend:
					m_DistCheckTimeStamp = gameLocal.time - m_LOD->DistCheckInterval - 0.1;
				}
				else
				{
					// just hide if outside
					fAlpha = 0.0f;
				}
				m_LODLevel = i;

				// early out, we found the right level and switched
				return fAlpha;
*/
		}
	}

//	gameLocal.Printf("%i: megamodel checked %i offsets\n", gameLocal.time, n);

	m_iNextUpdate = gameLocal.time + m_iUpdateTime;

	return UpdateRenderModel();
}

/*
===============
CMegaModel::StopUpdating

The entity with this megamodel will be hidden, so stop updating.
===============
*/
void CMegaModel::StopUpdating()
{
	m_bActive = false;
	m_iNextUpdate = gameLocal.time;
	m_Changes.Clear();
}

/*
===============
CMegaModel::StartUpdating

The entity with this megamodel is visible again, so start updates.
===============
*/
void CMegaModel::StartUpdating()
{
	m_bActive = true;
	m_iNextUpdate = gameLocal.time + m_iUpdateTime;
}

/*
===============
CMegaModel::ClearChanges

The entity with this megamodel will get destroyed, so we can throw away
all changes.
===============
*/
void CMegaModel::ClearChanges()
{
	m_Changes.Clear();
}

/*
===============
CMegaModel::AddChange
===============
*/
void CMegaModel::AddChange( const int entity, const int newLOD ) {

	// go through our changes and see if we already have one for this entity
	int n = m_Changes.Num();

	for (int i = 0; i < n; i++)
	{
		if (m_Changes[i].entity == entity)
		{
			// If the new change changes the model back to what it already is, remove
			// the change set:
			if ( m_Changes[i].oldLOD == newLOD )
			{
				// TODO: m_Changes.RemoveIndex(i,false);
				m_Changes.RemoveIndex(i);
			}
			else
			{
				// keep the change set with the new value
				m_Changes[i].newLOD = newLOD;
			}
			// TODO: move changed change sets to the front of the list, based
			// on the assumption they might change back again when the player
			// moves forth and back?

			// done
			return;
		}
	}

	// found no change set, add one
	model_changeinfo_t change;

	change.entity = entity;
	change.oldLOD = m_Offsets[ entity ].lod;
	change.newLOD = newLOD;

	m_Changes.Append( change );

	// done	
}

