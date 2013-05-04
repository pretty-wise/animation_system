#pragma once

#include "engine/core/StringId.h"
#include <vector>

//---------------------------------------------------------------------------------------
namespace Engine{
//---------------------------------------------------------------------------------------

/*

1. Animation Pipeline

- local pose calculation
- (optional) local poses postprocessing (?)
- global pose calculation
- (optional) procedural animation
- (optional) inverse kinematics ( enableIK(StringId jointName, u32 numJointsAffected, const Vec3& targetLocationWs) disableIK(StringId jointName) )
- (optional) rag dolls
- global pose recalculation (if any of optional steps took place)
- set physics position here
- matrix palette generation


2. Action State Machine

- state layers

3. Animation Controllers
*/


class AnimExprNode;

class AnimationSet
{
public:
	enum AnimationCurve
	{
		ac_VALUE,
		ac_CROSSFADE,
		ac_EASE_IN,
		ac_EASE_OUT
	};
public:

	//! AnimationSet.
	AnimationSet();

	// Copy constructor. used for cloning.
	AnimationSet(const AnimationSet& rhs);

	~AnimationSet();

	void							update(float dtMs);
	
	inline bool						isActive() { return m_pRoot != NULL; }

	bool							transition(StringId transitionName);
	bool							play(StringId stateName, float fEaseInTimeMs = 0.f);

	void							stop(float fEaseOutTimeMs = 0.f);

	inline void						setFactor(StringId name, float factor);

	void							setAnimationStates(AnimationStates* states);

	inline bool						evaluate(s16 jointIdx, AnimationClip::JointPose& outPose) { return m_pRoot->evaluate(m_globalClock, jointIdx, outPose); } 

	/*! Returns layer's curve factor.
		/return layer involvement in the final animation. <0, 1>
	*/
	inline float					getCurveValue() const { return m_fCurveFactor; }

private:
	//! Definition of animation layer's states and transitions.
	AnimationStatesPtr				m_pAnimationStates;

	//! Animation layer clock. Updated every frame.
	float							m_globalClock;

	//! Current state's name.
	StringId						m_currentStateName;

	//! State's animation tree.
	AnimExprNode*					m_pRoot;

	//! Current animation curve state (value, crossfade, ease-in, ease-out, etc.)
	AnimationCurve m_eCurrentCurve;

	//! Timer used to resolve curves.
	float m_fCurveTimerMs;

	//! Current curve duration.
	float m_fCurveDurationMs;

	//! Defines layer input for final animation blend.
	float m_fCurveFactor;
};

//---------------------------------------------------------------------------------------
//typedef shared_ptr<AnimationSet> AnimationSetPtr;
//---------------------------------------------------------------------------------------
} // namespace Engine
//---------------------------------------------------------------------------------------
//#include "engine\AnimationSet.inl"
//---------------------------------------------------------------------------------------