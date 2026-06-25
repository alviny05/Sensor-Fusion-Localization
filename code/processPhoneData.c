#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define temporalThreshold 0.9
#define VelocityThreshold 22        // in cm/sec (22)ex
#define accelerationThreshold 5    // in cm/(sec^2) (5)

// Define Structures
// Structures to Handle Distance, Velocity, and acceleration
struct accelerationVals {
    float accelX;
    float accelY;
    float accelZ;
};

struct velocityVals {
    float velX;
    float velY;
    float velZ;
};

struct distanceVals {
    float distX;
    float distY;
    float distZ;
};


// Global Variables
float dt = 0.1;
struct distanceVals prevDistance1;
struct velocityVals prevVelocity1;

// Function Definitions
void accelerationThresholdFilter(struct accelerationVals *acceleration){
  if(fabs(acceleration->accelX) < accelerationThreshold){
      acceleration->accelX = 0;
  }
  if(fabs(acceleration->accelY) < accelerationThreshold){
      acceleration->accelY = 0;
  }
  if(fabs(acceleration->accelZ) < accelerationThreshold){
      acceleration->accelZ = 0;
  }
}

void velocityThresholdFilter(struct velocityVals *velocity){
  if(fabs(velocity->velX) < VelocityThreshold){
      velocity->velX = 0;
  }
  if(fabs(velocity->velY) < VelocityThreshold){
      velocity->velY = 0;
  }
  if(fabs(velocity->velZ) < VelocityThreshold){
      velocity->velZ = 0;
  }
}

// Uses double integration to calculate distance from acceleration
struct distanceVals doubleIntegrationDistance(struct accelerationVals acceleration){
  // Initialize distanceValues and velocityVals
  struct distanceVals distance;
  struct velocityVals velocity;

  // Calculates velocity
  velocity.velX = prevVelocity1.velX + (acceleration.accelX * dt);
  velocity.velY = prevVelocity1.velY + (acceleration.accelY * dt);
  velocity.velZ = prevVelocity1.velZ + (acceleration.accelZ * dt);

  // Performs Temporal Filtering on Velocity with an LPF
  velocity.velX = (temporalThreshold * velocity.velX) + ((1-temporalThreshold) * prevVelocity1.velX);
  velocity.velY = (temporalThreshold * velocity.velY) + ((1-temporalThreshold) * prevVelocity1.velY);
  velocity.velZ = (temporalThreshold * velocity.velZ) + ((1-temporalThreshold) * prevVelocity1.velZ);

  // Filter the velocity
  velocityThresholdFilter(&velocity);

  // Calculates Distance
  distance.distX = prevDistance1.distX + (prevVelocity1.velX * dt) + (0.5 * acceleration.accelX * pow(dt,2));
  distance.distY = prevDistance1.distY + (prevVelocity1.velY * dt) + (0.5 * acceleration.accelY * pow(dt,2));
  distance.distZ = prevDistance1.distZ + (prevVelocity1.velZ * dt) + (0.5 * acceleration.accelZ * pow(dt,2));

  // Sets previous values to current values
  prevDistance1 = distance;
  prevVelocity1 = velocity;

  // Returns
  return distance;
}


int main() {

    // Counter for offset
    int counter = 0;

    // Offset Variables
    float xAccelOffset = 0;
    float yAccelOffset = 0;
    float zAccelOffset = 0;

    // Set PreviousDistance and previousVelocity values to zero
    prevDistance1.distX = 0;
    prevDistance1.distY = 0;
    prevDistance1.distZ = 0;

    prevVelocity1.velX = 0;
    prevVelocity1.velY = 0;
    prevVelocity1.velZ = 0;

    // Define accel values and distance values
    struct accelerationVals acceleration;
    struct distanceVals distance;

    // Open the Files for reading and writing
    FILE *xfile = fopen("x.txt", "r");
    FILE *yfile = fopen("y.txt", "r");
    FILE *zfile = fopen("z.txt", "r");
    FILE *writeFile = fopen("phoneData.csv","w");

    if (xfile == NULL || yfile == NULL || zfile == NULL) {
        printf("Could not open file\n");
        return 1;
    }

    char xline[256];
    char yline[256];
    char zline[256];


    // Defines values for parsing the string
    const char delimiter[] = ",";
    char *xtoken;
    char *ytoken;
    char *ztoken;

    // Process Data line-by-line
    while (fgets(xline, sizeof(xline), xfile) && fgets(yline, sizeof(yline), yfile) && fgets(zline, sizeof(zline), zfile)) {

        // Parse the data to get values
        xtoken = strtok(xline, delimiter);
        xtoken = strtok(NULL, delimiter);
        ytoken = strtok(yline, delimiter);
        ytoken = strtok(NULL, delimiter);
        ztoken = strtok(zline, delimiter);
        ztoken = strtok(NULL, delimiter);

        // Convert values to float
        float ax = atof(xtoken);
        float ay = atof(ytoken);
        float az = atof(ztoken);

        // Convert from G to cm/s*2
        acceleration.accelX = ax * 981;
        acceleration.accelY = ay * 981;
        acceleration.accelZ = az * 981;

        // Use first 10 pts to calculate offset
        if(counter < 10){
            counter++;
            xAccelOffset += acceleration.accelX;
            yAccelOffset += acceleration.accelY;
            zAccelOffset += acceleration.accelZ;
        }
        // Calculate offset
        else if(counter == 10){
            counter++;
            xAccelOffset /= 10;
            yAccelOffset /= 10;
            zAccelOffset /= 10;
        }
        // Process data
        else{
            // Apply offsets
            acceleration.accelX -= xAccelOffset;
            acceleration.accelY -= yAccelOffset;
            acceleration.accelZ -= zAccelOffset;

            // Filter Accel Data
            accelerationThresholdFilter(&acceleration);

            // Perform double integration
            distance = doubleIntegrationDistance(acceleration);

            // Write to CSV
            fprintf(writeFile, "%f,%f,%f,%f,%f,%f\n", acceleration.accelX, acceleration.accelY, acceleration.accelZ, distance.distX, distance.distY, distance.distZ);
        }
    }

    fclose(xfile);
    fclose(yfile);
    fclose(zfile);
    fclose(writeFile);

    return 0;
}