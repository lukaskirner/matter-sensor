#include "hs300x.h"
#include <cstdlib>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(hs300x, 4);

#define I2C1_NODE DT_NODELABEL(hs3003)

static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C1_NODE);

#define HS300X_TEMP_MULTY 0.010071415
#define HS300X_TEMP_MIN 40
#define HS300X_HUMD_MULTY 0.006163516
#define HS300X_DELAY_MEASUREMENT 35 // typical on datasheet 16.90 ms, rounded up a little (35ms = 1 try)

int HS300x::Init()
{
	if (!device_is_ready(dev_i2c.bus)) {
		LOG_ERR("I2C bus %s is not ready!", dev_i2c.bus->name);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

float HS300x::ReadTemperature(int units)
{
	_readSensor();

	if (units == FAHRENHEIT) { // Fahrenheit = (Celsius * 9 / 5) + 32
		return (_temperature * 9.0 / 5.0) + 32.0;
	} else {
		return _temperature;
	}
}

float HS300x::ReadHumidity()
{
	_readSensor();
	return _humidity;
}

void HS300x::_readSensor()
{
	int ret;

	ret = i2c_write_dt(&dev_i2c, 0, 1);
	if (ret != 0) {
		LOG_ERR("Failed to write to I2C device address %x", dev_i2c.addr);
	}

	k_msleep(HS300X_DELAY_MEASUREMENT);

	uint8_t buffer[4];
	ret = i2c_read_dt(&dev_i2c, buffer, sizeof(buffer));
	if (ret != 0) {
		LOG_ERR("Failed to read from I2C device address %x", dev_i2c.addr);
	}

	uint16_t _rawHumMSB = buffer[0] << 8; // MSB
	uint16_t _rawHum = buffer[1]; // LSB
	uint16_t _rawTempMSB = buffer[2] << 8;
	uint16_t _rawTemp = buffer[3];

	_rawHum |= _rawHumMSB;
	_rawTemp |= _rawTempMSB;

	_rawHum = _rawHum & 0x3FFF; // mask 2 bit first
	_rawTemp = _rawTemp >> 2; // mask 2 bit last

	if (_rawHum == 0x3FFF)
		return;
	if (_rawTemp == 0x3FFF)
		return;

	_temperature = (_rawTemp * HS300X_TEMP_MULTY) - HS300X_TEMP_MIN;
	_humidity = _rawHum * HS300X_HUMD_MULTY;

	LOG_DBG("Temp: %f, Hum: %f", _temperature, _humidity);
	return;
}
