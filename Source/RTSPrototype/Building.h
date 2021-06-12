// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Placeable.h"
#include "Building.generated.h"

class UDecalComponent;
class USceneComponent;
class UMaterialInstanceDynamic;
class UMaterialInstance;
class UAIPerceptionStimuliSourceComponent;
class UAISense;

UENUM()
enum class EBuildingState : uint8
{
	Preview,
	Built,
	Destroyed,
};

UCLASS()
class RTSPROTOTYPE_API ABuilding : public AActor, public IPlaceable
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ABuilding();

	virtual void OnConstruction(const FTransform& Transform);

	EPlaceableState PlaceableState;
	virtual void SetPlaceableState(EPlaceableState NewState) override;
	virtual EPlaceableState GetPlaceableState() override;
	virtual TArray<bool>& GetAttackSlots() override;
	virtual float GetRadius();
	// Initialize in children
	UPROPERTY(BlueprintReadWrite)
	float Radius;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite)
	FName BuildingType;

	UFUNCTION(BlueprintImplementableEvent)
	void OnPlaced();

	UMaterialInstance* TempGold;
	UMaterialInstance* TempUnit;
	UMaterialInstance* TempHealth;

	virtual void OnRep_Owner() override;

	/* UMaterialInstanceDynamic* GoldMat;
	UMaterialInstanceDynamic* UnitMat; */

	UPROPERTY(BlueprintReadOnly)
	UMaterialInstanceDynamic* GoldBuildingMat;

	UPROPERTY(BlueprintReadOnly)
	UMaterialInstanceDynamic* UnitBuildingMat;

	UPROPERTY(BlueprintReadOnly)
	UMaterialInstanceDynamic* HealthBuildingMat;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxHealth = 500;

	UPROPERTY(EditDefaultsOnly, replicated)
	float Health = MaxHealth;

	UPROPERTY(EditDefaultsOnly)
	UAIPerceptionStimuliSourceComponent* Stimuli;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAISense> SightSense;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) override;

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void Server_SetBuildingMaterials(UMaterialInstanceDynamic* Gold, UMaterialInstanceDynamic* Unit, UMaterialInstanceDynamic* HealthMat);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDecalComponent* CursorToWorld;

	UPROPERTY(EditDefaultsOnly)
	USceneComponent* Root;

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void Server_SetBuildingState(EBuildingState NewState);

	EBuildingState GetBuildingState();

	FName GetBuildingType();

	void SetOwnerUserName(FName UserName);

	UFUNCTION(BlueprintCallable)
	virtual FName GetOwnerUserName() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<bool> AttackSlots;

	UPROPERTY(replicated)
	bool bHasBeenPositioned = false;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Decal")
	void CanPlace(bool val);

	UPROPERTY(BlueprintReadWrite)
	bool bCanPlace = false;

	UFUNCTION(BlueprintCallable)
	float GetHealth();

private:
	UPROPERTY(VisibleAnywhere, replicated)
	EBuildingState BuildingState = EBuildingState::Built;

	UPROPERTY(replicated)
	FName OwnerUserName = "";

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Death();

};
