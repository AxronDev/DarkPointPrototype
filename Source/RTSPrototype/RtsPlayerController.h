// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RtsPlayerController.generated.h"

class ARTSPrototypeCharacter;
class AGameHUD;
class ACameraPawn;
class ABuilding;
class AAIController;

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

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const override;

	TArray<ARTSPrototypeCharacter*> SelectedUnits;
	TArray<ABuilding*> SelectedBuildings;

	UFUNCTION(BlueprintCallable)
	FName GetUserName();
	void SetUsername(const FName& NewUserName);

	virtual void PlayerTick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	APawn* ControlledPawn = nullptr;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ChangePlayerState(EPlayerState NewState);
	
	UPROPERTY(replicated)
	EPlayerState RTSPlayerState;

private:

	ACameraPawn* PlayerPawn;

	UPROPERTY(VisibleAnywhere)
	FName UserName = "";

	bool bAggressive = false;

	void SetAggression();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	AGameHUD* HUD{nullptr};

	void MoveTo();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MoveTo();

	FHitResult Hit;

	void LeftMousePress();
	void LeftMouseRelease();

	void RightMousePress();
	void RightMouseRelease();

	void SelectionInitiate();
	void SelectionTerminate();

	UPROPERTY(replicated)
	AActor* PlacementBuffer;

	void CreateGoldBuilding();
	void CreateUnitBuilding();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_CreateUnit();

	UFUNCTION(Client, Reliable)
	void PrepareUnit(AActor* NewUnit);

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
