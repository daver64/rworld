# RWorld - Living Active World Library

A C++ library for procedural generation of living world environments with environmental parameters including terrain, biomes, temperature, precipitation, and atmospheric conditions.

## Features

- **Procedural Terrain Generation**: Continents, oceans, mountains, valleys, and volcanic features
- **Climate Simulation**: Temperature based on latitude, altitude, and season with realistic gradients
- **Biome Classification**: 18+ biome types using Whittaker diagram approach
- **Atmospheric Physics**: Air pressure, humidity, cloud cover with weather-linked density
- **Weather Systems**: Precipitation, wind speed/direction, precipitation type (rain/snow/sleet)
- **Hydrology**: River networks with flow accumulation and width based on drainage
- **Day/Night Cycle**: Realistic solar angles with time-of-day and seasonal variations
- **Insolation Modeling**: Solar radiation affected by latitude, time, cloud cover, and season
- **Vegetation Density**: Realistic plant coverage based on biome, climate, and altitude
- **Resource Distribution**: Coal, iron ore, and oil deposits with geological realism
- **Deterministic Generation**: Same seed produces identical worlds
- **Clean API**: Simple coordinate-based queries (longitude, latitude, altitude, time)
- **Interactive Visualization**: SDL2-based graphical demo with 11+ display modes

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
  - `1-9, 0, V` keys: Switch between display modes
  - `Mouse Wheel`: Zoom in/out at cursor position
  - `SPACE`: Pause/resume time progression
  - `[ / ]`: Decrease/increase time of day
  - `< / >` (Shift+comma/period): Change season
  - `R` key: Regenerate world with random seed
  - `ESC/Q`: Quit application
  - Mouse hover: Shows location details (coordinates, biome, temperature, resources, etc.)

- **Real-time Information Panel**: Displays comprehensive environmental data including:
  - Position, elevation, biome type
  - Temperature, precipitation, humidity
  - Wind speed and direction
  - River information
  - Mineral deposits
  - Time of day and solar data
  - Current season (day of year)
  - Vegetation density

## Quick Start

### Building

```bash
# Install SDL2 for graphical visualization (optional but recommended)
sudo apt-get install libsdl2-dev libsdl2-ttf-dev  # Ubuntu/Debian
# or: brew install sdl2 sdl2_ttf                  # macOS

# Build the library
mkdir build && cd build
cmake ..
cmake --build .
```

### Running the Demo

```bash
./world_demo
```

The demo includes:
- **With SDL2 + SDL2_ttf**: Interactive 1200x600 window with colourful visualization and text info
- **With SDL2 only**: Graphical visualisation with colour-coded indicators (no text)
- **Without SDL2**: Text-based output with ASCII map and data tables

## Usage Example

```cpp
#include "rworld/world.h"

int main() {
    // Create a world with a specific seed
    rworld::WorldConfig config;
    config.seed = 12345;
    rworld::World world(config);
    
    // Query environmental parameters
    float lon = 0.0f, lat = 45.0f;  // Location coordinates
    float altitude = world.get_terrain_height(lon, lat);
    
    float temperature = world.get_temperature(lon, lat, altitude);
    float precipitation = world.get_precipitation(lon, lat, altitude);
    rworld::BiomeType biome = world.get_biome(lon, lat, altitude);
    float pressure = world.get_air_pressure(lon, lat, altitude);
    
    return 0;
}
```

## API Overview

### Core Methods

- `float get_terrain_height(longitude, latitude, detail_level)` - Terrain elevation with zoom-dependent detail
- `float get_temperature(longitude, latitude, altitude)` - Temperature in °C
- `BiomeType get_biome(longitude, latitude, altitude)` - Biome classification
- `float get_precipitation(longitude, latitude, altitude)` - Precipitation in mm/year
- `float get_air_pressure(longitude, latitude, altitude)` - Pressure in hPa/millibars
- `float get_humidity(longitude, latitude, altitude)` - Relative humidity (0-1)
- `PrecipitationType get_precipitation_type(...)` - Rain, snow, or sleet
- `float get_wind_speed(longitude, latitude, altitude)` - Wind speed in m/s
- `float get_wind_direction(longitude, latitude, altitude)` - Wind direction in degrees
- `bool is_river(longitude, latitude)` - Check for river presence
- `float get_river_width(longitude, latitude)` - River width in meters
- `float get_flow_accumulation(longitude, latitude)` - Upstream drainage
- `bool is_volcano(longitude, latitude)` - Check for volcanic features
- `float get_coal_deposit(longitude, latitude)` - Coal concentration (0-1)
- `float get_iron_deposit(longitude, latitude)` - Iron ore concentration (0-1)
- `float get_oil_deposit(longitude, latitude)` - Oil concentration (0-1)
- `float get_insolation(longitude, latitude, time)` - Solar radiation in W/m²
- `float get_solar_angle(longitude, latitude, time)` - Sun elevation angle
- `bool is_daylight(longitude, latitude, time)` - Day/night check
- `float get_vegetation_density(longitude, latitude, altitude)` - Plant coverage (0-1)

### Coordinate System

- **Longitude**: -180° to 180° (West to East)
- **Latitude**: -90° to 90° (South to North)
- **Altitude**: Meters above sea level

### Biome Types

Cold: Tundra, Taiga, Cold Desert
Temperate: Grassland, Temperate Deciduous Forest, Temperate Rainforest
Tropical: Savanna, Tropical Seasonal Forest, Tropical Rainforest
Dry: Desert, Cold Desert
Special: Ocean, Deep Ocean, Beach, Snow, Ice
Mountain: Mountain Tundra, Mountain Forest, Mountain Peak

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
config.seed = 42;
config.world_scale = 1.0f;
config.day_of_year = 172;            // Summer solstice (June 21)
config.equator_temperature = 30.0f;  // °C
config.pole_temperature = -40.0f;    // °C
config.temperature_lapse_rate = 6.5f; // °C per 1000m
config.terrain_frequency = 0.001f;
config.terrain_octaves = 6;

rworld::World world(config);
```

## Dependencies

- **FastNoiseLite**: Single-header noise library (included in `third_party/`)
- **C++17** or later
- **CMake 3.15+**
- **SDL2** (optional, for graphical visualization)
  - Ubuntu/Debian: `sudo apt-get install libsdl2-dev`
  - macOS: `brew install sdl2`
  - The library works without SDL2 (text-mode demo only)

## License

MIT License - See LICENSE file for details

## Future Enhancements

- Chunk-based caching for performance optimization
- Dynamic weather simulation with storm systems
- Batch query API for efficient region generation
- Ocean currents modeling (wind patterns implemented)
- Tectonic plate simulation
- Erosion and geological time simulation

## Contributing

Contributions welcome! Please ensure code follows existing style and includes tests.
