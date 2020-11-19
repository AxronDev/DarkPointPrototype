// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Building.h"
#include "UnitBuilding.generated.h"

/**
 * 
 */
UCLASS()
class RTSPROTOTYPE_API AUnitBuilding : public ABuilding
{
	GENERATED_BODY()

public:
	float BuildingPrice = 100;
};
