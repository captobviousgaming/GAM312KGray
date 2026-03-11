#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InteractableItem.h" // Added so the player knows what our Resource Enum is
#include "MainPlayer.generated.h"

UCLASS()
class GAM_312_KGRAY_API AMainPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	AMainPlayer();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FirstPersonCameraComponent;

	void MoveForward(float Value);
	void MoveRight(float Value);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Hunger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Stamina;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Wood;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Stone;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Berry;

	void HandleStatsOverTime();
	FTimerHandle StatTimerHandle;
	void Interact();

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// --- CUSTOM UI: WIDGET SETUP ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> StatsWidgetClass;

	UPROPERTY()
	class UUserWidget* StatsWidget;

	// --- CUSTOM UI: POPUP TRIGGER ---
	// This tells C++ to send a signal to Blueprints so we can spawn the visual popup
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void ShowResourcePopup(EResourceType ResourceType, int32 Amount);
};