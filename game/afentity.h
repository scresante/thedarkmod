/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision$
 * $Date$
 * $Author$
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __GAME_AFENTITY_H__
#define __GAME_AFENTITY_H__


/*
===============================================================================

idMultiModelAF

Entity using multiple separate visual models animated with a single
articulated figure. Only used for debugging!

===============================================================================
*/
const int GIB_DELAY = 200;  // only gib this often to keep performace hits when blowing up several mobs

class idMultiModelAF : public idEntity {
public:
	CLASS_PROTOTYPE( idMultiModelAF );

	void					Spawn( void );
							~idMultiModelAF( void );

	virtual void			Think( void );
	virtual void			Present( void );

protected:
	idPhysics_AF			physicsObj;

	void					SetModelForId( int id, const idStr &modelName );

private:
	idList<idRenderModel *>	modelHandles;
	idList<int>				modelDefHandles;
};


/*
===============================================================================

idChain

Chain hanging down from the ceiling. Only used for debugging!

===============================================================================
*/

class idChain : public idMultiModelAF {
public:
	CLASS_PROTOTYPE( idChain );

	void					Spawn( void );

protected:
	void					BuildChain( const idStr &name, const idVec3 &origin, float linkLength, float linkWidth, float density, int numLinks, bool bindToWorld = true );
};


/*
===============================================================================

idAFAttachment

===============================================================================
*/

class idAFAttachment : public idAnimatedEntity {
public:
	CLASS_PROTOTYPE( idAFAttachment );

							idAFAttachment( void );
	virtual					~idAFAttachment( void );

	void					Spawn( void );

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	void					SetBody( idEntity *bodyEnt, const char *headModel, jointHandle_t attachJoint );
	void					ClearBody( void );
	idEntity *				GetBody( void ) const;

	bool					IsMantleable( void );

	virtual void			Think( void );

	virtual void			Hide( void );
	virtual void			Show( void );

	void					PlayIdleAnim( int blendTime );

	virtual void			GetImpactInfo( idEntity *ent, int id, const idVec3 &point, impactInfo_t *info );
	virtual void			ApplyImpulse( idEntity *ent, int id, const idVec3 &point, const idVec3 &impulse );
	virtual void			AddForce( idEntity *ent, int id, const idVec3 &point, const idVec3 &force );

	virtual	void			Damage( idEntity *inflictor, idEntity *attacker, const idVec3 &dir,
									const char *damageDefName, const float damageScale,
									const int location, trace_t *tr = NULL );
	virtual void			AddDamageEffect( const trace_t &collision, const idVec3 &velocity, const char *damageDefName );

	void					SetCombatModel( void );
	idClipModel *			GetCombatModel( void ) const;
	virtual void			LinkCombat( void );
	virtual void			UnlinkCombat( void );

protected:
	idEntity *				body;
	idClipModel *			combatModel;	// render model for hit detection of head
	int						idleAnim;
	jointHandle_t			attachJoint;
};


/*
===============================================================================

idAFEntity_Base

===============================================================================
*/

/**
* TDM: Used for dynamically adding ents with clipmodels to the AF
**/
typedef struct SAddedEnt_s
{
	idEntityPtr<idEntity> ent; // associated entity

	// Must store string name because body ID's get reassigned when any are deleted
	idStr bodyName;
	// don't need to store constraint since they are deleted along with body
} SAddedEnt;


class idAFEntity_Base : public idAnimatedEntity 
{
public:
	CLASS_PROTOTYPE( idAFEntity_Base );

							idAFEntity_Base( void );
	virtual					~idAFEntity_Base( void );

	void					Spawn( void );

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	virtual void			Think( void );
	virtual void			GetImpactInfo( idEntity *ent, int id, const idVec3 &point, impactInfo_t *info );
	virtual void			ApplyImpulse( idEntity *ent, int id, const idVec3 &point, const idVec3 &impulse );
	virtual void			AddForce( idEntity *ent, int id, const idVec3 &point, const idVec3 &force );
	virtual bool			Collide( const trace_t &collision, const idVec3 &velocity );
	virtual bool			GetPhysicsToVisualTransform( idVec3 &origin, idMat3 &axis );
	virtual bool			UpdateAnimationControllers( void );
	virtual void			FreeModelDef( void );

	virtual bool			LoadAF( void );
	bool					IsActiveAF( void ) const { return af.IsActive(); }
	const char *			GetAFName( void ) const { return af.GetName(); }
	idPhysics_AF *			GetAFPhysics( void ) { return af.GetPhysics(); }

	virtual	void			Damage( idEntity *inflictor, idEntity *attacker, const idVec3 &dir,
									const char *damageDefName, const float damageScale,
									const int location, trace_t *tr = NULL );
	void					SetCombatModel( void );
	idClipModel *			GetCombatModel( void ) const;
							// contents of combatModel can be set to 0 or re-enabled (mp)
	void					SetCombatContents( bool enable );
	virtual void			LinkCombat( void );
	virtual void			UnlinkCombat( void );

