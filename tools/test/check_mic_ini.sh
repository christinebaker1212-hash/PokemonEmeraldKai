#!/bin/sh
# Scenario chatter_microphone's CHECK: the microphone picked is kept in moemon_coop.ini (microphone =)
grep -q "^microphone = TEST MICROPHONE$" "$1/a.ini" || { echo "a.ini lacks microphone = TEST MICROPHONE"; exit 1; }
