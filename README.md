# RWorld - Living Active World Library

A header-only C++ library for procedural generation of living world environments with environmental parameters including terrain, biomes, temperature, precipitation, and atmospheric conditions.

## Features

- **Procedural Terrain Generation**: Continents, oceans, mountains, valleys, and volcanic features
- **Climate Simulation**: Temperature based on latitude, altitude, and season with realistic gradients
- **Biome Classification**: 18+ biome types using Whittaker diagram approach
- **Atmospheric Physics**: Air pressure, humidity, cloud cover with weather-linked density
- **Storm Systems**: Pressure gradients, high/low pressure centers, and storm fronts
- **Weather Systems**: Precipitation, wind speed/direction, precipitation type (rain/snow/sleet)
- **Hydrology**: River networks with flow accumulation and width based on drainage
- **Day/Night Cycle**: Realistic solar angles with time-of-day and seasonal variations
- **Insolation Modeling**: Solar radiation affected by latitude, time, cloud cover, and season
- **Vegetation Density**: Realistic plant coverage based on biome, climate, and altitude
- **Soil System**: 8 soil types with fertility, pH, and organic matter content
- **Resource Distribution**: Coal, iron ore, and oil deposits with geological realism
- **Deterministic Generation**: Same seed produces identical worlds
- **Header-Only Library**: Easy integration - just include the header, no linking required
- **Clean API**: Simple coordinate-based queries (longitude, latitude, altitude, time)
- **Batch Query API**: High-performance bulk queries (10-100x faster for region generation)
- **Interactive Visualization**: SDL2-based graphical demo with 13 display modes

## SDL2 Visualization Features

The demo application includes an interactive graphical interface when built with SDL2:

- **Multiple Display Modes**:
  - Biomes (key 1) - Color-coded biome classification
  - Elevation (key 2) - Terrain height visualization
  - Temperature (key 3) - Heat map of world temperatures
  - Precipitation (key 4) - Rainfall and snowfall distribution
  - Clouds (key 5) - Cloud cover linked to weather
  - Rivers (key 6) - River networks with flow visualization
  - Coal (key 7) - Coal deposit concentrations
  - Iron (key 8) - Iron ore deposit locations
  - Oil (key 9) - Oil field distributions
  - Insolation (key 0) - Day/night cycle and solar radiation
  - Vegetation (key V) - Plant density and coverage

- **Interactive Controls**:
  - `1-9, 0, V, F, P` keys: Switch between display modes
  - `I` key: Toggle info panel (OFF by default to maximize viewing area)
  - `Mouse Wheel`: Zoom in/out at cursor position (adaptive terrain detail)
  - `SPACE`: Pause/resume time progression
  - `+ / -`: Increase/decrease time speed (0.1x to 100x)
  - `[ / ]`: Decrease/increase time of day (±30 minutes)
  - `< / >` (Shift+comma/period): Change season (±30 days)
  - `R` key: Regenerate world with random seed
  - `ESC/Q`: Quit application

- **Information Panel** (toggle with `I` key): Displays comprehensive environmental data including:
  - Position (longitude, latitude), elevation, biome type
  - Temperature (current, dynamic with time of day)
  - Precipitation, humidity, cloud cover
  - Wind speed and direction (current, time-varying)
  - River information (width, flow)
  - Mineral deposits (coal, iron, oil)
  - Soil type, fertility, and pH
  - Atmospheric pressure and storm fronts
  - Time of day (0-24 hours) and time speed multiplier
  - Current season (day of year)
  - Solar data (insolation, sun angle)
  - Vegetation density

## Quick Start

### Building the Demo

```bash
# Install SDL2 for graphical visualization (optional but recommended)
sudo apt-get install libsdl2-dev libsdl2-ttf-dev  # Ubuntu/Debian
# or: brew install sdl2 sdl2_ttf                  # macOS

# Build the demo application
mkdir build && cd build
cmake ..
cmake --build .
```

**Note**: The library itself is header-only and requires no compilation. The build step above is only for the demo application.

### Running the Demo

```bash
./world_demo
```

The demo includes:
- **With SDL2 + SDL2_ttf**: Interactive 1200x600 window with colourful visualization and text info
- **With SDL2 only**: Graphical visualisation with colour-coded indicators (no text)
- **Without SDL2**: Text-based output with ASCII map and data tables

## Usage Examples

### Basic World Query

