// Fill out your copyright notice in the Description page of Project Settings.


#include "RtsPlayerController.h"
#include "RTSPrototypeCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "CameraPawn.h"
#include "UnitAIController.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NavigationSystem.h"
#include "RTSPrototype/GameHUD.h"
#include "AIController.h"
#include "Building.h"
#define NETMODE_WORLD (((GEngine == nullptr) || (GetWorld() == nullptr)) ? TEXT("") \
: (GEngine->GetNetMode(GetWorld()) == NM_Client) ? TEXT("[Client] ") \
: (GEngine->GetNetMode(GetWorld()) == NM_ListenServer) ? TEXT("[ListenServer] ") \
: (GEngine->GetNetMode(GetWorld()) == NM_DedicatedServer) ? TEXT("[DedicatedServer] ") \
: TEXT("[Standalone] "))

ARtsPlayerController::ARtsPlayerController()
{
     bReplicates = true;
     bShowMouseCursor = true;
     DefaultMouseCursor = EMouseCursor::Default;
     UGameplayStatics::SetPlayerControllerID(this, 1);
}

void ARtsPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const {
     Super::GetLifetimeReplicatedProps(OutLifetimeProps);
     DOREPLIFETIME(ARtsPlayerController, RTSPlayerState);
     DOREPLIFETIME(ARtsPlayerController, PlacementBuffer); // below this is new
     DOREPLIFETIME(ARtsPlayerController, UserName);
     DOREPLIFETIME(ARtsPlayerController, SelectedUnits);
     DOREPLIFETIME(ARtsPlayerController, SelectedBuildings);
     DOREPLIFETIME(ARtsPlayerController, PlayerPawn);
     DOREPLIFETIME(ARtsPlayerController, bAggressive);
}

FName ARtsPlayerController::GetUserName() 
{
     return UserName;
}

void ARtsPlayerController::Server_SetUsername_Implementation(const FName& NewUserName)
{
     UserName = NewUserName;

     FString NameLog;
     UserName.ToString(NameLog);
     UE_LOG(LogTemp, Warning, TEXT("UserName set to %s on %s"), *NameLog, NETMODE_WORLD);
}

bool ARtsPlayerController::Server_SetUsername_Validate(const FName& NewUserName)
{
     return true;
}

void ARtsPlayerController::PlayerTick(float DeltaTime) 
{
     Super::PlayerTick(DeltaTime);

     ForceNetUpdate();
     // UE_LOG(LogTemp, Warning, TEXT("Forced Net Update Tick"));

     if(PlayerPawn == nullptr && HasAuthority())
     {
          Server_SetPlayerPawn();
     }

     // Return Enum Value
     const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EPlayerState"), true);
     FString EnumName = EnumPtr->GetDisplayNameText((uint8)RTSPlayerState).ToString();
     // GEngine->AddOnScreenDebugMessage(0, 2, FColor::Red, *EnumPtr->GetDisplayNameText((uint8)PlayerState).ToString());

     if(RTSPlayerState == EPlayerState::Menu) return;

     if(RTSPlayerState != EPlayerState::Default)
     {
          FHitResult HitPlacement;
          GetHitResultUnderCursor(ECC_GameTraceChannel2, false, HitPlacement);

          // Update building location
          Server_PositionPlacement(HitPlacement, PlacementBuffer);
     }
     if(IsPressLeft == true)
     {
          if(PlayerPawn->Units >= 1)
          {
               if(Cast<ARTSPrototypeCharacter>(PlacementBuffer)->bHasSpace == true)
               {
                    PlayerPawn->Units -= 1;
                    Server_ChangePlayerState(EPlayerState::Default);
                    Server_CreateUnit();
               }
          }
     }

     ControlledPawn = GetPawn();
}

void ARtsPlayerController::Server_SetPlayerPawn_Implementation() 
{
     if(GetPawn<ACameraPawn>())
     {
          UE_LOG(LogTemp, Warning, TEXT("Got Pawn %s"), NETMODE_WORLD);
          PlayerPawn = GetPawn<ACameraPawn>();
     }
     else
     {
          UE_LOG(LogTemp, Warning, TEXT("Failed to get Pawn %s"), NETMODE_WORLD);
     }
}

bool ARtsPlayerController::Server_SetPlayerPawn_Validate() 
{
     return true;
}


void ARtsPlayerController::SetAggression() 
{
     bAggressive = !bAggressive;
}

