#include <unity.h>
#include "measurement.hpp"

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}
#pragma region isCanError
void test_isCanError_PassSearchingShouldReturnTrue(void) {
  char testData[] = "SEARCHING";
  bool result = isCanError(testData);
  TEST_ASSERT_TRUE(result);
}

void test_isCanError_PassCanErrorShouldReturnTrue(void) {
  char testData[] = "CAN ERROR";
  bool result = isCanError(testData);
  TEST_ASSERT_TRUE(result);
}

void test_isCanError_PassStoppedShouldReturnTrue(void) {
  char testData[] = "STOPPED";
  bool result = isCanError(testData);
  TEST_ASSERT_TRUE(result);
}

void test_isCanError_PassUnableShouldReturnTrue(void) {
  char testData[] = "UNABLE";
  bool result = isCanError(testData);
  TEST_ASSERT_TRUE(result);
}

void test_isCanError_PassEmptyStringShouldReturnTrue(void) {
  char testData[] = "";
  bool result = isCanError(testData);
  TEST_ASSERT_TRUE(result);
}

void test_isCanError_ShouldReturnFalse(void) {
  char testData[] = "INCORRECT DATA";
  bool result = isCanError(testData);
  TEST_ASSERT_FALSE(result);
}
#pragma endregion

#pragma region getByteFromData
void test_getByteFromData_ShouldReturn0(void) {
  char testData[] = "0000000000000";
  size_t testDataLen = strlen(testData);
  uint8_t index = 2;

  int32_t result = getByteFromData(testData, testDataLen, index);
  TEST_ASSERT_EQUAL_INT32(0, result);
}

void test_getByteFromData_ShouldReturn255(void) {
  char testData[] = "00FF000000000";
  size_t testDataLen = strlen(testData);
  uint8_t index = 2;

  int32_t result = getByteFromData(testData, testDataLen, index);
  TEST_ASSERT_EQUAL_INT32(255, result);
}

void test_getByteFromData_IncorectDataLenShouldReturn0(void) {
  char testData[] = "0";
  size_t testDataLen = strlen(testData);
  uint8_t index = 2;

  int32_t result = getByteFromData(testData, testDataLen, index);
  TEST_ASSERT_EQUAL_INT32(0, result);
}
#pragma endregion

#pragma region calcFun_AB
void test_calcFun_AB_ShouldReturnFalse(void) {
  char testData[] = "SEARCHING";
  size_t testDataLen = strlen(testData);
  uint8_t index = 2;
  float value = 0.0f;
  float divider = 1.0f;

  bool result = calcFun_AB(testData, testDataLen, &value, divider, NULL);
  TEST_ASSERT_FALSE(result);
  TEST_ASSERT_EQUAL_FLOAT(-100.0f, value);
}

void test_calcFun_AB_ShouldReturnTrue(void) {
  char testData[] = "7E80562114E013B";
  size_t testDataLen = strlen(testData);
  uint8_t index = 2;
  float value = 0.0f;
  float divider = 1.0f;

  bool result = calcFun_AB(testData, testDataLen, &value, divider, NULL);
  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_EQUAL_FLOAT(315.0f, value);
}
#pragma endregion

#pragma region calcFun_ABCD
void test_calcFun_ABCD_ShouldReturnFalse(void) {
  char testData[] = "SEARCHING";
  size_t testDataLen = strlen(testData);
  uint8_t index = 2;
  float value = 0.0f;
  float divider = 1.0f;

  bool result = calcFun_ABCD(testData, testDataLen, &value, divider, NULL);
  TEST_ASSERT_FALSE(result);
  TEST_ASSERT_EQUAL_FLOAT(-100.0f, value);
}

void test_calcFun_ABCD_ShouldReturnTrue(void) {
  char testData[] = "222222222222222222";
  size_t testDataLen = strlen(testData);
  uint8_t index = 2;
  float value = 0.0f;
  float divider = 100.0f;

  bool result = calcFun_ABCD(testData, testDataLen, &value, divider, NULL);
  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_EQUAL_FLOAT(5726622.0f, value);
}
#pragma endregion