```cpp
// Define this in exactly ONE .cpp file before including the header
#define _RWORLD_IMPLEMENTATION
#include "rworld/world.h"

#include <iostream>

int main() {
    // Create a world with a specific seed
    rworld::WorldConfig config;
    config.seed = 12345;
    rworld::World world(config);
    
    // Query environmental parameters at a location
    float lon = 0.0f, lat = 45.0f;  // Greenwich meridian, mid-latitude
    float altitude = world.get_terrain_height(lon, lat, 1.0f);
    
    std::cout << "Location: " << lon << "°, " << lat << "°\n";
    std::cout << "Elevation: " << altitude << " m\n";
    
    // Get climate data
    float temperature = world.get_temperature(lon, lat, altitude);
    float precipitation = world.get_precipitation(lon, lat, altitude);
    rworld::BiomeType biome = world.get_biome(lon, lat, altitude);
    
    std::cout << "Temperature: " << temperature << " °C\n";
    std::cout << "Precipitation: " << precipitation << " mm/year\n";
    std::cout << "Biome: " << static_cast<int>(biome) << "\n";
    
    return 0;
}
```

### Time-Based Simulation

```cpp
#include "rworld/world.h"
#include <iostream>

int main() {
    rworld::WorldConfig config;
    config.seed = 42;
    config.day_of_year = 172;  // June 21 (summer solstice)
    rworld::World world(config);
    
    float lon = -75.0f, lat = 40.0f;  // New York area
    float altitude = world.get_terrain_height(lon, lat, 1.0f);
    
    // Simulate a day's temperature cycle
    std::cout << "24-hour temperature cycle:\n";
    for (float hour = 0.0f; hour < 24.0f; hour += 3.0f) {
        float temp = world.get_temperature_at_time(lon, lat, altitude, hour);
        float insolation = world.get_insolation(lon, lat, altitude, hour);
        float solar_angle = world.get_solar_angle(lon, lat, hour);
        bool daylight = world.is_daylight(lon, lat, hour);
        
        std::cout << "Hour " << hour << ": "
                  << temp << " °C, "
                  << insolation << " W/m², "
                  << "Sun angle: " << solar_angle << "°, "
                  << (daylight ? "Day" : "Night") << "\n";
    }
    
    return 0;
}
```

### Weather Monitoring

```cpp
#include "rworld/world.h"
#include <iostream>
#include <string>

std::string wind_direction_name(float degrees) {
    if (degrees < 22.5 || degrees >= 337.5) return "N";
    if (degrees < 67.5) return "NE";
    if (degrees < 112.5) return "E";
    if (degrees < 157.5) return "SE";
    if (degrees < 202.5) return "S";
    if (degrees < 247.5) return "SW";
    if (degrees < 292.5) return "W";
    return "NW";
}

int main() {
    rworld::World world;
    
    float lon = 120.0f, lat = -20.0f;  // Western Australia
    float altitude = world.get_terrain_height(lon, lat, 1.0f);
    float current_time = 14.5f;  // 2:30 PM
    
    // Get current weather conditions
    float temp = world.get_temperature_at_time(lon, lat, altitude, current_time);
    float precip_intensity = world.get_current_precipitation(lon, lat, altitude, current_time);
    auto precip_type = world.get_precipitation_type(lon, lat, altitude);
    float wind_speed = world.get_current_wind_speed(lon, lat, altitude, current_time);
    float wind_dir = world.get_current_wind_direction(lon, lat, altitude, current_time);
    float humidity = world.get_humidity(lon, lat, altitude);
    
    std::cout << "Current Weather Report:\n";
    std::cout << "Temperature: " << temp << " °C\n";
    std::cout << "Humidity: " << (humidity * 100) << "%\n";
    std::cout << "Wind: " << wind_speed << " m/s " << wind_direction_name(wind_dir) << "\n";
    
    if (precip_intensity > 0.0f) {
        std::string type = (precip_type == rworld::PrecipitationType::Rain) ? "Rain" :
                          (precip_type == rworld::PrecipitationType::Snow) ? "Snow" : "Sleet";
        std::cout << type << " intensity: " << (precip_intensity * 100) << "%\n";
    } else {
        std::cout << "No precipitation\n";
    }
    
    return 0;
}
```

### Resource Exploration

