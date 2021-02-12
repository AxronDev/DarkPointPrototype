// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Placeable.generated.h"

UCLASS()
class RTSPROTOTYPE_API APlaceable : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlaceable();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	uint8 Price = 0;

};
