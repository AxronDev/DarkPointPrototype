// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitAIController.h"
#include "Net/UnrealNetwork.h"
#include "RTSPrototypeCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Building.h"
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
     if(Char && Target)
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
     else if(EnemyUnits.Find(Target) == INDEX_NONE)
     {
          bFindNewTarget = true;
     }
     else if(Cast<ARTSPrototypeCharacter>(Target) != nullptr)
     {
          UE_LOG(LogTemp, Warning, TEXT("Cast Succeded Assign Unit Target"));
          if(Cast<ARTSPrototypeCharacter>(Target)->GetCharacterState() == ECharacterState::Dead)
          {
               UE_LOG(LogTemp, Warning, TEXT("Old target is dead"));
               bFindNewTarget = true;
          }
     }

     if(bFindNewTarget == true)
     {
          Target = nullptr;
          for(uint8 UnitIndex = 0; UnitIndex < EnemyUnits.Num(); UnitIndex++)
          {
               for(uint8 Slot = 0; Slot < Char->AttackSlots.Num(); Slot++)
               {
                    if(EnemyUnits[UnitIndex])
                    {
                         if(Cast<ARTSPrototypeCharacter>(EnemyUnits[UnitIndex]))
                         {
                              if(Cast<ARTSPrototypeCharacter>(EnemyUnits[UnitIndex])->AttackSlots[Slot] == false)
                              {
                                   Cast<ARTSPrototypeCharacter>(EnemyUnits[UnitIndex])->AttackSlots[Slot] = true;
                                   Target = EnemyUnits[UnitIndex];
                                   UE_LOG(LogTemp, Warning, TEXT("Unit being Assigned as Target"));
                                   Slot = Char->AttackSlots.Num();
                                   UnitIndex = EnemyUnits.Num();
                              }
                         }
                         else if(Cast<ABuilding>(EnemyUnits[UnitIndex]))
                         {
                              if(Cast<ABuilding>(EnemyUnits[UnitIndex])->AttackSlots[Slot] == false)
                              {
                                   Cast<ABuilding>(EnemyUnits[UnitIndex])->AttackSlots[Slot] = true;
                                   Target = EnemyUnits[UnitIndex];
                                   UE_LOG(LogTemp, Warning, TEXT("Building being Assigned as Target"));
                                   Slot = Char->AttackSlots.Num();
                                   UnitIndex = EnemyUnits.Num();
                              }
                         }
                    }
               }
          }
     }
}

