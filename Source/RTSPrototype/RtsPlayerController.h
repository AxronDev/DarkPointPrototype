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
	Menu = 0,
	Default = 1,		// Able to select units, give commands and perform most of what the game requires
	Placing = 2,		// Only able to move buildings, place and exit placing
};

UCLASS()
class RTSPROTOTYPE_API ARtsPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ARtsPlayerController();
	TArray<ARTSPrototypeCharacter*> SelectedUnits;
	TArray<ABuilding*> SelectedBuildings;

	UFUNCTION(BlueprintCallable)
	FName GetUserName();
	void SetUsername(const FName& NewUserName);

	virtual void PlayerTick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	APawn* ControlledPawn = nullptr;

	void ChangeState(EPlayerState NewState);

private:
	EPlayerState PlayerState;

	ACameraPawn* PlayerPawn;

	FName UserName = "";

	bool bAggressive = false;

	void SetAggression();

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

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABuilding> GoldBuildingClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<ABuilding> UnitBuildingClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ARTSPrototypeCharacter> UnitClass;

	FActorSpawnParameters BuildingSpawnParams;

	bool IsPressLeft = false;
};
