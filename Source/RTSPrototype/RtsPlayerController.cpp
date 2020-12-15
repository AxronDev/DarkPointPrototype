// Fill out your copyright notice in the Description page of Project Settings.


#include "RtsPlayerController.h"
#include "RTSPrototypeCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "CameraPawn.h"
#include "Components/BoxComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "RTSPrototype/GameHUD.h"
#include "Building.h"

ARtsPlayerController::ARtsPlayerController()
{
     bShowMouseCursor = true;
     DefaultMouseCursor = EMouseCursor::Default;
     UGameplayStatics::SetPlayerControllerID(this, 1);
}

FName ARtsPlayerController::GetUserName() 
{
     return UserName;
}

void ARtsPlayerController::SetUsername(const FName& NewUserName) 
{
     UserName = NewUserName;
}

void ARtsPlayerController::PlayerTick(float DeltaTime) 
{
     Super::PlayerTick(DeltaTime);

     if(PlayerState != EPlayerState::Default)
     {
          // Update building location
          PositionPlacement();
     }
     if(IsPressLeft == true)
     {
          if(PlayerPawn->Units >= 1)
          {
               if(Cast<ARTSPrototypeCharacter>(PlacementBuffer)->bHasSpace == true)
               {
                    PlayerPawn->Units -= 1;
                    ChangeState(EPlayerState::Default);
                    CreateUnit();
               }
          }
     }
}

void ARtsPlayerController::SetAggression() 
{
     bAggressive = !bAggressive;
}

void ARtsPlayerController::BeginPlay()
{
     HUD = Cast<AGameHUD>(GetHUD());

     BuildingSpawnParams.Owner = Cast<AActor>(GetPawn());

     PlayerState = EPlayerState::Default;

     PlayerPawn = Cast<ACameraPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
}

void ARtsPlayerController::SetupInputComponent()
{
     Super::SetupInputComponent();
     InputComponent->BindAction("LeftMouseClick", IE_Pressed, this, &ARtsPlayerController::LeftMousePress);
     InputComponent->BindAction("LeftMouseClick", IE_Released, this, &ARtsPlayerController::LeftMouseRelease);

     InputComponent->BindAction("RightMouseClick", IE_Released, this, &ARtsPlayerController::RightMousePress);

     InputComponent->BindAction("BuildGoldProduction", IE_Released, this, &ARtsPlayerController::CreateGoldBuilding);
     InputComponent->BindAction("BuildUnitProduction", IE_Released, this, &ARtsPlayerController::CreateUnitBuilding);

     InputComponent->BindAction("PlaceUnit", IE_Released, this, &ARtsPlayerController::CreateUnit);
     InputComponent->BindAction("AttackMovement", IE_Released, this, &ARtsPlayerController::SetAggression);
}

void ARtsPlayerController::MoveTo()
{
     // Trace to see what is under the mouse cursor
		Hit;
		GetHitResultUnderCursor(ECC_Visibility, false, Hit);

		if (Hit.bBlockingHit)
		{
			// We hit something, cycle through selected units and move there
               for(ARTSPrototypeCharacter* Unit : SelectedUnits)
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
                    UAIBlueprintHelperLibrary::SimpleMoveToLocation(Unit->GetController(), Hit.ImpactPoint);
               }
               // reset to passive
               bAggressive = false;
		}
}

void ARtsPlayerController::LeftMousePress() 
{
     if(PlayerState == EPlayerState::Default)
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
               ChangeState(EPlayerState::Default);
          }
          
     }
     
}

void ARtsPlayerController::LeftMouseRelease() 
{
     SelectionTerminate();
     IsPressLeft = false;
}

void ARtsPlayerController::RightMousePress() 
{
     if(PlayerState == EPlayerState::Default)
     {
          MoveTo();
     }
     else
     {
          PlacementBuffer->Destroy();
          ChangeState(EPlayerState::Default);
          
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

void ARtsPlayerController::CreateGoldBuilding() 
{
     if(PlayerPawn->Gold >= PlayerPawn->GoldPrice)
     {
          PlacementBuffer = GetWorld()->SpawnActor<ABuilding>(GoldBuildingClass);
          if(PlacementBuffer)
          {
               Cast<ABuilding>(PlacementBuffer)->SetOwnerUserName(UserName);
               PlayerPawn->AddGoldBuilding();
               // Makes Tick call PositionPlacement()
               ChangeState(EPlayerState::Placing);
          }
     }
     else
     {
          ChangeState(EPlayerState::Default);
     }
          
}

void ARtsPlayerController::CreateUnitBuilding() 
{
     if(PlayerPawn->Gold >= PlayerPawn->UnitPrice)
     {
          PlacementBuffer = GetWorld()->SpawnActor<ABuilding>(UnitBuildingClass);
          if(PlacementBuffer)
          {
               Cast<ABuilding>(PlacementBuffer)->SetOwnerUserName(UserName);
               PlayerPawn->AddUnitBuilding();
               // Makes Tick call PositionPlacement()
               ChangeState(EPlayerState::Placing);
          }
     }
     else
     {
          ChangeState(EPlayerState::Default);
     }
}

void ARtsPlayerController::CreateUnit() 
{
          PlacementBuffer = GetWorld()->SpawnActor<ARTSPrototypeCharacter>(UnitClass);
          if(PlacementBuffer)
          {
               Cast<ARTSPrototypeCharacter>(PlacementBuffer)->SetOwnerUserName(UserName);
               PlayerPawn->MyUnits.Add(PlacementBuffer);
               // Makes Tick call PositionPlacement()
               ChangeState(EPlayerState::Placing);
          }
}

void ARtsPlayerController::PositionPlacement() 
{
     GetHitResultUnderCursor(ECC_GameTraceChannel2, false, Hit);
     if(Cast<ARTSPrototypeCharacter>(PlacementBuffer))
     {
          Hit.Location.Z += 100.f;
     }

	if (Hit.bBlockingHit)
	{
          PlacementBuffer->SetActorLocation(Hit.Location);
     }

}

void ARtsPlayerController::ChangeState(EPlayerState NewState) 
{
     PlayerState = NewState;
}