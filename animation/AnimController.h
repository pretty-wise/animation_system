#pragma once

#include "engine/core/Types.h"
#include "engine/core/StringId.h"
#include "engine/core/ObjectArray.h"
#include "engine/math/Matrix4x4.h"
#include "engine/animation/AnimationClip.h"
#include "engine/animation/AnimLayer.h"
#include "engine/animation/AnimHierarchy.h"

//---------------------------------------------------------------------------------------
namespace Engine{
//---------------------------------------------------------------------------------------
    
class Skeleton;

class AnimController : public IObject
{
public:
    AnimController();
    
    // creates hierarchy and allocates matrix palette based on skeleton.
    void Initialize( Skeleton* skeleton, u32 layer_count );
    
    // destroys hierarchy and matrix palette.
    void Release();
    
    // updates all the layers.
    void Update( float delta_ms );
    
    // layer accessor.
    AnimLayer& GetLayer( u16 index );
    
    // skeleton getter.
    inline Skeleton* GetSkeleton();
    
    // hierarchy getter.
    inline AnimHierarchy* GetHierarchy();
    
    // skinning palette getter.
    inline Matrix4x4* GetSkinningPalette();
    
    // extracts a pose for a given joint in its current animation state.
    void GetJointPose( u16 joint_idx, AnimationClip::JointPose& pose );
    
private:
    // single animation layer supported now.
    AnimLayer* m_layers;
    
    // number of layers in controller.
    u32 m_layer_count;
    
    // animation skeleton.
    Skeleton* m_skeleton;
    
    // local/global animation hierarchy.
    AnimHierarchy* m_hierarchy;
    
    // skinning matrix palette used in shader. may be changed to dual quaternion representation in the future.
    Matrix4x4* m_skinning_palette;
};
    
//---------------------------------------------------------------------------------------
    
inline Skeleton* AnimController::GetSkeleton()
{
    return m_skeleton;
}

//---------------------------------------------------------------------------------------

inline AnimHierarchy* AnimController::GetHierarchy()
{
    return m_hierarchy;
}

//---------------------------------------------------------------------------------------

inline Matrix4x4* AnimController::GetSkinningPalette()
{
    return m_skinning_palette;
}

//---------------------------------------------------------------------------------------
} // namespace Engine
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------