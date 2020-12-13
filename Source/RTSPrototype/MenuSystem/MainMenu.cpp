// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/CanvasPanel.h"
#include "Components/VerticalBox.h"
#include "Components/EditableTextBox.h"

bool UMainMenu::Initialize() 
{
     bool Success = Super::Initialize();
     if(!Success) return false;

     if(!ensure(HostButton)) return false;
     HostButton->OnClicked.AddDynamic(this, &UMainMenu::HostServer);

     if(!ensure(JoinButton)) return false;
     JoinButton->OnClicked.AddDynamic(this, &UMainMenu::OpenJoinMenu);

     if(!ensure(JoinIPButton)) return false;
     JoinIPButton->OnClicked.AddDynamic(this, &UMainMenu::JoinIP);

     if(!ensure(BackButton)) return false;
     BackButton->OnClicked.AddDynamic(this, &UMainMenu::OpenMainMenu);

     if(!ensure(QuitButton)) return false;
     QuitButton->OnClicked.AddDynamic(this, &UMainMenu::Quit);

     return true;
}

void UMainMenu::HostServer() 
{
     if(MenuInterface)
     {
          MenuInterface->Host();
     }
}

void UMainMenu::JoinIP() 
{
     if(!ensure(IPAddressField)) return;
     if(MenuInterface)
     {
          const FString& Address = IPAddressField->GetText().ToString();
          MenuInterface->Join(Address);
     }
}

void UMainMenu::OpenJoinMenu() 
{
     if(!ensure(MenuSwitcher)) return;
     if(!ensure(JoinMenu)) return;
     MenuSwitcher->SetActiveWidget(JoinMenu);
}

void UMainMenu::OpenMainMenu() 
{
     if(!ensure(MenuSwitcher)) return;
     if(!ensure(MainMenu)) return;
     MenuSwitcher->SetActiveWidget(MainMenu);
}

void UMainMenu::Quit() 
{
     UWorld* World = GetWorld();
     if(ensure(!World)) return;

     APlayerController* PlayerController = World->GetFirstPlayerController();
     if(!ensure(PlayerController)) return;

     PlayerController->ConsoleCommand("Quit", false);
}

void UMainMenu::SetMenuInterface(IMenuInterface* MenuInterfaceBase) 
{
     MenuInterface = MenuInterfaceBase;
}

void UMainMenu::Setup() 
{
     this->AddToViewport();

     UWorld* World = GetWorld();
     if(!ensure(World)) return;

     APlayerController* PlayerController = World->GetFirstPlayerController();
     if(!ensure(PlayerController)) return;

     this->bIsFocusable = true;
     FInputModeUIOnly InputModeData;
     InputModeData.SetWidgetToFocus(this->TakeWidget());
     InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

     PlayerController->SetInputMode(InputModeData);

     PlayerController->bShowMouseCursor = true;
}

void UMainMenu::Teardown() 
{
     this->RemoveFromViewport();

     UWorld* World = GetWorld();
     if(!ensure(World)) return;

     APlayerController* PlayerController = World->GetFirstPlayerController();
     if(!ensure(PlayerController)) return;

     FInputModeGameOnly InputModeData;
     PlayerController->SetInputMode(InputModeData);

     PlayerController->bShowMouseCursor = false;
}
