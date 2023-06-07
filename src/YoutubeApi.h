#ifndef YoutubeApi_h
#define YoutubeApi_h

#include <Arduino.h>
#include <YoutubeTypes.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <Client.h>

class YoutubeApi
{
	public:
		YoutubeApi(const char *key, Client &newClient);
		YoutubeApi(const String& key, Client& newClient);

		static bool createRequestString(int mode, char *command, const char *id);

		int sendGetToYoutube(const char *command);
		int sendGetToYoutube(const String& command);

		static int allocAndCopy(char **pos, const char *data);
		static tm parseUploadDate(const char *dateTime);
		static tm parseDuration(const char *duration);
		static bool checkEmptyResponse(DynamicJsonDocument response);
		
		bool _debug = false;
		Client &client;

		void closeClient();	

	private:
		static char apiKey[YTAPI_KEY_LEN + 1];
		int getHttpStatusCode();

		void skipHeaders();
};

#endif
