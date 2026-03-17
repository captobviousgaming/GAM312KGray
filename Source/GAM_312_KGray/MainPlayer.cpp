#include "MainPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h" 
#include "InteractableItem.h"
#include "BuildablePiece.h"
#include "Blueprint/UserWidget.h"
#include "Math/UnrealMathUtility.h" 

// Sets default values
AMainPlayer::AMainPlayer()
{
	// I enable Tick here so the building ghost preview can follow the camera smoothly.
	PrimaryActorTick.bCanEverTick = true;

	// I initialize the camera and attach it to the capsule component.
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// I initialize my default stats and inventory values.
	Health = 100.0f;
	Hunger = 100.0f;
	Stamina = 100.0f;
	Wood = 0;
	Stone = 0;
	Berry = 0;

	// Menu and building variable initialization
	bIsBuildMenuOpen = false;
	bIsInventoryOpen = false;
	PreviewPiece = nullptr;
	BuildRotationYaw = 0.0f;
	CurrentVariationIndex = 0;
}

// Called when the game starts or when spawned
void AMainPlayer::BeginPlay()
{
	Super::BeginPlay();

	// I start the looping timer to handle stat depletion and restoration over time.
	GetWorldTimerManager().SetTimer(StatTimerHandle, this, &AMainPlayer::HandleStatsOverTime, 1.0f, true);

	// Spawn the main HUD (Stats) and add it to the screen.
	if (StatsWidgetClass)
	{
		StatsWidget = CreateWidget<UUserWidget>(GetWorld(), StatsWidgetClass);
		if (StatsWidget) { StatsWidget->AddToViewport(); }
	}

	// Create the Build Menu widget in the background.
	if (BuildMenuClass)
	{
		BuildMenuWidget = CreateWidget<UUserWidget>(GetWorld(), BuildMenuClass);
	}

	// Create my custom Bag UI in the background so it is ready when the player presses 'I'.
	if (InventoryMenuClass)
	{
		InventoryMenuWidget = CreateWidget<UUserWidget>(GetWorld(), InventoryMenuClass);
	}
}

// Called every frame
void AMainPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// If I currently have a building preview active, I update its location.
	if (PreviewPiece) { UpdateBuildingPreview(); }
}

// Called to bind functionality to input keys
void AMainPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Movement bindings
	PlayerInputComponent->BindAxis("MoveForward", this, &AMainPlayer::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainPlayer::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	// Interaction and Menu bindings
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMainPlayer::Interact);
	PlayerInputComponent->BindAction("ToggleBuildMenu", IE_Pressed, this, &AMainPlayer::ToggleBuildMenu);

	// Binding for my new Dynamic Inventory UI toggle
	PlayerInputComponent->BindAction("ToggleInventoryMenu", IE_Pressed, this, &AMainPlayer::ToggleInventoryMenu);

	// Building bindings
	PlayerInputComponent->BindAction("PlaceBuilding", IE_Pressed, this, &AMainPlayer::PlaceBuilding);
	PlayerInputComponent->BindAction("RotateBuilding", IE_Pressed, this, &AMainPlayer::RotateBuilding);
	PlayerInputComponent->BindAction("CycleVariation", IE_Pressed, this, &AMainPlayer::CycleBuildingVariation);
}

void AMainPlayer::MoveForward(float Value) { if (Value != 0.0f) { AddMovementInput(GetActorForwardVector(), Value); } }
void AMainPlayer::MoveRight(float Value) { if (Value != 0.0f) { AddMovementInput(GetActorRightVector(), Value); } }

// Timer function handling continuous stat changes
void AMainPlayer::HandleStatsOverTime()
{
	// Decrease hunger first; if empty, decrease health. Restore stamina passively up to 100.
	if (Hunger > 0.0f) { Hunger -= 1.0f; }
	else { Health -= 2.0f; }
	if (Stamina < 100.0f) { Stamina += 1.0f; }
}

