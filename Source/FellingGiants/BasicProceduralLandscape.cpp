// Fill out your copyright notice in the Description page of Project Settings.
//To do - play with render frame rate

#include "BasicProceduralLandscape.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/Engine.h" // Include the engine header file so we can use the debug functions
#include "GrassMesh.h"



// Sets default values
ABasicProceduralLandscape::ABasicProceduralLandscape()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>("ProceduralMesh");//create procedural mesh component
	ProceduralMesh->SetupAttachment(GetRootComponent()); //attach to root component

	GrassMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GrassMeshComponent")); //create grass mesh component
	GrassMeshComponent->SetupAttachment(GetRootComponent()); //attach to root component

	RenderTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("RenderTarget")); // Initialize RenderTarget

	bHasSpawnedGrass = false; //haven't spawned grass yet
	GrassMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); //grass has no collision


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
	SpawnGrass();
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("bHasSpawnedGrass: %d"), bHasSpawnedGrass));
}

// Called every frame
void ABasicProceduralLandscape::Tick(float DeltaTime)
{
	
	//print value of bHasSpawnedGrass to screen
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("bHasSpawnedGrass: %d"), bHasSpawnedGrass));
	
	Super::Tick(DeltaTime);

	if (!bHasSpawnedGrass)
	{
		SpawnGrass();
		bHasSpawnedGrass = true;
	}

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

	UpdateGrassZPosition();
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
		for (int i = 0; i < GrassDensity; i++){
			// Random size of grass
			float RandomHeight = FMath::RandRange(MinGrassHeight, MaxGrassHeight); // MinHeight and MaxHeight should be your desired values

			// Random rotation
			float RandomYaw = FMath::RandRange(0.0f, 360.0f);

			// Bounds of the landscape
			FVector Offset = FVector(FMath::RandRange(0.0f, GrassDensity*Scale),FMath::RandRange(0.0f, GrassDensity*Scale),0);
			FVector Position = GetActorTransform().TransformPosition(Vertex) +Offset;
			float Z = GetInterpolatedZ(Position);
			FBox Bounds = GetComponentsBoundingBox();
			if (!Bounds.IsInside(Position))
			{
				// 				FString PositionString = Position.ToString();
				// 				PrintToScreen((TEXT("Position %s is outside landscape bounds!"), *PositionString));
				continue;
			}
		
			FTransform InstanceTransform;
			
			// Set grass size
			FVector LocalGrassSize = FVector(FMath::RandRange(1.0f, GrassSize));
			InstanceTransform.SetScale3D(LocalGrassSize);
			InstanceTransform.SetLocation(GetActorTransform().TransformPosition(Vertex) +Offset); //distance between each vertex = scale
			// Calculate the Z position by interpolating between the surrounding vertices
			
			InstanceTransform.SetLocation(FVector(Position.X, Position.Y, Z));
			InstanceTransform.SetRotation(FRotator(0, RandomYaw, 0).Quaternion());

			GrassMeshComponent->AddInstance(InstanceTransform);
		}
	}
}

float ABasicProceduralLandscape::GetInterpolatedZ(const FVector2D& Point) const
{
	// Determine the grid cell indices
	int32 LowerX = FMath::FloorToInt(Point.X / Scale);
	int32 LowerY = FMath::FloorToInt(Point.Y / Scale);

	// Clamp the indices to valid range
	LowerX = FMath::Clamp(LowerX, 0, XSize - 1);
	LowerY = FMath::Clamp(LowerY, 0, YSize - 1);

	// Calculate the interpolation factors
	float AlphaX = (Point.X - LowerX * Scale) / Scale;
	float AlphaY = (Point.Y - LowerY * Scale) / Scale;

	// Get the heights of the four grid vertices
	float Height00 = Vertices[LowerY * (XSize + 1) + LowerX].Z;
	float Height01 = Vertices[LowerY * (XSize + 1) + LowerX + 1].Z;
	float Height10 = Vertices[(LowerY + 1) * (XSize + 1) + LowerX].Z;
	float Height11 = Vertices[(LowerY + 1) * (XSize + 1) + LowerX + 1].Z;

	// Interpolate the heights bilinearly
	float Height0 = FMath::Lerp(Height00, Height01, AlphaX);
	float Height1 = FMath::Lerp(Height10, Height11, AlphaX);
	float Height = FMath::Lerp(Height0, Height1, AlphaY);

	return Height;
}



