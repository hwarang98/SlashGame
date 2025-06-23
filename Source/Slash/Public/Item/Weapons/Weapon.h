// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Item.h"
#include "Weapon.generated.h"

class USoundBase;
class UBoxComponent;

/**
 * 
 */
UCLASS()
class SLASH_API AWeapon : public AItem
{
	GENERATED_BODY()

public:
	AWeapon();
	void Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator);
	void AttackMeshToSocket(USceneComponent* InParent, FName InSocketName);
	
	TArray<AActor*> IgnoreActors;
	
	FORCEINLINE UBoxComponent* GetWeaponBox() const { return WeaponBox; };

protected:
	virtual void BeginPlay() override; 
	
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	) override;

	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	) override;

	UFUNCTION()
	void OnBoxOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION(BlueprintImplementableEvent)
	void CreateFields(const FVector& FieldLocation);

private:
	UPROPERTY(EditAnywhere, Category = "무기 속성")
	USoundBase* EquipSound;

	UPROPERTY(VisibleAnywhere, Category = "무기 속성")
	UBoxComponent* WeaponBox;

	UPROPERTY(VisibleAnywhere, Category = "무기 속성")
	USceneComponent* BoxTraceStart;

	UPROPERTY(VisibleAnywhere, Category = "무기 속성")
	USceneComponent* BoxTraceEnd;

	UPROPERTY(EditAnywhere, Category = "무기 속성")
	float Damage = 20.f;
};
