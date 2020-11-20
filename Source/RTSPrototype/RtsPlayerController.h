// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RtsPlayerController.generated.h"

class ARTSPrototypeCharacter;
class AGameHUD;
class ACameraPawn;
class ABuilding;

/**
 * 
 */

UENUM()
enum class EPlayerState : uint8
{
	Default,		// Able to select units, give commands and perform most of what the game requires
	Placing,		// Only able to move buildings, place and exit placing
};

UCLASS()
class RTSPROTOTYPE_API ARtsPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ARtsPlayerController();
	TArray<ARTSPrototypeCharacter*> SelectedUnits;
	TArray<ABuilding*> SelectedBuildings;

	virtual void PlayerTick(float DeltaTime) override;

private:
	EPlayerState PlayerState;

	ACameraPawn* PlayerPawn;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	AGameHUD* HUD{nullptr};

	void MoveTo();

	FHitResult Hit;

	void LeftMousePress();
	void LeftMouseRelease();

	void RightMousePress();
	void RightMouseRelease();

	void SelectionInitiate();
	void SelectionTerminate();

	AActor* PlacementBuffer;

	void CreateGoldBuilding();
	void CreateUnitBuilding();

	void CreateUnit();

	void PositionPlacement();

	void ChangeState(EPlayerState NewState);

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABuilding> GoldBuildingClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<ABuilding> UnitBuildingClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ARTSPrototypeCharacter> UnitClass;

	FActorSpawnParameters BuildingSpawnParams;

	bool IsPressLeft = false;
};