// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RTSPrototypePlayerController.generated.h"

UCLASS()
class ARTSPrototypePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ARTSPrototypePlayerController();
	/** Navigate player to the given world location. */
	void SetNewMoveDestination(const FVector DestLocation);

protected:
};