void ARtsPlayerController::BeginPlay()
{
     HUD = Cast<AGameHUD>(GetHUD());

     BuildingSpawnParams.Owner = Cast<AActor>(GetPawn());

     RTSPlayerState = EPlayerState::Default;

     PlayerPawn = Cast<ACameraPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

     UE_LOG(LogTemp, Warning, TEXT("PlayerController made %s"), NETMODE_WORLD);
     if(HasAuthority())
     {
          Server_SetUsername(FName("Axron"));
          if(GetPawn<ACameraPawn>())
          {
               UE_LOG(LogTemp, Warning, TEXT("Got Pawn with auth %s"), NETMODE_WORLD);
               PlayerPawn = GetPawn<ACameraPawn>();
          }
          else
          {
               UE_LOG(LogTemp, Warning, TEXT("Failed to get Pawn with auth %s"), NETMODE_WORLD);
          }
     }
     else
     {
          Server_SetUsername(FName("Josh"));
          Server_SetPlayerPawn();
     }
}

void ARtsPlayerController::SetupInputComponent()
{
     Super::SetupInputComponent();
     InputComponent->BindAction("LeftMouseClick", IE_Pressed, this, &ARtsPlayerController::LeftMousePress);
     InputComponent->BindAction("LeftMouseClick", IE_Released, this, &ARtsPlayerController::LeftMouseRelease);

     InputComponent->BindAction("RightMouseClick", IE_Released, this, &ARtsPlayerController::RightMousePress);

     InputComponent->BindAction("BuildGoldProduction", IE_Released, this, &ARtsPlayerController::Server_CreateGoldBuilding);
     InputComponent->BindAction("BuildUnitProduction", IE_Released, this, &ARtsPlayerController::Server_CreateUnitBuilding);

     InputComponent->BindAction("PlaceUnit", IE_Released, this, &ARtsPlayerController::Server_CreateUnit);
     InputComponent->BindAction("AttackMovement", IE_Released, this, &ARtsPlayerController::SetAggression);
}

void ARtsPlayerController::MoveTo()
{
     /* UE_LOG(LogTemp, Warning, TEXT("MoveTo called"));
	GetHitResultUnderCursor(ECC_GameTraceChannel2, false, Hit);

	if (Hit.bBlockingHit)
	{
          UE_LOG(LogTemp, Warning, TEXT("Hit Something at X: %f Y: %f Z: %f"), Hit.ImpactPoint.X, Hit.ImpactPoint.Y, Hit.ImpactPoint.Z);
		// We hit something, cycle through selected units and move there
          for(ARTSPrototypeCharacter* Unit : SelectedUnits)
          {
               if(bAggressive == true)
               {
                    Unit->ChangeCharacterState(ECharacterState::Aggressive);
                    // UE_LOG(LogTemp, Warning, TEXT("Attack"));
               }
               else 
               {
                    Unit->ChangeCharacterState(ECharacterState::Passive);
                    // UE_LOG(LogTemp, Warning, TEXT("Passive"));
               }
               AUnitAIController* UnitController = Cast<AUnitAIController>(Unit->GetController());
               if(UnitController != nullptr)
               {
                    FString UnitName = Cast<AAIController>(UnitController)->GetDebugName(UnitController);
                    UE_LOG(LogTemp, Warning, TEXT("Attained unit controller %s"), *UnitName);
                    
                    UnitController->MoveUnit(Hit.ImpactPoint);
                    // UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, Hit.ImpactPoint);
               }
               else
               {
                    UE_LOG(LogTemp, Warning, TEXT("Failed to get unit controller"));
               }
               
          }
	} */
     
     FHitResult Hit;
    GetHitResultUnderCursor(ECC_GameTraceChannel2, false, Hit);
     
     Server_MoveTo(Hit, SelectedUnits);

     // reset to passive
     bAggressive = false;
}

void ARtsPlayerController::Server_MoveTo_Implementation(FHitResult Hit, const TArray<ARTSPrototypeCharacter*>& Units)
{// Trace to see what is under the mouse cursor

     UE_LOG(LogTemp, Warning, TEXT("MoveTo called"));

	if (Hit.bBlockingHit)
	{
          UE_LOG(LogTemp, Warning, TEXT("Hit Something at X: %f Y: %f Z: %f"), Hit.ImpactPoint.X, Hit.ImpactPoint.Y, Hit.ImpactPoint.Z);
		// We hit something, cycle through selected units and move there
          for(ARTSPrototypeCharacter* Unit : Units)
          {
               if(bAggressive == true)
               {
                    Unit->ChangeCharacterState(ECharacterState::Aggressive);
                    UE_LOG(LogTemp, Warning, TEXT("Attack"));
               }
               else 
               {
                    Unit->ChangeCharacterState(ECharacterState::Passive);
                    UE_LOG(LogTemp, Warning, TEXT("Passive"));
               }
               AAIController* UnitController = Cast<AAIController>(Unit->GetController());
               if(UnitController != nullptr)
               {
                    FString UnitName = UnitController->GetDebugName(UnitController);
                    UE_LOG(LogTemp, Warning, TEXT("Attained unit controller %s"), *UnitName);
                    if(HasAuthority())
                    {
                         UAIBlueprintHelperLibrary::SimpleMoveToLocation(Unit->GetController(), Hit.ImpactPoint);
                         UE_LOG(LogTemp, Warning, TEXT("SimpleMoveToLocation called in Server MoveTo"));
                    }
               }
               else
               {
                    UE_LOG(LogTemp, Warning, TEXT("Failed to get unit controller"));
               }
               
          }
	}/* 
     if(UnitChar && UnitController)
     {
          UE_LOG(LogTemp, Warning, TEXT("SimpleMoveToLocation called in Server MoveTo"));
          UAIBlueprintHelperLibrary::SimpleMoveToLocation(UnitChar->GetController(), HitResult.ImpactPoint);
     } */
}

