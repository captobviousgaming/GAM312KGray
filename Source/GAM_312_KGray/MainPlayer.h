#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InteractableItem.h" // Required so my struct recognizes the EResourceType enum
#include "BuildablePiece.h" 
#include "MainPlayer.generated.h"

class ABuildablePiece;

// --- DYNAMIC INVENTORY STRUCT ---
// I created a custom data container to hold both an Item Type and its Quantity in a single "Slot".
// BlueprintType allows me to access this struct easily within my UI Blueprints.
USTRUCT(BlueprintType)
struct FInventoryItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	EResourceType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Quantity;

	// Default constructor required by Unreal Engine
	FInventoryItem()
	{
		ItemType = EResourceType::Wood;
		Quantity = 0;
	}

	// Custom constructor allowing me to easily create a new inventory slot when I pick up an item
	FInventoryItem(EResourceType InType, int32 InQuantity)
	{
		ItemType = InType;
		Quantity = InQuantity;
	}
};

UCLASS()
class GAM_312_KGRAY_API AMainPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMainPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// --- CAMERA ---
	// The first-person camera component allowing the player to view the world
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FirstPersonCameraComponent;

	// Movement binding functions for WASD navigation
	void MoveForward(float Value);
	void MoveRight(float Value);

	// --- PLAYER STATS ---
	// Variables tracking player survival metrics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Hunger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Stamina;

	// --- RAW INVENTORY VALUES ---
	// I am keeping these base integers here so my existing building logic remains intact,
	// though I will primarily use the InventoryArray for my new Bag UI system.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Wood;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Stone;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Berry;

	// Handles continuous depletion of hunger and restoration of stamina over time
	void HandleStatsOverTime();
	FTimerHandle StatTimerHandle;

	// Fires a line trace to gather resources from interactable items
	void Interact();

	// --- DYNAMIC INVENTORY ARRAY ---
	// This array holds my inventory structs, naturally ordering items from left to right for the UI.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<FInventoryItem> InventoryArray;

	// Helper function I created to determine if I should stack a gathered item or create a new slot.
	void AddResourceToInventory(EResourceType GatheredType, int32 Amount);

	// Toggles the visibility of my new Bag UI menu
	void ToggleInventoryMenu();

	// Tracks the state of the Bag menu (open/closed)
	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	bool bIsInventoryOpen;

	// --- BUILDING LOGIC ---
	void ToggleBuildMenu();

	UPROPERTY(BlueprintReadWrite, Category = "Building")
	bool bIsBuildMenuOpen;

	void PlaceBuilding();
	void RotateBuilding();
	void CycleBuildingVariation();
	void UpdateBuildingPreview();

	UPROPERTY()
	ABuildablePiece* PreviewPiece;

	float BuildRotationYaw;
	int32 CurrentVariationIndex;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to specific input keys
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// --- UI WIDGETS ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> StatsWidgetClass;

	UPROPERTY()
	class UUserWidget* StatsWidget;

	// Blueprint implementable event to trigger the visual resource popup upon gathering
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void ShowResourcePopup(EResourceType ResourceType, int32 Amount);

	// --- BAG UI WIDGET ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> InventoryMenuClass;

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	class UUserWidget* InventoryMenuWidget;

	// --- BUILDING UI WIDGET ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> BuildMenuClass;

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	class UUserWidget* BuildMenuWidget;

	UFUNCTION(BlueprintCallable, Category = "Building")
	void StartBuilding(TArray<TSubclassOf<AActor>> Variations);

	UPROPERTY(BlueprintReadWrite, Category = "Building")
	TArray<TSubclassOf<AActor>> CurrentBuildingVariations;
};