/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision$
 * $Date$
 * $Author$
 *
 * $Log$
 * Revision 1.12  2006/05/06 20:23:35  sparhawk
 * Fixed problem with determining when the animation is finished.
 *
 * Revision 1.11  2006/05/03 21:31:21  sparhawk
 * Statechange callback script added.
 *
 * Revision 1.10  2006/05/02 20:39:33  sparhawk
 * Translation added
 *
 * Revision 1.9  2006/04/29 22:10:56  sparhawk
 * Added some script functions to query the state of a door.
 *
 * Revision 1.8  2006/01/22 09:20:24  ishtvan
 * rewrote to match new soundprop interface
 *
 * Revision 1.7  2005/09/29 04:03:08  ishtvan
 * added support for double doors
 *
 * Revision 1.6  2005/09/27 08:07:15  ishtvan
 * *) Added new member vars to support multiple frob functionality
 *
 * *) Overloaded idMover::RotationDone()
 *
 * Revision 1.5  2005/04/07 08:42:38  ishtvan
 * Added placeholder method GetSoundLoss, which is called by CsndProp
 *
 * Revision 1.4  2004/11/24 21:59:06  sparhawk
 * *) Multifrob implemented
 * *) Usage of items against other items implemented.
 * *) Basic Inventory system added.
 *
 * Revision 1.3  2004/11/21 01:02:03  sparhawk
 * Doors can now be properly opened and have sound.
 *
 * Revision 1.2  2004/11/16 23:56:03  sparhawk
 * Frobcode has been generalized now and resides for all entities in the base classe.
 *
 * Revision 1.1  2004/11/14 20:19:12  sparhawk
 * Initial Release
 *
 *
 ***************************************************************************/

// Copyright (C) 2004 Gerhard W. Gruber <sparhawk@gmx.at>
//

#ifndef FROBDOOR_H
#define FROBDOOR_H

/**
 * CFrobDoor is a replacement for idDoor. The reason for this replacement is
 * because idDoor is derived from idMover_binary and can only slide from one
 * position into another. In order to make swinging doors we need to rotate
 * them but this doesn't work with normal idDoors. So CFrobDoor is a mixture
 * of idDoor and idMover.
 */
class CFrobDoor : public idMover {
public:
	CLASS_PROTOTYPE( CFrobDoor );

							CFrobDoor( void );

	void					Spawn( void );

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	virtual void			WriteToSnapshot( idBitMsgDelta &msg ) const;
	virtual void			ReadFromSnapshot( const idBitMsgDelta &msg );

	void					Open(bool Master);
	void					Close(bool Master);
	void					Lock(bool Master);
	void					Unlock(bool Master);

	void					ToggleOpen(void);
	void					ToggleLock(void);
	void					GetOpen(void);
	void					GetLock(void);
	void					GetPickable(void);

	bool					UsedBy(idEntity *);

	/**
	* Write the proper sound loss value to the soundprop portal data
	* Called when door spawns, is and when it is opened or closed
	**/
	void					UpdateSoundLoss(void);

	/**
	* Overloading idMover::DoneRotating in order to close the portal when door closes
	**/
	void					DoneRotating( void );

	/**
	 *
	 */
	void					DoneMoving(void);

	/**
	 * A helper function that implements the finalisation for rotations or movings.
	 */
	void					DoneStateChange(void);
	void					CallStateScript(void);

	/**
	* Find out if this door is touching another door, and if they share the same portal
	* If so, store a pointer to the other door m_DoubleDoor on this door.
	*
	* This is posted as an event to be called on all doors after entities spawn
	**/
	void					FindDoubleDoor( void );

protected:
	/**
	 * LinkedOpen will point to a door that is to be switched when this
	 * one is triggered. Note that the next door is flipped! This means
	 * it will change it's state according to it's current state. So if
	 * this door is open and the other one is closed this door will be
	 * closed and the other one will be opened. If both are open and they
	 * are used, both are closed and vice versa. With this pointer you can
	 * also create a chain of doors by each door pointing to the next one.
	 * Of ocurse the last door in the chain should NOT point to the first
	 * door, otherwise it will result in an endless loop.
	 */
	idStr						m_MasterOpen;
	idList<idStr>				m_OpenList;

	/**
	 * This member is the same as m_LinkedOpen, only for locks. This means
	 * that, if this door is locked, or unlocked, all other associated doors
	 * will also be locked or unlocked. Again the state depends on the respective
	 * entity state and not on the action itself. This means that if one door
	 * is locked and the other is unlocked, the lockstate will reverse. If both
	 * are locked or unlocked, both will become unlocked or locked.
	 * This way you can create i.e. a safety catch were always one door is open
	 * and the other one is closed. Or you can create a set of doors that all are
	 * locked when this one is unlocked.
	 */
	idStr						m_MasterLock;
	idList<idStr>				m_LockList;

	/**
	* m_Open is only set to false when the door is completely closed
	**/
	bool						m_Open;

	bool						m_Locked;
	bool						m_Pickable;

	/**
	* Stores whether the player intends to open or close the door
	*	Useful for determining what to do when the door is stopped midway.
	**/
	bool						m_bIntentOpen;

	bool						m_StateChange;

	/**
	* Set to true if the door was stopped mid-rotation
	**/
	bool						m_bInterrupted;

	/**
	* Read from the spawnargs, interpreted into m_OpenAngles and m_ClosedAngles
	**/
	idAngles					m_Rotate;

	/**
	 * Original position
	 */
	idVec3						m_StartPos;

	/**
	 * Vector that specifies the direction and length of the translation.
	 * This is needed for doors that don't rotate, but slide to open.
	 */
	idVec3						m_Translation;

	/**
	* Door angles when completely closed
	**/
	idAngles					m_ClosedAngles;

	/**
	* Door angles when completely open
	**/
	idAngles					m_OpenAngles;

	/**
	* Pointer to the door's partner in a double door.
	* Double door means two doors placed right next to eachother, sharing the
	*	same visportal.
	* 
	* The doubledoor does not necessarily have to be linked in a frob chain,
	*	it could be independently opened.
	**/
	CFrobDoor					*m_DoubleDoor;

	/**
	 * Scriptfunction that is called, whenever the door is finished rotating
	 * or translating. i.E. when the statechange is completed.
	 * The function gets as parameters the current state:
	 * DoorComplete(boolean open, boolean locked, boolean interrupted);
	 */
	idStr						m_CompletionScript;

	bool						m_Rotating;
	bool						m_Translating;

private:
};

#endif /* !TDMDOOR_H */
