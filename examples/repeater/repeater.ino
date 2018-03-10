#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <user_interface.h>

SoftwareSerial repeater(D5, D6);
unsigned long start;
String bitRateTxt("Effective data rate: ");
int rxCount;
int seqErrors;
int expected;
constexpr int ReportInterval = 10000;

void setup()
{
	//system_update_cpu_freq(SYS_CPU_160MHZ);
	Serial.begin(115200);
	WiFi.mode(WIFI_OFF);
	WiFi.forceSleepBegin();
	delay(1);
	repeater.begin(115200);
	start = micros();
	rxCount = 0;
	seqErrors = 0;
	expected = -1;
}

void loop()
{
	while (repeater.available())
	{
		int r = repeater.read();
		if (expected == -1) expected = r;
		else
		{
			expected = ++expected % 256;
		}
		if (r != expected) {
			++seqErrors;
			expected = -1;
			Serial.println(String("tx: ") + static_cast<char>(expected) + " rx: " + static_cast<char>(r));
		}
		++rxCount;
		repeater.write(r);
		if (rxCount >= ReportInterval) break;
	}

	if (rxCount >= ReportInterval) {
		auto end = micros();
		unsigned long interval = end - start;
		long cps = rxCount * (1000000.0 / interval);
		long seqErrorsps = seqErrors * (1000000.0 / interval);
		Serial.println(bitRateTxt + 10 * cps + "bps, " + seqErrorsps + "seq. errors/s");
		start = end;
		rxCount = 0;
		seqErrors = 0;
	}
	wdt_reset();
}