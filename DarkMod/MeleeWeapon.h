/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 1749 $
 * $Date: 2008-01-21 17:12:43 -0800 (Mon, 21 Jan 2008) $
 * $Author: ishtvan $
 *
 ***************************************************************************/

#ifndef __MELEEWEAPON_H__
#define __MELEEWEAPON_H__

#include "../game/entity.h"

class idPlayer;

/**
* Class for melee weapons (and shields) that are usually attached to an AI.
* Handles activating and deactivating the clipmodels for attacks and parries.
* Handles setting correct contents on the clipmodels.
* Handles collisions between weapons and actors, between weapons and parries, 
* between weapons and world, and between weapons and the item that
* the player is currently holding in the Grabber.
**/

class CMeleeWeapon : public idMoveable 
{
public:
	CLASS_PROTOTYPE( CMeleeWeapon );

							CMeleeWeapon( void );
							virtual ~CMeleeWeapon( void );

	/**
	* We modify Collide to send some notifications back to weapon owner and damage
	* the AI hit based on the damageDef
	**/
	// virtual bool			Collide( const trace_t &collision, const idVec3 &velocity );

	/**
	* Think loop calls idMoveable::Think, then checks collisions for attacks with bound weapons
	**/
	virtual void			Think( void );

	/**
	* Set or clear the actor that owns this weapon
	**/
	void SetOwner( idActor *actor );
	void ClearOwner( void );

	/**
	* Set up the clipmodel for an attack
	* ActOwner is the actor that owns this weapon, AttName the string name of the attack,
	* DamageDef a reference to the DamageDef dict,
	* bChangeCM is set to true when you want to use a specific CM for that attack.
	* If bChangeCM is set to false, the regular CM of the moveable will be used.
	* bWorldCollide is set to true if this attack can collide with the world and other objects
	**/
	void ActivateAttack( idActor *ActOwner, idStr AttName, idDict *DamageDef,
							bool bChangeCM, bool bWorldCollide );
	/**
	* Disables the attack clipmodel and clears vars
	**/
	void DeactivateAttack( void );

	/**
	* Set up the clipmodel, clipmask and stims for a parry
	* ActOwner is the actor that owns this weapon, ParryName the string name of the parry
	* bChangeCM is set to true when you want to use a CM for that parry.
	* If bChangeCM is set to false, the regular CM of the moveable will be used.
	**/
	void ActivateParry( idActor *ActOwner, idStr ParryName, bool bChangeCM );

	/**
	* Disables the parry clipmodel and clears vars
	**/
	void DeactivateParry( void );

	/**
	* Called when weapon has just been deflected by a parry or the world
	**/
	void Deflected( idEntity *other );

	/**
	* Called when we hit a damagable and have to damage it
	**/
	void DealDamage( /* going to need a lot of args here */ );

protected:
	/**
	* Clears the extra melee clipmodel if it exists,
	* resets member vars
	**/
	void ClearClipModel( void );

protected:
	/**
	* Actor that owns this melee weapon.  Most likely also the bindmaster.
	**/
	idEntityPtr<idActor>	m_Owner;

	/**
	* Modified clipmodel for the weapon
	* Can optionally be used when either parrying or attacking
	* If we use the actual weapon clipmodel, this is set to NULL
	**/
	idClipModel				*m_WeapClip;

	/**
	* Whether we are actively parrying or attacking
	**/
	bool					m_bAttacking;
	bool					m_bParrying;

	/**
	* Whether the modified clipmodel is axis-aligned or moves with joint
	**/
	bool					m_bClipAxAlign;

	/**
	* Whether this attack is set to collide with world/solids or not
	* (Typically player attacks collide with solids, AI's only collide with melee parries)
	**/
	bool					m_bWorldCollide;


	/**
	* MeleeDef used by an active attack
	**/
	idDeclEntityDef			*m_MeleeDef;

	// TODO: Attack type, parry type.. hardcoded enum or mutable string list?

};

#endif /* !__MELEEWEAPON_H__ */
