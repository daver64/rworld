#include "rworld/world.h"
#include "../third_party/FastNoiseLite.h"
#include <cmath>
#include <algorithm>

namespace rworld {

// PIMPL implementation to hide FastNoiseLite from the header
class World::Impl {
public:
    WorldConfig config;
    FastNoiseLite terrain_noise;
    FastNoiseLite moisture_noise;
    FastNoiseLite temperature_variation_noise;
    
    explicit Impl(const WorldConfig& cfg) : config(cfg) {
        initialize_noise_generators();
    }
    
    void initialize_noise_generators() {
        // Terrain noise - creates continents, mountains, valleys
        terrain_noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        terrain_noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        terrain_noise.SetFractalOctaves(config.terrain_octaves);
        terrain_noise.SetFractalLacunarity(config.terrain_lacunarity);
        terrain_noise.SetFractalGain(config.terrain_gain);
        terrain_noise.SetFrequency(config.terrain_frequency * config.world_scale);
        terrain_noise.SetSeed(static_cast<int>(config.seed));
        
        // Moisture noise - affects precipitation and biomes
        moisture_noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        moisture_noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        moisture_noise.SetFractalOctaves(config.moisture_octaves);
        moisture_noise.SetFrequency(config.moisture_frequency * config.world_scale);
        moisture_noise.SetSeed(static_cast<int>(config.seed + 1000));
        
        // Temperature variation noise - adds local variations to base temperature
        temperature_variation_noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        temperature_variation_noise.SetFrequency(0.003f * config.world_scale);
        temperature_variation_noise.SetSeed(static_cast<int>(config.seed + 2000));
    }
    
    // Convert geographic coordinates to world space for noise sampling
    void geo_to_world(float longitude, float latitude, float& x, float& y, float& z) const {
        // Convert to radians
        float lon_rad = longitude * 3.14159265359f / 180.0f;
        float lat_rad = latitude * 3.14159265359f / 180.0f;
        
        // Project onto sphere surface for seamless wrapping
        float r = 1000.0f; // Arbitrary sphere radius
        x = r * std::cos(lat_rad) * std::cos(lon_rad);
        y = r * std::cos(lat_rad) * std::sin(lon_rad);
        z = r * std::sin(lat_rad);
    }
    
    float get_terrain_height(float longitude, float latitude) const {
        float x, y, z;
        geo_to_world(longitude, latitude, x, y, z);
        
        // Get base terrain noise (-1 to 1)
        float noise_value = terrain_noise.GetNoise(x, y, z);
        
        // Apply power curve to create more ocean and distinct continents
        // Values below 0 are ocean, above 0 are land
        float shaped = noise_value;
        if (shaped < 0.0f) {
            // Ocean - steeper falloff
            shaped = shaped * shaped * -1.0f;
            shaped = shaped * shaped * -1.0f;
        } else {
            // Land - gentler slope with occasional peaks
            shaped = std::pow(shaped, 0.7f);
        }
        
        // Map to actual height range
        // Ocean: -4000m to 0m, Land: 0m to max_terrain_height
        if (shaped < 0.0f) {
            return shaped * 4000.0f;
        } else {
            return shaped * config.max_terrain_height;
        }
    }
    
    float get_moisture(float longitude, float latitude) const {
        float x, y, z;
        geo_to_world(longitude, latitude, x, y, z);
        
        // Get moisture noise (0 to 1)
        float moisture = moisture_noise.GetNoise(x, y, z);
        moisture = (moisture + 1.0f) * 0.5f; // Convert from -1,1 to 0,1
        
        // Increase moisture near equator, decrease near poles
        float lat_factor = 1.0f - std::abs(latitude) / 90.0f;
        moisture = moisture * 0.7f + lat_factor * 0.3f;
        
        return std::clamp(moisture, 0.0f, 1.0f);
    }
    
    float get_base_temperature(float latitude, float altitude) const {
        // Base temperature from latitude
        float lat_factor = std::abs(latitude) / 90.0f; // 0 at equator, 1 at poles
        float base_temp = config.equator_temperature - 
                         (config.equator_temperature - config.pole_temperature) * lat_factor;
        
        // Adjust for altitude (temperature decreases with height)
        float altitude_adjustment = -(altitude / 1000.0f) * config.temperature_lapse_rate;
        
        return base_temp + altitude_adjustment;
    }
    