// Line trace function for harvesting resources
void AMainPlayer::Interact()
{
	if (!FirstPersonCameraComponent) return;

	// Calculate a trace starting from the camera and moving 500 units directly forward.
	FVector StartLocation = FirstPersonCameraComponent->GetComponentLocation();
	FVector ForwardVector = FirstPersonCameraComponent->GetForwardVector();
	FVector EndLocation = StartLocation + (ForwardVector * 500.0f);

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	// Execute the trace
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, CollisionParams);

	if (bHit)
	{
		// Drain stamina upon a successful interaction swing.
		if (Stamina > 0.0f) { Stamina -= 5.0f; }

		// Check if the hit object is one of my interactable resource nodes.
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			AInteractableItem* HitItem = Cast<AInteractableItem>(HitActor);
			if (HitItem)
			{
				EResourceType GatheredType;
				// Attempt to gather; if successful, determine the type and update arrays.
				if (HitItem->GatherResource(GatheredType))
				{
					// Update my raw integers so the existing building logic doesn't break.
					switch (GatheredType)
					{
					case EResourceType::Wood: Wood++; break;
					case EResourceType::Stone: Stone++; break;
					case EResourceType::Berry: Berry++; break;
					}

					// Trigger my visual resource popup.
					ShowResourcePopup(GatheredType, 1);

					// Pass the gathered item into my dynamic inventory array for the Bag UI.
					AddResourceToInventory(GatheredType, 1);
				}
			}
		}
	}
}

// --- ARRAY SORTING LOGIC ---
// I created this to handle the math of stacking items before passing them to the UI.
void AMainPlayer::AddResourceToInventory(EResourceType GatheredType, int32 Amount)
{
	// First, I loop through my existing inventory array to see if a stack of this item type already exists.
	for (int32 i = 0; i < InventoryArray.Num(); i++)
	{
		if (InventoryArray[i].ItemType == GatheredType)
		{
			// I found a matching stack, so I simply add the gathered amount to its quantity and exit the function.
			InventoryArray[i].Quantity += Amount;
			return;
		}
	}

	// If the loop finishes without finding a match, I know this is a completely new item.
	// I add a new FInventoryItem struct to the end of the array, which will generate a new slot in the UI.
	InventoryArray.Add(FInventoryItem(GatheredType, Amount));
}

// --- TOGGLE BAG UI ---
// Handles opening my custom Bag UI, returning mouse control, and refreshing the slots.
void AMainPlayer::ToggleInventoryMenu()
{
	// Ensure we actually selected the UI blueprint in the editor
	if (!InventoryMenuClass) return;

	APlayerController* PC = Cast<APlayerController>(GetController());

	if (bIsInventoryOpen)
	{
		// If it's open, remove it from the screen and clear the variable to destroy it
		if (InventoryMenuWidget)
		{
			InventoryMenuWidget->RemoveFromParent();
			InventoryMenuWidget = nullptr;
		}

		bIsInventoryOpen = false;

		// Hide the mouse and return input to game controls
		if (PC)
		{
			PC->SetShowMouseCursor(false);
			PC->SetInputMode(FInputModeGameOnly());
		}
	}
	else
	{
		// REBUILD the widget fresh every time we open the bag! 
		// This forces "Event Construct" in the blueprint to run and populate our array loop.
		InventoryMenuWidget = CreateWidget<UUserWidget>(GetWorld(), InventoryMenuClass);
		if (InventoryMenuWidget)
		{
			InventoryMenuWidget->AddToViewport();
		}

		bIsInventoryOpen = true;

		// Show the mouse so the player can interact with their bag slots
		if (PC)
		{
			PC->SetShowMouseCursor(true);
			PC->SetInputMode(FInputModeGameAndUI());
		}
	}
}

// Toggles my building menu
void AMainPlayer::ToggleBuildMenu()
{
	if (!BuildMenuWidget) return;

	if (bIsBuildMenuOpen)
	{
		BuildMenuWidget->RemoveFromParent();
		bIsBuildMenuOpen = false;
	}
	else
	{
		BuildMenuWidget->AddToViewport();
		bIsBuildMenuOpen = true;
	}
}

// --- BUILDING SYSTEM LOGIC ---

