#pragma once
#include <raylib.h>
#include <vector>
#include <memory>
#include "third_party/stb_perlin.h"

namespace Noise {
	struct noise_settings;
	struct noise_layer_settings;

	struct noise_settings {
		long seed;
		std::vector<noise_layer_settings> noiseLayerSettings;
	};

	struct noise_layer_settings {
		float horizontalScale;
		float verticalScale;

		int offsetX;
		int offsetZ;

		float lacunarity;
		float gain;
		int octaves;

		bool aroundZero; // If true, the noise will be in the range [-x, x], otherwise it will be in the range [0, 2x]
	};

	noise_settings newNoiseSettings();
	noise_layer_settings newNoiseLayerSettings();
	void getDefaultNoiseSettings(std::shared_ptr<noise_settings> noiseSettings);
	std::vector<Color*> generateNoiseLayers(std::shared_ptr<noise_settings> noiseSettings, Vector3 normalizedPos, int numWidth, int numHeight, float spacing, long globalSeed);
	float noiseHeight(std::vector<Color*>& noiseLayers, std::vector<noise_layer_settings> layerSettings, int indexX, int indexZ, int imageWidth);
}