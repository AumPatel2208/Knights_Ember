#pragma once
#ifndef __POWERKEEPER_H__
#define __POWERKEEPER_H__

#include "GameUtil.h"

#include "GameObject.h"
#include "GameObjectType.h"
#include "IPowerListener.h"
#include "IGameWorldListener.h"

class ScoreKeeper : public IGameWorldListener
{
public:
	ScoreKeeper() { aBulletPowerTimer = 0; }
	virtual ~ScoreKeeper() {}

	void OnWorldUpdated(GameWorld* world) {}
	void OnObjectAdded(GameWorld* world, shared_ptr<GameObject> object) {}

	void OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object)
	{
		if (object->GetType() == GameObjectType("Asteroid")) {
			mScore += 10;
			FireScoreChanged();
		}
	}

	void AddListener(shared_ptr<IPowerListener> listener)
	{
		mListeners.push_back(listener);
	}

	void FireScoreChanged()
	{
		// Send message to all listeners
		for (PowerListenerList::iterator lit = mListeners.begin(); lit != mListeners.end(); ++lit) {
			(*lit)->OnScoreChanged(mScore);
		}
	}

private:
	int aBulletPowerTimer;

	typedef std::list< shared_ptr<IPowerListener> > PowerListenerList;

	PowerListenerList mListeners;
};

#endif