// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CameraPawn.generated.h"

class ARTSPrototypeCharacter;

UCLASS()
class RTSPROTOTYPE_API ACameraPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACameraPawn();

	/* void LookX(float value);
	void LookY(float value); */
	void Zoom(float value);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void AddGoldBuilding();
	void AddUnitBuilding();

	TArray<AActor*> MyUnits;

	UPROPERTY(BlueprintReadOnly)
	float Gold = 1000;
	UPROPERTY(BlueprintReadOnly)
	float Units;

	UPROPERTY(EditAnywhere)
	float GoldPrice = 150.f;
	UPROPERTY(EditAnywhere)
	float UnitPrice = 100.f;

private:
	UPROPERTY(EditAnywhere)
	float PanRate = 1500;

	FVector MoveDirection;

	int32 GoldBuildings = 1;
	int32 UnitBuildings = 0;

	float GoldRate = 3.f;
	float UnitRate = .5f;

	void MouseMovement(float DeltaTime);
};
