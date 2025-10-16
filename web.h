#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <esp_wifi.h>
#include <time.h>
#include <Arduino_JSON.h> //make sure to have the Ardunio_JSON library by Ardunio
#include "ultrasonics.h"
#include "imu.h"


bool toggleSerial = false;
extern float distanceCm, distanceInch;
extern float a_x, a_y, a_z, g_x,g_y,g_z, temp_c;
extern void triggerUltraSonics(bool);
const char *ssid = "webServer";
const char *password = "12345678";
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1); 
//instanciation of server
DNSServer dnsServer;
WebServer server(80);

//defines the web interface
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
    <body>
        <label>Click to toggle the serial</label>
        <button id='button'onclick="toggleSerial()" >Button</button>
        <div id="serial_dataPanel">
            Distance from sensor (cm):<label></label>
            Distance from sensor (in):<label></label>
            <p></p>
            Acceleration(m/s^2):
            x: <label></label>y: <label></label> z:<label></label>
            <p></p>
            Gyro(rad/s):
            x: <label></label>y: <label></label> z:<label></label>
            temperature:
            <label></label>
        </div>
    </body>

    <script>
        var toggled = true;
        async function toggleSerial(){
            // console.log(toggled);
            // if(!toggled){
            //     document.getElementById('button').style.backgroundColor= 'red';
            //     document.getElementById('button').innerHTML = 'Disabled';
            //     toggled = true;
            // }
            // else{
            //     document.getElementById('button').style.backgroundColor = 'green';
            //     document.getElementById('button').innerHTML = 'Enabled';
            //     toggled = false;
            // }

            try {
                const rspd = await fetch('/toggle_serial'
                // ,{
                //     method: 'POST',
                //     headers: {
                //         'Content-Type': 'application/json'
                //     },
                //     body: JSON.stringify({
                //         toggle: toggled
                //     })
                // }
            );
                const respond = await rspd.json();
                console.log('Toggled: ',respond);//respond either on or off
            }catch (e){
                console.error('Action : toggle Serial failed: ',e);
            }
        }
        var cm;
        var inch;
        async function fetchSensorData(){
            try{
                const rspd = await fetch('/serial_data');
                const data = await rspd.json();
                cm = data.cm || 0;
                inch = data.inch || 0;
            }catch (e){
                console.error('Failed to fetch sensor data',e);
            }
            updateData();
        }

        function updateData(){
            try{
                labels = document.querySelectorAll('label');
                labels[1].innerText = cm;
                labels[2].innerText = inch;
                labels[3].innerText = a_x;
                labels[4].innerText = a_y;
                labels[5].innerText = a_z;
                labels[6].innerText = g_x;
                labels[7].innerText = g_y;
                labels[8].innerText = g_z;
                labels[9].innerText = temp;
            }catch (e){
                console.error('Failed to update labels',e);
            }
        }
        //make it such that at each .5 second, it will call fetchSensorData
        setInterval(fetchSensorData,250);
    </script>
</html>
)=====";


void handleSerialToggle(){
  //toggledSerial = server.arg("state");
  triggerUltraSonics(toggleSerial);
  server.send(200,"application/json","{\"status\":\"success\"}");
}

void handleData(){
  JSONVar doc;//creates a json document inside the memory of the esp32
  doc["cm"] = distanceCm;
  doc["inch"] = distanceInch;
  doc["a_x"] = a_x;
  doc["a_y"] = a_y;
  doc["a_z"] = a_z;
  doc["g_x"] = g_x;
  doc["g_y"] = g_y;
  doc["g_z"] = g_z;
  doc["temp"] = temp_c;
  server.send(200,"application/json",JSON.stringify(doc));
}

//This method is meant more than just the index request, but also any error, 
void handleAllRequests(){
  server.send(200, "text/html", MAIN_page);
}

void setupWebServer(void){
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  Serial.println("Wi-Fi AP started at 192.168.4.1");
  dnsServer.start(DNS_PORT, "*", apIP);
  delay(100);
  Serial.println(F(""));
  IPAddress myIP = WiFi.softAPIP();
  Serial.print(F("AP IP address: "));
  Serial.println(myIP);
  server.on("/",HTTP_GET,handleAllRequests);
  //handles serial toggle
  server.on("/toggle_serial",HTTP_GET, handleSerialToggle);
  server.onNotFound(handleAllRequests);//refresh the page upon error(not sure why)

  server.on("/serial_data",HTTP_GET, handleData);
  server.begin();
}