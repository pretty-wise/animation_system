#include "engine/animation/AnimationSystem.h"
#include "engine/animation/Skeleton.h"

//---------------------------------------------------------------------------------------
namespace Engine{
//---------------------------------------------------------------------------------------

AnimationSystem::AnimationSystem( u32 max_controller_count )
: m_controllers(max_controller_count)
{
    
}

//---------------------------------------------------------------------------------------
    
Handle AnimationSystem::CreateController( Skeleton* skeleton, u32 layer_count )
{
    if( !m_controllers.CanAdd() )
        return (Handle)-1;
    
    Handle h = m_controllers.Add();
    AnimController& controller = m_controllers.Get(h);
    controller.Initialize( skeleton, layer_count );
    
    return h;
}

//---------------------------------------------------------------------------------------

void AnimationSystem::DestroyController( Handle h )
{
    AnimController& controller = m_controllers.Get(h);
    controller.Release();
    m_controllers.Remove(h);
}

//---------------------------------------------------------------------------------------

bool AnimationSystem::HasController( Handle h )
{
    return m_controllers.Exists(h);
}

//---------------------------------------------------------------------------------------

AnimController& AnimationSystem::GetController( Handle h )
{
    return m_controllers.Get(h);
}

//---------------------------------------------------------------------------------------

void AnimationSystem::Update( float delta_ms )
{
    for( u32 i = 0; i < m_controllers.Count(); ++i )
    {
        AnimController& controller = m_controllers[i];
        
        controller.Update( delta_ms );
    }
}
    
//---------------------------------------------------------------------------------------

void AnimationSystem::LocalPoseCalculation()
{
    for( u32 i = 0; i < m_controllers.Count(); ++i )
    {
        AnimController& controller = m_controllers[i];
        Skeleton* skeleton = controller.GetSkeleton();
        AnimHierarchy* hierarchy = controller.GetHierarchy();
        
        for( u16 j = 0; j < skeleton->GetJointCount(); ++j )
        {
            AnimationClip::JointPose local_pose;
            controller.GetJointPose( j, local_pose );
            
            AnimTransformation& node = hierarchy->GetNode(j);
            
            node.SetTranslation( local_pose.translation );
            node.SetScale( local_pose.scale );
            node.SetRotation( local_pose.rotation );
            
            node.CalculateLocalTransformation();
        }
    }
}

//---------------------------------------------------------------------------------------

void AnimationSystem::GlobalPoseCalculation()
{
    for( u32 i = 0; i < m_controllers.Count(); ++i )
    {
        AnimController& controller = m_controllers[i];
        AnimHierarchy* hierarchy = controller.GetHierarchy();
        
        hierarchy->CalculateGlobalTransformation();
    }
 }

//---------------------------------------------------------------------------------------

void AnimationSystem::MatrixPaletteGeneration()
{
    for( u32 i = 0; i < m_controllers.Count(); ++i )
    {
        AnimController& controller = m_controllers[i];
        AnimHierarchy* hierarchy = controller.GetHierarchy();
        Skeleton* skeleton = controller.GetSkeleton();
        Matrix4x4* palette = controller.GetSkinningPalette();
        
        // we need inverse matrix of root node's world transformation to calculate model-space palette.
        Matrix4x4 parentInvMatrix = hierarchy->GetNode(0).GetWorldTransformation();
        parentInvMatrix.InverseIt();
        
        for( u16 j = 0; j < hierarchy->GetNodeCount(); ++j )
        {
            // creating skinning palette.
            // K = (Bj_M)^-1 * Cj_M
            // palette matrix is inverse bind pose in model-space multiplied by current pose in model-space.
            
            // world-space matrix
            palette[j] = hierarchy->GetNode(j).GetWorldTransformation();
            
            // world-space -> model-space
            palette[j] = parentInvMatrix * palette[j];
            
            // skinning palette matrix ( model-space * inverse bind pose )
            palette[j] *= skeleton->GetInvBindPose(j);
        }
    }
}
    
//---------------------------------------------------------------------------------------
    
void AnimationSystem::Draw( DebugRenderer& rend )
{
    for( u32 i = 0; i < m_controllers.Count(); ++i )
    {
        AnimController& controller = m_controllers[i];
        AnimHierarchy* hierarchy = controller.GetHierarchy();
        
        hierarchy->Draw( rend );
    }
}
    
//---------------------------------------------------------------------------------------
}; //namespace Engine
//---------------------------------------------------------------------------------------