```cpp
#include "rworld/world.h"
#include <iostream>

int main() {
    rworld::World world;
    
    // Scan a region for resources
    std::cout << "Scanning 10x10 degree region for resources:\n\n";
    
    for (float lat = 40.0f; lat < 50.0f; lat += 1.0f) {
        for (float lon = -10.0f; lon < 0.0f; lon += 1.0f) {
            float coal = world.get_coal_deposit(lon, lat);
            float iron = world.get_iron_deposit(lon, lat);
            float oil = world.get_oil_deposit(lon, lat);
            
            // Report significant deposits (threshold > 0.6)
            if (coal > 0.6f || iron > 0.6f || oil > 0.6f) {
                std::cout << "Location (" << lon << "°, " << lat << "°):\n";
                if (coal > 0.6f) std::cout << "  Coal: " << (coal * 100) << "%\n";
                if (iron > 0.6f) std::cout << "  Iron: " << (iron * 100) << "%\n";
                if (oil > 0.6f) std::cout << "  Oil: " << (oil * 100) << "%\n";
            }
        }
    }
    
    return 0;
}
```

### Hydrology Analysis

```cpp
#include "rworld/world.h"
#include <iostream>

int main() {
    rworld::World world;
    
    float lon = 30.0f, lat = -5.0f;  // Near equator in Africa
    
    // Check for river systems
    if (world.is_river(lon, lat)) {
        float width = world.get_river_width(lon, lat);
        float flow = world.get_flow_accumulation(lon, lat);
        
        std::cout << "River detected:\n";
        std::cout << "Width: " << width << " meters\n";
        std::cout << "Flow accumulation: " << flow << "\n";
        
        if (width > 100.0f) {
            std::cout << "This is a major river!\n";
        }
    } else {
        std::cout << "No river at this location\n";
    }
    
    // Check for volcanic activity
    if (world.is_volcano(lon, lat)) {
        std::cout << "Volcanic feature present\n";
    }
    
    return 0;
}
```

### Seasonal Comparison

```cpp
#include "rworld/world.h"
#include <iostream>

int main() {
    float lon = 0.0f, lat = 50.0f;  // London area
    
    // Compare summer and winter
    rworld::WorldConfig summer_config;
    summer_config.seed = 12345;
    summer_config.day_of_year = 172;  // June 21 (summer solstice)
    
    rworld::WorldConfig winter_config;
    winter_config.seed = 12345;
    winter_config.day_of_year = 355;  // December 21 (winter solstice)
    
    rworld::World summer_world(summer_config);
    rworld::World winter_world(winter_config);
    
    float altitude = summer_world.get_terrain_height(lon, lat, 1.0f);
    float noon = 12.0f;
    
    std::cout << "Location: " << lon << "°, " << lat << "°\n\n";
    
    std::cout << "Summer (June 21):\n";
    float summer_temp = summer_world.get_temperature_at_time(lon, lat, altitude, noon);
    float summer_solar = summer_world.get_solar_angle(lon, lat, noon);
    float summer_insol = summer_world.get_insolation(lon, lat, altitude, noon);
    std::cout << "  Noon temp: " << summer_temp << " °C\n";
    std::cout << "  Solar angle: " << summer_solar << "°\n";
    std::cout << "  Insolation: " << summer_insol << " W/m²\n\n";
    
    std::cout << "Winter (December 21):\n";
    float winter_temp = winter_world.get_temperature_at_time(lon, lat, altitude, noon);
    float winter_solar = winter_world.get_solar_angle(lon, lat, noon);
    float winter_insol = winter_world.get_insolation(lon, lat, altitude, noon);
    std::cout << "  Noon temp: " << winter_temp << " °C\n";
    std::cout << "  Solar angle: " << winter_solar << "°\n";
    std::cout << "  Insolation: " << winter_insol << " W/m²\n";
    
    return 0;
}
```

### Vegetation Mapping

