// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Channel names come from DefaultEngine.ini DefaultChannelResponses.
// GameTraceChannel1 is declared as "Climbable" and GameTraceChannel2 as
// "Projectile"; ECC_Projectile previously pointed at channel 1, so projectiles
// were using the climbable channel and enemy meshes were overlapping it.
#define ECC_Climbable ECollisionChannel::ECC_GameTraceChannel1
#define ECC_Projectile ECollisionChannel::ECC_GameTraceChannel2