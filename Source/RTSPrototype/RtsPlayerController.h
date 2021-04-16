// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RTSPrototypeCharacter.h"
#include "RtsPlayerController.generated.h"

class AGameHUD;
class ACameraPawn;
class ARTSPrototypeCharacter;
class ABuilding;
class AAIController;
class UMaterialInstance;

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

	UPROPERTY(replicated)
	TArray<ARTSPrototypeCharacter*> SelectedUnits;
	UPROPERTY(replicated)
	TArray<ABuilding*> SelectedBuildings;

	UFUNCTION(BlueprintCallable)
	FName GetUserName();

	void NextMoveQueue(bool IsAggro);

	UPROPERTY(replicated)
	TArray<FHitResult> QueuedMovements{};

	// Place in queue
	uint8 QueueNum = 0;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetUsername(const FName& NewUserName);

	virtual void PlayerTick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, replicated)
	APawn* ControlledPawn = nullptr;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ChangePlayerState(EPlayerState NewState);
	
	UPROPERTY(replicated)
	EPlayerState RTSPlayerState = EPlayerState::Default;

	UPROPERTY(replicated)
	bool bCanPosition = true;

protected:
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetPlayerPawn(ACameraPawn* Camera);

private:

	FVector StartLoc;

	FHitResult MoveToHit;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MultiRightMouse(const TArray<ARTSPrototypeCharacter*>& Units, FHitResult Hit);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SinglePatrol(const TArray<ARTSPrototypeCharacter*>& Units, FHitResult Hit);

	bool bIsShiftPress;

	void ShiftPress();

	void ShiftReleased();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetBuildingState(ABuilding* NewBuilding, EBuildingState NewState);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetUnitState(ARTSPrototypeCharacter* Unit, ECharacterState NewState);

	UPROPERTY(replicated)
	ACameraPawn* PlayerPawn;

	UPROPERTY(VisibleAnywhere, replicated)
	FName UserName = "";

	FTimerHandle Timer;

	void CanPosition();

	void StartPatrol(FHitResult First);

	UPROPERTY(replicated)
	bool bAggressive = false;

	UPROPERTY(replicated)
	bool bPatrol = false;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetAggression();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_InversePatrol();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	AGameHUD* HUD{nullptr};

	void MoveTo();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MoveTo(FHitResult Hit, const TArray<ARTSPrototypeCharacter*>& Units);

	void LeftMousePress();
	void LeftMouseRelease();

	void RightMousePress();
	void RightMouseRelease();

	void SelectionInitiate();
	void SelectionTerminate();

	UPROPERTY(replicated)
	AActor* PlacementBuffer;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_CreateGoldBuilding();
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_CreateUnitBuilding();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_CreateUnit(TSubclassOf<ARTSPrototypeCharacter> UnitClass);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_BuyUnit(uint8 UnitCost);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_DestroyUnit();

	UFUNCTION(Client, Reliable)
	void PrepareUnit(AActor* NewUnit);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_PositionPlacement(FHitResult HitRes, AActor* UnitToPlace);

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABuilding> GoldBuildingClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<ABuilding> UnitBuildingClass;

	void UnitModifierButtons();
	void APress();
	void SPress();
	void DPress();
	void FPress();

	UPROPERTY(replicated)
	bool bUnitButtons = false;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ARTSPrototypeCharacter> DefaultUnitClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<ARTSPrototypeCharacter> MeleeClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<ARTSPrototypeCharacter> RangedClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<ARTSPrototypeCharacter> TankClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<ARTSPrototypeCharacter> SpeedClass;

	FActorSpawnParameters BuildingSpawnParams;

	bool IsPressLeft = false;
};
