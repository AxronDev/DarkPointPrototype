// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Building.generated.h"

class UDecalComponent;

UCLASS()
class RTSPROTOTYPE_API ABuilding : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABuilding();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite)
	FName BuildingType;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UDecalComponent* CursorToWorld;

	FName GetBuildingType();
};