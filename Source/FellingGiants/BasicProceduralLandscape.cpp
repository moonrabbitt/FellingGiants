// Fill out your copyright notice in the Description page of Project Settings.
//To do - play with render frame rate

#include "BasicProceduralLandscape.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GrassMesh.h"


// Sets default values
ABasicProceduralLandscape::ABasicProceduralLandscape()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>("ProceduralMesh");
	ProceduralMesh->SetupAttachment(GetRootComponent());

	GrassMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GrassMeshComponent"));
	GrassMeshComponent->SetupAttachment(GetRootComponent());

	RenderTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("RenderTarget")); // Initialize RenderTarget

}

// Called when the game starts or when spawned
void ABasicProceduralLandscape::BeginPlay()
{
	Super::BeginPlay();
	CreateVerticesWithoutHeightMap();
	CreateTriangles();
	SpawnGrass();
	
}

void ABasicProceduralLandscape::OnConstruction(const FTransform& Transform)
{	//No longer need to run game, object loaded on construction script which is exectuted before game begins
	//Clear old variables on construction
	
	if (RenderTarget != nullptr) // SpoutSource is the UTextureRenderTarget2D
		{
		Vertices.Reset();
		Triangles.Reset();
		UV0.Reset();
		CreateVertices();
		CreateTriangles();
		UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UV0, Normals, Tangents);
		ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, TArray<FColor>(), Tangents, true);
		}
	else
	{
		// Handle case where HeightmapTexture becomes null after being initially not null
		Vertices.Reset();
		Triangles.Reset();
		UV0.Reset();
		CreateVerticesWithoutHeightMap();  // Using Perlin Noise
		CreateTriangles();
		UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UV0, Normals, Tangents);
		ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, TArray<FColor>(), Tangents, true);
	}
	
	
}

// Called every frame
void ABasicProceduralLandscape::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (RenderTarget != nullptr) // SpoutSource is the UTextureRenderTarget2D
		{
		Vertices.Reset();
		Triangles.Reset();
		UV0.Reset();
		CreateVertices();
		CreateTriangles();
		UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UV0, Normals, Tangents);
		ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, TArray<FColor>(), Tangents, true);
		}
	else
	{
		// Handle case where HeightmapTexture becomes null after being initially not null
		Vertices.Reset();
		Triangles.Reset();
		UV0.Reset();
		CreateVerticesWithoutHeightMap();  // Using Perlin Noise
		CreateTriangles();
		UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UV0, Normals, Tangents);
		ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, TArray<FColor>(), Tangents, true);
	}
}

void ABasicProceduralLandscape::CreateVerticesWithoutHeightMap()
{
	if (RenderTarget == nullptr)
	{
		// Use Perlin noise for height
		UE_LOG(LogTemp, Warning, TEXT("Heightmap texture is null, using Perlin Noise for height"));
		for (int X = 0; X <= XSize; ++X)
		{
			for (int Y = 0; Y <= YSize; ++Y)
			{
				float Z = FMath::PerlinNoise2D(FVector2D(X * NoiseScale + 0.1, Y * NoiseScale + 0.1)) * ZMultiplier;
				Vertices.Add(FVector(X * Scale, Y * Scale, Z));
				UV0.Add(FVector2D(X * UVScale, Y * UVScale));
			}
		}
	}
}

