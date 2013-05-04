#pragma once

#include "engine/core/Types.h"
#include "engine/core/ObjectArray.h"
#include "engine/animation/AnimController.h"

//---------------------------------------------------------------------------------------
namespace Engine{
//---------------------------------------------------------------------------------------

class Skeleton;
    
class AnimationSystem
{
public:
    AnimationSystem( u32 max_controller_count );
    
    Handle CreateController( Skeleton* skeleton, u32 layer_count );
    void DestroyController( Handle h );
    bool HasController( Handle h );
    
    AnimController& GetController( Handle h );
    
public:
    // updates animation controllers timer.
    void Update( float delta_ms );
    
    // calculates skeleton local transformation matrices.
    void LocalPoseCalculation();
    
    // calculates skeleton global transformation matrices.
    void GlobalPoseCalculation();
    
    // creates matrix palettes for all animation controllers.
    void MatrixPaletteGeneration();
    
    // renders animated skeletal poses.
    void Draw( DebugRenderer& rend );
    
private:
    ObjectArray<AnimController> m_controllers;
};

//---------------------------------------------------------------------------------------
}
//---------------------------------------------------------------------------------------
