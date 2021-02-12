// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitAIController.h"
#include "Net/UnrealNetwork.h"
#include "RTSPrototypeCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Navigation/PathFollowingComponent.h"
#include "TimerManager.h"
#include "Engine/EngineTypes.h"
#include "Perception/AISenseConfig_Sight.h"

AUnitAIController::AUnitAIController() 
{
     bReplicates = true;

     AIPercep = CreateDefaultSubobject<UAIPerceptionComponent>("SightPerception");
     SightSenseConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("Sight Config");

     // Setup SightSenseConfig
     SightSenseConfig->SightRadius = 800.f;
     SightSenseConfig->LoseSightRadius = 1000.f;
     SightSenseConfig->PeripheralVisionAngleDegrees = 360.f;
     SightSenseConfig->DetectionByAffiliation.bDetectEnemies = true;
     SightSenseConfig->DetectionByAffiliation.bDetectNeutrals = true;
     SightSenseConfig->DetectionByAffiliation.bDetectFriendlies = true;
     SightSenseConfig->SetMaxAge(MaxAge);

     AIPercep->ConfigureSense(*SightSenseConfig);
     AIPercep->SetDominantSense(SightSenseConfig->GetSenseImplementation());
     AIPercep->OnPerceptionUpdated.AddDynamic(this, &AUnitAIController::SortEnemyObjects);
}

void AUnitAIController::BeginPlay() 
{
     Super::BeginPlay();
     PrimaryActorTick.bCanEverTick = true;

     AIPercep->SetSenseEnabled(SightSenseClass, true);
}

bool AUnitAIController::WithinRange() 
{
     // UE_LOG(LogTemp, Display, TEXT("Target is %f cm away"), Target->GetDistanceTo(Char));
     if(Target->GetDistanceTo(Char) <= Char->AttackDist && Target->GetDistanceTo(Char) != -2.f)
     {
          return true;
     }
     else
     {
          return false;
     }
}

void AUnitAIController::CanAttack() 
{
    Char->bCanAttack = true;
}

void AUnitAIController::AssignTarget() 
{
     bool bFindNewTarget = false;
     if(Target == nullptr)
     {
          bFindNewTarget = true;
     }

     else if(EnemyUnits.Find(Target) == INDEX_NONE || Cast<ARTSPrototypeCharacter>(Target)->GetCharacterState() == ECharacterState::Dead)
     {
          bFindNewTarget = true;
     }

     if(bFindNewTarget == true)
     {
          Target = nullptr;
          UE_LOG(LogTemp, Display, TEXT("Target null or not in Enemy array"));
          for(uint8 UnitIndex = 0; UnitIndex < EnemyUnits.Num(); UnitIndex++)
          {
               for(uint8 Slot = 0; Slot < Char->AttackSlots.Num(); Slot++)
               {
                    if(EnemyUnits[UnitIndex]->AttackSlots[Slot] == false)
                    {
                         if(Cast<ARTSPrototypeCharacter>(EnemyUnits[UnitIndex]))
                         {
                              EnemyUnits[UnitIndex]->AttackSlots[Slot] = true;
                              Target = Cast<ARTSPrototypeCharacter>(EnemyUnits[UnitIndex]);
                              UE_LOG(LogTemp, Warning, TEXT("Cast Succeded AssignTarget"));
                              Slot = Char->AttackSlots.Num();
                              UnitIndex = EnemyUnits.Num();
                         }
                         else
                         {
                              UE_LOG(LogTemp, Warning, TEXT("Cast Failed AssignTarget"));
                         }
                    }
                    /* FString UnitName = GetDebugName(units);
                    FString Auth;    
                    UE_LOG(LogTemp, Warning, TEXT("Can see unit controller %s"), *UnitName); */
               }
          }
     }
}

void AUnitAIController::SortEnemyObjects(const TArray<AActor*>& Actors) 
{
     // UE_LOG(LogTemp, Warning, TEXT("Perception Updated"));
     for(AActor* Unit : Actors)
     {
          // Check if Unit
          if(Cast<ARTSPrototypeCharacter>(Unit))
          {
               // Check if enemy
               if(Cast<ARTSPrototypeCharacter>(Unit)->GetOwnerUserName() != Char->GetOwnerUserName())
               {
                    FActorPerceptionBlueprintInfo Info;
                    AIPercep->GetActorsPerception(Unit, Info);
                    if(Info.LastSensedStimuli.Num() == 0)
                    break;
                    FAIStimulus Stim = Info.LastSensedStimuli[0];
                    // Check if sight is lost
                    if(Stim.WasSuccessfullySensed() == false || Stim.GetAge() >= MaxAge || Cast<ARTSPrototypeCharacter>(Unit)->GetCharacterState() == ECharacterState::Dead)
                    {
                         int Index = EnemyUnits.Remove(Cast<ARTSPrototypeCharacter>(Unit));
                    }
                    else
                    {
                         FString UnitName = GetDebugName(Unit);   
                         UE_LOG(LogTemp, Warning, TEXT("Can see unit controller %s"), *UnitName);
                         EnemyUnits.Add(Cast<ARTSPrototypeCharacter>(Unit));
                    }
               }
          }
     }

     AssignTarget();
}

void AUnitAIController::Server_GetAICharacter_Implementation() 
{
     Char = GetPawn<ARTSPrototypeCharacter>();
     if(Char)
     {
          // UE_LOG(LogTemp, Warning, TEXT("Got Char in Server_GetAICharacter"));
     }
     else
     {
          UE_LOG(LogTemp, Warning, TEXT("Failed to get Char in Server_GetAICharacter"));
     }
     
}

bool AUnitAIController::Server_GetAICharacter_Validate() 
{
     return true;
}

void AUnitAIController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const 
{
     Super::GetLifetimeReplicatedProps(OutLifetimeProps);
     DOREPLIFETIME(AUnitAIController, Char);
}

void AUnitAIController::Tick(float DeltaSeconds)
{
     Super::Tick(DeltaSeconds);

     if(Char)
     {
          if(Char->GetCharacterState() == ECharacterState::Aggressive)
          {
               if(Target == nullptr || EnemyUnits.Find(Target) == INDEX_NONE || Cast<ARTSPrototypeCharacter>(Target)->GetCharacterState() == ECharacterState::Dead)
               {
                    TArray<AActor*> Actors{};
                    for(uint8 i = 0; i < Actors.Num(); i++)
                    {
                         Actors[i] = Cast<AActor>(EnemyUnits[i]);
                    }
                    SortEnemyObjects(Actors);
               }
               else
               {
                    MoveToActor(Target, Char->DistToTarget, true, true, true);
                    
                    if(WithinRange())
                    {
                         UE_LOG(LogTemp, Display, TEXT("Within range"));
                         if(Char->bCanAttack == true)
                         {
                              UE_LOG(LogTemp, Display, TEXT("Can attack"));
                              Char->Attack(Cast<AActor>(Target));
                              Char->bCanAttack = false;
                              GetWorldTimerManager().SetTimer(Timer, this, &AUnitAIController::CanAttack, Char->AttackSpeed);
                         }
                    }
               }
          }
          else if(Char->GetCharacterState() == ECharacterState::Passive)
          {
               // AIPercep->SetSenseEnabled(SightSenseClass, false);
          }
          
     }
     else
     {
          UE_LOG(LogTemp, Warning, TEXT("Char is null in AIController Tick"));
     }
}