	int						BodyForClipModelId( int id ) const;

	void					SaveState( idDict &args ) const;
	void					LoadState( const idDict &args );

	void					AddBindConstraints( void );
	void					RemoveBindConstraints( void );

	/**
	* Called when the given ent is about to be unbound/detached
	* Updates m_AddedEnts if this ent's clipmodel was added to the AF
	**/
	virtual void			UnbindNotify( idEntity *ent );

	/**
	* TDM: Adds the clipmodel of the given entity to the AF structure
	* Called during the binding process
	* AddEntByBody is called by BindToBody, AddEntByJoint called by BindToJoint
	**/
	void					AddEntByBody( idEntity *ent, int bodyID );
	void					AddEntByJoint( idEntity *ent, jointHandle_t jointNum );

	virtual void			ShowEditingDialog( void );

	static void				DropAFs( idEntity *ent, const char *type, idList<idEntity *> *list );

	/**
	* Return whether this entity should collide with its team when bound to a team
	**/
	bool					CollidesWithTeam( void );

public:
	/**
	* This AF should not be able to be picked up off the ground completely when dragged
	**/
	bool					m_bGroundWhenDragged;

	/**
	* List of integer Id's of "critical" bodies to check for touching the ground
	* when seeing if the AF is being lifted off the ground
	**/
	idList<int>				m_GroundBodyList;

	/**
	* Number of "critical" bodies that must be kept on the ground at all times
	* If the number is below this, dragging the AF up will not be allowed
	**/
	int						m_GroundBodyMinNum;

	/**
	* If set to true, this will use the "af" damping when being grabbed instead of the default damping
	**/
	bool					m_bDragAFDamping;

protected:
	idAF					af;				// articulated figure
	idClipModel *			combatModel;	// render model for hit detection
	int						combatModelContents;
	idVec3					spawnOrigin;	// spawn origin
	idMat3					spawnAxis;		// rotation axis used when spawned
	int						nextSoundTime;	// next time this can make a sound

	/**
	* List of ents that have been dynamically added to the AF via binding
	**/
	idList<SAddedEnt>		m_AddedEnts;

	/**
	* Set to true if this entity should collide with team members when bound to them
	**/
	bool					m_bCollideWithTeam;

	/**
	* Set to true if this animated AF should activate the AF body collision models
	*	and move them around to collide with the world when animating.
	* NOTE: This should NOT be set on AI, since they do this anyway
	**/
	bool					m_bAFPosedByAnim;

	/**
	* Set to true if the AF should push aside moveables while animating
	* NOTE: This does not apply to AI, they have their own var for this
	**/
	bool					m_bAFPushMoveables;

protected:
	/**
	* Set up grounding vars, which apply when the AF might not be able to be 
	* lifted completely off the ground by the player
	**/
	void					SetUpGroundingVars( void );

	void					Event_SetConstraintPosition( const char *name, const idVec3 &pos );

	/**
	* GetNumBodies returns the number of bodies in the AF.
	* If the AF physics pointer is NULL, it returns 0.
	**/
	void					Event_GetNumBodies( void );

	/**
	* Set the linear and angular velocities of a particular body given by ID argument
	* If the ID is invalid, no velocity is set.
	**/
	void					Event_SetLinearVelocityB( idVec3 &NewVelocity, int id );
	void					Event_SetAngularVelocityB( idVec3 &NewVelocity, int id );

	/**
	* Get the linear and angular velocities of a particular body given by int ID.  
	* If the body ID is invalid, returns (0,0,0)
	**/
	void					Event_GetLinearVelocityB( int id );
	void					Event_GetAngularVelocityB( int id );

};

/*
===============================================================================

idAFEntity_Gibbable

===============================================================================
*/

extern const idEventDef		EV_Gib;
extern const idEventDef		EV_Gibbed;

class idAFEntity_Gibbable : public idAFEntity_Base {
public:
	CLASS_PROTOTYPE( idAFEntity_Gibbable );

							idAFEntity_Gibbable( void );
							~idAFEntity_Gibbable( void );

	void					Spawn( void );
	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );
	virtual void			Present( void );
	virtual	void			Damage( idEntity *inflictor, idEntity *attacker, const idVec3 &dir, const char *damageDefName,
									const float damageScale, const int location, trace_t *tr = NULL );
	virtual void			SpawnGibs( const idVec3 &dir, const char *damageDefName );

protected:
	idRenderModel *			skeletonModel;
	int						skeletonModelDefHandle;
	bool					gibbed;

	virtual void			Gib( const idVec3 &dir, const char *damageDefName );
	void					InitSkeletonModel( void );

	void					Event_Gib( const char *damageDefName );
};

/*
===============================================================================

	idAFEntity_Generic

===============================================================================
*/