bool ARtsPlayerController::Server_MoveTo_Validate(FHitResult Hit, const TArray<ARTSPrototypeCharacter*>& Units)
{
     return true;
}

void ARtsPlayerController::LeftMousePress() 
{
     UE_LOG(LogTemp, Warning, TEXT("Left Mouse Pressed"));
     if(RTSPlayerState == EPlayerState::Menu) return;
     if(RTSPlayerState == EPlayerState::Default)
     {
          SelectionInitiate();
     }
     else
     {
          // Check PlacementBuffer isn't null
          if(Cast<ARTSPrototypeCharacter>(PlacementBuffer))
          {
               IsPressLeft = true;
          }
          else
          {
               // Check if building
               if(Cast<ABuilding>(PlacementBuffer))
               {
                    ABuilding* BuildingBuffer = Cast<ABuilding>(PlacementBuffer);
                    if(BuildingBuffer->GetBuildingType() == FName("Gold Building"))
                         PlayerPawn->Gold -= PlayerPawn->GoldPrice;

                    if(BuildingBuffer->GetBuildingType() == FName("Unit Building"))
                         PlayerPawn->Gold -= PlayerPawn->UnitPrice;
               }
               Server_ChangePlayerState(EPlayerState::Default);
          }
          
     }
     
}

void ARtsPlayerController::LeftMouseRelease() 
{
     if(RTSPlayerState == EPlayerState::Menu) return;
     SelectionTerminate();
     IsPressLeft = false;
     FString LogName;
     UserName.ToString(LogName);
     UE_LOG(LogTemp, Warning, TEXT("Username: %s Left Mouse Released %s"), *LogName, NETMODE_WORLD)
}

void ARtsPlayerController::RightMousePress() 
{
     if(RTSPlayerState == EPlayerState::Menu) return;
     if(RTSPlayerState == EPlayerState::Default)
     {
          MoveTo();
     }
     else
     {
          PlacementBuffer->Destroy();
          Server_ChangePlayerState(EPlayerState::Default);
          
     }
}

void ARtsPlayerController::RightMouseRelease() 
{
     
}

void ARtsPlayerController::SelectionInitiate()
{
     GetMousePosition(HUD->InitMousePos.X, HUD->InitMousePos.Y);
     HUD->SelectPressed = true;
}

void ARtsPlayerController::SelectionTerminate()
{
     HUD->SelectPressed = false;
}

void ARtsPlayerController::Server_CreateGoldBuilding_Implementation() 
{
     if(RTSPlayerState == EPlayerState::Menu) return;

     if(PlayerPawn->Gold >= PlayerPawn->GoldPrice)
     {
          PlacementBuffer = GetWorld()->SpawnActor<ABuilding>(GoldBuildingClass);
          if(PlacementBuffer)
          {
               Cast<ABuilding>(PlacementBuffer)->SetOwnerUserName(UserName);
               PlayerPawn->Server_AddGoldBuilding();
               // Makes Tick call Server_PositionPlacement()
               Server_ChangePlayerState(EPlayerState::Placing);
          }
     }
     else
     {
          Server_ChangePlayerState(EPlayerState::Default);
     }
          
}

bool ARtsPlayerController::Server_CreateGoldBuilding_Validate()
{
     return true;
}

void ARtsPlayerController::Server_CreateUnitBuilding_Implementation() 
{
     if(RTSPlayerState == EPlayerState::Menu) return;

     if(PlayerPawn->Gold >= PlayerPawn->UnitPrice)
     {
          PlacementBuffer = GetWorld()->SpawnActor<ABuilding>(UnitBuildingClass);
          if(PlacementBuffer)
          {
               Cast<ABuilding>(PlacementBuffer)->SetOwnerUserName(UserName);
               
               FString LogName;
               UserName.ToString(LogName);
               UE_LOG(LogTemp, Warning, TEXT("Username: %s create unit building %s"), *LogName, NETMODE_WORLD)
               PlayerPawn->Server_AddUnitBuilding();
               // Makes Tick call Server_PositionPlacement()
               Server_ChangePlayerState(EPlayerState::Placing);
          }
     }
     else
     {
          Server_ChangePlayerState(EPlayerState::Default);
     }
}

