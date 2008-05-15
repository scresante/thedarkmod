/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision$
 * $Date$
 * $Author$
 *
 ***************************************************************************/

#ifndef __AI_TAKE_COVER_STATE_H__
#define __AI_TAKE_COVER_STATE_H__

#include "State.h"

namespace ai
{

#define STATE_TAKE_COVER "TakeCover"

class TakeCoverState :
	public State
{
public:
	// Get the name of this state
	virtual const idStr& GetName() const;

	// This is called when the state is first attached to the AI's Mind.
	virtual void Init(idAI* owner);

	// Gets called each time the mind is thinking
	virtual void Think(idAI* owner);

	static StatePtr CreateInstance();
};

} // namespace ai

#endif /* __AI_TAKE_COVER_STATE_H__ */
