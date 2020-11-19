// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GameHUD.generated.h"

/**
 * 
 */
UCLASS()
class RTSPROTOTYPE_API AGameHUD : public AHUD
{
private:
	GENERATED_BODY()
	
	virtual void DrawHUD();

public:
	bool SelectPressed{false};
	FVector2D InitMousePos{NULL};
	FVector2D CurrentMousePos{NULL};
};
