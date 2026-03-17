#include "InteractableItem.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AInteractableItem::AInteractableItem()
{
	// Resource nodes are static objects in my game, so they do not need to update every frame.
	PrimaryActorTick.bCanEverTick = false;

	// I create the visual mesh component and set it as the root of the actor here.
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	RootComponent = ItemMesh;

	// I am setting default values here so the object has basic functionality immediately upon spawning.
	ResourceType = EResourceType::Wood;
	ResourceCapacity = 3;
}

// Executes when the player successfully interacts with this object via line trace
bool AInteractableItem::GatherResource(EResourceType& OutResourceType)
{
	// First, I check if the node still has resources left to give.
	if (ResourceCapacity > 0)
	{
		// I pass the specific resource type back to the player's inventory logic.
		OutResourceType = ResourceType;

		// I deplete the node's capacity by 1.
		ResourceCapacity--;

		// If the node is completely empty after this swing, I remove it from the game world.
		if (ResourceCapacity <= 0)
		{
			Destroy();
		}

		return true; // The gathering action was successful
	}

	return false; // The node was empty, so gathering failed
}