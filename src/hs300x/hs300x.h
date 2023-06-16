#ifndef _HS300x_H_
#define _HS300x_H_

enum { FAHRENHEIT, CELSIUS };

class HS300x {
public:
	static HS300x &Instance()
	{
		static HS300x sHS300x;
		return sHS300x;
	};

	int Init();
	float ReadTemperature(int units = CELSIUS);
	float ReadHumidity();

private:
	float _temperature;
	float _humidity;

	void _readSensor();
};
#endif