```cpp
#include "rworld/world.h"
#include <iostream>

int main() {
    rworld::World world;
    
    // Create a simple vegetation density map
    std::cout << "Vegetation Density Map (latitude bands):\n\n";
    
    for (float lat = 80.0f; lat >= -80.0f; lat -= 10.0f) {
        std::cout << "Lat " << lat << "°: ";
        
        for (float lon = -180.0f; lon < 180.0f; lon += 20.0f) {
            float altitude = world.get_terrain_height(lon, lat, 1.0f);
            
            if (altitude < 0.0f) {
                std::cout << "~";  // Ocean
            } else {
                float vegetation = world.get_vegetation_density(lon, lat, altitude);
                
                // ASCII density representation
                if (vegetation < 0.1f) std::cout << ".";
                else if (vegetation < 0.3f) std::cout << ",";
                else if (vegetation < 0.5f) std::cout << ";";
                else if (vegetation < 0.7f) std::cout << "o";
                else if (vegetation < 0.9f) std::cout << "O";
                else std::cout << "@";
            }
        }
        std::cout << "\n";
    }
    
    std::cout << "\nLegend: ~ ocean, . barren, , sparse, ; light, o moderate, O dense, @ rainforest\n";
    
    return 0;
}
```

### Batch Query for Performance

```cpp
#include "rworld/world.h"
#include <iostream>
#include <chrono>

int main() {
    rworld::World world;
    
    // Generate a 100x100 grid of locations
    std::vector<rworld::Location> locations;
    for (float lat = -50.0f; lat <= 50.0f; lat += 1.0f) {
        for (float lon = -50.0f; lon <= 50.0f; lon += 1.0f) {
            locations.push_back(rworld::Location(lon, lat, 0.0f, 12.0f, 1.0f));
        }
    }
    
    std::cout << "Querying " << locations.size() << " locations...\n";
    
    // Specify what data we want
    std::vector<rworld::DataType> data_types = {
        rworld::DataType::TERRAIN_HEIGHT,
        rworld::DataType::TEMPERATURE,
        rworld::DataType::BIOME,
        rworld::DataType::PRECIPITATION
    };
    
    // Batch query - much faster than individual calls
    auto start = std::chrono::high_resolution_clock::now();
    rworld::BatchResult result = world.batch_query(locations, data_types);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Batch query completed in " << duration.count() << " ms\n";
    std::cout << "That's " << (locations.size() * 1000.0 / duration.count()) << " queries/second\n\n";
    
    // Access results
    std::cout << "First location:\n";
    std::cout << "  Height: " << result.terrain_height[0] << " m\n";
    std::cout << "  Temperature: " << result.temperature[0] << " °C\n";
    std::cout << "  Biome: " << static_cast<int>(result.biome[0]) << "\n";
    std::cout << "  Precipitation: " << result.precipitation[0] << " mm/yr\n";
    
    // Find highest and lowest points
    float max_height = result.terrain_height[0];
    float min_height = result.terrain_height[0];
    for (float h : result.terrain_height) {
        max_height = std::max(max_height, h);
        min_height = std::min(min_height, h);
    }
    std::cout << "\nElevation range: " << min_height << " to " << max_height << " m\n";
    
    return 0;
}
```

## API Overview

### Core Terrain Methods

- `float get_terrain_height(float longitude, float latitude, float detail_level = 1.0f)` - Terrain elevation in meters (detail_level controls resolution: higher values = more detail for zoomed views)

### Temperature and Climate

- `float get_temperature(float longitude, float latitude, float altitude)` - Base temperature in °C (altitude-adjusted, latitude-based)
- `float get_temperature_at_time(float longitude, float latitude, float altitude, float current_time)` - Dynamic temperature in °C affected by solar heating, time of day, and cloud cover
  - `current_time`: 0.0 to 24.0 hours (0 = midnight, 12 = noon)
  - Returns temperature that varies with sun position and weather conditions

### Biomes and Vegetation

- `BiomeType get_biome(float longitude, float latitude, float altitude)` - Biome classification based on temperature and precipitation
- `float get_vegetation_density(float longitude, float latitude, float altitude)` - Plant coverage density (0.0 to 1.0)
  - 0.0: Barren (deserts, ice, peaks)
  - 0.1-0.3: Sparse (tundra, cold desert)
  - 0.4-0.6: Moderate (grassland, savanna)
  - 0.7-0.9: Dense (temperate forests, taiga)
  - 1.0: Very dense (tropical rainforest)

### Weather and Atmosphere

- `float get_precipitation(float longitude, float latitude, float altitude)` - Annual precipitation in mm/year
- `float get_current_precipitation(float longitude, float latitude, float altitude, float current_time)` - Current precipitation intensity (0.0 to 1.0)
  - Returns 0.0 for no rain, 1.0 for heavy precipitation
  - Varies with time based on weather patterns
  
