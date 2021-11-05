#!/bin/bash

CONF_PATH=${HOME}/Library/Preferences/com.flacon.flacon.plist


#plutil -convert xml1 ${CONF_PATH}
plutil -p ${CONF_PATH}
