#pragma once

#include "engine/core/StringId.h"
#include "engine/math/Matrix4x4.h"

//---------------------------------------------------------------------------------------
namespace Engine{
//---------------------------------------------------------------------------------------

class DebugRenderer;
    
class SkeletonJoint
{
public:
    SkeletonJoint();
    SkeletonJoint(StringId name, u8 parentIndex, const Matrix4x4& invBindPose);
    
    inline const Matrix4x4& GetInvBindPose() const { return m_inv_bind_pose; }
    
    inline StringId GetName() const { return m_name; }
    
    inline u32 GetParentIndex() const { return m_parent_index; }
    
private:
    // joints name.
    StringId m_name;
    
    // index of parent joint in skeleton joints array in Skeleton class.
    u32 m_parent_index;
    
    // used to produce skinning matrix which is K = Joints Inverse Bind Pose (Model Space) * Joints Current Pose (Model Space)
    Matrix4x4 m_inv_bind_pose;
};
    
//---------------------------------------------------------------------------------------
    
class Skeleton
{
public:
	Skeleton();

	void BreakPointers();
	void FixPointers();
    
	inline StringId GetName() const;
    
	inline u32 GetJointCount() const;
    
	inline const SkeletonJoint& GetJoint(u32 idx) const;
    
	inline const Matrix4x4& GetInvBindPose(u32 idx) const;
    
    void Draw( DebugRenderer& rend );
    
private:
    // skeleton name.
	StringId m_name;
    
    // number of joints in skeleton.
	u32 m_joint_count;
    
    // skeleton joint array.
	SkeletonJoint* m_joints;
};

//---------------------------------------------------------------------------------------

inline StringId Skeleton::GetName() const
{
    return m_name;
}

//---------------------------------------------------------------------------------------

inline u32 Skeleton::GetJointCount() const
{
    return m_joint_count;
}

//---------------------------------------------------------------------------------------

inline const SkeletonJoint& Skeleton::GetJoint(u32 idx) const
{
    ENGINE_ASSERT(idx < m_joint_count, "index out of bounds");
    return m_joints[idx];
}

//---------------------------------------------------------------------------------------

inline const Matrix4x4& Skeleton::GetInvBindPose(u32 idx) const
{
    ENGINE_ASSERT(idx < m_joint_count, "index out of bounds");
    return m_joints[idx].GetInvBindPose();
}
    
//---------------------------------------------------------------------------------------
} // namespace Engine
//---------------------------------------------------------------------------------------