    float get_temperature(float longitude, float latitude, float altitude) const {
        float base_temp = get_base_temperature(latitude, altitude);
        
        // Add local variation
        float x, y, z;
        geo_to_world(longitude, latitude, x, y, z);
        float variation = temperature_variation_noise.GetNoise(x, y, z) * 5.0f; // ±5°C variation
        
        return base_temp + variation;
    }
    
    float get_precipitation(float longitude, float latitude, float altitude) const {
        float moisture = get_moisture(longitude, latitude);
        float temp = get_temperature(longitude, latitude, altitude);
        
        // Base precipitation from moisture
        float base_precip = moisture * 2000.0f; // 0 to 2000mm
        
        // Temperature affects precipitation capacity
        // Warmer air holds more moisture
        float temp_factor = std::clamp((temp + 10.0f) / 40.0f, 0.1f, 1.5f);
        base_precip *= temp_factor;
        
        // Altitude effect - mountains capture moisture (orographic precipitation)
        // But very high altitudes are dry
        float terrain_height = get_terrain_height(longitude, latitude);
        if (terrain_height > 500.0f && terrain_height < 3000.0f) {
            base_precip *= 1.3f; // Mountain slopes get more rain
        } else if (altitude > 4000.0f) {
            base_precip *= 0.5f; // Very high altitudes are dry
        }
        
        return std::clamp(base_precip, 0.0f, 4000.0f);
    }
    
    float get_air_pressure(float altitude) const {
        // Standard atmospheric pressure model
        // P = P0 * exp(-altitude / scale_height)
        const float P0 = 1013.25f; // Sea level pressure in millibars
        const float scale_height = 8500.0f; // Atmospheric scale height in meters
        
        return P0 * std::exp(-altitude / scale_height);
    }
    
    float get_humidity(float longitude, float latitude, float altitude) const {
        float moisture = get_moisture(longitude, latitude);
        float temp = get_temperature(longitude, latitude, altitude);
        
        // Relative humidity is affected by temperature
        // Cold air has higher relative humidity for same absolute moisture
        float temp_factor = 1.0f - std::clamp((temp - 10.0f) / 40.0f, 0.0f, 0.5f);
        
        float humidity = moisture * (0.5f + temp_factor);
        
        // Very high altitudes are dry
        if (altitude > 3000.0f) {
            humidity *= std::clamp(1.0f - (altitude - 3000.0f) / 5000.0f, 0.2f, 1.0f);
        }
        
        return std::clamp(humidity, 0.0f, 1.0f);
    }
    