//
// void ABasicProceduralLandscape::SpawnGrass()
// {
// 	// Ensure the grass mesh has been set in the editor.
// 	if (!GrassMeshComponent->GetStaticMesh())
// 	{
// 		UE_LOG(LogTemp, Error, TEXT("Grass mesh not set!"));
// 		PrintToScreen(TEXT("Grass mesh not set!"));
// 		return;
// 	}
//
// 	// Clear existing grass instances
// 	GrassMeshComponent->ClearInstances();
//
// 	// Random engine
// 	FRandomStream RandStream;
// 	RandStream.Initialize(FMath::Rand());  // Initialize random stream
//
// 	// Bounds of the landscape
// 	FBox Bounds = GetComponentsBoundingBox();
//
// 	// Add an instance of the grass mesh for each vertex in the landscape
// 	for (const FVector& Vertex : Vertices)
// 	{
//
// 		// Generate grass instances around the vertex
// 		for (int32 j = 0; j < GrassDensity; ++j)
// 		{
// 			float Angle = RandStream.FRand() * 2.0f * PI; // Random angle
// 			float Radius = RandStream.FRand() * GrassSpawnRadius; // Random radius
//
// 			// FVector Offset(Radius * FMath::Cos(Angle), Radius * FMath::Sin(Angle), 0);
// 			FVector Offset(0,0, 0);
// 			FVector Position = Vertex + Offset;
//
// 			// Ensure the position is within the landscape bounds
// 			
// 			// Ensure the position is within the landscape bounds
// 			if (!Bounds.IsInside(Position))
// 			{
// 				FString PositionString = Position.ToString();
// 				PrintToScreen((TEXT("Position %s is outside landscape bounds!"), *PositionString));
// 				continue;
// 			}
//
// 			// Random height
// 			float RandomHeight = FMath::RandRange(MinGrassHeight, MaxGrassHeight);
//
// 			// Random rotation
// 			float RandomYaw = FMath::RandRange(0.0f, 360.0f);
// 			
// 			FTransform InstanceTransform; // Transform of the grass instance
//
// 			// Set grass size
// 			FVector LocalGrassSize = FVector(GrassSize);
// 			InstanceTransform.SetScale3D(LocalGrassSize);
// 			
// 			// Set grass location and rotation
// 			InstanceTransform.SetLocation(GetActorTransform().TransformPosition(Position) + FVector(0, 0, RandomHeight));
// 			InstanceTransform.SetRotation(FRotator(0, RandomYaw, 0).Quaternion());
//
// 			// Add the grass instance
// 			GrassMeshComponent->AddInstance(InstanceTransform);
// 		}
// 	}
// }


//function to print error message to screen
void ABasicProceduralLandscape::PrintToScreen(FString Message)
{
	FColor Color = FColor::Red;
	float DisplayTime = 5.0f; // Duration to display the message on screen
	GEngine->AddOnScreenDebugMessage(-1, DisplayTime, Color, Message);
}

void ABasicProceduralLandscape::UpdateGrassZPosition()
{	int32 Count = FMath::Min(GrassMeshComponent->GetInstanceCount(), Vertices.Num());
	for (int32 i = 0; i < Count; ++i)
	{
		FTransform InstanceTransform;
		GrassMeshComponent->GetInstanceTransform(i, InstanceTransform, true);
        
		// The index i should match with the index of the Vertices array, if the number of grass instances
		// and the number of vertices are the same.
		
		if (Vertices.IsValidIndex(i))
		{
			FVector NewLocation = InstanceTransform.GetLocation();
			NewLocation.Z = GetActorTransform().TransformPosition(Vertices[i]).Z;
			InstanceTransform.SetLocation(NewLocation);

			GrassMeshComponent->UpdateInstanceTransform(i, InstanceTransform, true, true);
		}
	}
}