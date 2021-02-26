# antsw2
Files for the second antenna switch - a six port version

An ESP8266 provides the WiFi interface.  An ATTiny85 converts serial data from the ESP to select a relay for the antenna port.

The callback code in the controller is not used (yet).  All commands to the controller are via http.