void AUnitAIController::SortEnemyObjects(const TArray<AActor*>& Actors) 
{
     if(Actors.Num() >= 1)
     {
          // UE_LOG(LogTemp, Warning, TEXT("Perception Updated"));
          for(AActor* Unit : Actors)
          {
               // Check if Unit
               if(Cast<ARTSPrototypeCharacter>(Unit))
               {
                    // Check if enemy unit
                    if(Cast<ARTSPrototypeCharacter>(Unit)->GetOwnerUserName() != Char->GetOwnerUserName())
                    {
                         FActorPerceptionBlueprintInfo Info;
                         AIPercep->GetActorsPerception(Unit, Info);
                         if(Info.LastSensedStimuli.Num() == 0)
                         {
                              break;
                         }
                         FAIStimulus Stim = Info.LastSensedStimuli[0];
                         // Check if sight is lost
                         if(Stim.WasSuccessfullySensed() == false || Stim.GetAge() >= MaxAge || Cast<ARTSPrototypeCharacter>(Unit)->GetCharacterState() == ECharacterState::Dead)
                         {
                              int Index = EnemyUnits.Remove(Unit);
                         }
                         else
                         {
                              FString UnitName = GetDebugName(Unit);   
                              UE_LOG(LogTemp, Warning, TEXT("Can see unit %s"), *UnitName);
                              EnemyUnits.Add(Unit);
                         }
                    }
               }
               else if(Cast<ABuilding>(Unit))
               {
                    // Check if enemy building
                    if(Cast<ABuilding>(Unit)->GetOwnerUserName() != Char->GetOwnerUserName())
                    {
                         FActorPerceptionBlueprintInfo Info;
                         AIPercep->GetActorsPerception(Unit, Info);
                         if(Info.LastSensedStimuli.Num() == 0)
                         break;
                         FAIStimulus Stim = Info.LastSensedStimuli[0];
                         // Check if sight is lost
                         if(Stim.WasSuccessfullySensed() == false || Stim.GetAge() >= MaxAge)
                         {
                              int Index = EnemyUnits.Remove(Unit);
                         }
                         else
                         {
                              FString UnitName = GetDebugName(Unit);   
                              UE_LOG(LogTemp, Warning, TEXT("Can see building %s"), *UnitName);
                              EnemyUnits.Add(Unit);
                         }
                    }
               }
          }

          AssignTarget();
     }

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
               // UE_LOG(LogTemp, Warning, TEXT("Aggressive in AIController Tick"));
               if(Target == nullptr || EnemyUnits.Find(Target) == INDEX_NONE)
               {
                    /* TArray<AActor*> Actors{};
                    for(uint8 i = 0; i < Actors.Num(); i++)
                    {
                         Actors[i] = Cast<AActor>(EnemyUnits[i]);
                    } */
                    SortEnemyObjects(EnemyUnits);
                    UE_LOG(LogTemp, Warning, TEXT("Target is null or not in Enemy array in AIController Tick"));
               }
               else if(Cast<ARTSPrototypeCharacter>(Target))
               {
                    if(Cast<ARTSPrototypeCharacter>(Target)->GetCharacterState() == ECharacterState::Dead)
                    {
                         /* TArray<AActor*> Actors{};
                         for(uint8 i = 0; i < Actors.Num(); i++)
                         {
                              Actors[i] = Cast<AActor>(EnemyUnits[i]);
                         } */
                         SortEnemyObjects(EnemyUnits);
                         UE_LOG(LogTemp, Warning, TEXT("Target Dead AIController Tick"));
                    }

                    UE_LOG(LogTemp, Warning, TEXT("Target is Valid1 AIController Tick"));
                    MoveToActor(Target, Char->DistToTarget, true, true, true);
                    
                    if(WithinRange())
                    {
                         UE_LOG(LogTemp, Display, TEXT("Within range"));
                         if(Char->bCanAttack == true)
                         {
                              UE_LOG(LogTemp, Display, TEXT("Can attack"));
                              Char->Attack(Target);
                              Char->bCanAttack = false;
                              GetWorldTimerManager().SetTimer(Timer, this, &AUnitAIController::CanAttack, Char->AttackSpeed);
                         }
                    }
               }
               else if(Cast<ABuilding>(Target))
               {
                    UE_LOG(LogTemp, Warning, TEXT("Target is Valid1 AIController Tick"));
                    MoveToActor(Target, Char->DistToTarget, true, true, true);
                    
                    if(WithinRange())
                    {
                         UE_LOG(LogTemp, Display, TEXT("Within range"));
                         if(Char->bCanAttack == true)
                         {
                              UE_LOG(LogTemp, Display, TEXT("Can attack"));
                              Char->Attack(Target);
                              Char->bCanAttack = false;
                              GetWorldTimerManager().SetTimer(Timer, this, &AUnitAIController::CanAttack, Char->AttackSpeed);
                         }
                    }
               }
          }
          else if(Char->GetCharacterState() == ECharacterState::Passive)
          {
               // AIPercep->SetSenseEnabled(SightSenseClass, false);
               // UE_LOG(LogTemp, Warning, TEXT("Passive in AIController Tick"));
          }
          
     }
     else
     {
          UE_LOG(LogTemp, Warning, TEXT("Char is null in AIController Tick"));
     }
}