void AMainPlayer::StartBuilding(TArray<TSubclassOf<AActor>> Variations)
{
	if (Variations.Num() == 0) return;

	CurrentBuildingVariations = Variations;
	CurrentVariationIndex = 0;
	BuildRotationYaw = 0.0f;

	// I destroy any existing preview ghost to prevent duplicate meshes.
	if (PreviewPiece) { PreviewPiece->Destroy(); }

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// I spawn the preview ghost and cast it to ABuildablePiece
	PreviewPiece = Cast<ABuildablePiece>(GetWorld()->SpawnActor<AActor>(CurrentBuildingVariations[CurrentVariationIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams));

	// I disable collision on the ghost so it doesn't block the player's movement or trace.
	if (PreviewPiece) { PreviewPiece->SetActorEnableCollision(false); }

	ToggleBuildMenu();
}

void AMainPlayer::RotateBuilding()
{
	// I add 90 degrees to snap the rotation to clean cardinal directions.
	BuildRotationYaw += 90.0f;
	if (BuildRotationYaw >= 360.0f) { BuildRotationYaw = 0.0f; }
}

void AMainPlayer::CycleBuildingVariation()
{
	if (CurrentBuildingVariations.Num() <= 1 || !PreviewPiece) return;

	// I increment the variation index, wrapping back to 0 if I hit the end of the array.
	CurrentVariationIndex++;
	if (CurrentVariationIndex >= CurrentBuildingVariations.Num()) { CurrentVariationIndex = 0; }

	FVector CurrentLoc = PreviewPiece->GetActorLocation();
	PreviewPiece->Destroy();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// I respawn the new variation at the exact location of the old one.
	PreviewPiece = Cast<ABuildablePiece>(GetWorld()->SpawnActor<AActor>(CurrentBuildingVariations[CurrentVariationIndex], CurrentLoc, FRotator(0.0f, BuildRotationYaw, 0.0f), SpawnParams));

	if (PreviewPiece) { PreviewPiece->SetActorEnableCollision(false); }
}

void AMainPlayer::UpdateBuildingPreview()
{
	if (!PreviewPiece || !FirstPersonCameraComponent) return;

	FVector StartLocation = FirstPersonCameraComponent->GetComponentLocation();
	FVector ForwardVector = FirstPersonCameraComponent->GetForwardVector();
	FVector EndLocation = StartLocation + (ForwardVector * 800.0f);

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	CollisionParams.AddIgnoredActor(PreviewPiece); // I make sure the trace ignores the ghost itself

	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, CollisionParams))
	{
		// I define my grid size to match the dimensions of my custom 3D models.
		float GridSize = 200.0f;

		// I calculate the snapped location by dividing, rounding, and multiplying against the grid size.
		FVector SnappedLocation;
		SnappedLocation.X = FMath::RoundToFloat(HitResult.Location.X / GridSize) * GridSize;
		SnappedLocation.Y = FMath::RoundToFloat(HitResult.Location.Y / GridSize) * GridSize;
		SnappedLocation.Z = FMath::RoundToFloat(HitResult.Location.Z / GridSize) * GridSize;

		// I apply the calculated location and rotation to the preview piece.
		PreviewPiece->SetActorLocationAndRotation(SnappedLocation, FRotator(0.0f, BuildRotationYaw, 0.0f));
	}
}

void AMainPlayer::PlaceBuilding()
{
	if (!PreviewPiece || CurrentBuildingVariations.Num() == 0) return;

	// I verify that the player has enough raw resources to afford the piece.
	if (Wood >= PreviewPiece->GetWoodCost() && Stone >= PreviewPiece->GetStoneCost())
	{
		// I deduct the resources.
		Wood -= PreviewPiece->GetWoodCost();
		Stone -= PreviewPiece->GetStoneCost();

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// I spawn the physical, solid AActor precisely where the preview ghost was located.
		GetWorld()->SpawnActor<AActor>(CurrentBuildingVariations[CurrentVariationIndex], PreviewPiece->GetActorLocation(), PreviewPiece->GetActorRotation(), SpawnParams);

		UE_LOG(LogTemp, Warning, TEXT("Successfully Built! Remaining Wood: %d, Stone: %d"), Wood, Stone);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Not enough resources to build this!"));
	}
}