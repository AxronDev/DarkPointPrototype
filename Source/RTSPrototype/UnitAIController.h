// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "UnitAIController.generated.h"

/**
 * 
 */
UCLASS()
class RTSPROTOTYPE_API AUnitAIController : public AAIController
{
	GENERATED_BODY()
	
	AUnitAIController();

public:

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void MoveUnit(FVector Destination);
	
};
