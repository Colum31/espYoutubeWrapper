/*******************************************************************
	Read YouTube Channel information from the YouTube API
	and print them to the serial monitor

	Compatible Boards:
	* Any ESP8266 board
	* Any ESP32 board

	Written by Colum31 and Brian Lough
*******************************************************************/

#if defined(ESP8266)
	#include <ESP8266WiFi.h>
#elif defined(ESP32)
	#include <WiFi.h>
#endif

#include <WiFiClientSecure.h>

#include <YoutubeApi.h>
#include <YoutubeChannel.h>
#include <YoutubePlaylist.h>
#include <YoutubeVideo.h>

// Library used for parsing Json from the API responses
// https://github.com/bblanchon/ArduinoJson
// (search for "Arduino Json" in the Arduino Library Manager)
#include <ArduinoJson.h>

//------- Replace the following! ------
#define WIFI_SSID "xxxx"      // your network SSID (name)
#define WIFI_PASSWORD "yyyy"  // your network key
#define API_KEY "zzzz"        // your API key
//------- ---------------------- ------

#define timeBetweenRequestGroup 120 * 1000  // 120 seconds, in milliseconds 	| time between all requests
#define timeBetweenRequests 2 * 1000       // 2 seconds, in milliseconds    	| time between single requests

WiFiClientSecure client;
YoutubeApi api(API_KEY, client);

char channelId[YT_CHANNELID_LEN + 1];
unsigned long startTime;

/**
 * @brief Tries to read an id from Serial.
 * 
 * @return 1 on success, 0 if no data available
 */
int readId(int len, char* var){

	if(Serial.available() > len- 1){

			for(int i = 0; i < len; i++){

				var[i] = Serial.read();
			}

			var[len] = '\0';
			return 1;
	}
	return 0;
}

/**
 * @brief Flushes the Serial input buffer.
 * 
 */
void flushSerialBuffer(){
	while(Serial.available()){
		Serial.read();
	}
}

/**
 * @brief Prints "Yes\n" if x or "No\n" if not x 
 * 
 * @param x parameter
 */
void printYesNo(bool x){
	if(x){
		Serial.println("Yes");
	}else{
		Serial.println("No");
	}
}


void setup() {
	Serial.begin(115200);
	
	// Set WiFi to 'station' mode and disconnect
	// from the AP if it was previously connected
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	delay(100);

	// Connect to the WiFi network
	Serial.print("\nConnecting to WiFi: ");
	Serial.println(WIFI_SSID);
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
		delay(500);
	}
	Serial.println("\nWiFi connected!");
	Serial.print("IP address: ");
	IPAddress ip = WiFi.localIP();
	Serial.println(ip);
		
	client.setInsecure();
	
	// Uncomment for extra debugging info
	// api._debug = true;

	flushSerialBuffer();
	Serial.print("Enter channelId: ");

	while(1){
		if(readId(YT_CHANNELID_LEN, channelId)){
			flushSerialBuffer();
			break;
		}
	}

	Serial.println(channelId);
}

void loop() {

	Serial.setTimeout(timeBetweenRequestGroup);

	YoutubeChannel channel(channelId, &api);

	if(channel.getChannelSnippet()){
		Serial.println("\n\nsnippet");

		channelSnippet *fetchedSnip = channel.channelSnip;


		Serial.print("|----- Channel title: ");
		Serial.println(fetchedSnip->title);

		Serial.print("|----- Channel description: ");
		Serial.println(fetchedSnip->description);

		Serial.print("|----- Channel country: ");
		Serial.println(fetchedSnip->country);

		tm channelCreation = fetchedSnip->publishedAt;

		Serial.print("|----- Channel creation (d.m.y h:m:s): ");
		Serial.print(channelCreation.tm_mday);
        Serial.print(".");
        Serial.print(channelCreation.tm_mon);
        Serial.print(".");
        Serial.print(channelCreation.tm_year + 1900);
        Serial.print(" ");
        Serial.print(channelCreation.tm_hour);
        Serial.print(":");
        Serial.print(channelCreation.tm_min);
        Serial.print(":");
        Serial.println(channelCreation.tm_sec);

		Serial.println("-------------------------------------------------");
	}

	if(channel.getChannelStatistics()){
		Serial.println("\n\nstatistics");

		channelStatistics *fetchedStats = channel.channelStats;

		Serial.print("|----- Channel views: ");
		Serial.println(fetchedStats->viewCount);

		Serial.print("|----- Channel subscriber count hidden? ");
		printYesNo(fetchedStats->hiddenSubscriberCount);

		Serial.print("|----- Channel subscribers: ");
		Serial.println(fetchedStats->subscriberCount);

		Serial.print("|----- Channel video count: ");
		Serial.println(fetchedStats->videoCount);

		Serial.println("-------------------------------------------------");
	}

	delay(timeBetweenRequests);

	if(channel.getChannelContentDetails()){
		Serial.println("\n\ncontent details");

		channelContentDetails *fetchedDetails = channel.channelContentDets;

		Serial.print("|----- Liked videos playlist id: ");
		Serial.println(fetchedDetails->relatedPlaylistsLikes);

		Serial.print("|----- Uploaded videos playlist id: ");
		Serial.println(fetchedDetails->relatedPlaylistsUploads);

		Serial.println("-------------------------------------------------");
	}

	if(channel.checkChannelContentDetailsSet() && channel.checkChannelSnipSet()){
		Serial.print("\n\nFetching last five videos of ");
		Serial.println(channel.channelSnip->title);

		YoutubePlaylist recentVideos = YoutubePlaylist(&api, channel.channelContentDets->relatedPlaylistsUploads);
		recentVideos.getPlaylistItemsPage(0);

		playlistItemsContentDetails *page = recentVideos.itemsContentDets;

		for(int i = 0; i < YT_PLAYLIST_ITEM_RESULTS_PER_PAGE; i++){
			char *videoId = page[i].videoId;

			if(!strcmp("", videoId)){
				break;
			}	

			YoutubeVideo vid = YoutubeVideo(videoId, &api);
			if(vid.getVideoSnippet() && vid.getVideoStatistics()){
				Serial.print("videoId: ");
				Serial.print(videoId);
				Serial.print(" | \"");
				Serial.print(vid.videoSnip->title);
				Serial.print("\" | Views: ");
				Serial.println(vid.videoStats->viewCount);
			}
		}
	}

	Serial.print("\nRefreshing in ");
	Serial.print(timeBetweenRequestGroup / 1000.0);
	Serial.println(" seconds...");
	Serial.print("Or set a new channelId: ");

	startTime = millis();
	flushSerialBuffer();

	while(millis() - startTime < timeBetweenRequestGroup){

		if(readId(YT_CHANNELID_LEN, channelId)){;
			Serial.println(channelId);
			break;
		}
	}
}
