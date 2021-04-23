// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Placeable.generated.h"

UENUM()
enum class EPlaceableState : uint8
{
	Preview,
	Placed,
	Destroyed,
};

UINTERFACE(MinimalAPI)
class UPlaceable : public UInterface
{
	GENERATED_BODY()
};

class RTSPROTOTYPE_API IPlaceable
{
	GENERATED_BODY()
	
public:
	virtual void SetPlaceableState(EPlaceableState NewState) = 0;
	virtual EPlaceableState GetPlaceableState() = 0;
	virtual TArray<bool>& GetAttackSlots() = 0;
	virtual FName GetOwnerUserName() = 0;
	virtual float GetRadius() = 0;

};
