// Fill out your copyright notice in the Description page of Project Settings.


#include "GameHUD.h"
#include "RTSPrototype/RtsPlayerController.h"
#include "RTSPrototypeCharacter.h"
#include "Components/DecalComponent.h"
#include "Building.h"

void AGameHUD::DrawHUD()
{
     if (SelectPressed == true)
     {
          for(ARTSPrototypeCharacter* Unit : Cast<ARtsPlayerController>(GetOwningPlayerController())->SelectedUnits)
          {
               Unit->CursorToWorld->SetVisibility(false);
          }
          for(ABuilding* Building : Cast<ARtsPlayerController>(GetOwningPlayerController())->SelectedBuildings)
          {
               Building->CursorToWorld->SetVisibility(false);
          }
          Cast<ARtsPlayerController>(GetOwningPlayerController())->SelectedUnits.Empty();
          Cast<ARtsPlayerController>(GetOwningPlayerController())->SelectedBuildings.Empty();
          GetOwningPlayerController()->GetMousePosition(CurrentMousePos.X, CurrentMousePos.Y);
          DrawRect(FLinearColor(0,0,1, .15f), InitMousePos.X, InitMousePos.Y, CurrentMousePos.X - InitMousePos.X, CurrentMousePos.Y - InitMousePos.Y);
          GetActorsInSelectionRectangle(InitMousePos, CurrentMousePos, Cast<ARtsPlayerController>(GetOwningPlayerController())->SelectedUnits, false);
          GetActorsInSelectionRectangle(InitMousePos, CurrentMousePos, Cast<ARtsPlayerController>(GetOwningPlayerController())->SelectedBuildings, false);
          for(ARTSPrototypeCharacter* Unit : Cast<ARtsPlayerController>(GetOwningPlayerController())->SelectedUnits)
          {
               Unit->CursorToWorld->SetVisibility(true);
          }
          for(ABuilding* Building : Cast<ARtsPlayerController>(GetOwningPlayerController())->SelectedBuildings)
          {
               Building->CursorToWorld->SetVisibility(true);
          }
     }
}