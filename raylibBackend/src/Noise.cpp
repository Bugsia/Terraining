#include "Noise.h"

namespace Noise {
	namespace {
		/*
		* Generates a noise layer image for a terrain element at the given position
		* @param layerSettings The settings for the noise layer
		* @param normalizedPos The position of the terrain, if the spacing would be 1.0f. So basically normalizing the pos against the spacing
		* @param numWidth The number of verticies along the width of the terrain element
		* @param numHeight The number of verticies along the height of the terrain element
		* @param spacing The distance between each vertex
		* @param globalSeed The seed of the noise
		* @return Color* The pixels of the noise layer
		*/
		Color* generateNoiseLayerImage(const noise_layer_settings& layerSettings, Vector3 normalizedPos, int numWidth, int numHeight, float spacing, long globalSeed) {
			float offsetX = normalizedPos.x + layerSettings.offsetX + globalSeed;
			float offsetZ = normalizedPos.z + layerSettings.offsetZ + globalSeed;

			Color* pixels = (Color*)RL_MALLOC(numWidth * numHeight * sizeof(Color));

			float scale = layerSettings.horizontalScale * std::min(numWidth, numHeight) / 20.0f; // Need to offset for bigger elements, so noise stays the same
			float aspectRatio = (float)numWidth / (float)numHeight;

			for (int z = 0; z < numHeight; z++) {
				for (int x = 0; x < numWidth; x++) {
					// float nx = ((float)x * (scale / (float)numWidth)) + offsetX;
					// float nz = ((float)z * (scale / (float)numHeight)) + offsetZ;
					float nx = (float)((x * spacing) + offsetX) * (scale / (float)numWidth);
					float nz = (float)((z * spacing) + offsetZ) * (scale / (float)numHeight);

					// Compensate for non-rectangular noise
					if (numWidth > numHeight) nx *= aspectRatio;
					else nz /= aspectRatio;

					float p = stb_perlin_fbm_noise3(nx, nz, 1.0f, layerSettings.lacunarity, layerSettings.gain, layerSettings.octaves);

					// Clamp between -1.0f and 1.0f
					if (p < -1.0f) p = -1.0f;
					if (p > 1.0f) p = 1.0f;

					// Normalize the data from [-1..1] to [0..1]
					float np = (p + 1.0f) / 2.0f;

					int intensity = (int)(np * 255.0f);
					pixels[z * numWidth + x] = { static_cast<unsigned char>(intensity), static_cast<unsigned char>(intensity), static_cast<unsigned char>(intensity), 255 };
				}
			}

			return pixels;
		}
	} // private namespace

	noise_settings newNoiseSettings() {
		noise_settings noiseSettings;

		noiseSettings.seed = static_cast<long>(GetRandomValue(-999999, 999999));

		TraceLog(LOG_DEBUG, "Noise: New noise settings have been created");

		return noiseSettings;
	}

	noise_layer_settings newNoiseLayerSettings() {
		TraceLog(LOG_DEBUG, "Noise: New noise layer settings have been created");

		return { 1.f, 125.f, 0, 0, 2.f, 0.5f, 6, true };
	}

	void getDefaultNoiseSettings(std::shared_ptr<noise_settings> noiseSettings) {
		std::vector<Noise::noise_layer_settings> noiseLayerSettings(3);
		noiseLayerSettings[0] = { 2.f, 125.f, 0, 0, 2.f, 0.5f, 6, true };
		noiseLayerSettings[1] = { 3.f, 125.f, 0, 0, 2.f, 0.5f, 6, true };
		noiseLayerSettings[2] = { 0.3f, 6.f, 0, 0, 2.f, 0.5f, 6, false };

		noiseSettings->noiseLayerSettings = noiseLayerSettings;

		TraceLog(LOG_DEBUG, "Noise: Default noise settings have been set");
	}

	std::vector<Color*> generateNoiseLayers(std::shared_ptr<noise_settings> noiseSettings, Vector3 normalizedPos, int numWidth, int numHeight, float spacing, long globalSeed) {
		std::vector<Color*> noiseLayers;

		// for (noise_layer_settings& layerSettings : noiseSettings->noiseLayerSettings) {
		// 	noiseLayers.push_back(generateNoiseLayerImage(layerSettings, normalizedPos, numWidth, numHeight, spacing, globalSeed));
		// }

		for (std::vector<noise_layer_settings>::iterator it = noiseSettings->noiseLayerSettings.begin(); it != noiseSettings->noiseLayerSettings.end(); it++) {
			noiseLayers.push_back(generateNoiseLayerImage((*it), normalizedPos, numWidth, numHeight, spacing, globalSeed));
		}

		TraceLog(LOG_DEBUG, "Noise: Noise layers have been generated");

		return noiseLayers;
	}

	float noiseHeight(std::vector<Color*>& noiseLayers, std::vector<noise_layer_settings> layerSettings, int indexX, int indexZ, int imageWidth) {
		float height = 0.0f;
		int index = indexX + indexZ * imageWidth;

		for (int i = 0; i < noiseLayers.size(); i++) {
			float grayValue = noiseLayers[i][index].r;
			if (layerSettings[i].aroundZero) grayValue -= 127.5f;
			height += grayValue / layerSettings[i].verticalScale;
		}

		return height;
	}
}