void ABasicProceduralLandscape::CreateVertices()
{ //read old vertices and just update z
	FRenderTarget* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();

	if (RenderTargetResource != nullptr)
	{
		FReadSurfaceDataFlags ReadSurfaceDataFlags;
		ReadSurfaceDataFlags.SetLinearToGamma(false); // Set this to true if you need gamma-corrected colors

		TArray<FColor> FormatedImageData;
		RenderTargetResource->ReadPixels(FormatedImageData, ReadSurfaceDataFlags);

		int32 TextureWidth = RenderTarget->SizeX;
		int32 TextureHeight = RenderTarget->SizeY;

		int32 VertexIndex = 0;  // Track the current index of Vertices array
		for (int X = 1; X <= XSize+1; ++X)
		{
			for (int Y = 1; Y <= YSize+1; ++Y)
			{
				int32 TextureX = FMath::Clamp(int(X * (TextureWidth / XSize)), 0, TextureWidth - 1);
				int32 TextureY = FMath::Clamp(int(Y * (TextureHeight / YSize)), 0, TextureHeight - 1);

				FColor PixelColor = FormatedImageData[TextureY * TextureWidth + TextureX];

				float Z = PixelColor.A / 255.0f * ZMultiplier; //set touch designer Normal setting to heightmap in alpha

				// Only change the Z value of the existing vertex, if it exists
				if (Vertices.IsValidIndex(VertexIndex))
				{
					Vertices[VertexIndex].Z = Z;
				}
				// If the vertex does not exist, add it to the array
				else
				{
					Vertices.Add(FVector(X * Scale, Y * Scale, Z));
					UV0.Add(FVector2D(X * UVScale, Y * UVScale));
				}
				++VertexIndex;  // Increment the vertex index for the next loop
			}
		}
	}
}


// void ABasicProceduralLandscape::CreateVertices()
// //create new vertices every frame
// {
// 	//change collision to always update so collision re-renders
// 		FRenderTarget* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
//
// 		if (RenderTargetResource != nullptr)
// 		{
// 			
// 			FReadSurfaceDataFlags ReadSurfaceDataFlags;
// 			ReadSurfaceDataFlags.SetLinearToGamma(false); // Set this to true if you need gamma-corrected colors
//
// 			TArray<FColor> FormatedImageData;
// 			RenderTargetResource->ReadPixels(FormatedImageData, ReadSurfaceDataFlags);
//
// 			int32 TextureWidth = RenderTarget->SizeX;
// 			int32 TextureHeight = RenderTarget->SizeY;
//
// 			for (int X = 1; X <= XSize+1; ++X)
// 			{
// 				for (int Y = 1; Y <= YSize+1; ++Y)
// 				{
// 					int32 TextureX = FMath::Clamp(int(X * (TextureWidth / XSize)), 0, TextureWidth - 1);
// 					int32 TextureY = FMath::Clamp(int(Y * (TextureHeight / YSize)), 0, TextureHeight - 1);
//
// 					FColor PixelColor = FormatedImageData[TextureY * TextureWidth + TextureX];
//
// 					float Z = PixelColor.A / 255.0f * ZMultiplier; //set touch designer Normal setting to heightmap in alpha
//
// 					Vertices.Add(FVector(X * Scale, Y * Scale, Z));
// 					UV0.Add(FVector2D(X * UVScale, Y * UVScale));
// 				}
// 			}
// 		}
// 	
// }



void ABasicProceduralLandscape::CreateTriangles()
{
	int Vertex = 0;

	for (int X = 0; X < XSize; ++X)
	{
		for (int Y = 0; Y < YSize; ++Y)
		{
			Triangles.Add(Vertex);//Bottom left corner
			Triangles.Add(Vertex + 1);//Bottom right corner
			Triangles.Add(Vertex + YSize + 1);//Top left corner
			Triangles.Add(Vertex + 1);//Bottom right corner
			Triangles.Add(Vertex + YSize + 2);//Top right corner
			Triangles.Add(Vertex + YSize + 1);//Top left corner

			++Vertex;
		}
		++Vertex;
	}
}

void ABasicProceduralLandscape::SpawnGrass()
{
	// Ensure the grass mesh has been set in the editor.
	if (!GrassMeshComponent->GetStaticMesh())
	{
		UE_LOG(LogTemp, Error, TEXT("Grass mesh not set!"));
		return;
	}

	// Clear existing grass instances
	GrassMeshComponent->ClearInstances();

	// Add an instance of the grass mesh for each vertex in the landscape
	for (const FVector& Vertex : Vertices)
	{
		FTransform InstanceTransform;
		InstanceTransform.SetLocation(Vertex);
		GrassMeshComponent->AddInstance(InstanceTransform);
	}
}
