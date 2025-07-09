#include "main.h"
#include "mpu6050.h"
#include "../../BSP/LCD/lcd.h"
extern I2C_HandleTypeDef hi2c2;

IMU_SensorData_Raw_Structer IMU_SensorData_Raw;

uint8_t BSP_MPU6050_ReadReg(uint8_t address)
{
	uint8_t Value = 0;
	HAL_I2C_Mem_Read(&hi2c2,IMU_ADDRESS_READ,address,I2C_MEMADD_SIZE_8BIT,&Value,1,0x10);
	return Value;
}

void BSP_MPU6050_ReadMultiReg(uint8_t address,uint8_t length,uint8_t *data)
{
	HAL_I2C_Mem_Read(&hi2c2,IMU_ADDRESS_READ,address,I2C_MEMADD_SIZE_8BIT,data,length,0x10);
}

void BSP_MPU6050_WriteReg(uint8_t address,uint8_t data)
{
	HAL_I2C_Mem_Write(&hi2c2,IMU_ADDRESS_WRITE,address,I2C_MEMADD_SIZE_8BIT,&data,1,0x10);
	HAL_Delay(100);
}

void BSP_MPU6050_WriteMultiReg(uint8_t address,uint8_t length,uint8_t *data)
{
	HAL_I2C_Mem_Write(&hi2c2,IMU_ADDRESS_WRITE,address,I2C_MEMADD_SIZE_8BIT,data,length,0x10);
	HAL_Delay(100);
}

uint8_t BSP_MPU6050_Read_WHOAMI(void)
{
	return BSP_MPU6050_ReadReg(MPUREG_WHOAMI);
}

uint8_t BSP_MPU6050_Init(void)
{
	uint8_t tries;

	for(tries = 0; tries<5; tries++)
	{
		BSP_MPU6050_WriteReg(MPUREG_PWR_MGMT_1,BIT_PWR_MGMT_1_DEVICE_RESET);//复位MPU6050
		BSP_MPU6050_WriteReg(MPUREG_PWR_MGMT_1,BIT_PWR_MGMT_1_CLK_ZGYRO);//设置时钟
		if(BSP_MPU6050_ReadReg(MPUREG_PWR_MGMT_1) == BIT_PWR_MGMT_1_CLK_ZGYRO)//判断是否写入，写入则跳出循环
			break;
	}
	if(tries == 5)
		return 0;

	BSP_MPU6050_WriteReg(MPUREG_PWR_MGMT_2,0x00);//禁止休眠
	BSP_MPU6050_WriteReg(MPUREG_USER_CTRL,0x00);//I2C主模式关闭
	BSP_MPU6050_WriteReg(MPUREG_CONFIG,BITS_DLPF_CFG_20HZ);//设置低通滤波器
	BSP_MPU6050_WriteReg(MPUREG_SMPLRT_DIV,MPUREG_SMPLRT_200HZ);//设置采样频率
	BSP_MPU6050_WriteReg(MPUREG_GYRO_CONFIG,BITS_GYRO_FS_2000DPS);//设置陀螺仪量程
	BSP_MPU6050_WriteReg(MPUREG_ACCEL_CONFIG,BITS_ACCEL_FS_8G);//设置加速度计量程
	BSP_MPU6050_WriteReg(MPUREG_INT_ENABLE,0x00);//禁止中断

	if(BSP_MPU6050_Read_WHOAMI() == IMU_WHOAMI)
		return 1;
	else
		return 0;
}

void BSP_MPU6050_UpdateSensors(void)
{
	uint8_t buffer[14];
	BSP_MPU6050_ReadMultiReg(MPUREG_ACCEL_XOUT_H,14,buffer);
	IMU_SensorData_Raw.ACC_X = (buffer[0]<<8)|buffer[1];
	IMU_SensorData_Raw.ACC_Y = (buffer[2]<<8)|buffer[3];
	IMU_SensorData_Raw.ACC_Z = (buffer[4]<<8)|buffer[5];
	IMU_SensorData_Raw.Temp = (buffer[6]<<8)|buffer[7];
	IMU_SensorData_Raw.GYR_X = (buffer[8]<<8)|buffer[9];
	IMU_SensorData_Raw.GYR_Y = (buffer[10]<<8)|buffer[11];
	IMU_SensorData_Raw.GYR_Z = (buffer[12]<<8)|buffer[13];
//	lcd_show_num(5, 230, IMU_SensorData_Raw.ACC_X, 10, 16,  BLUE);
//	     lcd_show_num(15, 210, IMU_SensorData_Raw.ACC_Y, 10, 16,  BLUE);
//	     lcd_show_num(15, 190, IMU_SensorData_Raw.ACC_Z, 10, 16,  BLUE);
//	     lcd_show_num(20, 170, IMU_SensorData_Raw.GYR_X, 10, 16,  BLUE);
//	     lcd_show_num(25, 150, IMU_SensorData_Raw.GYR_Y, 10, 16,  BLUE);
//	     lcd_show_num(30, 130, IMU_SensorData_Raw.GYR_Z, 10, 16,  BLUE);
//	     lcd_show_num(30, 110, IMU_SensorData_Raw.Temp, 10, 16,  BLUE);
}
