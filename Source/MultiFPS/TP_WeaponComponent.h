// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "TP_WeaponComponent.generated.h"

class UInputAction;
class UInputMappingContext;
class AMultiFPSCharacter;

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIFPS_API UTP_WeaponComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AMultiFPSProjectile> ProjectileClass;

	/** MuzzleFlashEffect to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	TObjectPtr<UParticleSystem> MuzzleFlashEffect;
	
	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	TObjectPtr<USoundBase> FireSound;
	
	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	TObjectPtr<UAnimMontage> FireAnimation;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector MuzzleOffset;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> FireMappingContext;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> FireAction;

	/** Sets default values for this component's properties */
	UTP_WeaponComponent();


	virtual void InitializeComponent() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	/** Attaches the actor to a FirstPersonCharacter */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void AttachWeapon(AMultiFPSCharacter* TargetCharacter);

	/** Make the weapon Fire a Projectile */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void Fire();

	UFUNCTION(BlueprintCallable, Category="Weapon")
	void OnFire(const FVector& SpawnLocation, const FRotator& SpawnRotation);
	
	UFUNCTION()
	void CreateProjectile(const FVector& SpawnLocation, const FRotator& SpawnRotation);

	UFUNCTION()
	void PlayEffect(const FVector& SpawnLocation, const FRotator& SpawnRotation);

	UFUNCTION()
	void PlaySound();

	UFUNCTION()
	void PlayMontage();

	UFUNCTION()
	FTransform GetSocketTransformByName( FName InSocketName, const class USkeletalMeshComponent* SkelComp);
	
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector& SpawnLocation, const FRotator& SpawnRotation);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector& SpawnLocation, const FRotator& SpawnRotation);

	UFUNCTION()
	void OnRep_FireStartTime();
	
protected:
	/** Ends gameplay for this component. */
	UFUNCTION()
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** The Character holding this weapon*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	TObjectPtr<AMultiFPSCharacter> Character;

	UPROPERTY(EditAnywhere, Category = "Properties|Weapon")
	FName MuzzleSocketName;
	
	UPROPERTY(ReplicatedUsing = OnRep_FireStartTime)
	float FireStartTime;
	
};