- `PrecipitationType get_precipitation_type(float longitude, float latitude, float altitude)` - Type of precipitation: `Rain`, `Snow`, or `Sleet`
- `float get_air_pressure(float longitude, float latitude, float altitude)` - Atmospheric pressure in hPa/millibars
- `float get_humidity(float longitude, float latitude, float altitude)` - Relative humidity (0.0 to 1.0)

### Wind System

- `float get_wind_speed(float longitude, float latitude, float altitude)` - Base wind speed in m/s
- `float get_current_wind_speed(float longitude, float latitude, float altitude, float current_time)` - Time-varying wind speed in m/s (±50% variation from base)
- `float get_wind_direction(float longitude, float latitude, float altitude)` - Base wind direction in degrees (0° = North, 90° = East, 180° = South, 270° = West)
- `float get_current_wind_direction(float longitude, float latitude, float altitude, float current_time)` - Time-varying wind direction (±45° variation from base)

### Hydrology

- `bool is_river(float longitude, float latitude)` - Returns true if location is a river
- `float get_river_width(float longitude, float latitude)` - River width in meters (0 if no river)
- `float get_flow_accumulation(float longitude, float latitude)` - Upstream drainage area (higher = larger river)

### Geology and Resources

- `bool is_volcano(float longitude, float latitude)` - Returns true if location is volcanic
- `float get_coal_deposit(float longitude, float latitude)` - Coal concentration (0.0 to 1.0)
- `float get_iron_deposit(float longitude, float latitude)` - Iron ore concentration (0.0 to 1.0)
- `float get_oil_deposit(float longitude, float latitude)` - Oil field concentration (0.0 to 1.0)

### Solar and Time-Based

- `float get_insolation(float longitude, float latitude, float altitude, float current_time)` - Solar radiation in W/m² (0 to ~1400 W/m²)
  - Accounts for sun angle, cloud cover, time of day, and season
  - Returns 0 at night, maximum around local noon
  
- `float get_solar_angle(float longitude, float latitude, float current_time)` - Sun elevation angle in degrees (-90° to 90°)
  - Negative values indicate sun is below horizon (night)
  - Accounts for seasonal variations (23.44° axial tilt)
  
- `bool is_daylight(float longitude, float latitude, float current_time)` - Returns true if sun is above horizon

### Batch Query API

For efficient bulk operations, use the batch query API:

- `BatchResult batch_query(const std::vector<Location>& locations, const std::vector<DataType>& data_types)` - Query multiple locations at once

**Benefits:**
- 10-100x faster than individual queries for bulk operations
- Reduced function call overhead
- Better CPU cache utilization
- Ideal for terrain generation, visualization, and data export

**Available DataTypes:**
- `TERRAIN_HEIGHT`, `TEMPERATURE`, `TEMPERATURE_AT_TIME`
- `BIOME`, `PRECIPITATION`, `CURRENT_PRECIPITATION`, `PRECIPITATION_TYPE`
- `AIR_PRESSURE`, `HUMIDITY`
- `WIND_SPEED`, `CURRENT_WIND_SPEED`, `WIND_DIRECTION`, `CURRENT_WIND_DIRECTION`
- `IS_RIVER`, `RIVER_WIDTH`, `FLOW_ACCUMULATION`
- `IS_VOLCANO`, `COAL_DEPOSIT`, `IRON_DEPOSIT`, `OIL_DEPOSIT`
- `INSOLATION`, `IS_DAYLIGHT`, `SOLAR_ANGLE`
- `VEGETATION_DENSITY`, `SOIL_TYPE`, `SOIL_FERTILITY`, `SOIL_PH`, `ORGANIC_MATTER`
- `PRESSURE_AT_LOCATION`, `PRESSURE_GRADIENT`, `IS_STORM_FRONT`

**Location Struct:**
```cpp
Location(float lon, float lat, float altitude = 0.0f, 
         float current_time = 12.0f, float detail_level = 1.0f)
```

**Example:**
```cpp
std::vector<Location> locs = {{0, 45}, {10, 45}, {20, 45}};
std::vector<DataType> types = {DataType::TEMPERATURE, DataType::BIOME};
BatchResult result = world.batch_query(locs, types);
// Access via result.temperature[i] and result.biome[i]
```

### Coordinate System

- **Longitude**: -180° to 180° (West to East, 0° = Prime Meridian)
- **Latitude**: -90° to 90° (South to North, 0° = Equator)
- **Altitude**: Meters above sea level (negative values = underwater)
- **Time**: 0.0 to 24.0 hours (0 = midnight, 12 = noon, 24 = next midnight)
- **Day of Year**: 0 to 364 (0 = January 1, 172 = June 21, 355 = December 21)

