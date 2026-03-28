#include "MainPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h" 
#include "TimerManager.h"
#include "DrawDebugHelpers.h" 
#include "InteractableItem.h"
#include "BuildablePiece.h"
#include "Blueprint/UserWidget.h"
#include "Math/UnrealMathUtility.h" 
#include "Materials/MaterialInterface.h"

AMainPlayer::AMainPlayer()
{
	// [Week 1] Enabling tick so we can update the building preview every frame.
	PrimaryActorTick.bCanEverTick = true;

	// [Week 1] Setting up the First Person Camera so the player can look around.
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// [Week 2] Initializing default player stats.
	// [Week 4] Setting the base values that will fill our HUD progress bars to 100% on start.
	Health = 100.0f; Hunger = 100.0f; Stamina = 100.0f;
	Wood = 1000; Stone = 1000; Berry = 0;

	// Setup starting booleans and states for the building system.
	bIsBuildMenuOpen = false;
	bIsInventoryOpen = false;
	PreviewPiece = nullptr;
	BuildRotationYaw = 0.0f;
	CurrentVariationIndex = 0;
	EquippedPieceIndex = 0;

	// [Week 3] Our default grid snap size. Can be overridden in Blueprints!
	SnapGridSize = 200.0f;
	bCanPlace = false;
}

void AMainPlayer::BeginPlay()
{
	Super::BeginPlay();

	// [Week 2] Start the timer that drains hunger/health.
	GetWorldTimerManager().SetTimer(StatTimerHandle, this, &AMainPlayer::HandleStatsOverTime, 1.0f, true);

	// [Week 3] I give the player 5 of each pre-made building item right at the start to test the shelter system!
	AddResourceToInventory(EResourceType::Wall, 5);
	AddResourceToInventory(EResourceType::Floor, 5);
	AddResourceToInventory(EResourceType::Roof, 5);

	// [Week 3] Create the stats widget and add it to the viewport immediately.
	// [Week 4] This is where the Player Stat HUD is actively spawned onto the player's screen to display the progress bars.
	if (StatsWidgetClass)
	{
		StatsWidget = CreateWidget<UUserWidget>(GetWorld(), StatsWidgetClass);
		if (StatsWidget) { StatsWidget->AddToViewport(); }
	}
}

void AMainPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// [Week 3] If we have the build menu open and are holding a piece, update its position every frame based on camera view.
	if (bIsBuildMenuOpen && PreviewPiece) { UpdateBuildingPreview(); }
}

void AMainPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// [Week 1] Binding standard WASD and mouse look inputs.
	PlayerInputComponent->BindAxis("MoveForward", this, &AMainPlayer::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainPlayer::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	// [Week 2 & 3] Binding interaction and UI toggles.
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMainPlayer::Interact);
	PlayerInputComponent->BindAction("ToggleInventoryMenu", IE_Pressed, this, &AMainPlayer::ToggleInventoryMenu);

	// [Week 3] Building system hotkeys (1, 2, 3 for pieces, Left Click to place, R to rotate).
	PlayerInputComponent->BindAction("ToggleBuildMenu", IE_Pressed, this, &AMainPlayer::ToggleBuildMenu);
	PlayerInputComponent->BindAction("SelectWall", IE_Pressed, this, &AMainPlayer::SelectWall);
	PlayerInputComponent->BindAction("SelectFloor", IE_Pressed, this, &AMainPlayer::SelectFloor);
	PlayerInputComponent->BindAction("SelectRoof", IE_Pressed, this, &AMainPlayer::SelectRoof);
	PlayerInputComponent->BindAction("PlaceBuilding", IE_Pressed, this, &AMainPlayer::PlaceBuilding);
	PlayerInputComponent->BindAction("RotateBuilding", IE_Pressed, this, &AMainPlayer::RotateBuilding);
	PlayerInputComponent->BindAction("CycleVariation", IE_Pressed, this, &AMainPlayer::CycleBuildingVariation);
}

