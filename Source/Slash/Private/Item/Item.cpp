// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/Item.h"
#include "Slash/DebugMacros.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "Characters/SlashCharacter.h"

AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComponent"));
	RootComponent = ItemMesh;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(GetRootComponent());
	
	EmbersEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Embers"));
	EmbersEffect->SetupAttachment(GetRootComponent());
}

void AItem::BeginPlay()
{
	Super::BeginPlay();

	/* 콜백을 델리게이트에 바인딩 */
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);
}

float AItem::TransformedSin()
{
	return Amplitude * FMath::Sin(RunningTime * TimeConstant);
}

float AItem::TransformedCos()
{
	return Amplitude * FMath::Cos(RunningTime * TimeConstant);
}

/*
 * 이 함수는 충돌 영역(Sphere Component)에 다른 액터가 겹쳤을 때 호출된다.
 * 아이템이 플레이어와 겹칠 때 실행되며, 겹친 액터(OtherActor)가 어떤 대상인지, 충돌 정보(SweepResult) 등을 확인하고 필요한 처리를 할 수 있다.
 */
void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(OtherActor);
	if (SlashCharacter)
	{
		SlashCharacter->SetOverlappingItem(this);
	}
}
// // 그걸 SlashCharacter에게 알려주는 거다 즉, “내가 지금 너랑 겹쳤던 아이템이야! 기억해!” 라는 뜻

/*
 * 이 함수는 충돌 영역(Sphere Component)에서 다른 액터가 영역 밖으로 나갔을 때 호출된다
 * 예를 들어, 플레이어가 아이템 주변을 벗어났을 때 해당 액터의 이름을 디버그 메시지로 출력한다.
 */
void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(OtherActor);
	if (SlashCharacter)
	{
		// this는 현재 클래스인 AItem을 의미
		SlashCharacter->SetOverlappingItem(nullptr);
	}
}

void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RunningTime += DeltaTime;

	if (ItemState == EItemState::EIS_Hovering)
	{
		AddActorWorldOffset(FVector(0.f, 0.f, TransformedSin()));
	}
}