class idAFEntity_Generic : public idAFEntity_Gibbable {
public:
	CLASS_PROTOTYPE( idAFEntity_Generic );

							idAFEntity_Generic( void );
							~idAFEntity_Generic( void );

	void					Spawn( void );

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	virtual void			Think( void );
	void					KeepRunningPhysics( void ) { keepRunningPhysics = true; }

private:
	void					Event_Activate( idEntity *activator );

	bool					keepRunningPhysics;
};


/*
===============================================================================

idAFEntity_WithAttachedHead

===============================================================================
*/

class idAFEntity_WithAttachedHead : public idAFEntity_Gibbable {
public:
	CLASS_PROTOTYPE( idAFEntity_WithAttachedHead );

							idAFEntity_WithAttachedHead();
							~idAFEntity_WithAttachedHead();

	void					Spawn( void );

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	void					SetupHead( void );

	virtual void			Think( void );

	virtual void			Hide( void );
	virtual void			Show( void );
	virtual void			ProjectOverlay( const idVec3 &origin, const idVec3 &dir, float size, const char *material );

	virtual void			LinkCombat( void );
	virtual void			UnlinkCombat( void );

protected:
	virtual void			Gib( const idVec3 &dir, const char *damageDefName );

private:
	idEntityPtr<idAFAttachment>	head;

	void					Event_Gib( const char *damageDefName );
	void					Event_Activate( idEntity *activator );
};


/*
===============================================================================

idAFEntity_Vehicle

===============================================================================
*/

class idAFEntity_Vehicle : public idAFEntity_Base {
public:
	CLASS_PROTOTYPE( idAFEntity_Vehicle );

							idAFEntity_Vehicle( void );

	void					Spawn( void );
	void					Use( idPlayer *player );

protected:
	idPlayer *				player;
	jointHandle_t			eyesJoint;
	jointHandle_t			steeringWheelJoint;
	float					wheelRadius;
	float					steerAngle;
	float					steerSpeed;
	const idDeclParticle *	dustSmoke;

	float					GetSteerAngle( void );
};


/*
===============================================================================

idAFEntity_VehicleSimple

===============================================================================
*/

class idAFEntity_VehicleSimple : public idAFEntity_Vehicle {
public:
	CLASS_PROTOTYPE( idAFEntity_VehicleSimple );

							idAFEntity_VehicleSimple( void );
							~idAFEntity_VehicleSimple( void );

	void					Spawn( void );
	virtual void			Think( void );

protected:
	idClipModel *			wheelModel;
	idAFConstraint_Suspension *	suspension[4];
	jointHandle_t			wheelJoints[4];
	float					wheelAngles[4];
};


/*
===============================================================================

idAFEntity_VehicleFourWheels

===============================================================================
*/

class idAFEntity_VehicleFourWheels : public idAFEntity_Vehicle {
public:
	CLASS_PROTOTYPE( idAFEntity_VehicleFourWheels );

							idAFEntity_VehicleFourWheels( void );

	void					Spawn( void );
	virtual void			Think( void );

protected:
	idAFBody *				wheels[4];
	idAFConstraint_Hinge *	steering[2];
	jointHandle_t			wheelJoints[4];
	float					wheelAngles[4];
};


/*
===============================================================================

idAFEntity_VehicleSixWheels

===============================================================================
*/

class idAFEntity_VehicleSixWheels : public idAFEntity_Vehicle {
public:
	CLASS_PROTOTYPE( idAFEntity_VehicleSixWheels );

							idAFEntity_VehicleSixWheels( void );

	void					Spawn( void );
	virtual void			Think( void );

private:
	idAFBody *				wheels[6];
	idAFConstraint_Hinge *	steering[4];
	jointHandle_t			wheelJoints[6];
	float					wheelAngles[6];
};


/*
===============================================================================

idAFEntity_SteamPipe

===============================================================================
*/

class idAFEntity_SteamPipe : public idAFEntity_Base {
public:
	CLASS_PROTOTYPE( idAFEntity_SteamPipe );

							idAFEntity_SteamPipe( void );
							~idAFEntity_SteamPipe( void );

	void					Spawn( void );
	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	virtual void			Think( void );

private:
	int						steamBody;
	float					steamForce;
	float					steamUpForce;
	idForce_Constant		force;
	renderEntity_t			steamRenderEntity;
	qhandle_t				steamModelDefHandle;

	void					InitSteamRenderEntity( void );
};


/*
===============================================================================

idAFEntity_ClawFourFingers

===============================================================================
*/

class idAFEntity_ClawFourFingers : public idAFEntity_Base {
public:
	CLASS_PROTOTYPE( idAFEntity_ClawFourFingers );

							idAFEntity_ClawFourFingers( void );

	void					Spawn( void );
	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

private:
	idAFConstraint_Hinge *	fingers[4];

	void					Event_SetFingerAngle( float angle );
	void					Event_StopFingers( void );
};

#endif /* !__GAME_AFENTITY_H__ */