void AMainPlayer::MoveForward(float Value)
{
	if (Value != 0.0f) { AddMovementInput(GetActorForwardVector(), Value); } // [Week 1]
}

void AMainPlayer::MoveRight(float Value)
{
	if (Value != 0.0f) { AddMovementInput(GetActorRightVector(), Value); } // [Week 1]
}

void AMainPlayer::HandleStatsOverTime()
{
	// [Week 2] Drop hunger first. If hunger is empty, start dropping health!
	// [Week 4] As this timer ticks down the values, our Player Stat HUD reads the changes and visually drains the progress bars.
	if (Hunger > 0.0f) { Hunger -= 1.0f; }
	else { Health -= 2.0f; }
}

void AMainPlayer::Interact()
{
	if (!FirstPersonCameraComponent) return;

	// [Week 2] Calculate the start and end points for the line trace (500 units forward from the camera).
	FVector Start = FirstPersonCameraComponent->GetComponentLocation();
	FVector End = Start + (FirstPersonCameraComponent->GetForwardVector() * 500.0f);
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this); // Don't hit ourselves!

	// Fire the trace!
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		// Did we hit a resource node?
		AInteractableItem* Item = Cast<AInteractableItem>(Hit.GetActor());
		if (Item)
		{
			EResourceType Type;
			// Attempt to gather the resource from it.
			if (Item->GatherResource(Type))
			{
				if (Type == EResourceType::Wood) Wood++;
				else if (Type == EResourceType::Stone) Stone++;

				ShowResourcePopup(Type, 1); // Trigger UI feedback.
				AddResourceToInventory(Type, 1); // Store in the backpack array.
			}
		}
	}
}

// [Extra Polish] Dynamic inventory management. 
void AMainPlayer::AddResourceToInventory(EResourceType Type, int32 Amount)
{
	// Check if we already have a stack of this item, and just add to it.
	for (auto& Item : InventoryArray)
	{
		if (Item.ItemType == Type) { Item.Quantity += Amount; return; }
	}
	// Otherwise, create a new slot for it!
	InventoryArray.Add(FInventoryItem(Type, Amount));
}

// [Week 3] Checks the dynamic inventory to ensure the player has the required wall/floor/roof piece to build.
bool AMainPlayer::HasBuildingItem(EPieceType Type)
{
	EResourceType TargetRes;
	if (Type == EPieceType::Wall) TargetRes = EResourceType::Wall;
	else if (Type == EPieceType::Floor) TargetRes = EResourceType::Floor;
	else if (Type == EPieceType::Roof) TargetRes = EResourceType::Roof;
	else return false;

	for (const FInventoryItem& Item : InventoryArray)
	{
		if (Item.ItemType == TargetRes && Item.Quantity > 0)
		{
			return true;
		}
	}
	return false;
}

// [Week 3] Removes the used piece from the inventory array upon successful placement.
void AMainPlayer::ConsumeBuildingItem(EPieceType Type)
{
	EResourceType TargetRes;
	if (Type == EPieceType::Wall) TargetRes = EResourceType::Wall;
	else if (Type == EPieceType::Floor) TargetRes = EResourceType::Floor;
	else if (Type == EPieceType::Roof) TargetRes = EResourceType::Roof;
	else return;

	for (FInventoryItem& Item : InventoryArray)
	{
		if (Item.ItemType == TargetRes && Item.Quantity > 0)
		{
			Item.Quantity--;
			return;
		}
	}
}

// [Extra Polish] UI Toggle
void AMainPlayer::ToggleInventoryMenu()
{
	if (!InventoryMenuClass) return;
	if (bIsInventoryOpen)
	{
		if (InventoryMenuWidget) { InventoryMenuWidget->RemoveFromParent(); InventoryMenuWidget = nullptr; }
		bIsInventoryOpen = false;
	}
	else
	{
		InventoryMenuWidget = CreateWidget<UUserWidget>(GetWorld(), InventoryMenuClass);
		if (InventoryMenuWidget) { InventoryMenuWidget->AddToViewport(); }
		bIsInventoryOpen = true;
	}
}