#pragma region calcFun_ABCD
void test_calcFun_Temperature_ShouldReturnFalse(void) {
  char testData[] = "SEARCHING";
  size_t testDataLen = strlen(testData);
  uint8_t index = 2;
  float value = 0.0f;
  float divider = 1.0f;

  bool result = calcFun_Temperature(testData, testDataLen, &value, divider, NULL);
  TEST_ASSERT_FALSE(result);
  TEST_ASSERT_EQUAL_FLOAT(-100.0f, value);
}

void test_calcFun_Temperature_ShouldReturnTrue(void) {
  char testData[] = "222222222222222222";
  size_t testDataLen = strlen(testData);
  uint8_t index = 2;
  float value = 0.0f;
  float divider = 100.0f;

  bool result = calcFun_Temperature(testData, testDataLen, &value, divider, NULL);
  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_EQUAL_FLOAT(60.07f, value);
}
#pragma endregion

#pragma region calcFun_SootLoad
void test_calcFun_SootLoad_ShouldReturnFalse(void) {
  char testData[] = "SEARCHING";
  size_t testDataLen = strlen(testData);
  uint8_t index = 2;
  float value = 0.0f;
  float divider = 1.0f;
  float param = 27.0f;

  bool result = calcFun_SootLoad(testData, testDataLen, &value, divider, &param);
  TEST_ASSERT_FALSE(result);
  TEST_ASSERT_EQUAL_FLOAT(-100.0f, value);
}

void test_calcFun_SootLoad_ShouldReturnTrueAnd100(void) {
  char testData[] = "222222222222222222";
  size_t testDataLen = strlen(testData);
  uint8_t index = 2;
  float value = 0.0f;
  float divider = 100.0f;
  float param = 27.0f;

  bool result = calcFun_SootLoad(testData, testDataLen, &value, divider, &param);
  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_EQUAL_FLOAT(100.0f, value);
}

void test_calcFun_SootLoad_ShouldReturnTrueAnd50(void) {
  /*                 01234567890123456    */
  /*                            HHLL      */
  char testData[] = "000000000000546000000";
  size_t testDataLen = strlen(testData);
  uint8_t index = 2;
  float value = 0.0f;
  float divider = 100.0f;
  float param = 27.0f;

  bool result = calcFun_SootLoad(testData, testDataLen, &value, divider, &param);
  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_EQUAL_FLOAT(50.0f, value);
}
#pragma endregion

void RUN_UNITY_TESTS() {
  UNITY_BEGIN();
  RUN_TEST(test_isCanError_PassSearchingShouldReturnTrue);
  RUN_TEST(test_isCanError_PassStoppedShouldReturnTrue);
  RUN_TEST(test_isCanError_PassStoppedShouldReturnTrue);
  RUN_TEST(test_isCanError_PassUnableShouldReturnTrue);
  RUN_TEST(test_isCanError_PassEmptyStringShouldReturnTrue);
  RUN_TEST(test_isCanError_ShouldReturnFalse);

  RUN_TEST(test_getByteFromData_ShouldReturn0);
  RUN_TEST(test_getByteFromData_ShouldReturn255);
  RUN_TEST(test_getByteFromData_IncorectDataLenShouldReturn0);

  RUN_TEST(test_calcFun_AB_ShouldReturnFalse);
  RUN_TEST(test_calcFun_AB_ShouldReturnTrue);

  RUN_TEST(test_calcFun_ABCD_ShouldReturnFalse);
  RUN_TEST(test_calcFun_ABCD_ShouldReturnTrue);

  RUN_TEST(test_calcFun_Temperature_ShouldReturnFalse);
  RUN_TEST(test_calcFun_Temperature_ShouldReturnTrue);

  RUN_TEST(test_calcFun_SootLoad_ShouldReturnFalse);
  RUN_TEST(test_calcFun_SootLoad_ShouldReturnTrueAnd100);
  RUN_TEST(test_calcFun_SootLoad_ShouldReturnTrueAnd50);
  UNITY_END();
}

int main(int argc, char** argv) {
  RUN_UNITY_TESTS();
  return 0;
}
