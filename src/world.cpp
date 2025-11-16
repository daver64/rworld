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
    FastNoiseLite wind_noise;
    FastNoiseLite river_noise;
    FastNoiseLite volcano_noise;
    FastNoiseLite coal_noise;
    FastNoiseLite iron_noise;
    FastNoiseLite oil_noise;
    FastNoiseLite cloud_noise;
    
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
        
        // Wind noise - creates wind patterns
        wind_noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        wind_noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        wind_noise.SetFractalOctaves(2);
        wind_noise.SetFrequency(0.002f * config.world_scale);
        wind_noise.SetSeed(static_cast<int>(config.seed + 3000));
        
        // River noise - adds variation to river placement
        river_noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        river_noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        river_noise.SetFractalOctaves(3);
        river_noise.SetFrequency(0.004f * config.world_scale);
        river_noise.SetSeed(static_cast<int>(config.seed + 4000));
        
        // Volcano noise - sparse cellular noise for volcano placement
        volcano_noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
        volcano_noise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Euclidean);
        volcano_noise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance);
        volcano_noise.SetFrequency(0.008f * config.world_scale);
        volcano_noise.SetSeed(static_cast<int>(config.seed + 5000));
        
        // Coal noise - for coal deposits (sedimentary, lowland swamps)
        coal_noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        coal_noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        coal_noise.SetFractalOctaves(4);
        coal_noise.SetFrequency(0.003f * config.world_scale);
        coal_noise.SetSeed(static_cast<int>(config.seed + 6000));
        
        // Iron noise - for iron ore deposits (volcanic/ancient seabeds)
        iron_noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        iron_noise.SetFractalType(FastNoiseLite::FractalType_Ridged);
        iron_noise.SetFractalOctaves(3);
        iron_noise.SetFrequency(0.004f * config.world_scale);
        iron_noise.SetSeed(static_cast<int>(config.seed + 7000));
        
        // Oil noise - for oil deposits (sedimentary basins)
        oil_noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
        oil_noise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Euclidean);
        oil_noise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2);
        oil_noise.SetFrequency(0.003f * config.world_scale);
        oil_noise.SetSeed(static_cast<int>(config.seed + 4000));
        
        // Cloud noise - for atmospheric modeling
        cloud_noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        cloud_noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        cloud_noise.SetFractalOctaves(3);
        cloud_noise.SetFrequency(0.005f * config.world_scale);
        cloud_noise.SetSeed(static_cast<int>(config.seed + 5000));
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
    
    float get_terrain_height(float longitude, float latitude, float detail_level = 1.0f) const {
        float x, y, z;
        geo_to_world(longitude, latitude, x, y, z);
        
        // Get base terrain noise (-1 to 1)
        float noise_value = terrain_noise.GetNoise(x, y, z);
        
        // Add finer detail layers when detail_level > 1.0
        if (detail_level > 1.0f) {
            // Add progressively finer octaves of detail
            float detail_contribution = 0.0f;
            float detail_amplitude = 0.3f; // Strength of detail layers
            float detail_frequency = 2.0f;
            
            // Add up to 3 detail octaves based on zoom level
            int detail_octaves = std::min(3, static_cast<int>(std::log2(detail_level)));
            
            for (int i = 0; i < detail_octaves; ++i) {
                float freq = detail_frequency * std::pow(2.0f, i);
                detail_contribution += terrain_noise.GetNoise(x * freq, y * freq, z * freq) * detail_amplitude;
                detail_amplitude *= 0.5f; // Each octave contributes less
            }
            
            // Blend detail based on zoom level
            float detail_blend = std::min(1.0f, (detail_level - 1.0f) / 4.0f);
            noise_value = noise_value * (1.0f - detail_blend * 0.3f) + detail_contribution * detail_blend;
        }
        
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
        float base_height = 0.0f;
        if (shaped < 0.0f) {
            base_height = shaped * 4000.0f;
        } else {
            base_height = shaped * config.max_terrain_height;
        }
        
        // Add volcanoes - only on land (independent of detail_level so always visible)
        if (base_height > 0.0f) {
            // Use cellular noise to find volcano centers
            float volcano_cell = volcano_noise.GetNoise(x, y, z);
            volcano_cell = (volcano_cell + 1.0f) * 0.5f; // Convert to 0-1
            
            // Only place volcanoes where cellular noise is very low (cell centers)
            if (volcano_cell < 0.2f) {
                // Calculate distance from volcano center
                // Lower cell value = closer to center
                float distance_factor = 1.0f - (volcano_cell / 0.2f);
                
                // Volcano height: cone shape with steep sides
                // Prefer higher elevations for volcanoes but can appear anywhere on land
                float elevation_preference = std::clamp((base_height - 300.0f) / 1500.0f, 0.2f, 1.0f);
                
                // Create cone shape: starts high at center, drops off with distance
                // Make them taller and more prominent
                float cone_height = distance_factor * distance_factor * distance_factor * 3000.0f; // Up to 3000m tall, steeper sides
                cone_height *= elevation_preference; 
                
                // Add a crater dip at the very center
                if (distance_factor > 0.85f) {
                    float crater_factor = (distance_factor - 0.85f) / 0.15f;
                    cone_height *= 1.0f - crater_factor * 0.4f; // 40% dip for crater
                }
                
                base_height += cone_height;
            }
        }
        
        return base_height;
    }
    
    bool is_volcano(float longitude, float latitude) const {
        float base_height = get_terrain_height(longitude, latitude);
        
        // No volcanoes in ocean
        if (base_height <= config.sea_level) {
            return false;
        }
        
        float x, y, z;
        geo_to_world(longitude, latitude, x, y, z);
        
        // Check volcano noise
        float volcano_cell = volcano_noise.GetNoise(x, y, z);
        volcano_cell = (volcano_cell + 1.0f) * 0.5f;
        
        // Location is a volcano if within the cone radius
        return volcano_cell < 0.2f;
    }
    
    float get_coal_deposit(float longitude, float latitude) const {
        float terrain_height = get_terrain_height(longitude, latitude);
        
        // No coal in ocean or very high mountains
        if (terrain_height <= config.sea_level || terrain_height > 2000.0f) {
            return 0.0f;
        }
        
        float x, y, z;
        geo_to_world(longitude, latitude, x, y, z);
        
        // Coal forms in ancient swamps - prefer wet, vegetated lowlands
        float coal_noise_value = coal_noise.GetNoise(x, y, z);
        coal_noise_value = (coal_noise_value + 1.0f) * 0.5f; // 0-1
        
        // Coal more likely in areas with:
        // - Moderate elevation (50-1200m - ancient swamps and forests)
        float elevation_factor = 0.0f;
        if (terrain_height >= 50.0f && terrain_height <= 1200.0f) {
            elevation_factor = 1.0f - std::abs(terrain_height - 500.0f) / 700.0f;
            elevation_factor = std::max(0.3f, elevation_factor); // Minimum factor of 0.3
        }
        
        // - High historical precipitation (ancient forests/swamps)
        float altitude = std::max(terrain_height, 0.0f);
        float precip = get_precipitation(longitude, latitude, altitude);
        float moisture_factor = std::clamp(precip / 1500.0f, 0.2f, 1.0f); // Minimum 0.2
        
        // - Temperate to tropical latitudes
        float lat_factor = 1.0f - std::abs(latitude) / 90.0f; // Higher at equator
        
        // Boost coal concentration overall
        float coal = coal_noise_value * elevation_factor * moisture_factor * (0.5f + lat_factor * 0.5f);
        coal = std::pow(coal, 0.7f); // Make deposits more concentrated
        return std::clamp(coal * 1.3f, 0.0f, 1.0f);
    }
    
    float get_iron_deposit(float longitude, float latitude) const {
        float terrain_height = get_terrain_height(longitude, latitude);
        
        // Iron can be anywhere on land
        if (terrain_height <= config.sea_level) {
            return 0.0f;
        }
        
        float x, y, z;
        geo_to_world(longitude, latitude, x, y, z);
        
        // Iron forms in banded iron formations and volcanic regions
        float iron_noise_value = iron_noise.GetNoise(x, y, z);
        iron_noise_value = (iron_noise_value + 1.0f) * 0.5f; // 0-1
        
        // Iron more likely in:
        // - Volcanic regions (smaller bonus)
        float volcano_bonus = is_volcano(longitude, latitude) ? 0.25f : 0.0f;
        
        // - Ancient seabeds (low-mid elevation)
        float elevation_factor = 0.3f;
        if (terrain_height < 500.0f) {
            elevation_factor = 0.8f;
        } else if (terrain_height < 1000.0f) {
            elevation_factor = 0.6f;
        }
        
        // - Areas with geological activity (use ridged noise pattern)
        // Square the noise to make deposits more rare and concentrated
        float iron = (iron_noise_value * iron_noise_value) * elevation_factor + volcano_bonus;
        return std::clamp(iron * 0.8f, 0.0f, 1.0f);
    }
    
    float get_oil_deposit(float longitude, float latitude) const {
        float terrain_height = get_terrain_height(longitude, latitude);
        
        // Oil forms in sedimentary basins - prefer low elevations and ancient ocean margins
        // Can be on land or shallow ocean
        if (terrain_height < -1000.0f || terrain_height > 1200.0f) {
            return 0.0f;
        }
        
        float x, y, z;
        geo_to_world(longitude, latitude, x, y, z);
        
        // Oil deposits using cellular pattern (basin-like structures)
        float oil_noise_value = oil_noise.GetNoise(x, y, z);
        oil_noise_value = (oil_noise_value + 1.0f) * 0.5f; // 0-1
        
        // Oil more likely in:
        // - Sedimentary basins (valleys, lowlands, coastal areas)
        float elevation_factor = 0.0f;
        if (terrain_height >= -200.0f && terrain_height <= 800.0f) {
            elevation_factor = 1.0f - std::abs(terrain_height - 200.0f) / 600.0f;
            elevation_factor = std::max(0.3f, elevation_factor); // Minimum factor
        }
        
        // - Areas that were ancient ocean (coastal/low areas)
        float coastal_factor = terrain_height < 400.0f ? 1.3f : 0.9f;
        
        // - Cellular pattern creates basin-like deposits
        oil_noise_value = std::pow(oil_noise_value, 1.2f); // Make deposits somewhat concentrated but not too rare
        
        float oil = oil_noise_value * elevation_factor * coastal_factor;
        return std::clamp(oil * 1.4f, 0.0f, 1.0f);
    }
    
    float get_solar_angle(float longitude, float latitude, float current_time) const {
        // Calculate hour angle: how far the sun has moved from solar noon
        // Solar noon is at 12:00 at longitude 0°
        // Earth rotates 360° in 24 hours = 15° per hour
        float local_solar_time = current_time + (longitude / 15.0f);
        
        // Wrap to 0-24 range
        while (local_solar_time < 0.0f) local_solar_time += 24.0f;
        while (local_solar_time >= 24.0f) local_solar_time -= 24.0f;
        
        // Hour angle: 0° at solar noon (12:00), ±15° per hour
        float hour_angle = (local_solar_time - 12.0f) * 15.0f; // degrees
        
        // Solar declination (axial tilt effect - simplified to equinox, 0° for now)
        // Could add seasonal variation based on day of year
        float solar_declination = 0.0f; // degrees
        
        // Convert to radians for trig
        float lat_rad = latitude * M_PI / 180.0f;
        float dec_rad = solar_declination * M_PI / 180.0f;
        float ha_rad = hour_angle * M_PI / 180.0f;
        
        // Solar elevation angle formula
        float sin_elevation = std::sin(lat_rad) * std::sin(dec_rad) +
                             std::cos(lat_rad) * std::cos(dec_rad) * std::cos(ha_rad);
        
        float elevation_angle = std::asin(std::clamp(sin_elevation, -1.0f, 1.0f)) * 180.0f / M_PI;
        
        return elevation_angle; // degrees above horizon (negative = below)
    }
    
    bool is_daylight(float longitude, float latitude, float current_time) const {
        return get_solar_angle(longitude, latitude, current_time) > 0.0f;
    }
    
    float get_insolation(float longitude, float latitude, float current_time) const {
        // Get solar angle
        float solar_angle = get_solar_angle(longitude, latitude, current_time);
        
        // No insolation if sun is below horizon
        if (solar_angle <= 0.0f) {
            return 0.0f;
        }
        
        // Solar constant at top of atmosphere
        const float SOLAR_CONSTANT = 1361.0f; // W/m²
        
        // Insolation based on solar angle (cosine law)
        float solar_angle_rad = solar_angle * M_PI / 180.0f;
        float base_insolation = SOLAR_CONSTANT * std::sin(solar_angle_rad);
        
        // Atmospheric attenuation (simplified - depends on air mass)
        // Air mass increases as sun gets lower in sky
        float air_mass = 1.0f / std::sin(solar_angle_rad);
        air_mass = std::clamp(air_mass, 1.0f, 10.0f);
        
        // Atmospheric transmission (simple exponential model)
        float atmospheric_transmission = std::pow(0.7f, air_mass);
        base_insolation *= atmospheric_transmission;
        
        // Cloud cover reduces insolation
        float terrain_height = get_terrain_height(longitude, latitude);
        float altitude = std::max(terrain_height, 0.0f);
        float cloud_density = get_cloud_density(longitude, latitude, altitude);
        
        // Clouds block 50-90% of radiation depending on density
        float cloud_factor = 1.0f - (cloud_density * 0.7f);
        
        float final_insolation = base_insolation * cloud_factor;
        
        return std::clamp(final_insolation, 0.0f, 1400.0f);
    }
    
    float get_cloud_density(float longitude, float latitude, float altitude) const {
        float x, y, z;
        geo_to_world(longitude, latitude, x, y, z);
        
        // Get base noise pattern for cloud variation
        float noise = cloud_noise.GetNoise(x, y, z);
        noise = (noise + 1.0f) * 0.5f; // 0-1
        
        // Get environmental factors
        float humidity = get_humidity(longitude, latitude, altitude);
        float precip = get_precipitation(longitude, latitude, altitude);
        float temp = get_temperature(longitude, latitude, altitude);
        
        // Cloud density is heavily influenced by humidity
        float cloud_base = humidity * 0.8f + 0.2f * noise;
        
        // High precipitation areas have more clouds
        float precip_factor = std::clamp(precip / 2000.0f, 0.0f, 1.0f);
        cloud_base = cloud_base * 0.6f + precip_factor * 0.4f;
        
        // Temperature affects cloud formation
        // Cold air holds less moisture, hot air more
        float temp_factor = 1.0f;
        if (temp < -10.0f) {
            temp_factor = 0.5f; // Very cold - less clouds
        } else if (temp > 25.0f) {
            temp_factor = 1.2f; // Hot tropical - more clouds
        }
        
        float cloud_density = cloud_base * temp_factor;
        return std::clamp(cloud_density, 0.0f, 1.0f);
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
    
    float get_wind_speed(float longitude, float latitude, float altitude) const {
        float x, y, z;
        geo_to_world(longitude, latitude, x, y, z);
        
        // Base wind from noise
        float wind_base = wind_noise.GetNoise(x, y, z);
        wind_base = (wind_base + 1.0f) * 0.5f; // Convert to 0-1
        
        // Global wind patterns based on latitude
        float abs_lat = std::abs(latitude);
        float lat_wind = 0.0f;
        
        // Trade winds (0-30°), westerlies (30-60°), polar easterlies (60-90°)
        if (abs_lat < 30.0f) {
            lat_wind = 5.0f + (30.0f - abs_lat) / 30.0f * 3.0f; // 5-8 m/s
        } else if (abs_lat < 60.0f) {
            lat_wind = 7.0f + (abs_lat - 30.0f) / 30.0f * 5.0f; // 7-12 m/s
        } else {
            lat_wind = 6.0f + (90.0f - abs_lat) / 30.0f * 2.0f; // 6-8 m/s
        }
        
        // Terrain effect - mountains and elevation reduce wind at surface
        float terrain_height = get_terrain_height(longitude, latitude);
        float terrain_factor = 1.0f;
        if (altitude <= std::max(terrain_height, 0.0f) + 10.0f) {
            // Near surface, terrain roughness matters
            if (terrain_height > 1000.0f) {
                terrain_factor = 0.6f; // Mountains reduce wind
            } else if (terrain_height > 500.0f) {
                terrain_factor = 0.8f; // Hills reduce wind somewhat
            }
        } else {
            // Higher altitude = stronger winds
            terrain_factor = 1.0f + (altitude - terrain_height) / 5000.0f;
        }
        
        // Combine factors
        float wind_speed = (lat_wind * 0.6f + wind_base * 8.0f * 0.4f) * terrain_factor;
        
        return std::clamp(wind_speed, 0.0f, 30.0f);
    }
    
    float get_wind_direction(float longitude, float latitude, float altitude) const {
        float x, y, z;
        geo_to_world(longitude, latitude, x, y, z);
        
        // Global wind patterns
        float abs_lat = std::abs(latitude);
        float base_direction = 0.0f;
        
        // Trade winds (easterlies 0-30°), westerlies (30-60°), polar easterlies (60-90°)
        if (abs_lat < 30.0f) {
            // Trade winds blow from east to west (270° = westward)
            base_direction = latitude >= 0.0f ? 240.0f : 120.0f; // NE in north, SE in south
        } else if (abs_lat < 60.0f) {
            // Westerlies blow from west to east (90° = eastward)
            base_direction = latitude >= 0.0f ? 60.0f : 300.0f; // SW in north, NW in south
        } else {
            // Polar easterlies
            base_direction = latitude >= 0.0f ? 120.0f : 240.0f; // NE in north, SE in south
        }
        
        // Add local variation from noise
        float noise_offset = wind_noise.GetNoise(x * 2.0f, y * 2.0f, z * 2.0f) * 60.0f;
        base_direction += noise_offset;
        
        // Normalize to 0-360
        while (base_direction < 0.0f) base_direction += 360.0f;
        while (base_direction >= 360.0f) base_direction -= 360.0f;
        
        return base_direction;
    }
    
    // Calculate flow accumulation based on terrain gradient
    float get_flow_accumulation(float longitude, float latitude) const {
        float terrain_height = get_terrain_height(longitude, latitude);
        
        // No rivers in ocean or underwater
        if (terrain_height <= config.sea_level) {
            return 0.0f;
        }
        
        // Sample nearby elevations to estimate gradient and flow direction
        const float sample_dist = 0.1f; // degrees
        float h_north = get_terrain_height(longitude, latitude + sample_dist);
        float h_south = get_terrain_height(longitude, latitude - sample_dist);
        float h_east = get_terrain_height(longitude + sample_dist, latitude);
        float h_west = get_terrain_height(longitude - sample_dist, latitude);
        
        // Calculate how much this location is a "sink" (lower than surroundings)
        float avg_neighbor = (h_north + h_south + h_east + h_west) / 4.0f;
        float height_diff = avg_neighbor - terrain_height;
        
        // Positive height_diff means we're in a valley/channel
        float valley_factor = std::clamp(height_diff / 50.0f, 0.0f, 1.0f);
        
        // Calculate gradient magnitude - steeper = more defined channels
        float grad_ns = std::abs(h_north - h_south) / (2.0f * sample_dist);
        float grad_ew = std::abs(h_east - h_west) / (2.0f * sample_dist);
        float gradient = std::sqrt(grad_ns * grad_ns + grad_ew * grad_ew);
        float gradient_factor = std::clamp(gradient / 500.0f, 0.2f, 1.5f);
        
        // Get precipitation - more water = more flow
        float altitude = std::max(terrain_height, 0.0f);
        float precip = get_precipitation(longitude, latitude, altitude);
        float precip_factor = std::clamp(precip / 1500.0f, 0.1f, 1.5f);
        
        // Add noise variation for natural-looking river networks
        float x, y, z;
        geo_to_world(longitude, latitude, x, y, z);
        float noise = river_noise.GetNoise(x, y, z);
        noise = (noise + 1.0f) * 0.5f; // 0-1
        
        // Boost noise influence to create more rivers
        float noise_factor = noise * noise; // Square for more variation
        
        // Combine factors: valleys + precipitation + noise + gradient
        float flow = valley_factor * 0.4f + precip_factor * 0.25f + noise_factor * 0.35f;
        flow *= gradient_factor;
        
        // Elevation influence - rivers more common at mid-elevations
        if (terrain_height < 100.0f) {
            flow *= 2.0f; // Near sea level - wide river plains and deltas
        } else if (terrain_height < 500.0f) {
            flow *= 1.3f; // Low elevation - good for rivers
        } else if (terrain_height > 3000.0f) {
            flow *= 0.4f; // High mountains - fewer/smaller rivers
        }
        
        return std::clamp(flow, 0.0f, 1.0f);
    }
    
    bool is_river(float longitude, float latitude) const {
        float terrain_height = get_terrain_height(longitude, latitude);
        
        // No rivers in ocean
        if (terrain_height <= config.sea_level) {
            return false;
        }
        
        float flow = get_flow_accumulation(longitude, latitude);
        // Lower threshold for river presence to show more rivers
        return flow > 0.4f;
    }
    
    float get_river_width(float longitude, float latitude) const {
        float terrain_height = get_terrain_height(longitude, latitude);
        
        // No rivers in ocean
        if (terrain_height <= config.sea_level) {
            return 0.0f;
        }
        
        float flow = get_flow_accumulation(longitude, latitude);
        
        if (flow < 0.4f) {
            return 0.0f; // No river
        }
        
        // Width increases with flow accumulation
        float base_width = (flow - 0.4f) / 0.6f; // 0-1 for flow 0.4-1.0
        
        // Use terrain height for elevation influence (already have it)
        float altitude = std::max(terrain_height, 0.0f);
        
        // Rivers get wider at lower elevations (approaching sea)
        float elevation_factor = 1.0f;
        if (terrain_height < 500.0f) {
            elevation_factor = 2.0f + (500.0f - terrain_height) / 500.0f * 3.0f; // 2-5x wider
        }
        
        // Get precipitation for flow volume
        float precip = get_precipitation(longitude, latitude, altitude);
        float precip_factor = 0.5f + std::clamp(precip / 2000.0f, 0.0f, 1.0f) * 0.5f; // 0.5-1.0
        
        // Calculate width: 5-200 meters
        float width = 5.0f + base_width * base_width * 40.0f * elevation_factor * precip_factor;
        
        return std::clamp(width, 0.0f, 500.0f);
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

float World::get_terrain_height(float longitude, float latitude, float detail_level) const {
    return pimpl_->get_terrain_height(longitude, latitude, detail_level);
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

float World::get_wind_speed(float longitude, float latitude, float altitude) const {
    return pimpl_->get_wind_speed(longitude, latitude, altitude);
}

float World::get_wind_direction(float longitude, float latitude, float altitude) const {
    return pimpl_->get_wind_direction(longitude, latitude, altitude);
}

bool World::is_river(float longitude, float latitude) const {
    return pimpl_->is_river(longitude, latitude);
}

float World::get_river_width(float longitude, float latitude) const {
    return pimpl_->get_river_width(longitude, latitude);
}

float World::get_flow_accumulation(float longitude, float latitude) const {
    return pimpl_->get_flow_accumulation(longitude, latitude);
}

bool World::is_volcano(float longitude, float latitude) const {
    return pimpl_->is_volcano(longitude, latitude);
}

float World::get_coal_deposit(float longitude, float latitude) const {
    return pimpl_->get_coal_deposit(longitude, latitude);
}

float World::get_iron_deposit(float longitude, float latitude) const {
    return pimpl_->get_iron_deposit(longitude, latitude);
}

float World::get_oil_deposit(float longitude, float latitude) const {
    return pimpl_->get_oil_deposit(longitude, latitude);
}

float World::get_insolation(float longitude, float latitude, float current_time) const {
    return pimpl_->get_insolation(longitude, latitude, current_time);
}

bool World::is_daylight(float longitude, float latitude, float current_time) const {
    return pimpl_->is_daylight(longitude, latitude, current_time);
}

float World::get_solar_angle(float longitude, float latitude, float current_time) const {
    return pimpl_->get_solar_angle(longitude, latitude, current_time);
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