// [Week 3] Opens/Closes the Build HUD and cleans up the preview piece if closed.
void AMainPlayer::ToggleBuildMenu()
{
	if (bIsBuildMenuOpen)
	{
		if (BuildMenuWidget) { BuildMenuWidget->RemoveFromParent(); }
		bIsBuildMenuOpen = false;
		if (PreviewPiece) { PreviewPiece->Destroy(); PreviewPiece = nullptr; }
		EquippedPieceIndex = 0;
		UpdateBuildUI(0, false); // Tells the UI to revert to the N/A icon.
	}
	else if (BuildMenuClass)
	{
		BuildMenuWidget = CreateWidget<UUserWidget>(GetWorld(), BuildMenuClass);
		if (BuildMenuWidget) { BuildMenuWidget->AddToViewport(); }
		bIsBuildMenuOpen = true;
	}
}

void AMainPlayer::SelectWall() { if (bIsBuildMenuOpen) { EquippedPieceIndex = 1; StartBuilding(WallVariations); } }
void AMainPlayer::SelectFloor() { if (bIsBuildMenuOpen) { EquippedPieceIndex = 2; StartBuilding(FloorVariations); } }
void AMainPlayer::SelectRoof() { if (bIsBuildMenuOpen) { EquippedPieceIndex = 3; StartBuilding(RoofVariations); } }

// [Week 3] Spawns the transparent preview piece and sets it up.
void AMainPlayer::StartBuilding(TArray<TSubclassOf<ABuildablePiece>> Variations)
{
	if (Variations.Num() == 0) return;
	CurrentBuildingVariations = Variations;
	CurrentVariationIndex = 0;
	if (PreviewPiece) { PreviewPiece->Destroy(); } // Clear the old piece if switching types

	// Spawn the invisible template
	PreviewPiece = GetWorld()->SpawnActor<ABuildablePiece>(CurrentBuildingVariations[0], FVector::ZeroVector, FRotator::ZeroRotator);
	if (PreviewPiece)
	{
		PreviewPiece->SetActorEnableCollision(false); // Make sure it doesn't block the player while moving it!

		// [Extra Polish] Bulletproof logic to find ALL mesh components on the blueprint and apply the translucent Ghost Material to them.
		if (GhostMaterial)
		{
			TArray<UStaticMeshComponent*> MeshComps;
			PreviewPiece->GetComponents<UStaticMeshComponent>(MeshComps);
			for (UStaticMeshComponent* MeshComp : MeshComps)
			{
				for (int32 i = 0; i < MeshComp->GetNumMaterials(); i++)
				{
					MeshComp->SetMaterial(i, GhostMaterial);
				}
			}
		}

		// Check inventory to see if we should even show the preview, and update the HUD.
		bool bHasMats = HasBuildingItem(PreviewPiece->GetPieceType());
		PreviewPiece->SetActorHiddenInGame(!bHasMats);
		UpdateBuildUI(EquippedPieceIndex, bHasMats);
	}
}

