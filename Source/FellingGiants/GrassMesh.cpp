// Fill out your copyright notice in the Description page of Project Settings.

#include "GrassMesh.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

// Sets default values
AGrassMesh::AGrassMesh()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	

	// Create and attach the mesh component
	GrassMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrassMeshComponent"));
	RootComponent = GrassMeshComponent;



	// // Load a specific mesh
	// auto MeshAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("/Game/Grass/grass.StaticMesh"));
	//
	// if (MeshAsset.Succeeded())
	// {
	// 	GrassMeshComponent->SetStaticMesh(MeshAsset.Object);
	// }
}

// Called when the game starts or when spawned
void AGrassMesh::BeginPlay()
{
	Super::BeginPlay();
	
}


void AGrassMesh::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    
	// your code here
}
