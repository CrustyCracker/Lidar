CXXFLAGS += -Wall -Wextra -Wpedantic

all: lidar examples

libVL53L0X_Rasp.a:
	API_DIR=../VL53L0X_1.0.4 $(MAKE) -C VL53L0X_rasp


examples:
	API_DIR=../VL53L0X_1.0.4 $(MAKE) examples -C VL53L0X_rasp
	mv VL53L0X_rasp/bin/vl53l0x_ContinuousRanging_Example lidarExample1
	mv VL53L0X_rasp/bin/vl53l0x_SingleRanging_Long_Range_Example lidarExample2



lidar: main.cpp libVL53L0X_Rasp.a
	$(CXX) -I VL53L0X_rasp/platform/inc -I VL53L0X_1.0.4/Api/core/inc $(CXXFLAGS) -o lidar main.cpp -LVL53L0X_rasp/bin -lVL53L0X_Rasp -lrt



clean:
	rm -f lidar*