// [Week 3] The core math for snapping pieces to the world grid based on the camera trace.
void AMainPlayer::UpdateBuildingPreview()
{
	if (!PreviewPiece || !FirstPersonCameraComponent) return;

	// Check inventory in real-time in case we ran out on the last click.
	bool bHasMats = HasBuildingItem(PreviewPiece->GetPieceType());
	PreviewPiece->SetActorHiddenInGame(!bHasMats);

	// Cast a line trace 800 units forward to find the placement spot.
	FVector Start = FirstPersonCameraComponent->GetComponentLocation();
	FVector End = Start + (FirstPersonCameraComponent->GetForwardVector() * 800.0f);
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(PreviewPiece);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		// [Extra Polish] Push the target outward along the surface normal (the face of the wall) before calculating the grid snap.
		// This forces roofs and walls to snap to the outer edge rather than getting stuck centered inside thin objects!
		FVector TargetLocation = Hit.Location + (Hit.ImpactNormal * (SnapGridSize * 0.4f));

		// [Extra Polish] Half-Grid Snapping. This allows walls to snap to the center OR the edge of a grid space, allowing corner building!
		float HalfGrid = SnapGridSize / 2.0f;
		FVector Snapped(
			FMath::RoundToFloat(TargetLocation.X / HalfGrid) * HalfGrid,
			FMath::RoundToFloat(TargetLocation.Y / HalfGrid) * HalfGrid,
			FMath::RoundToFloat(TargetLocation.Z / SnapGridSize) * SnapGridSize
		);

		bCanPlace = true;
		ABuildablePiece* HitPiece = Cast<ABuildablePiece>(Hit.GetActor());

		// [Week 3] Prevent placing exact duplicates on top of each other by checking proximity.
		for (TActorIterator<ABuildablePiece> It(GetWorld()); It; ++It)
		{
			if (*It != PreviewPiece && FVector::Dist(It->GetActorLocation(), Snapped) < 10.0f)
			{
				bCanPlace = false;
				break;
			}
		}

		// [Week 3] Specific Snapping Rules: Roofs can ONLY be placed on top of Walls.
		if (PreviewPiece->GetPieceType() == EPieceType::Roof)
		{
			if (HitPiece && HitPiece->GetPieceType() == EPieceType::Wall)
			{
				// Only override the Z (Height) axis so it sits on top. Leave X and Y alone so it stays snapped to the edge!
				float WallHeightOffset = 300.0f;
				Snapped.Z = HitPiece->GetActorLocation().Z + WallHeightOffset;
				bCanPlace = true;
			}
			else
			{
				bCanPlace = false; // Looked at the ground? No roof for you!
			}
		}

		// Update the physical location and rotation of the ghost piece.
		PreviewPiece->SetActorLocationAndRotation(Snapped, FRotator(0, BuildRotationYaw, 0));
	}
	else
	{
		bCanPlace = false; // Aiming at the sky.
	}
}

// [Week 3] The action function that converts a ghost piece into a real, solid game object.
void AMainPlayer::PlaceBuilding()
{
	// Ensure we have a preview, we have items, and the location is valid (no overlaps/floating roofs).
	if (PreviewPiece && CurrentBuildingVariations.Num() > 0 && bCanPlace)
	{
		if (HasBuildingItem(PreviewPiece->GetPieceType()))
		{
			// Take the item out of the player's backpack
			ConsumeBuildingItem(PreviewPiece->GetPieceType());

			// Spawn the final, solid object with collision enabled!
			GetWorld()->SpawnActor<ABuildablePiece>(CurrentBuildingVariations[CurrentVariationIndex], PreviewPiece->GetActorLocation(), PreviewPiece->GetActorRotation());

			// Tell the UI to check if that was our last piece and update the icon if necessary.
			bool bHasMats = HasBuildingItem(PreviewPiece->GetPieceType());
			UpdateBuildUI(EquippedPieceIndex, bHasMats);
		}
	}
}

void AMainPlayer::RotateBuilding()
{
	// [Week 3] Add 90 degrees to the yaw and wrap it using Fmod.
	BuildRotationYaw = FMath::Fmod(BuildRotationYaw + 90.0f, 360.0f);
}

void AMainPlayer::CycleBuildingVariation()
{
	// [Week 3] Loop through the array of variations (like windowed walls vs solid walls) and restart the preview.
	if (!PreviewPiece || CurrentBuildingVariations.Num() <= 1) return;
	CurrentVariationIndex = (CurrentVariationIndex + 1) % CurrentBuildingVariations.Num();
	StartBuilding(CurrentBuildingVariations);
}