#ifndef BSP_INC_BSP_MPU6050_H_
#define BSP_INC_BSP_MPU6050_H_

#define IMU_WHOAMI  0x68
#define IMU_ADDRESS_READ  0xD1
#define IMU_ADDRESS_WRITE 0xD0

#define MPUREG_SMPLRT_DIV                               0x19    // sample rate.  Fsample= 1Khz/(<this value>+1) = 200Hz
				#define MPUREG_SMPLRT_1000HZ                             0x00
				#define MPUREG_SMPLRT_500HZ                              0x01
				#define MPUREG_SMPLRT_250HZ                              0x03
				#define MPUREG_SMPLRT_200HZ                              0x04
				#define MPUREG_SMPLRT_100HZ                              0x09
				#define MPUREG_SMPLRT_50HZ                               0x13
#define MPUREG_CONFIG                                   0x1A    //µÍÍ¨ÂË²¨ÆµÂÊ
#define MPUREG_GYRO_CONFIG                              0x1B
// bit definitions for MPUREG_GYRO_CONFIG
				#define BITS_GYRO_FS_250DPS                              0x00
				#define BITS_GYRO_FS_500DPS                              0x08
				#define BITS_GYRO_FS_1000DPS                             0x10
				#define BITS_GYRO_FS_2000DPS                             0x18
				#define BITS_GYRO_FS_MASK                                0x18    // only bits 3 and 4 are used for gyro full scale so use this to mask off other bits
				#define BITS_GYRO_ZGYRO_SELFTEST                 		 0x20
				#define BITS_GYRO_YGYRO_SELFTEST                         0x40
				#define BITS_GYRO_XGYRO_SELFTEST                         0x80
#define MPUREG_ACCEL_CONFIG                             0x1C
				#define BITS_ACCEL_FS_2G                                 0x00
				#define BITS_ACCEL_FS_4G                                 0x08
				#define BITS_ACCEL_FS_8G                                 0x10
				#define BITS_ACCEL_FS_16G                                0x18
#define MPUREG_INT_ENABLE                               0x38
#define MPUREG_ACCEL_XOUT_H                             0x3B
#define MPUREG_ACCEL_XOUT_L                             0x3C
#define MPUREG_ACCEL_YOUT_H                             0x3D
#define MPUREG_ACCEL_YOUT_L                             0x3E
#define MPUREG_ACCEL_ZOUT_H                             0x3F
#define MPUREG_ACCEL_ZOUT_L                             0x40
#define MPUREG_TEMP_OUT_H                               0x41
#define MPUREG_TEMP_OUT_L                               0x42
#define MPUREG_GYRO_XOUT_H                              0x43
#define MPUREG_GYRO_XOUT_L                              0x44
#define MPUREG_GYRO_YOUT_H                              0x45
#define MPUREG_GYRO_YOUT_L                              0x46
#define MPUREG_GYRO_ZOUT_H                              0x47
#define MPUREG_GYRO_ZOUT_L                              0x48
#define MPUREG_USER_CTRL                                0x6A
#define MPUREG_PWR_MGMT_1                               0x6B
				#define BIT_PWR_MGMT_1_CLK_INTERNAL              				 0x00            // clock set to internal 8Mhz oscillator
				#define BIT_PWR_MGMT_1_CLK_XGYRO                 				 0x01            // PLL with X axis gyroscope reference
				#define BIT_PWR_MGMT_1_CLK_YGYRO                 				 0x02            // PLL with Y axis gyroscope reference
				#define BIT_PWR_MGMT_1_CLK_ZGYRO                 				 0x03            // PLL with Z axis gyroscope reference
				#define BIT_PWR_MGMT_1_CLK_EXT32KHZ              				 0x04            // PLL with external 32.768kHz reference
				#define BIT_PWR_MGMT_1_CLK_EXT19MHZ              				 0x05            // PLL with external 19.2MHz reference
				#define BIT_PWR_MGMT_1_CLK_STOP                  				 0x07            // Stops the clock and keeps the timing generator in reset
				#define BIT_PWR_MGMT_1_TEMP_DIS                  				 0x08            // disable temperature sensor
				#define BIT_PWR_MGMT_1_CYCLE                             0x20            // put sensor into cycle mode.  cycles between sleep mode and waking up to take a single sample of data from active sensors at a rate determined by LP_WAKE_CTRL
				#define BIT_PWR_MGMT_1_SLEEP                             0x40            // put sensor into low power sleep mode
				#define BIT_PWR_MGMT_1_DEVICE_RESET              				 0x80            // reset entire device

#define MPUREG_PWR_MGMT_2                               0x6C            // allows the user to configure the frequency of wake-ups in Accelerometer Only Low Power Mode

#define MPUREG_WHOAMI                                   0x75

#define BITS_DLPF_CFG_20HZ                              0x04

uint8_t BSP_MPU6050_ReadReg(uint8_t address);
void BSP_MPU6050_ReadMultiReg(uint8_t address,uint8_t length,uint8_t *data);
void BSP_MPU6050_WriteReg(uint8_t address,uint8_t data);
void BSP_MPU6050_WriteMultiReg(uint8_t address,uint8_t length,uint8_t *data);
uint8_t BSP_MPU6050_Read_WHOAMI(void);
uint8_t BSP_MPU6050_Init(void);
void BSP_MPU6050_UpdateSensors(void);


typedef struct
{
	int16_t ACC_X;
	int16_t ACC_Y;
	int16_t ACC_Z;
	int16_t GYR_X;
	int16_t GYR_Y;
	int16_t GYR_Z;
	int16_t Temp;
}IMU_SensorData_Raw_Structer;

#endif /* BSP_INC_BSP_MPU6050_H_ */