### Time-Dependent Behavior

The library supports dynamic time-based simulations:

- **Temperature Variation**: Changes throughout the day based on solar heating
  - Coldest just before dawn
  - Warmest in early afternoon (2-3 PM)
  - Cloud cover reduces temperature fluctuations
  
- **Solar Radiation**: Varies with time of day, season, and cloud cover
  - Maximum ~1400 W/m² at equator during clear noon
  - Reduced by clouds and atmospheric scattering
  - Zero during night
  
- **Weather Changes**: Wind and precipitation vary dynamically
  - Wind speed fluctuates ±50% from base values
  - Wind direction shifts ±45° over time
  - Precipitation comes and goes based on weather patterns
  
- **Seasonal Effects**: Configure `day_of_year` in `WorldConfig`
  - Solar declination varies with 23.44° axial tilt
  - Affects solar angles, day length, and temperature
  - Summer/winter differences more pronounced at higher latitudes

### Biome Types

The library classifies terrain into 18+ biome types using temperature and precipitation:

**Cold Biomes:**
- `Tundra` - Cold, low precipitation
- `Taiga` - Cold coniferous forest
- `ColdDesert` - Cold, very dry
- `MountainTundra` - High-altitude cold regions
- `Ice` - Permanent ice cover
- `Snow` - Snow-covered terrain

**Temperate Biomes:**
- `TemperateGrassland` - Moderate temperature, moderate precipitation
- `TemperateDeciduousForest` - Seasonal forests
- `TemperateRainforest` - High precipitation, moderate temperature
- `MountainForest` - Mid-altitude forests

**Tropical Biomes:**
- `Savanna` - Warm, seasonal precipitation
- `TropicalSeasonalForest` - Warm, distinct wet/dry seasons
- `TropicalRainforest` - Hot, very high precipitation

**Dry Biomes:**
- `Desert` - Hot, very low precipitation
- `ColdDesert` - Cold, very low precipitation

**Aquatic:**
- `Ocean` - Shallow ocean (<4000m depth)
- `DeepOcean` - Deep ocean (>4000m depth)

**Transition:**
- `Beach` - Coastal areas near sea level
- `MountainPeak` - High-altitude peaks above tree line

## Architecture

The library uses a layered approach:

1. **Noise Layer**: FastNoiseLite generates base terrain and moisture patterns
2. **Environmental Model**: Calculates temperature, pressure based on coordinates
3. **Biome Classification**: Maps environmental parameters to biome types
4. **Facade API**: Simple `World` class coordinates all subsystems

### Design Patterns

- **Facade Pattern**: `World` class provides simple interface to complex subsystems
- **PIMPL Idiom**: Implementation details hidden from public header
- **Strategy Pattern**: Configurable noise parameters and generation rules

## Configuration

Customize world generation through `WorldConfig`:

```cpp
rworld::WorldConfig config;

// Core generation parameters
config.seed = 42;                    // Random seed (same seed = same world)
config.world_scale = 1.0f;           // Overall scale factor

// Time and season
config.day_of_year = 172;            // Day of year (0-364)
                                     // 0 = Jan 1, 80 = March 21 (equinox)
                                     // 172 = June 21 (summer solstice)
                                     // 266 = Sept 23 (equinox)
                                     // 355 = Dec 21 (winter solstice)

// Climate parameters
config.equator_temperature = 30.0f;  // Base temperature at equator (°C)
config.pole_temperature = -40.0f;    // Base temperature at poles (°C)
config.temperature_lapse_rate = 6.5f; // Temperature drop per 1000m altitude (°C)

// Terrain generation
config.terrain_frequency = 0.001f;   // Controls continent size (smaller = larger)
config.terrain_octaves = 6;          // Detail levels (more = more detail)
config.terrain_lacunarity = 2.0f;    // Frequency multiplier per octave
config.terrain_gain = 0.5f;          // Amplitude multiplier per octave

// Moisture and precipitation
config.moisture_frequency = 0.002f;  // Humidity pattern scale
config.moisture_octaves = 4;         // Detail in moisture patterns

rworld::World world(config);
```

### Advanced Configuration

