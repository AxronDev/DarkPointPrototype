// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Building.h"
#include "GoldBuilding.generated.h"

/**
 * 
 */
UCLASS()
class RTSPROTOTYPE_API AGoldBuilding : public ABuilding
{
	GENERATED_BODY()
	
public:
	float BuildingPrice = 150;
};
