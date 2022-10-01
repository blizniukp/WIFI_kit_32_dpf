#include "measurement.hpp"

int32_t getByteFromData(char* data, size_t data_len, uint8_t index) {
  char buffer[3] = { 0, 0, 0 };
  if (data_len < index + 1) {
    return 0;
  }
  buffer[0] = data[index];
  buffer[1] = data[index + 1];
  return (strtol(buffer, NULL, 16));
}

bool isCanError(char* response) {
  char* ret = NULL;
  ret = strstr(response, "SEARCHING");
  if (ret) {
    return true;
  }

  ret = strstr(response, "CAN ERROR");
  if (ret) {
    return true;
  }

  ret = strstr(response, "STOPPED");
  if (ret) {
    return true;
  }

  ret = strstr(response, "UNABLE");
  if (ret) {
    return true;
  }

  if (response[0] == '\0') {
    return true;
  }

  return false;
}

bool calcFun_AB(char* data, size_t data_len, float* val, float divider) {
#ifdef RANDOM_DATA
  * val = ((random() % 100) + 1) / divider;
  return (bool)random() % 1;
#endif
  if (isCanError(data)) {
    *val = -100.0f;
    return false;
  }
  *val = (((getByteFromData(data, data_len, 11) << 8) + (getByteFromData(data, data_len, 13))) / divider);
  return true;
}

bool calcFun_ABCD(char* data, size_t data_len, float* val, float divider) {
#ifdef RANDOM_DATA
  * val = ((random() % 100) + 1) / divider;
  return (bool)random() % 1;
#endif
  if (isCanError(data)) {
    *val = -100.0f;
    return false;
  }
  *val = (((getByteFromData(data, data_len, 11) << 24) + (getByteFromData(data, data_len, 13) << 16) + (getByteFromData(data, data_len, 15) << 8) + (getByteFromData(data, data_len, 17))) / divider);
  return true;
}

bool calcFun_Temperature(char* data, size_t data_len, float* val, float divider) {
#ifdef RANDOM_DATA
  * val = ((random() % 100) + 1) / divider;
  return (bool)random() % 1;
#endif
  if (isCanError(data)) {
    *val = -100.0f;
    return false;
  }
  *val = ((((getByteFromData(data, data_len, 11) << 8) + (getByteFromData(data, data_len, 13))) - 2731) / divider);
  return true;
}