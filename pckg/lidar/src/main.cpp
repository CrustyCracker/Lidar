#include "vl53l0x_api.h"
#include "vl53l0x_platform.h"
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PWM_PATH "/sys/class/pwm/pwmchip0/"
#define MIN_DUTY_CYCLE 0.05
#define MAX_DUTY_CYCLE 0.1
#define SERVO_PERIOD_NS 20000000

#define PORT 8080
#define MAXLINE 1024

// Socket functions
void createSocket(int &sockfd, sockaddr_in &servaddr, sockaddr_in &cliaddr) {
  // Creating socket file descriptor
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));

  // Filling server information
  servaddr.sin_family = AF_INET; // IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(PORT);

  // Bind the socket with the server address
  if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }
}


// Sensor Functions

// Prints the sensor information
void printSensorStatus(VL53L0X_Error Status) {
  char buf[VL53L0X_MAX_STRING_LENGTH];
  VL53L0X_GetPalErrorString(Status, buf);
  printf("Sensor Status: %i : %s\n", Status, buf);
}

// Checks if the sensor works correctly
void checkSensorStatus(VL53L0X_Error Status) {
  if (Status != VL53L0X_ERROR_NONE) {
    printSensorStatus(Status);
    exit(1);
  }
}

// Sensor Initialization
void sensorInit(VL53L0X_Dev_t &dev, VL53L0X_Error &Status) {
  uint32_t refSpadCount;
  uint8_t isApertureSpads;
  uint8_t vhvSettings;
  uint8_t phaseCal;

  // Specify I2C address for sensor
  dev.I2cDevAddr = 0x29;
  dev.fd = VL53L0X_i2c_init("/dev/i2c-1", dev.I2cDevAddr);
  if (dev.fd < 0) {
    Status = VL53L0X_ERROR_CONTROL_INTERFACE;
    printf("Failed to init\n");
  }

  // Data initialization
  checkSensorStatus(Status);
  printf("Call of VL53L0X_DataInit\n");
  Status = VL53L0X_DataInit(&dev);

  // Device initialization
  checkSensorStatus(Status);
  printf("Call of VL53L0X_StaticInit\n");
  Status = VL53L0X_StaticInit(&dev);

  // Device calibration
  checkSensorStatus(Status);
  printf("Call of VL53L0X_PerformRefCalibration\n");
  Status = VL53L0X_PerformRefCalibration(&dev, &vhvSettings, &phaseCal);

  checkSensorStatus(Status);
  printf("Call of VL53L0X_PerformRefSpadManagement\n");
  Status =
      VL53L0X_PerformRefSpadManagement(&dev, &refSpadCount, &isApertureSpads);

  // Set device mode
  checkSensorStatus(Status);
  Status = VL53L0X_SetDeviceMode(&dev, VL53L0X_DEVICEMODE_CONTINUOUS_RANGING);

  // Start Measurement
  checkSensorStatus(Status);
  Status = VL53L0X_StartMeasurement(&dev);
}

// Get distance from sensor
uint16_t getDistance(VL53L0X_Dev_t &dev, VL53L0X_Error &Status) {
  VL53L0X_RangingMeasurementData_t rangingMeasurementData;

  Status = VL53L0X_GetRangingMeasurementData(&dev, &rangingMeasurementData);

  return rangingMeasurementData.RangeMilliMeter;
}

// Servo functions

// Write information to PWM
void writeToPWM(std::string path, std::string content) {
  std::ofstream file;
  file.open(path);
  file << content << "\n";
  file.close();
}

// Rotate servo to a certain angle
void servoRotation(uint8_t angle) {

  // Write period to PWM
  writeToPWM(PWM_PATH "/pwm0/period", std::to_string(SERVO_PERIOD_NS));

  // Calculate duty cycle
  uint32_t dutyCycle =
      (uint32_t)(MIN_DUTY_CYCLE * SERVO_PERIOD_NS + angle * (MAX_DUTY_CYCLE - MIN_DUTY_CYCLE) * SERVO_PERIOD_NS / 180.0);

    printf("Duty cycle:%i\n", dutyCycle);
  // Write duty cycle to PWM
  writeToPWM(PWM_PATH "/pwm0/duty_cycle", std::to_string(dutyCycle));

  // Make servo move
  writeToPWM(PWM_PATH "/pwm0/enable", "1");
  usleep(100000);
  writeToPWM(PWM_PATH "/pwm0/enable", "0");
}

// Clean up on exit
void clearExit() {
  // Sensor close
  VL53L0X_i2c_close();

  // Sevro unexport
  writeToPWM(PWM_PATH "/unexport", "0");
}

int main(int argc, char **argv) {
  // Servo variables
  uint8_t minAngle = 0;
  uint8_t maxAngle = 180;
  uint8_t stepAngle = 1;
  uint8_t currentAngle = minAngle;
  bool goForward = true;

  // Sensor variables
  VL53L0X_Dev_t dev;
  VL53L0X_Error Status = VL53L0X_ERROR_NONE;
  uint16_t distance;

  // Define cleanup on exit
  atexit(clearExit);

  // Initialize servo
  writeToPWM(PWM_PATH "/export", "0");

  // Initialize sensor
  sensorInit(dev, Status);

  // Check for errors before ranging
  checkSensorStatus(Status);
  printSensorStatus(Status);

  int sockfd;
  char buffer[MAXLINE];
  uint16_t x = 0;
  uint16_t y = 0;
  struct sockaddr_in servaddr, cliaddr;

  createSocket(sockfd, servaddr, cliaddr);

  unsigned int len, n;

  len = sizeof(cliaddr); // len is value/result


  // Main loop
  while (1) {
    servoRotation(currentAngle);
    distance = getDistance(dev, Status);
    printf("Distance: %i, angle: %i\n", distance, currentAngle);

    // If angle is out of range, change direction
    if (!goForward && (currentAngle - stepAngle <= minAngle) ||
        goForward && (currentAngle + stepAngle >= maxAngle)) {
      goForward = !goForward;
    }

    // Change current angle
    if (goForward) {
      currentAngle += stepAngle;
    } else {
      currentAngle -= stepAngle;
    }

    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL,
                 (struct sockaddr *)&cliaddr, &len);
    buffer[n] = '\0';

    sendto(sockfd, &currentAngle, sizeof(currentAngle), MSG_CONFIRM,
           (const struct sockaddr *)&cliaddr, len);

    sendto(sockfd, &distance, sizeof(distance), MSG_CONFIRM,
           (const struct sockaddr *)&cliaddr, len);

  }

  return 0;
}
