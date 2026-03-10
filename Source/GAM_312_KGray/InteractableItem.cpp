#include "InteractableItem.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AInteractableItem::AInteractableItem()
{
	// Set this actor to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;

	// Create the visual mesh component and set it as the root of the object
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	RootComponent = ItemMesh;

	// Set default values so it starts as a Tree with 3 wood
	ResourceType = EResourceType::Wood;
	ResourceCapacity = 3;
}

// Called when the game starts or when spawned
void AInteractableItem::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AInteractableItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// --- WEEK 2: GATHER LOGIC ---
bool AInteractableItem::GatherResource(EResourceType& OutResourceType)
{
	// Check if the item still has resources left to give
	if (ResourceCapacity > 0)
	{
		// Pass the resource type back to whoever called this function (the player)
		OutResourceType = ResourceType;

		// Deplete the capacity by 1
		ResourceCapacity--;

		// If the item is now empty, destroy it to remove it from the game world
		if (ResourceCapacity <= 0)
		{
			Destroy();
		}

		// Tell the player the gather was successful
		return true;
	}

	// Tell the player the gather failed (it was already empty)
	return false;
}