```cpp
// Create worlds at different seasons for comparison
rworld::WorldConfig spring, summer, autumn, winter;
spring.seed = 12345;
spring.day_of_year = 80;   // March 21 - Spring equinox

summer.seed = 12345;
summer.day_of_year = 172;  // June 21 - Summer solstice

autumn.seed = 12345;
autumn.day_of_year = 266;  // September 23 - Autumn equinox

winter.seed = 12345;
winter.day_of_year = 355;  // December 21 - Winter solstice

// Different terrain styles
rworld::WorldConfig detailed, smooth, continental;

detailed.terrain_octaves = 8;      // Very detailed terrain
detailed.terrain_frequency = 0.0015f;

smooth.terrain_octaves = 3;        // Smoother terrain
smooth.terrain_gain = 0.4f;

continental.terrain_frequency = 0.0005f;  // Huge continents
continental.world_scale = 2.0f;
```

## Dependencies

- **FastNoiseLite**: Single-header noise library (included in `third_party/`)
- **C++17** or later
- **CMake 3.15+** (only needed to build the demo)
- **SDL2** (optional, for graphical demo visualization)
  - Ubuntu/Debian: `sudo apt-get install libsdl2-dev`
  - macOS: `brew install sdl2`
  - The demo works without SDL2 (text-mode output only)
- **SDL2_ttf** (optional, for text rendering in demo)
  - Ubuntu/Debian: `sudo apt-get install libsdl2-ttf-dev`
  - macOS: `brew install sdl2_ttf`
  - Without SDL2_ttf, graphical demo shows colored visualizations but no text

## Using in Your Project

### Header-Only Usage

RWorld is a header-only library. To use it:

1. **Copy the headers** to your project:
   - `include/rworld/world.h`
   - `third_party/FastNoiseLite.h`

2. **In exactly ONE .cpp file**, define the implementation before including:
   ```cpp
   #define _RWORLD_IMPLEMENTATION
   #include "rworld/world.h"
   ```

3. **In all other files**, just include normally:
   ```cpp
   #include "rworld/world.h"
   ```

### CMake Integration

```cmake
# Add rworld include directory
target_include_directories(your_target PRIVATE 
    path/to/rworld/include
    path/to/rworld/third_party
)

# That's it! No linking needed for header-only library
```

### Manual Compilation

```bash
# Simply include the path to rworld headers when compiling
g++ -std=c++17 your_app.cpp -Ipath/to/rworld/include -Ipath/to/rworld/third_party -o your_app

# Remember to define _RWORLD_IMPLEMENTATION in exactly one .cpp file
```

## Performance Considerations

- **Caching**: Consider caching results for repeated queries at the same location
- **Batch Processing**: When generating regions, query in a systematic pattern to benefit from CPU cache
- **Detail Level**: Use lower `detail_level` values (0.5-1.0) for distant terrain, higher (2.0-4.0) for close-up views
- **Time Queries**: Static methods (`get_temperature`) are faster than time-varying ones (`get_temperature_at_time`)
- **Typical Performance**: ~100,000-500,000 queries per second on modern hardware (varies by query type)

### Optimization Tips

```cpp
// Cache terrain height for multiple queries at same location
float altitude = world.get_terrain_height(lon, lat, detail_level);

// Use cached altitude for all subsequent queries
float temp = world.get_temperature(lon, lat, altitude);
float precip = world.get_precipitation(lon, lat, altitude);
rworld::BiomeType biome = world.get_biome(lon, lat, altitude);
// ... rather than recalculating altitude each time

// For time-critical applications, query static values once
float base_wind_speed = world.get_wind_speed(lon, lat, altitude);
// Then only query dynamic values when time changes
float current_wind = world.get_current_wind_speed(lon, lat, altitude, time);
```

## License

MIT License - See LICENSE file for details

## Future Enhancements

- Chunk-based caching for performance optimization
- Ocean currents modeling (wind-driven circulation)
- Tectonic plate simulation with realistic mountain formation
- Erosion and geological time simulation
- Biome transition zones and ecotones
- Advanced river features (deltas, meanders, oxbow lakes)
- Glacier and ice sheet dynamics
- Multi-threaded batch processing
- GPU acceleration support

**Recently Implemented:**
- ✅ Batch query API for efficient region generation (10-100x faster)
- ✅ Advanced storm systems with fronts and pressure gradients
- ✅ Soil composition and fertility modeling

## Contributing

Contributions welcome! Please ensure code follows existing style and includes tests.
