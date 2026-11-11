// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "TimerManager.h"
#include "Devoir3ShooterCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class AController;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class ADevoir3ShooterCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input", meta=(AllowPrivateAccess="true"))
	UInputAction* FireAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

public:

    // Shared health system for player and AI characters
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Health", meta=(ClampMin="1.0"))
    float MaxHealth = 100.0f;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Health")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Health", meta=(ClampMin="0.0"))
    float RespawnDelay = 2.0f;

    UFUNCTION(BlueprintPure, Category="Combat|Health")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category="Combat|Health")
    bool IsDead() const { return bIsDead; }

    UFUNCTION(BlueprintImplementableEvent, Category="Combat|Health")
    void BP_OnHealthChanged(float NewHealth, float InMaxHealth);

    UFUNCTION(BlueprintImplementableEvent, Category="Combat|Health")
    void BP_OnDeath();

    virtual float TakeDamage(float DamageAmount, const struct FDamageEvent& DamageEvent,
                             AController* EventInstigator, AActor* DamageCauser) override;

    /** Constructor */
	ADevoir3ShooterCharacter();	

protected:

    virtual void BeginPlay() override;

private:
    bool bIsDead = false;
    FTimerHandle RespawnTimer;
    TWeakObjectPtr<AController> ControllerBeforeDeath;

    void HandleDeath();
    void RespawnPlayer();

protected:

    /** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Fire a hitscan shot from the player camera */
	UFUNCTION(BlueprintCallable, Category="Combat")
	void Fire();

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

