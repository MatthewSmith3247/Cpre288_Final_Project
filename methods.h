#ifndef METHODS_H_
#define METHODS_H_

//void cybotSendString(char string[50]);
void moveForwardDetect(oi_t *sensor, int centimeters);
int objectCollision();
int fastScan(int startDeg, int endDeg);
int fullScan(int startDeg, int endDeg);
void printIR(oi_t *sensor);
void sendToPutty(char string[], int length);

void objectAvoid(oi_t *sensor);
void secondObject(oi_t *sensor);
void manualDriver(oi_t* sensor_data);
void find_bathroom(oi_t* sensor_data);
void find_kitchen(oi_t* sensor_data);
void find_livingRoom(oi_t* sensor_data);
void find_exit(oi_t* sensor_data);

#endif
