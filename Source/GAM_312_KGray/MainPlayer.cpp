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
	PrimaryActorTick.bCanEverTick = true;

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	Health = 100.0f; Hunger = 100.0f; Stamina = 100.0f;
	Wood = 1000; Stone = 1000; Berry = 0;

	bIsBuildMenuOpen = false;
	bIsInventoryOpen = false;
	PreviewPiece = nullptr;
	BuildRotationYaw = 0.0f;
	CurrentVariationIndex = 0;
	EquippedPieceIndex = 0;

	SnapGridSize = 200.0f;
	bCanPlace = false;
}

void AMainPlayer::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(StatTimerHandle, this, &AMainPlayer::HandleStatsOverTime, 1.0f, true);

	AddResourceToInventory(EResourceType::Wall, 5);
	AddResourceToInventory(EResourceType::Floor, 5);
	AddResourceToInventory(EResourceType::Roof, 5);

	if (StatsWidgetClass)
	{
		StatsWidget = CreateWidget<UUserWidget>(GetWorld(), StatsWidgetClass);
		if (StatsWidget) { StatsWidget->AddToViewport(); }
	}
}

void AMainPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsBuildMenuOpen && PreviewPiece) { UpdateBuildingPreview(); }
}

void AMainPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMainPlayer::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainPlayer::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMainPlayer::Interact);
	PlayerInputComponent->BindAction("ToggleInventoryMenu", IE_Pressed, this, &AMainPlayer::ToggleInventoryMenu);
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
	if (Value != 0.0f) { AddMovementInput(GetActorForwardVector(), Value); }
}

void AMainPlayer::MoveRight(float Value)
{
	if (Value != 0.0f) { AddMovementInput(GetActorRightVector(), Value); }
}

void AMainPlayer::HandleStatsOverTime()
{
	if (Hunger > 0.0f) { Hunger -= 1.0f; }
	else { Health -= 2.0f; }
}

void AMainPlayer::Interact()
{
	if (!FirstPersonCameraComponent) return;
	FVector Start = FirstPersonCameraComponent->GetComponentLocation();
	FVector End = Start + (FirstPersonCameraComponent->GetForwardVector() * 500.0f);
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		AInteractableItem* Item = Cast<AInteractableItem>(Hit.GetActor());
		if (Item)
		{
			EResourceType Type;
			if (Item->GatherResource(Type))
			{
				if (Type == EResourceType::Wood) Wood++;
				else if (Type == EResourceType::Stone) Stone++;
				ShowResourcePopup(Type, 1);
				AddResourceToInventory(Type, 1);
			}
		}
	}
}

void AMainPlayer::AddResourceToInventory(EResourceType Type, int32 Amount)
{
	for (auto& Item : InventoryArray)
	{
		if (Item.ItemType == Type) { Item.Quantity += Amount; return; }
	}
	InventoryArray.Add(FInventoryItem(Type, Amount));
}

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

void AMainPlayer::ToggleBuildMenu()
{
	if (bIsBuildMenuOpen)
	{
		if (BuildMenuWidget) { BuildMenuWidget->RemoveFromParent(); }
		bIsBuildMenuOpen = false;
		if (PreviewPiece) { PreviewPiece->Destroy(); PreviewPiece = nullptr; }
		EquippedPieceIndex = 0;
		UpdateBuildUI(0, false);
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

void AMainPlayer::StartBuilding(TArray<TSubclassOf<ABuildablePiece>> Variations)
{
	if (Variations.Num() == 0) return;
	CurrentBuildingVariations = Variations;
	CurrentVariationIndex = 0;
	if (PreviewPiece) { PreviewPiece->Destroy(); }

	PreviewPiece = GetWorld()->SpawnActor<ABuildablePiece>(CurrentBuildingVariations[0], FVector::ZeroVector, FRotator::ZeroRotator);
	if (PreviewPiece)
	{
		PreviewPiece->SetActorEnableCollision(false);

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

		bool bHasMats = HasBuildingItem(PreviewPiece->GetPieceType());
		PreviewPiece->SetActorHiddenInGame(!bHasMats);
		UpdateBuildUI(EquippedPieceIndex, bHasMats);
	}
}

void AMainPlayer::UpdateBuildingPreview()
{
	if (!PreviewPiece || !FirstPersonCameraComponent) return;

	bool bHasMats = HasBuildingItem(PreviewPiece->GetPieceType());
	PreviewPiece->SetActorHiddenInGame(!bHasMats);

	FVector Start = FirstPersonCameraComponent->GetComponentLocation();
	FVector End = Start + (FirstPersonCameraComponent->GetForwardVector() * 800.0f);
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(PreviewPiece);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		// FIX: Push the target outward along the surface normal before calculating the grid snap.
		// This forces pieces to snap to the edge rather than getting stuck inside thin walls!
		FVector TargetLocation = Hit.Location + (Hit.ImpactNormal * (SnapGridSize * 0.4f));

		float HalfGrid = SnapGridSize / 2.0f;
		FVector Snapped(
			FMath::RoundToFloat(TargetLocation.X / HalfGrid) * HalfGrid,
			FMath::RoundToFloat(TargetLocation.Y / HalfGrid) * HalfGrid,
			FMath::RoundToFloat(TargetLocation.Z / SnapGridSize) * SnapGridSize
		);

		bCanPlace = true;
		ABuildablePiece* HitPiece = Cast<ABuildablePiece>(Hit.GetActor());

		for (TActorIterator<ABuildablePiece> It(GetWorld()); It; ++It)
		{
			if (*It != PreviewPiece && FVector::Dist(It->GetActorLocation(), Snapped) < 10.0f)
			{
				bCanPlace = false;
				break;
			}
		}

		if (PreviewPiece->GetPieceType() == EPieceType::Roof)
		{
			if (HitPiece && HitPiece->GetPieceType() == EPieceType::Wall)
			{
				float WallHeightOffset = 300.0f;
				Snapped.Z = HitPiece->GetActorLocation().Z + WallHeightOffset;
				bCanPlace = true;
			}
			else
			{
				bCanPlace = false;
			}
		}

		PreviewPiece->SetActorLocationAndRotation(Snapped, FRotator(0, BuildRotationYaw, 0));
	}
	else
	{
		bCanPlace = false;
	}
}

void AMainPlayer::PlaceBuilding()
{
	if (PreviewPiece && CurrentBuildingVariations.Num() > 0 && bCanPlace)
	{
		if (HasBuildingItem(PreviewPiece->GetPieceType()))
		{
			ConsumeBuildingItem(PreviewPiece->GetPieceType());

			GetWorld()->SpawnActor<ABuildablePiece>(CurrentBuildingVariations[CurrentVariationIndex], PreviewPiece->GetActorLocation(), PreviewPiece->GetActorRotation());

			bool bHasMats = HasBuildingItem(PreviewPiece->GetPieceType());
			UpdateBuildUI(EquippedPieceIndex, bHasMats);
		}
	}
}

void AMainPlayer::RotateBuilding()
{
	BuildRotationYaw = FMath::Fmod(BuildRotationYaw + 90.0f, 360.0f);
}

void AMainPlayer::CycleBuildingVariation()
{
	if (!PreviewPiece || CurrentBuildingVariations.Num() <= 1) return;
	CurrentVariationIndex = (CurrentVariationIndex + 1) % CurrentBuildingVariations.Num();
	StartBuilding(CurrentBuildingVariations);
}