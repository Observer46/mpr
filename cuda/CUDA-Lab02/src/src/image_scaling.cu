#include<stdio.h>
#include"scrImagePgmPpmPackage.h"
#include "gputimer.h"

//Kernel which calculate the resized image
__global__ void createResizedImage(unsigned char *imageScaledData, 
									int scaled_width,
									int scaled_height,
									float scale_factor, 
									cudaTextureObject_t texObj)
{
	unsigned int tidX = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int tidY = blockIdx.y * blockDim.y + threadIdx.y;
	unsigned index = tidY * scaled_width + tidX;

	const unsigned int coef_x = gridDim.x * blockDim.x;
	const unsigned int coef_y = gridDim.y * blockDim.y;
	const unsigned int pixel_count = scaled_width * scaled_height;
       	
	// Step 4: Read the texture memory from your texture reference in CUDA Kernel
	// printf("index: %d\n", index);
	while (index < pixel_count) {
		imageScaledData[index] = tex2D<unsigned char>(texObj,(float)(tidX * scale_factor),(float)(tidY * scale_factor));
		tidX += coef_x;
		if (tidX >= scaled_width) {
			tidX %= scaled_width;
			tidY += coef_y;
		}
		index = tidY * scaled_width + tidX;
	}
}

int main(int argc, char* argv[])
{ 
	if (argc != 6) {
		fprintf(stderr, "Usage: %s <blocks> <threads per block> <input image> <rescaled output image> <scale>", argv[0]);
		exit(1);
	}

	int blocks, threads_per_block;

	blocks = atoi(argv[1]);
	threads_per_block = atoi(argv[2]);

	int height = 0, width = 0, scaled_height = 0, scaled_width = 0;

	//Define the scaling ratio	
	float scaling_ratio = atof(argv[5]);
	unsigned char*data;
	unsigned char*scaled_data,*d_scaled_data;

	char* inputStr = argv[3];
	char* outputStr = argv[4];
	cudaError_t returnValue;

	//Create a channel Description to be used while linking to the tecture
	cudaArray* cu_array;
	cudaChannelFormatKind kind = cudaChannelFormatKindUnsigned;
	cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc(8, 0, 0, 0, kind);

	get_PgmPpmParams(inputStr, &height, &width);	//getting height and width of the current image
	data = (unsigned char*)malloc(height*width*sizeof(unsigned char));
	// printf("\n Reading image width height and width [%d][%d]", height, width);
	scr_read_pgm( inputStr , data, height, width );//loading an image to "inputimage"

	scaled_height = (int)(height*scaling_ratio);
	scaled_width = (int)(width*scaling_ratio);
	scaled_data = (unsigned char*)malloc(scaled_height*scaled_width*sizeof(unsigned char));
	// printf("\n scaled image width height and width [%d][%d]", scaled_height, scaled_width);

	//Allocate CUDA Array
 	returnValue = cudaMallocArray( &cu_array, &channelDesc, width, height);
	// printf("\n%s", cudaGetErrorString(returnValue));
	returnValue = (cudaError_t)(returnValue | cudaMemcpyToArray( cu_array, 0, 0, data, height * width * sizeof(unsigned char), cudaMemcpyHostToDevice));
	// printf("\n%s", cudaGetErrorString(returnValue));

	if(returnValue != cudaSuccess)
		printf("\n Got error while running CUDA API Array Copy");

	// Step 1. Specify texture
	struct cudaResourceDesc resDesc;
	memset(&resDesc, 0, sizeof(resDesc));
	resDesc.resType = cudaResourceTypeArray;
	resDesc.res.array.array = cu_array;

	// Step 2. Specify texture object parameters
	struct cudaTextureDesc texDesc;
	memset(&texDesc, 0, sizeof(texDesc));
	texDesc.addressMode[0] = cudaAddressModeClamp;
	texDesc.addressMode[1] = cudaAddressModeClamp;
	texDesc.filterMode = cudaFilterModePoint;
	texDesc.readMode = cudaReadModeElementType;
	texDesc.normalizedCoords = 0;

	// Step 3: Create texture object
	cudaTextureObject_t texObj = 0;
	cudaCreateTextureObject(&texObj, &resDesc, &texDesc, NULL);

	if(returnValue != cudaSuccess) 
		printf("\n Got error while running CUDA API Bind Texture");
	
	cudaMalloc(&d_scaled_data, scaled_height*scaled_width*sizeof(unsigned char) );

	dim3 dimBlock(threads_per_block, threads_per_block, 1);
	
	int grid_x = scaled_width / dimBlock.x + 1;
	int grid_y = scaled_height / dimBlock.y + 1;
	// printf("\n%d %d\n", grid_x, grid_y);
	float grid_scale = sqrt(1.f * grid_x * grid_y / blocks);
	// printf("%f\n", grid_scale);

	grid_x = (int) (grid_x / grid_scale);
	grid_y = (int) (grid_y / grid_scale);

	dim3 dimGrid(grid_x, grid_y, 1);
	// printf("\n Launching grid with blocks [%d][%d] ", dimGrid.x, dimGrid.y);

	GpuTimer timer;

	timer.Start();
	createResizedImage<<<dimGrid, dimBlock>>>(d_scaled_data, scaled_width, scaled_height, 1.0 / scaling_ratio, texObj);
	timer.Stop();

	printf("%s %d %d %f %f\n", inputStr, blocks, threads_per_block, timer.Elapsed(), scaling_ratio);

	returnValue = (cudaError_t)(returnValue | cudaDeviceSynchronize());

	returnValue = (cudaError_t)(returnValue |cudaMemcpy (scaled_data , d_scaled_data, scaled_height*scaled_width*sizeof(unsigned char), cudaMemcpyDeviceToHost ));
	if(returnValue != cudaSuccess) 
		printf("\n Got error while running CUDA API kernel");

	// Step 5: Destroy texture object
	cudaDestroyTextureObject(texObj);
	
	scr_write_pgm( outputStr, scaled_data, scaled_height, scaled_width, "####" ); //storing the image with the detections
		
	if(data != NULL)
		free(data);
	if(cu_array !=NULL)
		cudaFreeArray(cu_array);
	if(scaled_data != NULL)
		free(scaled_data);
	if(d_scaled_data!=NULL)
		cudaFree(d_scaled_data);
	
	return 0;
}