    BiomeType classify_biome(float longitude, float latitude, float altitude) const {
        float terrain_height = get_terrain_height(longitude, latitude);
        
        // Ocean biomes
        if (terrain_height < config.sea_level) {
            if (terrain_height < -1000.0f) {
                return BiomeType::DEEP_OCEAN;
            }
            return BiomeType::OCEAN;
        }
        
        // Beach transition
        if (terrain_height < 5.0f) {
            return BiomeType::BEACH;
        }
        
        float temp = get_temperature(longitude, latitude, altitude);
        float moisture = get_moisture(longitude, latitude);
        
        // Snow and ice
        if (temp < -15.0f) {
            if (terrain_height < 100.0f) {
                return BiomeType::ICE;
            }
            return BiomeType::SNOW;
        }
        
        // High mountain biomes
        if (altitude > 4000.0f) {
            return BiomeType::MOUNTAIN_PEAK;
        } else if (altitude > 2500.0f) {
            if (temp < 0.0f) {
                return BiomeType::MOUNTAIN_TUNDRA;
            }
            return BiomeType::MOUNTAIN_FOREST;
        }
        
        // Whittaker diagram classification
        // Based on temperature and moisture
        
        // Cold (< 0°C)
        if (temp < 0.0f) {
            if (moisture < 0.3f) {
                return BiomeType::COLD_DESERT;
            }
            return BiomeType::TUNDRA;
        }
        
        // Cool (0-10°C)
        if (temp < 10.0f) {
            if (moisture < 0.3f) {
                return BiomeType::COLD_DESERT;
            } else if (moisture < 0.6f) {
                return BiomeType::GRASSLAND;
            }
            return BiomeType::TAIGA;
        }
        
        // Temperate (10-20°C)
        if (temp < 20.0f) {
            if (moisture < 0.3f) {
                return BiomeType::GRASSLAND;
            } else if (moisture < 0.6f) {
                return BiomeType::TEMPERATE_DECIDUOUS_FOREST;
            }
            return BiomeType::TEMPERATE_RAINFOREST;
        }
        
        // Hot (> 20°C)
        if (moisture < 0.2f) {
            return BiomeType::DESERT;
        } else if (moisture < 0.5f) {
            return BiomeType::SAVANNA;
        } else if (moisture < 0.7f) {
            return BiomeType::TROPICAL_SEASONAL_FOREST;
        }
        return BiomeType::TROPICAL_RAINFOREST;
    }
};

// World implementation

World::World() : pimpl_(std::make_unique<Impl>(WorldConfig{})) {}

World::World(const WorldConfig& config) : pimpl_(std::make_unique<Impl>(config)) {}

World::~World() = default;

World::World(World&&) noexcept = default;
World& World::operator=(World&&) noexcept = default;

BiomeType World::get_biome(float longitude, float latitude, float altitude) const {
    return pimpl_->classify_biome(longitude, latitude, altitude);
}

float World::get_temperature(float longitude, float latitude, float altitude) const {
    return pimpl_->get_temperature(longitude, latitude, altitude);
}

float World::get_terrain_height(float longitude, float latitude) const {
    return pimpl_->get_terrain_height(longitude, latitude);
}

float World::get_precipitation(float longitude, float latitude, float altitude) const {
    return pimpl_->get_precipitation(longitude, latitude, altitude);
}

PrecipitationType World::get_precipitation_type(float longitude, float latitude, float altitude) const {
    float temp = get_temperature(longitude, latitude, altitude);
    float precip = get_precipitation(longitude, latitude, altitude);
    
    if (precip < 100.0f) {
        return PrecipitationType::NONE;
    }
    
    if (temp < -2.0f) {
        return PrecipitationType::SNOW;
    } else if (temp < 2.0f) {
        return PrecipitationType::SLEET;
    }
    return PrecipitationType::RAIN;
}

float World::get_air_pressure(float longitude, float latitude, float altitude) const {
    (void)longitude; // Unused - pressure mainly depends on altitude
    (void)latitude;
    return pimpl_->get_air_pressure(altitude);
}

float World::get_humidity(float longitude, float latitude, float altitude) const {
    return pimpl_->get_humidity(longitude, latitude, altitude);
}

void World::set_config(const WorldConfig& config) {
    pimpl_->config = config;
    pimpl_->initialize_noise_generators();
}

const WorldConfig& World::get_config() const {
    return pimpl_->config;
}

const char* biome_to_string(BiomeType biome) {
    switch (biome) {
        case BiomeType::TUNDRA: return "Tundra";
        case BiomeType::TAIGA: return "Taiga";
        case BiomeType::GRASSLAND: return "Grassland";
        case BiomeType::TEMPERATE_DECIDUOUS_FOREST: return "Temperate Deciduous Forest";
        case BiomeType::TEMPERATE_RAINFOREST: return "Temperate Rainforest";
        case BiomeType::SAVANNA: return "Savanna";
        case BiomeType::TROPICAL_SEASONAL_FOREST: return "Tropical Seasonal Forest";
        case BiomeType::TROPICAL_RAINFOREST: return "Tropical Rainforest";
        case BiomeType::COLD_DESERT: return "Cold Desert";
        case BiomeType::DESERT: return "Desert";
        case BiomeType::OCEAN: return "Ocean";
        case BiomeType::DEEP_OCEAN: return "Deep Ocean";
        case BiomeType::BEACH: return "Beach";
        case BiomeType::SNOW: return "Snow";
        case BiomeType::ICE: return "Ice";
        case BiomeType::MOUNTAIN_TUNDRA: return "Mountain Tundra";
        case BiomeType::MOUNTAIN_FOREST: return "Mountain Forest";
        case BiomeType::MOUNTAIN_PEAK: return "Mountain Peak";
        default: return "Unknown";
    }
}

} // namespace rworld