bool ARtsPlayerController::Server_CreateUnitBuilding_Validate() 
{
     return true;
}

void ARtsPlayerController::PrepareUnit_Implementation(AActor* NewUnit) 
{
     PlacementBuffer = NewUnit;
     if(NewUnit != nullptr && Cast<ARTSPrototypeCharacter>(NewUnit) && PlacementBuffer == NewUnit)
     {
          UE_LOG(LogTemp, Warning, TEXT("New Unit is good %s"), NETMODE_WORLD);
          
          // Makes Tick call PositionPlacement()
          Server_ChangePlayerState(EPlayerState::Placing);
          const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EPlayerState"), true);
          GEngine->AddOnScreenDebugMessage(0, 2, FColor::Red, *EnumPtr->GetDisplayNameText((uint8)RTSPlayerState).ToString());
     }
     else
     {
          UE_LOG(LogTemp, Warning, TEXT("New Unit is bad"));
     }
     GEngine->AddOnScreenDebugMessage(0, 2, FColor::Red, TEXT("PrepareUnit Called "));
     
}

// Called when U is pressed
void ARtsPlayerController::Server_CreateUnit_Implementation() 
{
     if(RTSPlayerState == EPlayerState::Menu) return;

     AActor* NewUnit = GetWorld()->SpawnActor<ARTSPrototypeCharacter>(UnitClass, FVector(-3028.0f, -276.0f, 277.56f), FRotator(0.0f));
     if(NewUnit)
     {
          Cast<ARTSPrototypeCharacter>(NewUnit)->SetOwnerUserName(UserName);
          NewUnit->SetReplicates(true);
          PlayerPawn->MyUnits.Add(NewUnit);
          FString UnitName = GetDebugName(NewUnit);
          UE_LOG(LogTemp, Warning, TEXT("Unit Buffer in CreateUnit: %s   %s"), *UnitName, NETMODE_WORLD);
          PrepareUnit_Implementation(NewUnit);
     }
}

bool ARtsPlayerController::Server_CreateUnit_Validate()
{
     return true;
}

void ARtsPlayerController::Server_PositionPlacement_Implementation(FHitResult HitRes, AActor* UnitToPlace) 
{
     /* if(UnitToPlace)
     {
          UE_LOG(LogTemp, Warning, TEXT("UnitToPlace GOOD PositionPlacement() "), NETMODE_WORLD);
     }
     else
     {
          UE_LOG(LogTemp, Warning, TEXT("UnitToPlace BAD PositionPlacement() "), NETMODE_WORLD);
          return;
     } */

     // Placement Trace Channel
     // GetHitResultUnderCursor(ECC_GameTraceChannel2, false, Hit);
     if(Cast<ARTSPrototypeCharacter>(UnitToPlace) != nullptr)
     {
          FString UnitName = GetDebugName(UnitToPlace);
          // UE_LOG(LogTemp, Warning, TEXT("Unit Buffer in PositionPlacement: %s  %s"), *UnitName, NETMODE_WORLD);
          HitRes.Location.Z += 100.f;
     }
     else
     {
          // UE_LOG(LogTemp, Warning, TEXT("Cast Failed in Position Placement"));
     }
     

	if (HitRes.bBlockingHit)
	{
          UnitToPlace->SetActorLocation(HitRes.Location);
     }
     else
     {
          UE_LOG(LogTemp, Warning, TEXT("Nothing Blocking Hit"));
     }
     

}

bool ARtsPlayerController::Server_PositionPlacement_Validate(FHitResult HitRes, AActor* UnitToPlace)
{
     return true;
}

void ARtsPlayerController::Server_ChangePlayerState_Implementation(EPlayerState NewState) 
{
     RTSPlayerState = NewState;
     const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EPlayerState"), true);
     FString Message = FString("Changed State to: ").Append(*EnumPtr->GetDisplayNameText((uint8)RTSPlayerState).ToString());
     GEngine->AddOnScreenDebugMessage(2, 2, FColor::Red, Message);
     FString EnumName = EnumPtr->GetDisplayNameText((uint8)RTSPlayerState).ToString();
     UE_LOG(LogTemp, Warning, TEXT("Changed State to: %s    %s"), *EnumName, NETMODE_WORLD);
}

bool ARtsPlayerController::Server_ChangePlayerState_Validate(EPlayerState NewState)
{
     return true;
}